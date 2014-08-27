#!/bin/bash

##
# jenkins_build.sh
#	called by jenkins to build/test exiv2 
#	- jenkins sets environment variables
#	called from terminal 
#	- script has build-in defaults for some environment variable
#
##

##
# functions
run_tests() {
	if [ "$tests" == true ]; then
		make tests
	fi
}

##
# Quick dodge, use rmills ~/bin/.profile to set some environment variables
# I think I need this is necessary to find pkg-config
# This code will be removed later
# set +v
DIR="$PWD"
if [ -e /home/rmills/bin/.profile ]; then
	source /home/rmills/bin/.profile
fi
if [ -e /Users/rmills/bin/.profile ]; then
	source /Users/rmills/bin/.profile
fi
cd "$DIR"
# set -v

##
# where are we?
export PLATFORM=''
if [ `uname` == Darwin	]; then
	PLATFORM=macosx
elif [ `uname -o` == Cygwin ]; then
	PLATFORM=cygwin
elif [ `uname -o` == Msys ]; then
	PLATFORM=mingw
else
	PLATFORM=linux
fi

##
# set up some defaults (used when running this script from the terminal)
if [						  -z "$tests"  ]; then export tests=true  ; fi
if [ $PLATFORM == "macosx" -a -z "$macosx" ]; then export macosx=true ; export label=macosx ; fi
if [ $PLATFORM == "cygwin" -a -z "cygwin"  ]; then export cygwin=true ; export label=cygwin ; fi
if [ $PLATFORM == "linux"  -a -z "$linux"  ]; then export linux=true  ; export label=linux	; fi
if [ $PLATFORM == "mingw"  -a -z "$mingw"  ]; then export mingw=true  ; export label=mingw	; fi

export PATH=$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/bin:/usr/lib/pkgconfig:/opt/local/bin:$PWD/usr/bin:/opt/local/bin:/opt/local/sbin:/opt/pkgconfig:bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig

echo   ----------------------
export
echo   ----------------------

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

##
# create ./configure
# we need to inspect configure to know if libssh and libcurl are options for this build
make config # &>/dev/null

##
# decide what to do about curl and ssh
# 3 possibilities:
# 1 withcurl is empty		   (for 0.24 builds without WebReady support)
# 2 withcurl == --with-curl	   (build supports curl and not requested)
# 3 withcurl == --without-curl (build supports curl and requested)
export withcurl=''
export withssh=''
if grep -q curl ./configure ; then
	if [ "$curl" == "true" ]; then withcurl=--with-curl ; else withcurl=--without-curl; fi
fi
if grep -q ssh	./configure ; then
	if [ "$ssh"	 == "true" ]; then withssh=--with-ssh	; else withssh=--without-ssh  ; fi
fi

##
# what kind of build is this?
NONE=0
UNIX=1
CYGW=2
MSVC=3
MING=$NONE
build=$NONE

if [ $PLATFORM == "linux"  -a "$label" == "linux"  -a "$linux"	== "true" ]; then build=$UNIX ; fi
if [ $PLATFORM == "macosx" -a "$label" == "macosx" -a "$macosx" == "true" ]; then build=$UNIX ; fi
if [ $PLATFORM == "cygwin" -a "$label" == "cygwin" -a "$cygwin" == "true" ]; then build=$CYGW ; fi
if [ $PLATFORM == "cygwin" -a "$label" == "mingw"  -a "$mingw"	== "true" ]; then build=$MING ; fi
if [ $PLATFORM == "cygwin" -a "$label" == "msvc"   -a "$MSVC"	== "true" ]; then build=$MSVC ; fi

case "$build" in
  "$UNIX" ) 
			echo -------------
			echo ./configure --prefix=$PWD/usr	$withcurl $withssh
			echo -------------
			./configure --prefix=$PWD/usr  $withcurl $withssh
			make "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs"
			make install
			make samples "CXXFLAGS=-I${PWD}/usr/include -I${PWD}/src" "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs -lexiv2"
			run_tests
  ;;
  
  "$CYGW" ) 
			# export LIBS=-lintl
			# I've given up:
			# 1. trying to get Cygwin to build with gettext and friends
			# 2. trying to get Cygwin to install into a local directory
			./configure --disable-nls  $withcurl $withssh
			make
			make install
			make samples
			run_tests
  ;;

  "$MSVC" ) 
			rm -rf $PWD/bin
			mkdir $PWD/bin

			PATH=$PATH:/cygdrive/c/Windows/System32
			cmd.exe /c "cd $(cygpath -aw .) && call jenkins_build.bat"
  ;;
  
  
  "$NONE") 
	 echo "*************************************************"
	 echo "*** no build for platform $PLATFORM requested ***"
	 echo "*************************************************"
  ;; 
esac
set -v
# That's all Folks!
##
