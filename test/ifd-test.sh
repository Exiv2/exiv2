#! /bin/sh
# Test driver for Ifd unit tests 
results="./tmp/ifd-test.out"
good="./data/ifd-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
binpath=" $VALGRIND ../../samples"
cd ./tmp
$binpath/ifd-test
) > $results 2>&1

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
