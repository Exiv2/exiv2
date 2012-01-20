#! /bin/sh
# Test driver with regression tests for bugfixes
export LC_ALL=C
results="./tmp/bugfixes-test.out"
good="./data/bugfixes-test.out"

prep_file()
{
    echo `prep_any_file $1 exiv2-bug$1.jpg`
}

prep_empty_file()
{
    echo `prep_any_file $1 exiv2-empty.jpg`
}

prep_any_file()
{
    printf "$1 " >&3
    echo '------>' Bug $1 '<-------' >&2
    cp -f ../data/$2 exiv2-bug$1.jpg
    echo exiv2-bug$1.jpg
}

(
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
cd ./tmp

num=426
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=440
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=443
filename=`prep_file $num`
$bin/exiv2 -u -b -pt $filename

num=444
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=445
filename=`prep_file $num`
$bin/exiv2 -u -v -M'set Exif.Photo.UserComment A comment' $filename
$bin/exiv2 -u -b -pt $filename
filename=exiv2-empty.jpg
cp -f ../data/$filename .
$bin/exiv2 -u -v -M'set Exif.Photo.UserComment A comment' $filename
$bin/exiv2 -u -b -pt $filename

num=447 # Problem only visible in Valgrind
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=452
filename=`prep_empty_file $num`
$bin/exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude SRational -1/3 -2/3 -3/3" $filename
$bin/exiv2 -u -pv $filename
$bin/exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude Rational 1/3 2/3 3/3" $filename
$bin/exiv2 -u -pv $filename

num=460
filename=`prep_file $num`
$bin/exiv2 -u -pt $filename

num=479
filename=`prep_file $num`
$bin/exiv2 -u -pt $filename

num=480
filename=`prep_file $num`
$samples/largeiptc-test $filename ../data/imagemagick.png

num=495
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=498
filename=`prep_empty_file $num`
$bin/exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude 0/1 1/1 2/1" $filename
$bin/exiv2 -u -v -pv $filename
$bin/exiv2 -u -v -M"del Exif.GPSInfo.GPSLatitude" $filename
$bin/exiv2 -u -v -pv $filename

num=501
filename=`prep_file $num`
$bin/exiv2 -u -pi $filename

num=528
filename=`prep_file $num`
$bin/exiv2 -u -pt $filename
$bin/exiv2 -u -v -M"set Exif.Image.Software GI" $filename

num=540
filename=`prep_file $num`
$bin/exiv2 -u -px $filename

num=554
filename=`prep_empty_file $num`
$bin/exiv2 -u -v -M"set Exif.Image.DateTime Date 2007-05-27" $filename
$bin/exiv2 -u -pt $filename

num=662
filename=`prep_empty_file $num`

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii An ascii comment" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii A\\nnewline" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A Unicode comment" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\u01c4" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A\\u01c4C" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\nNewline" $filename
$bin/exiv2 -u -PEnh $filename

$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\tTab" $filename
$bin/exiv2 -u -PEnh $filename

# Test invalid escape sequences
$bin/exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\ugggg" $filename
$bin/exiv2 -u -PEnh $filename

num=666
filename=`prep_empty_file $num`

$bin/exiv2 -u -v -M'set Exif.Image.Make NIKON' \
          -M'set Exif.Image.Model D90' \
          -M'set Exif.Nikon3.ShutterCount 100' \
          -M'set Exif.Nikon3.SerialNumber 123' \
          -M'set Exif.NikonSi02xx.Version 48 50 51 52' \
          -M'set Exif.NikonSi02xx.ShutterCount 100' $filename

$bin/exiv2 -u -pa -u -b $filename

num=683
filename=`prep_any_file $num exiv2-nikon-d70.jpg`
rm -f 2004-03-30-Tue-090.jpg
$bin/exiv2 -u -f -r %Y-%m-%d-%a-%j $filename
ls 2004-03-30-Tue-090.jpg

num=711
printf "$num " >&3
# Little endian (II)
filename=exiv2-bug${num}-1.jpg
cp -f ../data/exiv2-empty.jpg $filename
echo '------>' Bug $num '<-------' >&2
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Initial values, read from the command line' \
          -M'set Exif.Image.DocumentName Float 0.12345' \
          -M'set Exif.Image.ImageDescription Double 0.987654321' $filename
$bin/exiv2 -u -v -PEkyct $filename
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Non-intrusive update' $filename
$bin/exiv2 -u -v -PEkyct $filename
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Intrusive update, writing the structure from scratch' $filename
$bin/exiv2 -u -v -PEkyct $filename
# Big endian (MM)
filename=exiv2-bug${num}-2.jpg
cp -f ../data/exiv2-kodak-dc210.jpg $filename
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Initial values, read from the command line' \
          -M'set Exif.Image.DocumentName Float 0.12345' \
          -M'set Exif.Image.ImageDescription Double 0.987654321' $filename
$bin/exiv2 -u -v -PEkyct $filename
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Non-intrusive update' $filename
$bin/exiv2 -u -v -PEkyct $filename
$bin/exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Intrusive update, writing the structure from scratch' $filename
$bin/exiv2 -u -v -PEkyct $filename

# Test easy-access keys (using a dummy bug number)
if { test -f $samples/easyaccess-test || test -f $samples/easyaccess-test.exe; }; then
    num=726
    filename=`prep_empty_file $num`
    $bin/exiv2 -u -v -M"set Exif.Image.Make Samsung" $filename
    $samples/easyaccess-test $filename
else
    echo "bugfixes-test.sh: easyaccess-test executable not found. Skipping regression test for issue #726."
fi

# Test 'migration of XMP namespaces' (see #751 and related forum post)
num=751
filename=`prep_empty_file $num`
xmpname=exiv2-bug$num.xmp
$bin/exiv2 -v -M'reg imageapp orig/' -M 'set Xmp.imageapp.uuid abcd' $filename
$bin/exiv2 -f -eX $filename
cat $xmpname
$bin/exiv2 -v -M'reg imageapp dest/' -M 'set Xmp.imageapp.uuid abcd' $filename
$bin/exiv2 -f -eX $filename
cat $xmpname

num=769
filename=`prep_empty_file $num`
$bin/exiv2 -u -v -M"add Exif.Image.Make Canon" -M"add Exif.CanonCs.0x0001 Short 1" -M"add Exif.CanonCs.0x0000 Short 2" $filename
$bin/exiv2 -u -v -PEkyct $filename

num=799
filename=`prep_empty_file $num`
$bin/exiv2 -v -m ../data/bug$num.cmd $filename
$bin/exiv2 -v -pa $filename
$bin/exiv2 -v -f -eX $filename
cat exiv2-bug$num.xmp

num=800
printf "$num " >&3
for type in 8BIM AgHg DCSR PHUT; do
    for format in jpg psd; do
        echo "------> Bug $num ($type in $format) <-------" >&2
        filename=exiv2-bug$num-$type.$format
        cp -f ../data/$filename .
        $bin/exiv2 -u -v -M'set Exif.Photo.UserComment Test' $filename
        $bin/exiv2 -u -pt $filename
    done
done

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff -u $good $results-stripped
fi
