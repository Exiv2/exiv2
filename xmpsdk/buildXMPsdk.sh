#!/bin/bash

##
# buildXMPsdk.sh
# TODO:
#     1) Command-line parser for options such as:
#        version of xmpsdk (2014, 2016)
#        build options     (64/32 static/dynamic debug/release)
#        help
#     2) Cygwin support (in progress)
#     3) Write buildXMPsdk.cmd (for MSVC)
##

uname=$(uname -a | cut -d' ' -f 1 | cut -d_ -f 1)
case "$uname" in
    Darwin|Linux|CYGWIN) ;;
    *)  echo "*** unsupported platform $uname ***"
        exit 1
    ;;
esac

cd $(dirname "$0")  # always run this script in exiv2-dir/xmpsdk
SDK=XMP-Toolkit-SDK-CC201607
if [ "$1" == "2014" ]; then
	SDK=XMP-Toolkit-SDK-CC201412
fi

##
# Download the code from Adobe
if [ ! -e Adobe/$SDK ]; then (
    mkdir Adobe
    cd    Adobe
    if curl -O http://download.macromedia.com/pub/developer/xmp/sdk/$SDK.zip ; then
    	unzip $SDK.zip
    fi
) fi

if [ ! -d Adobe/$SDK ]; then
	echo "*** ERROR SDK = Adobe/$SDK not found" >2
	exit 1
fi

##
# copy third-party code into SDK
(
    find third-party -type d -maxdepth 1 -exec cp -R '{}' Adobe/$SDK/third-party ';'
)

##
# generate Makefile and build libraries
(   cd Adobe/$SDK/build
    ##
    # Tweak the code (depends on platform)
    case "$uname" in
        Linux)
            # modify ProductConfig.cmake (don't link libssp.a)
            f=ProductConfig.cmake
            if [ -e $f.orig ]; then mv $f.orig $f ; fi ; cp $f $f.orig
            sed -E -e 's? \$\{XMP_GCC_LIBPATH\}/libssp.a??g' $f.orig > $f

            # copy resources
            for f in XMPFiles XMPCore; do
                cp ../$f/resource/linux/* ../$f/resource
            done

            cmake . -G "Unix Makefiles"              \
                    -DXMP_ENABLE_SECURE_SETTINGS=OFF \
                    -DXMP_BUILD_STATIC=1             \
                    -DCMAKE_CL_64=ON                 \
                    -DCMAKE_BUILD_TYPE=Release
            make
            result=$?
        ;;

        CYGWIN)
            f=../source/Host_IO-POSIX.cpp
            if [ -e $f.orig ]; then mv $f.orig $f ; fi ; cp $f $f.orig

            sed -E -e $'s?// Writeable?// Writeable~#include <windows.h>~#ifndef PATH_MAX~#define PATH_MAX 512~#endif?' $f.orig | tr "~" "\n" > $f
            result=1 # build failed.  Can't build Cygwin yet!
        ;;

        Darwin)
            if [ ! -e ../../$SDK/public/libraries/macintosh/intel_64/release/libXMPCoreStatic.a ]; then
            	cmake . -G "Unix Makefiles"          \
                    -DXMP_ENABLE_SECURE_SETTINGS=OFF \
                    -DXMP_BUILD_STATIC=1             \
                    -DCMAKE_CL_64=ON                 \
                    -DCMAKE_BUILD_TYPE=Release
            	make
            	result=$?
            fi
        ;;
        *)
        	result=1  # build failed
        ;;
    esac
)

##
# copy headers and built libraries
if [ -z "$result" ]; then (

    cp Adobe/$SDK/third-party/zuid/interfaces/MD5.h  Adobe/$SDK/public/include/MD5.h

    # report archives we can see
    cd   Adobe/$SDK
    find public -name "*.a" -o -name "*.ar" | xargs ls -alt
    cd   ../

    # move the library/archives into xmpsdk
    case "$uname" in
      Linux)
      	  rm -rf *.a *.ar
          find $SDK/public -name "*.ar" -exec cp {} . ';'
          mv   staticXMPCore.ar  libXMPCore.a
          mv   staticXMPFiles.ar libXMPFiles.a
      ;;

      Darwin)
      	  rm -rf *.a *.ar
          find $SDK/public -name "*.a" -exec cp {} . ';'
          mv   libXMPCoreStatic.a  libXMPCore.a
          mv   libXMPFilesStatic.a libXMPFiles.a
      ;;
    esac
    ls -alt *.a
) ; fi

# That's all Folks!
##
