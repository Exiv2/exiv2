#!/bin/bash
# Test driver for webp

source ./functions.source

(   cd "$testdir"

    num=1199                    # WebPImage
    printf "WebP " >&3
    filename=exiv2-bug$num.webp # http://dev.exiv2.org/attachments/download/1033/Stonehenge-with-icc.webp
    icc_name=exiv2-bug$num.icc
    exv_name=exiv2-bug$num.exv
    xmp_name=exiv2-bug$num.xmp

    copyTestFile                        $filename
    runTest exiv2 -pS                   $filename
    runTest exiv2 -pR                   $filename
    runTest exiv2 -pX                   $filename | xmllint --pretty 2 -
    printf "delete " >&3
    # test deleting metadata
    for option in -dC -de -dx -dCe -dCx -dCxe; do
      copyTestFile                      $filename
      runTest exiv2 -pS                 $filename
      runTest exiv2 $option             $filename
      runTest exiv2 -pS                 $filename
    done

    printf "insert " >&3
    printf "ICC "    >&3
    copyTestFile                        $filename
    exiv2 -pS                           $filename
    copyTestFile                        Reagan.tiff
    exiv2 -pC                           Reagan.tiff > $icc_name
    exiv2 -iC                           $filename
    exiv2 -pS                           $filename

    printf "XMP " >&3
    # copy the XMP from the test file
    copyTestFile                        $filename
    exiv2 -pX                           $filename   > $xmp_name;
    exiv2 -ea --force                   $filename

    copyTestFile                        $filename
    exiv2 -pS                           $filename
    exiv2 -iXX                          $filename
    exiv2 -pS                           $filename
    exiv2 -ix                           $filename

    # copy the XMP from Reagan.tiff to test file
    copyTestFile                        Reagan.tiff
    exiv2 -pX                           Reagan.tiff > $xmp_name;
    exiv2 -ea --force                   Reagan.tiff
    mv                                  Reagan.exv    $exv_name

    copyTestFile                        $filename
    exiv2 -pS                           $filename
    exiv2 -iXX                          $filename
    exiv2 -pS                           $filename
    exiv2 -ix                           $filename

    # copy the XMP from exiv2-bug937.jpg to test file
    copyTestFile                        exiv2-bug937.jpg
    exiv2 -pX                           exiv2-bug937.jpg > $xmp_name
    exiv2 -ea --force                   exiv2-bug937.jpg 2>/dev/null
    mv                                  exiv2-bug937.exv   $exv_name

    copyTestFile                        $filename
    exiv2 -pS                           $filename
    exiv2 -ix                           $filename
    exiv2 -pS                           $filename
    exiv2 -iXX                          $filename
    exiv2 -pS                           $filename

    printf "EXIF " >&3
    copyTestFile                        exiv2-bug937.jpg $filename
    exiv2 --force -ea                   $filename
    copyTestFile                        $filename
    exiv2 -pS                           $filename
    exiv2 -ie                           $filename
    exiv2 -pS                           $filename

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
