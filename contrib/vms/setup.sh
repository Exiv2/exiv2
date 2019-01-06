#!/bin/bash

set -e

distro_id=$(grep '^ID=' /etc/os-release|awk -F = '{print $2}'|sed 's/\"//g')

case "$distro_id" in
    'fedora')
        dnf -y --refresh install python3-pip git
        ;;

    'debian' | 'ubuntu')
        apt-get install -y python3-pip git
        ;;

    'arch')
        pacman --noconfirm -S python-pip git
        ;;

    'centos' | 'rhel')
        yum -y install centos-release-scl-rh
        yum clean all
        yum -y install rh-python36-python-pip git
        ;;

    'opensuse' | 'opensuse-tumbleweed')
        zypper --non-interactive install python3-pip git
        ;;

    'alpine')
        apk add python3 git python3-dev
        python3 -m ensurepip
        pip3 install --upgrade pip setuptools
        ;;

    *)
        echo "Sorry, no predefined dependencies for your distribution exist yet"
        exit 1
        ;;
esac
