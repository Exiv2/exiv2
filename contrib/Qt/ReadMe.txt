contrib/Qt/ReadMe.txt
---------------------

Exiv2 works well with Qt.  To support Qt/Windows users, a daily build of Exiv2 for MinGW/32 is provided.

To build and run commandLineTool
--------------------------------

1) Either build Exiv2/MinGW/32 as documented:
   http://clanmills.com/exiv2/mingw.shtml
OR
   Download and install a build from the buildserver:
   http://exiv2.dyndns.org:8080/userContent/builds/Categorized/Platform/mingw/
   Instructions for installing the build are in the bundle ReadMe.txt

2) Generate Makefile
   /c/Users/rmills/gnu/exiv2/trunk/contrib/Qt > qmake commandLinePro.pro

3) Build commandLineTool.cpp
   /c/Users/rmills/gnu/exiv2/trunk/contrib/Qt > make -B LDFLAGS=-L/usr/local/lib

4) Run commandLineTool.exe
   /c/Users/rmills/gnu/exiv2/trunk/contrib/Qt > release/commandLineTool.exe

Gotchas (that I know about)

1  Set PKG_CONFIG_PATH correctly
   export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"

2  Set PATH to use the toolchain for Qt
   PATH="/c/Qt/Qt5.6.0/5.6/mingw49_32/bin:/c/Qt/Qt5.6.0/Tools/mingw492_32/bin:/c/MinGW/bin:/usr/bin:/usr/local/bin:/c/cygwin64/bin:."

3  You may need to copy libstdc++-6.dll to /usr/local/bin
   cp $(which libstdc++-6.dll) /usr/local/bin

4  You may have to edit commandLineTool.pro to match your environment

5  DO NOT USE Cygwin Builds

6  Do not give up.
   This stuff works.  You may have to work quite hard to get it working.
   Don't give up.     It will work.

7  If you give up with MinGW, use Visual Studio.

Running MinGW bash and setting Qt tools in the path
---------------------------------------------------

I have a batch file (mingw32.bat) to get me into MinGW.

C:\>type \Users\rmills\com\mingw32.bat
@echo off
rem ------------------
: mingw32.bat
: invoke MinGW bash
:
setlocal
set "PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"
set "INCLUDE_PATH=/usr/local/inc"
set "LIBRARY_PATH=/usr/local/lib"
set "BINARY_PATH=/usr/local/bin"
rem set "PATH=c:\MinGW\bin;c:\MinGW\msys\1.0\bin;c:\MinGW\msys\1.0\local\bin;c:\Users\rmills\com;."
set "PATH=c:\Qt\Qt5.6.0\5.6\mingw49_32\bin;c:\Qt\Qt5.6.0\Tools\mingw492_32\bin;c:\MinGW\bin;\usr\bin:\usr\local\bin;c:\cygwin64\bin;c:\Users\rmills\com;."
set "PS1=\! -32- ${PWD}> "
c:\MinGW\msys\1.0\bin\bash.exe %*%

: That's all Folks
rem ------------------

UNICODE_PATH
------------

Qt users may prefer to build Exiv2 to support UNICODE_PATH.  The sample application samples/exifprint.cpp works with UNICODE_PATH.

Searching for more information about Qt, MinGW and UNICODE_PATH
---------------------------------------------------------------
These matters are occasionally discussed on the forum.  Please search to read discussions.

You will probably find the following helpful:
http://dev.exiv2.org/boards/3/topics/2311?r=2312#message-2312
http://dev.exiv2.org/issues/1169
http://dev.exiv2.org/boards/3/topics/2705

However there have been others and there will be more in future.

Robin Mills
http://clanmills.com
2016-09-19
