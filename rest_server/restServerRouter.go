package main

import (
	"github.com/gin-gonic/gin"
)

type RestServerRouter struct {
	ginEngine *gin.Engine
	Port      uint64
}

func (restServerRouter *RestServerRouter) New(listeningPort uint64) (err error) {

	restServerRouter.Port = listeningPort
	return nil

}
