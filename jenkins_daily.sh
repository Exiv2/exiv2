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
  	cmd.exe /c "cd $build && vcvars 2013 64 && cmakeBuild --rebuild --exiv2=$exiv2 --webready"
  	result=$?
)

##
# test the build
pushd test
	./testMSVC.sh "$dist"
popd

exit $result
# That's all Folks!
##
