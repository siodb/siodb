package main

import (
	"fmt"
	"net"
	"sync"
)

var (
	JsonPayloadMinSize uint64 = 1024
	JsonPayloadMaxSize uint64 = 10 * 1024 * 1024
)

type IOMgrConnPool struct {
	network            string
	HostName           string
	Port               uint32
	lock               sync.Mutex
	connections        chan net.Conn
	minConnNum         int
	maxConnNum         int
	totalConnNum       int
	maxJsonPayloadSize uint64
}

func CreateIOMgrConnPool(config *SiodbConfigFile, minConn, maxConn int) (*IOMgrConnPool, error) {

	if minConn > maxConn || minConn < 0 || maxConn <= 0 {
		return nil, fmt.Errorf("illogical number of connection")
	}

	pool := &IOMgrConnPool{}
	pool.minConnNum = minConn
	pool.maxConnNum = maxConn
	pool.connections = make(chan net.Conn, maxConn)
	pool.totalConnNum = 0
	if err := pool.parseConfiguration(config); err != nil {
		return pool, err
	}
	if err := pool.init(); err != nil {
		return nil, err
	}
	return pool, nil

}

func (pool *IOMgrConnPool) init() error {
	for i := 0; i < pool.minConnNum; i++ {
		conn, err := pool.createConn()
		if err != nil {
			return err
		}
		pool.connections <- conn
	}
	return nil
}

func (pool *IOMgrConnPool) createConn() (net.Conn, error) {
	pool.lock.Lock()
	defer pool.lock.Unlock()
	if pool.totalConnNum >= pool.maxConnNum {
		return nil, fmt.Errorf("Connot Create new connection. Now has %d.Max is %d", pool.totalConnNum, pool.maxConnNum)
	}
	conn, err := net.Dial(pool.network, pool.HostName+":"+fmt.Sprintf("%v", pool.Port))
	if err != nil {
		return nil, fmt.Errorf("Cannot create new connection.%s", err)
	}
	pool.totalConnNum = pool.totalConnNum + 1
	return conn, nil
}

func (pool *IOMgrConnPool) GetConn() (net.Conn, error) {

	go func() {
		conn, err := pool.createConn()
		if err != nil {
			return
		}
		pool.connections <- conn
	}()
	select {
	case conn := <-pool.connections:
		return conn, nil
	}
}

func (pool *IOMgrConnPool) ReturnConn(conn net.Conn) error {

	if conn == nil {
		pool.lock.Lock()
		pool.totalConnNum = pool.totalConnNum - 1
		pool.lock.Unlock()
		return fmt.Errorf("cannot put nil to connection pool")
	}

	select {
	case pool.connections <- conn:
		return nil
	default:
		return conn.Close()
	}
}

func (pool *IOMgrConnPool) parseConfiguration(siodbConfigFile *SiodbConfigFile) (err error) {

	var value string
	pool.HostName = "localhost"
	pool.network = "tcp"

	if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv4_port"); err != nil {
		return fmt.Errorf("error for parameter 'iomgr.rest.ipv4_port': %v", err)
	}
	if pool.Port, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("error for parameter 'iomgr.rest.ipv4_port': %v", err)
	}

	if pool.Port == 0 {

		if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv6_port"); err != nil {
			return fmt.Errorf("error for parameter 'iomgr.rest.ipv6_port': %v", err)
		}
		if pool.Port, err = StringToPortNumber(value); err != nil {
			return fmt.Errorf("error for parameter 'iomgr.rest.ipv6_port': %v", err)
		}

		if pool.Port == 0 {
			return fmt.Errorf("no port enabled on iomgr for the rest server")
		}
		pool.network = "tcp6"
	}

	if value, err = siodbConfigFile.GetParameterValue("iomgr.max_json_payload_size"); err != nil {
		return err
	}
	if pool.maxJsonPayloadSize, err = StringToByteSize(value); err != nil {
		return fmt.Errorf("error for parameter 'iomgr.max_json_payload_size': %v", err)
	}
	if pool.maxJsonPayloadSize < JsonPayloadMinSize || pool.maxJsonPayloadSize > JsonPayloadMaxSize {
		return fmt.Errorf("parameter 'iomgr.max_json_payload_size' (%v) is out of range (%v-%v)",
			value, JsonPayloadMinSize, JsonPayloadMaxSize)
	}

	return nil

}
