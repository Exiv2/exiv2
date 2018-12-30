#!/bin/bash
set -e # Enables cheking of return values from each command
set -x # Prints every command

if [[ "$(uname -s)" == 'Linux' ]]; then
    sudo apt-get update
    sudo apt-get install cmake zlib1g-dev libssh-dev gettext expat libcurl4-openssl-dev libxml2-utils
    sudo pip install virtualenv
else
    brew update
    brew install gettext expat zlib curl md5sha1sum
    brew upgrade python
fi
