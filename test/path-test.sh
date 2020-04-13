#!/usr/bin/env bash
# Mini test-driver for path utility functions
source ./functions.source

(   cd "$testdir"

    if [ $(existsTest pathtest) == 1 ] ; then
        copyTestFile      path-test.txt
        runTest pathtest path-test.txt
    else
        echo "$0: path-test executable not found. Skipping path tests."
    fi
)

# That's all Folks!
##
