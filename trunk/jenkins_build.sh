#!/bin/bash

set +v
DIR="$PWD"
if [ -e /home/rmills/bin/.profile ]; then
	source /home/rmills/bin/.profile
fi
if [ -e /Users/rmills/bin/.profile ]; then
	source /Users/rmills/bin/.profile
fi
cd "$DIR"

echo ----------------------
export
echo ----------------------

if [ -z "$tests" ]; then tests=true; fi

export PATH=$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/bin:/usr/lib/pkgconfig:/opt/local/bin:$PWD/usr/bin:/opt/local/bin:/opt/local/sbin:/opt/pkgconfig:bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig

##
# cleanup from last time (if there was a last time)
if [ -e config/config.mk ]; then
    if [   -e bin ]; then
        rm -rf bin
    fi
    if [ ! -e bin ]; then
       mkdir bin
    fi
    make distclean
fi

make config

build=0
if [ $PLATFORM == "linux" -a "$linux" == "true" ]; then build=1; fi
if [ $PLATFORM == "macos" -a "$macos" == "true" ]; then build=1; fi


if [ "$PLATFORM" == "cygwin" ]; then 
#	export LIBS=-lintl
    # I've given up:
    # 1 trying to get Cygwin to build with gettext and friends
    # 2 trying to get Cygwin to install into a local directory
    
    if [ "$label" == "MSVC" ] ; then
		##
		# Invoke MSVC build
	 	export ACTION=/rebuild
	 	export BuildEnv=native
	 	export Builder=2005
	 	export COMPILER=G++
	 	export debug=false
	 	export dll=true
	 	export expat=true
	 	export libssh=false
	 	export Linux=true
	 	export MSVC=true
	 	export openssl=false
	 	export release=true
	 	export static=false
	 	export teste=false
	 	export testr=false
	 	export tests=true
	 	export testv=false
	 	export Win32=false
	 	export x64=true
	 	export zlib=true 

	 	rm -rf $PWD/bin
	 	mkdir $PWD/bind	

	 	PATH=$PATH:/cygdrive/c/Windows/System32
	 	cmd.exe /c "cd $(cygpath -aw .) && call jenkins_build.bat"
	 	exit $?
    else
		./configure --disable-nls
		make
		make install
		make samples
		make tests
	fi
fi

if [ $build == 1 ]; then
	./configure --prefix=$PWD/usr
	make "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs"
	make install
    make samples CXXFLAGS=-I${PWD}/usr/include "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs -lexiv2"
	if [ "$tests" == true ]; then
		make tests
	fi
fi


# That's all Folks!
##
