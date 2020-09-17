package main

import (
	"errors"
	"fmt"
	"log"
	"net"
	"strconv"
	"sync"
	"testing"
)

type IOMgrConnPool struct {
	network      string
	HostName     string
	Port         uint64
	lock         sync.Mutex
	connections  chan net.Conn
	minConnNum   int
	maxConnNum   int
	totalConnNum int
}

func CreateIOMgrConnPool(minConn, maxConn int, config Config) (*IOMgrConnPool, error) {

	if minConn > maxConn || minConn < 0 || maxConn <= 0 {
		return nil, errors.New("illogical number of connection")
	}

	pool := &IOMgrConnPool{}
	pool.minConnNum = minConn
	pool.maxConnNum = maxConn
	pool.connections = make(chan net.Conn, maxConn)
	pool.totalConnNum = 0
	pool.parseConfiguration(config)
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
		return errors.New("Cannot put nil to connection pool.")
	}

	select {
	case pool.connections <- conn:
		return nil
	default:
		return conn.Close()
	}
}

func (pool *IOMgrConnPool) parseConfiguration(config Config) (err error) {

	pool.network = "tcp"
	if pool.Port, err = strconv.ParseUint(config["iomgr.rest.ipv4_port"], 10, 64); err != nil {
		return err
	}
	if pool.Port == 0 {
		if pool.Port, err = strconv.ParseUint(config["iomgr.rest.ipv6_port"], 10, 64); err != nil {
			return err
		}
		if pool.Port == 0 {
			return errors.New("no port enabled on iomgr for the rest server")
		} else {
			pool.network = "tcp6"
		}
	}

	pool.HostName = "localhost"

	return nil

}
func TestIOMgrConnPool(*testing.T) {
	pool, err := CreateIOMgrConnPool(0, 3, Config{"iomgr.rest.ipv4_port": "8080", "iomgr.rest.ipv6_port": "0"})
	if err != nil {
		log.Fatalf("Cannot create connection pool: %v", err)

		siodbLoggerPool.Output(FATAL, "Cannot create connection pool: %v", err)
	}
	conn1, _ := pool.GetConn()
	log.Printf("%v", conn1)
	conn2, _ := pool.GetConn()
	log.Printf("%v", conn2)
	conn3, _ := pool.GetConn()
	log.Printf("%v", conn3)
	go func() { err = pool.ReturnConn(conn2) }()
	err = pool.ReturnConn(conn1)
	log.Printf("%v", err)
	conn1, _ = pool.GetConn()
	log.Printf("%v", conn1)
	conn4, _ := pool.GetConn()
	log.Printf("%v", conn4)
}
