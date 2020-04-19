#!/usr/bin/env bash
# Test driver for exiv2.exe --verbose --version

source ./functions.source

(   cd "$testdir"
                              runTest exiv2 --verbose --version | grep -v -e ^xmlns
    echo xmlns entry count: $(runTest exiv2 --verbose --version | grep    -e ^xmlns | wc -l)
)

# That's all Folks!
##
