// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"siodbproto"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang/protobuf/proto"
)

var (
	ioMgrChunkMaxSize    uint32 = (2 * 1024 * 1024 * 1024) - 1
	messageLengthMaxSize uint32 = 1 * 1024 * 1024
)

var (
	messageTypeDatabaseEngineReposnse uint32 = 4
	messageTypeDatabaseEngineRequest  uint32 = 13
)

type ioMgrConnection struct {
	*trackedNetConn
	pool *ioMgrConnPool
}

func (ioMgrConn *ioMgrConnection) writeJSONPayload(requestID uint64, c *gin.Context) (err error) {
	// Read Payload up to max json payload size
	var bytesRead int = 0
	var bytesReadTotal uint64 = 0
	var bytesWritten uint64 = 0
	var bytesWrittenTotal uint64 = 0
	buffer := make([]byte, config.requestPayloadBufferSize)

	log.Debug("writeJSONPayload | ioMgrConn.pool.maxJSONPayloadSize: %v, restServerConfig.RequestPayloadBufferSize: %v",
		ioMgrConn.pool.maxJSONPayloadSize, config.requestPayloadBufferSize)

	for {
		if bytesReadTotal > uint64(ioMgrConn.pool.maxJSONPayloadSize) {
			ioMgrConn.Close()
			ioMgrConn.trackedNetConn.Conn = nil
			return fmt.Errorf("JSON payload is too large:  received %v bytes, but expecting at most %v bytes",
				bytesReadTotal, ioMgrConn.pool.maxJSONPayloadSize)
		}
		bytesRead, err = io.ReadFull(c.Request.Body, buffer)
		if err == io.ErrUnexpectedEOF {
			bytesWritten, err = ioMgrConn.writeJSONPayloadChunk(buffer, bytesRead)
			if err != nil {
				return err
			}
			bytesReadTotal += uint64(bytesRead)
			bytesWrittenTotal += bytesWritten
			log.Debug("writeJSONPayload | bytes Read: %v (total: %v), bytes Written: %v (total: %v)",
				bytesRead, bytesReadTotal, bytesWritten, bytesWrittenTotal)
			break
		} else if err != nil {
			return err
		}
		bytesWritten, err = ioMgrConn.writeJSONPayloadChunk(buffer, bytesRead)
		if err != nil {
			return err
		}
		bytesReadTotal += uint64(bytesRead)
		bytesWrittenTotal += bytesWritten
		log.Debug("writeJSONPayload | bytes Read: %v (total: %v), bytes Written: %v (total: %v)",
			bytesRead, bytesReadTotal, bytesWritten, bytesWrittenTotal)
	}
	_, err = ioMgrConn.writeJSONPayloadChunk(buffer, 0)
	if err != nil {
		return err
	}

	return nil
}

func (ioMgrConn *ioMgrConnection) writeJSONPayloadChunk(
	JSONPayloadBuffer []byte, size int) (uint64, error) {

	var bytesWritten int = 0
	// Write Payload length
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(size))
	if _, err := ioMgrConn.Write(buf[:encodedLength]); err != nil {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return 0, err
	}
	log.Debug("writeJSONPayloadChunk | Payload size written to IOMgr: %v",
		uint64(size))

	if size > 0 {
		// Write raw payload
		if bytesWritten, err := ioMgrConn.Write(JSONPayloadBuffer[:size]); err != nil {
			ioMgrConn.Close()
			ioMgrConn.trackedNetConn.Conn = nil
			return uint64(bytesWritten), fmt.Errorf("Can't write payload to IOMgr: %v", err)
		}
	}

	return uint64(bytesWritten), nil
}

func (ioMgrConn *ioMgrConnection) writeIOMgrRequest(
	restVerb siodbproto.RestVerb,
	objectType siodbproto.DatabaseObjectType,
	userName string,
	token string,
	objectName string,
	objectID uint64) (requestID uint64, err error) {

	requestID = ioMgrConn.RequestID
	ioMgrConn.RequestID++
	var databaseEngineRestRequest siodbproto.DatabaseEngineRestRequest
	databaseEngineRestRequest.RequestId = requestID
	databaseEngineRestRequest.Verb = restVerb
	databaseEngineRestRequest.ObjectType = objectType
	databaseEngineRestRequest.UserName = userName
	databaseEngineRestRequest.Token = token

	if len(objectName) > 0 {
		databaseEngineRestRequest.ObjectName = objectName
	}
	if objectID > 0 {
		databaseEngineRestRequest.ObjectId = objectID
	}

	// WARNING: Do not log databaseEngineRestRequest here, because it contains secret token,
	// which should never be logged
	log.Debug("writeIOMgrRequest | ioMgrConn.RequestID: %v", requestID)

	if _, err := ioMgrConn.writeMessage(messageTypeDatabaseEngineRequest, &databaseEngineRestRequest); err != nil {
		return requestID, fmt.Errorf("Can't write message to IOMgr: %v", err)
	}

	return requestID, nil
}

func (ioMgrConn *ioMgrConnection) readIOMgrResponse(requestID uint64) (err error) {
	var databaseEngineResponse siodbproto.DatabaseEngineResponse

	if _, err := ioMgrConn.readMessage(messageTypeDatabaseEngineReposnse, &databaseEngineResponse); err != nil {
		return fmt.Errorf("Can't read response from IOMgr: %v", err)
	}
	log.Debug("readIOMgrResponse | databaseEngineResponse: %v", &databaseEngineResponse)
	log.Debug("readIOMgrResponse | databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	log.Debug("readIOMgrResponse | ioMgrConn.RequestID: %v", ioMgrConn.RequestID)

	if databaseEngineResponse.RequestId != requestID {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return fmt.Errorf("Request ID mismatch: databaseEngineResponse.RequestId (%v) != requestID (%v)",
			databaseEngineResponse.RequestId, requestID)
	}

	if len(databaseEngineResponse.Message) > 0 {
		return fmt.Errorf("code: %v, message: %v",
			databaseEngineResponse.Message[0].GetStatusCode(), databaseEngineResponse.Message[0].GetText())
	}

	return nil
}

func (ioMgrConn *ioMgrConnection) readChunkedJSON(c *gin.Context) (err error) {
	var ioMgrReceivedChunkSize uint32 = 0
	var position uint32 = 0
	var readBytes uint64 = 0
	var writtenBytes uint64 = 0
	var readBytesTotal uint64 = 0
	var writtenBytesTotal uint64 = 0
	httpChunkBuffer := make([]byte, uint32(config.httpChunkSize))

	c.Writer.Header().Add("Content-Type", "application/json")

	for {

		readBytesTotalPerChunk := uint64(0)
		writtenBytesTotalPerChunk := uint64(0)

		// Get Current IOMgr chunk Size
		if _, ioMgrReceivedChunkSize, err = ioMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("Can't read chunk size from IOMgr: %v", err)
		}
		log.Debug("readChunkedJSON | New chunk with size: %v", ioMgrReceivedChunkSize)

		if ioMgrReceivedChunkSize == 0 {
			break
		}

		if ioMgrReceivedChunkSize > ioMgrChunkMaxSize {
			ioMgrConn.Close()
			ioMgrConn.trackedNetConn.Conn = nil
			return fmt.Errorf("readChunkedJSON | Protocol error: invalid IOMgr chunk size")
		}

		for ioMgrReceivedChunkSize > 0 {
			maxReadSize := minUint32(uint32(len(httpChunkBuffer))-position, ioMgrReceivedChunkSize)
			readBytes, err = ioMgrConn.readBytesFromIomgr(httpChunkBuffer, maxReadSize, position)
			if err != nil {
				return err
			}
			ioMgrReceivedChunkSize -= uint32(readBytes)
			readBytesTotalPerChunk += readBytes
			position += uint32(readBytes)

			if position == config.httpChunkSize {
				writtenBytes, err = ioMgrConn.writeHTTPChunk(
					c, httpChunkBuffer, config.httpChunkSize)
				if err != nil {
					return err
				}
				position = 0
				writtenBytesTotalPerChunk += uint64(writtenBytes)
			}
			log.Debug("readChunkedJSON | IOMgr bytes read: %v (total: %v), Remain From chunk: %v, position: %v",
				readBytes, readBytesTotalPerChunk, ioMgrReceivedChunkSize, position)

		}

		readBytesTotal += readBytesTotalPerChunk
		writtenBytesTotal += writtenBytesTotalPerChunk

		log.Debug("readChunkedJSON | IOMgr chunk read: %v, HTTP chunk written: %v",
			readBytesTotalPerChunk, writtenBytesTotalPerChunk)
	}

	if position > 0 {
		writtenBytes, err = ioMgrConn.writeHTTPChunk(
			c, httpChunkBuffer, position)
		if err != nil {
			return err
		}
		writtenBytesTotal += uint64(writtenBytes)
	}

	if readBytesTotal != writtenBytesTotal {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return fmt.Errorf("readChunkedJSON | Protocol error: readBytesTotal(%v) != writtenBytesTotal(%v)",
			readBytesTotal, writtenBytesTotal)
	}

	log.Debug("readChunkedJSON | Finish with readBytesTotal: %v and writtenBytesTotal: %v",
		readBytesTotal, writtenBytesTotal)

	return nil
}

func (ioMgrConn *ioMgrConnection) writeHTTPChunk(
	c *gin.Context, buffer []byte, writeSize uint32) (uint64, error) {
	var err error
	writtenBytes, err := c.Writer.Write(buffer[:writeSize])
	if err != nil {
		return uint64(writtenBytes), err
	}
	c.Writer.Flush()
	return uint64(writtenBytes), nil
}

func (ioMgrConn *ioMgrConnection) readBytesFromIomgr(
	buffer []byte, BytesToRead uint32, position uint32) (bytesReadTotal uint64, err error) {

	ioMgrConn.SetReadDeadline(time.Now().Add(ioMgrCPool.readDeadline))
	bytesRead, err := io.ReadFull(ioMgrConn, buffer[position:BytesToRead+position])
	if err != nil {
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			ioMgrConn.Close()
			ioMgrConn.trackedNetConn.Conn = nil
			return uint64(bytesRead),
				fmt.Errorf("readMessage | Protocol error (read timeout): %v, bytes read: %v", err, bytesRead)
		}
		return uint64(bytesRead),
			fmt.Errorf("readMessage | Protocol error: %v, bytes read: %v", err, bytesRead)

	}
	bytesReadTotal += uint64(bytesRead)

	return bytesReadTotal, nil
}

func (ioMgrConn *ioMgrConnection) readVarint32() (bytesRead int, ruint32 uint32, err error) {
	var ruint64 uint64
	bytesRead, ruint64, err = ioMgrConn.readVarint(binary.MaxVarintLen32)
	return bytesRead, uint32(ruint64), err
}

func (ioMgrConn *ioMgrConnection) readVarint64() (bytesRead int, ruint64 uint64, err error) {
	return ioMgrConn.readVarint(binary.MaxVarintLen64)
}

func (ioMgrConn *ioMgrConnection) readVarint(maxVarintLen int) (bytesRead int, ruint64 uint64, err error) {
	// Function readVarint()
	// source: https://github.com/stashed/stash/blob/master/vendor/github.com/matttproud/golang_protobuf_extensions/pbutil/decode.go
	//
	// Copyright 2013 Matt T. Proud
	//
	// Licensed under the Apache License, Version 2.0 (the "License");
	// you may not use this file except in compliance with the License.
	// You may obtain a copy of the License at
	//
	//     http://www.apache.org/licenses/LICENSE-2.0
	//
	// Unless required by applicable law or agreed to in writing, software
	// distributed under the License is distributed on an "AS IS" BASIS,
	// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	// See the License for the specific language governing permissions and
	// limitations under the License.

	prefixBuf := make([]byte, maxVarintLen)
	var varIntBytes int
	for varIntBytes == 0 { // i.e. no varint has been decoded yet.
		if bytesRead >= len(prefixBuf) {
			return bytesRead, ruint64, fmt.Errorf("Invalid varint (size %v) encountered", maxVarintLen)
		}
		// We have to read byte by byte here to avoid reading more bytes
		// than required. Each read byte is appended to what we have
		// read before.
		ioMgrConn.SetReadDeadline(time.Now().Add(ioMgrCPool.readDeadline))
		newBytesRead, err := ioMgrConn.Read(prefixBuf[bytesRead : bytesRead+1])
		if err != nil {
			if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
				ioMgrConn.Close()
				ioMgrConn.trackedNetConn.Conn = nil
				return bytesRead, ruint64, fmt.Errorf("readVarint | Protocol error (read timeout): %v", err)
			}
			return bytesRead, ruint64, fmt.Errorf("readVarint | Protocol error: %v", err)

		}
		if newBytesRead == 0 {
			if io.EOF == err {
				return bytesRead, ruint64, nil
			} else if err != nil {
				return bytesRead, ruint64, err
			}
			// A Reader should not return (0, nil), but if it does,
			// it should be treated as no-op (according to the
			// Reader contract). So let's go on...
			continue
		}
		bytesRead += newBytesRead
		// Now present everything read so far to the varint decoder and
		// see if a varint can be decoded already.
		ruint64, varIntBytes = proto.DecodeVarint(prefixBuf[:bytesRead])
	}

	return bytesRead, ruint64, err
}

func (ioMgrConn *ioMgrConnection) writeMessage(
	messageTypeID uint32, message proto.Message) (uint64, error) {

	var bytesWritten int
	var bytesWrittenTotal uint64 = 0
	var err error
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(messageTypeID))
	bytesWritten, err = ioMgrConn.Write(buf[:encodedLength])
	if nil != err {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return 0, err
	}

	// Write message size
	encodedLength = binary.PutUvarint(buf[:], uint64(proto.Size(message)))

	bytesWritten, err = ioMgrConn.Write(buf[:encodedLength])
	if nil != err {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return 0, err
	}
	bytesWrittenTotal += uint64(bytesWritten)

	// Marshal and write message
	messageMarshaled, err := proto.Marshal(message)
	if err != nil {
		return 0, err
	}
	bytesWritten, err = ioMgrConn.Write(messageMarshaled)
	if nil != err {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return 0, err
	}
	bytesWrittenTotal += uint64(bytesWritten)

	return bytesWrittenTotal, err
}

func (ioMgrConn *ioMgrConnection) readMessage(
	messageTypeID uint32, m proto.Message) (uint64, error) {

	var bytesRead int
	var messageLength uint32
	var readMessageTypeID uint32
	var bytesReadTotal uint64 = 0
	var err error

	// Read Message Type Id
	if bytesRead, readMessageTypeID, err = ioMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += uint64(bytesRead)

	// Check Message Type Id
	if messageTypeID != readMessageTypeID {
		ioMgrConn.Close()
		ioMgrConn.trackedNetConn.Conn = nil
		return 0, fmt.Errorf("readMessage | Protocol error: messageTypeID (%v) != readMessageTypeID (%v)",
			messageTypeID, readMessageTypeID)
	}
	log.Debug("readMessage | readMessageTypeID: %v", readMessageTypeID)

	// Read messageLength
	if bytesRead, messageLength, err = ioMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += uint64(bytesRead)

	// Read message
	messageBuf := make([]byte, messageLength)
	if messageLength > messageLengthMaxSize {
		return 0, fmt.Errorf("readMessage | Protocol error: message length (%v) is bigger than allowed (%v)",
			messageLength, messageLengthMaxSize)
	}
	ioMgrConn.SetReadDeadline(time.Now().Add(ioMgrCPool.readDeadline))
	bytesRead, err = io.ReadFull(ioMgrConn, messageBuf)
	if err != nil {
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			ioMgrConn.Close()
			ioMgrConn.trackedNetConn.Conn = nil
			return uint64(bytesRead), fmt.Errorf("readMessage | Read timeout expired: %v, bytes read: %v",
				err, bytesRead)
		}
		return uint64(bytesRead), fmt.Errorf("readMessage | Protocol error: %v, bytes read: %v",
			err, bytesRead)

	}
	bytesReadTotal += uint64(bytesRead)
	if err != nil {
		return 0, err
	}

	return bytesReadTotal, proto.Unmarshal(messageBuf, m)
}
