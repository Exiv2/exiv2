#! /bin/sh
# Test driver for the write unit tests

# Function: 
#     runTestCase number file
# Params:
#     number: Test case number
#     file  : Input file
# Abstract:
# Run the requested test case number with the given file
runTestCase()
{
    rtc_number=$1
    rtc_infile=$2

    rtc_outfile=test${rtc_number}.jpg
    rtc_jpgthumb=thumb${rtc_number}.jpg
    rtc_tifthumb=thumb${rtc_number}.tif

    rm -f $rtc_outfile $rtc_jpgthumb $rtc_tifthumb
    rm -f iii ttt; 

    echo "------------------------------------------------------------"
    ./exifprint $rtc_infile > iii;
    cp $rtc_infile $rtc_outfile; 
    ./write-test $rtc_infile $rtc_number > ttt; 
    diff iii ttt
}

# **********************************************************************
# main

runTestCase  1 ../test/img_1771.jpg
runTestCase  2 ../test/img_1771.jpg
runTestCase  3 ../test/kodak-dc210.jpg
runTestCase  4 ../test/img_1771.jpg
runTestCase  5 ../test/img_1771.jpg
runTestCase  6 ../test/kodak-dc210.jpg
runTestCase  7 ../test/dscf0176.jpg
runTestCase  8 ../test/sigma-d10-IMG10086.jpg
runTestCase  9 ../test/nikon-coolpix-990-DSCN1180.jpg
runTestCase 10 ../test/nikon-e950.jpg
runTestCase 11 ../test/nikon-d70-dsc_0063.jpg
