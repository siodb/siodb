// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"net"
	"sync"
	"time"
)

var (
	JsonPayloadMinSize uint32 = 1024
	JsonPayloadMaxSize uint32 = 10 * 1024 * 1024
	readDeadlineMin    uint64 = 5
	readDeadlineMax    uint64 = 60
)

type TrackedNetConn struct {
	net.Conn
	RequestID uint64
}

type IOMgrConnPool struct {
	network            string
	HostName           string
	Port               uint32
	lock               sync.Mutex
	connections        chan *TrackedNetConn
	readDeadline       time.Duration
	minConnNum         int
	maxConnNum         int
	totalConnNum       int
	maxJsonPayloadSize uint32
}

func CreateIOMgrConnPool(config *SiodbConfigFile, minConn, maxConn int) (*IOMgrConnPool, error) {

	if minConn > maxConn || minConn < 0 || maxConn <= 0 {
		return nil, fmt.Errorf("Invalid number of connections for the IOMgr connection pool")
	}

	pool := &IOMgrConnPool{}
	pool.minConnNum = minConn
	pool.maxConnNum = maxConn
	pool.connections = make(chan *TrackedNetConn, maxConn)
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

func (pool *IOMgrConnPool) CloseAllConnections() {
	pool.lock.Lock()
	defer pool.lock.Unlock()
	for i := 0; i < pool.totalConnNum; i++ {
		connection := <-pool.connections
		err := connection.Close()
		if err != nil {
			siodbLoggerPool.Warning("Error closing connection (%v) from connection pool: %v",
				connection, err)
		}
	}
	siodbLoggerPool.Info("IOMgr connection pool stopped.")
}

func (pool *IOMgrConnPool) createConn() (*TrackedNetConn, error) {
	pool.lock.Lock()
	defer pool.lock.Unlock()
	if pool.totalConnNum >= pool.maxConnNum {
		return nil, fmt.Errorf("Too many connections, limit is %d", pool.maxConnNum)
	}
	conn, err := net.Dial(pool.network, pool.HostName+":"+fmt.Sprintf("%v", pool.Port))
	if err != nil {
		return nil, fmt.Errorf("Can't create connection: %s", err)
	}
	pool.totalConnNum = pool.totalConnNum + 1
	trackedNetConn := &TrackedNetConn{}
	trackedNetConn.Conn = conn
	trackedNetConn.RequestID = uint64(1)
	return trackedNetConn, nil
}

func (pool *IOMgrConnPool) GetTrackedNetConn() (*IOMgrConnection, error) {

	go func() {
		conn, err := pool.createConn()
		if err != nil {
			return
		}
		pool.connections <- conn
	}()
	select {
	case conn := <-pool.connections:
		return pool.packConn(conn), nil
	}
}

func (pool *IOMgrConnPool) ReturnTrackedNetConn(IOMgrConn *IOMgrConnection) error {

	siodbLoggerPool.Debug("ReturnTrackedNetConn trackedNetConn : %v", IOMgrConn)

	if IOMgrConn.TrackedNetConn.Conn == nil {
		siodbLoggerPool.Debug("Removing closed connection from pool: %v", IOMgrConn)
		pool.lock.Lock()
		pool.totalConnNum = pool.totalConnNum - 1
		pool.lock.Unlock()
		return fmt.Errorf("cannot put nil to connection pool")
	}

	select {
	case pool.connections <- IOMgrConn.TrackedNetConn:
		return nil
	default:
		return IOMgrConn.Close()
	}
}

func (pool *IOMgrConnPool) packConn(trackedNetConn *TrackedNetConn) *IOMgrConnection {
	ret := &IOMgrConnection{pool: pool}
	ret.TrackedNetConn = trackedNetConn
	return ret
}

func (pool *IOMgrConnPool) parseConfiguration(siodbConfigFile *SiodbConfigFile) (err error) {

	var value string
	pool.HostName = "localhost"
	pool.network = "tcp"

	if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv4_port"); err != nil {
		return fmt.Errorf("Invalid parameter 'iomgr.rest.ipv4_port': %v", err)
	}
	if pool.Port, err = StringToPortNumber(value); err != nil {
		return fmt.Errorf("Invalid parameter 'iomgr.rest.ipv4_port': %v", err)
	}

	if pool.Port == 0 {

		if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv6_port"); err != nil {
			return fmt.Errorf("Invalid parameter 'iomgr.rest.ipv6_port': %v", err)
		}
		if pool.Port, err = StringToPortNumber(value); err != nil {
			return fmt.Errorf("Invalid parameter 'iomgr.rest.ipv6_port': %v", err)
		}

		if pool.Port == 0 {
			return fmt.Errorf("Missing iomgr port")
		}
		pool.network = "tcp6"
	}

	// iomgr.max_json_payload_size
	if value, err = siodbConfigFile.GetParameterValue("iomgr.max_json_payload_size"); err != nil {
		return err
	}
	if pool.maxJsonPayloadSize, err = StringToByteSize(value); err != nil {
		return fmt.Errorf("Invalid parameter 'iomgr.max_json_payload_size': %v", err)
	}
	if pool.maxJsonPayloadSize < JsonPayloadMinSize || pool.maxJsonPayloadSize > JsonPayloadMaxSize {
		return fmt.Errorf("Invalid iomgr.max_json_payload_size=%v, expecting %v-%v",
			value, JsonPayloadMinSize, JsonPayloadMaxSize)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.iomgr_read_timeout"); err != nil {
		return err
	}

	// rest_server.iomgr_read_timeout
	var readDeadline uint64
	if readDeadline, err = StringToSeconds(value); err != nil {
		return fmt.Errorf("Invalid parameter 'rest_server.iomgr_read_timeout': %v", err)
	}
	if readDeadline < readDeadlineMin || readDeadline > readDeadlineMax {
		return fmt.Errorf("Invalid rest_server.iomgr_read_timeout=%v, expecting %v-%v",
			value, readDeadlineMin, readDeadlineMax)
	}
	pool.readDeadline = time.Duration(readDeadline) * time.Second

	return nil
}
