ReadMe.txt
----------

Build: __BUILD__

platform = linux
----------------

Structure of the bundle:

dist/linux/bin/exiv2.exe                              exiv2 and sample applications
dist/linux/lib/libexiv2.so.14.0.0                     shared library
       .../lib/exiv2.lib                              libraries for exiv2 and expat and zlib)
          /include/exiv2/                             include files
          /share/                                     man pages
dist/samples/exifprint.cpp                            sample code
dist/logs/                                            log files

To run exiv2:
$ cd dist
$ export LD_LIBRARY_PATH="$PWD/linux/lib:$LD_LIBRARY_PATH"
$ linux/bin/exiv2

To compile and link your own code:
$ cd dist
$ sudo cp -R linux/* /usr/local/
$ g++ samples/exifprint.cpp -lexiv2 -o exifprint
$ ./exifprint --version
exiv2=0.25.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$

platform = macosx
-----------------

Structure of the bundle:

dist/macosx/bin/exiv2.exe                             exiv2 and sample applications
        .../lib/libexiv2.so.14.0.0.dylib              shared library
           /lib/exiv2.lib                             libraries for exiv2/expat/zlib
           /include/exiv2/                            include files
           /share/                                    man pages
dist/logs/                                            log files
dist/samples/exifprint.cpp                            sample code

To run exiv2:
$ cd dist
$ export DYLD_LIBRARY_PATH="$PWD/macosx/lib:$DYLD_LIBRARY_PATH"
$ macosx/bin/exiv2

To compile and link your own code:
$ cd dist
$ sudo cp -R macosx/* /usr/local
$ g++ samples/exifprint.cpp -lexiv2 -o exifprint
$ ./exifprint --version
exiv2=0.25.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$

platform = cygwin
-----------------

dist/cygwin/bin/exiv2.exe                             exiv2.exe and sample applications
        .../bin/cygexiv2-14.dll                       shared library
           /lib/exiv2.dll.a                           libraries for exiv2/expat/zlib
           /include/exiv2/                            include files
           /share/                                    man pages
dist/samples/exifprint.cpp                            sample code
dist/logs/                                            log files

To run exiv2.exe:
$ cd dist
$ PATH="$PWD/cygwin/bin:$PATH"
$ cygwin/bin/exiv2

To compile and link your own code:
  Caution: This is a 64 bit build
           An error such as "incorrect architecture" is a symptom of using cygwin32
$ cd dist
$ cp -R cygwin/* /usr/local
$ g++ -L/usr/local/lib -lexiv2 samples/exifprint.cpp -o exifprint
$ ./exifprint --version
exiv2=0.25.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$

platform = mingw
-----------------

This a 32bit build of Exiv2 aimed at Qt/MinGW users.

dist/mingw/bin/exiv2.exe                              exiv2.exe and sample applications
       .../bin/libexiv2-14.dll                        shared library
          /lib/exiv2.dll.a                            libraries for exiv2/expat/zlib
          /include/exiv2/                             include files
          /share/                                     man pages
dist/samples/                                         sample code
dist/logs/                                            log files
dist/contrib/Qt                                       Qt code and notes

To run exiv2.exe:
$ cd dist
$ PATH="$PWD/mingw/bin:$PATH"
$ mingw/bin/exiv2

To compile and link your own code:
  Caution: This is a 32 bit build intended for use with MinGW32/Qt 5.6
           An error such as "incorrect architecture" is a symptom of using mingw64
$ cd dist
$ cp -R mingw/*  /usr/local
$ g++ --std=c++98 -L/usr/local/lib -lexiv2.dll samples/exifprint.cpp -o exifprint
$ ./exifprint --version
exiv2=0.25.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$

platform = msvc
---------------

dist\2013\x64\dll\Release\bin\exiv2.exe               exiv2.exe and sample applications
                      ...\bin\exiv2.dll               dlls for exiv2/expat/zlib
                         \lib\exiv2.lib               libraries for exiv2/expat/zlib
                         \include\exiv2/              include files
                         \share\                      man pages
dist\samples\exifprint.cpp                            sample code
dist\logs\                                            log files
dist/contrib/Qt                                       Qt code and notes
dist\vcvars.bat                                       script used by test_daily.sh

To run exiv2.exe:
c:\temp> cd dist
c:\temp\dist> set "PATH=%CD%\2013\x64\dll\Release\bin;%PATH%"
c:\temp\dist> exiv2

To compile and link your own code:
  Caution: You must use the same version of Visual Studio as the build
           You will need to use the "Visual Studio Command Prompt"
           or initialise the DOS environment by calling vcvarsall.bat

c:\temp> cd dist
c:\temp\dist> cl /EHsc -I2013\x64\dll\Release\include /MD samples\exifprint.cpp /link 2013\x64\dll\Release\lib\exiv2.lib
c:\temp\dist> exifprint --version
exiv2=0.25.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
c:\temp\dist>

Robin Mills
robin@clanmills.com
2016-09-13
