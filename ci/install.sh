#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update
    sudo apt-get install libssh-dev python-pip libxml2-utils
    if [ -n "$WITH_VALGRIND" ]; then
        sudo apt-get install valgrind
    fi
    sudo pip install virtualenv
    virtualenv conan
    source conan/bin/activate
else
    brew update
    brew install md5sha1sum pyenv-virtualenv gettext
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
pip install conan==1.11.1
pip install codecov
conan --version
conan config set storage.path=~/conanData
conan profile new default --detect

if [[ "$(uname -s)" == 'Linux' ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi
