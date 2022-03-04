package flux

import (
	"fmt"
	"sync"
)

const (
	minHardCode = 100 * 10000
)

//限流map with lock
type RateLimitKey struct {
	// key 数据
	data map[string]*RateLimitData
	mu   sync.RWMutex
}

//按key获取限流数据
func (r *RateLimitKey) GetData(business string, key string) (*RateLimitData, bool) {
	r.RLock()
	data, ok := r.data[buildFluxKey(business, key)]
	r.RUnlock()
	return data, ok
}
//添加限流key
func (r *RateLimitKey) AddData(business string, key string, averQuota int64) {
	data := newRateLimitData(business, key, averQuota)
	fmt.Println("添加限流key")
	r.Lock()
	r.data[buildFluxKey(business, key)] = data
	r.UnLock()
}

//加读锁
func (r *RateLimitKey) RLock() {
	r.mu.RLock()
}

//解读锁
func (r *RateLimitKey) RUnlock() {
	r.mu.RUnlock()
}

//加写锁
func (r *RateLimitKey) Lock() {
	r.mu.Lock()
}

//解写锁
func (r *RateLimitKey) UnLock() {
	r.mu.Unlock()
}

func buildFluxKey(business string, key string) string {
	return business + "_" + key
}
