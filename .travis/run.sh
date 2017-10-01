#!/bin/bash

set -e
set -x

source conan/bin/activate
mkdir build && cd build
conan install .. --build missing --profile release
cmake ${CMAKE_OPTIONS} ..
cmake -DCMAKE_INSTALL_PREFIX=install ..
make -j
make tests
make install
