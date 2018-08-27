#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update
    sudo apt-get install cmake zlib1g-dev libssh-dev gettext
    sudo apt-get install python-pip libxml2-utils
    sudo pip install virtualenv
    virtualenv conan
    source conan/bin/activate
else
    brew update
    brew install gettext md5sha1sum pyenv-virtualenv
    export CFLAGS="-I/usr/local/opt/openssl/include $CFLAGS"
    export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
    pyenv install $PYTHON
    # I would expect something like ``pyenv init; pyenv local $PYTHON`` or
    # ``pyenv shell $PYTHON`` would work, but ``pyenv init`` doesn't seem to
    # modify the Bash environment. ??? So, I hand-set the variables instead.
    export PYENV_VERSION=$PYTHON
    export PATH="/Users/travis/.pyenv/shims:${PATH}"
    eval "$(pyenv init -)"
    eval "$(pyenv virtualenv-init -)"
    pyenv virtualenv conan
    pyenv activate conan
fi

python --version
pip install urllib3[secure] -U #Should solve SSL issues
pip install conan==1.6.1
pip install codecov
conan --version
conan config set storage.path=~/conanData
conan remote add conan-bincrafters https://api.bintray.com/conan/bincrafters/public-conan

mkdir -p ~/.conan/profiles

if [[ "$(uname -s)" == 'Linux' ]]; then
    CC_VER=$(${CC} --version | head -1 | awk '{print $3}'| awk -F'.' '{ print $1"."$2 }')
    printf "[settings]\nos=Linux\narch=x86_64\ncompiler=$CC\ncompiler.version=$CC_VER\ncompiler.libcxx=libstdc++\nbuild_type=Release\n" > ~/.conan/profiles/release
else
    printf "[settings]\nos=Macos\narch=x86_64\ncompiler=apple-clang\ncompiler.version=9.0\ncompiler.libcxx=libc++\nbuild_type=Release\n" > ~/.conan/profiles/release
fi

