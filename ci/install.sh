#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

# This file is only used from Travis CI, where the only Linux distro used is Ubuntu

python3 --version

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update

    sudo apt-get install cmake zlib1g-dev libssh-dev python3-pip libxml2-utils
    
    if [ -n "$WITH_VALGRIND" ]; then
        # https://travis-ci.community/t/clang-10-was-recently-broken-on-linux-unmet-dependencies-for-clang-10-clang-tidy-10-valgrind/11527
        sudo apt-get install -yq --allow-downgrades libc6=2.31-0ubuntu9.2 libc6-dev=2.31-0ubuntu9.2
        sudo -E apt-get -yq --no-install-suggests --no-install-recommends --allow-downgrades --allow-remove-essential --allow-change-held-packages install valgrind
    fi
    sudo pip3 install virtualenv
    virtualenv conan
    source conan/bin/activate
    pip install conan==1.30.2
    pip install codecov
    pip install lxml
else
    sudo pip3 install virtualenv
    virtualenv conan
    source conan/bin/activate
    pip3 install conan==1.30.2
    pip3 install codecov
    pip3 install lxml
fi

conan --version
conan config set storage.path=~/conanData
conan profile new default --detect

if [[ "$(uname -s)" == 'Linux' ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi

