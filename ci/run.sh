#!/bin/bash

set -e
set -x

if [[ "$(uname -s)" == 'Linux' ]]; then
    source conan/bin/activate
else
    export PYENV_VERSION=$PYTHON
    export PATH="/Users/travis/.pyenv/shims:${PATH}"
    eval "$(pyenv init -)"
    eval "$(pyenv virtualenv-init -)"
    pyenv activate conan
fi


mkdir build && cd build
conan install .. --build missing --profile release
cmake ${CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=install ..
make -j2 VERBOSE=1

#On most systems, you can set the TZ environment variable to set the timezone for a process. It's a POSIX feature.
export TZ=UTC

make tests
make install
cd bin
./unit_tests

if [ -n "$COVERAGE" ]; then
    cd ..
    bash <(curl -s https://codecov.io/bash)
fi
