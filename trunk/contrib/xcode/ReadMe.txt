Exiv2 xcode/ReadMe.txt
--------------------------------------
See http://dev.exiv2.org/issues/1110

exiv2.xcodeproj is a "no thrills" wrapper for the autotools:

Target		Command
config		make config
configure   ./configure -prefix=/usr/local CXXFLAGS=-ggdb 
exiv2       make
install     sudo make install
samples     make samples
tests		make tests
exiv2json   make exiv2json

About executables:

When you add an executable to the "run" command, use:
bin/.libs/exiv2		This is the exiv2(.exe) you have built, however it uses the shared library

For all sample applications you will have to run install when you make library changes.

Robin Mills
robin@clanmills.com
