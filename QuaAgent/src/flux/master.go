package main

import (
	"github.com/Tencent/ModernFlux/QuaAgent/src/fluxpb"
	amsactcon "github.com/Tencent/ModernFlux/QuaAgent/src/gopb"
	"io"
	"log"
	"net"
	"strconv"
	"strings"

	"time"

	"bytes"
	"encoding/binary"
	"errors"
	"runtime/debug"
	"sync/atomic"

	proto "github.com/golang/protobuf/proto"
)

func Handle(conn net.Conn) error {
	defer func() {
		if re := recover(); re != nil {
			debug.PrintStack()
			log.Println("handleClient_err=", re)
		}
	}()

	defer conn.Close()
	conn.SetDeadline(time.Now().Add(time.Duration(60*60*24*365) * time.Second))
	for {
		recvByte, length := ReadBinary(conn)

		if length < 0 {
			atomic.AddInt64(&socknumber, -1)
			return errors.New("Handle Exist Err")
		} else if length > 0 {
			analyseToMaster(recvByte, conn)
		}
	}
}

func analyseToMaster(data []byte, conn net.Conn) {
	defer func() {
		if re := recover(); re != nil {
			debug.PrintStack()
			log.Println("analyseToMaster_err=", re)
		}
	}()

	workIndex := 0
	base_req := &amsactcon.CmdBase{}
	proto.Unmarshal(data, base_req)

	quotaReq := &openappdesc.QuotaReq{}
	proto.Unmarshal(base_req.GetBody(), quotaReq)

	glnum++
	gsnum = "flux-" + string(quotaReq.GetKey()) + "-" +
		string(strconv.FormatInt(time.Now().Unix(), 10)) + "-" +
		strconv.FormatInt(glnum, 10) + "-end"
	var tmp = &ReqData{
		Key:    string(quotaReq.GetKey()),
		Setkey: string(quotaReq.GetSetkey()),
		Tquota: quotaReq.GetTotalquota(),
		Conn:   &conn,
		Serial: gsnum,
	}

	arr := strings.Split(tmp.Setkey, "_")
	if len(arr) >= 2 {
		num, err := strconv.Atoi(arr[1])
		if err == nil {
			workIndex = num % workernum
		}
	}

	select {
	case masters[workIndex] <- tmp:

	default:

	}
}

func ReadBinary(conn net.Conn) ([]byte, int) {
	var err error
	defer func() {
		if re := recover(); re != nil {
			debug.PrintStack()
			log.Println("ReadBinary_err=", re)
		}
	}()
	header_buf := make([]byte, 4)
	_, err = conn.Read(header_buf)

	if err != nil {
		opErr, ok := err.(*net.OpError)
		if ok {
			if opErr.Temporary() {

				return nil, -1
			}

			if opErr.Timeout() {

				return nil, 0
			}
		}

		if err == io.EOF {

			return nil, -1
		}
		return nil, -1
	}

	bl := bytes.NewBuffer(header_buf)
	var length uint32
	binary.Read(bl, binary.BigEndian, &length)
	data_buf := make([]byte, length)
	_, err = conn.Read(data_buf)

	if err != nil {

		return nil, -1
	}
	return data_buf, len(data_buf)
}

func AppHeader(src []byte) []byte {

	bytes_buf := bytes.NewBuffer([]byte{})
	binary.Write(bytes_buf, binary.BigEndian, uint32(len(src)))
	bytes_buf.Write(src)
	return bytes_buf.Bytes()
}
