#! /bin/sh
# Test driver to run the addmoddel sample program
results="./tmp/addmoddel.out"
good="./data/addmoddel.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
binpath="../../src"
cp -f ./data/exiv2-empty.jpg ./tmp
cd ./tmp
$binpath/addmoddel exiv2-empty.jpg
$binpath/exiv2 -pv exiv2-empty.jpg
) > $results

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
