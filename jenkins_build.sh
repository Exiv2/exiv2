#!/bin/bash

export PATH=$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/bin:/usr/lib/pkgconfig:/opt/local/bin:$PWD/usr/bin:/opt/local/bin:/opt/local/sbin:/opt/pkgconfig:bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig

if [ "$ACTION" == /rebuild ]; then
	if [ -e configure ]; then
		make distclean;
	fi
fi

if [ ! -e configure ]; then 
	make config
	./configure --prefix=$PWD/usr
fi

touch src/version.cpp

make
make install
make samples

if [ "$tests" == true ]; then
	make tests
fi

# That's all Folks!
##
