#!/usr/bin/env bash
# Test driver for exiv2.exe --verbose --version

source ./functions.source

(   cd "$testdir"
    # Curiously the pipe into grep causes FreeBSD to core dump!
    if [ $(uname) != "FreeBSD" ]; then
								  runTest exiv2 --verbose --version | grep -v ^xmlns
		echo xmlns entry count: $(runTest exiv2 --verbose --version | grep    ^xmlns | wc -l)
    else
        runTest exiv2 --verbose --version     
    fi
)

# That's all Folks!
##
