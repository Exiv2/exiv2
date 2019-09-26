#!/usr/bin/env bash
# Test driver for exiv2.exe --verbose --version

source ./functions.source

(   cd "$testdir"
    runTest exiv2 --verbose --version
)

# That's all Folks!
##
