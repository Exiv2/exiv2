#!/bin/bash
set -e
set -x

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get install cmake zlib1g-dev libssh-dev libcurl4-openssl-dev gettext libexpat1-dev
else
    brew update
    brew install expat gettext libssh
    # By default it already has cmake 3.6.2
fi

