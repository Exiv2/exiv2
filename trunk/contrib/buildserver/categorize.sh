#!/bin/bash

##
# This script is rather slow and takes about 10 minutes
# One day I'll rewrite it in Python and it will run in seconds

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

##
# belongs element array : echo yes ? 1 : 0
belongs () {
    local result=0
    local e
    for e in "${@:2}"; do 
        if [ "$e" == "$1" ]; then
            result=1
        fi
    done
    echo $result;
}

##
# symlink - safely: ln -s $1 $2
symlink()
{
    if [ -e $1 ]; then
        if [ ! -e $2 ]; then
            ln -s $1 $2
        fi
    fi
}

for i in Daily Weekly Monthly; do
    if [ -e $i ]; then
        pushd $i >/dev/null
            for platform in $(ls -1 | cut -d- -f 1 | sort | uniq) ; do
                if [ $(belongs $platform "${platforms[@]}") == 0 ]; then
                    platforms+=($platform)
                fi
            done
            for svn in $(ls -1 | cut -d- -f 3 | sort | uniq) ; do
                if [ $(belongs $svn "${svns[@]}") == 0 ]; then
                    svns+=($svn)
                fi
            done
            for date in $(ls -1 | cut -d- -f 5- | cut -d+ -f 1 | sort | uniq) ; do
                if [ $(belongs $date "${dates[@]}") == 0 ]; then
                    dates+=($date)
                fi
            done
            for file in $(ls -1); do
                if [ $(belongs $file "${files[@]}") == 0 ]; then
                    files+=($i/$file)
                fi
            done;
        popd > /dev/null
    fi
done


# echo platform: ${platforms[*]}
# echo svn:      ${svns[*]}
# echo date:     ${dates[*]}
# echo files:    ${files[*]}


echo ---------- Platform: ${platforms[*]} -----------
for platform in ${platforms[*]}; do
    dir="$C/Platform/$platform"
    mkdir -p "$dir"
    for file in ${files[*]}; do
        file=$(basename $file)
        PLATFORM=$(echo $file | cut -d- -f 1)
        if [ "$platform" == "$PLATFORM" ]; then
            symlink $PWD/Daily/$file    $dir/$file
            symlink $PWD/Weekly/$file   $dir/$file
            symlink $PWD/Monthly/$file  $dir/$file
        fi
    done
done

echo ---------- SVN: ${svns[*]} -----------
latest=0
for svn in ${svns[*]}; do
    dir="$C/SVN/$svn"
    mkdir -p "$dir"
    for file in ${files[*]}; do
        file=$(basename $file)
        SVN=$(echo $file | cut -d- -f 3)
        if [ "$svn" == "$SVN" ]; then
            symlink $PWD/Daily/$file    $dir/$file
            symlink $PWD/Weekly/$file   $dir/$file
            symlink $PWD/Monthly/$file  $dir/$file
        fi
    done
    if [ $svn -gt $latest ]; then latest="$svn"; fi
done

echo ---------- Latest: $latest -----------
if [ "$latest" != "0" ]; then
    dir="$C/Latest"
    mkdir -p "$dir"
    for file in ${files[*]}; do
        file=$(basename $file)
        SVN=$(echo $file | cut -d- -f 3)
        if [ "$latest" == "$SVN" ]; then
            symlink $PWD/Daily/$file    $dir/$file
            symlink $PWD/Weekly/$file   $dir/$file
            symlink $PWD/Monthly/$file  $dir/$file
        fi
    done
fi

echo ---------- Date: ${dates[*]} -----------
for date in ${dates[*]}; do
    dir="$C/Date/$date"
    mkdir -p "$dir"
    for file in ${files[*]}; do
        file=$(basename $file)
        DATE=$(echo $file | cut -d- -f 5- | cut -d+ -f 1)
        if [ "$date" == "$DATE" ]; then
            symlink $PWD/Daily/$file    $dir/$file
            symlink $PWD/Weekly/$file   $dir/$file
            symlink $PWD/Monthly/$file  $dir/$file
        fi
    done
done

# That's all Folks!
##
