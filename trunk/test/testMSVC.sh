#!/bin/bash

msvc=$1
dryrun=$2

if [ -z $msvc ]; then
	echo usage: $0 \<path-to-msvc-bin\> [dryrun]
	exit
fi

if [ ! -z $dryrun ]; then
	dryrun=echo
fi

for d in $(find "$msvc" -name exiv2.exe -exec dirname {} ";"); do
    export EXIV2_BINDIR=$d
    bar='-----'
    Bar=$bar
    for i in $(seq 0 $((${#d}+1))); do Bar+=-; done
    Bar+=$bar
    echo $Bar
    echo $bar $d $bar
    echo $bar $(cygpath -aw "$msvc") $bar
    echo $Bar
    $dryrun make test
done

# That's all Folks!
##

