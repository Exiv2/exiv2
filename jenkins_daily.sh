#!/bin/bash

##
# jenkins_daily.sh
# At the moment, this performs a CMake/msvc build
##

##
#  environment
#  JENKINS   : URL of jenkins server. Default http://exiv2.dyndns.org:8080
if [ -z "$JENKINS" ]; then export JENKINS=http://exiv2.dyndns.org:8080; fi

##
# configure the build
arch=x64
mode=dll
config=Release
vs=2013

##
# determine location of the build and source directories
exiv2=$(cygpath -aw .)
build=$(cygpath -aw .\\build)
 dist=$(cygpath -au .\\build\\dist)
 msvc=$(cygpath -aw ./contrib/cmake/msvc)

##
# create a clean directory for an out-of-source build
rm    -rf $build
mkdir -p  $build

##
# get windows cmd.exe to perform the build
# use a sub-shell to temporarily set path for svn/7z/cmake/cmd
(
  	PATH="$msvc:c:\\Program Files\\csvn\\bin:c:\\Program Files\\7-zip:c:\\Program Files (x86)\\cmake\\bin:$PATH:/cygdrive/c/Windows/System32"
  	result=0
  	cmd.exe /c "cd $build && vcvars $vs $arch && cmakeBuild --rebuild --exiv2=$exiv2"
  	result=$?
)

##
# test the build
export EXIV2_BINDIR=$dist/$vs/$arch/$mode/$config/bin
pushd  test
    for t in $(ls *.sh|sort); do ./$i ; done
popd
$EXIV2_BINDIR/exiv2 -vV

exit $result
# That's all Folks!
##
