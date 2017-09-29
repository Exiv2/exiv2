#!/bin/bash
# Test driver with regression tests for bugfixes

source ./functions.source

(	cd "$testdir"

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

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good

# That's all Folks!
##
