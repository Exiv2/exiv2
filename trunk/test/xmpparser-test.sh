#! /bin/sh
# XMP parser test driver

# ----------------------------------------------------------------------
# Setup
export LC_ALL=C
results="./xmpparser-test.out"
good="../data/xmpparser-test.out"
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
cd ./tmp

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ ! -e ../../samples/xmpparser-test -a ! -e "$EXIV2_BINDIR/xmpparser-test" ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

# ----------------------------------------------------------------------
# Main routine
(
# ----------------------------------------------------------------------
# BlueSquare
testfile=BlueSquare.xmp
cp -f ../data/$testfile .
$samples/xmpparser-test $testfile
diff $testfile ${testfile}-new

# ----------------------------------------------------------------------
# StaffPhotographer-Example
testfile=StaffPhotographer-Example.xmp
cp -f ../data/$testfile .
$samples/xmpparser-test $testfile
diff $testfile ${testfile}-new

# ----------------------------------------------------------------------
# xmpsdk
testfile=xmpsdk.xmp
cp -f ../data/$testfile .
$samples/xmpparser-test $testfile
diff $testfile ${testfile}-new
$samples/xmpparse ${testfile} > t1 2>&1
$samples/xmpparse ${testfile}-new > t2 2>&1
diff t1 t2

# ----------------------------------------------------------------------
# xmpsample
$samples/xmpsample

# ----------------------------------------------------------------------
# XMP sample commands
cp -f ../data/exiv2-empty.jpg .
$bin/exiv2 -v -m ../data/cmdxmp.txt exiv2-empty.jpg
$bin/exiv2 -v -px exiv2-empty.jpg

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results-stripped $good
fi
