#!/bin/bash
# Test driver with regression tests for bugfixes

source ./functions.source

(   cd "$testdir"

    num=426
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=440
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=443
    filename=`prep_file $num`
    runTest exiv2 -u -b -pt $filename

    num=444
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=445
    filename=`prep_file $num`
    runTest exiv2 -u -v -M'set Exif.Photo.UserComment A comment' $filename
    runTest exiv2 -u -b -pt $filename
    filename=exiv2-empty.jpg
    copyTestFile $filename
    runTest exiv2 -u -v -M'set Exif.Photo.UserComment A comment' $filename
    runTest exiv2 -u -b -pt $filename

    num=447 # Problem only visible in Valgrind
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=452
    filename=`prep_empty_file $num`
    runTest exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude SRational -1/3 -2/3 -3/3" $filename
    runTest exiv2 -u -pv $filename
    runTest exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude Rational 1/3 2/3 3/3" $filename
    runTest exiv2 -u -pv $filename

    num=460
    filename=`prep_file $num`
    runTest exiv2 -u -pt $filename

    num=479
    filename=`prep_file $num`
    runTest exiv2 -u -pt $filename

    num=480
    filename=`prep_file $num`
    runTest largeiptc-test $filename $datadir/imagemagick.png

    num=495
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=498
    filename=`prep_empty_file $num`
    runTest exiv2 -u -v -M"set Exif.GPSInfo.GPSLatitude 0/1 1/1 2/1" $filename
    runTest exiv2 -u -v -pv $filename
    runTest exiv2 -u -v -M"del Exif.GPSInfo.GPSLatitude" $filename
    runTest exiv2 -u -v -pv $filename

    num=501
    filename=`prep_file $num`
    runTest exiv2 -u -pi $filename

    num=528
    filename=`prep_file $num`
    runTest exiv2 -u -pt $filename
    runTest exiv2 -u -v -M"set Exif.Image.Software GI" $filename

    num=540
    filename=`prep_file $num`
    runTest exiv2 -u -px $filename

    num=554
    filename=`prep_empty_file $num`
    runTest exiv2 -u -v -M"set Exif.Image.DateTime Date 2007-05-27" $filename
    runTest exiv2 -u -pt $filename

    num=662
    filename=`prep_empty_file $num`

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii An ascii comment" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Ascii A\\nnewline" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A Unicode comment" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\u01c4" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode A\\u01c4C" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\nNewline" $filename
    runTest exiv2 -u -PEnh $filename

    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode With\\tTab" $filename
    runTest exiv2 -u -PEnh $filename

    # Test invalid escape sequences
    runTest exiv2 -u -M"set Exif.Photo.UserComment charset=Unicode \\ugggg" $filename
    runTest exiv2 -u -PEnh $filename

    num=666
    filename=`prep_empty_file $num`

    runTest exiv2 -u -v -M'set Exif.Image.Make NIKON' \
          -M'set Exif.Image.Model D90' \
          -M'set Exif.Nikon3.ShutterCount 100' \
          -M'set Exif.Nikon3.SerialNumber 123' \
          -M'set Exif.NikonSi02xx.Version 48 50 51 52' \
          -M'set Exif.NikonSi02xx.ShutterCount 100' $filename

    runTest exiv2 -u -pa -u -b $filename

    num=683
    filename=`prep_any_file $num exiv2-nikon-d70.jpg`
    rm -f 2004-03-30-Tue-090.jpg
    runTest exiv2 -u -f -r %Y-%m-%d-%a-%j $filename
    ls 2004-03-30-Tue-090.jpg

    num=711
    printf "$num " >&3
    # Little endian (II)
    filename=exiv2-bug${num}-1.jpg
    copyTestFile exiv2-empty.jpg $filename
    echo '------>' Bug $num '<-------' >&2
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Initial values, read from the command line' \
              -M'set Exif.Image.DocumentName Float 0.12345' \
              -M'set Exif.Image.ImageDescription Double 0.987654321' $filename
    runTest exiv2 -u -v -PEkyct $filename
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Non-intrusive update' $filename
    runTest exiv2 -u -v -PEkyct $filename
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Intrusive update, writing the structure from scratch' $filename
    runTest exiv2 -u -v -PEkyct $filename
    # Big endian (MM)
    filename=exiv2-bug${num}-2.jpg
    copyTestFile  exiv2-kodak-dc210.jpg $filename
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Initial values, read from the command line' \
              -M'set Exif.Image.DocumentName Float 0.12345' \
              -M'set Exif.Image.ImageDescription Double 0.987654321' $filename
    runTest exiv2 -u -v -PEkyct $filename
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Non-intrusive update' $filename
    runTest exiv2 -u -v -PEkyct $filename
    runTest exiv2 -u -v -M'set Exif.Image.ProcessingSoftware Intrusive update, writing the structure from scratch' $filename
    runTest exiv2 -u -v -PEkyct $filename

    # Test easy-access keys (using a dummy bug number)
    if [ 1 = $(existsTest easyaccess-test) ]; then
        num=726
        filename=`prep_empty_file $num`
        runTest exiv2 -u -v -M"set Exif.Image.Make Samsung" $filename
        runTest easyaccess-test $filename
    else
        echo "bugfixes-test.sh: easyaccess-test executable not found. Skipping regression test for issue #726."
    fi

    # Test 'migration of XMP namespaces' (see #751 and related forum post)
    num=751
    filename=`prep_empty_file $num`
    xmpname=exiv2-bug$num.xmp
    runTest exiv2 -v -M'reg imageapp orig/' -M 'set Xmp.imageapp.uuid abcd' $filename
    runTest exiv2 -f -eX $filename
    cat $xmpname
    runTest exiv2 -v -M'reg imageapp dest/' -M 'set Xmp.imageapp.uuid abcd' $filename
    runTest exiv2 -f -eX $filename
    cat $xmpname

    num=769
    filename=`prep_empty_file $num`
    runTest exiv2 -u -v -M"add Exif.Image.Make Canon" -M"add Exif.CanonCs.0x0001 Short 1" -M"add Exif.CanonCs.0x0000 Short 2" $filename
    runTest exiv2 -u -v -PEkyct $filename

    num=784
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -px $filename
    runTest exiv2 -M"add Xmp.lr.hierarchicalSubject root|1st|2nd|3rd|4th|5th" $filename
    runTest exiv2 -px $filename

    num=799
    filename=`prep_empty_file $num`
    copyTestFile         bug$num.cmd
    runTest exiv2 -v -m  bug$num.cmd $filename
    runTest exiv2 -v -pa $filename
    runTest exiv2 -f -eX $filename
    cat exiv2-bug$num.xmp

    num=800
    printf "$num " >&3
    for type in 8BIM AgHg DCSR PHUT; do
        for format in jpg psd; do
            echo "------> Bug $num ($type in $format) <-------" >&2
            filename=exiv2-bug$num-$type.$format
            copyTestFile $filename
            runTest exiv2 -u -v -M'set Exif.Photo.UserComment Test' $filename
            runTest exiv2 -u -pt $filename
        done
    done

    num=812 # updating a hardlinked file can "empty" the other files!
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile exiv2-bug884c.jpg bug$num.jpg
    hardLinkFiles                  bug$num.jpg bug$num-B.jpg bug$num-C.jpg
    runTest exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num" bug$num.jpg
    runTest exiv2 -PE -g UserComment bug${num}*.jpg
    runTest exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num modified" bug$num.jpg
    runTest exiv2 -PE -g UserComment bug${num}*.jpg

    num=816 # test Camera: Pentax + Lens:Sigma 55-200mm F4-5.6 DC is correctly reported
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    for X in a b c d e; do
        filename=exiv2-bug${num}${X}.exv
        copyTestFile $filename
        runTest exiv2 -pa --grep Lens $filename
    done

    num=825 # Panasonic Manometer Tag
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    for X in a b; do
        filename=exiv2-bug${num}${X}.exv
        copyTestFile $filename
        runTest exiv2 -pv --grep mano/i $filename
        runTest exiv2 -pa --grep mano/i $filename
    done

    num=831
    filename=exiv2-bug$num.tif
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile mini9.tif $filename
    runTest exiv2 -v -Qd -M'set Exif.Image.ImageDescription Just GIMP' $filename
    runTest exiv2 -v -pa $filename

    num=836
    filename=exiv2-bug$num.eps
    echo '------>' Bug $num '<-------' >&2
    copyTestFile $filename $filename
    if [ -e $filename/rsrc ]; then
        printf "$num " >&3
        copyTestFile $filename.rsrc $filename/rsrc
        runTest exiv2 -M'set Exif.Photo.UserComment Test' $filename
        diff -q $datadir/$filename.rsrc $filename/rsrc
    else
        # skip this test on systems which do not have resource forks
        printf "($num skipped) " >&3
    fi

    num=841
    filename=exiv2-bug$num.png
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile $filename
    runTest exiv2 $filename

    num=855
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile              $filename
    runTest exiv2 -pa         $filename

    num=876
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -pa $filename | grep Model

    num=884
    filenames="exiv2-bug${num}a.jpg exiv2-bug${num}b.jpg exiv2-bug${num}c.jpg"
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    for filename in $filenames; do
      copyTestFile  $filename
      runTest exiv2 -pt $filename | grep -i lenstype
    done

    num=922
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile      $filename
    runTest exiv2 -pX $filename ; echo '' # add a lf after the XMP/xml
    filename=iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg
    copyTestFile      $filename
    runTest exiv2 -pX $filename
    for filename in exiv2-bug$num.png exiv2-bug$num.tif exiv2-bug${num}a.jpg; do
        copyTestFile $filename ;
        runTest exiv2 -pX $filename ; echo ''
        runTest exiv2 -pS $filename
    done

    num=935
    filename=`prep_empty_file $num`
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime 605/10" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime 2/3" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime 0/0" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime 605/605" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime 100/1500" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -v -M"set Exif.Photo.ExposureTime Ascii Test" $filename
    runTest exiv2 -u -pv -gExif.Photo.ExposureTime $filename
    runTest exiv2 -u -pa -gExif.Photo.ExposureTime $filename

    num=937
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -q -pa      -g dwc  $filename
    runTest exiv2 -q -PXkyctl -g Date $filename

    num=937a
    filename=exiv2.dc.dwc.i18n.jpg
    dataname=exiv2.dc.dwc.i18n.txt
    diffname=exiv2.dc.dwc.i18n.diff
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile         $filename
    copyTestFile         $dataname
    copyTestFile         $diffname
    runTest exiv2 -pa    $filename | sort            > $num-before.txt
    runTest exiv2 -m     $dataname  $filename
    runTest exiv2 -pa    $filename | sort            > $num-after.txt
    diff $diffargs $num-before.txt $num-after.txt    > $num.txt
    diff $diffargs $num.txt        $diffname

    num=1024
    filename=exiv2-bug$num.exv
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -pa --grep gpsl/i $filename

    num=1026
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -q -pa -g Lens $filename

    num=1040
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -px $filename

    num=1043  # looping and writing to a samba drive can hang!
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    for n in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z; do
        copyTestFile exiv2-bug884c.jpg bug${num}-$n.jpg
    done
    for name in bug${num}-*.jpg; do
        runTest exiv2 -u -v -M"set Exif.Photo.UserComment Test Bug $num my filename is $name" $name
    done
    runTest exiv2 -PE -g UserComment bug${num}*.jpg

    num=1044
    filename=exiv2-bug$num.tif
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile  $filename
    runTest exiv2 -q -pa -g PageNumber $filename

    num=1053
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile exiv2-bug884c.jpg $filename
    runTest exiv2 -PE -g ImageWidth            $filename
    runTest exiv2 -PE -K ImageWidth            $filename
    runTest exiv2 -PE -K Exif.Image.ImageWidth $filename

    num=1054
    filename1=exiv2-bug${num}-1.jpg
    filename2=exiv2-bug${num}-2.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile BlueSquare.xmp   $filename1
    copyTestFile exiv2-bug784.jpg $filename2
    runTest exiv2json        $filename1
    runTest exiv2json x      $filename1
    runTest exiv2json        $filename2

    num=1058
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile exiv2-empty.jpg $filename
    # Add titles in 2 languages and one default
    runTest exiv2 -M'set Xmp.dc.title lang="de-DE" GERMAN'  $filename
    runTest exiv2 -M'set Xmp.dc.title lang="en-GB" BRITISH' $filename
    runTest exiv2 -M'set Xmp.dc.title Everybody else'       $filename
    runTest exiv2 -px                                       $filename
    # Remove languages, test case for the language
    runTest exiv2 -M'set Xmp.dc.title lang="DE-de" german'  $filename
    runTest exiv2 -M'set Xmp.dc.title lang="EN-gb"'         $filename
    runTest exiv2 -M'set Xmp.dc.title'                      $filename
    runTest exiv2 -px                                       $filename

    num=1062
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile              $filename
    runTest exiv2 -pa -g zone $filename

    num=1080
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile              $filename
    runTest exiv2 -pa         $filename

    num=1074
    filename=exiv2-bug$num.png
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile      $filename
    runTest exiv2 -pC $filename       > ${num}_1.icc ; checkSum ${num}_1.icc
    copyTestFile      imagemagick.png
    runTest exiv2 -pC imagemagick.png > ${num}_2.icc ; checkSum ${num}_2.icc
    copyTestFile      Reagan.tiff
    runTest exiv2 -pC Reagan.tiff     > ${num}_3.icc ; checkSum ${num}_3.icc
    copyTestFile      Reagan.jpg
    runTest exiv2 -pC Reagan.jpg      > ${num}_4.icc ; checkSum ${num}_4.icc

    num=1108
    filename=exiv2-bug$num.exv
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pR                   $filename
    copyTestFile      imagemagick.png
    runTest exiv2 -pR imagemagick.png
    copyTestFile      ReaganLargeTiff.tiff
    runTest exiv2 -pR ReaganLargeTiff.tiff
    copyTestFile      Reagan.jpg
    runTest exiv2 -pR Reagan.jpg
    echo ''

    num=1112
    filename=exiv2-bug$num.xmp
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -M 'del Xmp.dc.title' $filename
    cat                                 $filename
    echo ''

    num=1114
    filename=exiv2-bug$num.jpg
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pv -g TuneAdj        $filename

    num=1122
    filename=exiv2-bug$num.exv
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pv -g Lens           $filename
    runTest exiv2 -pa -g Lens           $filename

    num=1137
    filename=exiv2-bug$num.exv
    printf "$num " >&3
    echo '------>' Bug $num '<-------' >&2
    copyTestFile       Stonehenge.exv   $filename
    copyTestFile       Stonehenge.exv
    runTest exiv2 -PkV --grep GPSL      $filename
    runTest exiv2 -PkV --grep GPSL      Stonehenge.exv | runTest exiv2 -m- $filename
    runTest exiv2 -pa  --grep GPSL      $filename

    num=1140
    printf "$num " >&3
    filename=exiv2-bug$num.exv
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pa -g Lens           $filename

    num=1144
    printf "$num " >&3
    for i in a b c d e f g; do
      filename=exiv2-bug$num$i.exv
      echo '------>' Bug $num$i '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa -g Lens         $filename
    done

    num=1145
    printf "$num " >&3
    for i in a b c d e; do
      filename=exiv2-bug$num$i.exv
      echo '------>' Bug $num$i '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa -g Lens         $filename
    done

    num=1153
    printf "$num " >&3
    for E in A J; do for i in a b c d e f g h i j k; do
      filename=exiv2-bug$num${E}${i}.exv
      echo '------>' Bug $num${E}${i} '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa -g Lens         $filename
    done;done

    num=1155
    printf "$num " >&3
    for i in a b c d e f g h i; do
      filename=exiv2-bug$num${i}.exv
      echo '------>' Bug $num${i} '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa -g Lens         $filename
    done

    num=1166
    printf "$num " >&3
    filename=exiv2-bug$num.exv
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pa --grep lens/i     $filename

    num=1167
    printf "$num " >&3
    filename=exiv2-bug$num.exv
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pa --grep lens/i     $filename

    num=1170
    printf "$num " >&3
    filename=exiv2-bug$num.exv
    echo '------>' Bug $num '<-------' >&2
    copyTestFile                        $filename
    runTest exiv2 -pa --grep lens/i     $filename

    num=1179
    printf "$num " >&3
    for X in a b c d e f g h i j; do
      filename=exiv2-bug$num$X.exv
      echo '------>' Bug $filename '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa --grep fuji/i   $filename
    done

    num=1202
    printf "$num " >&3
    filename=exiv2-bug$num.jpg # test/tmp/20030925_201850.jpg
    copyTestFile                        $filename
    for value in 0 1 8 9 -1; do
      runTest exiv2 -M"set Exif.CanonCs.FocusContinuous SShort $value" $filename
      runTest exiv2 -K Exif.CanonCs.FocusContinuous $filename
    done

    num=1223
    printf "$num " >&3
    filename=exiv2-bug$num.jpg
    copyTestFile   RAW_PENTAX_K30.exv
    copyTestFile   RAW_PENTAX_K100.exv
    runTest exiv2 -pa --grep Shutter    RAW_PENTAX_K30.exv
    runTest exiv2 -pa --grep Shutter    RAW_PENTAX_K100.exv

    num=1225
    printf "$num " >&3
    filename=exiv2-bug$num.exv
    copyTestFile                        $filename
    runTest exiv2 -pa --grep Bar        $filename

    num=1229
    printf "$num " >&3
    filename=exiv2-bug$num.jpg
    copyTestFile                        $filename
    runTest exiv2 -pX                   $filename | xmllint --format -

    num=1231
    printf "$num " >&3
    for X in a b; do
      filename=exiv2-bug$num$X.jpg
      echo '------>' Bug $filename '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa --grep CanonTi/i   $filename
    done

    num=1242
    printf "$num " >&3
    filename=Reagan.jp2
    copyTestFile                        $filename
    runTest exiv2 -pa                   $filename

    num=1247
    printf "$num " >&3
    filename=exiv2-bug$num.jpg
    copyTestFile                        $filename
    runTest exiv2 -pa                   $filename

    num=1252
    printf "$num " >&3
    for X in a b; do
      filename=exiv2-bug$num$X.exv
      echo '------>' Bug $filename '<-------' >&2
      copyTestFile                      $filename
      runTest exiv2 -pa --grep lens/i   $filename
    done

    num=g20
    printf "$num " >&3
    filename=exiv2-$num.exv
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2 -pa --grep lens/i   $filename
    for filename in CanonEF100mmF2.8LMacroISUSM.exv      \
                    TamronSP15-30mmF2.8DiVCUSDA012.exv   \
                    TamronSP90mmF2.8DiVCUSDMacroF004.exv \
                    TamronSP90mmF2.8DiVCUSDMacroF017.exv
    do
      copyTestFile                    $filename
      runTest exiv2 -pa --grep lenstype/i  $filename
    done

    num=g45
    printf "$num " >&3
    filename=exiv2-$num.exv
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2 -pa --grep lens/i   $filename

    num=g55
    printf "$num " >&3
    filename=POC8
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename 2>/dev/null

    num=g57
    printf "$num " >&3
    filename=POC
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g79
    printf "$num " >&3
    filename=POC2
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g52
    printf "$num " >&3
    filename=POC5
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g51
    printf "$num " >&3
    filename=POC4
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g50
    printf "$num " >&3
    filename=POC3
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g53
    printf "$num " >&3
    filename=POC6
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g56
    printf "$num " >&3
    filename=POC9
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g58
    printf "$num " >&3
    filename=POC11
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g59
    printf "$num " >&3
    filename=POC12
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g60
    printf "$num " >&3
    filename=POC13
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g71
    printf "$num " >&3
    filename=003-heap-buffer-over
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g73
    printf "$num " >&3
    filename=02-Invalid-mem-def
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g74
    printf "$num " >&3
    filename=005-invalid-mem
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g75
    printf "$num " >&3
    filename=008-invalid-mem
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g76
    printf "$num " >&3
    filename=010_bad_free
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g54
    printf "$num " >&3
    filename=POC7
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g134
    printf "$num " >&3
    filename=004-heap-buffer-over
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

    num=g132
    printf "$num " >&3
    filename=01-Null-exiv2-poc
    echo '------>' Bug $filename '<-------' >&2
    copyTestFile                      $filename
    runTest exiv2                     $filename

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##
