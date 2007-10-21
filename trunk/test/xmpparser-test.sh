#! /bin/sh
# XMP parser test driver

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ ! -e ../samples/xmpparser-test ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

# ----------------------------------------------------------------------
# Setup
results="./tmp/xmpparser-test.out"
good="./data/xmpparser-test.out"

# ----------------------------------------------------------------------
# Main routine
(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND ../../src/exiv2"
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

# ----------------------------------------------------------------------
# xmpsample
$binpath/xmpsample

# ----------------------------------------------------------------------
# XMP sample commands
cp -f ../data/exiv2-empty.jpg .
$exiv2 -v -m ../data/cmdxmp.txt exiv2-empty.jpg
$exiv2 -v -px exiv2-empty.jpg

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d '\r' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results-stripped $good
fi
