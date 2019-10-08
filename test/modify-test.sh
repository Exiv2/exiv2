#!/usr/bin/env bash
# Test driver for write unit tests to build Exif metadata from scratch
source ./functions.source

(   cd "$testdir"

    copyTestFiles exiv2-empty.jpg exiv2-gc.jpg modifycmd1.txt modifycmd2.txt
    runTest exiv2 -v -m modifycmd1.txt exiv2-empty.jpg
    runTest exiv2 -v -m modifycmd2.txt exiv2-gc.jpg
    runTest exiv2 -v -pi exiv2-empty.jpg
    runTest exiv2 -v -pt exiv2-empty.jpg exiv2-gc.jpg

) > $results

reportTest

# That's all Folks!
##