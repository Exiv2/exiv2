#!/bin/bash


syntaxError() {
	echo "usage: $0 [{--help|-?}| {platform}+]"
	echo "platform: all | msvc | cygwin | linux | mingw | macosx "
	exit 1
}

bomb() {
	echo "*** $1 requires an argument ***" 1>&2
	exit 1
}


msvc=0
cygwin=0
mingw=0
cygwin=0
macosx=0
linux=0
verbose=0

if [ "$#" == "0" ]; then syntaxError; fi

while [ "$#" != "0" ]; do
    case "$1" in
      -h|--help|-\?) syntaxError; exit 0 ;;
      -v|--verbose)  verbose=1 ;;
      all)       msvc=1; cygwin=1; linux=1; mingw=1; macosx=1 ;;
      msvc)      msvc=1   ;;
      cygwin)    cygwin=1 ;;
      mingw)     mingw=1  ;;
      macosx)    macosx=1 ;;
      linux)     linux=1  ;;
      *)         echo "invalid option: $1" 1>&2; syntaxError; exit 1;;
    esac
    if [ "$#" != "0" ]; then shift ; fi
done


if [ $macosx == 1 ]; then
ssh rmills@rmillsmm <<EOF
cd ~/gnu/github/exiv2/exiv2
git pull
mkdir -p build
rm   -rf build
mkdir    build
cd build
cmake .. -G "Unix Makefiles"
make
make tests
EOF
fi

if [ $linux == 1 ]; then
ssh rmills@rmillsmm-ubuntu <<EOF
cd ~/gnu/github/exiv2/exiv2
git pull
mkdir -p build
rm   -rf build
mkdir    build
cd build
cmake .. -G "Unix Makefiles"
make
make tests
EOF
fi

if [ $msvc == 1 ]; then
ssh rmills@rmillsmm-w7 <<EOF
cd /Users/rmills/gnu/github/exiv2/exiv2/
git pull
IF EXIST build rmdir/s/q build
IF EXIST dist  rmdir/s/q dist
mkdir    build
cd       build
conan install .. --profile msvc2017Release64 --build missing
cmake         .. -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release  -DCMAKE_INSTALL_PREFIX=dist
cmake --build .  --config Release

rem cd       contrib/cmake/msvc
rem mkdir -p build
rem rm   -rf build
rem cmd/c "vcvars 2017 64 && cmakeBuild --build --test"
EOF
fi


if [ $cygwin == 1 ];then
ssh rmills@rmillsmm-w7 C:\\cygwin64\\bin\\bash.exe <<EOF
cd /home/rmills/gnu/github/exiv2/exiv2
git pull
mkdir -p build
rm   -rf build
mkdir    build
cd build
cmake .. -G "Unix Makefiles"
make
make tests
EOF
fi


if [ $mingw == 1 ]; then
ssh rmills@rmillsmm-w7 msys64 <<EOF
cd ~/gnu/github/exiv2/exiv2
git pull
mkdir -p build
rm   -rf build
mkdir    build
cd build
cmake .. -G "Unix Makefiles"
make
make tests
EOF
fi

# That's all Folks
##

