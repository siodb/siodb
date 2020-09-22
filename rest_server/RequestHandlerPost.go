package main

import (
	"SiodbIomgrProtocol"
	"fmt"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
)

func (restWorker RestWorker) postRows(c *gin.Context) {

	siodbLoggerPool.Debug("postRows")
	restWorker.post(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (restWorker RestWorker) post(
	c *gin.Context, ObjectType SiodbIomgrProtocol.DatabaseObjectType, ObjectName string, ObjectId uint64) error {

	start := time.Now()
	IOMgrConn, _ := IOMgrCPool.GetTrackedNetConn()
	defer IOMgrCPool.ReturnTrackedNetConn(IOMgrConn)
	siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)

	// Use to trap interuption from client
	defer func() {
		if r := recover(); r != nil {
			siodbLoggerPool.Debug("Recovered from: %v", r)
			if err := IOMgrConn.cleanupTCPConn(); err != nil {
				siodbLoggerPool.Fatal(FATAL_UNABLE_TO_CLEANUP_TCP_BUFFER, "unable to cleanup TCP buffer after broken pipe from client: %v", err)
			}
		}
		siodbLoggerPool.Debug("IOMgrConn: %v", IOMgrConn)
		siodbLoggerPool.LogRequest(c, time.Now().Sub(start))
	}()

	var databaseEngineRestRequest SiodbIomgrProtocol.DatabaseEngineRestRequest
	databaseEngineRestRequest.RequestId = IOMgrConn.RequestID
	databaseEngineRestRequest.Verb = SiodbIomgrProtocol.RestVerb_POST
	databaseEngineRestRequest.ObjectType = ObjectType

	UserName, Token, _ := loadAuthenticationData(c)
	siodbLoggerPool.Debug("user: %v", UserName)
	databaseEngineRestRequest.UserName = UserName
	databaseEngineRestRequest.Token = Token

	if len(ObjectName) > 0 {
		databaseEngineRestRequest.ObjectName = ObjectName
	}
	if ObjectId > 0 {
		databaseEngineRestRequest.ObjectId = ObjectId
	}
	siodbLoggerPool.Debug("databaseEngineRestRequest: %v", databaseEngineRestRequest)

	// Send DatabaseEngineRestRequest here
	if _, err := IOMgrConn.writeMessage(DATABASEENGINERESTREQUEST, &databaseEngineRestRequest); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Unable to write message to IOMgr: %v", err)})
		return err
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Get response + check RequestID
	var databaseEngineResponse SiodbIomgrProtocol.DatabaseEngineResponse

	// Get Returned message
	if _, err := IOMgrConn.readMessage(DATABASEENGINERESPONSE, &databaseEngineResponse); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Unable to read response from IOMgr: %v", err)})
		return err
	}
	siodbLoggerPool.Debug("databaseEngineResponse: %v", databaseEngineResponse)
	siodbLoggerPool.Debug("databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

	if IOMgrConn.RequestID != databaseEngineResponse.RequestId {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "request IDs mismatch."})
		return nil
	}
	//IOMgrConn.RequestID++
	siodbLoggerPool.Debug("IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

	// Return error or read and stream chunked JSON
	if len(databaseEngineResponse.Message) > 0 {
		c.JSON(http.StatusInternalServerError, gin.H{"code": databaseEngineResponse.Message[0].GetStatusCode(), "message": databaseEngineResponse.Message[0].GetText()})
		return nil
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Write Payload
	if err := IOMgrConn.streamJSONPayload(c); err != nil {
		// Get Returned message
		if _, err := IOMgrConn.readMessage(DATABASEENGINERESPONSE, &databaseEngineResponse); err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Unable to read response from IOMgr: %v", err)})
			return err
		}
		siodbLoggerPool.Debug("databaseEngineResponse: %v", databaseEngineResponse)
		siodbLoggerPool.Debug("databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
		siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

		if IOMgrConn.RequestID != databaseEngineResponse.RequestId {
			c.JSON(http.StatusInternalServerError, gin.H{"Error:": "request IDs mismatch."})
			return nil
		}
		IOMgrConn.RequestID++
		siodbLoggerPool.Debug("IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

		c.JSON(http.StatusInternalServerError, gin.H{
			"code": databaseEngineResponse.Message[0].GetStatusCode(),
			"message": "Unable to write payload to IOMgr: " + fmt.Sprintf("%v", err) +
				"( IOMgr: " + databaseEngineResponse.Message[0].GetText() + ")"})
		return err
	}

	// Get response + check RequestID
	//var databaseEngineResponse SiodbIomgrProtocol.DatabaseEngineResponse

	// Get Returned message
	if _, err := IOMgrConn.readMessage(DATABASEENGINERESPONSE, &databaseEngineResponse); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": fmt.Sprintf("Unable to read response from IOMgr: %v", err)})
		return err
	}
	siodbLoggerPool.Debug("databaseEngineResponse: %v", databaseEngineResponse)
	siodbLoggerPool.Debug("databaseEngineResponse.RequestId: %v", databaseEngineResponse.RequestId)
	siodbLoggerPool.Debug("IOMgrConn.RequestID: %v", IOMgrConn.RequestID)

	if IOMgrConn.RequestID != databaseEngineResponse.RequestId {
		c.JSON(http.StatusInternalServerError, gin.H{"Error:": "request IDs mismatch."})
		return nil
	}
	IOMgrConn.RequestID++
	siodbLoggerPool.Debug("IOMgrConn.RequestID++: %v", IOMgrConn.RequestID)

	// Return error or read and stream chunked JSON
	if len(databaseEngineResponse.Message) > 0 {
		c.JSON(http.StatusInternalServerError, gin.H{"code": databaseEngineResponse.Message[0].GetStatusCode(), "message": databaseEngineResponse.Message[0].GetText()})
		return nil
	} else {
		IOMgrConn.readChunkedJSON(c)
	}

	return nil

}
