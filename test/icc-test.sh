#!/usr/bin/env bash
# Test driver for exiv2.exe ICC support (-pS, -pC, -eC, -iC)

source ./functions.source

test1120() # --comment and -dc clobbered by writing ICC/JPG
{
    runTest exiv2 --comment abcdefg     $filename
    runTest exiv2 -pS                   $filename
    runTest exiv2 -pc                   $filename
    runTest exiv2 -dc                   $filename
    runTest exiv2 -pS                   $filename
}

(   cd "$testdir"

    num=1074                    # ICC Profile Support
    printf "ICC " >&3
    for filename in Reagan.jpg exiv2-bug1199.webp ReaganLargePng.png ReaganLargeJpg.jpg Reagan2.jp2 # 1272 ReaganLargeTiff.tiff
    do
        format=$(echo $filename|cut -d. -f 2)
        stub=$(  echo $filename|cut -d. -f 1)
        printf $format >&3
        iccname_=$stub.icc

        copyTestFiles  large.icc small.icc  $filename
        runTest exiv2 -pS                   $filename
        runTest exiv2 -pC                   $filename > ${stub}_1.icc
        runTest exiv2 -eC --force           $filename
                mv                          $iccname_   ${stub}_2.icc
        test1120

        copyTestFile          large.icc     $iccname_
        runTest exiv2 -iC                   $filename
        runTest exiv2 -pC                   $filename > ${stub}_large_1.icc
        runTest exiv2 -pS                   $filename
        runTest exiv2 -eC --force           $filename
                mv                          $iccname_   ${stub}_large_2.icc
        test1120

        copyTestFile          small.icc     $iccname_
        runTest exiv2 -iC                   $filename
        runTest exiv2 -pC                   $filename > ${stub}_small_1.icc
        runTest exiv2 -pS                   $filename
        runTest exiv2 -eC --force           $filename
                mv                          $iccname_   ${stub}_small_2.icc
        test1120

        printf " md5 " >&3

        for f in ${stub} ${stub}_small ${stub}_large; do
            for i in 1 2; do
                checkSum ${f}_${i}.icc
            done
        done
    done

) 3>&1 > $results 2>&1

reportTest

# That's all Folks!
##
