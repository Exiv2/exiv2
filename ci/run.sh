#!/bin/bash

set -e
set -x

export CMAKE_OPTIONS="$COMMON_CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ -n "$WITH_COVERAGE" ]; then
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_WITH_COVERAGE=ON"
fi

if [ -n "$WITH_SANITIZERS" ]; then
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DEXIV2_TEAM_USE_SANITIZERS=ON"
fi

if [ -n "$WITH_VALGRIND" ]; then
    export EXIV2_VALGRIND="valgrind --quiet"
fi


if [[ "$(uname -s)" == 'Linux' ]]; then
    source conan/bin/activate
else
    export CMAKE_OPTIONS="$CMAKE_OPTIONS -DEXIV2_ENABLE_NLS=OFF"
    export PYENV_VERSION=$PYTHON
    export PATH="/Users/travis/.pyenv/shims:${PATH}"
    eval "$(pyenv init -)"
    eval "$(pyenv virtualenv-init -)"
    pyenv activate conan
fi


mkdir build && cd build
conan install .. --build missing

cmake ${CMAKE_OPTIONS} ..
make -j2

make tests
make install

# Check for detecting issues with the installation of headers
if [ `ls install/include/exiv2/ | wc -l` > 10 ]; then
    echo Headers installed correctly
else
    echo There was some problem with the installation of the public headers
    exit 1
fi

pushd .
cd bin
$EXIV2_VALGRIND ./unit_tests
popd

if [ -n "$WITH_COVERAGE" ]; then
    bash <(curl -s https://codecov.io/bash)
fi

