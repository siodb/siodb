module github.com/siodb/siodb/rest_server

go 1.16

require (
	github.com/gin-gonic/gin v1.6.3
	github.com/golang/protobuf v1.5.1
	golang.org/x/sys v0.0.0-20210326220804-49726bf1d181
	siodb.io/siodb/siodbproto v0.0.0
)

replace (
	siodb.io/siodb/siodbproto => ../build/cross-config/generated-src/go-modules/siodb.io/siodb/siodbproto
)
