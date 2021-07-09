package main

import (
	"fluxpb"

	"fmt"
	"gopb"
	"net"
	"sync/atomic"
	"time"

	proto "github.com/golang/protobuf/proto"
)

func workerstart(index int) {

	reqchan := masters[index]
	realmap := workermaps[index]
	workerchan := workers[index]
	quotachan := quotas[index]
	period := time.Now().Unix()
	for {
		// process quota rsp
		for n := 0; n < 1024; n++ {
			flag := false
			select {
			case rsp := <-quotachan:

				if len(rsp.Key) > 0 {
					if _, exist := realmap[rsp.Key]; !exist {
						tmp := &QUOTA{0, 0, 0, 0, 0, 0, 0}
						realmap[rsp.Key] = tmp
					}

					realmap[rsp.Key].Aquota = rsp.Aquota
					realmap[rsp.Key].Procquo = rsp.Procquo
					realmap[rsp.Key].Updatetime = rsp.Updatetime
					realmap[rsp.Key].Averq = rsp.Averq
					realmap[rsp.Key].Paverq = rsp.Paverq
				}
			default:
				flag = true

			}
			if flag {
				break
			}
		}

		tnow := time.Now().Unix()
		if (tnow - period) > (24 * 60 * 60) {
			for k, v := range realmap {
				if (tnow - v.Times) > 3*24*60*60 {
					delete(realmap, k)
				}
			}
			period = tnow
		}

		data := <-reqchan

		now := time.Now().Unix()
		if _, exist := realmap[data.Key]; !exist {
			tmp := &QUOTA{0, 0, 0, 0, 0, 0, 0}
			realmap[data.Key] = tmp
		}
		nowdata := realmap[data.Key]
		heartbeat := (now > nowdata.Updatetime) && ((now - nowdata.Updatetime) > 60*5)
		threshold := nowdata.Aquota <= s_threshold_per_second

		setret := 1
		ret := 1
		if now != nowdata.Times {
			nowdata.Sum = 1
			nowdata.Times = now
		} else {
			nowdata.Sum++
			if heartbeat || threshold {
				if nowdata.Averq > 0 && nowdata.Sum >= nowdata.Averq {

					ret = 0
				}
			} else {
				if nowdata.Aquota > 0 && nowdata.Sum >= nowdata.Aquota {
					ret = 0
				}
			}
		}

		tmpsnow := atomic.LoadInt64(&setnow)
		if tmpsnow != now {
			atomic.StoreInt64(&setnow, now)
			atomic.StoreInt64(&setnum, 1)
		} else {
			atomic.AddInt64(&setnum, 1)
		}
		if ret > 0 {
			if heartbeat || threshold {
				if nowdata.Paverq > 0 && nowdata.Sum >= nowdata.Paverq {
					setret = 0
				}
			} else {
				if nowdata.Procquo > 0 && nowdata.Sum >= nowdata.Procquo {
					setret = 0
				}
			}
		}

		//response
		var rspb = &openappdesc.QuotaRsp{
			Key:       []byte(data.Key),
			Localq:    proto.Int64(int64(ret)),
			Procquota: proto.Int32(int32(setret)),
			Serial:    []byte(data.Serial),
		}

		tmpbody, _ := proto.Marshal(rspb)
		var base = &amsactcon.CmdBase{
			Type:  proto.Int64(2), //BODY_RESPONSE
			ModId: proto.Int64(QUOTA_QUERY_CMD),
			Body:  tmpbody,
		}
		tmpbase, _ := proto.Marshal(base)

		go func(conn net.Conn, src []byte) {

			_, err := conn.Write(AppHeader(src))
			if err != nil {
				fmt.Println("sendrsp1", err)
			}

		}(*(data.Conn), tmpbase)

		data.Qret = ret
		data.Sret = setret

		select {
		case workerchan <- data:
		default:
		}
	}
}
