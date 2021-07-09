package l5

import (
	"encoding/binary"
	"fmt"
	"net"
	"sync"
	"time"
)

type Option struct {
	Endian                  binary.ByteOrder
	Host                    string
	Port                    int16
	Timeout                 time.Duration
	MaxConn                 int
	MaxPacketSize           int
	StaticDomainFiles       []string
	DomainExpire            time.Duration
	StaticDomainReload      time.Duration
	StaticServerFiles       []string
	ServerExpire            time.Duration
	StatErrorReportInterval time.Duration
	StatReportInterval      time.Duration
	StatMaxErrorCount       int
	StatMaxErrorRate        float64
	Balancer                int
}

type Api struct {
	sync.RWMutex
	*Option
	Domains
	SidCache
	conn chan net.Conn
}

func NewApi(opt *Option) (*Api, error) {
	api := &Api{
		RWMutex:  sync.RWMutex{},
		Option:   opt,
		Domains:  make(Domains),
		SidCache: make(SidCache),
		conn:     make(chan net.Conn, opt.MaxConn),
	}
	for i := 0; i < opt.MaxConn; i++ {
		conn, err := net.DialTimeout("udp", fmt.Sprintf("%s:%d", opt.Host, opt.Port), opt.Timeout)
		if err != nil {
			return nil, err
		}
		api.conn <- conn
	}
	go api.interval()
	return api, nil
}

func NewDefaultApi() (*Api, error) {
	return NewApi(&Option{
		Endian:                  binary.LittleEndian,                                                                       
		Host:                    "127.0.0.1",                                                                              
		Port:                    8888,                                                                                      
		Timeout:                 time.Second,                                                                               
		MaxConn:                 8,                                                                                         
		MaxPacketSize:           1024,                                                                                      
		StaticDomainFiles:       []string{"/data/L5Backup/name2sid.backup", "/data/L5Backup/name2sid.cache.bin"},          
		DomainExpire:            30 * time.Second,                                                                         
		StaticDomainReload:      30 * time.Second,                                                                          
		StaticServerFiles:       []string{"/data/L5Backup/current_route.backup", "/data/L5Backup/current_route_v2.backup"}, 
		ServerExpire:            30 * time.Second,                                                                          
		StatErrorReportInterval: time.Second,                                                                               
		StatReportInterval:      5 * time.Second,                                                                           
		StatMaxErrorCount:       16,                                                                                        
		StatMaxErrorRate:        0.2,                                                                                       
		Balancer:                DefaultBalancer,                                                                           
	})
}

//通过l5名称获取服务器信息
func (c *Api) GetServerByName(name string) (*Server, error) {
	domain, err := c.Query(name)
	if err != nil {
		return nil, err
	}
	return domain.Get()
}

//通过sid获取服务器信息
func (c *Api) GetServerBySid(mod int32, cmd int32) (*Server, error) {
	sid := fmt.Sprintf("%d_%d", mod, cmd)
	c.RLock()
	if domain, exists := c.SidCache[sid]; exists {
		c.RUnlock()
		return domain.Get()
	}
	c.RUnlock()
	c.Lock()
	defer c.Unlock()
	c.SidCache[sid] = &Domain{
		api: c,
		Mod: mod, 
		Cmd: cmd,
		expire: time.Now().Add(c.DomainExpire),
		balancer: NewBalancer(c.Balancer)
	}
	return c.SidCache[sid].Get()
}
