package main

import (
	"SiodbIomgrProtocol"
	"fmt"

	"github.com/gin-gonic/gin"
)

func (restWorker RestWorker) postRows(c *gin.Context) {

	siodbLoggerPool.Output(DEBUG, "postRows")
	restWorker.post(c, SiodbIomgrProtocol.DatabaseObjectType_ROW, c.Param("database_name")+"."+c.Param("table_name"), 0)
}

func (restWorker RestWorker) post(c *gin.Context, ObjectType SiodbIomgrProtocol.DatabaseObjectType, ObjectName string, ObjectId uint64) {

	IOMgrConn := &IOMgrConnection{pool: IOMgrCPool}
	IOMgrConn.Conn, _ = IOMgrCPool.GetConn()
	defer IOMgrCPool.ReturnConn(IOMgrConn)
	siodbLoggerPool.Output(DEBUG, "IOMgrConn: %v", IOMgrConn)

	// Use to trap interuption from client
	defer func() {
		if r := recover(); r != nil {
			siodbLoggerPool.Output(DEBUG, "Recovered from: %v", r)
			if err := IOMgrConn.cleanupTCPConn(); err != nil {
				siodbLoggerPool.Output(FATAL, "unable to cleanup TCP buffer after broken pipe from client: %v", err)
			}
		}
	}()

	var databaseEngineRestRequest SiodbIomgrProtocol.DatabaseEngineRestRequest
	databaseEngineRestRequest.RequestId = restWorker.RequestID
	databaseEngineRestRequest.Verb = 1
	databaseEngineRestRequest.ObjectType = ObjectType

	UserName, Token, _ := loadAuthenticationData(c)
	siodbLoggerPool.Output(DEBUG, "Authorization scheme: %v| token: ***", UserName)
	databaseEngineRestRequest.UserName = UserName
	databaseEngineRestRequest.Token = Token

	if len(ObjectName) > 0 {
		databaseEngineRestRequest.ObjectName = ObjectName
	}
	if ObjectId > 0 {
		databaseEngineRestRequest.ObjectId = ObjectId
	}
	siodbLoggerPool.Output(DEBUG, "databaseEngineRestRequest: %v", databaseEngineRestRequest)

	// Send DatabaseEngineRestRequest here
	if _, err := IOMgrConn.writeMessage(13, &databaseEngineRestRequest); err != nil {
		c.JSON(500, gin.H{"Error:": fmt.Sprintf("Not able to write message to IOMgr: %v", err)})
	}

	IOMgrConn.streamJSONPayload(c)

	// Get response + check RequestID
	var databaseEngineResponse SiodbIomgrProtocol.DatabaseEngineResponse

	if _, err := IOMgrConn.readMessage(4, &databaseEngineResponse); err != nil {
		c.JSON(500, gin.H{"Error:": fmt.Sprintf("Not able to read response from IOMgr: %v", err)})
	}

	siodbLoggerPool.Output(DEBUG, "DatabaseEngineResponse: %v", databaseEngineResponse)
	if restWorker.RequestID != databaseEngineResponse.RequestId {
		c.JSON(500, gin.H{"Error:": "request IDs mismatch."})
	}
	restWorker.RequestID++

	// Return error or read and stream chunked JSON
	if len(databaseEngineResponse.Message) > 0 {
		c.JSON(500, gin.H{"code": databaseEngineResponse.Message[0].GetStatusCode(), "message": databaseEngineResponse.Message[0].GetText()})
	} else {
		IOMgrConn.readChunkedJSON(c, databaseEngineResponse.RestStatusCode)
	}

}
