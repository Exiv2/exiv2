#!/bin/bash
# Test driver for exiv2 utility tests

source ./functions.source
diffargs="-w --text $diffargs"

(   cd "$testdir"

    # Add each image to the following three lists.
    # The image basename in the second and third lists
    # is the Exif timestamp adjusted by -12:01:01.

    images="exiv2-empty.jpg \
        exiv2-canon-powershot-s40.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg \
        exiv2-canon-eos-300d.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-sigma-d10.jpg \
        exiv2-olympus-c8080wz.jpg \
        exiv2-panasonic-dmc-fz5.jpg \
        exiv2-sony-dsc-w7.jpg \
        exiv2-canon-eos-20d.jpg \
        exiv2-canon-eos-d30.jpg \
        exiv2-canon-powershot-a520.jpg"

    image2="exiv2-empty.jpg \
        20031214_000043.jpg \
        20000506_020544.jpg \
        20040329_224245.jpg \
        20010405_235039.jpg \
        20030925_201850.jpg \
        20001026_044550.jpg \
        20030926_111535.jpg \
        20040316_075137.jpg \
        20040208_093744.jpg \
        20050218_212016.jpg \
        20050527_051833.jpg \
        20060802_095200.jpg \
        20001004_015404.jpg \
        20060127_225027.jpg"

    image3="exiv2-empty.exv \
        20031214_000043.exv \
        20000506_020544.exv \
        20040329_224245.exv \
        20010405_235039.exv \
        20030925_201850.exv \
        20001026_044550.exv \
        20030926_111535.exv \
        20040316_075137.exv \
        20040208_093744.exv \
        20050218_212016.exv \
        20050527_051833.exv \
        20060802_095200.exv \
        20001004_015404.exv \
        20060127_225027.exv"

    for i in $images; do copyTestFile $i; done
    echo "Exiv2 test directory -----------------------------------------------------"
    cd "$testdir"

    echo tmp/
    echo
    echo "Exiv2 version ------------------------------------------------------------"
    runTest exiv2 -u -V | sed '1 s, (.. bit build)$, (__ bit build),'
    echo
    echo "Exiv2 help ---------------------------------------------------------------"
    runTest exiv2 -u -h
    echo
    echo "Adjust -------------------------------------------------------------------"
    runTest exiv2 -u -v -a-12:01:01 adjust $images
    echo
    echo "Rename -------------------------------------------------------------------"
    runTest exiv2 -u -vf rename $images
    echo
    echo "Print --------------------------------------------------------------------"
    runTest exiv2 -u -v print $image2
    runTest exiv2 -u -v -b -pt print $image2
    runTest exiv2 -u -v -b -pt print $image2 > iii
    echo
    echo "Extract Exif data --------------------------------------------------------"
    runTest exiv2 -u -vf extract $image2
    echo
    echo "Extract Thumbnail --------------------------------------------------------"
    runTest exiv2 -u -vf -et extract $image2
    runTest exiv2 -u -v -b -pt print $image3 > jjj
    echo
    echo "Compare image data and extracted data ------------------------------------"
    diff $diffargs iii jjj
    echo
    echo "Delete Thumbnail ---------------------------------------------------------"
    runTest exiv2 -u -v -dt delete $image2
    runTest exiv2 -u -vf -et extract $image2
    echo
    echo "Delete Exif data ---------------------------------------------------------"
    runTest exiv2 -u -v delete $image2
    runTest exiv2 -u -v print $image2
    echo
    echo "Insert Exif data ---------------------------------------------------------"
    runTest exiv2 -u -v insert $image2
    runTest exiv2 -u -v -b -pt print $image3 > kkk
    echo
    echo "Compare original and inserted image data ---------------------------------"
    diff $diffargs iii kkk

) > $results 2>&1

reportTest

# That's all Folks!
##
