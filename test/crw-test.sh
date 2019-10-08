#!/usr/bin/env bash
# Test driver for CRW file operations

source ./functions.source

(   cd "$testdir"

    crwfile=exiv2-canon-powershot-s40.crw

    # ----------------------------------------------------------------------
    # Testcases: Add and modify tags
    cmdfile=cmdfile1
    (   echo  set Exif.Photo.ColorSpace      65535
        echo  set Exif.Canon.OwnerName       Different owner
        echo  set Exif.Canon.FirmwareVersion Whatever version
        echo  set Exif.Canon.SerialNumber    1
        echo  add Exif.Canon.SerialNumber    2
        echo  set Exif.Photo.ISOSpeedRatings 155
        echo  set Exif.Photo.DateTimeOriginal 2007:11:11 09:10:11
    )                            > $cmdfile

    copyTestFile                   $crwfile
    runTest exiv2 -v -pt           $crwfile
    runTest exiv2 -v -m $cmdfile   $crwfile
    # runTest crwparse             $crwfile
    runTest exiv2 -v -pt           $crwfile

    # ----------------------------------------------------------------------
    # Testcases: Delete tags
    cmdfile=cmdfile2
    (   echo del Exif.Canon.OwnerName
    )                            > $cmdfile2

    copyTestFile                   $crwfile
    runTest exiv2 -v -pt           $crwfile
    runTest exiv2 -v -m $cmdfile   $crwfile
    # runTest crwparse             $crwfile
    runTest exiv2 -v -pt           $crwfile

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##
