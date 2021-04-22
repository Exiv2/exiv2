#!/bin/sh -e

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
    if [ -f "lib/libgtest.so" ]; then
        # Ubuntu 20.04 with gtest 1.10
        cp lib/libgtest* /usr/lib/
    else
        # Debian 9 with gtest 1.8
        cp libgtest* /usr/lib/
    fi
    cd ..
}

# workaround for really bare-bones Archlinux containers:
if [ -x "$(command -v pacman)" ]; then
    pacman --noconfirm -Sy
    pacman --noconfirm -S grep gawk sed
fi

distro_id=$(grep '^ID=' /etc/os-release|awk -F = '{print $2}'|sed 's/\"//g')

case "$distro_id" in
    'fedora')
        dnf -y --refresh install gcc-c++ clang cmake make ccache expat-devel zlib-devel libssh-devel libcurl-devel gtest-devel which dos2unix glibc-langpack-en diffutils
        ;;

    'debian')
        apt-get update
        apt-get install -y cmake g++ clang make ccache python3 libexpat1-dev zlib1g-dev libssh-dev libcurl4-openssl-dev libgtest-dev libxml2-utils
        debian_build_gtest
        ;;

    'arch')
        pacman --noconfirm -Syu
        pacman --noconfirm -S gcc clang cmake make ccache expat zlib libssh curl gtest python dos2unix which diffutils
        ;;

    'ubuntu')
        apt-get update
        apt-get install -y cmake g++ clang make ccache python3 libexpat1-dev zlib1g-dev libssh-dev libcurl4-openssl-dev libgtest-dev google-mock libxml2-utils
        debian_build_gtest
        ;;

    'alpine')
        apk update
        apk add gcc g++ clang cmake make ccache expat-dev zlib-dev libssh-dev curl-dev gtest gtest-dev gmock libintl gettext-dev which dos2unix bash libxml2-utils diffutils python3
        ;;

    'centos'|'rhel')
        yum -y install epel-release
        # enable copr for gtest
        curl https://copr.fedorainfracloud.org/coprs/defolos/devel/repo/epel-7/defolos-devel-epel-7.repo > /etc/yum.repos.d/_copr_defolos-devel.repo
        yum clean all
        yum -y install gcc-c++ clang cmake3 make ccache expat-devel zlib-devel libssh-devel libcurl-devel gtest-devel which python3 dos2unix
        # symlink up to date versions of cmake to 'default' name
        mv /bin/cmake /bin/.cmake.old
        ln -s /bin/cmake3 /bin/cmake
        ;;

    'opensuse'|'opensuse-tumbleweed')
        zypper --non-interactive refresh
        zypper --non-interactive install gcc-c++ clang cmake make ccache libexpat-devel zlib-devel libssh-devel curl tar libcurl-devel git which dos2unix libxml2-tools gzip
        pushd /tmp
          curl -LO https://github.com/google/googletest/archive/release-1.8.0.tar.gz
          tar xzf   release-1.8.0.tar.gz
          mkdir -p  googletest-release-1.8.0/build
          pushd     googletest-release-1.8.0/build
            cmake .. ; make ; make install
          popd
        popd
        ;;
    *)
        echo "Sorry, no predefined dependencies for your distribution $distro_id exist yet"
        exit 1
        ;;
esac
