#!/usr/bin/env bash
# Test PNG support (-pS, -pc, -pa -c )

source ./functions.source

(   cd "$testdir"

    files="1343_empty.png 1343_comment.png 1343_exif.png ReaganSmallPng.png"
    copyTestFiles  $files
    for  i in 1 2 ; do 
        runTest exiv2 -pS   $files
        runTest exiv2 -pc   $files
        runTest exiv2 -pa   $files
        runTest exiv2 -c  'changed comment' $files
    done
) > $results 2>&1

reportTest

# That's all Folks!
##
