contrib/Qt/ReadMe.txt
---------------------

Exiv2 works well with Qt.

To build and run commandLineTool
--------------------------------

1) Windows Users should install MinGW/msys2 as documented in README.md

2) All users should build Exiv2 with C++11 support as documented in README.md

3) Generate Makefile
   Caution: You will have to modify commandLineTool.pro to fit your environment.
   $ cd <exiv2dir>
   $ cd contrib/Qt
   $ qmake commandLinePro.pro

4) Build commandLineTool.cpp
   $ make

5) Run commandLineTool.exe
   $ commandLineTool.exe

UNICODE_PATH on Windows
-----------------------

Qt users may prefer to build Exiv2 to support UNICODE_PATH.  The sample application samples/exifprint.cpp works with UNICODE_PATH.

Searching for more information about Qt, MinGW and UNICODE_PATH
---------------------------------------------------------------
These matters are occasionally discussed on the forum.  Please search to read discussions.

https://github.com/Exiv2/exiv2/issues/1101#issuecomment-623141576
http://dev.exiv2.org/boards/3/topics/2311?r=2312#message-2312
http://dev.exiv2.org/issues/1169
http://dev.exiv2.org/boards/3/topics/2705

Robin Mills
http://clanmills.com
2020-05-04
