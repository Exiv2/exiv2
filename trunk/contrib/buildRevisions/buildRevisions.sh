#!/bin/bash

# This is a "throw away" script for building lots of revisions
# You're expected to modify this script to perform whatever test
# is appropriate for your revision investigation

##
# test a sequence of build revisions
# to use a list of revisions, use for "r in 3800 3820 3877; do"
for r in $(seq 3800 50 3900); do
	echo ------------- $r begin --------------
	##
	# get the code
	R="../../../$r"
	if [ -e $R ]; then rm -rf $R ; fi
	svn checkout svn://dev.exiv2.org/svn/trunk --revision $r $R | wc
	pushd $R
		##
		# build and install it
		echo $PWD
		make config 2>/dev/null  | wc
		./configure              | wc
		make -j  2>&1            | wc
		sudo make install        | wc
		make -j samples 2>&1     | wc
		echo build = $(exiv2 -vV | grep -e svn -e dll) $(ls -alt $(which exiv2))

		##
		# test the build
		T=tst.tiff
		curl  --silent        -O http://dev.exiv2.org/attachments/download/1004/$T
		exiv2 --verbose       -M"set Exif.Image.Software revision $r"           $T
		exiv2 --grep Software -pa                                               $T
	popd > /dev/null
	echo ------------- $r end -------------- $'\n'
done

# That's all Folks!
##
