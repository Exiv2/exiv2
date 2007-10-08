#! /bin/sh
# Test driver for the write unit tests

# Function: 
#     runTestCase number file
# Params:
#     number: Test case number
#     file  : Input file
# Abstract:
# Run the requested test case number with the given file
(
runTestCase()
{
    rtc_number=$1
    rtc_infile=$2

    rtc_outfile=$datapath/test${rtc_number}.jpg
    rtc_jpgthumb=$datapath/thumb${rtc_number}.jpg
    rtc_tifthumb=$datapath/thumb${rtc_number}.tif

    rm -f $rtc_outfile $rtc_jpgthumb $rtc_tifthumb
    rm -f $datapath/iii $datapath/ttt; 

    echo "------------------------------------------------------------"
    $binpath/exifprint $rtc_infile > $datapath/iii;
    cp $rtc_infile $rtc_outfile; 
    $binpath/write-test $rtc_infile $rtc_number > $datapath/ttt; 
    diff -a $datapath/iii $datapath/ttt
}

# **********************************************************************
# main

binpath="$VALGRIND ../../samples"
datapath="."
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

images="exiv2-canon-powershot-s40.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-sigma-d10.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg"

for i in $images; do cp -f ./data/$i ./tmp; done
cd ./tmp

runTestCase  1 $datapath/exiv2-canon-powershot-s40.jpg
runTestCase  2 $datapath/exiv2-canon-powershot-s40.jpg
runTestCase  3 $datapath/exiv2-kodak-dc210.jpg
runTestCase  4 $datapath/exiv2-canon-powershot-s40.jpg
runTestCase  5 $datapath/exiv2-canon-powershot-s40.jpg
runTestCase  6 $datapath/exiv2-kodak-dc210.jpg
runTestCase  7 $datapath/exiv2-fujifilm-finepix-s2pro.jpg
runTestCase  8 $datapath/exiv2-sigma-d10.jpg
runTestCase  9 $datapath/exiv2-nikon-e990.jpg
runTestCase 10 $datapath/exiv2-nikon-e950.jpg
runTestCase 11 $datapath/exiv2-nikon-d70.jpg

) > tmp/write-test.out 2>&1

diff -q -w -a $diffargs tmp/write-test.out data/write-test.out
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff -w -a $diffargs tmp/write-test.out data/write-test.out
fi
