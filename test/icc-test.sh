#!/bin/bash
# Test driver for webp

source ./functions.source

test2120() # --comment and -dc clobbered by writing ICC/JPG
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
    for filename in Reagan.jpg exiv2-bug1199.webp ReaganLargePng.png ReaganLargeTiff.tiff ReaganLargeJpg.jpg
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
		test2120

		copyTestFile          large.icc     $iccname_
		runTest exiv2 -iC                   $filename
		runTest exiv2 -pC                   $filename > ${stub}_large_1.icc
		runTest exiv2 -pS                   $filename
		runTest exiv2 -eC --force           $filename
				mv                          $iccname_   ${stub}_large_2.icc
		test2120

		copyTestFile          small.icc     $iccname_
		runTest exiv2 -iC                   $filename
		runTest exiv2 -pC                   $filename > ${stub}_small_1.icc
		runTest exiv2 -pS                   $filename
		runTest exiv2 -eC --force           $filename
				mv                          $iccname_   ${stub}_small_2.icc
		test2120

	    printf " md5 " >&3

		for f in ${stub} ${stub}_small ${stub}_large; do
			for i in 1 2; do
        		checkSum ${f}_${i}.icc
    		done
    	done
    done

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
