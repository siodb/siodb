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
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "unable to parse row_id."})
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
				siodbLoggerPool.Fatal(FATAL_UNABLE_TO_CLEANUP_TCP_BUFFER,
					"unable to cleanup TCP buffer after broken pipe from client: %v", err)
			}
		}
		siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)
		siodbLoggerPool.LogRequest(c, time.Now().Sub(start))
	}()

	var UserName, Token string
	if UserName, Token, err = loadAuthenticationData(c); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.writeIOMgrRequest(
		SiodbIomgrProtocol.RestVerb_PATCH, ObjectType, UserName, Token, ObjectName, ObjectId); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	if err := IOMgrConn.readIOMgrResponse(false); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Write Payload
	if err := IOMgrConn.streamJSONPayload(c); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	// Read and stream chunked JSON
	if err := IOMgrConn.readChunkedJSON(c); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("%v", err)})
		return err
	}

	return nil

}
