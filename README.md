| Travis        | AppVeyor      | GitLab| Codecov|
|:-------------:|:-------------:|:-----:|:------:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=master)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/master?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/master) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/master/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/master) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/master/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) |

<name id="TOC"></a>
### T A B L E &nbsp;&nbsp;&nbsp; O&nbsp;&nbsp;F &nbsp;&nbsp;&nbsp; C O N T E N T S

1. [Welcome to Exiv2](#1)
2. [Building and Installing Exiv2](#2)
    1. [Build/Install Exiv2 on a UNIX-like system](#2-1)
    2. [Build/Install Exiv2 with Visual Studio](#2-2)
    3. [Uninstall Exiv2 on a UNIX-like system](#2-3)
    4. [Build Options](#2-4)
    5. [Dependencies](#2-5)
    6. [Consuming Exiv2 with CMake](#2-6)
    7. [Building Exiv2 Documentation](#2-7)
    8. [Building Exiv2 Packages](#2-8)
3. [License and Support](#2)
    1. [License](#3-1)
    2. [Support](#3-2)
4. [Platform Notes](#4)
    1. [Linux](#4-1)
    2. [MacOS-X](#4-2)
    3. [MinGW/msys2](#4-3)
    4. [Cygwin](#4-3)
    5. [Microsoft Visual C++](#4-5)
5. [Test Suit](#5)
    1. [Running tests on a UNIX-like system](#5-1)
    2. [Running tests on Visual Studio builds](#5-2)

<name id="1"></a>
# Welcome to Exiv2

![Exiv2](http://www.exiv2.org/include/exiv2-logo-big.png)

Exiv2 is a C++ library and a command line utility to read and
write Exif, IPTC and XMP image metadata. The homepage of Exiv2 is: https://github.com/Exiv2/exiv2

Downloads: [[Source](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-Source.tar.gz)]
[[Linux](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-Linux.tar.gz)]
[[MacOSX](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-Darwin.tar.gz)]
[[Cygwin](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-CYGWIN.tar.gz)]
[[MinGW](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-MinGW.tar.gz)]
[[MSVC](http://exiv2.dyndns.org/0.27.0.1/bundles/exiv2-0.27.0.1-msvc.zip)]
[[Documentation](http://exiv2.dyndns.org/0.27.0.1/doc/index.html)]
[[CMake](https://cmake.org/download/)]

The file "ReadMe.txt" in a bundle describes how to install/link code with libraries for their respective platform.
The file [license.txt](license.txt) in a bundle describes is a copy of GPLv2 License

[TOC](#TOC)
<name id="2"></a>
## 2 Building and Installing

You need CMake to build Exiv2:  https://cmake.org/download/

<name id="2-1"></a>
### 2.1 Build/Install Exiv2 on a UNIX-like system:

    $ cd <exiv2dir> ; mkdir build ; cd build
    $ cmake .. -G "Unix Makefiles" ; cmake --build . ; make test
    $ sudo make install

### The default install locations are:

| Location       | Purpose           |
|:-------------  |:------------- |
| /usr/local/lib | for the library |
| /usr/local/bin | for the `exiv2` utility |
| /usr/local/include/exiv2 | for header files |

[TOC](#TOC)
<name id="2-2"></a>
### 2.2 Build/Install Exiv2 with Visual Studio

We recommend that you use cocan with CMake to build  Exiv2 with Visual Studio.
See [README-CONAN](README-CONAN.md) for more information

[TOC](#TOC)
<name id="2-3"></a>
### 2.3 Uninstall Exiv2 on a UNIX-like system:

    $ sudo make uninstall

[TOC](#TOC)
<name id="2-4"></a>
### 2.4 Build options

There are two groups of CMake options.  Options defined by CMake include:

| Options       | Purpose       |
|:------------- |:------------- |
| CMAKE\_INSTALL\_PREFIX | where to install on your computer _**(/usr/local)**_ |
| CMAKE\_BUILD\_TYPE     | type of build _**(Release)**__|
| BUILD\_SHARED\_LIBS    | build exiv2lib as shared or static _**(On)**_ |

Options defined by <exiv2>/CMakeLists.txt:

```
576 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $ grep ^option CMakeLists.txt
option( BUILD_SHARED_LIBS             "Build exiv2lib as a shared library"                    ON  )
option( EXIV2_ENABLE_XMP              "Build with XMP metadata support"                       ON  )
option( EXIV2_ENABLE_EXTERNAL_XMP     "Use external version of XMP"                           OFF )
option( EXIV2_ENABLE_PNG              "Build with png support (requires libz)"                ON  )
option( EXIV2_ENABLE_NLS              "Build native language support (requires gettext)"      ON  )
option( EXIV2_ENABLE_PRINTUCS2        "Build with Printucs2"                                  ON  )
option( EXIV2_ENABLE_LENSDATA         "Build including lens data"                             ON  )
option( EXIV2_ENABLE_VIDEO            "Build video support into library"                      OFF )
option( EXIV2_ENABLE_WEBREADY         "Build webready support into library"                   OFF )
option( EXIV2_ENABLE_DYNAMIC_RUNTIME  "Use dynamic runtime (used for static libs)"            OFF )
option( EXIV2_ENABLE_WIN_UNICODE      "Use Unicode paths (wstring) on Windows"                OFF )
option( EXIV2_ENABLE_CURL             "USE Libcurl for HttpIo"                                OFF )
option( EXIV2_ENABLE_SSH              "USE Libssh for SshIo"                                  OFF )
option( EXIV2_BUILD_SAMPLES           "Build sample applications"                             ON  )
option( EXIV2_BUILD_PO                "Build translations files"                              OFF )
option( EXIV2_BUILD_EXIV2_COMMAND     "Build exiv2 command-line executable"                   ON  )
option( EXIV2_BUILD_UNIT_TESTS        "Build unit tests"                                      OFF )
option( EXIV2_BUILD_DOC               "Add 'doc' target to generate documentation"            OFF )
option( EXIV2_TEAM_EXTRA_WARNINGS     "Add more sanity checks using compiler flags"           OFF )
option( EXIV2_TEAM_WARNINGS_AS_ERRORS "Treat warnings as errors"                              OFF )
option( EXIV2_TEAM_USE_SANITIZERS     "Enable ASAN and UBSAN when available"                  OFF )
option( BUILD_WITH_CCACHE             "Use ccache to speed up compilations"                   OFF )
option( BUILD_WITH_COVERAGE           "Add compiler flags to generate coverage stats"         OFF )
577 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $
```

Options are defined on the CMake command line:
```
$ cmake -DBUILD_SHARED_LIBS=On -DEXIV2_ENABLE_NLS=OFF
```

[TOC](#TOC)
<name id="2-5"></a>
### 2.5 Dependencies

You can choose to use dependent libraries using your platform's package installer.

We also support conan which is especially useful for users of Visual Studio.
See [README-CONAN](http://exiv2.dyndns.org/0.27.0.1/README-CONAN.md) for more information

The following libexiv2 features are enabled by default and may
require external libraries. You can disable the dependency with CMake options

| Feature                    | Package  |  cmake option                | Availability |
|:-------------------------- |:-------- |:---------------------------- |:----------- |
|PNG image support           | zlib      | `-DEXIV2_ENABLE_PNG=Off`     | http://www.gnu.org/software/gettext/ |
|Native language support     | gettext   | `-DEXIV2_ENABLE_NLS=Off`     | http://zlib.net/ |
|XMP support                 | expat     | `-DEXIV2_ENABLE_XMP=Off`     | http://expat.sourceforge.net/ |

On Linux, it is usually best to install the dependencies through the
package management system of the distribution together with the
corresponding development packages (for the header files and static
libraries).

[TOC](#TOC)
<name id="2-6"></a>
### 2.6 Consuming Exiv2 with CMake

When installing exiv2 by running the **install** target we get some files under the folder
`${CMAKE_INSTALL_PREFIX}/share/exiv2/cmake/`.

In the example project https://github.com/piponazo/exiv2Consumer you could see how to consume
exiv2 via CMake by using these files.

<name id="2-7"></a>
### 2.7 Building Exiv2 Documentation

```
$ cmake ..options.. -DEXIV2_BUILD_DOC=On
$ make doc
```

To build the documentation, you will need the following products:

| Product      | Availability |
|:------------ |:------------ |
| doxygen      |     http://www.doxygen.org/ |
| graphviz     |    http://www.graphviz.org/ |
| python       |   http://www.python.org/    |
| xsltproc     |  http://xmlsoft.org/XSLT/   |
| md5sum       | http://www.microbrew.org/tools/md5sha1sum/ |
| pkg-config   | http://pkg-config.freedesktop.org/wiki/ |

[TOC](#TOC)
<name id="2-8"></a>
### 2.8 Building Exiv2 Packages

There are two types of Exiv2 packages which are generated by cpack from the cmake command-line.

1) Binary Package (library, headers, documentation and sample applications)

Create and build exiv2 for your plantform.

```bash
$ git clone https://github.com/exiv2/exiv2
$ mkdir -p exiv2/build
$ cd       exiv2/build
$ cmake .. -G "Unix Makefiles"
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
CPack: - package: /media/linuxDev/programming/exiv2/build/exiv2-0.27.0.1-Source.tar.bz2 generated.
```

You may prefer to run `$ cmake --build . --config Release --target package_source`

[TOC](#TOC)
<name id="3"></a>
## 3 License and Support

All project resources are accessible from the project website.
    https://github.com/Exiv2/exiv2

<name id="3-1"></a>
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
<name id="3-2"></a>
### 3.2 Support
For new bug reports and feature requests, please open an issue in Github.

[TOC](#TOC)
<name id="4"></a>
## 4 Platform Notes

<name id="4-1"></a>
### 4.1 Linux

Update your system and install the build tools.

```
sudo apt --yes update
sudo apt install --yes build-essential git libxml2-utils cmake python3 libexpat1-dev
```

Get the code from Git and Build

```
$ mkdir -p ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

[TOC](#TOC)
<name id="4-2"></a>
### 4.2 MacOS-X

You will need to install Xcode and the Xcode command-line tools to build anything on the Mac.

You should build and install libexpat and zlib from source.

You should build and install CMake from source.

[TOC](#TOC)
<name id="4-3"></a>
### 4.3 MinGW/msys2

Support for MinGW/msys1.0 32 bit build was provided for Exiv2 v0.26.

#### MSYS 64 bit
https://www.msys2.org
Download:  http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20180531.exe

I use this batch file to start the MinGW bash shell from the Dos Command Prompt (cmd.exe)

```
$ cat msys64.bat
setlocal
set "PATH=c:\msys64\usr\bin;c:\msys64\usr\local\bin;c:\msys64\mingw64\bin;"
set "PS1=\! \u@\h-64:\w \$ "
bash.exe -norc
$
```

#### MSYS 32 bit
http://repo.msys2.org/distrib/i686/msys2-i686-20180531.exe

I use this batch file to start the MinGW bash shell from the Dos Command Prompt (cmd.exe)

```
$ cat msys32.bat
setlocal
set "PATH=c:\msys32\usr\bin;c:\msys32\usr\local\bin;c:\msys32\mingw64\bin;"
set "PS1=\! \u@\h-32:\w \$ "
bash.exe -norc
$
```

#### Install MinGW Dependencies

```
for i in base-devel git cmake coreutils python3 man gcc gdb make dos2unix diffutils zlib-devel libexpat-devel libiconv-devel; do (echo y|pacman -S $i); done
```

#### Download exiv2 from github and build

```
$ mkdir -p ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

#### MinGW and Regex

The exiv2 command line program provides a `--grep` option which filters output.  The implementation requires the header file `<regex.h>` and supporting library to be available during the build.  When not available, the option degenerates to a substring match.  Because there are multiple versions of `<regex.h>` available on the MinGW platform, detection of Regex is always disabled on this platform.

[TOC](#TOC)
<name id="4-4"></a>
### 4.4 Cygwin

Download: https://cygwin.com/install.html and run setup-x86_64.exe

You need:
make, cmake, gcc, pkg-config, dos2unix, zlib-devel, libexpat1-devel, git, python3-interpreter, libiconv, libxml2-utils, libncurses.

Download and build libiconv-1.15:
https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz

Download and build cmake from source because I can't get the cygwin installed cmake 3.6.2 to work
To build cmake from source (for 3.11+), you need libncurses.

[TOC](#TOC)
<name id="4-5"></a>
### 4.5 Microsoft Visual C++

We recommend that you use Conan to build Exiv2 using Microsoft Visual C++.  For v0.27, we support Visual Studio 2008, 2010, 2012, 2013, 2015 and 2017.

As well as Microsoft Visual Studio, you will need to install CMake, Python3, and Conan.
1) Binary installers for CMake on Windows are availably from kitware.org.
2) Binary installers for Python3 are available from python.org
3) Conan can be installed using python/pip.  Details in README-CONAN.md

[TOC](#TOC)
<name id="5"></a>
## 5 Running the test suite

The test suite is a mix of bash and python scripts.  The python scripts are new to v0.27 and the bash scripts are being replaced as time permits.

<name id="5-1"></a>
### 5.1 Running tests on a UNIX-like system

You can run the suite directly from the build:

```
$ make build
...
$ make tests
... lots of output ...
Summary report
```

You can run individual tests in the test directory using EXIV2\_BINDIR to specify the location of the build artifacts.  For Cygwin and MinGW/msys builds, set EXIV2_EXT=.exe

```
rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/build $ cd ../test
rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/test $ env EXIV2_BINDIR=${PWD}/../build/bin ./icc-test.sh
ICC jpg md5 webp md5 png md5 jpg md5
all testcases passed.

rmills@rmillsmbp-w7 ~/gnu/github/exiv2/exiv2/test $ env EXIV2_BINDIR=${PWD}/../build/bin make newtests
```

[TOC](#TOC)
<name id="5-2"></a>
### 5.2 Running tests on Visual Studio builds

Use the bash interpreter for MinGW/msys2 to run the test suite.  It's essential to have a DOS Python3 interpreter on your path.  The variables EXIV2\_BINDIR and EXIV2\_EXT enable the test suite to locate the MSVC build artifacts.

```
$ cd <exiv2dir>/build
$ cd ../test
$ PATH="/c/Python36:$PATH"
$ export EXIV2_EXT=.exe
$ export EXIV2_BINDIR=${PWD}/../build/bin
```

You can proceed to execute the components of the test suite as described for Unix type systems:

```
$ make test
$ make newtests
$ ./icc-test.sh
```

[TOC](#TOC)

