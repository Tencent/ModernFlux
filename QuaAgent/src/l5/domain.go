package l5

import (
	"fmt"
	"net"
	"os"
	"sync"
	"time"
)

type Domain struct {
	sync.Mutex
	balancer Balancer
	api      *Api
	Name     string
	Mod      int32
	Cmd      int32
	expire   time.Time
}

func (d *Domain) Get() (*Server, error) {
	d.Lock()
	if d.balancer == nil {
		d.Unlock()
		return nil, ErrNotBalancer
	}
	now := time.Now()
	//step1：从balancer中获取
	srv, err := d.balancer.Get()
	d.Unlock()
	if err == nil {
		if srv.expire.After(now) {
			d.Lock()
			defer d.Unlock()
			return srv.allocate(), nil
		} else {
			d.Lock()
			if d.balancer != nil {
				d.balancer.Remove(srv)
			}
			d.Unlock()
		}
	} else if err != ErrNotFound {
		return nil, err
	}
	//step2：从agent中获取
	var buf []byte
	buf, err = d.api.dial(QOS_CMD_BATCH_GET_ROUTE_WEIGHT, 0, d.Mod, d.Cmd, int32(os.Getpid()), int32(Version))
	if err != nil {
		return nil, err
	}
	size := len(buf) - 16
	list := make([]*Server, size/14)
	for k, _ := range list {
		list[k] = &Server{
			//Domain: d,
			Ip:     net.IPv4(buf[16+k*14], buf[17+k*14], buf[18+k*14], buf[19+k*14]),
			Port:   d.api.Endian.Uint16(buf[20+k*14 : 22+k*14]),
			weight: int32(d.api.Endian.Uint32(buf[22+k*14 : 26+k*14])),
			total:  int32(d.api.Endian.Uint32(buf[26+k*14 : 30+k*14])),
			expire: now.Add(d.api.ServerExpire),
		}
	}
	if len(list) < 1 {
		return nil, ErrNotFound
	}
	err = d.Set(list)
	if err == nil {
		return d.Get()
	}
	//step3：从静态文件中获取
	var fp *os.File
	list = make([]*Server, 0)
	for _, v := range d.api.StaticServerFiles {
		if fp, err = os.Open(v); err != nil {
			continue
		}
		for {
			var (
				mod  int32
				cmd  int32
				ip   string
				port uint16
			)
			if n, fail := fmt.Fscanln(fp, &mod, &cmd, &ip, &port); n == 0 || fail != nil {
				break
			}
			if d.Mod != mod || d.Cmd != cmd {
				continue
			}
			list = append(list, &Server{
				//Domain: d,
				Ip:     net.ParseIP(ip),
				Port:   port,
				weight: 100, //default weight: 100
				total:  0,
				expire: now.Add(d.api.ServerExpire),
			})
		}
		fp.Close()
	}
	if len(list) < 1 {
		return nil, ErrNotFound
	}
	err = d.Set(list)
	if err == nil {
		return d.Get()
	}
	return nil, ErrNotFound
}

func (d *Domain) Set(list []*Server) error {
	d.Lock()
	defer d.Unlock()
	if d.balancer == nil {
		return ErrNotBalancer
	}
	if err := d.balancer.Destory(); err != nil {
		return err
	}
	for _, v := range list {
		if err := d.balancer.Set(v); err != nil {
			return err
		}
	}
	return nil
}

func (d *Domain) Destory() error {
	d.Lock()
	defer d.Unlock()
	if d.balancer == nil {
		return ErrNotBalancer
	}
	return d.balancer.Destory()
}

func (d *Domain) SetBalancer(b Balancer) {
	d.Lock()
	defer d.Unlock()
	d.balancer = b
}

type Domains map[string]*Domain

type SidCache map[string]*Domain

//根据l5名称查询l5 mod+cmd
func (c *Api) Query(name string) (*Domain, error) {
	now := time.Now()
	c.RLock()
	domain, exists := c.Domains[name]
	c.RUnlock()
	if exists && domain.expire.After(now) {
		return domain, nil
	} else {
		domain = &Domain{
			api:      c,
			Name:     name,
			Mod:      0,
			Cmd:      0,
			expire:   now.Add(c.DomainExpire),
			balancer: NewBalancer(c.Balancer),
		}
	}
	buf, err := c.dial(QOS_CMD_QUERY_SNAME, 0, domain.Mod, domain.Cmd,
		int32(os.Getpid()), int32(len(domain.Name)), domain.Name)
	if err != nil {
		return nil, err
	}
	domain.Mod = int32(c.Endian.Uint32(buf[0:4]))
	domain.Cmd = int32(c.Endian.Uint32(buf[4:8]))
	c.Lock()
	c.Domains[name] = domain
	c.Unlock()
	return domain, nil
}

//定时load静态domain文件
func (c *Api) interval() {
	interval := time.NewTicker(c.StaticDomainReload)
	for {
		select {
		case <-interval.C:
			var (
				err error
				fp  *os.File
			)
			for _, v := range c.StaticDomainFiles {
				if fp, err = os.Open(v); err != nil {
					//log.Printf("open file failed: %s", err.Error())
					continue
				}
				for {
					var (
						name string
						mod  int32
						cmd  int32
					)
					if n, fail := fmt.Fscanln(fp, &name, &mod, &cmd); n == 0 || fail != nil {
						break
					}
					now := time.Now()
					c.Lock()
					_, exists := c.Domains[name]
					if !exists {
						c.Domains[name] = &Domain{}
					}
					if c.Domains[name].expire.Before(now) {
						c.Domains[name].Lock()
						c.Domains[name].Cmd = cmd
						c.Domains[name].Mod = mod
						c.Domains[name].expire = now.Add(c.DomainExpire)
						c.Domains[name].Unlock()
					}
					c.Unlock()
				}
				fp.Close()
			}
		}
	}
}
