#!/bin/bash

set -e

function clone_exiv2() {
    git clone https://github.com/Exiv2/exiv2.git

    cd exiv2
    sed -i '/fetch = +refs\/heads\/\*:refs\/remotes\/origin\//a \ \ \ \ \ \ \ \ fetch = +refs\/pull\/\*\/head:refs\/remotes\/origin\/pr\/*' .git/config
    cd ..
}

distro_id=$(grep '^ID=' /etc/os-release|awk -F = '{print $2}'|sed 's/\"//g')

case "$distro_id" in
    'debian' | 'ubuntu' | 'fedora' | 'opensuse' | 'opensuse-tumbleweed' | 'alpine')
        PIP=pip3
        ;;

    'arch')
        PIP=pip
        ;;

    *)
        echo "Sorry, no predefined dependencies for your distribution exist yet"
        exit 1
        ;;
esac

$PIP install conan --user --upgrade

CONAN_PROFILE=~/.conan/profiles/default

# create a new conan profile & set the used libstdc++ to use the C++11 ABI
[ -e $CONAN_PROFILE ] || ~/.local/bin/conan profile new --detect default
sed -i 's/compiler.libcxx=libstdc++/compiler.libcxx=libstdc++11/' $CONAN_PROFILE

[ -d exiv2 ] || clone_exiv2

cd exiv2 && git fetch && cd ..
