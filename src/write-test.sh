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

images="img_1771.jpg \
        kodak-dc210.jpg \
        dscf0176.jpg \
        sigma-d10-IMG10086.jpg \
        nikon-coolpix-990-DSCN1180.jpg \
        nikon-d70-dsc_0063.jpg \
        nikon-e950.jpg"

for i in $images; do cp -f ../test/images/$i ./; done

runTestCase  1 ./img_1771.jpg
runTestCase  2 ./img_1771.jpg
runTestCase  3 ./kodak-dc210.jpg
runTestCase  4 ./img_1771.jpg
runTestCase  5 ./img_1771.jpg
runTestCase  6 ./kodak-dc210.jpg
runTestCase  7 ./dscf0176.jpg
runTestCase  8 ./sigma-d10-IMG10086.jpg
runTestCase  9 ./nikon-coolpix-990-DSCN1180.jpg
runTestCase 10 ./nikon-e950.jpg
runTestCase 11 ./nikon-d70-dsc_0063.jpg
