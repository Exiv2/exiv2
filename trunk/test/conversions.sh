#! /bin/sh
# XMP parser test driver

# ----------------------------------------------------------------------
# Setup
export LC_ALL=C
results="./conversions.out"
good="../data/conversions.out"
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
cd ./tmp

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ ! -e ../../samples/xmpparser-test -a ! -e "$EXIV2_BINDIR/xmpparser-test" ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

# ----------------------------------------------------------------------
# Main routine
(
IMG=../data/exiv2-empty.jpg

# 1) Convert Exif ImageDescription to XMP x-default langAlt value
echo Testcase 1
echo ==========
\cp $IMG h.jpg
$bin/exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg
rm -f h.xmp
$bin/exiv2 -eX h.jpg
$bin/exiv2 -px h.xmp
$bin/exiv2 -PEkycv h.xmp
$bin/exiv2 -pi h.xmp

# 2) Convert XMP x-default langAlt value back to Exif ImageDescription
echo
echo Testcase 2
echo ==========
\cp $IMG i.jpg
\cp h.xmp i.xmp
$bin/exiv2 -iX i.jpg
$bin/exiv2 -px i.jpg
$bin/exiv2 -PEkycv i.jpg
$bin/exiv2 -pi i.jpg

# 3) Convert XMP single non-x-default langAlt value to Exif ImageDescription
echo
echo Testcase 3
echo ==========
sed s/x-default/de-DE/ i.xmp > j.xmp
\cp $IMG j.jpg
$bin/exiv2 -iX j.jpg
$bin/exiv2 -px j.jpg
$bin/exiv2 -PEkycv j.jpg
$bin/exiv2 -pi j.jpg

# 4) This shouldn't work: No x-default, more than one language
echo
echo Testcase 4
echo ==========
sed 's,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>,' j.xmp > k.xmp
\cp $IMG k.jpg
$bin/exiv2 -iX k.jpg
$bin/exiv2 -px k.jpg
$bin/exiv2 -v -PEkycv k.jpg
$bin/exiv2 -v -pi k.jpg

# 5) Add a default language to the XMP file and convert to Exif and IPTC
echo
echo Testcase 5
echo ==========
\cp k.xmp l.xmp
$bin/exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp
grep x-default l.xmp
\cp $IMG l.jpg
$bin/exiv2 -iX l.jpg
$bin/exiv2 -px l.jpg
$bin/exiv2 -PEkycv l.jpg
$bin/exiv2 -pi l.jpg

# 6) Convert an Exif user comment to XMP
echo
echo Testcase 6
echo ==========
\cp $IMG m.jpg
$bin/exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg
$bin/exiv2 -PEkycv m.jpg
rm -f m.xmp
$bin/exiv2 -eX m.jpg
$bin/exiv2 -px m.xmp
$bin/exiv2 -PEkycv m.xmp
$bin/exiv2 -v -pi m.xmp

# 7) And back to Exif
echo
echo Testcase 7
echo ==========
\cp $IMG n.jpg
\cp m.xmp n.xmp
$bin/exiv2 -iX n.jpg
$bin/exiv2 -px n.jpg
$bin/exiv2 -PEkycv n.jpg
$bin/exiv2 -v -pi n.jpg

# 8) Convert IPTC keywords to XMP
echo
echo Testcase 8
echo ==========
\cp $IMG o.jpg
$bin/exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg
$bin/exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg
$bin/exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg
$bin/exiv2 -pi o.jpg
rm -f o.xmp
$bin/exiv2 -eX o.jpg
$bin/exiv2 -px o.xmp
$bin/exiv2 -v -PEkycv o.xmp
$bin/exiv2 -pi o.xmp

# 9) And back to IPTC
echo
echo Testcase 9
echo ==========
\cp $IMG p.jpg
\cp o.xmp p.xmp
$bin/exiv2 -iX p.jpg
$bin/exiv2 -px p.jpg
$bin/exiv2 -v -PEkycv p.jpg
$bin/exiv2 -pi p.jpg

# 10) Convert an Exif tag to an XMP text value
echo
echo Testcase 10
echo ===========
\cp $IMG q.jpg
$bin/exiv2 -M'set Exif.Image.Software Exiv2' q.jpg
$bin/exiv2 -PEkycv q.jpg
rm -f q.xmp
$bin/exiv2 -eX q.jpg
$bin/exiv2 -px q.xmp
$bin/exiv2 -PEkycv q.xmp
$bin/exiv2 -v -pi q.xmp

# 11) And back to Exif
echo
echo Testcase 11
echo ===========
\cp $IMG r.jpg
\cp q.xmp r.xmp
$bin/exiv2 -iX r.jpg
$bin/exiv2 -px r.jpg
$bin/exiv2 -PEkycv r.jpg
$bin/exiv2 -v -pi r.jpg

# 12) Convert an IPTC dataset to an XMP text value
echo
echo Testcase 12
echo ===========
\cp $IMG s.jpg
$bin/exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg
$bin/exiv2 -pi s.jpg
rm -f s.xmp
$bin/exiv2 -eX s.jpg
$bin/exiv2 -px s.xmp
$bin/exiv2 -v -PEkycv s.xmp
$bin/exiv2 -pi s.xmp

# 13) And back to IPTC
echo
echo Testcase 13
echo ===========
\cp $IMG t.jpg
\cp s.xmp t.xmp
$bin/exiv2 -iX t.jpg
$bin/exiv2 -px t.jpg
$bin/exiv2 -v -PEkycv t.jpg
$bin/exiv2 -pi t.jpg

# 14) Convert a few other tags of interest from Exif/IPTC to XMP
echo
echo Testcase 14
echo ===========
\cp $IMG u.jpg
$bin/exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg
$bin/exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg
$bin/exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg
$bin/exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg
$bin/exiv2 -M'set Exif.Photo.Flash 73' u.jpg
$bin/exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg
$bin/exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg
$bin/exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg
$bin/exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg
$bin/exiv2 -M'set Iptc.Application2.DateCreated 2007-05-09' u.jpg

$bin/exiv2 -PEkycv u.jpg
$bin/exiv2 -pi u.jpg
rm -f u.xmp
$bin/exiv2 -eX u.jpg
$bin/exiv2 -px u.xmp
$bin/exiv2 -PEkycv u.xmp
$bin/exiv2 -pi u.xmp

# 15) And back to Exif/IPTC
echo
echo Testcase 15
echo ===========
\cp $IMG v.jpg
\cp u.xmp v.xmp
$bin/exiv2 -M'set Xmp.tiff.DateTime 2003-12-14T12:01:44Z' v.xmp
TZ=GMT-8 $bin/exiv2 -iX v.jpg
$bin/exiv2 -px v.jpg
$bin/exiv2 -PEkycv v.jpg
$bin/exiv2 -pi v.jpg

) > $results 2>&1

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' | tr '\033' '$' > $results-stripped
diff -q $results-stripped $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results-stripped $good
fi
