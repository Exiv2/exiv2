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
 dist=$(cygpath -au .\\build\\dist\\$vs\\$arch\\$mode\\$config\\bin)
 di32=$(cygpath -au .\\build\\dist\\$vs\\Win32\\$mode\\$config\\bin)
 di64=$(cygpath -au .\\build\\dist\\$vs\\x64\\$mode\\$config\\bin)
 msvc=$(cygpath -aw ./contrib/cmake/msvc)

##
# create a clean directory for an out-of-source build
rm    -rf $dist
mkdir -p  $dist

##
# get windows cmd.exe to perform the build
# use a sub-shell to temporarily set path for svn/7z/cmake/cmd
(
  	PATH="$msvc:c:\\Program Files\\csvn\\bin:c:\\Program Files\\7-zip:c:\\Program Files (x86)\\cmake\\bin:$PATH:/cygdrive/c/Windows/System32"
  	result=0
  	cmd.exe /c "cd $build && vcvars $vs $arch && cmakeBuild --rebuild --exiv2=$exiv2 $*"
  	result=$?
)

##
# test the build
if [ -e $dist/exiv2.exe ]; then
    pushd  test
    export EXIV2_BINDIR=$dist
    for test in addmoddel.sh \
        bugfixes-test.sh     \
        exifdata-test.sh     \
        exiv2-test.sh        \
        imagetest.sh         \
        iotest.sh            \
        iptctest.sh          \
        modify-test.sh       \
        path-test.sh         \
        preview-test.sh      \
        stringto-test.sh     \
        tiff-test.sh         \
        write-test.sh        \
        write2-test.sh       \
        xmpparser-test.sh    \
        conversions.sh
    do
      echo '++' $test '++' ; ./$test
    done
    popd

    $EXIV2_BINDIR/exiv2 -vV
    ls -alt $EXIV2_BINDIR
    $EXIV2_BINDIR/exiv2 -vV -g date -g time -g version
    ls -alt $EXIV2_BINDIR/exiv2.exe
else
    echo ""
    echo "**** no build created ****"
    echo ""
fi

exit $result
# That's all Folks!
##
