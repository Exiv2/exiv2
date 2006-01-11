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
LD_LIBRARY_PATH=../../src:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
binpath="$VALGRIND ../../src"
cmdfile=cmdfile
crwfile=CanonRaw.crw

cd ./tmp

# ----------------------------------------------------------------------
# Testcases: Add tags

#   Create an image from scratch with just one tag
#   Add one tag to an existing image
#   Add a non-CIFF tag
#   Add a second tag with the same tag id
#   Are new tags created as directory data if possible?

# ----------------------------------------------------------------------
# Testcases: Modify tags

#   Modify tag value only
#   Change number of components (from directory data to value data)
#   Change value type

# Exif.Canon.FirmwareVersion Ascii
# Exif.Canon.OwnerName       Ascii
# Exif.Canon.ImageType       Ascii
# Exif.Canon.0x0002          Short
# Exif.Canon.CustomFunctions Short
# Exif.Canon.PictureInfo     Short
# Exif.Canon.SerialNumber    Short
# Exif.Canon.ImageNumber     Long

cat > $cmdfile <<EOF
set Exif.Photo.ColorSpace 65535
set Exif.Canon.OwnerName Somebody else's Camera
set Exif.Canon.FirmwareVersion Whatever version
set Exif.Canon.SerialNumber 1
add Exif.Canon.SerialNumber 2
EOF

cp -f ../data/$crwfile .
$binpath/exiv2 -v -pt $crwfile

$binpath/exiv2 -v -m $cmdfile $crwfile
$binpath/crwparse $crwfile

$binpath/exiv2 -v -pt $crwfile

# ----------------------------------------------------------------------
# Testcases: Delete tags

#   Delete one tag
#   Delete one directory completely
#   Delete all


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
