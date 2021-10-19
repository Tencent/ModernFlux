package main

import (
	"log"
	"math/rand"
	"time"
)

const (
	QosCheckTnm         string = "869201"
	SHit                string = "869200"
	SESCAPE             string = "869199"
	s_down_req_hit      string = "869198"
	s_down_req_miss     string = "869197"
	s_qos_status_switch string = "869196"
	s_qos_update        string = "869195"
	s_qos_zero          string = "869194"
	WinNum              int64  = 30
	s_win_len           uint32 = 6
	s_pasttime          int64  = 30  //the past time: second
	s_threshold         uint32 = 500 //absolute quantity TODO
	s_flow_threshold    uint32 = 20  // err rate %
	s_ratio_check       uint32 = 100
)

type QUAWIN struct {
	Item       []QUAITEM
	updatetime int64
}
type QUAITEM struct {
	tTime int64
	suc   uint32
	fail  uint32
}

var quawin *QUAWIN
var Quawin chan QUAWIN

var RetMsCh chan RetMs
var CheckQosCh chan bool

type RetMs struct {
	Ret int32
	Ms  uint64
}

func CheckQos(quawin *QUAWIN) bool {

	ret := true
	qosStatus := true
	InitKeyQua()
	ttime := time.Now().Unix()

	var suc uint32 = 0
	var fail uint32 = 0

	index := ttime % s_pasttime
	var i uint32 = 0
	for ; i < s_win_len; i++ {
		pasttime := quawin.Item[index].tTime
		if ttime > pasttime && ttime-pasttime <= s_pasttime {
			suc += quawin.Item[index].suc
			fail += quawin.Item[index].fail
		}
		index--
		index = (index + WinNum) % WinNum

		if (i + 1) >= s_win_len {
			break
		}
	}

	if fail > s_threshold {
		if fail*(s_ratio_check-s_flow_threshold) > s_flow_threshold*suc {
			proba := GenFlowProba(s_flow_threshold)
			//			log.Println("ppppppppppppppppppppproba:", proba)
			if HitProbaCheck(proba) {

				ret = true
			} else {

				ret = false
			}
		}
	}

	if ret != qosStatus {

		qosStatus = ret
	}
	return ret
}

//常驻进程
func UpdateQos() int {
	RetMsCh = make(chan RetMs, 100)
	CheckQosCh = make(chan bool, 100)
	CheckQosCh <- true

	log.Println("UpdateQosbegin")

	item := make([]QUAITEM, WinNum)
	quawin := &QUAWIN{item, 0}
	for {

		tmpRm := <-RetMsCh

		ttime := time.Now().Unix()
		index := ttime % s_pasttime

		if (0 == quawin.Item[index].tTime) || (ttime > quawin.Item[index].tTime && (ttime-quawin.Item[index].tTime) >= s_pasttime) {
			quawin.Item[index].tTime = ttime
			quawin.Item[index].suc = 0
			quawin.Item[index].fail = 0

		}

		if 0 == tmpRm.Ret {
			quawin.Item[index].suc += 1
		} else {
			quawin.Item[index].fail += 1
		}

		quawin.updatetime = ttime
		IsOk := CheckQos(quawin)
		CheckQosCh <- IsOk

	}

	return 0
}
func UpdateQosToCh(ret int32 /* 0:suc other:fail  -3:timeout .......*/, ms uint64 /*ms*/) int {
	tmp := RetMs{
		Ret: ret,
		Ms:  ms,
	}
	ChanIsFull := 0
	select {
	case RetMsCh <- tmp:
	default:
		ChanIsFull = -1
	}

	return ChanIsFull
}
func CheckQosFromCh() bool {
	ret := true
	select {
	case ret = <-CheckQosCh:
	default:
	}

	return ret
}

func InitKeyQua() {
	if quawin == nil {
		item := make([]QUAITEM, WinNum)
		quawin = &QUAWIN{item, 0}
	}
	return
}

//GenFlowProba 获取历史错误比例
func GenFlowProba(erate uint32) uint32 {
	rate := s_ratio_check
	if erate <= s_ratio_check {
		rate = s_ratio_check - erate
	}
	return rate
}
func HitProbaCheck(target uint32) bool {

	if target >= s_ratio_check {
		return true
	}
	rand.Seed(time.Now().Unix())

	value := rand.Uint32() % s_ratio_check
	if value < target {
		return true
	} else {
		return false
	}
}
