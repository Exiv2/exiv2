#!/bin/sh

set -e

# Debian & derivatives don't provide binary packages of googletest
# => have to build them ourselves
#
# This script builds a shared library of googletest (not googlemock!) inside
# gtest_build and copies it to /usr/lib/
debian_build_gtest() {
    [ -d gtest_build ] || mkdir gtest_build
    cd gtest_build
    cmake -DBUILD_SHARED_LIBS=1 /usr/src/googletest/googletest
    make
    cp libgtest* /usr/lib/
    cd ..
}

distro_id=$(grep '^ID=' /etc/os-release|awk -F = '{print $2}'|sed 's/\"//g')

case "$distro_id" in
    'fedora')
        dnf -y --refresh install gcc-c++ clang cmake make ccache expat-devel zlib-devel libssh-devel libcurl-devel gtest-devel which dos2unix
        ;;

    'debian')
        apt-get update
        apt-get install -y cmake g++ clang make ccache python3 libexpat1-dev zlib1g-dev libssh-dev libcurl4-openssl-dev libgtest-dev libxml2-utils
        debian_build_gtest
        ;;

    'arch')
        pacman --noconfirm -Sy
        pacman --noconfirm -S gcc clang cmake make ccache expat zlib libssh curl gtest python dos2unix
        ;;

    'ubuntu')
        apt-get update
        apt-get install -y cmake g++ clang make ccache python3 libexpat1-dev zlib1g-dev libssh-dev libcurl4-openssl-dev libgtest-dev google-mock libxml2-utils
        debian_build_gtest
        ;;

    'centos'|'rhel')
        yum -y install epel-release
        # enable copr for gtest
        curl https://copr.fedorainfracloud.org/coprs/defolos/devel/repo/epel-7/defolos-devel-epel-7.repo > /etc/yum.repos.d/_copr_defolos-devel.repo
        yum clean all
        yum -y install gcc-c++ clang cmake3 make ccache expat-devel zlib-devel libssh-devel libcurl-devel gtest-devel which python36 dos2unix
        # symlink up to date versions of python & cmake to 'default' names
        if [ ! -e /usr/bin/python3 ]; then
            ln -s /usr/bin/python36 /usr/bin/python3
        elif [ -L /usr/bin/python3 ]; then
            rm /usr/bin/python3
            ln -s /usr/bin/python36 /usr/bin/python3
        fi
        mv /bin/cmake /bin/.cmake.old
        ln -s /bin/cmake3 /bin/cmake
        ;;

    'opensuse'|'opensuse-tumbleweed')
        zypper --non-interactive refresh
        zypper --non-interactive install gcc-c++ clang cmake make ccache libexpat-devel zlib-devel libssh-devel libcurl-devel gtest which dos2unix libxml2-tools
        ;;

    'alpine')
        apk update
        apk add gcc g++ clang cmake make ccache expat-dev zlib-dev libssh-dev curl-dev gtest gtest-dev libintl gettext-dev which dos2unix bash libxml2-utils diffutils
        ;;
    *)
        echo "Sorry, no predefined dependencies for your distribution $distro_id exist yet"
        exit 1
        ;;
esac
