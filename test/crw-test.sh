#!/usr/bin/env bash
# Test driver for CRW file operations

source ./functions.source
(
    cd "$testdir"
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
        echo  set Exif.Image.DateTime          2020:05:26 07:31:41
        echo  set Exif.Photo.DateTimeDigitized 2020:05:26 07:31:42
    )                            > $cmdfile

    copyTestFile                   $crwfile
    runTest exiv2 -v -pt           $crwfile
    runTest exiv2 -v -m$cmdfile    $crwfile
    runTest exiv2 -v -pt           $crwfile  

    # ----------------------------------------------------------------------
    # Testcases: Delete tags
    copyTestFile                   $crwfile
    runTest exiv2 -v -pt           $crwfile
    runTest exiv2 -v -M'del Exif.Canon.OwnerName'    $crwfile
    runTest exiv2 -v -pt           $crwfile

) 2>&1 | sed -e 's#19:54#18:54#g' > $results   # sed evades TZ issue on MSVC builds #1221

reportTest

# That's all Folks!
##
