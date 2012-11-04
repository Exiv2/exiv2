#!/bin/bash
# Mini test-driver for path utility functions
source ./functions.source
cd ./tmp

if [ $(existsTest path-test) == 1 ] ; then
    runTest path-test ../data/path-test.txt
else
    echo "$0: path-test executable not found. Skipping path tests."
fi

# That's all Folks!
##