#!/usr/bin/env bash
# Test PNG support (-pS, -pc, -pa -c )

source ./functions.source

(   cd "$testdir"

    files="1343_empty.png 1343_comment.png 1343_exif.png"
    copyTestFiles  $files
    for file in $files ; do
        for  i in 1 2 ; do 
            runTest exiv2 -pR   $file
            runTest exiv2 -pc   $file
            runTest exiv2 -pa   $file
            runTest exiv2 -c  'changed comment' $file
        done
    done
) > $results 2>&1

reportTest

# That's all Folks!
##
