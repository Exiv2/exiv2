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

##
# Download the code from Adobe
if [ ! -e Adobe ]; then (
    mkdir Adobe
    cd    Adobe
    curl  -O http://download.macromedia.com/pub/developer/xmp/sdk/XMP-Toolkit-SDK-CC201607.zip
    unzip XMP-Toolkit-SDK-CC201607.zip
) fi

##
# copy third-party code into SDK
(
    find third-party -type d -maxdepth 1 -exec cp -R '{}' Adobe/XMP-Toolkit-SDK-CC201607/third-party ';'
)

##
# generate Makefile and build
(   cd Adobe/XMP-Toolkit-SDK-CC201607/build
    result=1  # assume the build will fail
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
        ;;

        Darwin)
            cmake . -G "Unix Makefiles"              \
                    -DXMP_ENABLE_SECURE_SETTINGS=OFF \
                    -DXMP_BUILD_STATIC=1             \
                    -DCMAKE_CL_64=ON                 \
                    -DCMAKE_BUILD_TYPE=Release
            make
            result=$?
        ;;
    esac
)

##
# copy headers and build libraries
if [ -z "$result" ]; then (
    rm -rf include
    cp -R  Adobe/XMP-Toolkit-SDK-CC201607/public/include include

    # report archives we can see
    cd Adobe/XMP-Toolkit-SDK-CC201607
    find public -name "*.a" -o -name "*.ar" | xargs ls -alt
    cd ../..

    # move the library/archives into xmpsdk
    case "$uname" in
      Linux)
          find Adobe/XMP-Toolkit-SDK-CC201607/public -name "*.ar" -exec cp {} . ';'
          mv staticXMPCore.ar  libXMPCore.a
          mv staticXMPFiles.ar libXMPFiles.a
      ;;

      Darwin)
          find Adobe/XMP-Toolkit-SDK-CC201607/public -name "*.a" -exec cp {} . ';'
          mv libXMPCoreStatic.a  libXMPCore.a
          mv libXMPFilesStatic.a libXMPFiles.a
      ;;
    esac
    ls -alt *.a
) ; fi

# That's all Folks!
##
