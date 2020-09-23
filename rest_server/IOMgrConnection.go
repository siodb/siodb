package main

import (
	"SiodbIomgrProtocol"
	"encoding/binary"
	"fmt"
	"io"
	"net/http"
	"unicode/utf8"

	"github.com/gin-gonic/gin"
	"github.com/golang/protobuf/proto"
)

var (
	IOMgrChunkMaxBufferedSize uint32 = 1024
)

var (
	DATABASEENGINERESPONSE    uint64 = 4
	DATABASEENGINERESTREQUEST uint64 = 13
)

type IOMgrConnection struct {
	TrackedNetConn
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

func (IOMgrConn *IOMgrConnection) streamJSONPayload(c *gin.Context) (err error) {

	// Read Payload up to max json payload size
	body := make([]byte, 0)
	buffer := make([]byte, 1)
	var bytesRead uint64 = 0

	siodbLoggerPool.Debug("IOMgrConn.pool.maxJsonPayloadSize: %v", IOMgrConn.pool.maxJsonPayloadSize)

	for true {
		if bytesRead > IOMgrConn.pool.maxJsonPayloadSize {
			break
		}
		newBytesRead, err := c.Request.Body.Read(buffer)
		if newBytesRead == 0 {
			if io.EOF == err {
				break
			} else if err != nil {
				return err
			}
		}
		body = append(body, buffer[0])
		bytesRead = bytesRead + uint64(newBytesRead)
	}
	siodbLoggerPool.Debug("Request.body: %s", body)
	siodbLoggerPool.Debug("bytes read: %v", bytesRead)
	siodbLoggerPool.Debug("len(Request.body): %v", len(body))

	var buf [binary.MaxVarintLen32]byte
	// Write Payload length
	encodedLength := binary.PutUvarint(buf[:], uint64(len(body)))
	_, err = IOMgrConn.Write(buf[:encodedLength])
	if nil != err {
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
	if err = IOMgrConn.readIOMgrResponse(true); err != nil {
		if bytesRead > IOMgrConn.pool.maxJsonPayloadSize {
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
	objectId uint64) (err error) {

	var databaseEngineRestRequest SiodbIomgrProtocol.DatabaseEngineRestRequest
	databaseEngineRestRequest.RequestId = IOMgrConn.RequestID
	databaseEngineRestRequest.Verb = restVerb
	databaseEngineRestRequest.ObjectType = objectType

	siodbLoggerPool.Debug("user: %v", userName)
	databaseEngineRestRequest.UserName = userName
	databaseEngineRestRequest.Token = token

	if len(objectName) > 0 {
		databaseEngineRestRequest.ObjectName = objectName
	}
	if objectId > 0 {
		databaseEngineRestRequest.ObjectId = objectId
	}
	siodbLoggerPool.Debug("databaseEngineRestRequest: %v", databaseEngineRestRequest)

	if _, err := IOMgrConn.writeMessage(DATABASEENGINERESTREQUEST, &databaseEngineRestRequest); err != nil {
		return fmt.Errorf("Unable to write message to IOMgr: %v", err)
	}

	return nil

}

func (IOMgrConn *IOMgrConnection) readIOMgrResponse(incrementRequestID bool) (err error) {

	var databaseEngineResponse SiodbIomgrProtocol.DatabaseEngineResponse

	if _, err := IOMgrConn.readMessage(DATABASEENGINERESPONSE, &databaseEngineResponse); err != nil {
		return fmt.Errorf("Unable to read response from IOMgr: %v", err)
	}
	siodbLoggerPool.Debug("databaseEngineResponse: %v", databaseEngineResponse)
	siodbLoggerPool.Debug("databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

	if IOMgrConn.RequestID != databaseEngineResponse.RequestId {
		return fmt.Errorf("request IDs mismatch")
	}

	if incrementRequestID {
		IOMgrConn.RequestID++
	}
	siodbLoggerPool.Debug("IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

	if len(databaseEngineResponse.Message) > 0 {
		return fmt.Errorf("code: %v, message: %v", databaseEngineResponse.Message[0].GetStatusCode(), databaseEngineResponse.Message[0].GetText())
	}

	return nil
}

func (IOMgrConn *IOMgrConnection) readChunkedJSON(c *gin.Context) (err error) {

	// Stream through HTTP > 1.1
	// https://stackoverflow.com/questions/29486086/how-http2-http1-1-proxy-handle-the-transfer-encoding

	var IOMgrChunkSize uint32
	var IOMgrbuffSize uint32
	var IOMgrBytesRead int
	var IOMgrbuffRemain []byte
	var JSONChunk string
	var JSONChunkBuff string

	for {

		// Get Current IOMgr chunk Size
		if _, IOMgrChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return fmt.Errorf("Not able to read chunk size from IOMgr: %v", err)
		}
		siodbLoggerPool.Debug("IOMgrChunkSize: %v", IOMgrChunkSize)

		if IOMgrChunkSize == 0 {
			break
		}

		for IOMgrChunkSize != 0 {
			if IOMgrChunkSize < IOMgrChunkMaxBufferedSize {
				IOMgrbuffSize = IOMgrChunkSize
				IOMgrChunkSize = 0
			} else if IOMgrChunkSize >= IOMgrChunkMaxBufferedSize {
				IOMgrbuffSize = IOMgrChunkMaxBufferedSize
				IOMgrChunkSize = IOMgrChunkSize - IOMgrChunkMaxBufferedSize
			}
			siodbLoggerPool.Debug("IOMgrbuffSize: %v", IOMgrbuffSize)

			IOMgrbuff := make([]byte, uint32(len(IOMgrbuffRemain))+IOMgrbuffSize)

			// Form JSONChunk
			IOMgrBytesRead, err = io.ReadFull(IOMgrConn, IOMgrbuff[len(IOMgrbuffRemain):])
			siodbLoggerPool.Debug("Bytes read from IOMgr: %v", IOMgrBytesRead)
			if len(IOMgrbuffRemain) > 0 {
				bc := copy(IOMgrbuff, IOMgrbuffRemain)
				siodbLoggerPool.Debug("Bytes copied from IOMgrbuffRemain into IOMgrbuff : %v", bc)
				IOMgrbuffRemain = nil
			}

			for len(IOMgrbuff) > 0 {
				UTF8rune, size := utf8.DecodeRune(IOMgrbuff)
				if UTF8rune == utf8.RuneError { // Truncated UTF8 read from IOMgr: keeps for next loop
					IOMgrbuffRemain = make([]byte, len(IOMgrbuff))
					IOMgrbuffRemain = IOMgrbuff
					break
				} else {
					JSONChunk = JSONChunk + fmt.Sprintf("%c", UTF8rune)
					IOMgrbuff = IOMgrbuff[size:]
				}
			}

			// Bufferizing JSONChunk
			JSONChunkBuff = JSONChunkBuff + JSONChunk
			siodbLoggerPool.Debug("len(IOMgrbuffRemain): %v", len(IOMgrbuffRemain))
			siodbLoggerPool.Debug("len(JSONChunk): %v", len(JSONChunk))
			siodbLoggerPool.Debug("len(JSONChunkBuff): %v", len(JSONChunkBuff))
			JSONChunk = ""

			siodbLoggerPool.Debug("int(restServerConfig.HTTPChunkSize): %v", int(restServerConfig.HTTPChunkSize))
			siodbLoggerPool.Trace("JSONChunkBuff: %s", JSONChunkBuff)
			if len(JSONChunkBuff) >= int(restServerConfig.HTTPChunkSize) {
				siodbLoggerPool.Debug("# len(JSONChunkBuff) > restServerConfig.HTTPChunkSize: sending to client...")
				pos := int(0)
				for i := 0; i <= (int(len(JSONChunkBuff)) - int(uint64(len(JSONChunkBuff))%restServerConfig.HTTPChunkSize) - int(restServerConfig.HTTPChunkSize)); i = i + int(restServerConfig.HTTPChunkSize) {
					siodbLoggerPool.Debug("Steaming buffer from position %v to %v.", i, i+int(restServerConfig.HTTPChunkSize))
					siodbLoggerPool.Trace("JSONChunkBuff Steamed: %s", JSONChunkBuff[i:i+int(restServerConfig.HTTPChunkSize)])
					c.String(http.StatusOK, JSONChunkBuff[i:i+int(restServerConfig.HTTPChunkSize)])
					pos = i
				}
				siodbLoggerPool.Debug("pos: %v", pos)
				JSONChunkBuff = JSONChunkBuff[pos+int(restServerConfig.HTTPChunkSize):]
				siodbLoggerPool.Debug("JSONChunkBuff leftovers size: %v", len(JSONChunkBuff))
				siodbLoggerPool.Trace("JSONChunkBuff final leftovers: %s", JSONChunkBuff)
			}

		}

	}

	siodbLoggerPool.Debug("JSONChunkBuff final leftovers size: %v", len(JSONChunkBuff))
	siodbLoggerPool.Trace("JSONChunkBuff final leftovers: %v", JSONChunkBuff)
	c.String(http.StatusOK, JSONChunkBuff)

	return nil

}

func (IOMgrConn *IOMgrConnection) readVarint32() (bytesRead int, ruint32 uint32, err error) {
	var ruint64 uint64
	if bytesRead, ruint64, err = IOMgrConn.readVarint(); err != nil {
		return bytesRead, uint32(ruint64), err
	}
	return bytesRead, uint32(ruint64), err
}

func (IOMgrConn *IOMgrConnection) readVarint64() (bytesRead int, ruint64 uint64, err error) {
	return IOMgrConn.readVarint()
}

func (IOMgrConn *IOMgrConnection) readVarint() (bytesRead int, n uint64, err error) {

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

	var prefixBuf [binary.MaxVarintLen64]byte
	var varIntBytes int
	for varIntBytes == 0 { // i.e. no varint has been decoded yet.
		if bytesRead >= len(prefixBuf) {
			return bytesRead, n, fmt.Errorf("invalid varint64 encountered")
		}
		// We have to read byte by byte here to avoid reading more bytes
		// than required. Each read byte is appended to what we have
		// read before.
		newBytesRead, err := IOMgrConn.Read(prefixBuf[bytesRead : bytesRead+1])
		if newBytesRead == 0 {
			if io.EOF == err {
				return bytesRead, n, nil
			} else if err != nil {
				return bytesRead, n, err
			}
			// A Reader should not return (0, nil), but if it does,
			// it should be treated as no-op (according to the
			// Reader contract). So let's go on...
			continue
		}
		bytesRead += newBytesRead
		// Now present everything read so far to the varint decoder and
		// see if a varint can be decoded already.
		n, varIntBytes = proto.DecodeVarint(prefixBuf[:bytesRead])
	}

	return bytesRead, n, err
}

func (IOMgrConn *IOMgrConnection) writeMessage(messageTypeID uint64, m proto.Message) (int, error) {

	// Function writeMessage()
	// source: https://github.com/stashed/stash/blob/master/vendor/github.com/matttproud/golang_protobuf_extensions/pbutil/encode.go
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

	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], messageTypeID)
	IOMgrConn.Write(buf[:encodedLength])

	em, err := proto.Marshal(m)
	if nil != err {
		return 0, err
	}

	encodedLength = binary.PutUvarint(buf[:], uint64(proto.Size(m)))

	vib, err := IOMgrConn.Write(buf[:encodedLength])
	if nil != err {
		return vib, err
	}

	pbmmb, err := IOMgrConn.Write(em)

	return vib + pbmmb, err
}

func (IOMgrConn *IOMgrConnection) readMessage(messageTypeID uint64, m proto.Message) (n int, err error) {

	// Function readMessage()
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

	var prefixBuf [binary.MaxVarintLen64]byte
	var bytesRead, varIntBytes int
	var messageLength uint64
	var readMessageTypeID uint64

	// Read and check Message Type Id
	_, readMessageTypeID, err = IOMgrConn.readVarint()
	if messageTypeID != readMessageTypeID {
		return 0, err
	}

	// Read Message
	for varIntBytes == 0 { // i.e. no varint has been decoded yet.
		if bytesRead >= len(prefixBuf) {
			return 0, fmt.Errorf("invalid varint64 encountered")
		}
		// We have to read byte by byte here to avoid reading more bytes
		// than required. Each read byte is appended to what we have
		// read before.
		newBytesRead, err := IOMgrConn.Read(prefixBuf[bytesRead : bytesRead+1])
		if newBytesRead == 0 {
			if io.EOF == err {
				return 0, nil
			} else if err != nil {
				return 0, err
			}
			// A Reader should not return (0, nil), but if it does,
			// it should be treated as no-op (according to the
			// Reader contract). So let's go on...
			continue
		}
		bytesRead += newBytesRead
		// Now present everything read so far to the varint decoder and
		// see if a varint can be decoded already.
		messageLength, varIntBytes = proto.DecodeVarint(prefixBuf[:bytesRead])
	}

	messageBuf := make([]byte, messageLength)
	newBytesRead, err := io.ReadFull(IOMgrConn, messageBuf)
	bytesRead += newBytesRead
	if err != nil {
		return 0, err
	}

	return bytesRead, proto.Unmarshal(messageBuf, m)
}
