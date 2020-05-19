#!/bin/bash

echo Copying custom built libraries...
rm -rf lib
mkdir lib
cp -f /usr/local/lib/libantlr4-runtime.so.4.8 lib
cp -f /usr/local/lib/libprotobuf.so.21 lib


echo Copying Siodb binaries...
rm -rf bin
cp -Rf ../../debug/bin .

#DOCKER_BUILDKIT=1 \
docker build -t siodb-test --ssh default .
