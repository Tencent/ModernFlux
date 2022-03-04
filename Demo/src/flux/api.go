package flux

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

const (
	flowChanBufferSize = 100000
	// 1min = 60s = 6 Window * 10 length
	timeWindowPerMin  = 6
	timeWindowsLength = 10
	updateQuotaInterval = 5
	minQuotaThreshold = 10
	defaultQuota = 50
	dispatchResCodeSuccess = 200000

	//monitor
	errCodeInternalLogicErr    = -100001001
	errCodeInternalChannelFull = -100001002
	errCodeDataFormatErr       = -100001003
	errCodeNetPolarisErr       = -100002001
	errCodeNetHttpErr          = -100002002
	errCodeNetServerErr        = -100002003
)

// 限流API结构体定义
type RateLimitAPI struct {
	// 命名空间、服务名配置
	env                string
	service            string
	//被调服务是否有配置
	hasConfig bool

	// 调度服务测试用
	dispatchIp   string
	dispatchPort int

	// 流控server、日志server配置
	quotaServerEnv   string
	quotaServerName  string
	logServerNameEnv string
	logServerName    string

	//todo: 未命中规则时使用的一个较低qps的流控限制，待确定是否使用

	// 流量控制结果收集管道
	flowRetChan chan flowRetData
	// 上报触发管道
	flowUploadChan chan flowData

	rlKey *RateLimitKey
	rlSet *RateLimitSet
}

type flowData struct {
	Business string
	Key      string
}

type flowRetData struct {
	Business string
	Key      string
	Ret      int
	SetRet   int
	Time     int64
}

var (
	// Export functions
	_ = NewRateLimitAPI
)

// 创建限流API对象
func NewRateLimitAPI(env string, serverName string) (RateLimitAPI, error) {
	if env == "" || serverName == "" {
		return RateLimitAPI{}, fmt.Errorf("empty Env or serverName")
	}

	var r = RateLimitAPI{
		env:                env,
		service:            serverName,
		hasConfig:          true,
		flowRetChan:        make(chan flowRetData, flowChanBufferSize),
		flowUploadChan:     make(chan flowData, flowChanBufferSize),
		rlKey: &RateLimitKey{
			data: make(map[string]*RateLimitData),
			mu:   sync.RWMutex{},
		},
		rlSet: &RateLimitSet{
			setQuota:         0,
			setSum:           new(Counter),
			setLastResetTime: time.Now(),
			mu:               sync.RWMutex{},
			minQuota:         0,
		},
	} // 向调度服务拉取流控server、日志server Polaris服务名 以及全量的配额信息
	err := r.Init()
	if err != nil {
		return RateLimitAPI{}, err
	}

	// start task routine
	r.startTask()

	return r, nil
}

// 向调度服务发起http请求，获取流控server，日志server Polaris 服务名
// 初始化限流规则
func (r *RateLimitAPI) Init() error {
	// 发起http请求

	r.quotaServerEnv = "FluxSetEnv"
	r.quotaServerName = "FluxSetServerName"
	r.logServerNameEnv = "MonitorSetEnv"
	r.logServerName = "MonitorSetServerName"

	//初始化本服务可用实例数目，熟悉MySql、Redis、Docker、微服务等技术
	instancesNum := 1
	// 初始化全局quota信息
	var (
		setQuotaInt = 20
		minQuotaInt = 10
	)

	if r.rlSet.setQuota == 0 {
		r.rlSet.setQuota = int64(setQuotaInt / instancesNum)
	}
	if r.rlSet.minQuota == 0 {
		r.rlSet.minQuota = int64(minQuotaInt / instancesNum)
	}

	r.rlSet.minEnable = true

	// 初始化 各个key的 平均qps
	averQuota := int64(50)


	data, ok := r.rlKey.GetData("Production", "Test")
	if !ok {
		r.rlKey.AddData("Production", "Test", averQuota)
		return nil
	}

	// 如果过去5分钟没有从流控server拉取到动态的配额信息，用调度服务拉取的信息更新平均配额
	if data.updateTime.Before(time.Now().Add(-dispatchServiceRefreshTime * time.Second)) {
		atomic.StoreInt64(&data.averQuota, averQuota)
		atomic.StoreInt64(&data.restrainQuota, averQuota)
	}
	return nil
}

func (r *RateLimitAPI) startTask() {
	go r.consumeCheckRet()
	go r.uploadFlowStat()
	go r.resetStatistics()
	go r.updateGlobalInfo()
}

//设置被调服务命名空间
func (r *RateLimitAPI) SetNamespace(namespace string) {
	r.env = namespace
}

//设置被调服务服务名
func (r *RateLimitAPI) SetService(service string) {
	r.service = service
}

//获取被调服务命名空间
func (r *RateLimitAPI) GetNamespace() string {
	return r.env
}

//获取被调服务服务名
func (r *RateLimitAPI) GetService() string {
	return r.service
}

//设置调度ip，调试使用
func (r *RateLimitAPI) SetDispatchIp(ip string) {
	r.dispatchIp = ip
}

//设置调度服务端口，调试使用
func (r *RateLimitAPI) SetDispatchPort(port int) {
	r.dispatchPort = port
}

//获取调度服务IP，调试使用
func (r *RateLimitAPI) GetDispatchIp() string {
	return r.dispatchIp
}

//获取调度服务端口，调试使用
func (r *RateLimitAPI) GetDispatchPort() int {
	return r.dispatchPort
}

/*
判断请求是否需要限流，返回值含义：
0-可以通过
1-流量超过平均值（总配额/机器数），可以通过
2-流量超出分配的配额，不可通过
3-集群流量超限，不可通过
*/
func (r *RateLimitAPI) CheckQuota(business string, key string) int {
	var (
		ret    = 0
		setRet = 0
	)

	if !r.hasConfig {
		if debug {
			fmt.Println("no config, pass")
		}
		//没有配置过限流规则，直接放过流量
		return 0
	}
	//1s reset setSum
	r.rlSet.Lock()
	now := time.Now()
	if r.rlSet.setLastResetTime.Before(now.Add(-1 * time.Second)) {
		r.rlSet.setLastResetTime = now
		r.rlSet.setSum.Reset()
	}
	r.rlSet.UnLock()
	ret = r.checkKey(business, key, now)
	// set logic
	r.rlSet.Lock()
	if ret < 2 && !r.CheckSet() {
		setRet = 2
	}
	if ret < 2 && setRet == 0 {
		r.rlSet.setSum.Add()
	}
	r.rlSet.UnLock()
	if 2 == setRet {
		ret = 3
	}

	//return
	select {
	case r.flowRetChan <- flowRetData{
		Business: business,
		Key:      key,
		Ret:      ret,
		SetRet:   setRet,
		Time:     now.Unix(),
	}:
		break
	default:
		r.sendWarn(errCodeInternalChannelFull, fmt.Sprintf("flowRetChan is full"))
	}

	return ret
}

//检查set流量是否超限
func (r *RateLimitAPI) CheckSet() bool {
	setSum := r.rlSet.setSum.Get()
	setQuota := atomic.LoadInt64(&r.rlSet.setQuota)
	if debug {
		fmt.Println("set", setSum, setQuota)
		if setSum > setQuota {
			fmt.Println("set over", setSum, setQuota)
		}
	}
	return setSum < setQuota
}

func (r *RateLimitAPI) checkKey(business string, key string, now time.Time) int {
	var ret = 0
	local, ok := r.rlKey.GetData(business, key)
	if ok {
		local.Lock()
		defer local.UnLock()
		if now.Before(local.updateTime) {
			return ret
		}
		//1s reset key nowSum
		nowsum := local.nowSum.AddThenGet()
		fmt.Println("nowsum:", nowsum)
		if local.lastResetTime.Before(now.Add(-1 * time.Second)) {
			fmt.Println("local.lastResetTime.Before(now.Add(-1 * time.Second))")
			if local.maxPer < nowsum {
				local.maxPer = nowsum
			}
			if local.minPer > nowsum {
				local.minPer = nowsum
			}
			flowVary := local.maxPer - local.minPer
			if flowVary > local.flowVary {
				local.flowVary = flowVary
			}
			local.lastResetTime = now
			local.nowSum.Reset()
		}
		//根据配额大小判断是否需要限流
		restrainQuota := atomic.LoadInt64(&local.restrainQuota)
		averQuota := atomic.LoadInt64(&local.averQuota)
		fmt.Println("restrainQuota:", restrainQuota, " averQuota:", averQuota, " nowsum:", nowsum);
		if restrainQuota > minQuotaThreshold && nowsum > restrainQuota {
			ret = 2
		} else if averQuota > minQuotaThreshold && nowsum > averQuota {
			ret = 1
		}
		if debug {
			//fmt.Println(business, key, nowsum, averQuota, restrainQuota)
			if ret == 2 {
				fmt.Println("over ", business, key, nowsum, averQuota, restrainQuota)
			}
		}
		//判断是否需要上报流控server
		if now.After(local.updateTime.Add(updateQuotaInterval * time.Second)) {
			select {
			case r.flowUploadChan <- flowData{
				Business: business,
				Key:      key,
			}:
				break
			default:
				r.sendWarn(errCodeInternalChannelFull, fmt.Sprintf("flowUploadChan is full"))
			}

		}
		//数据统计逻辑
		deltaQuota := averQuota - nowsum
		if deltaQuota < 0 {
			overQuota := 0 - deltaQuota
			if overQuota > local.overMaxQuota {
				local.overMaxQuota = overQuota
			}
		} else {
			if deltaQuota < local.leftMinQuota {
				local.leftMinQuota = deltaQuota
			}
		}
	} else {
		r.rlKey.AddData(business, key, defaultQuota)
		fmt.Println("r.rlKey.AddData(business, key, defaultQuota)")
	}
	return ret
}

func (r *RateLimitAPI) sendWarn(errCode int, errMsg string) {
	fmt.Println("errCode: ", errCode, ", errMsg: ", errMsg)
}
