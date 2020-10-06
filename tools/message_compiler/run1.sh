#!/bin/bash

../../build/debug/bin/message_compiler -T -VM -i test.msg -o test.txt
res=$?
if [[ $res != 0 ]]; then exit $res; fi

cat test.txt
