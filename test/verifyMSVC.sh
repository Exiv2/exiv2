#!/bin/bash

##
# read the output from testMSVC.sh and generate summary
# there should be 24 tests (3 compilers) * (2 archs) * (4 targets)
# compilers: 2005    | 8 | 10
# archs:     Win32   | x64
# targets:   Release | ReleaseDLL | Debug | DebugDLL
##

if [ $# != 1 ]; then
	echo usage: $0 \<path-to-results\>
	exit
fi

filename="$1"
if [ ! -e "$filename" ]; then
	echo filename "$filename" does not exist
	exit
fi	

nl "$filename" | grep "\-\-\-\-\ " | while read i ; do
	let x=$(echo $i | cut -d' ' -f 1)
	let d=x-o
	if [ $d != 1 ]; then
		echo $d $i
	fi
	let o=x
done

# That's all Folks
##
