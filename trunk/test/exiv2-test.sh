#! /bin/sh
# Test driver for exiv2 utility tests
(
images="exiv2-empty.jpg \
        exiv2-canon-powershot-s40.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg \
        exiv2-canon-eos-300d.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-sigma-d10.jpg"

for i in $images; do cp -f data/$i tmp/; done
echo "Test directory -----------------------------------------------------------"
cd tmp/ || exit 1;
exiv2="../../src/exiv2"
echo
echo "Exiv2 version ------------------------------------------------------------"
which $exiv2 || exit 2;
$exiv2 -V
echo
echo "Exiv2 help ---------------------------------------------------------------"
$exiv2 -h
echo
echo "Adjust -------------------------------------------------------------------"
$exiv2 -v -a-12:01:01 adjust *.jpg
echo
echo "Rename -------------------------------------------------------------------"
$exiv2 -vf rename *.jpg
echo
echo "Print --------------------------------------------------------------------"
$exiv2 -v print *.jpg
$exiv2 -v -pi print *.jpg
$exiv2 -v -pi print *.jpg > iii
echo
echo "Extract Exif data --------------------------------------------------------"
$exiv2 -vf extract *.jpg
echo
echo "Extract Thumbnail --------------------------------------------------------"
$exiv2 -vf -et extract *.jpg
$exiv2 -v -pi print *.exv > jjj
echo
echo "Compare image data and extracted data ------------------------------------"
diff iii jjj
echo
echo "Delete Thumbnail ---------------------------------------------------------"
$exiv2 -v -dt delete *.jpg
$exiv2 -vf -et extract *.jpg
echo
echo "Delete Exif data ---------------------------------------------------------"
$exiv2 -v delete *.jpg
$exiv2 -v print *.jpg
echo
echo "Insert Exif data ---------------------------------------------------------"
$exiv2 -v insert *.jpg
$exiv2 -v -pi print *.exv > kkk
echo
echo "Compare original and inserted image data ---------------------------------"
diff iii kkk

) > exiv2-test.out 2>&1
