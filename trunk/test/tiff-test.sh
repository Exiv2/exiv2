#! /bin/sh
# TIFF parser test driver

# ----------------------------------------------------------------------
# Setup
results="./tmp/tiff-test.out"
good="./data/tiff-test.out"

# ----------------------------------------------------------------------
# Main routine
(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND ../../src/exiv2"
cd ./tmp

# ----------------------------------------------------------------------
# Basic write test
testfile=mini9.tif
cp -f ../data/$testfile .
exifprobe $testfile
$binpath/tiff-test $testfile
exifprobe $testfile

) > $results

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
