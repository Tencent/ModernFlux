package flux

import (
	"Demo/src/fluxpb"
	"fmt"
	"sync/atomic"
	"time"
)

const (
	quotaQueryCmd              = 150
	dispatchServiceRefreshTime = 5 * 60
	quotaFloatPercent          = 0
)

func (r *RateLimitAPI) consumeCheckRet() {

	for {
		select {
		case ret, ok := <-r.flowRetChan:
			if !ok {
				r.sendWarn(errCodeInternalLogicErr,
					fmt.Sprintf("flowRetChan closed, quit consumeFlowData routine"))
				return
			}

			// key logic
			local, ok := r.rlKey.GetData(ret.Business, ret.Key)
			if !ok {
				r.sendWarn(errCodeInternalLogicErr,
					fmt.Sprintf("consumeCheckRet key not found, business: %s, key: %s", ret.Business, ret.Key))
				break
			}
			nowUnix := ret.Time
			index, timekey := r.getTimeKey(nowUnix)
			index = index % timeWindowPerMin

			local.TimeSum[index].Lock()
			if local.TimeSum[index].timeKey != timekey {
				// Init
				local.TimeSum[index].Reset(timekey)
			}
			// count
			if ret.Ret < 2 {
				local.TimeSum[index].sum += 1
			} else {
				local.TimeSum[index].refuse += 1
			}

			if ret.SetRet == 2 {
				local.TimeSum[index].setRefuse += 1
			}
			local.TimeSum[index].UnLock()
		}
	}
}

func (r *RateLimitAPI) getTimeKey(nowUnix int64) (int64, int64) {
	left := nowUnix % 60
	index := left / timeWindowsLength
	base := nowUnix - left
	timekey := base + index*timeWindowsLength
	return index, timekey
}

func (r *RateLimitAPI) uploadFlowStat() {
	for {
		select {
		case flow, ok := <-r.flowUploadChan:
			if !ok {
				r.sendWarn(errCodeInternalLogicErr,
					fmt.Sprintf("flowUploadChan closed, quit consumeFlowData routine"))
				return
			}
			data, ok := r.rlKey.GetData(flow.Business, flow.Key)
			if !ok {
				r.sendWarn(errCodeInternalLogicErr,
					fmt.Sprintf("uploadFlowStat rlKey not found, business: %s, key: %s",
						flow.Business, flow.Key))
				continue
			}
			if time.Now().Before(data.updateTime.Add(updateQuotaInterval * time.Second)) {
				// 过去5秒有upload过 结束
				continue
			}

			//发送tcp请求
			start := time.Now()
			quotaResp, err := r.requestQuotaServer(data)
			fmt.Printf("resp: %+v\n", quotaResp)
			if err != nil {
				r.sendWarn(errCodeNetServerErr,
					fmt.Sprintf("requestQuotaServer failed,time cost: %+v, err: %s\n",
						time.Now().Sub(start), err.Error()))
				continue
			}
			end := time.Now()
			delta := end.Sub(start).Milliseconds()
			//更新本地配额、统计数据信息

			fmt.Printf("update local data, time cost: %+v, resp: %+v\n", delta, quotaResp)
			r.updateLocalData(data, quotaResp, delta)
		}
	}
}

func (r *RateLimitAPI) updateLocalData(data *RateLimitData, quotaResp *pb.QuotaRsp, delta int64) {
	data.Lock()
	defer data.UnLock()
	data.updateTime = time.Now()
	data.reqNum += 1
	if delta > data.maxTime {
		data.maxTime = delta
	}
	data.totalReqTime += delta
	if quotaResp.Key == nil {
		return
	}
	if data.key == string(quotaResp.Key)  {
		if quotaResp.Localq != 0 {

			//key配额更新
			data.qIndex = (data.qIndex + 1) % quotaArrayNum
			data.qArray[data.qIndex] = quotaResp.Localq
			atomic.StoreInt64(&data.averQuota, data.getAverQ())
			if data.averQuota > data.averQuotaMax {
				data.averQuotaMax = data.averQuota
			}
			if data.averQuota < data.averQuotaMin {
				data.averQuotaMin = data.averQuota
			}
			averQuotaVary := data.averQuotaMax - data.averQuotaMin
			if averQuotaVary > data.averQuotaVary {
				data.averQuotaVary = averQuotaVary
			}
			atomic.StoreInt64(&data.restrainQuota, (data.averQuota*(100+quotaFloatPercent))/100)
			fmt.Println("atomic.StoreInt64(&data.restrainQuota, (data.averQuota*(100+quotaFloatPercent))/100)")
			if data.quota != quotaResp.Localq {
				data.quota = quotaResp.Localq
			}
			// 统计数据
			if data.quota > data.quotaMax {
				data.quotaMax = data.quota
			}
			if data.quota < data.quotaMin {
				data.quotaMin = data.quota
			}
			quotaVary := data.quotaMax - data.quotaMin
			if quotaVary > data.quotaVary {
				data.quotaVary = quotaVary
			}
			averQuotaVery := data.averQuotaMax - data.averQuotaMin
			if averQuotaVery > data.averQuotaVary {
				data.averQuotaVary = averQuotaVery
			}
			//set配额、流量更新
			r.rlSet.Lock()
			defer r.rlSet.UnLock()
			if quotaResp.Setsum != 0 {
				r.rlSet.setSum.Set(int64(quotaResp.Setsum))
			} else {
				r.sendWarn(errCodeDataFormatErr,
					fmt.Sprintf("requestQuotaServer receive nil setsum, resp:%s\n",
						quotaResp.String()))
			}

			if quotaResp.Setquota != 0 {
				if debug {
					fmt.Println("before update set quota", atomic.LoadInt64(&r.rlSet.setQuota), int64(quotaResp.Setquota))
				}

				atomic.StoreInt64(&r.rlSet.setQuota, int64(quotaResp.Setquota))

				if debug {
					fmt.Println("after update set quota", atomic.LoadInt64(&r.rlSet.setQuota), int64(quotaResp.Setquota))
				}
			}

		}
	} else {
		r.sendWarn(errCodeInternalLogicErr,
			fmt.Sprintf("quota server key & business not match,"+
				" local business: %s, key: %s, server business: %s, key: %s",
				data.business, data.key, "quotaResp.Businuss", string(quotaResp.Key)))
	}
}

func (r *RateLimitAPI) resetStatistics() {
	for {
		nowMin := time.Now().Minute()
		r.rlKey.RLock()
		for _, v := range r.rlKey.data {
			if v.lastReport == nowMin {
				continue
			}

			v.Lock()
			fmt.Printf("reset stat for %s-%s, data: %+v\n", v.business, v.key, v)
			v.reqNum = 0
			v.totalReqTime = 0
			v.maxTime = 0

			v.maxPer = 0
			v.minPer = minHardCode
			v.flowVary = 0

			v.quotaMax = 0
			v.quotaMin = minHardCode
			v.quotaVary = 0

			v.averQuotaMax = 0
			v.averQuotaMin = minHardCode
			v.averQuotaVary = 0

			v.leftMinQuota = minHardCode
			v.overMaxQuota = 0
			v.lastReport = nowMin
			v.UnLock()
		}
		r.rlKey.RUnlock()
		time.Sleep(time.Second)
	}

}

func (r *RateLimitAPI) updateGlobalInfo() {
	for {
		time.Sleep(dispatchServiceRefreshTime * time.Second)
		_ = r.Init()
	}
}
