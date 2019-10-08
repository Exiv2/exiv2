#!/usr/bin/env bash

msvc=$1
test=$2
home=$(dirname $(readlink -f $0))

if [ -z "$msvc" ]; then
    echo usage: $0 \<path-to-msvc-bin\> [test]
    exit
fi

if [ ! -d "$msvc" ]; then
    echo directory "$msvc" does not exist
    exit
else
    msvc=$(readlink -f "$msvc")
fi

if [ -z $test ]; then
    test=tests
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
    (
      cd "$home/.."
      make $test
      if [ -z "$2" ]; then
          $d/exiv2.exe -v -V
      fi
    )
done

# That's all Folks!
##