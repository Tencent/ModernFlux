package flux

import (
	"sync"
	"time"
)

const (
	quotaArrayNum = 12
)

//限流key结构定义
type RateLimitData struct {
	// key相关参数
	business string
	key      string

	// 限流相关参数
	nowSum        Counter
	lastResetTime time.Time
	restrainQuota int64
	averQuota     int64

	// 统计参数
	maxPer        int64
	minPer        int64
	flowVary      int64
	quota         int64
	quotaMax      int64
	quotaMin      int64
	quotaVary     int64
	leftMinQuota  int64
	overMaxQuota  int64
	averQuotaMax  int64
	averQuotaMin  int64
	averQuotaVary int64

	reqNum       int
	totalReqTime int64
	maxTime      int64
	lastReport   int

	// 上报参数
	updateTime time.Time
	TimeSum    []*TimeSum

	// 拉取参数
	qArray []int64
	qIndex int

	// for updateTime
	mu sync.RWMutex
}

func newRateLimitData(bzNameSpace string, key string, averQuota int64) *RateLimitData {
	rld := RateLimitData{
		business:      bzNameSpace,
		key:           key,
		nowSum:        Counter{},
		lastResetTime: time.Now(),
		restrainQuota: averQuota,
		averQuota:     averQuota,
		maxPer:        0,
		minPer:        minHardCode,
		flowVary:      0,
		quota:         averQuota,
		quotaMax:      0,
		quotaMin:      minHardCode,
		quotaVary:     0,
		leftMinQuota:  minHardCode,
		overMaxQuota:  0,
		averQuotaMax:  0,
		averQuotaMin:  minHardCode,
		averQuotaVary: 0,
		reqNum:        0,
		totalReqTime:  0,
		maxTime:       0,
		lastReport:    time.Now().Minute(),
		updateTime:    time.Unix(0, 0),
		TimeSum:       make([]*TimeSum, timeWindowPerMin),
		qArray:        make([]int64, 60/updateQuotaInterval),
		qIndex:        0,
		mu:            sync.RWMutex{},
	}
	for i, _ := range rld.TimeSum {
		rld.TimeSum[i] = new(TimeSum)
	}

	return &rld
}

//加读锁
func (r *RateLimitData) RLock() {
	r.mu.RLock()
}
//解读锁
func (r *RateLimitData) RUnlock() {
	r.mu.RUnlock()
}
//加写锁
func (r *RateLimitData) Lock() {
	r.mu.Lock()
}
//解写锁
func (r *RateLimitData) UnLock() {
	r.mu.Unlock()
}

func (r *RateLimitData) getAverQ() int64 {
	num := int64(0)
	quota := int64(0)
	for i := 0; i < quotaArrayNum; i++ {
		if r.qArray[i] > 0 {
			num += 1
			quota += r.qArray[i]
		}
	}
	if num > 0 {
		return quota / num
	}

	return quota
}
