#!/usr/bin/env bash
# Test driver for exifdata copy construction and assignment unit tests
source ./functions.source

(   cd "$testdir"

    copyTestFiles exiv2-gc.jpg exiv2-canon-powershot-s40.jpg exiv2-nikon-d70.jpg

    runTest exifdata-test exiv2-gc.jpg                  | grep -v -e UserComment
    runTest exifdata-test exiv2-canon-powershot-s40.jpg | grep -v -e UserComment
    runTest exifdata-test exiv2-nikon-d70.jpg           | grep -v -e UserComment

)  | tr -d '\r' | sed 's/[ \t]+$//' > $results

reportTest

# That's all Folks!
##