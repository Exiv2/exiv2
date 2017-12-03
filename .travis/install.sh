#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get install cmake zlib1g-dev libssh-dev gettext
    sudo apt-get install python-pip libxml2-utils
    sudo pip install virtualenv
else
    brew update
    brew install gettext libssh md5sha1sum
    brew install pyenv-virtualenv
    # By default it already has cmake 3.6.2
fi

virtualenv conan
source conan/bin/activate
pip install conan==0.27.0
conan --version
conan config set storage.path=~/conanData
conan remote add conan-pix4d https://api.bintray.com/conan/pix4d/conan
mkdir -p ~/.conan/profiles

if [[ "$(uname -s)" == 'Linux' ]]; then
    printf "os=Linux\narch=x86_64\ncompiler=gcc\ncompiler.version=4.8\nbuild_type=Release\n" > ~/.conan/profiles/release
else
    printf "os=Macos\narch=x86_64\ncompiler=apple-clang\ncompiler.version=7.3\nbuild_type=Release\n" > ~/.conan/profiles/release
fi

