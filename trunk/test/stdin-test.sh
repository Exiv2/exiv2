#!/bin/bash
# Test driver for stdin

source ./functions.source

(   cd "$testdir"

    printf "stdin XMP " >&3
    copyTestFile      exiv2-bug1229.jpg # jpg with 2 APP1/xap segments

    copyTestFile      exiv2-bug1229.jpg girl.jpg
    runTest exiv2 -pS girl.jpg
    runTest exiv2 -dX girl.jpg   # remove first
    runTest exiv2 -pS girl.jpg
    runTest exiv2 -pX exiv2-bug1229.jpg | runTest exiv2 -iXX- girl.jpg
    runTest exiv2 -pS girl.jpg

    copyTestFile   exiv2-bug1229.jpg girl.jpg
    runTest exiv2 -dX girl.jpg
    runTest exiv2 -dX girl.jpg
    runTest exiv2 -pS girl.jpg
    runTest exiv2 -pX exiv2-bug1229.jpg | runTest exiv2 -iXX- girl.jpg
    runTest exiv2 -pS girl.jpg

    for f in Reagan.tiff Reagan.tiff ReaganSmallPng.png; do
        copyTestFile   $f
        runTest exiv2 -pX exiv2-bug1229.jpg | runTest exiv2 -iXX- $f
        exiv2 -pX $f | xmllint --format -
    done

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##
