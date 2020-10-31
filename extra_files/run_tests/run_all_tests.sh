#!/bin/bash

for f in `ls *_test`; do
    if [[ -f ./$f && -x ./$f ]]; then
        echo "Running $f"
        ./$f
        res=$?
        if [[ $res -ne 0 ]]; then
            echo "$f failed, exit code $res"
            exit $?
        fi
    fi
done
