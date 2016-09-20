#!/bin/bash

# This is a "throw away" script for building lots of revisions
# You're expected to modify this script to perform whatever test
# is appropriate for your revision investigation

buildRevision()
{
	r="$1"
	echo ------------- $r begin --------------
	##
	# get the code
	R="../../../$r"
	if [ ! -e $R ]; then
		svn checkout svn://dev.exiv2.org/svn/trunk --revision $r $R | wc
	fi
	pushd $R
		##
		# build and install it
		echo $PWD
		make config 2>/dev/null  | wc
		./configure              | wc
		make -j             2>&1 | wc
		sudo make install        | wc
		make -j  samples    2>&1 | wc
		echo build = $(exiv2 -vV | grep -e svn -e dll -e svn) $(ls -alt $(which exiv2))

		##
		# test the build
		make teste
	popd > /dev/null
	echo ------------- $r end -------------- $'\n'
}

##
# test a sequence of build revisions
# to use a list of revisions, use for "r in 3800 3820 3877; do"
if [ "$#"  == "0" ]; then
    for r in $(seq 3000 500 4000); do buildRevision $r ; done;
else
    for r in "$@"                ; do buildRevision $r ; done;
fi

# That's all Folks!
##
