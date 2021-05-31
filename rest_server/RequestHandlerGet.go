// Copyright (C) 2019-2021 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

package main

import (
	"fmt"
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"

	"siodb.io/siodb/siodbproto"
)

func (worker restWorker) getDatabases(c *gin.Context) {
	log.Debug("getDatabases")
	worker.get(c, siodbproto.DatabaseObjectType_DATABASE, "", 0, true)
}

func (worker restWorker) getTables(c *gin.Context) {
	log.Debug("getTables")
	worker.get(c, siodbproto.DatabaseObjectType_TABLE, c.Param("database_name"), 0, true)
}

func (worker restWorker) getRows(c *gin.Context) {
	log.Debug("getRows")
	worker.get(c, siodbproto.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0, true)
}

func (worker restWorker) getRow(c *gin.Context) {
	log.Debug("handler: getRow")
	var rowID uint64
	var err error
	if rowID, err = strconv.ParseUint(c.Param("row_id"), 10, 64); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"Error:": "Invalid row_id"})
		log.Error("%v", err)
		return
	}
	worker.get(c, siodbproto.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), rowID, true)
}

func (worker restWorker) getSqlQuery(c *gin.Context) {
	log.Debug("getSqlQuery")
	var err error

	// Try single query
	var q = c.Query("q")
	if q != "" {
		// Handle single query
		worker.get(c, siodbproto.DatabaseObjectType_SQL, q, 0, true)
		return
	}

	// Try multiple queries
	q = c.Query("q1")
	if q == "" {
		c.JSON(http.StatusBadRequest, gin.H{"Error:": "Missing query"})
		log.Error("%v", "Missing query")
		return
	}

	// Multiple queries found
	c.Writer.WriteHeader(200) // always 200, but see actual JSON then
	_, err = c.Writer.WriteString("[")
	if err != nil {
		log.Error("%v", err)
		return
	}
	var i int = 1
	for {
		var parameterName = "q" + strconv.Itoa(i)
		q = c.Query(parameterName)
		if q == "" {
			break
		}
		if i > 1 {
			_, err = c.Writer.WriteString(",")
			if err != nil {
				log.Error("%v", err)
				return
			}
		}
		err = worker.get(c, siodbproto.DatabaseObjectType_SQL, q, 0, false)
		if err != nil {
			log.Error("%v", err)
			return
		}
		i = i + 1
	}
	_, err = c.Writer.WriteString("]")
	if err != nil {
		log.Error("%v", err)
		return
	}
	log.Debug("getSqlQuery finished")
}

func (worker restWorker) get(
	c *gin.Context, ObjectType siodbproto.DatabaseObjectType, ObjectName string, ObjectID uint64,
	CanWriteHeader bool) (err error) {

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
		siodbproto.RestVerb_GET, ObjectType, userName, token, ObjectName, ObjectID); err != nil {
		log.Error("%v", err)
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if restStatusCode, err := ioMgrConn.readIOMgrResponse(requestID); err != nil {
		log.Error("%v", err)
		c.JSON(int(restStatusCode), gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	} else if CanWriteHeader {
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
