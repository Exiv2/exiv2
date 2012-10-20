#! /bin/sh
# Test driver for exifdata copy construction and assignment unit tests 
results="./tmp/exifdata-test.out"
good="./data/exifdata-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi
(
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../bin"
else
    bin="$VALGRIND $EXIV2_BINDIR"
fi
cp -f ./data/exiv2-gc.jpg ./tmp
cp -f ./data/exiv2-canon-powershot-s40.jpg ./tmp
cp -f ./data/exiv2-nikon-d70.jpg ./tmp
cd ./tmp
$bin/exifdata-test exiv2-gc.jpg
$bin/exifdata-test exiv2-canon-powershot-s40.jpg
$bin/exifdata-test exiv2-nikon-d70.jpg
)  | tr -d '\r' > $results

diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
