#! /bin/sh
# TIFF parser test driver

# ----------------------------------------------------------------------
# Check if the exifprobe tool is available
if [ `which exifprobe 2>/dev/null`x = x ] ; then
    echo "tiff-test.sh: exifprobe not found. Skipping TIFF tests."
    exit 0
fi

# ----------------------------------------------------------------------
# Setup
results="./tmp/tiff-test.out"
good="./data/tiff-test.out"

# ----------------------------------------------------------------------
# Main routine
(
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
cd ./tmp

# ----------------------------------------------------------------------
# Basic write test
testfile=mini9.tif
cp -f ../data/$testfile .
exifprobe $testfile
$samples/tiff-test $testfile
exifprobe $testfile

) > $results

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
