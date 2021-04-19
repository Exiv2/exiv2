#!/usr/bin/env bash
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

    for f in Reagan.jpg Reagan.tiff ReaganSmallPng.png; do
        copyTestFile   $f
        runTest exiv2 -pX exiv2-bug1229.jpg | runTest exiv2 -iXX- $f
        runTest exiv2 -pX $f | xmllint --format -
    done

    printf "a (exiv2, xmp, iptc) " >&3
    for f in Reagan.jpg ReaganSmallPng.png; do
        copyTestFile          $f
        copyTestFile  exiv2-bug1229.jpg            girl.jpg
        runTest exiv2 -pS                          girl.jpg
        runTest exiv2 -ea- $f | runTest exiv2 -ia- girl.jpg
        runTest exiv2 -pS                          girl.jpg
    done

    printf "ICC " >&3
    copyTestFile        Reagan.tiff # 1272 ReaganLargeTiff.tiff
    for f in Reagan.jpg ReaganSmallPng.png exiv2-bug1199.webp ; do
        copyTestFile                                        $f
        runTest exiv2 -eC- Reagan.tiff | runTest exiv2 -iC- $f
        runTest exiv2 -pS                                   $f
    done

) 3>&1 > $results 2>&1

printf "\n"

reportTest

# That's all Folks!
##
