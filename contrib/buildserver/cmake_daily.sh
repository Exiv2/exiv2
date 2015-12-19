#!/bin/bash

##
# jenkins_daily.sh
##

##
# configure the build (only used for msvc builds)
arch=x64
mode=dll
config=Release
vs=2013

result=0
source functions.so

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
mkdir -p  $build/dist/logs

echo "---- dist = $dist ------"
echo "---- build = $build ------"

##
# perform the build
(
  if [ "$PLATFORM" == "msvc" ]; then
    ##
    # get windows cmd.exe to perform the build
    # use a subshell to restore the path
    (
      PATH="$msvc:/cygdrive/c/Program Files/csvn/bin:/cygdrive/c/Program Files (x86)/WANdisco/Subversion/csvn/bin:/cygdrive/c/Program Files/7-zip:/cygdrive/c/Program Files (x86)/cmake/bin:$PATH:/cygdrive/c/Windows/System32"
      cmd.exe /c "cd $build && vcvars $vs $arch && cmakeBuild --rebuild --exiv2=$exiv2 $*"
      result=$?
      cp     $msvc/vcvars.bat $build/dist # required by test_daily.sh
    )
  else
    pushd $build > /dev/null
    (
      # build 64 bit library
      export CFLAGS=-m64
      export CXXFLAGS=-m64
      export LDFLAGS=-m64
      # Always use /usr/local/bin/cmake
      # I can guarantee it to be at least 3.4.1
      # because I built it from source and installed it
      /usr/local/bin/cmake -DCMAKE_INSTALL_PREFIX=$dist -DEXIV2_ENABLE_NLS=OFF $exiv2
      make
      /usr/local/bin/cmake --build . --target install
    )
    popd > /dev/null
  fi
) | tee "$build/dist/logs/build.log"

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
    (
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
    ) | tee "$build/dist/logs/test.log"

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
        dow=$(date  '+%w') # 0..6   day of the week
        dom=$(date  '+%d') # 1..31  day of the month
        mon=$(date  '+%m') # 1..12  month
        date=$(date '+%Y-%m-%d+%H-%M-%S')
        svn=$($EXIV2_BINDIR/exiv2$exe -vV | grep -e ^svn | cut -d= -f 2)
        b="${PLATFORM}-svn-${svn}-date-${date}.tar.gz"

        # add documentation and samples to dist
        cat contrib/buildserver/dailyReadMe.txt | sed -E -e "s/__BUILD__/$b/"  > "$build/dist/ReadMe.txt"
        mkdir -p                    "$build/dist/samples/"
        cp    samples/exifprint.cpp "$build/dist/samples/"

        # create the bundle
        pushd "$build" > /dev/null
            rm -rf   *.tar.gz
            tar czf "$b" dist/
            ls -alt
            mv   $b ..
        popd > /dev/null

        # clean userContent/build directories
        # daily > 50 days; weekly > 1 year;   monthly > 5 years
        if [ -e $daily ]; then find $daily -type f -ctime +50          -exec rm -rf {} \; ; fi
        if [ -e $weely ]; then find $weely -type f -ctime +365         -exec rm -rf {} \; ; fi
        if [ -e $monly ]; then find $monly -type f -ctime $((366 * 5)) -exec rm -rf {} \; ; fi

        # store the build
        cp $b $daily
        if [ "$dow" == "1" ]; then cp $b $weely; fi # Monday
        if [ "$dom" == "1" ]; then cp $b $monly; fi # First day of the month

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
