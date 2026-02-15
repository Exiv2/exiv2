#!/usr/bin/env bash
# Test driver to run the addmoddel sample program
source ./functions.source

(   cd "$testdir"

    copyTestFile      exiv2-empty.jpg
    runTest addmoddel exiv2-empty.jpg
    runTest exiv2 -pv exiv2-empty.jpg

) > $results

reportTest

# That's all Folks!
##