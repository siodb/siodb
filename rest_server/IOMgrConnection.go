package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"net/http"
	"unicode/utf8"

	"github.com/gin-gonic/gin"
	"github.com/golang/protobuf/proto"
)

var (
	DATABASEENGINERESPONSE    uint64 = 4
	DATABASEENGINERESTREQUEST uint64 = 13
)

type IOMgrConnection struct {
	net.Conn
	pool *IOMgrConnPool
}

func (IOMgrConn IOMgrConnection) cleanupTCPConn() (err error) {

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
		_, err = io.ReadFull(IOMgrConn.Conn, buff)

		counter++
	}

}

func (IOMgrConn IOMgrConnection) streamJSONPayload(c *gin.Context) (err error) {

	var rawData []byte

	//
	// Read until EOF and write by chunk as defined in HTTPChunkSize
	//

	// Read raw payload
	if len(c.Request.Header.Get("Content-Length")) > 0 { // "Transfer-Encoding: chunked" not set
		// send full size
		siodbLoggerPool.Debug("Content-Length: %v", c.Request.Header.Get("Content-Length"))
	} else { // "Transfer-Encoding: chunked" set

	}

	rawData, err = ioutil.ReadAll(c.Request.Body)
	siodbLoggerPool.Output(TRACE, "rawData: %v, %v", rawData, err)

	// Write Payload length
	var buf [binary.MaxVarintLen32]byte
	encodedLength := binary.PutUvarint(buf[:], uint64(len(rawData)))
	siodbLoggerPool.Debug("uint64(len(rawData)): %v", uint64(len(rawData)))
	_, err = IOMgrConn.Conn.Write(buf[:encodedLength])
	if nil != err {
		siodbLoggerPool.Error("err: %v", err)
	}

	// Write raw payload
	if _, err = IOMgrConn.Write(rawData); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Not able to write payload to IOMgr: %v", err)})
	}

	// Write 0 to indicate EOF
	encodedLength = binary.PutUvarint(buf[:], uint64(0))
	_, err = IOMgrConn.Conn.Write(buf[:encodedLength])
	if nil != err {
		siodbLoggerPool.Error("err: %v", err)
	}

	return nil

}

func (IOMgrConn IOMgrConnection) readChunkedJSON(c *gin.Context) (err error) {

	// Stream through HTTP > 1.1
	// https://stackoverflow.com/questions/29486086/how-http2-http1-1-proxy-handle-the-transfer-encoding

	var IOMgrChunkSize uint32
	var JSONChunk string
	var JSONChunkBuff string

	for {

		// Get Current IOMgr chunk Size
		if _, IOMgrChunkSize, err = IOMgrConn.readVarint32(); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Not able to read chunk size from IOMgr: %v", err)})
		}
		siodbLoggerPool.Debug("IOMgrChunkSize: %v", IOMgrChunkSize)
		if IOMgrChunkSize == 0 {
			break
		}

		// Form JSONChunk
		IOMgrbuff := make([]byte, IOMgrChunkSize)
		_, err = io.ReadFull(IOMgrConn.Conn, IOMgrbuff)
		for len(IOMgrbuff) > 0 {
			r, size := utf8.DecodeRune(IOMgrbuff)
			JSONChunk = JSONChunk + fmt.Sprintf("%c", r)

			IOMgrbuff = IOMgrbuff[size:]
		}

		// Bufferizing JSONChunk
		JSONChunkBuff = JSONChunkBuff + JSONChunk
		JSONChunk = ""

		siodbLoggerPool.Debug("len(JSONChunkBuff): %v", len(JSONChunkBuff))
		siodbLoggerPool.Debug("int(restServerConfig.HTTPChunkSize): %v", int(restServerConfig.HTTPChunkSize))
		if len(JSONChunkBuff) >= int(restServerConfig.HTTPChunkSize) {
			siodbLoggerPool.Debug("# len(JSONChunkBuff) > restServerConfig.HTTPChunkSize")
			pos := int(0)
			for i := 0; i <= (int(len(JSONChunkBuff)) - int(uint64(len(JSONChunkBuff))%restServerConfig.HTTPChunkSize) - int(restServerConfig.HTTPChunkSize)); i = i + int(restServerConfig.HTTPChunkSize) {
				siodbLoggerPool.Debug("Steaming buffer from position %v to %v.", i, i+int(restServerConfig.HTTPChunkSize))
				siodbLoggerPool.Output(TRACE, "JSONChunked: %v", JSONChunkBuff[i:i+int(restServerConfig.HTTPChunkSize)])
				c.String(http.StatusOK, JSONChunkBuff[i:i+int(restServerConfig.HTTPChunkSize)])
				pos = i
			}
			siodbLoggerPool.Debug("pos: %v", pos)
			JSONChunkBuff = JSONChunkBuff[pos+int(restServerConfig.HTTPChunkSize):]
		}

	}

	siodbLoggerPool.Debug("JSONChunkBuff leftovers size: %v", len(JSONChunkBuff))
	siodbLoggerPool.Output(TRACE, "JSONChunkBuff leftovers: %v", JSONChunkBuff)
	c.String(http.StatusOK, JSONChunkBuff)

	return nil

}

func (IOMgrConn IOMgrConnection) readVarint32() (bytesRead int, ruint32 uint32, err error) {
	var ruint64 uint64
	if bytesRead, ruint64, err = IOMgrConn.readVarint(); err != nil {
		return bytesRead, uint32(ruint64), err
	}
	return bytesRead, uint32(ruint64), err
}

func (IOMgrConn IOMgrConnection) readVarint64() (bytesRead int, ruint64 uint64, err error) {
	return IOMgrConn.readVarint()
}

func (IOMgrConn IOMgrConnection) readVarint() (bytesRead int, n uint64, err error) {

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
		newBytesRead, err := IOMgrConn.Conn.Read(prefixBuf[bytesRead : bytesRead+1])
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

func (IOMgrConn IOMgrConnection) writeMessage(messageTypeID uint64, m proto.Message) (int, error) {

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
	IOMgrConn.Conn.Write(buf[:encodedLength])

	em, err := proto.Marshal(m)
	if nil != err {
		return 0, err
	}

	encodedLength = binary.PutUvarint(buf[:], uint64(proto.Size(m)))

	vib, err := IOMgrConn.Conn.Write(buf[:encodedLength])
	if nil != err {
		return vib, err
	}

	pbmmb, err := IOMgrConn.Conn.Write(em)

	return vib + pbmmb, err
}

func (IOMgrConn IOMgrConnection) readMessage(messageTypeID uint64, m proto.Message) (n int, err error) {

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
		newBytesRead, err := IOMgrConn.Conn.Read(prefixBuf[bytesRead : bytesRead+1])
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
	newBytesRead, err := io.ReadFull(IOMgrConn.Conn, messageBuf)
	bytesRead += newBytesRead
	if err != nil {
		return 0, err
	}

	return bytesRead, proto.Unmarshal(messageBuf, m)
}

func (IOMgrConn IOMgrConnection) readPayload() (json string, err error) {

	var chunkSize uint32

	for {
		// Get Current Row Size
		if _, chunkSize, err = IOMgrConn.readVarint32(); err != nil {
			return json, err
		}
		siodbLoggerPool.Debug("chunkSize: %v", chunkSize)
		if chunkSize == 0 {
			return json, nil
		}
		buff := make([]byte, chunkSize)
		_, err = io.ReadFull(IOMgrConn.Conn, buff)

		for len(buff) > 0 {
			r, size := utf8.DecodeRune(buff)
			json = json + fmt.Sprintf("%c", r)

			buff = buff[size:]
		}

		siodbLoggerPool.Output(TRACE, "payload: %v", json)

	}

}
