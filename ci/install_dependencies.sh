#!/bin/bash

set -e

# this script expects one parameter, which is the path to utils.source
# if the parameter is omitted, it defaults to the relative path in the
# repository, but it can be provided manually (e.g. for vagrant VMs)

if [ $# -eq 0 ]; then
    source $(dirname "${BASH_SOURCE[0]}")/../contrib/vms/utils.source
elif [ $# -gt 1 ]; then
    cat << EOF
usage: install_dependencies.sh [path/to/utils.source]

Installs the dependencies required to build & test exiv2 on some Linux
distributions.
The optional parameter is the path to the utils.source file, which can be set to
an alternative location (currently used for vagrant builds)
EOF
    exit 1
else
    source "$1"
fi

distro_id=$(get_distro_id)

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
    *)
        echo "Sorry, no predefined dependencies for your distribution $distro_id exist yet"
        exit 1
        ;;
esac
