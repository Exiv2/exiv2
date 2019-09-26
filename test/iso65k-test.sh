#!/usr/bin/env bash
# test for ISOs which follow Annex G of EXIF 2.3 spec, i.e. ISOs,
# which cannot be represented by Exif.Photo.ISOSpeedRatings due to
# being larger than 65k

# All tests use the summary view as a result-check, because it uses
# the isoSpeed() "easyaccess" function, which handles the higher ISO
# readout.
#
# Tests getting executed:
#

source ./functions.source

(   cd "$testdir"

    num=0001
    # Checks for old way of ISO readout based on the 16bit value
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to something <65k
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 60001'            $filename
    runTest exiv2 -ps                                                 $filename

    num=0002
    # Old ISO is read out first, so if it doesn't indicate that
    # some higher ISO is used, the 16bit value should be returned,
    # ignoring the other tags (for now)
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to something <65k
    # - Exif.Photo.SensitivityType being set to "REI"
    # - Exif.Photo.RecommendedExposureIndex being set to != ISOSpeedRatings
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 60002'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -M'set Exif.Photo.RecommendedExposureIndex 444444'  $filename
    runTest exiv2 -ps                                                 $filename

    num=0003
    # Corner case check (highest ISO value not indicating possible
    # 16bit overflow in ISO)
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65534
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65534'            $filename
    runTest exiv2 -ps                                                 $filename

    num=0004
    # Corner case check (ISO value indicating possible overflow,
    # but no additional informations available)
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType NOT SET
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -ps                                                 $filename

    num=0005
    # possible ISO value overflow, but additional information not valid
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType being set to 0
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 0'                $filename
    runTest exiv2 -ps                                                 $filename

    num=0006
    # possible ISO value overflow, but additional information not valid
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType being set to 8
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 8'                $filename
    runTest exiv2 -ps                                                 $filename

    num=0007
    # possible ISO value overflow, but additional information partially valid
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType being set to 2 ("REI")
    # - Exif.Photo.RecommendedExposureIndex NOT SET
    # output:
    # - value of Exif.Photo.ISOSpeedRatings
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -ps                                                 $filename

    num=0008
    # ISO value overflow, REI contains same value as 16bit ISO, though
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType being set to 2 ("REI")
    # - Exif.Photo.RecommendedExposureIndex set to 65530
    # output:
    # - value of Exif.Photo.RecommendedExposureIndex
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -M'set Exif.Photo.RecommendedExposureIndex 65530'   $filename
    runTest exiv2 -ps                                                 $filename

    num=0009
    # ISO value overflow, REI contains 16bit ISO value +1
    # input:
    # - Exif.Photo.ISOSpeedRatings being set to 65535
    # - Exif.Photo.SensitivityType being set to 2 ("REI")
    # - Exif.Photo.RecommendedExposureIndex set to 65536
    # output:
    # - value of Exif.Photo.RecommendedExposureIndex
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.ISOSpeedRatings 65535'            $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -M'set Exif.Photo.RecommendedExposureIndex 65536'   $filename
    runTest exiv2 -ps                                                 $filename

    num=0010
    # old ISO not set
    # input:
    # - Exif.Photo.ISOSpeedRatings is NOT SET
    # - Exif.Photo.SensitivityType being set to 2 ("REI")
    # - Exif.Photo.RecommendedExposureIndex set to <65k
    # output:
    # - value of Exif.Photo.RecommendedExposureIndex
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -M'set Exif.Photo.RecommendedExposureIndex 60010'   $filename
    runTest exiv2 -ps                                                 $filename

    num=0011
    # old ISO not set
    # input:
    # - Exif.Photo.ISOSpeedRatings is NOT SET
    # - Exif.Photo.SensitivityType being set to 2 ("REI")
    # - Exif.Photo.RecommendedExposureIndex set to >65k
    # output:
    # - value of Exif.Photo.RecommendedExposureIndex
    echo '------>' iso65k test $num '<-------' >&2
    filename=exiv2-iso65k-$num.jpg
    copyTestFile exiv2-empty.jpg $filename
    runTest exiv2 -M'set Exif.Photo.SensitivityType 2'                $filename
    runTest exiv2 -M'set Exif.Photo.RecommendedExposureIndex 100011'  $filename
    runTest exiv2 -ps                                                 $filename

) > $testdir/iso65k-test.out 2>&1

reportTest
