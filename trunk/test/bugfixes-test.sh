#!/bin/bash
# Test driver with regression tests for bugfixes

source ./functions.source

(
	cd ./tmp

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
	runTest largeiptc-test $filename ../data/imagemagick.png

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
	
	num=799
	filename=`prep_empty_file $num`
	runTest exiv2 -v -m ../data/bug$num.cmd $filename
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
		diff -q ../data/$filename.rsrc $filename/rsrc
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

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | sed 's/\x0d$//' > $results-stripped
reportTest $results-stripped $good 

# That's all Folks!
##