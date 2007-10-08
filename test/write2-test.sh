#! /bin/sh
# Test driver for write unit tests to build Exif metadata from scratch
results="./tmp/write2-test.out"
good="./data/write2-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
binpath="$VALGRIND ../../samples"
cp -f ./data/exiv2-empty.jpg ./tmp
cd ./tmp
$binpath/write2-test exiv2-empty.jpg
) > $results

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
