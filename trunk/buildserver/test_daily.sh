#!/bin/bash

if [ -z "$JENKINS" ]; then export JENKINS=http://exiv2.dyndns.org:8080 ; fi
if [ -z "$DAILY"   ]; then export DAILY=userContent/builds/Daily       ; fi

##
# which PLATFORM
# JOB_NAME is defined when script is called by Jenkins
# example: JOB_NAME=trunk-cmake-daily/label=msvc
# PLATFORM must be defined as msvc when called from ssh
if [ ! -z "$JOB_NAME" ];then
    PLATFORM=$(echo $JOB_NAME | cut -d= -f 2)
fi
if [ "$PLATFORM" == "" ]; then
    export PLATFORM=''
    if [ `uname` == Darwin  ]; then
        PLATFORM=macosx
    elif [ `uname -o` == Cygwin ]; then
        PLATFORM=cygwin
        # tweak path to ensure the correct version of perl and expr for autotools
        export "PATH=/bin:$PATH"
    elif [ `uname -o` == Msys ]; then
        PLATFORM=mingw
    else
        PLATFORM=linux
    fi
fi

date=$(date '+%Y-%m-%d')

echo date = $date
echo url  = $JENKINS/$DAILY

build=$(curl --silent $JENKINS/$DAILY/                 \
       |xmllint --pretty 1 - | grep $PLATFORM          \
       |grep $date | grep -v -e view | cut -d'"' -f 2  ) 2>/dev/null
echo build = $build
cd /tmp
rm -rf $build
curl -O --silent $JENKINS/$DAILY/$build
ls -alt $build
if [ -e dist ]; then rm -rf dist ;fi

##
# expand the bundle
tar xzf $build
if [ ! -e dist ]; then echo '*** no dist directory ***' ; exit 1; fi

# enter the dist and test it
cd dist
case $PLATFORM in
    macosx)
        # test the delivered exiv2
        DYLD_LIBRARY_PATH="$PWD/$PLATFORM/lib:$DYLD_LIBRARY_PATH"
        $PLATFORM/bin/exiv2 -vV | grep -e libexiv2 -e ^date -e ^bits -e ^version -e ^time

        # compile, link and test the sample code
        echo ''
        g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o exifprint
        ls -alt exifprint
        echo ''

        exifprint --version     | grep -e libexiv2 -e ^date -e ^bits -e ^version -e ^time
    ;;

    *) echo unknown platform $platform
    ;;
esac

# That's all Folks!
##
