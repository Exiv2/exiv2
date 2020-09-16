#!/usr/bin/env bash
# XMP parser test driver

# ----------------------------------------------------------------------
# Setup
source ./functions.source

# ----------------------------------------------------------------------
# Check if xmpparser-test exists
if [ $(existsTest xmpparser-test) == 0 ] ; then
    echo "xmpparser-test not found. Assuming XMP support is not enabled."
    exit 0
fi

(   cd "$testdir"

    IMG=exiv2-empty.jpg

    # 1) Convert Exif ImageDescription to XMP x-default langAlt value
    echo Testcase 1
    echo ==========
    copyTestFile $IMG h.jpg
    runTest exiv2 -M'set Exif.Image.ImageDescription The Exif image description' h.jpg
    rm -f h.xmp
    runTest exiv2 -eX h.jpg
    runTest exiv2 -px h.xmp
    runTest exiv2 -PEkycv h.xmp
    runTest exiv2 -pi h.xmp

    # 2) Convert XMP x-default langAlt value back to Exif ImageDescription
    echo
    echo Testcase 2
    echo ==========
    copyTestFile $IMG  i.jpg
    copy         h.xmp i.xmp
    runTest exiv2 -iX  i.jpg
    runTest exiv2 -px  i.jpg
    runTest exiv2 -PEkycv i.jpg
    runTest exiv2 -pi  i.jpg

    # 3) Convert XMP single non-x-default langAlt value to Exif ImageDescription
    echo
    echo Testcase 3
    echo ==========
    sed s/x-default/de-DE/ i.xmp > j.xmp
    copyTestFile $IMG j.jpg
    runTest exiv2 -iX j.jpg
    runTest exiv2 -px j.jpg
    runTest exiv2 -PEkycv j.jpg
    runTest exiv2 -pi j.jpg

    # 4) This shouldn't work: No x-default, more than one language
    echo
    echo Testcase 4
    echo ==========
    sed 's,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li>,<rdf:li xml:lang="de-DE">The Exif image description</rdf:li><rdf:li xml:lang="it-IT">Ciao bella</rdf:li>,' j.xmp > k.xmp
    copyTestFile $IMG k.jpg
    runTest exiv2 -iX k.jpg
    runTest exiv2 -px k.jpg
    runTest exiv2 -v -PEkycv k.jpg
    runTest exiv2 -v -pi k.jpg

    # 5) Add a default language to the XMP file and convert to Exif and IPTC
    echo
    echo Testcase 5
    echo ==========
    copy k.xmp l.xmp
    runTest exiv2 -M'set Xmp.dc.description lang="x-default" How to fix this mess' l.xmp
    grep x-default l.xmp
    copyTestFile $IMG l.jpg
    runTest exiv2 -iX l.jpg
    runTest exiv2 -px -b l.jpg
    runTest exiv2 -PEkycv l.jpg
    runTest exiv2 -pi l.jpg

    # 6) Convert an Exif user comment to XMP
    echo
    echo Testcase 6
    echo ==========
    copyTestFile $IMG m.jpg
    runTest exiv2 -M'set Exif.Photo.UserComment charset=Jis This is a JIS encoded Exif user comment. Or was it?' m.jpg
    runTest exiv2 -PEkycv m.jpg
    rm -f m.xmp
    runTest exiv2 -eX m.jpg
    runTest exiv2 -px m.xmp
    runTest exiv2 -PEkycv m.xmp
    runTest exiv2 -v -pi m.xmp

    # 7) And back to Exif
    echo
    echo Testcase 7
    echo ==========
    copyTestFile $IMG n.jpg
    copy m.xmp n.xmp
    runTest exiv2 -iX n.jpg
    runTest exiv2 -px n.jpg
    runTest exiv2 -PEkycv n.jpg
    runTest exiv2 -v -pi n.jpg

    # 8) Convert IPTC keywords to XMP
    echo
    echo Testcase 8
    echo ==========
    copyTestFile $IMG o.jpg
    runTest exiv2 -M'add Iptc.Application2.Keywords Sex' o.jpg
    runTest exiv2 -M'add Iptc.Application2.Keywords Drugs' o.jpg
    runTest exiv2 -M"add Iptc.Application2.Keywords Rock'n'roll" o.jpg
    runTest exiv2 -pi o.jpg
    rm -f o.xmp
    runTest exiv2 -eX o.jpg
    runTest exiv2 -px o.xmp
    runTest exiv2 -v -PEkycv o.xmp
    runTest exiv2 -pi o.xmp

    # 9) And back to IPTC
    echo
    echo Testcase 9
    echo ==========
    copyTestFile $IMG p.jpg
    copy o.xmp p.xmp
    runTest exiv2 -iX p.jpg
    runTest exiv2 -px p.jpg
    runTest exiv2 -v -PEkycv p.jpg
    runTest exiv2 -pi p.jpg

    # 10) Convert an Exif tag to an XMP text value
    echo
    echo Testcase 10
    echo ===========
    copyTestFile $IMG q.jpg
    runTest exiv2 -M'set Exif.Image.Software Exiv2' q.jpg
    runTest exiv2 -PEkycv q.jpg
    rm -f q.xmp
    runTest exiv2 -eX q.jpg
    runTest exiv2 -px q.xmp
    runTest exiv2 -PEkycv q.xmp
    runTest exiv2 -v -pi q.xmp

    # 11) And back to Exif
    echo
    echo Testcase 11
    echo ===========
    copyTestFile $IMG r.jpg
    copy q.xmp r.xmp
    runTest exiv2 -iX r.jpg
    runTest exiv2 -px r.jpg
    runTest exiv2 -PEkycv r.jpg
    runTest exiv2 -v -pi r.jpg

    # 12) Convert an IPTC dataset to an XMP text value
    echo
    echo Testcase 12
    echo ===========
    copyTestFile $IMG s.jpg
    runTest exiv2 -M'set Iptc.Application2.SubLocation Kuala Lumpur' s.jpg
    runTest exiv2 -pi s.jpg
    rm -f s.xmp
    runTest exiv2 -eX s.jpg
    runTest exiv2 -px s.xmp
    runTest exiv2 -v -PEkycv s.xmp
    runTest exiv2 -pi s.xmp

    # 13) And back to IPTC
    echo
    echo Testcase 13
    echo ===========
    copyTestFile $IMG t.jpg
    copy s.xmp t.xmp
    runTest exiv2 -iX t.jpg
    runTest exiv2 -px t.jpg
    runTest exiv2 -v -PEkycv t.jpg
    runTest exiv2 -pi t.jpg

    # 14) Convert a few other tags of interest from Exif/IPTC to XMP
    echo
    echo Testcase 14
    echo ===========
    copyTestFile $IMG u.jpg
    runTest exiv2 -M'set Exif.Photo.DateTimeOriginal 2003:12:14 12:01:44' u.jpg
    runTest exiv2 -M'set Exif.Photo.SubSecTimeOriginal 999999999' u.jpg
    runTest exiv2 -M'set Exif.Photo.ExifVersion 48 50 50 49' u.jpg
    runTest exiv2 -M'set Exif.Photo.ComponentsConfiguration 1 2 3 0' u.jpg
    runTest exiv2 -M'set Exif.Photo.Flash 73' u.jpg
    runTest exiv2 -M'set Exif.GPSInfo.GPSLatitude 3/1 8/1 29734512/1000000' u.jpg
    runTest exiv2 -M'set Exif.GPSInfo.GPSLatitudeRef N' u.jpg
    runTest exiv2 -M'set Exif.GPSInfo.GPSVersionID 2 2 0 1' u.jpg
    runTest exiv2 -M'set Exif.GPSInfo.GPSTimeStamp 1/1 2/1 999999999/1000000000' u.jpg

    runTest exiv2 -PEkycv u.jpg
    runTest exiv2 -pi u.jpg
    rm -f u.xmp
    runTest exiv2 -eX u.jpg
    runTest exiv2 -px u.xmp
    runTest exiv2 -PEkycv u.xmp
    runTest exiv2 -pi u.xmp

    # 15) And back to Exif/IPTC
    echo
    echo Testcase 15
    echo ===========
    copyTestFile $IMG v.jpg
    copy u.xmp v.xmp
    runTest exiv2 -M'set Xmp.xmp.ModifyDate 2015-04-17T18:10:22Z' v.xmp
    TZ=GMT-8 runTest exiv2 -iX v.jpg
    runTest exiv2 -px v.jpg
    # evade this test on MSVC builds (Issue #485)
    platform=$(runTest exiv2 -vV 2>/dev/null | grep platform=)
    if [ "$platform" == "platform=windows" -o "$platform" == "platform=mingw64" -o "$platform" == "platform=msys" ]; then
        runTest exiv2 -PEkycv v.jpg | sed -E -e 's#1. 1.:10:22#18 02:10:22#g'
    else
        runTest exiv2 -PEkycv v.jpg
    fi
    runTest exiv2 -pi v.jpg

    # 16) https://github.com/Exiv2/exiv2/issues/521
    echo
    echo Testcase 16
    echo ===========
    copyTestFile                             DSC_3079.jpg
    runTest exiv2 -px                        DSC_3079.jpg
    runTest exiv2 -M"del Xmp.mwg-rs.Regions" DSC_3079.jpg
    runTest exiv2 -px                        DSC_3079.jpg

) > $results 2>&1

reportTest

# That's all Folks!
##
