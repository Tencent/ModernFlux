package flux

import (
	"math/rand"
	"sync"
	"time"
)

var (
	debug = false
)

//有reset、set接口的计数器封装
type Counter struct {
	mu sync.Mutex
	n  int64
}

//计数+1
func (c *Counter) Add() {
	c.mu.Lock()
	c.n++
	c.mu.Unlock()
}

//获取当前计数
func (c *Counter) Get() int64 {
	c.mu.Lock()
	n := c.n
	c.mu.Unlock()
	return n
}

//先+1，然后获取计数
func (c *Counter) AddThenGet() int64 {
	c.mu.Lock()
	c.n++
	n := c.n
	c.mu.Unlock()
	return n
}

//重置计数
func (c *Counter) Reset() {
	c.mu.Lock()
	c.n = 0
	c.mu.Unlock()
}

//设定计数大小
func (c *Counter) Set(num int64) {
	c.mu.Lock()
	c.n = num
	c.mu.Unlock()
}

func getTimeInMillsecond() int64 {
	return int64(time.Nanosecond) * time.Now().UnixNano() / int64(time.Millisecond)
}

func rangeIn(low, hi int) int {
	return low + rand.Intn(hi-low)
}

//调试开关
func OpenDebug(isDebug bool) {
	debug = isDebug
}
