package l5

import (
	"fmt"
)

const (
	CL5_LB_TYPE_WRR = iota
	CL5_LB_TYPE_STEP
	CL5_LB_TYPE_MOD
	CL5_LB_TYPE_CST_HASH
	CL5_LB_TYPE_RANDOM
)

var (
	DefaultBalancer = CL5_LB_TYPE_WRR
	ErrNotBalancer  = fmt.Errorf("not set balancer")
	ErrNotFound     = fmt.Errorf("not found")
	ErrInsertFailed = fmt.Errorf("insert failed")
)

/**
 * 负载器定义
 *
 */
type Balancer interface {
	Get() (*Server, error)
	Set(*Server) error
	Remove(*Server) error
	Destory() error
}

//带过期时间的权重轮询调度实现
type weightedRoundRobin struct {
	effectives map[*Server]int
	currents map[*Server]int
}

//取出
func (w *weightedRoundRobin) Get() (*Server, error) {
	var (
		total = 0
		max = 0
		priority *Server
	)
	if w.effectives == nil || len(w.effectives) < 1 {
		return nil, ErrNotFound
	}
	for srv, effective := range w.effectives  {
		total += effective
		w.currents[srv] = w.currents[srv] + effective
		if max < w.currents[srv] {
			priority = srv
			max = w.currents[srv]
		}
	}
	if priority == nil {
		return nil, ErrNotFound
	}
	w.effectives[priority] -= 1
	w.currents[priority] -= total
	if w.effectives[priority] < 1 {
		w.Remove(priority)
	}
	return priority, nil
}

//插入
func (w *weightedRoundRobin) Set(s *Server) error {
	w.currents[s] = 0
	w.effectives[s] = int(s.weight)
	return nil
}

//移除
func (w *weightedRoundRobin) Remove(srv *Server) error {
	delete(w.currents, srv)
	delete(w.effectives, srv)
	return nil
}

//销毁
func (w *weightedRoundRobin) Destory() error {
	w.currents = make(map[*Server]int)
	w.effectives = make(map[*Server]int)
	return nil
}

func NewBalancer(typ int) Balancer {
	switch typ {
	case CL5_LB_TYPE_WRR:
		return &weightedRoundRobin{currents:make(map[*Server]int), effectives:make(map[*Server]int)}
		//@todo
	}
	return nil
}

/*
//带过期时间的权重轮询调度实现
type weightedRoundRobin struct {
	list  []*Server
	index int
	max   int32
	gcd   int32
}

//取出
func (w *weightedRoundRobin) Get() (*Server, error) {
	length := len(w.list)
	if length < 1 {
		return nil, ErrNotFound
	}

	if w.list[w.index].weight <= 0 {
		w.list[w.index].weight++
		srv := w.list[w.index]
		if srv.weight >= 0 {
			if w.index+1 < length {
				w.list = append(w.list[0:w.index], w.list[w.index+1:]...)
			} else {
				//w.list = w.list[0:w.index]
			}
			w.index = 0
		}
		return srv, nil
	}

	for {
		srv := w.list[w.index]
		if srv.weight > w.max {
			w.index = (w.index + 1) % length
			if w.index == 0 {
				if w.max > w.list[length-1].weight {
					w.max = 0
				} else {
					w.max += w.gcd
				}
			}
			return srv, nil
		} else {
			if w.max > w.list[length-1].weight {
				w.max = 0
			} else {
				w.max += w.gcd
			}
			w.index = 0
		}
	}
}

//插入时排序
func (w *weightedRoundRobin) Set(s *Server) error {
	if s.weight <= 0 {
		w.list = append([]*Server{s}, w.list...)
		if w.index > 0 {
			w.index++
		}
		return nil
	}

	length := len(w.list)
	if length == 0 {
		w.list = append(w.list, s)
		w.gcd = GreatestCommonDivider(w.gcd, s.weight)
		return nil
	}

	for i := 0; i < length; i++ {
		if w.list[i].weight > 0 && s.weight > w.list[i].weight {
			relace := append(w.list[0:i:i], s)
			w.list = append(relace, w.list[i:]...)
		} else if i == length-1 {
			w.list = append(w.list, s)
		} else {
			continue
		}
		if w.index > 0 && i >= w.index {
			w.index++
		}
		w.gcd = GreatestCommonDivider(w.gcd, s.weight)
		return nil
	}
	return ErrInsertFailed
}

//移除服务器，并重新计算GCD
func (w *weightedRoundRobin) Remove(srv *Server) error {
	length := len(w.list)
	w.gcd = 1
	for i := length - 1; i >= 0; i-- {
		if w.list[i].Ip.String() == srv.Ip.String() && w.list[i].Port == srv.Port {
			if i == length-1 {
				w.list = w.list[0:i]
			} else {
				go w.list[i].Report(true)
				w.list = append(w.list[0:i], w.list[i+1:]...)
			}
			length--
		} else {
			w.gcd = GreatestCommonDivider(w.gcd, w.list[i].weight)
			if w.index > 0 && w.index >= i {
				w.index--
			}
		}
	}
	return nil
}

//初始化
func (w *weightedRoundRobin) Destory() error {
	w.gcd = 1
	for _, v := range w.list {
		go v.Report(true)
	}
	w.list = make([]*Server, 0)
	w.index = 0
	w.max = 0
	return nil
}

func NewBalancer(typ int) Balancer {
	switch typ {
	case CL5_LB_TYPE_WRR:
		return &weightedRoundRobin{
			list:  []*Server{},
			index: 0,
			max:   0,
			gcd:   1,
		}
		//@todo
	}
	return nil
}
*/