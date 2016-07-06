#!/bin/bash

source $(find . -name buildserver.library 2>/dev/null)

echo -------------------------------
echo PLATFORM = $PLATFORM
echo -------------------------------
##
# figure out today's build
# http://exiv2.dyndns.org:8080/userContent/builds/Daily
date=$(date '+%Y-%m-%d')
build=$(/usr/local/bin/curl --silent $JENKINS/$DAILY/             \
       |xmllint --html --pretty 1 - 2>/dev/null | grep $PLATFORM  \
       |grep $date | grep -v -e view | cut -d'"' -f 2 | tail -1   )

echo date  = $date
echo url   = $JENKINS/$DAILY/
echo build = $build

##
# collect build from server
if [ -e /tmp/jenkins ]; then rm -rf /tmp/jenkins ; fi
mkdir /tmp/jenkins
cd /tmp/jenkins
/usr/local/bin/curl -O --silent $JENKINS/$DAILY/$build
ls -alt $build
if [ ! -e $build ]; then echo '*** $build has not been downloaded ***' ; exit 1; fi

##
# expand the bundle
if [ -e dist ]; then rm -rf dist ;fi
tar xzf $build
if [ ! -e dist ]; then echo '*** no dist directory ***' ; exit 1; fi

##
# enter the dist and test it
cd dist
grep_args="-e libexiv2 -e ^date -e ^bits -e ^version -e ^time"
case $PLATFORM in
    macosx)
        # test the delivered exiv2
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

    *) echo unknown platform $platform
    ;;
esac

# That's all Folks!
##
