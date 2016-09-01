#!/bin/bash
# Test driver for webp

source ./functions.source

(   cd "$testdir"

    num=1074                    # ICC Profile Support
    printf "ICC " >&3
    printf "jpg " >&3
    filename=Reagan.jpg
    iccname_=Reagan.icc

    copyTestFiles big.icc small.icc     $filename
    runTest exiv2 -pS                   $filename
    runTest exiv2 -pC                   $filename > reagan_1.icc
    runTest exiv2 -eC --force           $filename
            mv                          $iccname_   reagan_2.icc

    copyTestFile          big.icc       $iccname_
    runTest exiv2 -iC                   $filename
    runTest exiv2 -pC                   $filename > big_1.icc
    runTest exiv2 -pS                   $filename
    runTest exiv2 -eC --force           $filename
            mv                          $iccname_   big_2.icc


    copyTestFile          small.icc     $iccname_
    runTest exiv2 -iC                   $filename
    runTest exiv2 -pC                   $filename > small_.icc
    runTest exiv2 -pS                   $filename
    runTest exiv2 -eC --force           $filename
            mv                          $iccname_   small_2.icc

	for f in reagan small big; do for i in 1 2; do
      md5 ${f}${i}_.icc
    done ; done

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
