#!/usr/bin/env bash
# Test driver for the write unit tests

source ./functions.source

(	cd "$testdir"

	images="exiv2-canon-powershot-s40.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-sigma-d10.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg"

	copyTestFiles ${images[@]}

	runTestCase  1 ./exiv2-canon-powershot-s40.jpg
	runTestCase  2 ./exiv2-canon-powershot-s40.jpg
	runTestCase  3 ./exiv2-kodak-dc210.jpg
	runTestCase  4 ./exiv2-canon-powershot-s40.jpg
	runTestCase  5 ./exiv2-canon-powershot-s40.jpg
	runTestCase  6 ./exiv2-kodak-dc210.jpg
	runTestCase  7 ./exiv2-fujifilm-finepix-s2pro.jpg
	runTestCase  8 ./exiv2-sigma-d10.jpg
	runTestCase  9 ./exiv2-nikon-e990.jpg
	runTestCase 10 ./exiv2-nikon-e950.jpg
	runTestCase 11 ./exiv2-nikon-d70.jpg

) > $testdir/write-test.out 2>&1

reportTest

# That's all Folks!
##