#!/bin/bash

syntax() {
    echo "usage: $0 { --help | -? | -h | platform | option | switch }+ "
    echo "platform: all | cygwin | linux | macosx | mingw | mingw32 | msvc "
    echo "switch:   --2015 | --2017 | --publish | --verbose | --static | --status"
    echo "option: --branch x | --server x | --user x"
}

announce()
{
    if [ "$status" != "1" ]; then
        echo ++++++++++++++++++++++++++++++++
        echo $*
        echo ++++++++++++++++++++++++++++++++
    fi
}

bomb() {
    echo "*** $1 requires an argument ***" >&2
    exit 1
}


unixBuild()
{
    announce  $1 $2
if [ "$status" == "1" ]; then
    ! ssh $1 ${command} <<EOF
cd ${cd}/buildserver/build
ls -alt *.tar.gz | sed -E -e 's/\+ / /g' # remove extended attribute marker
# echo +++ scp *.tar.gz rmills@rmillsmm:/mmHD/Users/Shared/Jenkins/Home/userContent/builds/$tag
EOF
else
    ! ssh $1 ${command} <<EOF
PATH="/usr/local/bin/:/usr/bin:/mingw64/bin:$PATH"
cd ${cd}
mkdir -p buildserver
rm   -rf buildserver
git clone --branch $branch https://github.com/exiv2/exiv2 buildserver
cd       buildserver
git pull --rebase
mkdir -p build
cd       build
cmake .. -G "Unix Makefiles"
make
make tests
make package
ls -alt *.tar.gz | sed -E -e 's/\+ / /g'
EOF
fi
}

msvcBuild()
{
    cd=c:\\\\Users\\\\rmills\\\\gnu\\\\github\\\\exiv2\\\\
    config=Release
    profile=msvc2017Release64
    generator='"Visual Studio 15 2017 Win64"'
    if [ "$edition" == "2015" ]; then
        profile=msvc2015Release64
        generator='"Visual Studio 14 2015 Win64"'
    fi
    announce  $1 ${profile}
if [ "$status" == "1" ]; then
    ! ssh $1 msys64 <<EOF
cd ${cd}buildserver\\\\build
ls -alt *.zip
# echo scp exiv2-0.27.0.1-msvc.zip rmills@rmillsmm:/mmHD/Users/Shared/Jenkins/Home/userContent/builds/$tag-exiv2-0.27.0.1-msvc.zip
EOF
else
    ! ssh $1 <<EOF
cd ${cd}
IF EXIST buildserver rmdir/s/q buildserver
git clone --branch ${branch} https://github.com/exiv2/exiv2 buildserver
cd buildserver
git pull --rebase
mkdir    build
cd       build
conan install .. --profile ${profile} --build missing
cmake         .. -G ${generator} -DCMAKE_BUILD_TYPE=${config}  -DCMAKE_INSTALL_PREFIX=..\\dist\\${profile}
cmake --build .  --config ${config}   --target install
cmake --build .  --config ${config}   --target package
ls -alt *.zip
EOF
fi
}

##
# assign defaults
cygwin=0
linux=0
macosx=0
mingw=0
mingw32=0
msvc=0
help=0
publish=0
verbose=0
static=0
edition=2017
branch=RC1
dryrun=0
server=rmillsmm
user=rmills
status=0
all=0
date=$(date '+%Y-%m-%d+%H-%M-%S')
tag=${date}

if [ "$#" == "0" ]; then help=1; fi

##
# parse command line
while [ "$#" != "0" ]; do
    arg="$1"
    shift
    case "$arg" in
      -h|--help|-\?) help=1    ;;
      all)       all=1         ;;
      cygwin)    cygwin=1      ;;
      linux)     linux=1       ;;
      macosx)    macosx=1      ;;
      mingw)     mingw=1       ;;
      mingw32)   mingw32=1     ;;
      msvc)      msvc=1        ;;
      --verbose) verbose=1     ;;
      --dryrun)  dryrun=1      ;;
      --publish) publish=1     ;;
      --static)  static=1      ;;
      --2017)    edition=2017  ;;
      --2015)    edition=2015  ;;
      --status)  status=1      ;;
      --server)  if [ $# -gt 0 ]; then server=$1; shift; else bomb $arg ; fi ;;
      --branch)  if [ $# -gt 0 ]; then branch=$1; shift; else bomb $arg ; fi ;;
      --user)    if [ $# -gt 0 ]; then user=$1  ; shift; else bomb $arg ; fi ;;
      *)         echo "*** invalid option: $arg ***" 1>&2; help=1; ;;
    esac
done

if [ $help == 1 ]; then
    syntax;
    exit 0;
fi

if [ "$all" == "1" ]; then
    cygwin=1; linux=1; macosx=1; mingw=1; mingw32=1;msvc=1;
fi

##
# perform builds
if [ $cygwin == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='c:\\cygwin64\\bin\\bash.exe'
    unixBuild ${user}@${server}-w7 Cygwin
fi

if [ $linux == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command=''
    unixBuild ${user}@${server}-ubuntu Linux
fi

if [ $macosx == 1 ]; then
    cd=/Users/rmills/gnu/github/exiv2/
    command=''
    unixBuild ${user}@${server} MacOSX
fi

if [ $mingw == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='msys64'
    unixBuild ${user}@${server}-w7 MinGW/64
fi

if [ $mingw32 == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='msys32'
    unixBuild ${user}@${server}-w7 MinGW/32
fi

if [ $msvc == 1 ]; then
    command=''
    msvcBuild ${user}@${server}-w7
fi

# That's all Folks
##
