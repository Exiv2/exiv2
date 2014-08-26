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

make config # 2>/dev/null >/dev/null

##
# decide what to do about libcurl and libssh
export withcurl=''
export withssh=''
if grep -q curl ./configure ; then
    if [ "$curl" == "true" ]; then withcurl=--with-curl ; else withcurl=--without-curl; fi
fi
if grep -q ssh  ./configure ; then
    if [ "$ssh"  == "true" ]; then withssh=--with-ssh   ; else withssh=--without-ssh  ; fi
fi

if [ "$PLATFORM" == "cygwin" ]; then 
    if [ "$label" == "MSVC" ] ; then
        if [ "$msvc" == "true" ]; then
		    ##
		    # Invoke MSVC build

	 	    rm -rf $PWD/bin
	 	    mkdir $PWD/bin

	 	    PATH=$PATH:/cygdrive/c/Windows/System32
	 	    cmd.exe /c "cd $(cygpath -aw .) && call jenkins_build.bat"
	 	else
	 		echo "*** msvc build not requested ***"
	 	fi
	 	exit $?
    else
		# export LIBS=-lintl
		# I've given up:
		# 1. trying to get Cygwin to build with gettext and friends
		# 2. trying to get Cygwin to install into a local directory
		./configure --disable-nls  $withcurl $withssh
		make
		make install
		make samples
		if [ "$tests" == true ]; then
			make tests
		fi
	fi
fi

build=0
if [ $PLATFORM == "linux"  -a "$linux"  == "true" ]; then build=1; fi
if [ $PLATFORM == "macosx" -a "$macosx" == "true" ]; then build=1; fi

if [ $build == 1 ]; then
    echo -------------
	echo ./configure --prefix=$PWD/usr  $withcurl $withssh
    echo -------------
	./configure --prefix=$PWD/usr  $withcurl $withssh
	make "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs"
	make install
    make samples "CXXFLAGS=-I${PWD}/usr/include -I${PWD}/src" "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs -lexiv2"
	if [ "$tests" == true ]; then
		make tests
	fi
fi

# That's all Folks!
##
