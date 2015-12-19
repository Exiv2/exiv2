#!/bin/bash

source $(find . -name functions.so)

##
# figure out today's build
# http://exiv2.dyndns.org:8080/userContent/builds/Daily
date=$(date '+%Y-%m-%d')
build=$(/usr/local/bin/curl --silent $JENKINS/$DAILY/             \
       |xmllint --html --pretty 1 - 2>/dev/null | grep $PLATFORM  \
       |grep $date | grep -v -e view | cut -d'"' -f 2 | tail -1   )

echo date  = $date
echo url   = $JENKINS/$DAILY
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
        # test the delivered exiv2
        PATH="$PWD/2013/x64/dll/Release/bin:$PATH"
        exiv2 -vV | grep $grep_args

        # compile, link and test the sample code
        (
            export PATH="/cygdrive/c/Windows/System32:$PATH"
            echo ''
		    cmd /c 'vcvars.bat 2013 64 && cl /EHsc -I2013\x64\dll\Release\include /MD samples\exifprint.cpp /link 2013\x64\dll\Release\lib\exiv2.lib'
            ls -alt exifprint.exe
            echo ''
        )

        ./exifprint.exe --version     | grep $grep_args
    ;;

    *) echo unknown platform $platform
    ;;
esac

# That's all Folks!
##
