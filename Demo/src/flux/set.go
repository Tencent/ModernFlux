package flux

import (
	"sync"
	"time"
)

//集群限流数据结构定义
type RateLimitSet struct {
	// set配置
	setQuota         int64
	setSum           *Counter
	setLastResetTime time.Time
	mu               sync.RWMutex

	minQuota  int64
	minEnable bool
}

//加读锁
func (r *RateLimitSet) RLock() {
	r.mu.RLock()
}
//解读锁
func (r *RateLimitSet) RUnlock() {
	r.mu.RUnlock()
}
//加写锁
func (r *RateLimitSet) Lock() {
	r.mu.Lock()
}
//解写锁
func (r *RateLimitSet) UnLock() {
	r.mu.Unlock()
}
