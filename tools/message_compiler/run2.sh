#!/bin/bash

../../build/debug/bin/message_compiler \
    -T -VM -i ../../iomgr/lib/messages/iomgr.msg -o iomgr.txt

exit $?
