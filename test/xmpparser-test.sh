#! /bin/sh
# XMP parser test driver

# ----------------------------------------------------------------------
# Setup
results="./tmp/xmpparser-test.out"
good="./data/xmpparser-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

# ----------------------------------------------------------------------
# Main routine
(
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
cd ./tmp

# ----------------------------------------------------------------------
# BlueSquare
testfile=BlueSquare.xmp
cp -f ../data/$testfile .
$binpath/xmpparser-test $testfile
diff $testfile ${testfile}-new

# ----------------------------------------------------------------------
# StaffPhotographer-Example
testfile=StaffPhotographer-Example.xmp
cp -f ../data/$testfile .
$binpath/xmpparser-test $testfile
diff $testfile ${testfile}-new

# ----------------------------------------------------------------------
# xmpsdk
testfile=xmpsdk.xmp
cp -f ../data/$testfile .
$binpath/xmpparser-test $testfile
diff $testfile ${testfile}-new
$binpath/xmpparse ${testfile} > t1 2>&1
$binpath/xmpparse ${testfile}-new > t2 2>&1
diff t1 t2

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
