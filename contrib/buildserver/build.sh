#!/bin/bash

syntax() {
	echo "usage: $0 { --help | -? | -h | platform }+ "
	echo "platform: all | cygwin | linux | macosx | mingw | msvc "
}

announce()
{
	echo ++++++++++++++++++++++++++++++++
	echo $*
	echo ++++++++++++++++++++++++++++++++
}

msvc=0
cygwin=0
mingw=0
cygwin=0
macosx=0
linux=0
help=0

if [ "$#" == "0" ]; then help=1; fi

while [ "$#" != "0" ]; do
    case "$1" in
      -h|--help|-\?) help=1    ;;
      all)       cygwin=1; linux=1; macosx=1; mingw=1; msvc=1;  ;;
      cygwin)    cygwin=1      ;;
      linux)     linux=1       ;;
      macosx)    macosx=1      ;;
      mingw)     mingw=1       ;;
      msvc)      msvc=1        ;;
      *)         echo "invalid option: $1" 1>&2; help=1; ;;
    esac
    if [ "$#" != "0" ]; then shift ; fi
done

if [ $help == 1 ]; then
	syntax;
	exit 0;
fi

if [ $linux == 1 ]; then
me=Linux
server=rmills@rmillsmm-ubuntu
command=''
cd=/home/rmills/gnu/github/exiv2/
fi

if [ $macosx == 1 ]; then
me=MacOS-X
server=rmills@rmillsmm
cd=/Users/rmills/gnu/github/exiv2/
command=''
fi

if [ $mingw == 1 ]; then
me=MinGW
server=rmills@rmillsmm-w7
command='msys64'
cd=/home/rmills/gnu/github/exiv2/
fi

if [ $cygwin == 1 ]; then
me=Cygwin
server=rmills@rmillsmm-w7
command='c:\\cygwin64\\bin\\bash.exe'
fi

if [ ! -z $server ]; then
announce $me
! ssh ${server} ${command} <<EOF
PATH="/usr/local/bin/:/usr/bin:/mingw64/bin:$PATH"
cd ${cd}
mkdir -p buildserver
rm   -rf buildserver
git clone --branch RC1 https://github.com/exiv2/exiv2 buildserver
mkdir -p buildserver/build
cd       buildserver/build
cmake .. -G "Unix Makefiles"
make
make tests
make package
EOF
fi

if [ $msvc == 1 ]; then
cd=c:\\Users\\rmills\\gnu\\github\\exiv2\\
profile=msvc2017Release64
config=Release
generator='"Visual Studio 15 2017 Win64"'
announce  ${profile}
! ssh rmills@rmillsmm-w7 <<EOF
cd ${cd}
IF EXIST buildserver rmdir/s/q buildserver
git clone --branch RC1 https://github.com/exiv2/exiv2 buildserver
mkdir    buildserver\build
cd       buildserver\build
conan install .. --profile ${profile} --build missing
cmake         .. -G ${generator} -DCMAKE_BUILD_TYPE=${config}  -DCMAKE_INSTALL_PREFIX=..\\dist\\${profile}
cmake --build .  --config ${config}   --target install
EOF
fi

# That's all Folks
##

