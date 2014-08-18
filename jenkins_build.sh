#!/bin/bash

set +v
DIR="$PWD"
if [ "$HOME" == "/Users/rmills" ]; then 
  source ~/bin/.profile
fi
cd "$DIR"

if [ -z "$tests" ]; then tests=true; fi

export PATH=$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/bin:/usr/lib/pkgconfig:/opt/local/bin:$PWD/usr/bin:/opt/local/bin:/opt/local/sbin:/opt/pkgconfig:bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig
##
# cleanup from last time (if there was a last time)
if [ -e config/config.mk ]; then
    if [ ! -e bin ]; then
       mkdir bin
    fi
    make distclean
fi

##
# remove anything left from previous build
#for ext in .a .la .exe ; do
#   find . -name "\"*$ext\"" -exec rm -rf {} ";"
#done

make config
./configure --prefix=$PWD/usr
make LDFLAGS=-L${PWD}/usr/lib
make install
make samples CXXFLAGS=-I${PWD}/usr/include "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs  -lexiv2"
if [ "$tests" == true ]; then
	make tests
fi


# That's all Folks!
##
