#! /bin/sh
# Test driver with regression tests for bugfixes
results="./tmp/bugfixes-test.out"
good="./data/bugfixes-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

prep_file()
{
    gf_num=$1
    gf_filename=exiv2-bug$gf_num.jpg
    echo '------>' Bug $gf_num '<-------' >&2
    cp -f ../data/$gf_filename .
    echo $gf_filename
}

(
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
cd ./tmp

num=426
filename=`prep_file $num`
$binpath/exiv2 -pi $filename

num=440
filename=`prep_file $num`
$binpath/exiv2 -pi $filename

num=443
filename=`prep_file $num`
$binpath/exiv2 -pt $filename

num=444
filename=`prep_file $num`
$binpath/exiv2 -pi $filename

num=445
filename=`prep_file $num`
$binpath/exiv2 -v -M'set Exif.Photo.UserComment A comment' $filename
$binpath/exiv2 -pt $filename
filename=exiv2-empty.jpg
cp -f ../data/$filename .
$binpath/exiv2 -v -M'set Exif.Photo.UserComment A comment' $filename
$binpath/exiv2 -pt $filename

num=447 # Problem only visible in Valgrind
filename=`prep_file $num`
$binpath/exiv2 -pi $filename

) > $results 2>&1

if [ x`which unix2dos.exe` != x ]; then
    unix2dos.exe -q $results
fi
diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
