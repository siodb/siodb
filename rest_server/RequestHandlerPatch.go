// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"SiodbIomgrProtocol"
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

func (restWorker RestWorker) patchRow(c *gin.Context) {
	siodbLoggerPool.Debug("handler: patchRow")
	var rowID uint64
	var err error
	if rowID, err = strconv.ParseUint(c.Param("row_id"), 10, 64); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "Invalid row_id"})
		siodbLoggerPool.Error("%v", err)
	} else {
		restWorker.patch(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), rowID)
	}
}

func (restWorker RestWorker) patch(
	c *gin.Context, ObjectType SiodbIomgrProtocol.DatabaseObjectType, ObjectName string, ObjectId uint64) (err error) {

	start := time.Now()
	IOMgrConn, _ := IOMgrCPool.GetTrackedNetConn()
	defer IOMgrCPool.ReturnTrackedNetConn(IOMgrConn)
	siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)

	// Use to trap interuption from client
	defer func() {
		if r := recover(); r != nil {
			siodbLoggerPool.Debug("Recovered from: %v", r)
			if err := IOMgrConn.cleanupTCPConn(); err != nil {
				siodbLoggerPool.FatalAndExit(FATAL_UNABLE_TO_CLEANUP_TCP_BUFFER,
					"unable to cleanup TCP buffer after broken pipe from client: %v", err)
			}
		}
		siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)
		siodbLoggerPool.LogRequest(c, time.Now().Sub(start))
	}()

	var UserName, Token string
	if UserName, Token, err = loadAuthenticationData(c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusUnauthorized, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	var requestID uint64
	if requestID, err = IOMgrConn.writeIOMgrRequest(
		SiodbIomgrProtocol.RestVerb_PATCH, ObjectType, UserName, Token, ObjectName, ObjectId); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.readIOMgrResponse(requestID); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Write Payload
	if err := IOMgrConn.writeJSONPayload(requestID, c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Read and stream chunked JSON
	if err := IOMgrConn.readChunkedJSON(c); err != nil {
		siodbLoggerPool.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	return nil
}
