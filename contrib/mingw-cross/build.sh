#!/usr/bin/env bash

#
# exiv2/contrib/cross-ming/build.sh
#
# This is an "experimental" script for cross-compiling exiv2 for 64bit MinGW from Linux
# This is derived from work discussed with Kamran Ghanaat
# https://github.com/Exiv2/exiv2/issues/610
#
# Filesystem layout
# rmills@rmillsmm-ubuntu:~/gnu/github/exiv2$ ls -l
# drwxrwxr-x 30 rmills rmills 4096 Jan  2 12:02 exiv2
# drwxr-xr-x 14 rmills rmills 4096 Jan  1 19:26 expat-2.2.6
# drwxr-xr-x 16 rmills rmills 4096 Jan  1 18:48 zlib-1.2.11
# rmills@rmillsmm-ubuntu:~/gnu/github/exiv2$
#
# rmills@rmillsmm-ubuntu:~/gnu/github/exiv2/exiv2/contrib/mingw-cross$ ls -l
# -rwxrwxr-x 1 rmills rmills 2889 Jan  2 13:59 build.sh     this script
# drwxrwxr-x 2 rmills rmills 4096 Jan  2 13:52 gcc64        DLLs
# drwxrwxr-x 5 rmills rmills 4096 Jan  2 12:06 build        build generated in build/exiv2/bin
# rmills@rmillsmm-ubuntu:~/gnu/github/exiv2/exiv2/contrib/mingw-cross$
#
# prerequisites: sudo apt install --yes realpath binutils-mingw-w64-x86-64 \
#                g++-mingw-w64-x86-64 gcc-mingw-w64-x86-64 mingw-w64-x86-64-dev libwine-dev
# binutils-mingw-w64-x86-64/bionic,now 2.30-7ubuntu1+8ubuntu1 amd64
# g++-mingw-w64-x86-64/bionic,now 7.3.0-11ubuntu1+20.2build1 amd64
# gcc-mingw-w64-x86-64/bionic,now 7.3.0-11ubuntu1+20.2build1 amd64
# mingw-w64-x86-64-dev/bionic,bionic,now 5.0.3-1 all
# wine64/bionic,now 3.0-1ubuntu1 amd64
#
# use
# # rmills@rmillsmm-ubuntu:~/gnu/github/exiv2/exiv2/contrib/mingw-cross$ ./build.sh
#

echo PWD = $PWD
echo this = $0

COMPILER_PREFIX="x86_64-w64-mingw32"

CM_GENERAL="-DCMAKE_SYSTEM_NAME=Windows                 \
-DCMAKE_C_COMPILER=$(which ${COMPILER_PREFIX}-gcc)      \
-DCMAKE_CXX_COMPILER=$(which ${COMPILER_PREFIX}-g++)    \
-DCMAKE_RC_COMPILER=$(which ${COMPILER_PREFIX}-windres) \
-DCMAKE_MAKE_PROGRAM=$(which make)               \
-DCMAKE_FIND_ROOT_PATH="/usr/${COMPILER_PREFIX}" \
-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER        \
-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY         \
-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY         \
-DCMAKE_BUILD_TYPE=Release"


ZLIB=$( realpath ${PWD}/../../../zlib-1.2.11)
EXPAT=$(realpath ${PWD}/../../../expat-2.2.6)
EXIV2=$(realpath ${PWD}/../..)
clean=1

for D in $ZLIB $EXPAT $EXIV2; do
   if [ ! -e $D/CMakeLists.txt ]; then
     echo $D is wrong.  $D/CMakeLists.txt does not exist
     exit 1
   else
      echo YES $D/CMakeLists.txt exists
   fi
done

mkdir  -p build/zlib
mkdir  -p build/expat
mkdir  -p build/exiv2


# build zlib
pushd     build/zlib
echo ---- building zlib in $PWD -------------
if [ $clean == 1 ]; then rm -rf  * ; fi
cmake    "$ZLIB"     $CM_GENERAL           \
         -DCMAKE_INSTALL_PREFIX=$PWD

cmake    --build . --config Release
cmake    --build . --target install
popd

# build expat
pushd     build/expat
echo ---- building expat in $PWD -------------
if [ $clean == 1 ]; then rm -rf  * ; fi
cmake    "$EXPAT"  $CM_GENERAL             \
         -DBUILD_shared=Off                \
         -DCMAKE_INSTALL_PREFIX=$PWD

cmake    --build . --config Release
cmake    --build . --target install
popd

# build exiv2
pushd     build/exiv2
echo ---- building exiv2 in $PWD -------------
if [ $clean == 1 ]; then rm -rf  * ; fi
cmake    "$EXIV2"  $CM_GENERAL                              \
         -DBUILD_SHARED_LIBS=0                              \
         -DZLIB_INCLUDE_DIR=${PWD}/../zlib/include          \
         -DZLIB_LIBRARY=${PWD}/../zlib/lib/libzlibstatic.a  \
         -DEXPAT_INCLUDE_DIR=${PWD}/../expat/include        \
         -DEXPAT_LIBRARY=${PWD}/../expat/lib/libexpat.a     \
         -DEXIV2_ENABLE_DYNAMIC_RUNTIME=Off                 \
         -DCMAKE_INSTALL_PREFIX=$PWD

cmake    --build . --config Release
popd
cp       gcc64/*    build/exiv2/bin
wine     build/exiv2/bin/exiv2 --verbose --version

# That's all Folks!
##
