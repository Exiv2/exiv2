#! /bin/sh
# Test driver for exiv2 utility tests
export LC_ALL=C
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
results="./tmp/exiv2-test.out"
good="./data/exiv2-test.out"
tmpfile=tmp/ttt
touch $tmpfile
da1="--strip-trailing-cr"
diff -q $da1 $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    da1=""
fi
da2="--ignore-matching-lines=^Usage:.*exiv2"
diff -q $da2 $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    da2=""
fi
diffargs="$da1 $da2"

(
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

for i in $images; do cp -f data/$i tmp/; done
echo "Exiv2 test directory -----------------------------------------------------"
cd tmp/ >/dev/null || exit 1;
echo tmp/
echo
echo "Exiv2 version ------------------------------------------------------------"
$bin/exiv2 -u -V | sed '1 s, (.. bit build)$, (__ bit build),'
echo
echo "Exiv2 help ---------------------------------------------------------------"
$bin/exiv2 -u -h
echo
echo "Adjust -------------------------------------------------------------------"
$bin/exiv2 -u -v -a-12:01:01 adjust $images
echo
echo "Rename -------------------------------------------------------------------"
$bin/exiv2 -u -vf rename $images
echo
echo "Print --------------------------------------------------------------------"
$bin/exiv2 -u -v print $image2
$bin/exiv2 -u -v -b -pt print $image2
$bin/exiv2 -u -v -b -pt print $image2 > iii
echo
echo "Extract Exif data --------------------------------------------------------"
$bin/exiv2 -u -vf extract $image2
echo
echo "Extract Thumbnail --------------------------------------------------------"
$bin/exiv2 -u -vf -et extract $image2
$bin/exiv2 -u -v -b -pt print $image3 > jjj
echo
echo "Compare image data and extracted data ------------------------------------"
diff iii jjj
echo
echo "Delete Thumbnail ---------------------------------------------------------"
$bin/exiv2 -u -v -dt delete $image2
$bin/exiv2 -u -vf -et extract $image2
echo
echo "Delete Exif data ---------------------------------------------------------"
$bin/exiv2 -u -v delete $image2
$bin/exiv2 -u -v print $image2
echo
echo "Insert Exif data ---------------------------------------------------------"
$bin/exiv2 -u -v insert $image2
$bin/exiv2 -u -v -b -pt print $image3 > kkk
echo
echo "Compare original and inserted image data ---------------------------------"
diff iii kkk

) > $results 2>&1

if [ `../config/config.guess` = "i686-pc-mingw32" ] ; then
    sed 's,\\,/,g' $results > ${results}-new
    mv -f ${results}-new $results
    unix2dos -q $results
fi

diff -q -w $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff -w $diffargs $results $good
fi
