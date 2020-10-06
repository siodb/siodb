#!/bin/bash

SLEEP_TIME=5

mkdir ~/tmp

while true
do
    rm -rf ~/tmp
    ./request_handler_test --gtest_filter=DDL.CreateManyTables 2>&1 | tee DDL.CreateManyTables.log
    if [[ $? != 0 ]]; then
        exit
    fi
    echo "===== Sleeping for ${SLEEP_TIME}s ====="
    sleep ${SLEEP_TIME}
done
