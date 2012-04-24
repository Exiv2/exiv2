#!/bin/sh

##
# for MING, ensure that we have .exe files in src and samples
os=$(uname)
if [ "${os:0:4}" == "MING" ]; then
	for dir in ../src/.libs ../samples/.libs; do
		for file in $dir/*.dll $dir/*.exe ; do
			leaf=$(basename $file)
			if [ ! -e $dir/../$leaf ]; then
				cp $file $dir/.. 2>/dev/null >/dev/null
			fi
		done
	done
fi

# That's all Folks!
##
