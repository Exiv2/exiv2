#!/bin/bash

##
# jenkins_build.sh
#	called by jenkins to build/test exiv2 
#	- jenkins sets environment variables
#	called from terminal 
#	- script has build-in defaults for some environment variable
#
#  arguments:
#    status    : filter last build with grep
#
#  environment variables (all optional)
#    JENKINS   : URL of jenkins server. Default http://exiv2.dyndns.org:8080
##
if [ -z "$JENKINS" ]; then export JENKINS=http://exiv2.dyndns.org:8080; fi
result=0
base=$(basename $0)
if [ -z "$base" ]; then base=jenkins_build ; fi
tmp=/tmp/$base.tmp
start=$(date)
starts=$(date +%s)

##
# are we recursively building mingw?
buildmingw=0
if [ "$1" == "buildmingw" ]; then
	buildmingw=1
	shift
fi

##
# functions
run_tests() {
	if [ "$result" == "0" ]; then
		if [ "$tests" == true ]; then
			make tests
		fi
	fi
}

thepath () { 
    if [ -d $1 ]; then
        ( cd $1;
        pwd );
    else
        ( cd $(dirname $1);
        echo $(pwd)/$(basename $1) );
    fi
}

##
# arg: status [grep-args]
# example: ./jenkins_build.sh status -e URL
if [ "$1" == "status" ]; then
	shift
	build=$(basename $PWD)
	declare -A expects=( [linux]=900 [macosx]=900 [cygwin]=1000 [mingw]=100 [msvc]=1200 )
	for b in linux macosx cygwin mingw msvc ; do
		echo $build/$b
		curl --silent $JENKINS/job/Exiv2-$build/label=$b/lastBuild/consoleText \
		   | tee $tmp \
		   | grep -E -e SVN_[A-Z]+= \
		             -e JOB_NAME    \
		             -e BUILD_ID    \
		             -e Finished    \
		             -e seconds     \
		             -e succeeded   \
		             $@
		declare -i lines=$(wc -l $tmp | cut -d/ -f 1)
		declare -i expect=${expects[$b]}
		diff=$(( lines-expect>0?lines-expect:expect-lines ))
		msg=''
		warn=''
		if [ "$diff" -gt "200" ]; then warn="***" ; msg="TAKE CARE " ; fi
		echo "${warn}${msg}lines= ${lines} expect~${expect} diff=${diff}" "${msg}${warn}"
		echo ''
	done
	exit $result
fi


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
echo "1 target = $target platform = $PLATFORM WORKSPACE = $WORKSPACE"
if [ $PLATFORM == "macosx" -a -z "$macosx" ]; then export macosx=true ; export target=macosx    ; fi
if [ $PLATFORM == "linux"  -a -z "$linux"  ]; then export linux=true  ; export target=linux	    ; fi
if [ -z "$cygwin"          -a ! -z $CYGWIN ]; then export cygwin=$CYGWIN                        ; fi                   
if [ -z "$tests"     ]; then export tests=true                                                  ; fi
if [ -z "$WORKSPACE" ]; then export WORKSPACE="$0/$PLATFORM"                                    ; fi

if [ -z "$target" ]; then export target=$(basename $(echo $WORKSPACE | sed -E -e 's#\\#/#g'))   ; fi
echo "2 target = $target platform = $PLATFORM WORKSPACE = $WORKSPACE"

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
make config &>/dev/null

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
build=NONE

if [ $PLATFORM == "linux"  -a "$target" == "linux"  -a "$linux"	 == "true"  ]; then build=UNIX ; fi
if [ $PLATFORM == "macosx" -a "$target" == "macosx" -a "$macosx" == "true"  ]; then build=UNIX ; fi
if [ $PLATFORM == "cygwin" -a "$target" == "cygwin" -a "$cygwin" == "true"  ]; then build=CYGW ; fi
if [ $PLATFORM == "cygwin" -a "$target" == "mingw"  -a "$mingw"	 == "true"  ]; then build=MING ; fi
if [ $PLATFORM == "cygwin" -a "$target" == "msvc"   -a "$msvc"	 == "true"  ]; then build=MSVC ; fi
if [ $PLATFORM == "mingw"  -a "$target" == "mingw"                          ]; then build=MING ; fi

echo "3 target = $target platform = $PLATFORM build = $build"

case "$build" in
  UNIX) 
		echo -------------
		echo ./configure --prefix=$PWD/usr	$withcurl $withssh
		echo -------------
		./configure --prefix=$PWD/usr  $withcurl $withssh
		make -j4 "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs"
		make install
		make -j4 samples "CXXFLAGS=-I${PWD}/usr/include -I${PWD}/src" "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs -lexiv2"
		result=$?
		run_tests
  ;;
  
  CYGW) 
		# export LIBS=-lintl
		# I've given up:
		# 1. trying to get Cygwin to build with gettext and friends
		# 2. trying to get Cygwin to install into a local directory
		./configure --disable-nls  $withcurl $withssh
		make -j4
		# result=$?
		make install
		make -j4 samples
		run_tests
  ;;

  MING) 
		if [ "$buildmingw" == "1" ]; then
		    ./configure --disable-nls  $withcurl $withssh
			make -j4
			make install
			make -j4 samples
			run_tests
		else
			make distclean
			/c/Users/rmills/com/mingw64.sh "-c jenkins_build.sh buildmingw"
		fi
  ;;

  MSVC) 
		rm -rf $PWD/bin
		mkdir $PWD/bin

		PATH=$PATH:/cygdrive/c/Windows/System32
		cmd.exe /c "cd $(cygpath -aw .) && call jenkins_build.bat"
		result=$?
  ;;
  
  
  NONE) 
		echo "**************************************"
		echo "*** no build requested for $target ***"
		echo "**************************************"
  ;; 
esac

echo target "$target" start: "$start" finish: $(date) diff: $(( $(date +%s) - starts )) seconds
set -v
# That's all Folks!
##
exit $result
