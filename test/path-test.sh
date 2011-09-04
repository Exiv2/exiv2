#! /bin/sh
# Mini test-driver for path utility functions
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
cd ./tmp

if { test -f $bin/path-test || test -f $bin/path-test.exe; }; then
    $bin/path-test ../data/path-test.txt
else
    echo "path-test.sh: path-test executable not found. Skipping path tests."
fi
