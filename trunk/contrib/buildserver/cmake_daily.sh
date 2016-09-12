#!/bin/bash

##
# cmake_daily.sh
##
source $(find . -name buildserver.library)

##
# determine location of the build and source directories
if [ "$PLATFORM" == "msvc" ]; then
    exiv2=$(cygpath -aw .)
    build=$(cygpath -aw ./build)
     dist=$(cygpath -au ./build/dist/)
     msvc=$(cygpath -au ./contrib/cmake/msvc)
else
    exiv2=$PWD
    build=$PWD/build
     dist=$PWD/build/dist/$PLATFORM

    if [ -e $exiv2/CMakeCache.txt ]; then rm -rf $exiv2/CMakeCache.txt ; fi
fi

##
# create a clean directory for an out-of-source build
rm    -rf  dist
mkdir -p  $dist
mkdir -p  $build/dist/logs

echo "---- dist = $dist ------"
echo "---- build = $build ------"

##
# test the build (don't test msvc because it was tested by cmakeBuild)
testBuild()
{
    if [ -e $dist/bin/exiv2 ]; then
        pushd  ../test > /dev/null
        # EXIV2_BINDIR is used by the test suite to locate executables
        export EXIV2_BINDIR=$dist/bin
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
    else
        echo ''
        echo '**** no build created ****'
        echo ''
        result=1
    fi
}

##
# perform the build
(
    case $PLATFORM in
        msvc)
            ##
            # get windows cmd.exe to perform the build
            # use a subshell to restore the path
            (
                PATH="$msvc:/cygdrive/c/Windows/System32:/cygdrive/c/Program Files/csvn/bin:/cygdrive/c/Program Files (x86)/WANdisco/Subversion/csvn/bin:/cygdrive/c/Program Files/7-zip:/cygdrive/c/Program Files (x86)/cmake/bin"
                # cmd.exe /c "cd $build && vcvars $vs $arch && cmakeBuild --rebuild --exiv2=$exiv2 $*"
                for ARCH in 64 32; do
                	for VS in 2005 2008 2010 2012 2013 2015; do
                        echo -=-=-=-=-=-=-=-=-=-=-=-
                        echo cmd.exe /c "cd $build && vcvars $VS $ARCH && cmakeBuild --rebuild --exiv2=$exiv2 $*"
                        echo -=-=-=-=-=-=-=-=-=-=-=-
                             cmd.exe /c "cd $build && vcvars $VS $ARCH && cmakeBuild --rebuild --exiv2=$exiv2 $*"
                    done
                done
                # cmd.exe /c "cd $build && cmakeBuildAll --rebuild --exiv2=$exiv2 $*"
                result=$?
            )
            cp     $msvc/vcvars.bat $build/dist # required by test_daily.sh
        ;;

        mingw)
            if [ ! -z "$RECURSIVE" ]; then
                # we are already in MinGW/bash, so build
                # cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=$dist -DEXIV2_ENABLE_NLS=OFF -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) $exiv2
                make distclean
                make config
                ./configure --prefix=/usr/local
                make
                # run exiv2 to check the build is sane
                bin/.libs/exiv2 --verbose --version

                # install and copy the build in the dist
                make   install
                # run exiv2 to check the build is sane
                for d in bin lib include; do
                	mkdir -p                               "$dist/$d"
                	cp    -R /usr/local/$d/*expat* /usr/local/$d/*exiv* /usr/local/$d/z* /usr/local/$d/libz* /usr/local/$d/libdl*  "$dist/$d"
                done

                # fix up minor stuff
                rm     -rf                                 "$dist/lib/libexiv2-13.dll"
                cp     bin/*.exe                           "$dist/bin/"
                mkdir -p                                   "$dist/lib/pkgconfig"
                cp    -R /usr/local/lib/pkgconfig/*        "$dist/lib/pkgconfig"
                mkdir -p                                   "$dist/share/man/man1/"
                cp    -R /usr/local/share/man/man1/*exiv2* "$dist/share/man/man1/"
                mkdir -p                                   "$dist/samples"
                export "PATH=$dist/bin/:$PATH"

                # run the test suite
                export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
                make samples
                make tests
            else
                # recursively invoke MinGW/bash with appropriate tool chain
                export RECURSIVE=1

                export TMP=/tmp
                export TEMP=$TMP
                if [ "$x64" == true ]; then
                    export CFLAGS=-m64
                    export CXXFLAGS=-m64
                    export LDFLAGS=-m64
                    /c/MinGW64/msys/1.0/bin/bash.exe -c "export PATH=/c/TDM-GCC-64/bin:/c/MinGW64/bin:/c/MinGW64/msys/1.0/bin:/c/MinGW64/msys/1.0/local/bin; $0"
                    result=$?
                    if [ "$result" == "0" ]; then
                        testBuild
                    fi
                fi
                if [ "$win32" == true ]; then
                    export CFLAGS=-m32
                    export CXXFLAGS=-m32
                    export LDFLAGS=-m32
                    /c/MinGW/msys/1.0/bin/bash.exe -c "export PATH=/c/Qt/Qt5.6.0/5.6/mingw49_32/bin:/c/Qt/Qt5.6.0/Tools/mingw492_32/bin:/c/MinGW/bin:/usr/bin:/usr/local/bin:/c/cygwin64/bin:/c/Users/rmills/com:.; $0"
                    result=$?
                    if [ "$result" == "0" ]; then
                        testBuild
                    fi
                fi
            fi
        ;;

        *)  pushd $build > /dev/null
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
                result=$?
                /usr/local/bin/cmake --build . --target install
                testBuild
            )
            popd > /dev/null
        ;;
    esac
) | tee "$build/dist/logs/build.log"

##
# store the build for users to collect
if [ "$result" == "0" ]; then
    case $PLATFORM in
        linux)              mmHD="/media/psf/Home/Jenkins" ;;
        msvc|cygwin|mingw)  mmHD="//psf/Home/Jenkins"      ;;
        *)                  mmHD="/Users/rmills/Jenkins"   ;;
    esac

    jpubl="$mmHD/builds"
    echo jpubl = $jpubl

    daily="$jpubl/Daily"
    weely="$jpubl/Weekly"
    monly="$jpubl/Monthly"
    if [ ! -e $jpubl ]; then mkdir -p $jpubl ; fi
    if [ ! -e $daily ]; then mkdir -p $daily ; fi
    if [ ! -e $weely ]; then mkdir -p $weely ; fi
    if [ ! -e $monly ]; then mkdir -p $monly ; fi

    if [ -e $jpubl ]; then
        svn=0
        /usr/local/bin/svn info . 2>/dev/null >/dev/null
        if [ "$?" == "0" ]; then
        	svn=$(/usr/local/bin/svn info . | grep Revision | cut -d' ' -f 2)
          # svn=$($EXIV2_BINDIR/exiv2$exe -vV | grep -e ^svn | cut -d= -f 2)
        fi
        dow=$(date  '+%w') # 0..6   day of the week
        dom=$(date  '+%d') # 1..31  day of the month
        mon=$(date  '+%m') # 1..12  month
        dat=$(date '+%Y-%m-%d+%H-%M-%S')
        zip="${PLATFORM}-svn-${svn}-date-${dat}.tar.gz"

        # add documentation and samples to dist
        cat contrib/buildserver/dailyReadMe.txt | sed -E -e "s/__BUILD__/$zip/"  > "$build/dist/ReadMe.txt"
        mkdir -p                    "$build/dist/samples/"
        cp    samples/exifprint.cpp "$build/dist/samples/"

        # create the bundle
        pushd "$build" > /dev/null
            rm -rf   *.tar.gz
            tar czf "$zip" dist/
            ls -alt
            mv   $zip ..
        popd > /dev/null

        # clean userContent/build directories
        # daily > 50 days; weekly > 1 year;   monthly > 5 years
        if [ -e $daily ]; then find $daily -type f -ctime +50          -exec rm -rf {} \; ; fi
        if [ -e $weely ]; then find $weely -type f -ctime +365         -exec rm -rf {} \; ; fi
        if [ -e $monly ]; then find $monly -type f -ctime $((366 * 5)) -exec rm -rf {} \; ; fi

        # store the build
        cp $zip $daily
        if [ "$dow" == "1" ]; then cp $zip $weely; fi # Monday
        if [ "$dom" == "1" ]; then cp $zip $monly; fi # First day of the month

        echo '***' build = $zip '***'
    else
        echo '***' jenkins builds directory does not exist ${jpubl} '***'
        result=2
    fi
fi

exit $result
# That's all Folks!
##
