package main

import (
	"fluxpb"

	"gopb"
	"log"
	"time"

	proto "github.com/golang/protobuf/proto"
)

func quotastart(index int) {
	quotamap := quotamaps[index]
	workerchan := workers[index]
	quotachan := quotas[index]
	rspchan := nets[index]
	pool := NewConnPool(AQUOTA_IP, AQUOTA_PORT, MaxOpenConn)
	defer pool.Shutdown()
	for {
		// process quota rsp from server
		for n := 0; n < 1024; n++ {
			tmpnow := time.Now().Unix()
			flag := false
			select {
			case binrsp := <-rspchan:

				rspbase := &amsactcon.CmdBase{}
				err := proto.Unmarshal(binrsp, rspbase)
				if err != nil {
					log.Println("proto.Unmarshal(binrsp, rspbase)", err)
					continue
				}

				rsp := &openappdesc.QuotaRsp{}
				err = proto.Unmarshal(rspbase.Body, rsp)
				if err != nil {
					log.Println(" proto.Unmarshal(rspbase.Body, rsp)", err)
					continue
				}

				Key := string(rsp.Key)
				if _, exist := quotamap[Key]; !exist {
					tmp := &LOCALDATA{}
					tmp.Updatetime = tmpnow
					quotamap[Key] = tmp
				}

				local := quotamap[Key]


				if tmpnow-local.Sendtime >= 3 {
					local.Sendtime = 0
					break
				}
				local.Sendtime = 0

				local.Updatetime = tmpnow
				local.Index++
				arrind := local.Index % QUOTA_ARRAY_NUM
				local.Qarray[arrind] = int(rsp.GetLocalq())
				local.Localquotaver = local.GetAverQ()
				local.Localquota = int(rsp.GetLocalq())
				local.Totalworkernum = GetPositiveDataInt(int(rsp.GetTotalworkernum()), local.Totalworkernum)
				local.Totalquota = int(rsp.GetTotalq())
				local.Setquota = GetPositiveDataInt(int(rsp.GetSetquota()), local.Setquota)
				local.Setsum = GetPositiveDataInt(int(rsp.GetSetsum()), local.Setsum)
				local.Procquo = GetPositiveDataInt(int(rsp.GetProcquota()), local.Procquo)	
				local.GetAver()
				local.Serial = GetPositiveDataString(string(rsp.Serial), local.Serial)

				rspdata := &RspData{}
				rspdata.Key = string(rsp.Key)
				rspdata.Aquota = local.Localquotaver
				rspdata.Procquo = local.Procquo
				rspdata.Averq = local.Averq
				rspdata.Paverq = local.Paverq
				rspdata.Updatetime = local.Updatetime
				rspdata.Serial = local.Serial
				select {
				case quotachan <- rspdata:

				default:

				}

				local.Lock = 0

			default:

				flag = true
				break
			}

			if flag {
				break
			}
		}

		data := <-workerchan

		now := time.Now().Unix()
		if _, exist := quotamap[data.Key]; !exist {
			tmp := &LOCALDATA{}
			tmp.Updatetime = now
			quotamap[data.Key] = tmp

		}
		local := quotamap[data.Key]

		index, timekey := GetTimeKey(now)
		if local.Timesum[index].Timekey != timekey {
			local.Timesum[index].Timekey = timekey
			local.Timesum[index].Sum = 0
			local.Timesum[index].Refuse = 0
			local.Timesum[index].Setrefuse = 0
			if data.Qret > 0 {
				local.Timesum[index].Sum = 1
			} else {
				local.Timesum[index].Refuse = 1
			}
			if 0 == data.Sret {
				local.Timesum[index].Setrefuse = 1
			}
		} else {
			if data.Qret > 0 {
				local.Timesum[index].Sum += 1
			} else {
				local.Timesum[index].Refuse += 1
			}

			if 0 == data.Sret {
				local.Timesum[index].Setrefuse += 1
			}
		}

		delta := now - local.Updatetime

		if delta >= REPORT_PERIOD {
			if 1 == local.Lock {
				if (local.Sendtime > 0) && ((now - local.Sendtime) > (3 + 1)) {
					local.Lock = 0
					local.Sendtime = 0
				}
			}

			if 0 == local.Lock {
				local.Lock = 1
				local.Sendtime = now
				var base = &amsactcon.CmdBase{
					Type:  proto.Int64(3),
					CmdId: proto.Int64(QUOTA_QUERY_CMD),
				}
				var req = &openappdesc.QuotaReq{
					Key:        []byte(data.Key),
					Totalquota: proto.Int32(data.Tquota),
					Localquota: proto.Int(local.Localquota),
					Id:         proto.Int32(0),
					Nowsum:     proto.Int64(local.Nowsum),
					Workernum:  proto.Int64(1),
					Serial:     []byte(data.Serial),
				}

				req.Reportitem = make([]*openappdesc.ReportItem, 0, 100)
				for n := 0; n < NUM_PER_MIN; n++ {
					if local.Timesum[n].Timekey > 0 {
						item := &openappdesc.ReportItem{
							Timekey:   proto.Int64(local.Timesum[n].Timekey),
							Reportnum: proto.Int64(local.Timesum[n].Sum),
							Refuse:    proto.Int64(local.Timesum[n].Refuse),
							Setrefuse: proto.Int64(local.Timesum[n].Setrefuse),
						}

						req.Reportitem = append(req.Reportitem, item)
					}
				}
				base.Body, _ = proto.Marshal(req)
				src, _ := proto.Marshal(base)

				go pool.TCPSend(AppHeader(src), rspchan)
			}
		}

	}
}

func GetPositiveDataInt(checknum int, data int) int {
	if checknum > 0 {
		return checknum
	}
	return data
}

func GetPositiveDataString(check string, data string) string {
	if len(check) > 0 {
		return check
	}
	return data
}
