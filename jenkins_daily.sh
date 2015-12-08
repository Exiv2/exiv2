#!/bin/bash

##
# jenkins_daily.sh
# At the moment, this performs a CMake/msvc build
##

##
#  environment
#  JENKINS   : URL of jenkins server. Default http://exiv2.dyndns.org:8080
##
if [ -z "$JENKINS" ]; then export JENKINS=http://exiv2.dyndns.org:8080; fi


##
# determine location of the build and source directories
exiv2=$(cygpath -aw .)
build=$(cygpath -aw c:\\temp\\build)
 dist=$(cygpath -au  c:\\temp\\build\\dist)

##
# create a clean directory for the out-of-source build
rm    -rf $build
mkdir -p  $build

##
# get the windows cmd to perform the build
# set the path to find svn/7z/cmake and cmd.exe
(
  	PATH="c:\\gnu\\exiv2\\trunk\\contrib\\cmake\\msvc;c:\\Program files\\csvn\\bin;c:\\program files\\7-zip;c:\\program files (x86)\\cmake\\bin;$PATH:/cygdrive/c/Windows/System32;"
  	result=0
  	/cygdrive/c/Windows/System32/cmd.exe /c "cd $build && vcvars 2015 64 && cmakeBuild --rebuild --exiv2=$exiv2"
  	result=$?
)

##
# test the build
pushd test
	./testMSVC.sh "$dist"
popd

# That's all Folks!
##
exit "$result"
