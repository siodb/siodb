// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"SiodbIomgrProtocol"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang/protobuf/proto"
)

var (
	IOMgrChunkMaxSize    uint32 = (2 * 1024 * 1024 * 1024) - 1
	MessageLengthMaxSize uint32 = 1 * 1024 * 1024
)

var (
	DATABASEENGINERESPONSE    uint32 = 4
	DATABASEENGINERESTREQUEST uint32 = 13
)

type IOMgrConnection struct {
	*TrackedNetConn
	pool *IOMgrConnPool
}

func (IOMgrConn *IOMgrConnection) writeJSONPayload(requestID uint64, c *gin.Context) (err error) {

	// Read Payload up to max json payload size
	var bytesRead int = 0
	var bytesReadTotal uint64 = 0
	var bytesWritten uint64 = 0
	var bytesWrittenTotal uint64 = 0
	buffer := make([]byte, restServerConfig.RequestPayloadBufferSize)

	siodbLoggerPool.Debug("writeJSONPayload | IOMgrConn.pool.maxJsonPayloadSize: %v, restServerConfig.RequestPayloadBufferSize: %v",
		IOMgrConn.pool.maxJsonPayloadSize, restServerConfig.RequestPayloadBufferSize)

	for {
		if bytesReadTotal > uint64(IOMgrConn.pool.maxJsonPayloadSize) {
			IOMgrConn.Close()
			IOMgrConn.TrackedNetConn.Conn = nil
			return fmt.Errorf("JSON payload is too large:  received %v bytes, but expecting at most %v bytes",
				bytesReadTotal, IOMgrConn.pool.maxJsonPayloadSize)
		}
		bytesRead, err = io.ReadFull(c.Request.Body, buffer)
		if err == io.ErrUnexpectedEOF {
			bytesWritten, err = IOMgrConn.writeJSONPayloadChunk(buffer, bytesRead)
			if err != nil {
				return err
			}
			bytesReadTotal += uint64(bytesRead)
			bytesWrittenTotal += bytesWritten
			siodbLoggerPool.Debug("writeJSONPayload | bytes Read: %v (total: %v), bytes Written: %v (total: %v)",
				bytesRead, bytesReadTotal, bytesWritten, bytesWrittenTotal)
			break
		} else if err != nil {
			return err
		}
		bytesWritten, err = IOMgrConn.writeJSONPayloadChunk(buffer, bytesRead)
		if err != nil {
			return err
		}
		bytesReadTotal += uint64(bytesRead)
		bytesWrittenTotal += bytesWritten
		siodbLoggerPool.Debug("writeJSONPayload | bytes Read: %v (total: %v), bytes Written: %v (total: %v)",
			bytesRead, bytesReadTotal, bytesWritten, bytesWrittenTotal)
	}
	_, err = IOMgrConn.writeJSONPayloadChunk(buffer, 0)
	if err != nil {
		return err
	}

	return nil
}

func (IOMgrConn *IOMgrConnection) writeJSONPayloadChunk(
	JSONPayloadBuffer []byte, size int) (uint64, error) {

	var bytesWritten int = 0
	// Write Payload length
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(size))
	if _, err := IOMgrConn.Write(buf[:encodedLength]); err != nil {
		return 0, err
	}
	siodbLoggerPool.Debug("writeJSONPayloadChunk | Payload size written to IOMgr: %v",
		uint64(size))

	if size > 0 {
		// Write raw payload
		if bytesWritten, err := IOMgrConn.Write(JSONPayloadBuffer[:size]); err != nil {
			siodbLoggerPool.Error("err: %v", err)
			return uint64(bytesWritten), fmt.Errorf("not able to write payload to IOMgr: %v", err)
		}
	}

	return uint64(bytesWritten), nil
}

func (IOMgrConn *IOMgrConnection) writeIOMgrRequest(
	restVerb SiodbIomgrProtocol.RestVerb,
	objectType SiodbIomgrProtocol.DatabaseObjectType,
	userName string,
	token string,
	objectName string,
	objectId uint64) (requestId uint64, err error) {

	requestId = IOMgrConn.RequestID
	IOMgrConn.RequestID++
	var databaseEngineRestRequest SiodbIomgrProtocol.DatabaseEngineRestRequest
	databaseEngineRestRequest.RequestId = requestId
	databaseEngineRestRequest.Verb = restVerb
	databaseEngineRestRequest.ObjectType = objectType
	databaseEngineRestRequest.UserName = userName
	databaseEngineRestRequest.Token = token

	if len(objectName) > 0 {
		databaseEngineRestRequest.ObjectName = objectName
	}
	if objectId > 0 {
		databaseEngineRestRequest.ObjectId = objectId
	}
	siodbLoggerPool.Debug("writeIOMgrRequest | databaseEngineRestRequest: %v", databaseEngineRestRequest)
	siodbLoggerPool.Debug("writeIOMgrRequest | IOMgrConn.RequestID: %v", requestId)
	siodbLoggerPool.Debug("writeIOMgrRequest | IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

	if _, err := IOMgrConn.writeMessage(DATABASEENGINERESTREQUEST, &databaseEngineRestRequest); err != nil {
		return requestId, fmt.Errorf("Unable to write message to IOMgr: %v", err)
	}

	return requestId, nil
}

func (IOMgrConn *IOMgrConnection) readIOMgrResponse(requestID uint64) (err error) {

	var databaseEngineResponse SiodbIomgrProtocol.DatabaseEngineResponse

	if _, err := IOMgrConn.readMessage(DATABASEENGINERESPONSE, &databaseEngineResponse); err != nil {
		return fmt.Errorf("Unable to read response from IOMgr: %v", err)
	}
	siodbLoggerPool.Debug("readIOMgrResponse | databaseEngineResponse: %v", databaseEngineResponse)
	siodbLoggerPool.Debug("readIOMgrResponse | databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	siodbLoggerPool.Debug("readIOMgrResponse | IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

	if databaseEngineResponse.RequestId != requestID {
		return fmt.Errorf("request IDs mismatch")
	}

	if len(databaseEngineResponse.Message) > 0 {
		return fmt.Errorf("code: %v, message: %v", databaseEngineResponse.Message[0].GetStatusCode(), databaseEngineResponse.Message[0].GetText())
	}

	return nil
}

func (IOMgrConn *IOMgrConnection) readChunkedJSON(c *gin.Context) (err error) {

	var IOMgrReceivedChunkSize uint32 = 0
	var position uint32 = 0
	var readBytes uint64 = 0
	var writtenBytes uint64 = 0
	var readBytesTotal uint64 = 0
	var writtenBytesTotal uint64 = 0
	httpChunkBuffer := make([]byte, uint32(restServerConfig.HTTPChunkSize))

	c.Writer.Header().Add("Content-Type", "application/json")

	for {

		readBytesTotalPerChunk := uint64(0)
		writtenBytesTotalPerChunk := uint64(0)

		// Get Current IOMgr chunk Size
		if _, IOMgrReceivedChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("Not able to read chunk size from IOMgr: %v", err)
		}
		siodbLoggerPool.Debug("readChunkedJSON | New chunk with size: %v", IOMgrReceivedChunkSize)

		if IOMgrReceivedChunkSize == 0 {
			break
		}

		if IOMgrReceivedChunkSize > IOMgrChunkMaxSize {
			IOMgrConn.Close()
			IOMgrConn.TrackedNetConn.Conn = nil
			return fmt.Errorf("readChunkedJSON | protocol error: can't read IOMgr chunk size")
		}

		for IOMgrReceivedChunkSize > 0 {
			maxReadSize := minUint32(uint32(len(httpChunkBuffer))-position, IOMgrReceivedChunkSize)
			readBytes, err = IOMgrConn.readBytesFromIomgr(httpChunkBuffer, maxReadSize, position)
			if err != nil {
				return err
			}
			IOMgrReceivedChunkSize -= uint32(readBytes)
			readBytesTotalPerChunk += readBytes
			position += uint32(readBytes)

			if position == restServerConfig.HTTPChunkSize {
				writtenBytes, err = IOMgrConn.writeHttpChunk(
					c, httpChunkBuffer, restServerConfig.HTTPChunkSize)
				if err != nil {
					return err
				}
				position = 0
				writtenBytesTotalPerChunk += uint64(writtenBytes)
			}
			siodbLoggerPool.Debug("readChunkedJSON | IOMgr bytes read: %v (total: %v), Remain From chunk: %v, position: %v",
				readBytes, readBytesTotalPerChunk, IOMgrReceivedChunkSize, position)

		}

		readBytesTotal += readBytesTotalPerChunk
		writtenBytesTotal += writtenBytesTotalPerChunk

		siodbLoggerPool.Debug("readChunkedJSON | IOMgr chunk read: %v, HTTP chunk written: %v",
			readBytesTotalPerChunk, writtenBytesTotalPerChunk)
	}

	if position > 0 {
		writtenBytes, err = IOMgrConn.writeHttpChunk(
			c, httpChunkBuffer, position)
		if err != nil {
			return err
		}
		writtenBytesTotal += uint64(writtenBytes)
	}

	if readBytesTotal != writtenBytesTotal {
		IOMgrConn.Close()
		IOMgrConn.TrackedNetConn.Conn = nil
		return fmt.Errorf("readChunkedJSON | protocol error: readBytesTotal(%v) != writtenBytesTotal(%v)",
			readBytesTotal, writtenBytesTotal)
	}

	siodbLoggerPool.Debug("readChunkedJSON | Finish with readBytesTotal: %v and writtenBytesTotal: %v",
		readBytesTotal, writtenBytesTotal)

	return nil
}

func (IOMgrConn *IOMgrConnection) writeHttpChunk(
	c *gin.Context, buffer []byte, writeSize uint32) (bytesWrittenTotal uint64, err error) {
	writtenBytes, err := c.Writer.Write(buffer[:writeSize])
	if err != nil {
		return uint64(writtenBytes), err
	}
	bytesWrittenTotal += uint64(writtenBytes)
	c.Writer.Flush()
	return bytesWrittenTotal, nil
}

func (IOMgrConn *IOMgrConnection) readBytesFromIomgr(
	buffer []byte, BytesToRead uint32, position uint32) (bytesReadTotal uint64, err error) {

	IOMgrConn.SetReadDeadline(time.Now().Add(IOMgrCPool.readDeadline))
	bytesRead, err := io.ReadFull(IOMgrConn, buffer[position:BytesToRead+position])
	if err != nil {
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			IOMgrConn.Close()
			IOMgrConn.TrackedNetConn.Conn = nil
			return uint64(bytesRead),
				fmt.Errorf("readMessage | Protocol error (read timeout): %v, bytes read: %v", err, bytesRead)
		} else {
			return uint64(bytesRead),
				fmt.Errorf("readMessage | Protocol error: %v, bytes read: %v", err, bytesRead)
		}
	}
	bytesReadTotal += uint64(bytesRead)

	return bytesReadTotal, nil
}

func (IOMgrConn *IOMgrConnection) readVarint32() (bytesRead int, ruint32 uint32, err error) {
	var ruint64 uint64
	bytesRead, ruint64, err = IOMgrConn.readVarint(binary.MaxVarintLen32)
	ruint32 = uint32(ruint64)
	return bytesRead, ruint32, err
}

func (IOMgrConn *IOMgrConnection) readVarint64() (bytesRead int, ruint64 uint64, err error) {
	return IOMgrConn.readVarint(binary.MaxVarintLen64)
}

func (IOMgrConn *IOMgrConnection) readVarint(maxVarintLen int) (bytesRead int, ruint64 uint64, err error) {

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
			return bytesRead, ruint64, fmt.Errorf("invalid varint (size %v) encountered", maxVarintLen)
		}
		// We have to read byte by byte here to avoid reading more bytes
		// than required. Each read byte is appended to what we have
		// read before.
		IOMgrConn.SetReadDeadline(time.Now().Add(IOMgrCPool.readDeadline))
		newBytesRead, err := IOMgrConn.Read(prefixBuf[bytesRead : bytesRead+1])
		if err != nil {
			if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
				IOMgrConn.Close()
				IOMgrConn.TrackedNetConn.Conn = nil
				return bytesRead, ruint64, fmt.Errorf("readVarint | Protocol error (read timeout): %v", err)
			} else {
				return bytesRead, ruint64, fmt.Errorf("readVarint | Protocol error: %v", err)
			}
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

func (IOMgrConn *IOMgrConnection) writeMessage(
	messageTypeID uint32, message proto.Message) (uint64, error) {

	var bytesWritten int
	var bytesWrittenTotal uint64 = 0
	var err error
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(messageTypeID))
	IOMgrConn.Write(buf[:encodedLength])

	// Write message size
	encodedLength = binary.PutUvarint(buf[:], uint64(proto.Size(message)))

	bytesWritten, err = IOMgrConn.Write(buf[:encodedLength])
	if nil != err {
		return 0, err
	}
	bytesWrittenTotal += uint64(bytesWritten)

	// Marshal and write message
	messageMarshaled, err := proto.Marshal(message)
	if err != nil {
		return 0, err
	}
	bytesWritten, err = IOMgrConn.Write(messageMarshaled)
	bytesWrittenTotal += uint64(bytesWritten)

	return bytesWrittenTotal, err
}

func (IOMgrConn *IOMgrConnection) readMessage(
	messageTypeID uint32, m proto.Message) (uint64, error) {

	var bytesRead int
	var messageLength uint32
	var readMessageTypeID uint32
	var bytesReadTotal uint64 = 0
	var err error

	// Read Message Type Id
	if bytesRead, readMessageTypeID, err = IOMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += uint64(bytesRead)

	// Check Message Type Id
	if messageTypeID != readMessageTypeID {
		return 0, err
	}
	siodbLoggerPool.Debug("readMessage | readMessageTypeID: %v", readMessageTypeID)

	// Read messageLength
	if bytesRead, messageLength, err = IOMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += uint64(bytesRead)

	// Read message
	messageBuf := make([]byte, messageLength)
	if messageLength > MessageLengthMaxSize {
		return 0, fmt.Errorf("message length received (%v) bigger than allowed (%v)",
			messageLength, MessageLengthMaxSize)
	}
	IOMgrConn.SetReadDeadline(time.Now().Add(IOMgrCPool.readDeadline))
	bytesRead, err = io.ReadFull(IOMgrConn, messageBuf)
	if err != nil {
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			IOMgrConn.Close()
			IOMgrConn.TrackedNetConn.Conn = nil
			return uint64(bytesRead), fmt.Errorf("readMessage | Protocol error (read timeout): %v, bytes read: %v",
				err, bytesRead)
		} else {
			return uint64(bytesRead), fmt.Errorf("readMessage | Protocol error: %v, bytes read: %v",
				err, bytesRead)
		}
	}
	bytesReadTotal += uint64(bytesRead)
	if err != nil {
		return 0, err
	}

	return bytesReadTotal, proto.Unmarshal(messageBuf, m)
}
