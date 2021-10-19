package main

import (
	_ "log"

	"net"
	_ "os"
)

var glnum int64 = 0
var gsnum string = "start"
var workernum int = 1
var masters []chan *ReqData
var workers []chan *ReqData
var quotas []chan *RspData
var nets []chan []byte
var workermaps []map[string]*QUOTA
var quotamaps []map[string]*LOCALDATA
var setnow int64 = 0
var setnum int64 = 0

const (
	total_link               string = "869188" //创建链接
	total_requst             string = "869187" // 总请求数
	write_to_masterchan_suc  string = "869186" //写入masterChan 成功
	write_to_masterchan_fail string = "869185" //写入masterChan 失败
	read_from_masterchan     string = "869184" //从masterChan 读取
	write_to_workerchan_suc  string = "869183" //写入 workerchan 成功
	write_to_workerchan_fail string = "869182" //写入workerchan 失败
	read_from_workerchan     string = "869181" // 从workerchan 读取
	write_to_quotachan_suc   string = "869180" //写入 quotachan 成功
	write_to_quotachan_fail  string = "869179" // 写入 quotachan  失败
	read_from_quotachan_suc  string = "869178" // 读取 quotachan 成功
	read_from_quotachan_no   string = "869177" // 读取 quotachan   失败
	write_to_netsChan        string = "869176" //写入 netsChan
	read_from_netsChan_suc   string = "869175" // 读取 netsChan 成功
	read_from_netsChan_no    string = "869174" // 读取netsChan 失败
	link_break               string = "869173" // link break
	link_continue            string = "869172"
	send_to_service_fail     string = "869171"
	pool_send_fail_acquire   string = "869170" //发送到后端失败
	pool_send_fail_write     string = "869160"
	pool_send_fail_read      string = "869159"
	quoto_tcpsend            string = "869169" //发送到后端请求数
	write_to_netsChan_fail   string = "869168"
	active_quota_up          string = "869143"
	active_aver_up           string = "869144"
	set_quota_up             string = "869145"
	set_aver_up              string = "869142"
	flux_heart_beat          string = "869138"
	net_temp_err             string = "869134"
	net_timeout_err          string = "869135"
	net_eof_err              string = "869136"
	net_unknown_err          string = "869137"
	net_sock_num             string = "869133"
)

const (
	NUM_PER_MIN            int   = 6
	NUM_PER_INDEX          int64 = 10
	REPORT_PERIOD          int64 = 5
	QUOTA_QUERY_CMD        int64 = 150
	AQUOTA_IP              string = "127.0.0.1"
	AQUOTA_PORT            string = "9999"
	QUOTA_ARRAY_NUM        int   = 12
	MAXINDEX               int   = 100
	MaxOpenConn            int   = 10
	s_ratio                int   = 100
	QUOTA_FLOAT_PERCENT    int   = 0
	s_threshold_per_second int   = 0
	Timeout                int   = 11
)

type ReqData struct {
	Key    string
	Setkey string
	Tquota int32
	Conn   *net.Conn
	Qret   int
	Sret   int
	Serial string
}

type RspData struct {
	Key        string
	Setkey     string
	Aquota     int
	Procquo    int
	Averq      int
	Paverq     int
	Times      int64
	Serial     string
	Updatetime int64
}

type QUOTA struct {
	Sum        int
	Aquota     int
	Procquo    int
	Averq      int
	Paverq     int
	Times      int64
	Updatetime int64
}

type TIMESUM struct { // client
	Timekey   int64
	Sum       int64
	Refuse    int64
	Setrefuse int64
}

type LOCALDATA struct { // client
	Timesum    [NUM_PER_MIN]TIMESUM
	Totalquota int

	Localquota    int
	Localquotaver int
	Averq         int

	Nowsum     int64
	Updatetime int64 // last update time
	Now        int64
	Sendtime   int64 //

	Index          int
	Qarray         [QUOTA_ARRAY_NUM]int
	Lock           int
	Totalworkernum int

	// set logic
	Setquota int
	Setsum   int
	Procquo  int
	Paverq   int

	Serial string
}

func (local *LOCALDATA) GetAverQ() int {
	ret := 0
	num := 0
	quota := 0
	for i := 0; i < QUOTA_ARRAY_NUM; i++ {
		if local.Qarray[i] > 0 {
			num++
			quota = quota + local.Qarray[i]
		}
	}

	if num > 0 {
		ret = quota / num
	}

	return ret
}

func (local *LOCALDATA) ClearQuota() {
	for i := 0; i < QUOTA_ARRAY_NUM; i++ {
		local.Qarray[i] = 0
	}
}

func (item *TIMESUM) Init(tkey int64) {
	item.Timekey = 0
	item.Sum = 0
	item.Refuse = 0
	item.Setrefuse = 0
}

func (local *LOCALDATA) GetAver() bool {
	local.Averq = local.Totalquota
	local.Paverq = local.Setquota
	//fmt.Println(local.Averq, local.Paverq, local.Totalworkernum)
	if local.Totalworkernum > 0 {
		local.Averq = local.Totalquota / local.Totalworkernum
		local.Paverq = local.Setquota / local.Totalworkernum
	}

	return true
}

func (local *LOCALDATA) Init(times int64) {
	for i := 0; i < NUM_PER_MIN; i++ {
		local.Timesum[i].Init(times)
	}
}

func GetTimeKey(ttime int64) (int64, int64) {
	left := ttime % 60
	index := left / NUM_PER_INDEX
	base := ttime - left
	timekey := base + index*NUM_PER_INDEX
	return index, timekey
}

type ConnStruct struct {
	Conn     net.Conn
	Index    int
	ModId    int32
	ComId    int32
	MaxIndex int
}

