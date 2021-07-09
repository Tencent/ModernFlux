package main

import (
	"flag"
	"fmt"
	"log"
	"net"
	"os"

	"strings"

	"encoding/json"
	"expvar"

	"net/http"
	_ "net/http/pprof"

	"runtime"

	"runtime/trace"
	//	"sync"
	"sync/atomic"
	"time"

	"runtime/pprof"

	_ "github.com/go-sql-driver/mysql"
)

var (
	server = "0.0.0.0:8080"
)

type TcpServer struct {
	listener *net.TCPListener
	tcpSvr   *net.TCPAddr
}

//手动GC
func gc(w http.ResponseWriter, r *http.Request) {
	runtime.GC()
	w.Write([]byte("StartGC"))
}

//运行trace
func traces(w http.ResponseWriter, r *http.Request) {
	f, err := os.Create("trace.out")
	if err != nil {
		panic(err)
	}

	err = trace.Start(f)
	if err != nil {
		panic(err)
	}
	w.Write([]byte("TrancStart"))
	fmt.Println("StartTrancs")
}

//停止trace
func traceStop(w http.ResponseWriter, r *http.Request) {
	trace.Stop()
	w.Write([]byte("TrancStop"))
	fmt.Println("StopTrancs")
}

var visits = expvar.NewInt("visits")

func handler(w http.ResponseWriter, r *http.Request) {
	visits.Add(1)
	fmt.Fprintf(w, "Hi there, I love %s!", r.URL.Path[1:])
}

var start = time.Now()

func sysUptime() interface{} {
	return time.Now()
}

func calculateUptime() interface{} {
	return time.Since(start).String()
}

func currentGoVersion() interface{} {
	return runtime.Version()
}

func getNumCPUs() interface{} {
	return runtime.NumCPU()
}

func getGoOS() interface{} {
	return runtime.GOOS
}

func getLastGCPauseTime() interface{} {
	//var gcPause uint64
	ms := new(runtime.MemStats)

	statString := expvar.Get("memstats").String()
	if statString != "" {
		json.Unmarshal([]byte(statString), ms)
		return ms.PauseNs
	}

	return statString
}

var path = "/data/app/golang/qagent/bin"
var owner = "cherieyuan;scottzhu"
var ginfo []byte

var socknumber int64 = 0

func ShowMap(msgmap map[string]string) {
	for key, value := range msgmap {
		fmt.Println("key:", key, "value:", value)
	}
}

func ParseReq(recvByte []byte, msgmap map[string]string) {
	firstslice := strings.Split(string(recvByte[:len(recvByte)-1]), "&")
	for i := 0; i < len(firstslice); i++ {
		secondslice := strings.Split(firstslice[i], "=")
		if len(secondslice) == 2 {
			msgmap[secondslice[0]] = secondslice[1]
		}
	}
}

func checkErr(err error) {
	if err != nil {
		fmt.Println(err)
		//		os.Exit(-1)
	}
}

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")
var port = flag.String("port", "8094", "port")

func main() {
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	LocalIp := GetEth1IP()

	go func() {
		log.Println("pprof")
		expvar.Publish("运行时间", expvar.Func(calculateUptime))
		expvar.Publish("uptime", expvar.Func(sysUptime))
		expvar.Publish("version", expvar.Func(currentGoVersion))
		expvar.Publish("cores", expvar.Func(getNumCPUs))
		expvar.Publish("os", expvar.Func(getGoOS))
		http.HandleFunc("/", handler)
		log.Println(http.ListenAndServe(LocalIp+":8099", nil))
	}()
	runtime.GOMAXPROCS(runtime.NumCPU())
	go UpdateQos()

	log.Println("port:", *port)

	tcpSvr, err := net.ResolveTCPAddr("tcp", LocalIp+":"+*port)
	checkErr(err)
	if err != nil {
		os.Exit(-1)
	}

	listen, err := net.ListenTCP("tcp", tcpSvr)
	checkErr(err)
	if err != nil {
		os.Exit(-1)
	}
	defer listen.Close()
	tcpServer := &TcpServer{
		listener: listen,
		tcpSvr:   tcpSvr,
	}
	fmt.Println("start server successful......")

	masters = make([]chan *ReqData, workernum)
	workers = make([]chan *ReqData, workernum)
	quotas = make([]chan *RspData, workernum)
	nets = make([]chan []byte, workernum)
	workermaps = make([]map[string]*QUOTA, workernum)
	quotamaps = make([]map[string]*LOCALDATA, workernum)
	for i := 0; i < workernum; i++ {
		masters[i] = make(chan *ReqData, 1024)
		workers[i] = make(chan *ReqData, 1024)
		quotas[i] = make(chan *RspData, 1024)
		nets[i] = make(chan []byte, 1024)
		workermaps[i] = make(map[string]*QUOTA)
		quotamaps[i] = make(map[string]*LOCALDATA)
	}

	for i := 0; i < workernum; i++ {
		go workerstart(i)
	}

	for i := 0; i < workernum; i++ {
		go quotastart(i)
	}

	for {
		conn, err := tcpServer.listener.Accept()
		atomic.AddInt64(&socknumber, 1)
		if err != nil {
			fmt.Println(err)
		} else {
			go Handle(conn)

		}
	}
}
func GetEth1IP() string {
	ip := ""
	const ETH string = "eth1"
	ifi, err := net.InterfaceByName(ETH)
	if err != nil {
		log.Println(err)
	}
	addrs, err := ifi.Addrs()
	if err != nil {
		log.Println(err)
	}

	for _, a := range addrs {
		//fmt.Println(ifi.Name, a.String())
		if ifi.Name == ETH {
			ip = strings.Split(a.String(), "/")[0]
			break
		}
	}
	return ip
}
