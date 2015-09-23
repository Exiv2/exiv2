Exiv2 xcode/ReadMe.txt
--------------------------------------
See http://dev.exiv2.org/issues/1110

exiv2.xcodeproj is a "no thrills" wrapper for the autotools:

Target		Command
config		make config
configure   ./configure
exiv2       make CXXFLAGS=-ggdb
install     sudo make install
samples     make samples
tests		make tests
exiv2json   make exiv2json

About executables:

When you add an executable to the "run" command, use:

bin/.libs/exiv2		As this uses the build library (without having to run install)

For all sample applications, including exiv2json, you will have to run install when you make library changes.

Robin Mills
robin@clanmills.com
