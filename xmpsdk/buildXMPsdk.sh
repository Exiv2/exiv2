#!/bin/bash

##
# buildXMPsdk.sh
# TODO:
#     1) Cygwin support (in progress)
#     2) Write buildXMPsdk.cmd (for MSVC)
##

syntaxError() {
    echo "usage: $0 [--help|-?|--2013|--2014|--2016|--32|--64]"
    exit 1
}

uname=$(uname -a | cut -d' ' -f 1 | cut -d_ -f 1)
case "$uname" in
    Darwin|Linux|CYGWIN) ;;
    *)  echo "*** unsupported platform $uname ***"
        exit 1
    ;;
esac

##
# always run this script in <exiv2dir>/xmpsdk
cd $(dirname "$0")

##
# parse command-line
bits=64
SDK=2016

while [ $# -ne 0 ]; do
    case "$1" in
      -h|--help|-\?) syntaxError; exit 0;;
      --64|64)       bits=64    ; shift ;;
      --32|32)       bits=32    ; shift ;;
      --2013|2013)   SDK=2013   ; shift ;;
      --2014|2014)   SDK=2014   ; shift ;;
      --2016|2016)   SDK=2016   ; shift ;;
        install)     exit 0             ;; # argument passed by make

      *)             syntaxError; exit 1;;
    esac
done

if [ "$SDK" == "2013" ] ; then SDK=XMP-Toolkit-SDK-CC201306 ; ZIP=XMP-Toolkit-SDK-CC-201306 ;fi
if [ "$SDK" == "2014" ] ; then SDK=XMP-Toolkit-SDK-CC201412 ; ZIP=$SDK; fi
if [ "$SDK" == "2016" ] ; then SDK=XMP-Toolkit-SDK-CC201607 ; ZIP=$SDK; fi

##
# if it's already built, we're done
if [ -e Adobe/$SDK/libXMPCore.a ]; then ls -alt Adobe/$SDK/libXMPCore.a ; exit 0 ; fi

##
# Download the code from Adobe
if [ ! -e    Adobe/$SDK ]; then (
    mkdir -p Adobe
    cd       Adobe
    if curl -O http://download.macromedia.com/pub/developer/xmp/sdk/$ZIP.zip ; then
        unzip $ZIP.zip
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
    # Tweak the code (depends on platform) and build using adobe tools
    case "$uname" in
        Linux)
            target=StaticRelease64
            if [ "$BITS" == "32" ]; then target=StaticRelease32 ; fi
            make "$target"
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
                target=5 # (5=64 bit build for 2013 and 2014, 3 = 64 bit build for 2016)
                if [ "$SDK" == "XMP-Toolkit-SDK-CC201607" ]; then target=3; fi
                echo $target | ./GenerateXMPToolkitSDK_mac.sh

                project=XMPToolkitSDK64.xcodeproj
                find . -name "$project" -execdir xcodebuild -project {} -target XMPCoreStatic  -configuration Release \;
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
    rm -rf *.a *.ar  # clean up from previous build
    find public -name "*.a" -o -name "*.ar" | xargs ls -alt

    # move the library/archives into xmpsdk/Adobe/$SDK
    case "$uname" in
      Linux)
          find public -name "*.ar" -exec cp {} . ';'
          mv   staticXMPCore.ar  libXMPCore.a
      ;;

      Darwin)
          find public -name "libXMPCoreStatic.a" -exec cp {} libXMPCore.a ';'
      ;;
    esac
    ls -alt *.a
) ; fi

# That's all Folks!
##
