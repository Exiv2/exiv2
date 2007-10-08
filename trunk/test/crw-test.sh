#! /bin/sh
# Test driver for CRW file operations

# ----------------------------------------------------------------------
# Setup
results="./tmp/crw-test.out"
good="./data/crw-test.out"
diffargs="--strip-trailing-cr"
tmpfile=tmp/ttt
touch $tmpfile
diff -q $diffargs $tmpfile $tmpfile 2>/dev/null
if [ $? -ne 0 ] ; then
    diffargs=""
fi

# ----------------------------------------------------------------------
# Main routine
(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND exiv2"
cmdfile=cmdfile
crwfile=exiv2-canon-powershot-s40.crw

cd ./tmp

# ----------------------------------------------------------------------
# Testcases: Add and modify tags

cat > $cmdfile <<EOF
set Exif.Photo.ColorSpace 65535
set Exif.Canon.OwnerName Somebody else's Camera
set Exif.Canon.FirmwareVersion Whatever version
set Exif.Canon.SerialNumber 1
add Exif.Canon.SerialNumber 2
set Exif.Photo.ISOSpeedRatings 155
set Exif.Photo.DateTimeOriginal 2007:11:11 09:10:11
EOF

cp -f ../data/$crwfile .
$exiv2 -v -pt $crwfile

$exiv2 -v -m $cmdfile $crwfile
$binpath/crwparse $crwfile

$exiv2 -v -pt $crwfile

# ----------------------------------------------------------------------
# Testcases: Delete tags

cat > $cmdfile <<EOF
del Exif.Canon.OwnerName
EOF

cp -f ../data/$crwfile .
$exiv2 -v -pt $crwfile

$exiv2 -v -m $cmdfile $crwfile
$binpath/crwparse $crwfile

$exiv2 -v -pt $crwfile

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
diff -q $diffargs $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $diffargs $results $good
fi
