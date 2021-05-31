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
	jsonPayloadMinSize uint32 = 1024
	jsonPayloadMaxSize uint32 = 10 * 1024 * 1024
	readDeadlineMin    uint64 = 5
	readDeadlineMax    uint64 = 60
)

type trackedNetConn struct {
	net.Conn
	RequestID uint64
}

type ioMgrConnPool struct {
	network            string
	HostName           string
	Port               uint32
	lock               sync.Mutex
	connections        chan *trackedNetConn
	readDeadline       time.Duration
	minConnNum         int
	maxConnNum         int
	totalConnNum       int
	maxJSONPayloadSize uint32
}

func createIOMgrConnPool(config *siodbConfigFile, minConn, maxConn int) (*ioMgrConnPool, error) {

	if minConn > maxConn || minConn < 0 || maxConn <= 0 {
		return nil, fmt.Errorf("invalid number of connections for the IOMgr connection pool")
	}

	pool := &ioMgrConnPool{}
	pool.minConnNum = minConn
	pool.maxConnNum = maxConn
	pool.connections = make(chan *trackedNetConn, maxConn)
	pool.totalConnNum = 0
	if err := pool.parseConfiguration(config); err != nil {
		return pool, err
	}
	if err := pool.init(); err != nil {
		return nil, err
	}
	return pool, nil
}

func (pool *ioMgrConnPool) init() error {
	for i := 0; i < pool.minConnNum; i++ {
		conn, err := pool.createConn()
		if err != nil {
			return err
		}
		pool.connections <- conn
	}
	return nil
}

func (pool *ioMgrConnPool) CloseAllConnections() {
	pool.lock.Lock()
	defer pool.lock.Unlock()
	for i := 0; i < pool.totalConnNum; i++ {
		connection := <-pool.connections
		err := connection.Close()
		if err != nil {
			log.Warning("Error closing connection (%v) from connection pool: %v",
				connection, err)
		}
	}
	log.Info("IOMgr connection pool stopped.")
}

func (pool *ioMgrConnPool) createConn() (*trackedNetConn, error) {
	pool.lock.Lock()
	defer pool.lock.Unlock()
	if pool.totalConnNum >= pool.maxConnNum {
		return nil, fmt.Errorf("too many connections, limit is %d", pool.maxConnNum)
	}
	conn, err := net.Dial(pool.network, pool.HostName+":"+fmt.Sprintf("%v", pool.Port))
	if err != nil {
		return nil, fmt.Errorf("can't create connection: %s", err)
	}
	pool.totalConnNum = pool.totalConnNum + 1
	trackedNetConn := &trackedNetConn{}
	trackedNetConn.Conn = conn
	trackedNetConn.RequestID = uint64(1)
	return trackedNetConn, nil
}

func (pool *ioMgrConnPool) GetTrackedNetConn() (*ioMgrConnection, error) {
	go func() {
		conn, err := pool.createConn()
		if err != nil {
			return
		}
		pool.connections <- conn
	}()
	conn := <-pool.connections
	return pool.packConn(conn), nil
}

func (pool *ioMgrConnPool) ReturnTrackedNetConn(ioMgrConn *ioMgrConnection) error {
	log.Debug("ReturnTrackedNetConn trackedNetConn : %v", ioMgrConn)

	if ioMgrConn.trackedNetConn.Conn == nil {
		log.Debug("Removing closed connection from pool: %v", ioMgrConn)
		pool.lock.Lock()
		pool.totalConnNum = pool.totalConnNum - 1
		pool.lock.Unlock()
		return fmt.Errorf("can't put nil to connection pool")
	}

	select {
	case pool.connections <- ioMgrConn.trackedNetConn:
		return nil
	default:
		return ioMgrConn.Close()
	}
}

func (pool *ioMgrConnPool) packConn(trackedNetConn *trackedNetConn) *ioMgrConnection {
	ret := &ioMgrConnection{pool: pool}
	ret.trackedNetConn = trackedNetConn
	return ret
}

func (pool *ioMgrConnPool) parseConfiguration(siodbConfigFile *siodbConfigFile) (err error) {
	var value string
	pool.HostName = "localhost"
	pool.network = "tcp"

	if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv4_port"); err != nil {
		return fmt.Errorf("invalid parameter 'iomgr.rest.ipv4_port': %v", err)
	}
	if pool.Port, err = stringToPortNumber(value); err != nil {
		return fmt.Errorf("invalid parameter 'iomgr.rest.ipv4_port': %v", err)
	}

	if pool.Port == 0 {
		if value, err = siodbConfigFile.GetParameterValue("iomgr.rest.ipv6_port"); err != nil {
			return fmt.Errorf("invalid parameter 'iomgr.rest.ipv6_port': %v", err)
		}
		if pool.Port, err = stringToPortNumber(value); err != nil {
			return fmt.Errorf("invalid parameter 'iomgr.rest.ipv6_port': %v", err)
		}

		if pool.Port == 0 {
			return fmt.Errorf("missing iomgr port")
		}
		pool.network = "tcp6"
	}

	// iomgr.max_json_payload_size
	if value, err = siodbConfigFile.GetParameterValue("iomgr.max_json_payload_size"); err != nil {
		return err
	}
	if pool.maxJSONPayloadSize, err = stringToByteSize(value); err != nil {
		return fmt.Errorf("invalid parameter 'iomgr.max_json_payload_size': %v", err)
	}
	if pool.maxJSONPayloadSize < jsonPayloadMinSize || pool.maxJSONPayloadSize > jsonPayloadMaxSize {
		return fmt.Errorf("invalid iomgr.max_json_payload_size=%v, expecting %v-%v",
			value, jsonPayloadMinSize, jsonPayloadMaxSize)
	}

	if value, err = siodbConfigFile.GetParameterValue("rest_server.iomgr_read_timeout"); err != nil {
		return err
	}

	// rest_server.iomgr_read_timeout
	var readDeadline uint64
	if readDeadline, err = stringToSeconds(value); err != nil {
		return fmt.Errorf("invalid parameter 'rest_server.iomgr_read_timeout': %v", err)
	}
	if readDeadline < readDeadlineMin || readDeadline > readDeadlineMax {
		return fmt.Errorf("invalid rest_server.iomgr_read_timeout=%v, expecting %v-%v",
			value, readDeadlineMin, readDeadlineMax)
	}
	pool.readDeadline = time.Duration(readDeadline) * time.Second

	return nil
}
