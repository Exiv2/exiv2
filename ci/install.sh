#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

python --version
python3 --version

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update
    sudo apt-get install libssh-dev python-pip libxml2-utils
    if [ -n "$WITH_VALGRIND" ]; then
        sudo apt-get install valgrind
    fi
    sudo pip install virtualenv
    virtualenv conan
    source conan/bin/activate
    pip install conan==1.22.0
    pip install codecov
else
    sudo pip3 install virtualenv
    virtualenv conan
    source conan/bin/activate
    pip3 install conan==1.22.0
    pip3 install codecov
fi

conan --version
conan config set storage.path=~/conanData
conan profile new default --detect

if [[ "$(uname -s)" == 'Linux' ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi
