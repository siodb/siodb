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
	IOMgrChunkMaxSize         uint32 = 65536
	IOMgrChunkMaxBufferedSize uint32 = 2048
	JSONPayloadBufferSize     uint64 = 64 * 1024
	MessageLengthMaxSize      uint32 = 1 * 1024 * 1024
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
	var bytesReadTotal uint64 = 0
	var body []byte
	siodbLoggerPool.Debug("IOMgrConn.pool.maxJsonPayloadSize: %v", IOMgrConn.pool.maxJsonPayloadSize)

	for true {
		buffer := make([]byte, JSONPayloadBufferSize)
		if JSONPayloadBufferSize > IOMgrConn.pool.maxJsonPayloadSize {
			buffer = make([]byte, IOMgrConn.pool.maxJsonPayloadSize)
		}
		if bytesReadTotal > IOMgrConn.pool.maxJsonPayloadSize {
			break
		}
		bytesRead, err := io.ReadFull(c.Request.Body, buffer)
		if err == io.ErrUnexpectedEOF {
			body = AppendBytes(body, buffer[:bytesRead])
			bytesReadTotal = bytesReadTotal + uint64(bytesRead)
			break
		} else if err != nil {
			return err
		}
		body = AppendBytes(body, buffer)
		bytesReadTotal = bytesReadTotal + uint64(bytesRead)
	}
	siodbLoggerPool.Trace("body: %s", body)
	siodbLoggerPool.Debug("bytes read: %v", bytesReadTotal)
	siodbLoggerPool.Debug("len(body): %v", len(body))

	// Write Payload length
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(len(body)))
	if _, err = IOMgrConn.Write(buf[:encodedLength]); err != nil {
		return err
	}

	// Write raw payload
	if _, err = IOMgrConn.Write(body); err != nil {
		siodbLoggerPool.Error("err: %v", err)
		return fmt.Errorf("not able to write payload to IOMgr: %v", err)
	}

	// Write 0 to indicate EOF
	encodedLength = binary.PutUvarint(buf[:], uint64(0))
	_, err = IOMgrConn.Write(buf[:encodedLength])
	if nil != err {
		siodbLoggerPool.Error("err: %v", err)
	}

	// Get Returned message
	if err = IOMgrConn.readIOMgrResponse(requestID); err != nil {
		if bytesReadTotal > IOMgrConn.pool.maxJsonPayloadSize {
			return fmt.Errorf("JSON payload truncated because it has bigger size than authoried (%v). IOMgr: %v",
				IOMgrConn.pool.maxJsonPayloadSize, err)
		}
		return err
	}

	return nil

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
	siodbLoggerPool.Debug("databaseEngineRestRequest: %v", databaseEngineRestRequest)
	siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", requestId)
	siodbLoggerPool.Debug("IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

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
	siodbLoggerPool.Debug("databaseEngineResponse: %v", databaseEngineResponse)
	siodbLoggerPool.Debug("databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

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
	//var IOMgrBytesRead uint32
	var IOMgrbuffSize uint32
	if IOMgrChunkMaxSize < IOMgrChunkMaxBufferedSize {
		IOMgrbuffSize = IOMgrChunkMaxSize
	} else if IOMgrChunkMaxSize >= IOMgrChunkMaxBufferedSize {
		IOMgrbuffSize = IOMgrChunkMaxBufferedSize
	}
	IOMgrbuff := make([]byte, IOMgrbuffSize)

	c.Writer.Header().Add("Content-Type", "application/json")

	for {

		// Get Current IOMgr chunk Size
		if _, IOMgrReceivedChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("Not able to read chunk size from IOMgr: %v", err)
		}
		siodbLoggerPool.Debug("New chunk arriving from IOMgr with size: %v", IOMgrReceivedChunkSize)

		if IOMgrReceivedChunkSize == 0 {
			break
		}

		// Fill IOMgrbuff from IOMgr data and send restServerConfig.HTTPChunkSize
		IOMgrReceivedChunkReadBytes := uint32(0)

		for i := uint64(0); i < (uint64(IOMgrReceivedChunkSize)-(uint64(IOMgrReceivedChunkSize)%restServerConfig.HTTPChunkSize))/restServerConfig.HTTPChunkSize; i++ {
			bytesRead, _ := io.ReadFull(IOMgrConn, IOMgrbuff[:restServerConfig.HTTPChunkSize])
			IOMgrReceivedChunkReadBytes += uint32(bytesRead)
			writtenBytes, _ := c.Writer.Write(IOMgrbuff[:bytesRead])
			c.Writer.Flush()
			siodbLoggerPool.Debug("IOMgr bytes read: %v, HTTP bytes sent: %v", bytesRead, writtenBytes)
			siodbLoggerPool.Trace("HTTP chunk content of bytes read : %s", IOMgrbuff[:bytesRead])
		}
		siodbLoggerPool.Debug("IOMgr chunk size received: %v, chunk bytes read total: %v",
			IOMgrReceivedChunkSize, IOMgrReceivedChunkReadBytes)

		bytesRead, _ := io.ReadFull(IOMgrConn, IOMgrbuff[:IOMgrReceivedChunkSize-IOMgrReceivedChunkReadBytes])
		IOMgrReceivedChunkReadBytes += uint32(bytesRead)
		_, _ = c.Writer.Write(IOMgrbuff[:bytesRead])
		c.Writer.Flush()
		siodbLoggerPool.Debug("IOMgr chunk size received: %v, chunk bytes read total: %v",
			IOMgrReceivedChunkSize, IOMgrReceivedChunkReadBytes)

	}

	return nil
}

func (IOMgrConn *IOMgrConnection) readAndStreamHTTPChunksToClient(
	c *gin.Context, buffer []byte, offset uint32, byteToRead uint32) (uint32, uint32, error) {

	var bytesReadTotal int = 0

	if uint64(byteToRead) >= restServerConfig.HTTPChunkSize {
		for i := uint64(0); i < (uint64(byteToRead)-(uint64(byteToRead)%restServerConfig.HTTPChunkSize))/restServerConfig.HTTPChunkSize; i++ {
			bytesRead, _ := io.ReadFull(IOMgrConn, buffer[offset:restServerConfig.HTTPChunkSize])
			bytesReadTotal += bytesRead
			c.Writer.Write(buffer[:bytesRead])
			c.Writer.Flush()
			siodbLoggerPool.Trace("Content of bytes read : %s", buffer[:offset+uint32(bytesRead)])
			offset = 0
		}
		return uint32(bytesReadTotal), uint32(byteToRead) % uint32(restServerConfig.HTTPChunkSize), nil
	} else {
		bytesRead, _ := io.ReadFull(IOMgrConn, buffer[offset:restServerConfig.HTTPChunkSize])
		bytesReadTotal += bytesRead
		return uint32(bytesReadTotal), uint32(byteToRead), nil
	}

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
	siodbLoggerPool.Debug("readMessage > readMessageTypeID: %v", readMessageTypeID)

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
