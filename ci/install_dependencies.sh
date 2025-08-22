#!/bin/sh -e

# Debian & derivatives don't provide binary packages of googletest
# => have to build them ourselves
#
# This script builds a shared library of googletest (not googlemock!) inside
# gtest_build and copies it to /usr/lib/
debian_build_gtest() {
    [ -d gtest_build ] || mkdir gtest_build
    cd gtest_build
    cmake -GNinja -DBUILD_SHARED_LIBS=1 /usr/src/googletest/googletest
    cmake --build .
    if [ -f "lib/libgtest.so" ]; then
        # Ubuntu 20.04 with gtest 1.10
        cp lib/libgtest* /usr/lib/
    else
        # Debian 9 with gtest 1.8
        cp libgtest* /usr/lib/
    fi
    cd ..
}

# Centos doesn't have a working version of the inih library, so we need to build it ourselves.
centos_build_inih() {
    [-d inih_build ] || git clone https://github.com/benhoyt/inih.git inih_build
    cd inih_build
    git checkout r58
    meson --buildtype=plain builddir
    meson compile -C builddir
    meson install -C builddir
    cd ..
}

# workaround for really bare-bones Archlinux containers:
if [ -x "$(command -v pacman)" ]; then
    pacman --noconfirm -Sy
    pacman --noconfirm --needed -S grep gawk sed
fi

distro_id=$(grep '^ID=' /etc/os-release|awk -F = '{print $2}'|sed 's/\"//g')

case "$distro_id" in
    'fedora')
        dnf -y --refresh install gcc-c++ clang cmake ninja-build expat-devel zlib-devel brotli-devel libssh-devel libcurl-devel gmock-devel glibc-langpack-en inih-devel fmt-devel
        ;;

    'debian')
        apt-get update
        apt-get install -y cmake ninja-build g++ clang libexpat1-dev zlib1g-dev libbrotli-dev libssh-dev libcurl4-openssl-dev libgmock-dev libxml2-utils libinih-dev libfmt-dev
        # debian_build_gtest
        ;;

    'arch')
        pacman --noconfirm -Syu
        pacman --noconfirm --needed -S gcc clang cmake ninja expat zlib brotli libssh curl gtest libinih fmt
        ;;

    'ubuntu')
        apt-get update
        apt-get install -y cmake ninja-build g++ clang libexpat1-dev zlib1g-dev libbrotli-dev libssh-dev libcurl4-openssl-dev libgmock-dev libxml2-utils libinih-dev libfmt-dev
        # debian_build_gtest
        ;;

    'alpine')
        apk update
        apk add gcc g++ clang cmake samurai expat-dev zlib-dev brotli-dev libssh-dev curl-dev gtest gtest-dev gmock libintl gettext-dev libxml2-utils inih-dev inih-inireader-dev fmt-dev
        ;;

    'rhel')
        dnf clean all
        dnf -y install gcc-c++ clang cmake ninja-build expat-devel zlib-devel brotli-devel libssh-devel libcurl-devel inih-devel fmt-devel
        ;;

    'centos')
        dnf clean all
        dnf -y install gcc-c++ clang cmake expat-devel zlib-devel brotli-devel libssh-devel libcurl-devel git fmt-devel
        dnf -y --enablerepo=crb install ninja-build meson
        centos_build_inih
        ;;

    'opensuse-tumbleweed')
        zypper --non-interactive refresh
        zypper --non-interactive install gcc-c++ clang cmake ninja libexpat-devel zlib-devel libbrotli-devel libssh-devel libcurl-devel gmock libxml2-tools libinih-devel libfmt-devel
        ;;
    *)
        echo "Sorry, no predefined dependencies for your distribution $distro_id exist yet"
        exit 1
        ;;
esac
