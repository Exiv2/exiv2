#! /bin/sh
# Mini test-driver for path utility functions
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
cd ./tmp

$binpath/path-test ../data/path-test.txt
