#!/bin/bash

##
# jenkins_daily.sh

##
# configure the build (only used for msvc builds)
arch=x64
mode=dll
config=Release
vs=2013

result=0

##
# which PLATFORM
# JOB_NAME is defined when script is called by Jenkins
# example: JOB_NAME=trunk-cmake-daily/label=msvc
# PLATFORM must be defined as msvc when called from ssh
if [ ! -z "$JOB_NAME" ];then 
    PLATFORM=$(echo $JOB_NAME | cut -d= -f 2)
fi
if [ "$PLATFORM" == "" ]; then 
    export PLATFORM=''
    if [ `uname` == Darwin  ]; then
        PLATFORM=macosx
    elif [ `uname -o` == Cygwin ]; then
        PLATFORM=cygwin
        # tweak path to ensure the correct version of perl and expr for autotools
        export "PATH=/bin:$PATH"
    elif [ `uname -o` == Msys ]; then
        PLATFORM=mingw
    else
        PLATFORM=linux
    fi
fi

##
# determine location of the build and source directories
if [ "$PLATFORM" == "msvc" ]; then
    exiv2=$(cygpath -aw .)
    build=$(cygpath -aw ./build)
     dist=$(cygpath -au ./build/dist/$vs/$arch/$mode/$config/bin)
     msvc=$(cygpath -au ./contrib/cmake/msvc)
     exe=.exe
     bin=''
else
	PATH="/usr/local/bin:$PATH"
    exiv2=$PWD
    build=$PWD/build
    dist=$PWD/build/dist/$PLATFORM
    exe=''
    bin=bin
    if [ -e $exiv2/CMakeCache.txt ]; then rm -rf $exiv2/CMakeCache.txt ; fi 
fi

##
# create a clean directory for an out-of-source build
rm    -rf $dist
mkdir -p  $dist

echo "---- dist = $dist ------"
echo "---- build = $build ------"

##
# perform the build
if [ "$PLATFORM" == "msvc" ]; then
    ##
    # get windows cmd.exe to perform the build
    # use a subshell to restore the path
    (
        PATH="$msvc:/cygdrive/c/Program Files/csvn/bin:/cygdrive/c/Program Files (x86)/WANdisco/Subversion/csvn/bin:/cygdrive/c/Program Files/7-zip:/cygdrive/c/Program Files (x86)/cmake/bin:$PATH:/cygdrive/c/Windows/System32"
        cmd.exe /c "cd $build && vcvars $vs $arch && cmakeBuild --rebuild --exiv2=$exiv2 $*"
        result=$?
    )
else
    pushd $build > /dev/null
    cmake -DCMAKE_INSTALL_PREFIX=$dist -DEXIV2_ENABLE_NLS=OFF $exiv2
    make
    cmake --build . --target install 
    popd > /dev/null
fi

##
# test the build
if [ -e $dist/$bin/exiv2$exe ]; then
    pushd  test > /dev/null
    # EXIV2_BINDIR is used by the test suite to locate executables
    export EXIV2_BINDIR=$dist/$bin
    # set LD_LIBRARY_PATH (and DYLD_LIBRARY_PATH for macosx)
    # to be sure we run the tests with the newly built library
	export DYLD_LIBRARY_PATH=$dist/lib
	export LD_LIBRARY_PATH=$dist/lib
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
    popd > /dev/null

    $EXIV2_BINDIR/exiv2 -vV
    ls -alt $EXIV2_BINDIR
    $EXIV2_BINDIR/exiv2 -vV -g date -g time -g version
    ls -alt $EXIV2_BINDIR/exiv2$exe

    ##
    # store the build for users to collect
    mmHD=""
    if [ "$PLATFORM" == "linux" ]; then
    	mmHD=/media/psf/Host
    fi
    if [ "$PLATFORM" == "msvc" -o  "$PLATFORM" == "cygwin" ]; then
        mmHD="//psf/Host/"
    fi
    jpubl=$mmHD/Users/Shared/Jenkins/Home/userContent/builds

    daily=$jpubl/daily
    weely=$jpubl/weekly
    monly=$jpubl/monthly
    if [ ! -e $jpubl ]; then mkdir -p $jpubl ; fi
    if [ ! -e $daily ]; then mkdir -p $daily ; fi
    if [ ! -e $weely ]; then mkdir -p $weely ; fi
    if [ ! -e $monly ]; then mkdir -p $monly ; fi

    if [ -e $jpubl ]; then
        # parse output of date: Thu 10 Dec 2015 14:02:51 GMT
        dow=$(date|cut -d' ' -f 1) # Thu   day of the week
        dom=$(date|cut -d' ' -f 2) # 10    day of the month
        mon=$(date|cut -d' ' -f 2) # Dec   month
        svn=$($EXIV2_BINDIR/exiv2$exe -vVg|grep -e ^svn | cut -d= -f 2)
        date=$(date +'%Y-%m-%d+%H-%M-%S')
        b="${PLATFORM}-svn-${svn}-date-${date}.tar.gz"
        echo build = "$b"

        # create the bundle
        pushd build
        if [ -e "$b" ]; then rm -rf "$b"; fi
        tar czf "$b" dist/
        ls -alt
        mv   $b ..
        popd

        # clean userContent/build directories
        # daily > 50 days; weekly > 1 year;   monthly > 5 years
        if [ -e $daily ]; then find $daily -type f -ctime +50          -exec rm -rf {} \; ; fi
        if [ -e $weely ]; then find $weely -type f -ctime +365         -exec rm -rf {} \; ; fi
        if [ -e $monly ]; then find $monly -type f -ctime $((366 * 5)) -exec rm -rf {} \; ; fi

        # store the build
        cp $b $daily
        if [ "$dow" == "Mon" ]; then cp $b $weely; fi
        if [ "$dom" == "1"   ]; then cp $b $monly; fi

        echo '***' build = $b '***'
    else
    	echo '***' jenkins builds directory does not exist ${jpubl} '***'
    	result=2
    fi
else
    echo ''
    echo '**** no build created ****'
    echo ''
    result=1
fi

exit $result
# That's all Folks!
##
