#!/bin/bash
# Test driver for the write unit tests

# Function: 
#     runTestCase number file
# Params:
#     number: Test case number
#     file  : Input file
# Abstract:
# Run the requested test case number with the given file

source ./functions.source

(
	cd ./tmp
	datapath=.

	images="exiv2-canon-powershot-s40.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-sigma-d10.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg"

	copyTestFiles ${images[@]}

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
reportTest

# That's all Folks!
##