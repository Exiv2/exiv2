#!/bin/bash

source $(find . -name buildserver.library 2>/dev/null)

echo -------------------------------
echo PLATFORM = $PLATFORM PWD = $PWD
if [ "$(osName)" == "mingw" ]; then
    echo "Windows directory (mingw) = " $(pwd -W)
fi
if [ "$(osName)" == "cygwin" ]; then
    echo "Windows directory (cygwin) = " $(cygpath -aw .)
fi
echo -------------------------------

##
#  figure out today's build
#  http://exiv2.dyndns.org:8080/userContent/builds/Daily
date=$(date '+%Y-%m-%d')
count=4
curl='/usr/local/bin/curl --silent --connect-timeout 30 --max-time 40'
while [ "$count" != "0" ]; do
  if  [ "$count" != "0" ]; then echo "*** count = $count ***" ; fi
  build=$($curl "$JENKINS/$DAILY/"             \
         |xmllint --html --format - 2>/dev/null | grep $PLATFORM  \
         |grep $date | grep -v -e view | cut -d'"' -f 2 | tail -1   )
  if [ "$build" != "" ]; then
    count=0;
  fi
  if [ "$count" != "0" ]; then
      count=$(expr $count - 1)
  fi
  if [ "$count" == "1" ]; then
     echo --------
     echo $curl "$JENKINS/$DAILY/" | xmllint --html --format
          $curl "$JENKINS/$DAILY/" | xmllint --html --format
     echo --------
  fi
done

echo date  =  $date
echo url   = "$JENKINS/$DAILY/"
echo build =  $build

if [ -z "$RECURSIVE" ]; then
    ##
    # collect build from server
    if [  -e /tmp/jenkins ]; then
      rm -rf /tmp/jenkins
    fi

    mkdir    /tmp/jenkins
    pushd    /tmp/jenkins 2>/dev/null
        echo $curl -O "$JENKINS/$DAILY/$build"
             $curl -O "$JENKINS/$DAILY/$build"
        ls -alt $build
        ##
        # expand the bundle
        if [ -e dist ]; then rm -rf dist ;fi
        tar xzf $build
        if [ ! -e dist ]; then echo '*** no dist directory ***' ; exit 0; fi
    popd 2>/dev/null
fi
if [ ! -e /tmp/jenkins/dist ]; then echo '*** $build has not been opened in /tmp/jenkins/dist ***' ; exit 0; fi

grep_args="-e libexiv2 -e ^date -e ^bits -e ^version -e ^time -e ^platform"
case $PLATFORM in
    macosx)
        # test the delivered exiv2
        cd /tmp/jenkins/dist
        export DYLD_LIBRARY_PATH="$PWD/$PLATFORM/lib:$DYLD_LIBRARY_PATH"
        $PLATFORM/bin/exiv2 -vV | grep $grep_args

        # compile, link and test the sample code
        echo ''
        g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o exifprint
        ls -alt exifprint
        echo ''

        ./exifprint --version     | grep $grep_args
    ;;

    linux)
        # test the delivered exiv2
        cd /tmp/jenkins/dist
        export LD_LIBRARY_PATH="$PWD/$PLATFORM/lib:$LD_LIBRARY_PATH"
        $PLATFORM/bin/exiv2 -vV | grep $grep_args

        # compile, link and test the sample code
        echo ''
        g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o exifprint
        ls -alt exifprint
        echo ''

        ./exifprint --version     | grep $grep_args
    ;;

    cygwin)
        # test the delivered exiv2
        cd /tmp/jenkins/dist
        PATH="$PWD/$PLATFORM/bin:$PATH"
        $PLATFORM/bin/exiv2 -vV | grep $grep_args

        # compile, link and test the sample code
        echo ''
        g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o exifprint
        ls  -alt exifprint.exe
        echo ''

        ./exifprint --version     | grep $grep_args
    ;;

    msvc)
        cd /tmp/jenkins/dist
        for vs in 2005 2008 2010 2012 2013 2015; do
          for arch in x64 Win32; do
            if [ -e "$PWD/$vs/$arch/dll/Release/bin/exiv2.exe" ] ; then (
              # test the delivered exiv2
              PATH="$PWD/$vs/$arch/dll/Release/bin:$PATH"
              exiv2 -vV | grep $grep_args

              # compile, link and test the sample code
              (
                export PATH="/cygdrive/c/Windows/System32:$PATH"
                echo ''
                a='32'
                if [ $arch == x64 ]; then a=64 ; fi
                echo cmd /c "vcvars.bat $vs $a && cd && cl /EHsc -I$vs\\$arch\\dll\Release\include /MD samples\exifprint.cpp /link $vs\\$arch\dll\\Release\lib\exiv2.lib"
                     cmd /c "vcvars.bat $vs $a && cd && cl /EHsc -I$vs\\$arch\\dll\Release\include /MD samples\exifprint.cpp /link $vs\\$arch\dll\\Release\lib\exiv2.lib"
                ls -alt exifprint.exe
                echo ''
              )
              ./exifprint.exe --version     | grep $grep_args
            ) fi
          done
        done
    ;;

    mingw)
        if [ ! -z "$RECURSIVE" ]; then
            # test the delivered exiv2
            CD=$PWD # build exifprint from the dist into the current directory
            cd /tmp/jenkins/dist
            PATH="$PWD/$PLATFORM/bin:$PATH"
            echo ''
            echo "ls -alt $PWD/$PLATFORM/bin/libexiv2-14.dll"
                  ls -alt $PWD/$PLATFORM/bin/libexiv2-14.dll
            echo ''
            echo "$PWD/$PLATFORM/bin/exiv2.exe -vV | grep $grep_args"
                  $PWD/$PLATFORM/bin/exiv2.exe -vV | grep $grep_args

            # compile, link and test the sample code
            echo ''
            echo g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o $CD/exifprint
                 g++ -I$PLATFORM/include -L$PLATFORM/lib samples/exifprint.cpp -lexiv2 -o $CD/exifprint
            echo "ls -alt $CD/exifprint.exe"
                  ls  -alt $CD/exifprint.exe
            echo ''
            echo "$CD/exifprint.exe --version     | grep $grep_args"
                  $CD/exifprint.exe --version     | grep $grep_args
        else
           # recursively invoke MinGW/bash with appropriate tool chain
           export RECURSIVE=1

           export TMP=/tmp
           export TEMP=$TMP
           if [ "$x64" == true ]; then
               export CFLAGS=-m64
               export CXXFLAGS=-m64
               export LDFLAGS=-m64
               /c/MinGW64/msys/1.0/bin/bash.exe -c "export PATH=/c/TDM-GCC-64/bin:/c/MinGW64/bin:/c/MinGW64/msys/1.0/bin:/c/MinGW64/msys/1.0/local/bin ; $0"
           fi
           if [ "$win32" == true ]; then
               export CFLAGS=-m32
               export CXXFLAGS=-m32
               export LDFLAGS=-m32
               /c/MinGW/msys/1.0/bin/bash.exe -c "export PATH=/c/Qt/Qt5.6.0/5.6/mingw49_32/bin:/c/Qt/Qt5.6.0/Tools/mingw492_32/bin:/c/MinGW/bin:/usr/bin:/usr/local/bin:/c/cygwin64/bin:/c/Users/rmills/com:. ; $0"
           fi
        fi
    ;;

    *) echo unknown platform $platform
    ;;
esac

# That's all Folks!
##
