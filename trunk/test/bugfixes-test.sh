#! /bin/sh
# Test driver with regression tests for bugfixes
results="./tmp/bugfixes-test.out"
good="./data/bugfixes-test.out"

prep_file()
{
    gf_num=$1
    gf_filename=exiv2-bug$gf_num.jpg
    echo '------>' Bug $gf_num '<-------' >&2
    cp -f ../data/$gf_filename .
    echo $gf_filename
}

(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND exiv2"
cd ./tmp

num=426
filename=`prep_file $num`
$exiv2 -pi $filename

num=440
filename=`prep_file $num`
$exiv2 -pi $filename

num=443
filename=`prep_file $num`
$exiv2 -b -pt $filename

num=444
filename=`prep_file $num`
$exiv2 -pi $filename

num=445
filename=`prep_file $num`
$exiv2 -v -M'set Exif.Photo.UserComment A comment' $filename
$exiv2 -b -pt $filename
filename=exiv2-empty.jpg
cp -f ../data/$filename .
$exiv2 -v -M'set Exif.Photo.UserComment A comment' $filename
$exiv2 -b -pt $filename

num=447 # Problem only visible in Valgrind
filename=`prep_file $num`
$exiv2 -pi $filename

num=452
filename=exiv2-bug$num.jpg
cp -f ../data/exiv2-empty.jpg $filename
$exiv2 -v -M"set Exif.GPSInfo.GPSLatitude SRational -1/3 -2/3 -3/3" $filename
$exiv2 -pv $filename
$exiv2 -v -M"set Exif.GPSInfo.GPSLatitude Rational 1/3 2/3 3/3" $filename
$exiv2 -pv $filename

num=460
filename=`prep_file $num`
$exiv2 -pt $filename

num=479
filename=`prep_file $num`
$exiv2 -pt $filename

num=480
filename=`prep_file $num`
$binpath/largeiptc-test $filename ../data/imagemagick.png

num=495
filename=`prep_file $num`
$exiv2 -pi $filename

num=498
filename=exiv2-bug$num.jpg
cp -f ../data/exiv2-empty.jpg $filename
$exiv2 -v -M"set Exif.GPSInfo.GPSLatitude 0/1 1/1 2/1" $filename
$exiv2 -v -pv $filename
$exiv2 -v -M"del Exif.GPSInfo.GPSLatitude" $filename
$exiv2 -v -pv $filename

num=501
filename=`prep_file $num`
$exiv2 -pi $filename

num=528
filename=`prep_file $num`
$exiv2 -pt $filename
$exiv2 -v -M"set Exif.Image.Software GI" $filename

num=540
filename=`prep_file $num`
$exiv2 -px $filename

num=554
filename=exiv2-bug$num.jpg
cp -f ../data/exiv2-empty.jpg $filename
$exiv2 -v -M"set Exif.Image.DateTime Date 2007-05-27" $filename
$exiv2 -pt $filename

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d '\r' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results-stripped $good
fi
