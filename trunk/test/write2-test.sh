#! /bin/sh
# Test driver for write unit tests to build Exif metadata from scratch
results="./tmp/write2-test.out"
good="./data/write2-test.out"
(
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
binpath="../../src"
cp -f ./data/exiv2-empty.jpg ./tmp
cd ./tmp
$binpath/write2-test exiv2-empty.jpg
) > $results

diff -q $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results $good
fi
