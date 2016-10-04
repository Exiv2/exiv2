#!/bin/bash
# Test driver for webp

source ./functions.source

(   cd "$testdir"

    num=1199                    # WebPImage
    printf "WebP " >&3
#    verbose=--verbose
    filename=exiv2-bug$num.webp # http://dev.exiv2.org/attachments/download/1033/Stonehenge-with-icc.webp
    icc_name=exiv2-bug$num.icc
    exv_name=exiv2-bug$num.exv
    xmp_name=exiv2-bug$num.xmp

    copyTestFile                        $filename
    runTest exiv2 $verbose -pS          $filename
    runTest exiv2 $verbose -pR          $filename
    runTest exiv2 $verbose -pX          $filename | xmllint --format -
    printf "delete " >&3
    # test deleting metadata
    for option in -dC -de -dx -dCe -dCx -dCxe; do
      copyTestFile                      $filename
      runTest exiv2 $verbose -pS        $filename
      runTest exiv2 $option             $filename
      runTest exiv2 $verbose -pS        $filename
    done

    printf "insert " >&3
    printf "ICC "    >&3
    copyTestFile                        $filename
    exiv2 $verbose -pS                  $filename
    copyTestFile                        Reagan.tiff
    exiv2 $verbose -pC                  Reagan.tiff > $icc_name
    exiv2 $verbose -iC                  $filename
    exiv2 -pS                           $filename

    printf "XMP " >&3
    # copy the XMP from the test file
    copyTestFile                        $filename
    exiv2 $verbose -pX                  $filename   > $xmp_name;
    exiv2 $verbose -ea --force          $filename

    copyTestFile                        $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -iXX                 $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -ix                  $filename

    # copy the XMP from Reagan.tiff to test file
    copyTestFile                        Reagan.tiff
    exiv2 $verbose -pX                  Reagan.tiff > $xmp_name;
    exiv2 $verbose -ea --force          Reagan.tiff
    mv                                  Reagan.exv    $exv_name

    copyTestFile                        $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -iXX                 $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -ix                  $filename

    # copy the XMP from exiv2-bug922.jpg to test file
    copyTestFile                        exiv2-bug922.jpg
    exiv2 $verbose -pX                  exiv2-bug922.jpg > $xmp_name
    exiv2 $verbose -ea --force          exiv2-bug922.jpg 2>/dev/null
    mv                                  exiv2-bug922.exv   $exv_name

    copyTestFile                        $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -ix                  $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -iXX                 $filename
    exiv2 $verbose -pS                  $filename

    printf "EXIF " >&3
    copyTestFile                        exiv2-bug922.jpg $filename
    exiv2 $verbose --force -ea          $filename
    copyTestFile                        $filename
    exiv2 $verbose -pS                  $filename
    exiv2 $verbose -ie                  $filename
    exiv2 $verbose -pS                  $filename

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##
