Configuration and Build Strategy                      12-Jul-2004, ahu
--------------------------------

Building and Installing
-----------------------

Exiv2 now comes with a rudimentary configuration script. Run the 
following commands from the top directory (containing this file) to 
build the library and utility:

     $ ./configure

     Check the generated config.mk and config.h files, particularly if
     you want to build a shared library with a compiler other than g++. 
     See caveat, below.

     $ make

     and, if you wish,

     $ make install 

     To install the library and header files, use 

     $ make install-lib

     There are corresponding uninstall and uninstall-lib targets.

Caveat
------

The configuration script does not support checks to test how to build
a shared library. If you are not using g++ on a GNU/Linux system, you
will need to check the generated config.mk file and manually set the
correct compilation flags. The related variables in config.mk are
CXXFLAGS_SHARED, LDFLAGS_SHARED and SHAREDLIB_SUFFIX.

MinGW and Mac OS X builds

The current build environment does not support building Exiv2 as a DLL
on MinGW or shared library on Mac OS X. When building on one of these
platforms, you need to unset SHARED_LIBS and set STATIC_LIBS in
config.mk (after running ./configure):

  # Define which libraries (shared and/or static) to build
  SHARED_LIBS = 
  STATIC_LIBS = 1

You should *not* need to modify any Makefile directly.

If your compiler uses a repository for object files of templates, try
setting CXX_REPOSITORY.

Developing with Exiv2
---------------------

To compile a module that includes an Exiv2 header file from the
default location, pass -DHAVE_CONFIG_H and -I/usr/local/include/exiv2
to the compiler.  Linking with the library requires -lexiv2 and
-L/usr/local/lib, for example (for a statically linked app):

g++ -DHAVE_CONFIG_H -I/usr/local/include/exiv2 -c yourapp.cpp -o yourapp.o
g++ yourapp.o -lexiv2 -L/usr/local/lib -o yourapp
