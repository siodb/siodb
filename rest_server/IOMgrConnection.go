// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"SiodbIomgrProtocol"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/gin-gonic/gin"
	"github.com/golang/protobuf/proto"
)

var (
	IOMgrChunkMaxSize    uint32 = 65536
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

func (IOMgrConn *IOMgrConnection) cleanupTCPConn() (err error) {

	var IOMgrChunkSize uint32
	var counter int16

	for {
		// Get Current Row Size
		if _, IOMgrChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("cleanupTCPConn: unable to read chunk size")
		}
		if IOMgrChunkSize == 0 {
			siodbLoggerPool.Debug("cleanupTCPConn: %d chunks dropped.", counter)
			return err
		}
		siodbLoggerPool.Debug("cleanupTCPConn: Chunk size to drop: %d.", IOMgrChunkSize)
		buff := make([]byte, IOMgrChunkSize)
		_, err = io.ReadFull(IOMgrConn, buff)

		counter++
	}
}

func (IOMgrConn *IOMgrConnection) writeJSONPayload(requestID uint64, c *gin.Context) (err error) {

	// Read Payload up to max json payload size
	var bytesRead int = 0
	var bytesReadTotal int = 0
	var bytesWritten int = 0
	var bytesWrittenTotal int = 0
	buffer := make([]byte, restServerConfig.RequestPayloadBufferSize)

	siodbLoggerPool.Debug("writeJSONPayload | IOMgrConn.pool.maxJsonPayloadSize: %v, restServerConfig.RequestPayloadBufferSize: %v",
		IOMgrConn.pool.maxJsonPayloadSize, restServerConfig.RequestPayloadBufferSize)

	for {
		if uint64(bytesReadTotal) > IOMgrConn.pool.maxJsonPayloadSize {
			break
		}
		bytesRead, err = io.ReadFull(c.Request.Body, buffer)
		if err == io.ErrUnexpectedEOF {
			bytesWritten, err = IOMgrConn.writeJSONPayloadChunk(buffer, bytesRead)
			if err != nil {
				return err
			}
			bytesReadTotal += bytesRead
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
		bytesReadTotal += bytesRead
		bytesWrittenTotal += bytesWritten
		siodbLoggerPool.Debug("writeJSONPayload | bytes Read: %v (total: %v), bytes Written: %v (total: %v)",
			bytesRead, bytesReadTotal, bytesWritten, bytesWrittenTotal)
	}
	_, err = IOMgrConn.writeJSONPayloadChunk(buffer, 0)
	if err != nil {
		return err
	}

	// Get Returned message
	if err = IOMgrConn.readIOMgrResponse(requestID); err != nil {
		if uint64(bytesReadTotal) > IOMgrConn.pool.maxJsonPayloadSize {
			return fmt.Errorf("JSON payload truncated because it has bigger size than authoried (%v). IOMgr: %v",
				IOMgrConn.pool.maxJsonPayloadSize, err)
		}
		return err
	}

	return nil
}

func (IOMgrConn *IOMgrConnection) writeJSONPayloadChunk(
	JSONPayloadBuffer []byte, size int) (bytesWritten int, err error) {

	// Write Payload length
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(size))
	if _, err = IOMgrConn.Write(buf[:encodedLength]); err != nil {
		return 0, err
	}
	siodbLoggerPool.Debug("writeJSONPayloadChunk | Payload size written to IOMgr: %v",
		uint64(size))

	if size > 0 {
		// Write raw payload
		if bytesWritten, err = IOMgrConn.Write(JSONPayloadBuffer[:size]); err != nil {
			siodbLoggerPool.Error("err: %v", err)
			return bytesWritten, fmt.Errorf("not able to write payload to IOMgr: %v", err)
		}
	}

	return bytesWritten, nil
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

	var IOMgrReceivedChunkSize uint32
	IOMgrbuffSize := uint32(restServerConfig.HTTPChunkSize)
	IOMgrbuff := make([]byte, IOMgrbuffSize)

	c.Writer.Header().Add("Content-Type", "application/json")

	for {

		// Get Current IOMgr chunk Size
		if _, IOMgrReceivedChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("Not able to read chunk size from IOMgr: %v", err)
		}
		siodbLoggerPool.Debug("readChunkedJSON | New chunk arriving from IOMgr with size: %v", IOMgrReceivedChunkSize)

		if IOMgrReceivedChunkSize == 0 {
			break
		}

		numberOfIOMmgrChunksInIOMgrChunk := IOMgrReceivedChunkSize / IOMgrChunkMaxSize
		if IOMgrReceivedChunkSize%IOMgrChunkMaxSize > 0 {
			numberOfIOMmgrChunksInIOMgrChunk++
		}
		siodbLoggerPool.Debug("readChunkedJSON | number of chunks for current IOMgr chunk: %v",
			numberOfIOMmgrChunksInIOMgrChunk)
		for i := uint64(0); i < uint64(numberOfIOMmgrChunksInIOMgrChunk); i++ {

			var NumberOfbytesToReadInIOMmgrCurrentChunk uint32
			if IOMgrReceivedChunkSize > IOMgrChunkMaxSize {
				NumberOfbytesToReadInIOMmgrCurrentChunk = IOMgrChunkMaxSize
				if IOMgrReceivedChunkSize%IOMgrChunkMaxSize > 0 && i == uint64(numberOfIOMmgrChunksInIOMgrChunk)-1 {
					NumberOfbytesToReadInIOMmgrCurrentChunk = IOMgrReceivedChunkSize % IOMgrChunkMaxSize
				}
			} else {
				NumberOfbytesToReadInIOMmgrCurrentChunk = IOMgrReceivedChunkSize
				if IOMgrReceivedChunkSize%IOMgrChunkMaxSize > 0 && i == uint64(numberOfIOMmgrChunksInIOMgrChunk)-1 {
					NumberOfbytesToReadInIOMmgrCurrentChunk = IOMgrReceivedChunkSize % IOMgrChunkMaxSize
				}
			}
			siodbLoggerPool.Debug("readChunkedJSON | NumberOfbytesToReadInIOMmgrCurrentChunk: %v", NumberOfbytesToReadInIOMmgrCurrentChunk)

			// Fill IOMgrbuff from IOMgr data and send IOMgrbuffSize
			IOMgrReceivedChunkReadBytes := uint32(0)
			writtenBytesTotal := uint32(0)
			var numberOfHTTPChunksInIOMgrChunk = NumberOfbytesToReadInIOMmgrCurrentChunk / IOMgrbuffSize
			if NumberOfbytesToReadInIOMmgrCurrentChunk%IOMgrbuffSize > 0 {
				numberOfHTTPChunksInIOMgrChunk++
			}

			siodbLoggerPool.Debug("readChunkedJSON | number of HTTP chunks for current chunk: %v",
				numberOfHTTPChunksInIOMgrChunk)
			for i := uint64(0); i < uint64(numberOfHTTPChunksInIOMgrChunk); i++ {
				NumberOfbytesToRead := IOMgrbuffSize
				if NumberOfbytesToReadInIOMmgrCurrentChunk%IOMgrbuffSize > 0 && i == uint64(numberOfHTTPChunksInIOMgrChunk)-1 {
					NumberOfbytesToRead = NumberOfbytesToReadInIOMmgrCurrentChunk % IOMgrbuffSize
				}
				siodbLoggerPool.Debug("readChunkedJSON | NumberOfbytesToRead: %v", NumberOfbytesToRead)
				bytesRead, err := io.ReadFull(IOMgrConn, IOMgrbuff[:NumberOfbytesToRead])
				if err != nil {
					return err
				}
				IOMgrReceivedChunkReadBytes += uint32(bytesRead)
				writtenBytes, err := c.Writer.Write(IOMgrbuff[:bytesRead])
				if err != nil {
					return err
				}
				writtenBytesTotal += uint32(writtenBytes)
				c.Writer.Flush()
				siodbLoggerPool.Debug("readChunkedJSON | chunk: %v, IOMgr bytes read: %v (total: %v), HTTP bytes written: %v",
					i, bytesRead, IOMgrReceivedChunkReadBytes, writtenBytes)
				siodbLoggerPool.Trace("readChunkedJSON | HTTP chunk content of bytes read : %s", IOMgrbuff[:bytesRead])
			}
			siodbLoggerPool.Debug("readChunkedJSON | IOMgr chunk size received: %v, chunk bytes read total: %v, chunk bytes read written: %v",
				NumberOfbytesToReadInIOMmgrCurrentChunk, IOMgrReceivedChunkReadBytes, writtenBytesTotal)
			if NumberOfbytesToReadInIOMmgrCurrentChunk != IOMgrReceivedChunkReadBytes || NumberOfbytesToReadInIOMmgrCurrentChunk != writtenBytesTotal {
				siodbLoggerPool.Error("Bytes read (%v) diffent from bytes received from IOMgr (%v) and bytes written (%v)",
					NumberOfbytesToReadInIOMmgrCurrentChunk, IOMgrReceivedChunkReadBytes, writtenBytesTotal)
				return fmt.Errorf("Bytes read (%v) diffent from bytes received from IOMgr (%v) and bytes written (%v)",
					NumberOfbytesToReadInIOMmgrCurrentChunk, IOMgrReceivedChunkReadBytes, writtenBytesTotal)
			}
		}
	}

	return nil
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
		newBytesRead, err := IOMgrConn.Read(prefixBuf[bytesRead : bytesRead+1])
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
	messageTypeID uint32, message proto.Message) (bytesWrittenTotal int, err error) {

	var bytesWritten int
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(messageTypeID))
	IOMgrConn.Write(buf[:encodedLength])

	// Write message size
	encodedLength = binary.PutUvarint(buf[:], uint64(proto.Size(message)))

	bytesWritten, err = IOMgrConn.Write(buf[:encodedLength])
	if nil != err {
		return 0, err
	}
	bytesWrittenTotal += bytesWritten

	// Marshal and write message
	messageMarshaled, err := proto.Marshal(message)
	if err != nil {
		return 0, err
	}
	bytesWritten, err = IOMgrConn.Write(messageMarshaled)
	bytesWrittenTotal += bytesWritten

	return bytesWritten, err
}

func (IOMgrConn *IOMgrConnection) readMessage(
	messageTypeID uint32, m proto.Message) (bytesReadTotal int, err error) {

	var bytesRead int
	var messageLength uint32
	var readMessageTypeID uint32

	// Read Message Type Id
	if bytesRead, readMessageTypeID, err = IOMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += bytesRead

	// Check Message Type Id
	if messageTypeID != readMessageTypeID {
		return 0, err
	}
	siodbLoggerPool.Debug("readMessage | readMessageTypeID: %v", readMessageTypeID)

	// Read messageLength
	if bytesRead, messageLength, err = IOMgrConn.readVarint32(); err != nil {
		return 0, err
	}
	bytesReadTotal += bytesRead

	// Read message
	messageBuf := make([]byte, messageLength)
	if messageLength > MessageLengthMaxSize {
		return 0, fmt.Errorf("message length received (%v) bigger than allowed (%v)",
			messageLength, MessageLengthMaxSize)
	}
	bytesRead, err = io.ReadFull(IOMgrConn, messageBuf)
	bytesReadTotal += bytesRead
	if err != nil {
		return 0, err
	}

	return bytesReadTotal, proto.Unmarshal(messageBuf, m)
}
