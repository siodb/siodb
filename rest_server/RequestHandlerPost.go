// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"net/http"
	"siodbproto"
	"time"

	"github.com/gin-gonic/gin"
)

func (worker restWorker) postRows(c *gin.Context) {
	log.Debug("handler: postRows")
	worker.post(c, siodbproto.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (worker restWorker) post(
	c *gin.Context, ObjectType siodbproto.DatabaseObjectType, ObjectName string, ObjectID uint64) (err error) {

	start := time.Now()
	ioMgrConn, _ := ioMgrCPool.GetTrackedNetConn()
	defer closeRequest(c, ioMgrConn, start)
	log.Debug("ioMgrConn: %v", ioMgrConn)

	var userName, token string
	if userName, token, err = loadAuthenticationData(c); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusUnauthorized, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	var requestID uint64
	if requestID, err = ioMgrConn.writeIOMgrRequest(
		siodbproto.RestVerb_POST, ObjectType, userName, token, ObjectName, ObjectID); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if _, err := ioMgrConn.readIOMgrResponse(requestID); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Write Payload
	if err := ioMgrConn.writeJSONPayload(requestID, c); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if restStatusCode, err := ioMgrConn.readIOMgrResponse(requestID); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	} else {
		c.Writer.WriteHeader(int(restStatusCode))
	}

	// Read and stream chunked JSON
	if err := ioMgrConn.readChunkedJSON(c); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	return nil
}
