#! /bin/sh
# XMP parser test driver

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ ! -e ../samples/xmpparser-test ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

# ----------------------------------------------------------------------
# Setup
results="./tmp/conversions.out"
good="./data/conversions.out"

# ----------------------------------------------------------------------
# Main routine
(
binpath="$VALGRIND ../../samples"
exiv2="$VALGRIND exiv2"
cd ./tmp

IMG=../data/exiv2-empty.jpg

# 1) Convert Exif ImageDescription to XMP x-default langAlt value
echo Testcase 1
echo ==========
\cp $IMG h.jpg
$exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg
rm -f h.xmp
$exiv2 -eX h.jpg
$exiv2 -px h.xmp
$exiv2 -Pkycv h.xmp
$exiv2 -pi h.xmp

# 2) Convert XMP x-default langAlt value back to Exif ImageDescription
echo
echo Testcase 2
echo ==========
\cp $IMG i.jpg
\cp h.xmp i.xmp
$exiv2 -iX i.jpg
$exiv2 -px i.jpg
$exiv2 -Pkycv i.jpg
$exiv2 -pi i.jpg

# 3) Convert XMP single non-x-default langAlt value to Exif ImageDescription
echo
echo Testcase 3
echo ==========
sed s/x-default/de-DE/ i.xmp > j.xmp
\cp $IMG j.jpg
$exiv2 -iX j.jpg
$exiv2 -px j.jpg
$exiv2 -Pkycv j.jpg
$exiv2 -pi j.jpg

# 4) This shouldn't work: No x-default, more than one language
echo
echo Testcase 4
echo ==========
sed 's,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>,' j.xmp > k.xmp
\cp $IMG k.jpg
$exiv2 -iX k.jpg
$exiv2 -px k.jpg
$exiv2 -Pkycv k.jpg
$exiv2 -pi k.jpg

# 5) Add a default language to the XMP file and convert to Exif and IPTC
echo
echo Testcase 5
echo ==========
\cp k.xmp l.xmp
$exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp
grep x-default l.xmp
\cp $IMG l.jpg
$exiv2 -iX l.jpg
$exiv2 -px l.jpg
$exiv2 -Pkycv l.jpg
$exiv2 -pi l.jpg

# 6) Convert an Exif user comment to XMP
echo
echo Testcase 6
echo ==========
\cp $IMG m.jpg
$exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg
$exiv2 -Pkycv m.jpg
rm -f m.xmp
$exiv2 -eX m.jpg
$exiv2 -px m.xmp
$exiv2 -Pkycv m.xmp
$exiv2 -pi m.xmp

# 7) And back to Exif
echo
echo Testcase 7
echo ==========
\cp $IMG n.jpg
\cp m.xmp n.xmp
$exiv2 -iX n.jpg
$exiv2 -px n.jpg
$exiv2 -Pkycv n.jpg
$exiv2 -pi n.jpg

# 8) Convert IPTC keywords to XMP
echo
echo Testcase 8
echo ==========
\cp $IMG o.jpg
$exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg
$exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg
$exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg
$exiv2 -pi o.jpg
rm -f o.xmp
$exiv2 -eX o.jpg
$exiv2 -px o.xmp
$exiv2 -Pkycv o.xmp
$exiv2 -pi o.xmp

# 9) And back to IPTC
echo
echo Testcase 9
echo ==========
\cp $IMG p.jpg
\cp o.xmp p.xmp
$exiv2 -iX p.jpg
$exiv2 -px p.jpg
$exiv2 -Pkycv p.jpg
$exiv2 -pi p.jpg

# 10) Convert an Exif tag to an XMP text value
echo
echo Testcase 10
echo ===========
\cp $IMG q.jpg
$exiv2 -M'set Exif.Image.Software Exiv2' q.jpg
$exiv2 -Pkycv q.jpg
rm -f q.xmp
$exiv2 -eX q.jpg
$exiv2 -px q.xmp
$exiv2 -Pkycv q.xmp
$exiv2 -pi q.xmp

# 11) And back to Exif
echo
echo Testcase 11
echo ===========
\cp $IMG r.jpg
\cp q.xmp r.xmp
$exiv2 -iX r.jpg
$exiv2 -px r.jpg
$exiv2 -Pkycv r.jpg
$exiv2 -pi r.jpg

# 12) Convert an IPTC dataset to an XMP text value
echo
echo Testcase 12
echo ===========
\cp $IMG s.jpg
$exiv2 -M'set Iptc.Application2.LocationName Kuala Lumpur' s.jpg
$exiv2 -pi s.jpg
rm -f s.xmp
$exiv2 -eX s.jpg
$exiv2 -px s.xmp
$exiv2 -Pkycv s.xmp
$exiv2 -pi s.xmp

# 13) And back to IPTC
echo
echo Testcase 13
echo ===========
\cp $IMG t.jpg
\cp s.xmp t.xmp
$exiv2 -iX t.jpg
$exiv2 -px t.jpg
$exiv2 -Pkycv t.jpg
$exiv2 -pi t.jpg

# 10) Convert a few other tags of interest from Exif/Iptc to XMP
echo
echo Testcase 14
echo ===========
\cp $IMG u.jpg
$exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg
$exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg
$exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg
$exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg
$exiv2 -M'set Exif.Photo.Flash 73' u.jpg
$exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg
$exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg
$exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg
$exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg
$exiv2 -M'set Iptc.Application2.DateCreated 2007-05-09' u.jpg

$exiv2 -Pkycv u.jpg
$exiv2 -pi u.jpg
rm -f u.xmp
$exiv2 -eX u.jpg
$exiv2 -px u.xmp
$exiv2 -Pkycv u.xmp
$exiv2 -pi u.xmp

# 15) And back to Exif/IPTC
echo
echo Testcase 15
echo ===========
\cp $IMG v.jpg
\cp u.xmp v.xmp
$exiv2 -M'set Xmp.tiff.DateTime 2003-12-14T12:01:44Z' v.xmp
$exiv2 -iX v.jpg
$exiv2 -px v.jpg
$exiv2 -Pkycv v.jpg
$exiv2 -pi v.jpg

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
