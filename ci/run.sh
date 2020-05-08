#!/bin/bash

set -e
set -x

source conan/bin/activate

if [[ "$(uname -s)" == 'Linux' ]]; then
    if [ "$CC" == "clang" ]; then
        # clang + Ubuntu don't like to run with UBSAN, but ASAN works
        export CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_CXX_FLAGS=\"-fsanitize=address\" -DCMAKE_C_FLAGS=\"-fsanitize=address\" -DCMAKE_EXE_LINKER_FLAGS=\"-fsanitize=address\" -DCMAKE_MODULE_LINKER_FLAGS=\"-fsanitize=address\""
    elif [ -n "$WITH_VALGRIND" ]; then
        export EXIV2_VALGRIND="valgrind --quiet"
    else
        export CMAKE_OPTIONS="$CMAKE_OPTIONS -DEXIV2_TEAM_USE_SANITIZERS=ON"
    fi
else
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DEXIV2_TEAM_USE_SANITIZERS=ON"
fi

mkdir build
cd    build
conan install .. -o webready=True --build missing
cmake ${CMAKE_OPTIONS} -DEXIV2_TEAM_WARNINGS_AS_ERRORS=ON -DCMAKE_INSTALL_PREFIX=install ..
make
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

