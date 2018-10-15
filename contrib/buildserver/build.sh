#!/bin/bash

syntax() {
    echo "usage: $0 { --help | -? | -h | platform | option | switch }+ "
    echo "platform: all | cygwin | linux | macosx | mingw | msvc "
    echo "switch: --32 | --64 | --2015 | --2017  --publish | --verbose | --static | --status"
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
EOF
else
    ! ssh $1 ${command} <<EOF
PATH="/usr/local/bin/:/usr/bin:/mingw64/bin:$PATH"
cd ${cd}
mkdir -p buildserver
rm   -rf buildserver
git clone --branch $branch https://github.com/exiv2/exiv2 buildserver
mkdir -p buildserver/build
cd       buildserver/build
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
    cd=c:\\Users\\rmills\\gnu\\github\\exiv2\\
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
cd ${cd}buildserver\\build
ls -alt *.zip
EOF
else
    ! ssh $1 <<EOF
cd ${cd}
IF EXIST buildserver rmdir/s/q buildserver
git clone --branch ${branch} https://github.com/exiv2/exiv2 buildserver
mkdir    buildserver\build
cd       buildserver\build
conan install .. --profile ${profile} --build missing
cmake         .. -G ${generator} -DCMAKE_BUILD_TYPE=${config}  -DCMAKE_INSTALL_PREFIX=..\\dist\\${profile}
cmake --build .  --config ${config}   --target install
cmake --build .  --config ${config}   --target package
ls -alt *.zip
EOF
fi
}

publishBuild()
{
    date=$(date '+%Y-%m-%d+%H-%M-%S')
    builds="/mmHD/Users/Shared/Jenkins/Home/userContent/builds"
    input=something.tar.gz
    output="$daily/date-${date}-$input"
    scp     input rmills@rmillsmm:/mmHD/Users/Shared/Jenkins/Home/userContent/builds/$output
}

##
# assign defaults
msvc=0
cygwin=0
mingw=0
cygwin=0
macosx=0
linux=0
help=0
publish=0
verbose=0
b64=0
b32=0
static=0
edition=2017
branch=RC1
dryrun=0
server=rmillsmm
user=rmills
status=0
all=0

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
      msvc)      msvc=1        ;;
      --64)      b64=1         ;;
      --32)      b32=1         ;;
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
      *)         echo "invalid option: $arg" 1>&2; help=1; ;;
    esac
done

if [ $help == 1 ]; then
    syntax;
    exit 0;
fi

if [ "$all" == "1" ]; then
    cygwin=1; linux=1; macosx=1; mingw=1; msvc=1;
    if [ "$b64" == "0" -a "$b32" == "0" ]; then
          b64=1;b32=1
    fi
fi
if [ "$b64" == "0" -a "$b32" == "0" ]; then b64=1; fi

##
# begin builds
if [ $linux == 1 ]; then
    command=''
    cd=/home/rmills/gnu/github/exiv2/
    unixBuild ${user}@${server}-ubuntu Linux
fi

if [ $macosx == 1 ]; then
    cd=/Users/rmills/gnu/github/exiv2/
    command=''
    unixBuild ${user}@${server} MacOSX
fi

if [ $mingw == 1 ]; then
    if [ $b64 == 1 ]; then
       command='msys64'
       cd=/home/rmills/gnu/github/exiv2/
       unixBuild ${user}@${server}-w7 MinGW/32
    fi
    if [ $b32 == 1 ]; then
       command='msys32'
       cd=/home/rmills/gnu/github/exiv2/
       unixBuild ${user}@${server}-w7 MinGW/32
    fi
fi

if [ $cygwin == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='c:\\cygwin64\\bin\\bash.exe'
    unixBuild ${user}@${server}-w7 Cygwin
fi

if [ $msvc == 1 ]; then
    command=''
    msvcBuild ${user}@${server}-w7
fi

# That's all Folks
##

