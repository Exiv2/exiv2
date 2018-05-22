#!/bin/bash

set -e

source /vagrant/utils.source

distro_id=$(get_distro_id)

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

    *)
        echo "Sorry, no predefined dependencies for your distribution exist yet"
        exit 1
        ;;
esac
