package main

import (
	"errors"
	"fmt"
	"io"

	"net"
	"strconv"
	"time"
)

var (
	ErrInvalidConfig = errors.New("invalid pool config")
	ErrPoolClosed    = errors.New("pool closed")
)

type factory func() (net.Conn, error)

type Pool interface {
	Acquire() (net.Conn, error) // 获取资源
	Release(net.Conn) error     // 释放资源
	Close(net.Conn) error       // 关闭资源
	Shutdown() error            // 关闭池
}
type GenericPool struct {
	pool    chan net.Conn
	closed  bool    // 池是否已关闭
	factory factory // 创建连接的方法
	maxOpen int
}

var AFLConnPool *GenericPool

func NewConnPool(modId int32, comId int32, maxOpen int) *GenericPool { //

	var f factory = func() (net.Conn, error) {
		Server, ServerErr := L5Api.GetServerBySid(modId, comId)
		if ServerErr != nil {
			Server, ServerErr = L5Api.GetServerBySid(modId, comId)

			if ServerErr != nil {
				return nil, ServerErr
			}
		}
		ip := Server.Ip.String()
		port := strconv.Itoa(int(Server.Port))

		conn, err := net.DialTimeout("tcp", ip+":"+port, time.Second*3)
		if err != nil {
			fmt.Printf(" dial error: %s\n", err)
			return nil, err
		}
		conn.SetReadDeadline(time.Now().Add(30 * time.Second))
		return conn, nil
	}

	if maxOpen < 0 {
		maxOpen = 100
	}
	gPool, err := NewGenericPool(maxOpen, f)
	if err != nil {
		return nil
	}
	go gPool.Ping()
	return gPool

}
func (connPool *GenericPool) Ping() {
	var ticker *time.Ticker = time.NewTicker(20 * time.Second)

	for range ticker.C {
		flag := false
		for i := len(connPool.pool); i > 0; i-- {
			select {
			case conn := <-connPool.pool:
				buf := make([]byte, 10)
				_, err := conn.Read(buf)

				if err == io.EOF {

					conn.Close()
				} else {
					conn.SetReadDeadline(time.Now().Add(30 * time.Second))
					select {
					case connPool.pool <- conn:
					default:
					}

				}
			default:
				flag = true

			}

			if flag {
				break
			}

		}
	}

}
func (connPool *GenericPool) TCPSend(data []byte, resChan chan []byte) int {
	if !CheckQosFromCh() {
		return -2
	}

	conn, err := connPool.Acquire()
	if err != nil {
		return -3
	}
	dataRsp, length := TCPDataSend(conn, data)
	if length < 0 {
		conn.Close()
		conn, err = connPool.factory()
		if err != nil {
			UpdateQosToCh(-1, 0) //失败
			return -4
		}

		dataRsp, length = TCPDataSend(conn, data)
		if length < 0 {
			connPool.Close(conn)
			UpdateQosToCh(-1, 0) //失败

			return -4
		}
	}
	conn.SetReadDeadline(time.Now().Add(30 * time.Second))
	connPool.Release(conn)
	UpdateQosToCh(0, 0) //成功

	select {
	case resChan <- dataRsp:
	default:
	}
	return 0
}

func TCPDataSend(conn net.Conn, data []byte) ([]byte, int) {
	_, e := conn.Write(data)
	if e != nil {
		return nil, -2

	}
	dataRsp, length := ReadBinary(conn)

	if length < 0 {
		return nil, length
	}
	return dataRsp, length
}

func NewGenericPool(maxOpen int, factory factory) (*GenericPool, error) {
	if maxOpen <= 0 {
		return nil, ErrInvalidConfig
	}
	p := &GenericPool{
		maxOpen: maxOpen,
		factory: factory,
		pool:    make(chan net.Conn, maxOpen),
	}

	for i := 0; i < maxOpen; i++ {
		closer, err := factory()
		if err != nil {
			continue
		}
		p.pool <- closer
	}
	return p, nil
}

func (p *GenericPool) Acquire() (net.Conn, error) {
	if p.closed {
		return nil, ErrPoolClosed
	}
	for {
		closer, err := p.getOrCreate()
		if err != nil {
			return nil, err
		}
		// todo maxLifttime处理
		return closer, nil
	}
}

func (p *GenericPool) getOrCreate() (net.Conn, error) {
	var closer net.Conn
	select {
	case closer = <-p.pool:
	default:
		closerNew, err := p.factory()
		if err != nil {

			return nil, err
		}
		return closerNew, nil
	}

	return closer, nil

}

// 释放单个资源到连接池
func (p *GenericPool) Release(closer net.Conn) error {
	if closer == nil {
		return nil
	}
	if p.closed {

		closer.Close()

		return ErrPoolClosed
	}
	select {
	case p.pool <- closer:
	default:
		closer.Close()
	}

	return nil
}

// 关闭单个资源
func (p *GenericPool) Close(closer net.Conn) (net.Conn, error) {
	if closer != nil {
		closer.Close()
	}

	// 新建连接
	closerNew, err := p.factory()
	if err != nil {

		return nil, err
	}
	select {
	case p.pool <- closerNew:
	default:
		closerNew.Close()
	}

	return closerNew, nil
}

// 关闭连接池，释放所有资源
func (p *GenericPool) Shutdown() error {
	if p.closed {
		return ErrPoolClosed
	}
	for closer := range p.pool {
		if closer != nil {
			closer.Close()
		}
	}
	p.closed = true

	return nil
}
