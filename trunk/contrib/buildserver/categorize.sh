#!/bin/bash

if [ "$#" != "1" ]; then
	echo "syntax: $0 directory"
	exit 1
fi

if [ ! -e $1 ]; then
	echo "directory $1 does not exist"
	exit 2
fi

if [ ! -e $1/Daily ]; then
	echo "directory $1/Daily does not exist"
	exit 3
fi

cd $1

##
# cleanup from last time
C=Categorized
if [ -e $C ]; then rm -rf "$C" ; fi

declare -a svns
declare -a platforms
declare -a dates
declare -a files

for i in Daily Weekly Monthly; do
	if [ -e $i ]; then
		pushd $i >/dev/null
			for platform in $(ls -1 | cut -d- -f 1 | sort | uniq) ; do
				platforms+=($platform)
			done
			for svn in $(ls -1 | cut -d- -f 3 | sort | uniq) ; do
				svns+=($svn)
			done
			for date in $(ls -1 | cut -d- -f 5- | cut -d+ -f 1 | sort | uniq) ; do
				dates+=($date)
			done
			for file in $(ls -1); do
				files+=($i/$file)
			done;
		popd > /dev/null
	fi
done

echo svns      ${svns[*]}
echo platforms ${platforms[*]}
echo dates     ${dates[*]}
# echo files     ${files[*]}

echo ---------- Platform -----------
for platform in ${platforms[*]}; do
	dir="$C/Platform/$platform"
	mkdir -p "$dir"
	for file in ${files[*]}; do
		file=$(basename $file)
		PLATFORM=$(echo $file | cut -d- -f 1)
		if [ "$platform" == "$PLATFORM" ]; then
			if   [ -e Daily/$file   ]; then ln -s $PWD/Daily/$file    $dir/$file
			elif [ -e Weekly/$file  ]; then ln -s $PWD/Weekly/$file   $dir/$file
			elif [ -e Monthly/$file ]; then ln -s $PWD/Monthly/$file  $dir/$file
			fi
		fi
	done
done

echo ---------- SVN -----------
latest=0
for svn in ${svns[*]}; do
	dir="$C/SVN/$svn"
	mkdir -p "$dir"
	for file in ${files[*]}; do
		file=$(basename $file)
		SVN=$(echo $file | cut -d- -f 3)
		if [ "$svn" == "$SVN" ]; then
			if   [ -e Daily/$file   ]; then ln -s $PWD/Daily/$file    $dir/$file
			elif [ -e Weekly/$file  ]; then ln -s $PWD/Weekly/$file   $dir/$file
			elif [ -e Monthly/$file ]; then ln -s $PWD/Monthly/$file  $dir/$file
			fi
		fi
	done
	if [ $svn -gt $latest ]; then latest="$svn"; fi
done

echo ---------- Latest -----------
if [ "$latest" != "0" ]; then
	dir="$C/Latest"
	mkdir -p "$dir"
	for file in ${files[*]}; do
		file=$(basename $file)
		SVN=$(echo $file | cut -d- -f 3)
		if [ "$latest" == "$SVN" ]; then
			if   [ -e Daily/$file   ]; then ln -s $PWD/Daily/$file    $dir/$file
			elif [ -e Weekly/$file  ]; then ln -s $PWD/Weekly/$file   $dir/$file
			elif [ -e Monthly/$file ]; then ln -s $PWD/Monthly/$file  $dir/$file
			fi
		fi
	done
fi

echo ---------- Date -----------
for date in ${dates[*]}; do
	dir="$C/Date/$date"
	mkdir -p "$dir"
	for file in ${files[*]}; do
		file=$(basename $file)
		DATE=$(echo $file | cut -d- -f 5- | cut -d+ -f 1)
		if [ "$date" == "$DATE" ]; then
			if   [ -e Daily/$file   ]; then ln -s $PWD/Daily/$file    $dir/$file
			elif [ -e Weekly/$file  ]; then ln -s $PWD/Weekly/$file   $dir/$file
			elif [ -e Monthly/$file ]; then ln -s $PWD/Monthly/$file  $dir/$file
			fi
		fi
	done
done


# That's all Folks!
##
