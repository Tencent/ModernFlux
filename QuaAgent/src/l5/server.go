package l5

import (
	"fmt"
	"net"
	"os"
	"sync"
	"time"
)

type Server struct {
	sync.RWMutex
	domain     *Domain
	Ip         net.IP
	Port       uint16
	weight     int32
	total      int32
	expire     time.Time
	allocCount uint32
	succCount  uint32
	succDelay  uint64
	errCount   uint32
	errDelay   uint64
	lastReport time.Time
}

func (s Server) Network() string {
	return "tcp"
}

func (s Server) String() string {
	s.RLock()
	defer s.RUnlock()
	if s.Ip != nil {
		return fmt.Sprintf("%s:%d", s.Ip.String(), s.Port)
	} else {
		return ""
	}
}

/*
func (s Server) IpAddr() net.IP {
	ip := make(net.IP, 4)
	binary.BigEndian.PutUint32(ip, uint32(s.Ip))
	return ip
}
*/

func (s Server) needReport() bool {
	now := time.Now()
	interval := s.domain.api.StatReportInterval
	if s.errCount > 0 {
		interval = s.domain.api.StatErrorReportInterval
	}
	if s.lastReport.Add(interval).After(now) {
		return true
	}
	if s.errCount >= uint32(s.domain.api.StatMaxErrorCount) {
		return true
	}
	if float64(s.allocCount)*s.domain.api.StatMaxErrorRate <= float64(s.errCount) {
		return true
	}
	return false
}

func (s *Server) Report(args ...bool) error {
	force := false
	if len(args) > 0 {
		force = args[0]
	}
	s.RLock()
	if !force && !s.needReport() {
		s.RUnlock()
		return nil
	} else {
		s.RUnlock()
	}
	s.Lock()
	defer s.Unlock()
	if s.domain != nil {
		var err error
		_, err = s.domain.api.dial(QOS_CMD_GET_STAT, 0, s.domain.Mod, s.domain.Cmd, s.Ip, uint32(s.Port), s.allocCount, int32(os.Getpid()))
		if err != nil {
			return err
		}
		_, err = s.domain.api.dial(QOS_CMD_CALLER_UPDATE_BIT64, 0, int32(0), int32(0), s.domain.Mod, s.domain.Cmd, s.Ip, uint32(s.Port), int32(-1), s.errCount, s.errDelay, int32(os.Getpid()))
		if err != nil {
			return err
		}
		_, err = s.domain.api.dial(QOS_CMD_CALLER_UPDATE_BIT64, 0, int32(0), int32(0), s.domain.Mod, s.domain.Cmd, s.Ip, uint32(s.Port), int32(0), s.succCount, s.succDelay, int32(os.Getpid()))
		if err != nil {
			return err
		}
	}
	s.allocCount = 0
	s.succCount = 0
	s.succDelay = 0
	s.errCount = 0
	s.errDelay = 0
	s.lastReport = time.Now()
	return nil
}

//上报服务器信息
func (s *Server) StatUpdate(result int32, usetime uint64) error {
	s.Lock()
	defer s.Unlock()
	if result >= 0 {
		s.succCount++
		s.succDelay += usetime
	} else {
		s.errCount++
		s.errDelay += usetime
	}
	return nil
}

func (s *Server) allocate() *Server {
	s.allocCount++
	return s
}
