#! /bin/sh
# Test driver to run the addmoddel sample program
results="./tmp/addmoddel.out"
good="./data/addmoddel.out"
(
binpath="../../src"
cp -f ./data/exiv2-empty.jpg ./tmp
cd ./tmp
$binpath/addmoddel exiv2-empty.jpg
$binpath/exiv2 -pv exiv2-empty.jpg
) > $results

diff -q $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results $good
fi
