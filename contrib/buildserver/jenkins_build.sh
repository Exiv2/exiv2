#!/bin/bash

##
# jenkins_build.sh
#   called by jenkins to build/test exiv2
#   - jenkins sets environment variables
#   called from terminal
#   - script has build-in defaults for some environment variable
#
#  arguments:
#    status    : filter last build with grep
#
##
source $(find . -name buildserver.library)

base=$(basename $0)
if [ -z "$base" ]; then base=jenkins_build ; fi
tmp=/tmp/$base.tmp
start=$(date)
starts=$(date +%s)


##
# arg: status [grep-args]
# example: ./jenkins_build.sh status -e URL
if [ "$1" == "status" ]; then
    shift
    build=$(basename $PWD)
    declare -A expects=( [linux]=900 [macosx]=900 [cygwin]=1000 [mingw]=100 [msvc]=1200 )
    for b in linux macosx cygwin mingw msvc ; do
        echo $build/$b
        curl --silent $JENKINS/job/Exiv2-$build/label=$b/lastBuild/consoleText | tee $tmp |\
            grep -E -e SVN_[A-Z]+= -e JOB_NAME -e BUILD_ID -e Finished -e seconds $@ ;
        declare -i lines=$(wc -l $tmp | cut -d/ -f 1)
        declare -i expect=${expects[$b]}
        diff=$(( lines-expect>0?lines-expect:expect-lines ))
        msg=''
        warn=''
        if [ "$diff" -gt "200" ]; then warn="***" ; msg="TAKE CARE " ; fi
        echo "${warn}${msg}lines= ${lines} expect~${expect} diff=${diff}" "${msg}${warn}"
        echo ''
    done
    exit $result
fi

run_tests() {
    if [ "$result" == "0" ]; then
        if [ "$tests" == true ]; then
            make tests
        fi
    fi
}

##
# set up some defaults (used when running this script from the terminal)
echo "1 target = $target platform = $PLATFORM WORKSPACE = $WORKSPACE"
if [ $PLATFORM == "macosx" -a -z "$macosx"   ]; then export macosx=true ; export target=macosx    ; fi
if [ $PLATFORM == "linux"  -a -z "$linux"    ]; then export linux=true  ; export target=linux     ; fi
if [ -z "$cygwin"    -a ! -z "$CYGWIN"       ]; then export cygwin=$CYGWIN                        ; fi
if [ -z "$tests"                             ]; then export tests=true                            ; fi
if [ -z "$WORKSPACE"                         ]; then export WORKSPACE="$0/$PLATFORM"              ; fi

if [ -z "$target" ]; then export target=$(basename $(echo $WORKSPACE | sed -E -e 's#\\#/#g'))     ; fi
echo "2 target = $target platform = $PLATFORM WORKSPACE = $WORKSPACE"

export PATH=$PATH:/bin:/sbin:/usr/sbin:/usr/local/bin:/usr/bin:/usr/lib/pkgconfig:/opt/local/bin:$PWD/usr/bin:/opt/local/bin:/opt/local/sbin:/opt/pkgconfig:bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig

echo   ----------------------
export
echo   ----------------------

##
# cleanup from last time (if there was a last time)
if [ -e config/config.mk ]; then
    if [   -e bin ]; then
        rm -rf bin
    fi
    if [ ! -e bin ]; then
       mkdir bin
    fi
    make distclean
fi

##
# create ./configure
# we need to inspect configure to know if libssh and libcurl are options for this build
make config &>/dev/null

##
# decide what to do about curl and ssh
export withcurl='--without-curl'
export withssh='--without-ssh'
if [ "$curl" == "true" ]; then export withcurl=--with-curl ; fi
if [ "$ssh"  == "true" ]; then export withssh=--with-ssh   ; fi

##
# what kind of build is this?
build=NONE

if [ $PLATFORM == "linux"  -a "$target" == "linux"  -a "$linux"  == "true"  ]; then build=UNIX ; fi
if [ $PLATFORM == "macosx" -a "$target" == "macosx" -a "$macosx" == "true"  ]; then build=UNIX ; fi
if [ $PLATFORM == "cygwin" -a "$target" == "cygwin" -a "$cygwin" == "true"  ]; then build=CYGW ; fi
if [                          "$target" == "mingw"  -a "$mingw"  == "true"  ]; then build=MING ; fi
if [                          "$target" == "msvc"   -a "$msvc"   == "true"  ]; then build=MSVC ; fi
if [ $PLATFORM == "mingw"  -o "$target" == "mingw"                          ]; then build=MING ; fi

echo "3 target = $target platform = $PLATFORM build = $build"
echo ---- path and perl -----
(IFS=:;for i in $PATH; do echo $i ; done)
echo -- which perl = $(which perl) ---
echo -- which expr = $(which expr) ---
perl --version
echo ---- end of path and perl ----

case "$build" in

  UNIX)
        echo ./configure  --prefix=$PWD/usr  $withcurl $withssh
             ./configure "--prefix=$PWD/usr" $withcurl $withssh
        make -j "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs"
        make install
        make -j samples "CXXFLAGS=-I${PWD}/usr/include -I${PWD}/src" "LDFLAGS=-L${PWD}/usr/lib -L${PWD}/xmpsdk/src/.libs -lexiv2"
        result=$?
        run_tests
        "$PWD/usr/bin/exiv2" -v -V
  ;;

  CYGW)
        if [ "$x64" == true ]; then
            export CFLAGS=-m64
            export CXXFLAGS=-m64
            export LDFLAGS=-m64
        fi
        echo make config
             make config
        # I've given up trying to get Cygwin to install into a local directory
        echo ./configure ${withcurl} ${withssh}
             ./configure ${withcurl} ${withssh}
        make -j
        result=$?
        make install
        make -j samples
        run_tests
        /usr/local/bin/exiv2 -v -V
  ;;

  MING)
        if [ ! -z "$RECURSIVE" ]; then
            export  CC=$(which gcc)
            export  CXX=$(which g++)
            export "PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"
            echo --- recursive MinGW build ---

            ##########################
            #  To build curl/MinGW/64 (on 32bit MinGW):
            #  PKG_CONFIG_PATH="/usr/local/lib/pkgconfig/:/usr/lib/pkgconfig/"
            #  cd /home/rmills/gnu/curl/curl-7.39.0>
            #  ./configure --prefix=/usr/local --disable-static --enable-shared --host=i686-x64-mingw32
            #  I haven't succeeded in getting openssl to link with curl on MinGW
            #  --with-ssl=/usr/local
            ##########################
            if [ "$withcurl" == "--with-curl" ]; then
                withcurl="--with-curl=/usr/local"
            fi

            ##########################
            # I have be unable to build libssh on MinGW (neither 32 nor 64 bits)
            ##########################
            if [ "$withssh" == "--with-ssh" ]; then
                echo "*** unable to build --with-ssh on MING ***"
                withssh="--without-ssh"
            fi

            ./configure $withcurl $withssh
            make -j       # DO NOT USE -j4.  It seems to hang the build!
            make install
            make -j samples
            run_tests
            exiv2 -v -V
        else
            if [ -e config/config.mk ]; then make clean ; fi
            (
                export TMP=/tmp
                export TEMP=$TMP
                export RECURSIVE=1
                # recursively invoke MinGW/bash with appropriate tool chain
                if [ "$x64" == true ]; then
                    /cygdrive/c/MinGW64/msys/1.0/bin/bash.exe -c "export PATH=/c/TDM-GCC-64/bin:/c/MinGW64/bin:/c/MinGW64/msys/1.0/bin:/c/MinGW64/msys/1.0/local/bin; $0"
                    result=$?
                fi
                if [ "$Win32" == true ]; then
                    /cygdrive/c/MinGW/msys/1.0/bin/bash.exe   -c "export PATH=/c/MinGW/bin:/c/MinGW/msys/1.0/bin:/c/MinGW/msys/1.0/local/bin; $0"
                    result=$?
                fi
            )
        fi
  ;;

  MSVC)
        rm -rf $PWD/bin
        mkdir  $PWD/bin

        PATH=$PATH:/cygdrive/c/Windows/System32
        cmd.exe /c "cd $(cygpath -aw .) && call contrib\\buildserver\\jenkins_build.bat"
        result=$?
  ;;

  NONE)
        echo "**************************************"
        echo "*** no build requested for $target ***"
        if [ "$target" == "cygwin" ]; then echo try export CYGWIN=true and run $(basename $0) again ; fi
        echo "**************************************"
  ;;
esac

echo target "$target" start: "$start" finish: $(date) diff: $(( $(date +%s) - starts )) seconds
set -v
# That's all Folks!
##
exit $result
