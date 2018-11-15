| Travis        | AppVeyor      | GitLab| Codecov|
|:-------------:|:-------------:|:-----:|:------:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=master)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/master?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/master) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/master/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/master) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/master/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) |

<name id="TOC">
### T A B L E _ OF _ C O N T E N T S

1. [Welcome to Exiv2](#1)
2. [Building, Installing, Using and Uninstalling Exiv2](#2)
    1. [Build, Install, Use and Uninstall Exiv2 on a UNIX-like system](#2-1)
    2. [Build and Install Exiv2 with Visual Studio](#2-2)
    3. [Build Options](#2-3)
    4. [Dependencies](#2-4)
    5. [Building and linking your code with Exiv2](#2-5)
    6. [Consuming Exiv2 with CMake](#2-6)
    7. [Using pkg-config to compile and link your code with Exiv2](#2-7)
    8. [Localisation](#2-8)
    9. [Building Exiv2 Documentation](#2-9)
   10. [Building Exiv2 Packages](#2-10)
3. [License and Support](#3)
    1. [License](#3-1)
    2. [Support](#3-2)
4. [Test Suit](#4)
    1. [Running tests on a UNIX-like system](#4-1)
    2. [Running tests on Visual Studio builds](#4-2)
<name id="platforms">
5. [Platform Notes](#5)
    1. [Linux](#5-1)
    2. [MacOS-X](#5-2)
    3. [MinGW](#5-3)
    4. [Cygwin](#5-4)
    5. [Microsoft Visual C++](#5-5)

<name id="1">
# Welcome to Exiv2

![Exiv2](exiv2.png)

Exiv2 is a C++ library and a command line utility to read,
write, delete and modify Exif, IPTC, XMP and ICC image metadata.

| Exiv2 Resource | Location |
|:------         |:----     |
| Project Homepage            | [https://github.com/Exiv2/exiv2](https://github.com/Exiv2/exiv2) |
| Downloads and Documentation | [http://exiv2.dyndns.org](http://exiv2.dyndns.org:8080) |
| BuildServer:                | [http://exiv2.dyndns.org:8080](http://exiv2.dyndns.org:8080) |
| License (GPLv2)             | [license.txt](license.txt) |
| CMake Downloads             | [https://cmake.org/download/](https://cmake.org/download/) |

The file ReadMe.txt in a Build bundle describes how to install the library on the platform.  ReadMe.txt also documents how to compile and link code on the platform.

[TOC](#TOC)
<name id="2">
## 2 Building, Installing, Using and Uninstalling Exiv2

You need CMake to build Exiv2:  https://cmake.org/download/

<name id="2-1">
### 2.1 Build, Install, Use, Uninstall Exiv2 on a UNIX-like system

```
$ cd <exiv2dir> ; mkdir build ; cd build
$ cmake .. -G "Unix Makefiles" ; cmake --build . ; make test
$ sudo make install
```

This will install the library into the "standard locations".  The library will be installed in `/usr/local/lib`, executables (including the exiv2 command-line program) in `/usr/local/bin/` and header files in `/usr/local/include/exiv2`

#### Using the exiv2 command-line program

To execute the exiv2 command line program, you should update your path to search /usr/local/bin/

```
$ export PATH="/usr/local/bin:$PATH"
```

 you'll also need to locate libexiv2 at run time:

```
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"      # Linux, Cygwin, MinGW/msys2
$ export DYLD_LIBRARY_PATH="/usr/local/lib:$DYLD_LIBRARY_PATH"  # MacOS-X
```

#### Uninstall Exiv2 on a UNIX-like system

```
$ sudo make uninstall
```

[TOC](#TOC)
<name id="2-2">
### 2.2 Build and Install Exiv2 with Visual Studio

We recommend that you use conan with CMake to build Exiv2 with Visual Studio.
See [README-CONAN](README-CONAN.md) for more information

[TOC](#TOC)
<name id="2-3">
### 2.3 Build options

There are two groups of CMake options.  There are many options defined by CMake.  Here are some particularly useful options:

| Options       | Purpose (_default_)       |
|:------------- |:------------- |
| CMAKE\_INSTALL\_PREFIX<br/>CMAKE\_BUILD\_TYPE<br/>BUILD\_SHARED\_LIBS | Where to install on your computer _**(/usr/local)**_<br/>Type of build _**(Release)**_<br/>Build exiv2lib as shared or static _**(On)**_ |

Options defined by <exiv2>/CMakeLists.txt include:

```
576 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $ grep ^option CMakeLists.txt
option( BUILD_SHARED_LIBS             "Build exiv2lib as a shared library"                    ON  )
option( EXIV2_ENABLE_XMP              "Build with XMP metadata support"                       ON  )
option( EXIV2_ENABLE_EXTERNAL_XMP     "Use external version of XMP"                           OFF )
option( EXIV2_ENABLE_PNG              "Build with png support (requires libz)"                ON  )
option( EXIV2_ENABLE_NLS              "Build native language support (requires gettext)"      ON  )
...
577 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $
```

Options are defined on the CMake command line:
```
$ cmake -DBUILD_SHARED_LIBS=On -DEXIV2_ENABLE_NLS=OFF
```

[TOC](#TOC)
<name id="2-4">
### 2.4 Dependencies

The following Exiv2 features are enabled by default and require external libraries. You can disable the dependency with CMake options:

| Feature                     | Package   |  cmake option to disable     | Availability |
|:--------------------------  |:--------  |:---------------------------- |:----------- |
| PNG image support           | zlib      | -DEXIV2\_ENABLE\_PNG=Off     | [http://zlib.net/](http://zlib.net/) |
| Native language support     | gettext   | -DEXIV2\_ENABLE\_NLS=Off     | [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) |
| XMP support                 | expat     | -DEXIV2\_ENABLE\_XMP=Off     | [http://expat.sourceforge.net](http://expat.sourceforge.net)/<br/>Use _**Expat 2.2.6**_ and later |

On Linux, you may install the dependencies using the distribution's package management system.  Install the development package of a dependency to install the header files and static libraries required to build Exiv2.

Notes about different platforms are included in this document: [Platform Notes](#platforms)

You may choose to install dependences with conan.  This is supported on all platforms and is especially useful for users of Visual Studio.
See [README-CONAN](README-CONAN.md) for more information.

[TOC](#TOC)
<name id="2-5">
### 2.5 Building and linking your code with Exiv2

There are detailed platform notes about linking code in releasenotes\platform\ReadMe.txt

Platform: Linux | Darwin | MinGW | CYGWIN | MSVC

In general you need to do the following:

1) Application code should be written in C++ 98 and should include exiv2 headers:

```
#include <exiv2/exiv2.hpp>
```

2 You should compile your C++ code with the directive: `-I/usr/local/include`

3 You should link your code with libexiv2 using the linker options: `-lexiv2` and `-L/usr/local/lib`

The following is a typical command to build an link with libexiv2:

```
$ g++ -std=c++98 myprog.cpp -o myprog -I/usr/local/include -L/usr/local/lib -lexiv2
```

[TOC](#TOC)
<name id="2-6">
### 2.6 Consuming Exiv2 with CMake

When exiv2 is installed, the files required to consume Exiv2 are installed in `${CMAKE_INSTALL_PREFIX}/share/exiv2/cmake/`

A Project to demonstrate consuming Exiv2 via CMake using those files is available here:  [https://github.com/piponazo/exiv2Consumer](https://github.com/piponazo/exiv2Consumer)

[TOC](#TOC)
<name id="2-7">
### 2.7 Using pkg-config to compile and link your code with Exiv2

When exiv2 is installed, the file exiv2.pc used by pkg-config is installed in `${CMAKE_INSTALL_PREFIX}/lib/pkgconfig`  You will need to set the following in your environment:

```
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

To compile and link using exiv2.pc, you usually add the following to your Makefile.

```
PKGCONFIG=pkg-config
CPPFLAGS := `pkg-config exiv2 --cflags`
LDFLAGS := `pkg-config exiv2 --libs`
```

If you are not using make, you can use pkg-config as follows:

```
g++ -std=c++98 myprogram.cpp -o myprogram $(pkg-config exiv2 --libs --cflags)
```

[TOC](#TOC)
<name id="2-8">
### 2.8 Localisation

Localisation is supported on "*ix" platforms:  Linux, MacOS-X, Cygwin and MinGW/msys2.  Localisation is not supported for Visual Studio builds.

To build localisation support, use the CMake options `-DEXIV2_BUILD_PO=On` `-DEXIV2_ENABLE_NLS=On`.  There are no additional build steps as the normal build command will compile the library, samples and localisation support.

1) Running exiv2 in a foreign language

```
$ env LANG=fr_FR exiv2
exiv2: Une action doit être spécifié
exiv2: Au moins un fichier est nécessaire
Utilisation : exiv2  [ options ] [ action ] fichier ...

Manipulation des métadonnées EXIF issues des images.
$
```

2) Adding additional languages to exiv2

To support a new language which we'll designate 'xy' for this discussion:

2.1) Generate a po file from the po template:

```
$ cd <exiv2dir>
$ mkdir -p po/xy
$ msginit --input=po/exiv2.pot --locale=xy --output=po/xy.po
```
2.2) Edit/Translate the strings in po/xy.po

I edited the following:

```
#: src/exiv2.cpp:237
msgid "Manipulate the Exif metadata of images.\n"
msgstr ""
```
to:

```
#: src/exiv2.cpp:237
msgid "Manipulate the Exif metadata of images.\n"
msgstr "Manipulate image metadata.\n"
```

2.3) Generate the messages file:

```
$ mkdir -p             po/xy/LC_MESSAGES
$ msgfmt --output-file=po/xy/LC_MESSAGES/exiv2.mo po/xy.po
```

2.4) Install and test your messages:

You have to install your messages to test them.  It's not possible to test a message file from the build/bin directory.

```
$ sudo cp -R  po/xy /usr/local/share/locale/xy
$ env LANG=xy exiv2
exiv2: An action must be specified
exiv2: At least one file is required
Usage: exiv2 [ options ] [ action ] file ...

Manipulate image metadata.   <--------- Edited message!
$
```

2.5) Submitting your new language file for inclusion in future versions of Exiv2:

Open a new issue on https://github.com/exiv2/exiv2 and attach the file po/xy/exiv2.po


[TOC](#TOC)
<name id="2-9">
### 2.9 Building Exiv2 Documentation

Building documentation requires installing special tools.  You will probably prefer to
read the documentation on-line from the project website: http://exiv2.dyndns.org

Additionally, complete copies of the project website are archived on the buildserver
and can be downloaded for off-line use.  http://exiv2.dyndns.org:8080/userContent/builds/Website/

To build documentation, use the CMake option `-DEXIV2_BUILD_DOC=On`.
Additionally, you will require an additional build step to actually build the documentation.

```
$ cmake ..options.. -DEXIV2_BUILD_DOC=On
$ make doc
```

To build the documentation, you must install the following products:

| Product      | Availability |
|:------------ |:------------ |
| doxygen<br/>graphviz<br/>python<br/>xsltproc<br/>md5sum  | [http://www.doxygen.org/](http://www.doxygen.org/)<br/>[http://www.graphviz.org/](http://www.graphviz.org/)<br/>[http://www.python.org/](http://www.python.org/)<br/>[http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/)<br/>[http://www.microbrew.org/tools/md5sha1sum/](http://www.microbrew.org/tools/md5sha1sum/) |

[TOC](#TOC)
<name id="2-10">
### 2.10 Building Exiv2 Packages

To enable building of packages, use the CMake option `-DEXIV2_TEAM_PACKAGING=On`.

You should not build Exiv2 Packages.  This feature is intended for use by Team Exiv2 to create Platform and Source Packages on the buildserver.

There are two types of Exiv2 packages which are generated by cpack from the cmake command-line.

1) Platform Package (header files, binary library and samples.  Some documentation and release notes)

Create and build exiv2 for your platform.

```bash
$ git clone https://github.com/exiv2/exiv2
$ mkdir -p exiv2/build
$ cd       exiv2/build
$ cmake .. -G "Unix Makefiles" -DEXIV2_TEAM_PACKAGING=On
...
-- Build files have been written to: .../build
$ cmake --build . --config Release
...
[100%] Built target addmoddel
$ make package
...
CPack: - package: /path/to/exiv2/build/exiv2-0.27.0.1-Linux.tar.gz generated.
```

2) Source Package

```bash
$ make package_source
Run CPack packaging tool for source...
...
CPack: - package: /path/to/exiv2/build/exiv2-0.27.0.1-Source.tar.gz generated.
```

You may prefer to run `$ cmake --build . --config Release --target package_source`

[TOC](#TOC)
<name id="3">
## 3 License and Support

All project resources are accessible from the project website.
    https://github.com/Exiv2/exiv2

<name id="3-1">
### 3.1 License

Copyright (C) 2004-2018 Exiv2 authors.
You should have received a copy of the file [license.txt](license.txt) which details the GPLv2 license.

Exiv2 is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

Exiv2 is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, 5th Floor, Boston,
MA 02110-1301 USA.

[TOC](#TOC)
<name id="3-2">
### 3.2 Support
For new bug reports and feature requests, please open an issue in Github.

[TOC](#TOC)
<name id="4">
## 4 Running the test suite

The test suite is a mix of bash and python scripts.  The python scripts are new to v0.27 and the bash scripts are being replaced as time permits.

<name id="4-1">
### 4.1 Running tests on a UNIX-like system

You can run the suite directly from the build:

```
$ make build
...
$ make tests
... lots of output ...
Summary report
```

You can run individual tests in the test directory using the environment variable EXIV2\_BINDIR to specify the location of the build artifacts.  For Cygwin and MinGW/msys builds, also set EXIV2_EXT=.exe

```
rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/build $ cd ../test
rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/test $ env EXIV2_BINDIR=${PWD}/../build/bin ./icc-test.sh
ICC jpg md5 webp md5 png md5 jpg md5
all testcases passed.

rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/test $ env EXIV2_BINDIR=${PWD}/../build/bin make newtests
```

[TOC](#TOC)
<name id="4-2">
### 4.2 Running tests on Visual Studio builds

Use the bash interpreter for MinGW/msys2 to run the test suite.  It's essential to have a DOS Python3 interpreter on your path.  The variables EXIV2\_BINDIR and EXIV2\_EXT enable the test suite to locate the MSVC build artifacts.

```
$ cd <exiv2dir>/build
$ cd ../test
$ PATH="/c/Python36:$PATH"
$ export EXIV2_EXT=.exe
$ export EXIV2_BINDIR=${PWD}/../build/bin
```

Once you have modified the PATH and and exported EXIV2\_BINDIR and EXIV2\_EXT, you can execute the test suite as described for Unix type systems:

```
$ cd <exiv2dir>/test
$ make test
$ make newtests
$ ./icc-test.sh
```

[TOC](#TOC)
<name id="5">
## 5 Platform Notes

There are many ways to set up and configure your platform.  The following notes are provided as a guide.

<name id="5-1">
### 5.1 Linux

Update your system and install the build tools.

```
sudo apt --yes update
sudo apt install --yes build-essential git libxml2-utils cmake python3 libexpat1-dev libz-dev
```

Get the code from GitHub and build

```
$ mkdir -p ~/gnu/github/exiv2
$ cd ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

[TOC](#TOC)
<name id="5-2">
### 5.2 MacOS-X

You will need to install Xcode and the Xcode command-line tools to build anything on the Mac.

You should build and install libexpat and zlib.  You may use brew, macports, build from source, or use conan.

I recommend that you build and install CMake from source.

[TOC](#TOC)
<name id="5-3">
### 5.3 MinGW

We provide support for both 64bit and 32bit builds using MinGW/msys2. [https://www.msys2.org](https://www.msys2.org)

Support for MinGW/msys1.0 32 bit build was provided for Exiv2 v0.26.  MinGW/msys1.0 is not supported by Team Exiv2 for Exiv2 v0.27 and later.

#### MinGW/msys2 64 bit
Install: [http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe](http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe)

I use the following batch file to start the MinGW/msys2 64 bit bash shell from the Dos Command Prompt (cmd.exe)

```
@echo off
setlocal
set "PATH=c:\msys64\usr\bin;c:\msys64\usr\local\bin;"
set "HOME=c:\msys64\home\rmills"
cd  %HOME%
set "PS1=\! \u@\h-64:\w \$ "
bash.exe -norc
```

#### MinGW/msys2 32 bit
Install: [http://repo.msys2.org/distrib/i686/msys2-i686-20180531.exe](http://repo.msys2.org/distrib/i686/msys2-i686-20180531.exe)

I use the following batch file to start the MinGW/msys2 32 bit bash shell from the Dos Command Prompt (cmd.exe)

```
@echo off
setlocal
set "PATH=c:\msys32\usr\bin;c:\msys32\usr\local\bin;"
set "HOME=c:\msys32\home\rmills"
cd  %HOME%
set "PS1=\! \u@\h-32:\w \$ "
bash.exe -norc
```

#### Install MinGW Dependencies

Install tools and dependencies:

```
$ for i in base-devel git cmake coreutils python3 man gcc gdb make dos2unix diffutils zlib-devel libexpat-devel libiconv-devel gettext-devel; do (echo y|pacman -S $i); done
```

You can upgrade all installed packages on your system with the following command.  For me, this broke msys32 and I had to reinstall msys32 and all the dependencies.  Your experience may be different.

```
$ pacman -Syu
```

#### Download exiv2 from github and build

```
$ mkdir -p ~/gnu/github/exiv2
$ cd       ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

#### MinGW and Regex

The exiv2 command line program provides a `--grep` option which filters output.  The implementation requires the header file `<regex.h>` and supporting library to be available during the build.  When not available, the option degrades to a substring match.  Because there are several versions of `<regex.h>` available on the MinGW platform, detection of Regex is always disabled on this platform and uses substring match.

[TOC](#TOC)
<name id="5-4">
### 5.4 Cygwin

Download: [https://cygwin.com/install.html](https://cygwin.com/install.html) and run setup-x86_64.exe

You need:
make, cmake, gcc, pkg-config, dos2unix, zlib-devel, libexpat1-devel, git, python3-interpreter, libiconv, libxml2-utils, libncurses.

Download and build libiconv-1.15: [https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz](https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz)

Download and build cmake from source because I can't get the cygwin installed cmake 3.6.2 to work.
To build cmake from source, you need libncurses. [https://cmake.org/download/](https://cmake.org/download/)

[TOC](#TOC)
<name id="5-5">
### 5.5 Microsoft Visual C++

We recommend that you use Conan to build Exiv2 using Microsoft Visual C++.  For v0.27, we support Visual Studio 2008, 2010, 2012, 2013, 2015 and 2017.

As well as Microsoft Visual Studio, you will need to install CMake, Python3, and Conan.

1) Binary installers for CMake on Windows are availably from [https://cmake.org/download/](https://cmake.org/download/).<br/>
2) Binary installers for Python3 are available from [python.org](https://python.org)<br/>
3) Conan can be installed using python/pip.  Details in [README-CONAN.md](README-CONAN.md)

[TOC](#TOC)

