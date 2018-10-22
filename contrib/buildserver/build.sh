#!/bin/bash

syntax() {
    echo "usage: $0 { --help | -? | -h | platform | option value | switch }+ "
    echo "platform:  all | cygwin | linux | macosx | mingw | mingw32 | msvc"
    echo "switch: --2015 | --2017 | --publish | --status | --clone | --test"
    echo "option: --branch x | --server x | --user x | --builds x"
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

# write tag into the build directory (in Unix format without \r)
writeTag()
{
    # $1 = server name (eg rmillsmm-w7)
    # $2 = command for ssh (eg msys64 or bash)
    # $3 = destination of tag
    echo "echo tag=$tag > $3" | ssh ${user}@$1 $2
}

# if we're asked to clone, we remove the old build directory
prepareToClone()
{
    # $1 = server name (eg rmillsmm-w7)
    # $2 = command to remove directory 'buildserver' ("rmdir/s/q ${cd}buildserver")
    if [ $clone == 1 ]; then
        echo "$2" | ssh ${user}@$1 2>/dev/null
    fi
}

# list the current build
reportStatus()
{
    # $1 = server (eg rmillsmm-w7)
    # $2 = program to run (eg msys64 or bash)
    # $3 = string to execute (eg cd .../buildserver/build ; ls -alt *.tar *.zip)
    echo "$3" | ssh ${user}@$1 $2
}

unixBuild()
{
    # $1 = server                   (eg rmillsmm-w7)
    # $2 = string for announcement  (eg 'MinGW 64' )
    announce  $1 $2
    if [ "$status" == "1" ]; then
        reportStatus $1 $command "cd ${cd}/buildserver/build; ls -alt *.tar.gz | sed -E -e 's/\+ / /g'"
    else
        # remove the buildserver directory if we are to clone
        prepareToClone $1 "rm -rf ${cd}/buildserver"
        ! ssh ${user}@$1 ${command} <<EOF
PATH="/usr/local/bin/:/usr/bin:/mingw64/bin:$PATH"
cd ${cd}
if [ ! -e buildserver ]; then
    git clone --branch $branch https://github.com/exiv2/exiv2 buildserver
fi
cd       buildserver
git pull --rebase
mkdir -p build
cd       build
cmake .. -G "Unix Makefiles"
make
if [ "$test" == "1" ]; then make tests ; fi
make  package
if [ $(uname) == 'Darwin' ]; then
  # make package_source
  source=$(ls -1 *.tar.gz|sed -E -e 's#Darwin#Source#g')
  git clone --branch $branch https://github.com/exiv2/exiv2 package
  cd package
  rm -rf .git test/data
  tar czf $source *
  cd ..
  mv package/$source .
  rm -rf package
fi
ls -alt *.tar.gz | sed -E -e 's/\+ / /g'
EOF
        writeTag $1 $command ${cd}buildserver/build/tag
    fi
}

msvcBuild()
{
    # $1 = server                   (eg rmillsmm-w7)
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
        reportStatus $1 msys64 "cd ${cd}\\\\buildserver\\\\build ; ls -alt *.zip | sed -E -e 's/\+ / /g'"
    else
        prepareToClone $1 "rmdir/s/q ${cd}buildserver"
        ! ssh ${user}@$1 <<EOF
cd ${cd}
IF NOT EXIST buildserver git clone --branch ${branch} https://github.com/exiv2/exiv2 buildserver
cd buildserver
git pull --rebase
if NOT EXIST build mkdir build
cd           build
conan install .. --profile ${profile} --build missing
cmake         .. -G ${generator} -DCMAKE_BUILD_TYPE=${config}  -DCMAKE_INSTALL_PREFIX=..\\dist\\${profile}
cmake --build .  --config ${config}   --target install
cmake --build .  --config ${config}   --target package
ls -alt *.zip
EOF
        writeTag $1 msys64 ${cd}buildserver\\\\build\\\\tag
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
test=0
edition=2017
branch=RC1
clone=0
server=rmillsmm
user=rmills
status=0
all=0
builds=/Users/rmills/Jenkins/builds

tag=$(date '+%Y:%m:%d_%H:%M:%S')
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
      --test)    test=1        ;;
      --publish) publish=1     ;;
      --clone)   clone=1       ;;
      --static)  static=1      ;;
      --2017)    edition=2017  ;;
      --2015)    edition=2015  ;;
      --status)  status=1      ;;
      --server)  if [ $# -gt 0 ]; then server="$1"  ; shift; else bomb $arg ; fi ;;
      --branch)  if [ $# -gt 0 ]; then branch="$1"  ; shift; else bomb $arg ; fi ;;
      --user)    if [ $# -gt 0 ]; then user="$1"    ; shift; else bomb $arg ; fi ;;
      --builds)  if [ $# -gt 0 ]; then builds="$1"  ; shift; else bomb $arg ; fi ;;
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

publishBundle()
{
    # $1 = server    (eg rmillsmm-w7)
    # $2 = path      (eg /c/msys32/home/rmills/gnu/github/exiv2/buildserver/build)
    # $3 = extension (eg tar.gz or zip)
    # find the build tag left during the build
    tag_saved=$tag
    if [ -e tag ]; then rm -rf tag ; fi
    scp -q "$user@$1:$2/tag" . 2>/dev/null          # silently collect build tag file
    if [ -e tag ]; then source tag; fi              # and read it!

    files=$(ssh $user@$1 "ls -1 $2/*$3" 2>/dev/null)    # find the names of the bundles
    for file in $files; do
        if [ ! -z $file ]; then                         # copy to builds/all and merge the tag into the filename
            scp -pq "$user@$1:$file" $builds/all/$(basename $file $3)-$tag$3
            echo $(basename $file $3)-$tag$3
        fi
    done
    tag=$tag_saved
}

if [ $publish == 1 ]; then
    publishBundle $server        /Users/$user/gnu/github/exiv2/buildserver/build           '.tar.gz'
    publishBundle $server-ubuntu /home/$user/gnu/github/exiv2/buildserver/build            '.tar.gz'
    publishBundle $server-w7     /c/msys32/home/$user/gnu/github/exiv2/buildserver/build   '.tar.gz'
    publishBundle $server-w7     /c/msys64/home/$user/gnu/github/exiv2/buildserver/build   '.tar.gz'
    publishBundle $server-w7     /c/cygwin64/home/$user/gnu/github/exiv2/buildserver/build '.tar.gz'
    publishBundle $server-w7     /c/users/$user/gnu/github/exiv2/buildserver/build         '.zip'
    cygwin=0; linux=0; macosx=0; mingw=0; mingw32=0;msvc=0; # don't build anything
    $(dirname $0)/categorize.py  $builds
fi

##
# perform builds
if [ $cygwin == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='c:\\cygwin64\\bin\\bash.exe'
    unixBuild ${server}-w7 Cygwin
fi

if [ $linux == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='bash'
    unixBuild ${server}-ubuntu Linux
fi

if [ $macosx == 1 ]; then
    cd=/Users/rmills/gnu/github/exiv2/
    command='bash'
    unixBuild ${server} MacOSX
fi

if [ $mingw == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='msys64'
    unixBuild ${server}-w7 MinGW/64
fi

if [ $mingw32 == 1 ]; then
    cd=/home/rmills/gnu/github/exiv2/
    command='msys32'
    unixBuild ${server}-w7 MinGW/32
fi

if [ $msvc == 1 ]; then
    command='bash'
    msvcBuild ${server}-w7
fi

# That's all Folks
##
