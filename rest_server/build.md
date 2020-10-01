# Build Instruction for the REST server

## Install Go + dependencies

```bash
wget https://golang.org/dl/go1.15.2.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.15.2.linux-amd64.tar.gz
```

Official Golang instructions [here](https://golang.org/doc/install).

## Install dependencies

```bash
export PATH=$PATH:/usr/local/go/bin
go get -u github.com/golang/protobuf/protoc-gen-go
go get -u github.com/gin-gonic/gin
```

## Compile IOMgr protobuf message for Go

```bash
export PATH=$PATH:~/go/bin
export GOPATH=~/go
mkdir -p $GOPATH/src/SiodbIomgrProtocol
cd siodb
protoc --proto_path=common/lib/siodb/common/proto --go_out=$GOPATH/src/SiodbIomgrProtocol common/lib/siodb/common/proto/CommonMessages.proto
protoc --proto_path=common/lib/siodb/common/proto --go_out=$GOPATH/src/SiodbIomgrProtocol common/lib/siodb/common/proto/ColumnDataType.proto
protoc --proto_path=common/lib/siodb/common/proto --go_out=$GOPATH/src/SiodbIomgrProtocol common/lib/siodb/common/proto/IOManagerProtocol.proto
```

## Build

```bash
cd siodb
export PATH=$PATH:/usr/local/go/bin
```

### Release

```bash
go build -o /tmp/siodb_rest_server -ldflags "-s -w" ./rest_server
```

### Debug

```bash
go build -o /tmp/siodb_rest_server -gcflags="all=-N -l -dwarf -dwarflocationlists" ./rest_server
```
