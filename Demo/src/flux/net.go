package flux

import (
	"Demo/src/fluxpb"
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/golang/protobuf/proto"
	"net"
	"time"
)

const (
	quotaBuffLen        = 1024 * 4
	quotaServerTimeOut  = 80 //
)

//调度server请求
type DispatchReq struct {
	Type    int                `json:"type"`
	Service DispatchReqService `json:"service"`
}

//调度server请求service字段
type DispatchReqService struct {
	Name string `json:"name"`
	Env  string `json:"namespace"`
}

//调度server回包
type DispatchResp struct {
	Code                 int                     `json:"code"`
	Info                 string                  `json:"info"`
	FluxSDKRateLimitRule []DispatchRespRatelimit `json:"fluxSDKRateLimitRule"`
}

//调度server回包fluxSDKRateLimitRule字段
type DispatchRespRatelimit struct {
	FluxSet        PolarisNaming  `json:"fluxSet"`
	MonitorSet     PolarisNaming  `json:"monitorSet"`
	SetQuota       string         `json:"setQuota"`
	MinQuota       string         `json:"minQuota"`
	MinQuotaEnable string         `json:"minQuotaEnable"`
	KeyQuota       []KeyQuotaRule `json:"keyQuota"`
}

//调度server回包名字服务字段
type PolarisNaming struct {
	Env        string `json:"Env"`
	ServerName string `json:"serverName"`
}

//调度server回包限流规则字段
type KeyQuotaRule struct {
	Business string `json:"business"`
	Key      string `json:"key"`
	Quota    string `json:"quota"`
}

/*
getQuota
sdk <> quota server
upload local flow data, get global flow info, update local quota
through protobuf + tcp
*/
func (r *RateLimitAPI) requestQuotaServer(rateLimitData *RateLimitData) (*pb.QuotaRsp, error) {
	reqBase := &pb.CmdBase{CmdId: int64(quotaQueryCmd), Type: int64(pb.BODY_TYPE_BODY_REQUEST)}
	body := &pb.QuotaReq{
		Key:        []byte(rateLimitData.key),
		Reportitem: make([]*pb.ReportItem, 0),
		Localquota: int32(rateLimitData.quota),
		Id:         int32(1),
		Totalquota: int32(rateLimitData.quota),
		Nowsum:     rateLimitData.nowSum.Get(),
		Workernum:  int64(1),
		Setkey:     []byte(r.service),
		Business:   []byte(rateLimitData.business),
		Setenv:     []byte(r.env)}
	for i := 0; i < timeWindowPerMin; i++ {
		rateLimitData.TimeSum[i].mu.RLock()
		if rateLimitData.TimeSum[i].timeKey > 0 {
			body.Reportitem = append(body.Reportitem, &pb.ReportItem{
				Timekey:   rateLimitData.TimeSum[i].timeKey,
				Reportnum: rateLimitData.TimeSum[i].sum,
				Refuse:    rateLimitData.TimeSum[i].refuse,
				Setrefuse: rateLimitData.TimeSum[i].setRefuse})
		}
		rateLimitData.TimeSum[i].mu.RUnlock()
	}
	fmt.Println("req_body:", body)
	bodyBytes, err := proto.Marshal(body)
	if err != nil {
		fmt.Sprintf("uploadFlowStat Marshal reqBase.body failed, body: %+v,err: %s\n", body, err.Error())
		return nil, err
	}
	reqBase.Body = bodyBytes
	baseBytes, err := proto.Marshal(reqBase)
	if err != nil {
		fmt.Sprintf("uploadFlowStat Marshal reqBase failed, reqBase: %+v,err: %s\n", reqBase, err.Error())
		return nil, err
	}
	var reqLength = uint32(len(baseBytes))
	var data = []interface{}{reqLength, baseBytes}
	buf := new(bytes.Buffer)
	for _, v := range data {
		if err := binary.Write(buf, binary.BigEndian, v); err != nil {
				fmt.Sprintf("uploadFlowStat write tcp packet failed, reqBase: %+v, reqBody: %+v err: %s",
					reqBase, body, err.Error())
			return nil, err
		}
	}
	respBytes, err := r.accessQuotaServerTCP(buf.Bytes())
	if err != nil {
		return nil, err
	}
	pkgLen := getPkgLen(respBytes)
	pbdata := respBytes[4:pkgLen]
	var base pb.CmdBase
	var respBase pb.CmdBaseResp
	var respQuota pb.QuotaRsp
	err = proto.Unmarshal(pbdata, &base)
	if err != nil {
		fmt.Sprintf("unmarshal quota server resp failed, err: %s",
			err.Error())
		return nil, err
	}
	if base.GetType() == int64(pb.BODY_TYPE_BODY_RESPONSE) {
		err = proto.Unmarshal(base.Body, &respQuota)
		if err != nil {
			return nil, err
		}
	} else {
		err = proto.Unmarshal(base.Body, &respBase)
		if err != nil {
			return nil, err
		}
		err = proto.Unmarshal(respBase.Resp, &respQuota)
		if err != nil {
			return nil, err
		}
	}
	return &respQuota, nil
}

//流控server TCP通信封装
func (r *RateLimitAPI) accessQuotaServerTCP(reqBytes []byte) ([]byte, error) {
	d := net.Dialer{Timeout: quotaServerTimeOut * time.Millisecond}
	var getPort uint32 = 19106
	fmt.Println(fmt.Sprintf("%s:%d", "9.134.131.166", getPort))
	conn, err := d.Dial("tcp", "9.134.131.166:19106")
	if err != nil {
		fmt.Println(err)
		return nil, err
	}
	fmt.Println("tcp connect ok")

	_, err = conn.Write(reqBytes)
	defer conn.Close()
	if err != nil {
		fmt.Println(err)
		return nil, err
	}

	fmt.Println("write tcp packet ok")

	resLen := 8192
	bdata := make([]byte, 0)
	tmpdata := make([]byte, resLen)
	total := 0
	start := getTimeInMillsecond()
	end := start
	err = conn.SetReadDeadline(time.Now().Add(quotaServerTimeOut * time.Millisecond))
	if err != nil {
		fmt.Println(err)
		return nil, err
	}
	fmt.Println("SetReadDeadline ok")
	for {
		end = getTimeInMillsecond()
		if end-start > quotaServerTimeOut {
			break
		}
		readLen, err := conn.Read(tmpdata)
		if err != nil {
			fmt.Printf("read data failed, total: %d,bdata: %v,err: %s\n",
				total, bdata, err)
			return nil, fmt.Errorf("read data failed, total: %d,bdata: %v,err: %s",
				total, bdata, err.Error())
		}
		fmt.Println("read packet ok")
		if total+readLen >= resLen {
			return nil, errors.New("read data len is too longer")
		}
		bdata = append(bdata, tmpdata[0:readLen]...)
		total = total + readLen
		if total >= 4 {
			pkgLen := getPkgLen(bdata)
			if pkgLen == -1 {
				return nil, errors.New("res fmt is err")
			}
			if pkgLen >= int32(resLen) {
				return nil, errors.New("read data len is too longer")
			}
			if pkgLen <= int32(total) {
				fmt.Println("receive tcp ", pkgLen, total)
				break
			}
		}
	}
	return bdata, nil
}

func getPkgLen(msg []byte) int32 {
	pkgLen := int32(-1)
	bufReader := bytes.NewReader(msg[0:4])
	err := binary.Read(bufReader, binary.BigEndian, &pkgLen)
	if err != nil {
		fmt.Printf("read head failed, err=%s\n", err.Error())
		return -1
	}
	return pkgLen + 4
}
