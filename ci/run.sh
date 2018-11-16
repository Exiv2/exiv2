#!/bin/bash

set -e
set -x

if [[ "$(uname -s)" == 'Linux' ]]; then
    source conan/bin/activate

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
    export PYENV_VERSION=$PYTHON
    export PATH="/Users/travis/.pyenv/shims:${PATH}"
    eval "$(pyenv init -)"
    eval "$(pyenv virtualenv-init -)"
    pyenv activate conan
fi


mkdir build && cd build
conan install .. --build missing --profile release

cmake ${CMAKE_OPTIONS} -DEXIV2_TEAM_WARNINGS_AS_ERRORS=ON -DCMAKE_INSTALL_PREFIX=install ..
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

if [ -n "$COVERAGE" ]; then
    bash <(curl -s https://codecov.io/bash)
fi

