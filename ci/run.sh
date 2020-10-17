#!/bin/bash

set -e
set -x

source conan/bin/activate

if [[ "$(uname -s)" == 'Linux' ]]; then
    if [ "$CC" == "clang" ]; then
        # clang + Ubuntu don't like to run with UBSAN, but ASAN works
        export CMAKE_OPTIONS="$COMMON_CMAKE_OPTIONS"
    elif [ -n "$WITH_VALGRIND" ]; then
        export EXIV2_VALGRIND="valgrind --quiet"
    else
        export CMAKE_OPTIONS="$COMMON_CMAKE_OPTIONS -DEXIV2_TEAM_USE_SANITIZERS=OFF"
    fi
else
    export CMAKE_OPTIONS="$COMMON_CMAKE_OPTIONS -DEXIV2_TEAM_USE_SANITIZERS=OFF"
fi
CMAKE_OPTIONS="$COMMON_CMAKE_OPTIONS -DCMAKE_CXX_FLAGS=-Wno-deprecated -DCMAKE_CXX_STANDARD=98 -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

mkdir build
cd    build
conan install .. -o webready=True --build missing
cmake ${CMAKE_OPTIONS} ..
make  -j
make  tests
make  install

# Check for detecting issues with the installation of headers
if [ `ls install/include/exiv2/ | wc -l` > 10 ]; then
    echo Headers installed correctly
else
    echo There was some problem with the installation of the public headers
    exit 1
fi

if [ -n "$COVERAGE" ]; then
    bash <(curl -s https://codecov.io/bash)
fi

