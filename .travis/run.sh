#!/bin/bash

set -e
set -x

source conan/bin/activate
mkdir build && cd build
conan install .. --build missing --profile release
cmake ${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=install ..
make -j2
make tests
make install
cd bin
./unit_tests
