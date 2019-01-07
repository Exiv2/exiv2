#!/usr/bin/env bash


EXIV2=$(realpath ../..)
if [ ! -e $EXIV2/CMakeLists.txt ]; then 
   echo "***" file $EXIV2/CMakeLists.txt does not exist "***"
   exit 1
fi

mkdir -p conan-build
cd       conan-build
echo PWD = $PWD

# Save the profile in a file
cat - > conan-build.profile <<EOF
\$toolchain=/usr/bin
target_host=x86_64-w64-mingw32
cc_compiler=gcc
cxx_compiler=g++

[env]
CONAN_CMAKE_FIND_ROOT_PATH=$toolchain
CHOST=\$target_host
AR=\$target_host-ar
AS=\$target_host-as
RANLIB=\$target_host-ranlib
CC=\$target_host-$cc_compiler
CXX=\$target_host-$cxx_compiler
STRIP=\$target_host-strip
RC=\$target_host-windres

[settings]
# We are building in Ubuntu Linux
os_build=Linux
arch_build=x86_64

# We are cross building to Windows
os=Windows
arch=x86_64
compiler=gcc

# Adjust to the gcc version of your MinGW package
compiler.version=7.3
compiler.libcxx=libstdc++11
build_type=Release

EOF

# request conan to build/download/install dependencies
conan install ../../.. --build missing --profile conan-build.profile

# set up CMake cross compiler (see build.sh)
COMPILER_PREFIX="x86_64-w64-mingw32"

CM_GENERAL="-DCMAKE_SYSTEM_NAME=Windows                 \
-DCMAKE_C_COMPILER=$(which ${COMPILER_PREFIX}-gcc)      \
-DCMAKE_CXX_COMPILER=$(which ${COMPILER_PREFIX}-g++)    \
-DCMAKE_RC_COMPILER=$(which ${COMPILER_PREFIX}-windres) \
-DCMAKE_MAKE_PROGRAM=$(which make)                      \
-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER               \
-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY                \
-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY                \
-DCMAKE_BUILD_TYPE=Release"

# build exiv2
echo ---- building exiv2 in $PWD -------------
cmake    ../../.. $CM_GENERAL                           \
         -DBUILD_SHARED_LIBS=0                          \
         -DEXIV2_ENABLE_DYNAMIC_RUNTIME=Off             \
         -DCMAKE_INSTALL_PREFIX=$PWD

cmake    --build . --config Release
cp       ../gcc64/*.dll     bin/
wine     bin/exiv2 -vV

# That's all Folks!
##

