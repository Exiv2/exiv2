#!/usr/bin/env bash
# Test driver for write unit tests to build Exif metadata from scratch

source ./functions.source

(   cd "$testdir"

    copyTestFile        exiv2-empty.jpg
    runTest write2-test exiv2-empty.jpg

) > $results

reportTest

# That's all Folks!
##