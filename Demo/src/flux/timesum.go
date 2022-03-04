package flux

import (
	"sync"
	"sync/atomic"
)

//流控server上报流量信息结构定义
type TimeSum struct {
	timeKey   int64
	sum       int64
	refuse    int64
	setRefuse int64
	mu        sync.RWMutex
}

//重置接口
func (t *TimeSum) Reset(timekey int64) {
	atomic.StoreInt64(&t.timeKey, timekey)
	atomic.StoreInt64(&t.sum, 0)
	atomic.StoreInt64(&t.refuse, 0)
	atomic.StoreInt64(&t.setRefuse, 0)
}
//加读锁
func (t *TimeSum) RLock() {
	t.mu.RLock()
}
//解读锁
func (t *TimeSum) RUnlock() {
	t.mu.RUnlock()
}
//加写锁
func (t *TimeSum) Lock() {
	t.mu.Lock()
}
//解写锁
func (t *TimeSum) UnLock() {
	t.mu.Unlock()
}

