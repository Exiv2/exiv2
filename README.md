| Travis        | AppVeyor      | GitLab| Codecov| Repology| Chat |
|:-------------:|:-------------:|:-----:|:------:|:-------:|:----:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=0.27-maintenance)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/0.27-maintenance?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/0.27-maintenance) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/0.27-maintenance/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/0.27-maintenance) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/0.27-maintenance/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) | [![Packaging status](https://repology.org/badge/tiny-repos/exiv2.svg)](https://repology.org/metapackage/exiv2/versions) | [![#exiv2-chat on matrix.org](matrix-standard-vector-logo-xs.png)](https://matrix.to/#/#exiv2-chat:matrix.org) |

<div id="1">
# Welcome to Exiv2

Exiv2 is a C++ library and a command-line utility to read,
write, delete and modify Exif, IPTC, XMP and ICC image metadata.

| Exiv2 Resource | Location |
|:----------     |:------    |
| Releases and Documentation<br>Prereleases:<br>Project Resources<br>License (GPLv2)<br>CMake Downloads  | [https://exiv2.org](https://exiv2.org)<br>[https://pre-release.exiv2.org](https://pre-release.exiv2.org)<br>[https://github.com/Exiv2/exiv2](https://github.com/Exiv2/exiv2)<br>[COPYING](COPYING)<br>[https://cmake.org/download/](https://cmake.org/download/) |
| README.md<br>README-CONAN.md<br>README-SAMPLES.md | User Manual. _This document_<br>Conan User Manual _[click here](README-CONAN.md)_<br>Sample Code Manual. _[click here](README-SAMPLES.md)_ |

The file ReadMe.txt in a build bundle describes how to install the library on the platform.  ReadMe.txt also documents how to compile and link code on the platform.

<div id="TOC">
### TABLE  OF  CONTENTS
![Exiv2](exiv2.png)

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
   11. [Debugging Exiv2](#2-11)
   12. [Building  Exiv2 with Clang and other build chains](#2-12)
   13. [Building  Exiv2 with ccache](#2-13)
   14. [Thread Safety](#2-14)
   15. [Library Initialisation and Cleanup](#2-15)
   16. [Cross Platform Build and Test on Linux for MinGW](#2-16)
   17. [Building with C++11 and other compilers](#2-17)
   18. [Static and Shared Libraries](#2-18)
   19. [Support for bmff files (CR3, HEIF, HEIC, and AVIF)](#2-19)
3. [License and Support](#3)
    1. [License](#3-1)
    2. [Support](#3-2)
4. [Test Suit](#4)
    1. [Running tests on a UNIX-like system](#4-1)
    2. [Running tests on Visual Studio builds](#4-2)
    3. [Unit tests](#4-3)
    4. [Python tests](#4-4)
    5. [Test Summary](#4-5)
5. [Platform Notes](#5)
    1. [Linux](#5-1)
    2. [macOS](#5-2)
    3. [MinGW/msys2](#5-3)
    4. [Cygwin](#5-4)
    5. [Visual Studio](#5-5)
    6. [Unix](#5-6)

[TOC](#TOC)
<div id="2">
## 2 Building, Installing, Using and Uninstalling Exiv2

You need [CMake](https://cmake.org/download/) to configure the Exiv2 project and the GCC or Clang compiler and associated tool chain.

<div id="2-1">
### 2.1 Build, Install, Use Exiv2 on a UNIX-like system

```bash
$ cd ~/gnu/github/exiv2  # location of the project code
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
$ make tests
$ sudo make install
```

This will install the library into the "standard locations".  The library will be installed in `/usr/local/lib`, executables (including the exiv2 command-line program) in `/usr/local/bin/` and header files in `/usr/local/include/exiv2`

cmake generates files in the build directory.  cmake generates the project/solution/makefiles required to build the exiv2 library and sample applications.  cmake also creates the files exv\_conf.h and exiv2lib\_export which contain compiler directives about the build options you have chosen and the availability of libraries on your machine.

#### Using the exiv2 command-line program

To execute the exiv2 command line program, you should update your path to search /usr/local/bin/

```bash
$ export PATH="/usr/local/bin:$PATH"
```

You will also need to locate libexiv2 at run time:

```bash
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"      # Linux, Cygwin, MinGW/msys2
$ export DYLD_LIBRARY_PATH="/usr/local/lib:$DYLD_LIBRARY_PATH"  # macOS
```

#### Uninstall

I don't know why anybody would uninstall Exiv2.

```bash
$ cd ~/gnu/github/exiv2  # location of the project code
$ cd build
$ sudo make uninstall
```

These commands will remove the exiv2 executables, library, header files and man page from the standard locations.

[TOC](#TOC)
<div id="2-2">
### 2.2 Build and Install Exiv2 with Visual Studio

We recommend that you use conan to download the Exiv2 external dependencies on Windows.  On other platforms (maxOS, Ubuntu and others), you should use the platform package manger.  These are discussed: [Platform Notes](#5) The options to configure and compile the project using Visual Studio are similar to UNIX like systems.
See [README-CONAN](README-CONAN.md) for more information about Conan.

When you build, you may install with the following command.

```cmd
> cmake --build . --target install
```
This will create and copy the exiv2 build artefacts to C:\Program Files (x86)\exiv2\.  You should modify your path to include C:\Program Files (x86)\exiv2\bin.

[TOC](#TOC)
<div id="2-3">
### 2.3 Build options

There are two groups of CMake options.  There are many options defined by CMake.  Here are some particularly useful options:

| Options       | Purpose (_default_)       |
|:------------- |:------------- |
| CMAKE\_INSTALL\_PREFIX<br/>CMAKE\_BUILD\_TYPE<br/>BUILD\_SHARED\_LIBS | Where to install on your computer _**(/usr/local)**_<br/>Type of build _**(Release)**_ See: [Debugging Exiv2](#2-11) <br/>Build exiv2lib as shared or static _**(On)**_ |

Options defined by <exiv2>/CMakeLists.txt include:

```bash
576 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $ grep ^option CMakeLists.txt
option( BUILD_SHARED_LIBS          "Build exiv2lib as a shared library"      ON )
option( EXIV2_ENABLE_XMP           "Build with XMP metadata support"         ON )
option( EXIV2_ENABLE_EXTERNAL_XMP  "Use external version of XMP"            OFF )
option( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"  ON )
...
option( EXIV2_ENABLE_BMFF          "Build with BMFF support"                 OFF)
577 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $
```

Options are defined on the CMake command-line:

```bash
$ cmake -DBUILD_SHARED_LIBS=On -DEXIV2_ENABLE_NLS=Off
```

It is planned to set the default -DEXIV2\_ENABLE\_BMFF=On for Exiv2 v1.00.  BMFF support is disabled by default in v0.27.4.


[TOC](#TOC)
<div id="2-4">
### 2.4 Dependencies

The following Exiv2 features require external libraries:

| Feature                     | Package   |  Default | To change default             | Availability |
|:--------------------------  |:--------  |:--------:| :---------------------------- |:-----------  |
| PNG image support           | zlib      | ON       | -DEXIV2\_ENABLE\_PNG=Off      | [http://zlib.net/](http://zlib.net/) |
| XMP support                 | expat     | ON       | -DEXIV2\_ENABLE\_XMP=Off      | [http://expat.sourceforge.net](http://expat.sourceforge.net)/<br/>Use _**Expat 2.2.6**_ and later |
| Natural language system     | gettext   | OFF      | -DEXIV2\_ENABLE\_NLS=On       | [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) |
| Character set conversion    | libiconv  |          | Disabled for Visual Studio.<br>Linked when installed on UNIX like platforms.                              | [https://www.gnu.org/software/libiconv/](https://www.gnu.org/software/libiconv/) |

On UNIX systems, you may install the dependencies using the distribution's package management system. Install the
development package of a dependency to install the header files and libraries required to build Exiv2. The script
`ci/install_dependencies.sh` is used to setup CI images on which we build and test Exiv2 on many platforms when we modify code.  You may find that helpful in setting up your platform dependencies.

Natural language system is discussed in more detail here: [Localisation](#2-8)

Notes about different platforms are included here: [Platform Notes](#5)

You may choose to install dependences with conan.  This is supported on all platforms and is especially useful for users of Visual Studio.
See [README-CONAN](README-CONAN.md) for more information.

### Libiconv

The library libiconv is used to perform character set encoding in the tags Exif.Photo.UserComment, Exif.GPSInfo.GPSProcessingMethod and Exif.GPSInfo.GPSAreaInformation.  This is documented in the exiv2 man page.

CMake will detect libiconv of all UNIX like systems including Linux, macOS, UNIX, Cygwin64 and MinGW/msys2.  If you have installed libiconv on your machine, Exiv2 will link and use it.

The library libiconv is a GNU library and we do not recommend using libiconv with Exiv2 when building with Visual Studio.

Exiv2 includes the file cmake/FindIconv.cmake which contains a guard to prevent CMake from finding libiconv when you build with Visual Studio.  This was added because of issues reported when Visual Studio attempted to link libconv libraries installed by Cygwin, or MinGW or gnuwin32. [https://github.com/Exiv2/exiv2/issues/1250](https://github.com/Exiv2/exiv2/issues/1250)

There are build instructions about Visual Studio in libiconv-1.16/INSTALL.window require you to install Cygwin.  There is an article here about building libiconv with Visual Studio. [https://www.codeproject.com/Articles/302012/How-to-Build-libiconv-with-Microsoft-Visual-Studio](https://www.codeproject.com/Articles/302012/How-to-Build-libiconv-with-Microsoft-Visual-Studio).  

If you wish to use libiconv with Visual Studio you will have to build libiconv and remove the "guard" in cmake/FindIconv.cmake.  Team Exiv2 will not provide support concerning libiconv and Visual Studio.

[TOC](#TOC)
<div id="2-5">
### 2.5 Building and linking your code with Exiv2

There are detailed platform notes about compiling and linking in `releasenotes/{platform}/ReadMe.txt`

where `platform: { CYGWIN | Darwin | Linux | MinGW | msvc | Unix }`

In general you need to do the following:

1) Application code should be written in C++98 and include exiv2 headers:

```cpp
#include <exiv2/exiv2.hpp>
```

2) Compile your C++ code with the directive: **`-I/usr/local/include`**

3) Link your code with libexiv2 using the linker options: **`-lexiv2`** and **`-L/usr/local/lib`**

The following is a typical command to build and link with libexiv2:

```bash
$ g++ -std=c++98 myprog.cpp -o myprog -I/usr/local/include -L/usr/local/lib -lexiv2
```

[TOC](#TOC)
<div id="2-6">
### 2.6 Consuming Exiv2 with CMake

When exiv2 is installed, the files required to consume Exiv2 are installed in `${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2`

You can build samples/exifprint.cpp as follows:

```bash
$ cd <exiv2dir>
$ mkdir exifprint
$ cd    exifprint
$ cat - > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.8)
project(exifprint VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(exiv2 REQUIRED CONFIG NAMES exiv2)    # search ${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2/
add_executable(exifprint ../samples/exifprint.cpp) # compile this
target_link_libraries(exifprint exiv2lib)          # link exiv2lib
EOF
$ cmake .                                          # generate the makefile
$ make                                             # build the code
$ ./exifprint                                      # test your executable
Usage: bin/exifprint [ path | --version | --version-test ]
$
```

[TOC](#TOC)
<div id="2-7">
### 2.7 Using pkg-config to compile and link your code with Exiv2

When exiv2 is installed, the file exiv2.pc used by pkg-config is installed in `${CMAKE_INSTALL_PREFIX}/lib/pkgconfig`  You will need to set the following in your environment:

```bash
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
```

To compile and link using exiv2.pc, you usually add the following to your Makefile.

```bash
PKGCONFIG=pkg-config
CPPFLAGS := `pkg-config exiv2 --cflags`
LDFLAGS := `pkg-config exiv2 --libs`
```

If you are not using make, you can use pkg-config as follows:

```bash
g++ -std=c++98 myprogram.cpp -o myprogram $(pkg-config exiv2 --libs --cflags)
```

[TOC](#TOC)
<div id="2-8">
### 2.8 Localisation

Localisation is supported on a UNIX-like platform:  Linux, macOS, Cygwin and MinGW/msys2.  Localisation is not supported for Visual Studio builds.

Crowdin have provided Exiv2 with a free open-source license to use their services.  The Exiv2 localisation project is located at [https://crowdin.com/project/exiv2](https://crowdin.com/project/exiv2).  You will also need to register to have a free user account on Crowdin.  The Crowdin setup is discussed here: [https://github.com/Exiv2/exiv2/issues/1510](https://github.com/Exiv2/exiv2/issues/1510).  It is recommended that you coordinate with Leonardo before contributing localisation changes on Crowdin.  You can contact Leonardo by via GitHub.

To build localisation support, use the CMake option `-DEXIV2_ENABLE_NLS=On`.  You must install the `gettext` package with your package manager or from source.  The `gettext` package is available from [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) and includes the library `libintl` and utilities to build localisation files.  If CMake produces error messages which mention libintl or gettext, you should verify that the package `gettext` has been correctly built and installed.

You must install the build to test localisation.  This ensures that the localisation message files can be found at run-time.  You cannot test localisation in the directory `build\bin`.

1) Running exiv2 in another language

```bash
$ env LANG=fr_FR exiv2    # env LANGUAGE=fr_FR exiv2 on Linux!
exiv2: Une action doit être spécifié
exiv2: Au moins un fichier est nécessaire
Utilisation : exiv2  [ options ] [ action ] fichier ...

Manipulation des métadonnées EXIF issues des images.
$
```

2) Adding additional languages to exiv2

To support a new language which we'll designate 'xy' for this discussion:

2.1) Generate a po file from the po template:

```bash
$ cd <exiv2dir>
$ mkdir -p po/xy
$ msginit --input=po/exiv2.pot --locale=xy --output=po/xy.po
```
2.2) Edit/Translate the strings in po/xy.po

I edited the following:

```bash
#: src/exiv2.cpp:237
msgid "Manipulate the Exif metadata of images.\n"
msgstr ""
```
to:

```bash
#: src/exiv2.cpp:237
msgid "Manipulate the Exif metadata of images.\n"
msgstr "Manipulate image metadata.\n"
```

2.3) Generate the messages file:

```bash
$ mkdir -p             po/xy/LC_MESSAGES
$ msgfmt --output-file=po/xy/LC_MESSAGES/exiv2.mo po/xy.po
```

2.4) Install and test your messages:

You have to install your messages to test them.  It's not possible to test a messages file by executing build/bin/exiv2.

```bash
$ sudo mkdir -p                          /usr/local/share/locale/xy/LC_MESSAGES
$ sudo cp -R  po/xy/LC_MESSAGES/exiv2.mo /usr/local/share/locale/xy/LC_MESSAGES
$ env LANG=xy exiv2                      # env LANGUAGE=xy on Linux!
exiv2: An action must be specified
exiv2: At least one file is required
Usage: exiv2 [ options ] [ action ] file ...

Manipulate image metadata.   <--------- Edited message!
$
```

2.5) Submitting your new language file for inclusion in future versions of Exiv2:

You may submit a PR which contains po/xy.po AND a modification to po/CMakeLists.txt

Or, open a new issue on https://github.com/exiv2/exiv2 and attach the file xy.po.zip which can be created as follows:

```
$ zip xy.po.zip po/xy.po
  adding: po/xy.po (deflated 78%)
ls -l xy.po.zip
-rw-r--r--+ 1 rmills  staff  130417 25 Jun 10:15 xy.po.zip
$
```

[TOC](#TOC)
<div id="2-9">
### 2.9 Building Exiv2 Documentation

Building documentation requires installing special tools.  You will probably prefer to
read the documentation on-line from the project website: https://exiv2.org

To build documentation, use the CMake option **`-DEXIV2_BUILD_DOC=On`**.
Additionally, you will require an additional build step to actually build the documentation.

```bash
$ cmake ..options.. -DEXIV2_BUILD_DOC=On
$ make doc
```

To build the documentation, you must install the following products:

| Product      | Availability |
|:------------ |:------------ |
| doxygen<br/>graphviz<br/>python<br/>xsltproc<br/>md5sum  | [http://www.doxygen.org/](http://www.doxygen.org/)<br/>[http://www.graphviz.org/](http://www.graphviz.org/)<br/>[http://www.python.org/](http://www.python.org/)<br/>[http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/)<br/>[http://www.microbrew.org/tools/md5sha1sum/](http://www.microbrew.org/tools/md5sha1sum/) |

[TOC](#TOC)
<div id="2-10">
### 2.10 Building Exiv2 Packages

To enable the building of Exiv2 packages, use the CMake option `-DEXIV2_TEAM_PACKAGING=On`.

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
CPack: - package: /path/to/exiv2/build/exiv2-0.27.1-Linux.tar.gz generated.
```

2) Source Package

```bash
$ make package_source
Run CPack packaging tool for source...
...
CPack: - package: /path/to/exiv2/build/exiv2-0.27.1-Source.tar.gz generated.
```

You may prefer to run `$ cmake --build . --config Release --target package_source`


[TOC](#TOC)
<div id="2-11">
### 2.11 Debugging Exiv2

1) Generating and installing a debug library

In general to generate a debug library, you should use the option *cmake* option `-DCMAKE_RELEASE_TYPE=Debug` and build in the usual way.

```bash
$ cd <exiv2dir>
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles" "-DCMAKE_BUILD_TYPE=Debug"
$ make

```

You must install the library to ensure that your code is linked to the debug library.

You can check that you have generated a debug build with the command:

```bash
$ exiv2 -vVg debug
exiv2 0.27.1
debug=1
$
```

[TOC](#TOC)

2) About preprocessor symbols `NDEBUG` and `EXIV2_DEBUG_MESSAGES`

Exiv2 respects the symbol `NDEBUG` which is set only for Release builds. There are sequences of code which are defined within:

```cpp
#ifdef EXIV2_DEBUG_MESSAGES
....
#endif
```

Those blocks of code are not compiled unless you define `EXIV2_DEBUG_MESSAGES`. They are provided for additional debugging information. For example, if you are interested in additional output from webpimage.cpp, you can update your build as follows:

```bash
$ cd <exiv2dir>
$ touch src/webpimage.cpp
$ make CXXFLAGS=-DEXIV2_DEBUG_MESSAGES
$ bin/exiv2 ...
-- or --
$ sudo make install
$ exiv2     ...
```

If you are debugging library code, it is recommended that you use the exiv2 command-line program as your test harness as Team Exiv2 is very familiar with this tool and able to give support.

[TOC](#TOC)

3) Starting the debugger

This is platform specific.  On Linux:

```bash
$ gdb exiv2
```

[TOC](#TOC)

4) Using Debugger IDEs such as Xcode, CLion, Visual Studio, Eclipse or QtCreator

I have used all those IDEs to debug the Exiv2 library and applications.  All of them work.  You may find it takes initial effort, however I assure you that they all work well.

I work on macOS and use Xcode to develop Exiv2.  For a couple of years, Team Exiv2 had free
open-source licences from JetBrains for CLion.  I really liked CLion as it is cross platform
and runs on Windows, Mac and Linux.  It has excellent integration with CMake and will automatically
add **`-DCMAKE_BUILD_TYPE=Debug`** to the cmake command.  It keeps build types in separate directories
such as **`<exiv2dir>/cmake-build-debug`**.

[TOC](#TOC)

5) cmake --build . options **`--config Release|Debug`** and **`--target install`**

Visual Studio and Xcode can build debug or release builds without using the option **`-DCMAKE_BUILD_TYPE`** because the generated project files can build multiple types.  The option **`--config Debug`** can be specified on the cmake command-line to specify the build type.  Alternatively, if you prefer to build in the IDE, the UI provides options to select the configuration and target.

With the Unix Makefile generator, the targets can be listed:

```bash
$ make help
The following are some of the valid targets for this Makefile:
... all (the default if no target is provided)
... clean
... depend
... install/local
.........
```

[TOC](#TOC)
<div id="2-12">
### 2.12 Building Exiv2 with **clang** and other build chains

1) On Linux

```bash
$ cd <exiv2dir>
$ rm -rf build ; mkdir build ; cd build
$ cmake .. -DCMAKE_C_COMPILER=$(which clang) -DCMAKE_CXX_COMPILER=$(which clang++)
$ cmake --build .
```

**_OR_**

```bash
$ export CC=$(which clang)
$ export CXX=$(which clang++)
$ cd <exiv2dir>
$ rm -rf build ; mkdir build ; cd build
$ cmake ..
$ cmake --build .
```

2) On macOS

Apple provide clang with Xcode.  GCC has not been supported by Apple since 2013.  The _"normal unix build"_ uses Clang.

3) On Cygwin, MinGW/msys2, Windows (using clang-cl) and Visual Studio.

I have been unable to get clang to work on any of those platforms.

[TOC](#TOC)
<div id="2-13">
### 2.13 Building Exiv2 with ccache

To speed up compilation, the utility ccache can be installed to cache the output of the compiler.  This greatly speeds up the build when you frequently built code that has not been modified.

Installing and using ccache (and other similar utilities), is platform dependent.  On Ubuntu:

```bash
$ sudo apt install --yes ccache
```

To build with ccache, use the cmake option **-DBUILD\_WITH\_CCACHE=On**

```bash
$ cd <exiv2dir>
$ mkdir build ; cd build ; cd build
$ cmake .. -G "Unix Makefiles" -DBUILD_WITH_CCACHE=On
$ make
# Build again to appreciate the performance gain
$ make clean
$ make
```

Due to the way in which ccache is installed in Fedora (and other Linux distros), ccache effectively replaces the compiler.  A default build or **-DBUILD\_WITH\_CCACHE=Off** is not effective and the environment variable CCACHE_DISABLE is required to disable ccache. [https://github.com/Exiv2/exiv2/issues/361](https://github.com/Exiv2/exiv2/issues/361)

[TOC](#TOC)
<div id="2-14">
### 2.14 Thread Safety

Exiv2 heavily relies on standard C++ containers. Static or global variables are used read-only, with the exception of the XMP namespace registration function (see below). Thus Exiv2 is thread safe in the same sense as C++ containers:
Different instances of the same class can safely be used concurrently in multiple threads.

In order to use the same instance of a class concurrently in multiple threads the application must serialize all write access to the object.

The level of thread safety within Exiv2 varies depending on the type of metadata: The Exif and IPTC code is reentrant. The XMP code uses the Adobe XMP toolkit (XMP SDK), which according to its documentation is thread-safe. It actually uses mutexes to serialize critical sections. However, the XMP SDK initialisation function is not mutex protected, thus Exiv2::XmpParser::initialize is not thread-safe. In addition, Exiv2::XmpProperties::registerNs writes to a static class variable, and is also not thread-safe.

Therefore, multi-threaded applications need to ensure that these two XMP functions are serialized, e.g., by calling them from an initialization section which is run before any threads are started.  All exiv2 sample applications begin with:

```cpp
#include <exiv2/exiv2.hpp>
int main(int argc, const char* argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF(true);
#endif
    ...
}
```
The use of the _**thread unsafe function**_ Exiv2::enableBMFF(true) is discussed in [2.19 Support for bmff files](#2-19)

[TOC](#TOC)
<div id="2-15">
### 2.15 Library Initialisation and Cleanup

As discussed in the section on Thread Safety, Exiv2 classes for Exif and IPTC metadata are fully reentrant and require no initialisation or cleanup.

Adobe's XMPsdk is generally thread-safe, however it has to be initialized and terminated before and after starting any threads to access XMP metadata. The Exiv2 library will initialize this if necessary, however it does not terminate the XMPsdk.

The exiv2 command-line program and sample applications call the following at the outset:

```cpp
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF(true);
#endif
```

[TOC](#TOC)
<div id="2-16">
### 2.16 Cross Platform Build and Test on Linux for MinGW

You can cross compile Exiv2 on Linux for MinGW.  We have used the following method on **Fedora** and believe this is also possible on Ubuntu and other distros.  Detailed instructions are provided here for **Fedora**.

### Cross Build and Test On Fedora

####1 Install the cross platform build tools

```bash
$ sudo dnf install mingw64-gcc-c++ mingw64-filesystem mingw64-expat mingw64-zlib cmake make
```

####2 Install Dependancies

You will need to install x86_64 libraries to support the options you wish to use.  By default, you will need libz and expat.  Your `dnf` command above has installed them for you.  If you wish to use features such as `webready` you should install openssl and libcurl as follows:

```bash
[rmills@rmillsmm-fedora 0.27-maintenance]$ sudo yum install libcurl.x86_64 openssl.x86_64
Last metadata expiration check: 0:00:18 ago on Fri 10 Apr 2020 10:50:30 AM BST.
Dependencies resolved.
=========================
Package                          Architecture                                        Version                                                      Repository                       Size
=========================
Installing:
...
```

####3 Get the code and build

```bash
$ git clone://github.com/exiv2/exiv2 --branch 0.27-maintenance exiv2
$ cd exiv2
$ mkdir build_mingw_fedora
$ mingw64-cmake ..
$ make
```

Note, you may wish to choose to build with optional features and/or build static libraries.  To do this, request appropriately on the mingw64-cmake command:

```bash
$ mingw64-cmake .. -DEXIV2_TEAM_EXTRA_WARNINGS=On \
                   -DEXIV2_ENABLE_VIDEO=On        \
                   -DEXIV2_ENABLE_WEBREADY=On     \
                   -DEXIV2_ENABLE_WIN_UNICODE=On  \
                   -DBUILD_SHARED_LIBS=Off
```
The options available for cross-compiling are the same as provided for all builds.  See: [Build Options](#2-3)


####4 Copy "system dlls" in the bin directory

These DLLs are required to execute the cross-platform build in the bin from Windows

```bash
for i in libexpat-1.dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll zlib1.dll ; do
    cp -v /usr/x86_64-w64-mingw32/sys-root/mingw/bin/$i bin
done
```

####5 Executing exiv2 in wine

You may wish to use wine to execute exiv2 from the command prompt.  To do this:

```bash
[rmills@rmillsmm-fedora build_mingw_fedora]$ wine cmd
Microsoft Windows 6.1.7601

Z:\Home\gnu\github\exiv2\0.27-maintenance\build_mingw_fedora>bin\exiv2
exiv2: An action must be specified
exiv2: At least one file is required
Usage: exiv2 [ options ] [ action ] file ...

Manipulate the Exif metadata of images.

Z:\Home\gnu\github\exiv2\0.27-maintenance\build_mingw_fedora>
```

If you have not installed wine, Fedora will offer to install it for you.

####6 Running the test suite

On a default wine installation, you are in the MSDOS/cmd.exe prompt.  You cannot execute the exiv2 test suite in this environment as you require python3 and MSYS/bash to run the suite.

You should mount the your Fedora exiv2/ directory on a Windows machine on which you have installed MinGW/msys2.  You will need python3 and make.

My build machines is a MacMini with VMs for Windows, Fedora and other platforms.  On Fedora, I build in a Mac directory which is shared to all VMs.

```bash
[rmills@rmillsmm-fedora 0.27-maintenance]$ pwd
/media/psf/Home/gnu/github/exiv2/0.27-maintenance
[rmills@rmillsmm-fedora 0.27-maintenance]$ ls -l build_mingw_fedora/bin/exiv2.exe
-rwxrwxr-x. 1 rmills rmills 754944 Apr 10 07:44 build_mingw_fedora/bin/exiv2.exe
[rmills@rmillsmm-fedora 0.27-maintenance]$
```

On MinGW/msys2, I can directly access the share:

```bash
$ cd //Mac/Home/gnu/github/exiv2/0.27/maintenance/build_mingw_fedora
$ export EXIV2_BINDIR=$pwd/bin
$ cd ../test
$ make tests
```

You will find that 3 tests fail at the end of the test suite.  It is safe to ignore those minor exceptions.

[TOC](#TOC)
<div id="2-17">
### 2.17 Building with C++11 and other compilers

Exiv2 uses the default compiler for your system.  Exiv2 v0.27 was written to the C++ 1998 standard and uses auto\_ptr.  The C++11 and C++14 compilers will issue deprecation warnings about auto\_ptr.  As _auto\_ptr support has been removed from C++17, you cannot build Exiv2 v0.27 with C++17 or later compilers._  Exiv2 v0.28 and later do not use auto\_ptr and will build with all modern C++ Standard Compilers.

To build with C++11:

```bash
$ cd <exiv2dir>
$ mkdir build ; cd build
$ cmake .. -DCMAKE_CXX_STANDARD=11 -DCMAKE_CXX_FLAGS=-Wno-deprecated
$ make
```

The option -DCMAKE\_CXX\_STANDARD=11 specifies the C++ Language Standard.  Possible values are 98, 11 or 14.

The option -DCMAKE\_CXX\_FLAGS=-Wno-deprecated suppresses warnings from C++11 concerning auto\_ptr.  The compiler will issue deprecation warnings about video, eps and ssh code in Exiv2 v0.27.  This is intentional.  These features of Exiv2 will not be available in Exiv2 v0.28. 

**Caution:** Visual Studio users should not use -DCMAKE\_CXX\_FLAGS=-Wno-deprecated.

[TOC](#TOC)
<div id="2-18">
### 2.18 Static and Shared Libraries

You can build either static or shared libraries.  Both can be linked with either static or shared run-time libraries.  You specify the shared/static with the option `-BUILD_SHARED_LIBS=On|Off` You specify the run-time with the option `-DEXIV2_ENABLE_DYNAMIC_RUNTIME=On|Off`.  The default for both options default is On.  So you build shared and use the shared libraries which are `.dll` on Windows (msvc, Cygwin and MinGW/msys), `.dylib` on macOS and `.so` on Linux and UNIX.  

CMake creates your build artefacts in the directories `bin` and `lib`.  The `bin` directory contains your executables and .dlls.  The `lib` directory contains your static libraries.  When you install exiv2, the build artefacts are copied to your system's prefix directory which by default is `/usr/local/`.  If you wish to test and use your build without installing, you will have to set you PATH appropriately.  Linux/Unix users should also set `LD_LIBRARY_PATH` and macOS users should set `DYLD_LIBRARY_PATH`.

The default build is SHARED/DYNAMIC and this arrangement treats all executables and shared libraries in a uniform manner.

**Caution:** _The following discussion only applies if you are linking to a static version of the exiv2 library._  You may get the following error from CMake:

```bash
CMake Error at src/CMakeLists.txt:30 (add_library):
Target "my-app-or-library" links to target "Iconv::Iconv" but the target was
not found. Perhaps a find_package() call is missing for an IMPORTED
target, or an ALIAS target is missing?
```

Be aware that the warning concerning `src/CMakeLists.txt:30 (add_library)` refers to your file src/CMakeLists.txt.  Although exiv2 has statically linked `Iconv()`, your code also needs to link.  You achieve that in your src/CMakeLists.txt with the code:

```cmake
find_package(Iconv)
if( ICONV_FOUND )
    target_link_libraries( my-app-or-library PRIVATE Iconv::Iconv )
endif()
```

This is discussed: [https://github.com/Exiv2/exiv2/issues/1230](https://github.com/Exiv2/exiv2/issues/1230)

[TOC](#TOC)
<div id="2-19">
### 2.19 Support for bmff files (CR3, HEIF, HEIC, and AVIF)

**Attention is drawn to the possibility that bmff support may be the subject of patent rights. _Exiv2 shall not be held responsible for identifying any or all such patent rights.  Exiv2 shall not be held responsible for the legal consequences of the use of this code_.**

Access to the bmff code is guarded in two ways.  Firstly, you have to build the library with the cmake option: `-DEXIV2_ENABLE_BMFF=On`.  Secondly, the application must enable bmff support at run-time by calling the following function.

```cpp
EXIV2API bool enableBMFF(bool enable);
```

The return value from `enableBMFF()` is true if the library has been build with bmff support (cmake option -DEXIV2_ANABLE_BMFF=On).

Applications may wish to provide a preference setting to enable bmff support and thereby place the responsibility for the use of this code with the user of the application.

[TOC](#TOC)
<div id="3">
## 3 License and Support

All project resources are accessible from the project website.
    https://github.com/Exiv2/exiv2

<div id="3-1">
### 3.1 License

Copyright (C) 2004-2021 Exiv2 authors.
You should have received a copy of the file [COPYING](COPYING) which details the GPLv2 license.

Exiv2 is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Exiv2 program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


[TOC](#TOC)
<div id="3-2">
### 3.2 Support

For new bug reports, feature requests and support:  Please open an issue in Github.  [https://github.com/exiv2/exiv2](https://github.com/exiv2/exiv2)

[TOC](#TOC)
<div id="4">
## 4 Running the test suite

#### Different kinds of tests:

| Description        | Language  | Location       | Command<br>_(in build directory)_ | CMake Option to Build |
|:--                 |:--        |:--             |:--                     |:--   |
| Run all tests      |           |                                   | $ make tests                             |  |
| Run all tests      |           | **Visual Studio Users**           | > cmake --build . --target tests         |  |
| Bash tests         | python    | tests/bash\_tests       | $ make bash_tests    | -DEXIV2\_BUILD\_SAMPLES=On     |
| Python tests       | python    | tests                   | $ make python_tests  | -DEXIV2\_BUILD\_SAMPLES=On     |
| Unit tests         | C++       | unitTests               | $ make unit_test     | -DEXIV2\_BUILD\_UNIT\_TESTS=On |
| Version test       | C++       | src/version.cpp         | $ make version_test  | Always in library              |

The term _**bash scripts**_ is historical.  The implementation of the tests in this collection originally required bash.  These
scripts have been rewritten in python.  Visual Studio Users will appreciate the python implementation as it avoids the
installation of mingw/cygwin and special PATH settings.

#### Environment Variables used by the test suite:

If you build the code in the directory \<exiv2dir\>build, tests will run using the default values of Environment Variables.

| Variable           | Default                    | Platforms     | Purpose |
|:--                 |:--                         |:--            |:--      |
| EXIV2_BINDIR       | **\<exiv2dir\>/build/bin** | All Platforms | Path of built binaries (exiv2.exe) |
| EXIV2_PORT         | **12762**<br>**12671**<br>**12760**  | Cygwin<br>MinGW/msys2<br>Other Platforms | Test TCP/IP Port   |
| EXIV2_HTTP         | **http://localhost**       | All Platforms | Test http server   |
| EXIV2_ECHO         | _**not set**_              | All Platforms | For debugging bash scripts |
| VALGRIND           | _**not set**_              | All Platforms | For debugging bash scripts |
| VERBOSE            | _**not set**_              | All Platforms | Causes make to report its actions |
| PATH<br>DYLD\_LIBRARY\_PATH<br>LD\_LIBRARY\_PATH    | $EXIV2\_BINDIR/../lib | Windows<br>macOS<br>Other platforms | Path of dynamic libraries |

The Variable EXIV2\_PORT or EXIV2\_HTTP can be set to None to skip http tests.  The http server is started with the command `python3 -m http.server $port`.  On Windows, you will need to run this manually _**once**_ to authorise the firewall to permit python to use the port.

[TOC](#TOC)
<div id="4-1">
### 4.1 Running tests on a UNIX-like system

You can run tests directly from the build:

```bash
$ cmake .. -G "Unix Makefiles" -DEXIV2_BUILD_UNIT_TESTS=On 
$ make
... lots of output ...
$ make tests
... lots of output ...
$
```

You can run individual tests in the `test` directory.  **Caution:** If you build in a directory other than \<exiv2dir\>/build, you must set EXIV2\_BINDIR to run tests from the `test` directory.

```bash
$ cd <exiv2dir>/build
$ make bash_tests
addmoddel_test (testcases.TestCases) ... ok
....
Ran 176 tests in 9.526s
OK (skipped=6)

$ make python_tests
... lots of output ...
test_run (tiff_test.test_tiff_test_program.TestTiffTestProg) ... ok
----------------------------------------------------------------------
Ran 176 tests in 9.526s
OK (skipped=6)
$
```

[TOC](#TOC)
<div id="4-2">
### 4.2 Running tests on Visual Studio builds from cmd.exe

**Caution:** _The python3 interpreter must be on the PATH, build for DOS, and called python3.exe.  I copied the python.exe program:

```cmd
> copy c:\Python37\python.exe c:\Python37\python3.exe
> set "PATH=c:\Python37;%PATH%
```

You can execute the test suite as described for UNIX-like systems.
The main difference is that you must use cmake to initiate the test
as make is not a system utility on Windows.

```bash
> cd <exiv2dir>/build
> cmake --build . --target tests
> cmake --build . --target python_tests
```

##### Running tests from cmd.exe

You can build with Visual Studio using Conan.  The is described in detail in [README-CONAN.md](README-CONAN.md)

As a summary, the procedure is:

```
c:\...\exiv2>mkdir build
c:\...\exiv2>cd build
c:\...\exiv2\build>conan install .. --build missing --profile msvc2019Release
c:\...\exiv2\build>cmake .. -DEXIV2_BUILD_UNIT_TESTS=On -G "Visual Studio 16 2019"
c:\...\exiv2\build>cmake --build . --config Release
... lots of output from compiler and linker ...
c:\...\exiv2\build>
```

If you wish to use an environment variables, use set:

```
set VERBOSE=1
cmake --build . --config Release --target tests
set VERBOSE=
```

[TOC](#TOC)
<div id="4-3">
### 4.3 Unit tests

The code for the unit tests is in `<exiv2dir>/unitTests`.  To include unit tests in the build, use the *cmake* option `-DEXIV2_BUILD_UNIT_TESTS=On`.

There is a discussion on the web about installing GTest: [https://github.com/Exiv2/exiv2/issues/575](https://github.com/Exiv2/exiv2/issues/575)

```bash
$ pushd /tmp
$ curl -LO https://github.com/google/googletest/archive/release-1.8.0.tar.gz
$ tar xzf   release-1.8.0.tar.gz
$ mkdir -p  googletest-release-1.8.0/build
$ pushd     googletest-release-1.8.0/build
$ cmake .. ; make ; make install
$ popd
$ popd
```

[TOC](#TOC)
<div id="4-4">
### 4.4 Python tests

You can run the python tests from the build directory:

```bash
$ cd <exiv2dir>/build
$ make python_tests  
```

If you wish to run in verbose mode:

```bash
$ cd <exiv2dir>/build
$ make python_tests VERBOSE=1
```

The python tests are stored in the directory tests and you can run them all with the command:

```bash
$ cd <exiv2dir>/tests
$ export LD_LIBRARY_PATH="$PWD/../build/lib:$LD_LIBRARY_PATH"
$ python3 runner.py
```

You can run them individually with the commands such as:

```bash
$ cd <exiv2dir>/tests
$ python3 runner.py --verbose bugfixes/redmine/test_issue_841.py  # or $(find . -name "*841*.py")
```

You may wish to get a brief summary of failures with commands such as:

```bash
$ cd <exiv2dir>/build
$ make python_tests 2>&1 | grep FAIL
```

[TOC](#TOC)
<div id="4-5">
### 4.5 Test Summary

| *Tests*      | Unix Style Platforms _(bash)_      | Visual Studio _(cmd.exe)_             |
|:--           |:---                                |:--                                    |
|              | $ cd \<exiv2dir\>/build            |  \> cd \<exiv2dir\>/build             |
| tests        | $ make tests                       | \> cmake --build . --config Release --target tests |
| bash_tests   | $ make bash_tests                  | \> cmake --build . --config Release --target bash_tests |
| python_tests | $ make python_tests                | \> cmake --build . --config Release --target python_tests |
| unit_test    | $ make unit_test                   | \> cmake --build . --config Release --target unit_test |
| version_test | $ make version_test                | \> cmake --build . --config Release --target version_test |

The name **bash_tests** is historical.  They are implemented in python.

[TOC](#TOC)
<div id="5">
## 5 Platform Notes

There are many ways to set up and configure your platform.  The following notes are provided as a guide.

<div id="5-1">
### 5.1 Linux

Update your system and install the build tools and dependencies (zlib, expat, gtest and others)

```bash
$ sudo apt --yes update
$ sudo apt install --yes build-essential git clang ccache python3 libxml2-utils cmake python3 libexpat1-dev libz-dev zlib1g-dev libssh-dev libcurl4-openssl-dev libgtest-dev google-mock
```

For users of other platforms, the script <exiv2dir>/ci/install_dependencies.sh has code used to configure many platforms.  The code in that file is a useful guide to configuring your platform.

Get the code from GitHub and build

```bash
$ mkdir -p ~/gnu/github/exiv2
$ cd ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

[TOC](#TOC)
<div id="5-2">
### 5.2 macOS

You will need to install Xcode and the Xcode command-line tools to build on macOS.

You should build and install libexpat and zlib.  You may use brew, macports, build from source, or use conan.

I recommend that you build and install CMake from source.

[TOC](#TOC)
<div id="5-3">
### 5.3 MinGW/msys2

Please note that the platform MinGW/msys2 32 is obsolete and superceded by MinGW/msys2 64.

#### MinGW/msys2 64 bit
Install: [https://repo.msys2.org/distrib/x86\_64/msys2-x86\_64-20200903.exe](https://repo.msys2.org/distrib/x86_64/msys2-x86_64-20200903.exe)

The file `appveyor_mingw_cygwin.yml` has instructions to configure the AppVeyor CI to configures itself to build Exiv2 on MinGW/msys2 and Cygwin/64.

I use the following batch file to start the MinGW/msys2 64 bit bash shell from the Dos Command Prompt (cmd.exe)

```bat
@echo off
setlocal
set "PATH=c:\msys64\mingw64\bin;c:\msys64\usr\bin;c:\msys64\usr\local\bin;"
set "PS1=\! MSYS \u@\h:\w \$ "
set "HOME=c:\msys64\home\rmills"
if NOT EXIST %HOME% mkdir %HOME%
cd  %HOME%
color 1f
c:\msys64\usr\bin\bash.exe -norc
endlocal

```

#### Install MinGW Dependencies

Install tools and dependencies:

```bash
for i in base-devel git coreutils dos2unix tar diffutils make                     \
    mingw-w64-x86_64-toolchain mingw-w64-x86_64-gcc      mingw-w64-x86_64-gdb     \
    mingw-w64-x86_64-cmake     mingw-w64-x86_64-gettext  mingw-w64-x86_64-python3 \
    mingw-w64-x86_64-libexpat  mingw-w64-x86_64-libiconv mingw-w64-x86_64-zlib    \
    mingw-w64-x86_64-gtest
do (echo y | pacman -S $i) ; done
```

#### Download exiv2 from github and build

```bash
$ mkdir -p ~/gnu/github/exiv2
$ cd       ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"   # or "MSYS Makefiles"
$ make
```

#### MinGW and Regex

The exiv2 command-line program provides an option **`--grep`** to filter output.  The implementation requires the header file **`<regex.h>`** and supporting library to be available during the build.  When not available, the option **`--grep`** degrades to a substring match.  Because there are several versions of **`<regex.h>`** available on the MinGW platform, detection of regex is always disabled on this platform and uses substring match.  The following command reveals if regex is included in your build:

```bash
$ exiv2 -vVg regex
exiv2 0.27.1
have_regex=1
$
```

[TOC](#TOC)
<div id="5-4">
### 5.4 Cygwin/64

Please note that the platform Cygwin/32 is obsolete and superceded by Cygwin/64.

Download: [https://cygwin.com/install.html](https://cygwin.com/install.html) and run setup-x86_64.exe.  I install into c:\\cygwin64

You need:
make, cmake, curl, gcc, gettext-devel pkg-config, dos2unix, tar, zlib-devel, libexpat1-devel, git, libxml2-devel python3-interpreter, libiconv, libxml2-utils, libncurses, libxml2-devel libxslt-devel python38 python38-pip python38-libxml2

The file `appveyor_mingw_cygwin.yml` has instructions to configure the AppVeyor CI to configures itself to build Exiv2 on MinGW/msys2 and Cygwin/64.

To build unit tests, you should install googletest-release-1.8.0 as discussed [4.3 Unit tests](#4-3)

I use the following batch file "cygwin64.bat" to start the Cygwin/64 bash shell from the Dos Command Prompt (cmd.exe).

```bat
@echo off
setlocal
set "PATH=c:\cygwin64\usr\local\bin;c:\cygwin64\bin;c:\cygwin64\usr\bin;c:\cygwin64\usr\sbin;"
if NOT EXIST %HOME% mkdir %HOME%
set "HOME=c:\cygwin64\home\rmills"
cd  %HOME%
set "PS1=\! CYGWIN64:\u@\h:\w \$ "
bash.exe -norc
endlocal
```

[TOC](#TOC)
<div id="5-5">
### 5.5 Visual Studio

We recommend that you use Conan to build Exiv2 using Visual Studio. Exiv2 v0.27 can be built with Visual Studio versions 2008 and later.  We actively support and build with Visual Studio 2015, 2017 and 2019.

As well as Visual Studio, you will need to install CMake, Python3, and Conan.

1) Binary installers for CMake on Windows are availably from [https://cmake.org/download/](https://cmake.org/download/).<br/>
2) Binary installers for Python3 are available from [python.org](https://python.org)<br/>
3) Conan can be installed using python/pip.  Details in [README-CONAN.md](README-CONAN.md)


```
..>copy c:\Python37\python.exe c:\Python37\python3.exe
```

The python3 interpreter must be on your PATH.

[TOC](#TOC)
<div id="5-6">
### 5.6 Unix

Exiv2 can be built on many Unix and Linux distros.  With v0.27.2, we are starting to actively support the Unix Distributions NetBSD and FreeBSD.  For v0.27.3, I have added support for Solaris 11.4

We do not have CI support for these platforms on GitHub.  However, I regularly build and test them on my MacMini Buildserver.  The device is private and not on the internet.

I have provided notes here based on my experience with these platforms.   Feedback is welcome.  I am willing to support Exiv2 on other commercial Unix distributions such as AIX, HP-UX and OSF/1 if you provide with an ssh account for your platform.  I will require super-user privileges to install software.

For all platforms you will need the following components to build:

1. gcc or clang
2. cmake
3. bash
4. sudo
5. gettext

To run the test suite, you need:

1. python3
2. chksum
3. dos2unix
4. xmllint

#### NetBSD

You can build exiv2 from source using the methods described for linux.  I built and installed exiv2 using "Pure CMake" and didn't require conan.

You will want to use the package manager `pkgsrc` to build/install the build and test components listed above.

I entered links into the file system

```
# ln -s /usr/pkg/bin/python37 /usr/local/bin/python3
# ln -s /usr/pkg/bin/bash /bin/bash`
```

It's important to ensure that `LD_LIBRARY_PATH` includes `/usr/local/lib` and `/usr/pkg/lib`.

It's important to ensure that `PATH` includes `/usr/local/bin`, `/usr/pkg/bin` and `/usr/pkg/sbin`.

#### FreeBSD

Clang is pre-installed as ``/usr/bin/{cc|c++}` as well as libz and expat.  FreeBSD uses pkg as the package manager which I used to install cmake and git.

```bash
$ su root
Password:
# pkg install cmake
# pkg install git
# pkg install bash
# pkg install python
```

**Caution**: _The package manager *pkg* is no longer working on FreeBSD 12.0.  I will move to 12.1 for future work.  Others have reported this issue on 12.1.  Broken package manager is very bad news.  There are other package managers (such as ports), however installing and getting it to work is formidable._

```
634 rmills@rmillsmm-freebsd:~/gnu/github/exiv2/0.27-maintenance/build $ sudo pkg install libxml2
Updating FreeBSD repository catalogue...
pkg: repository meta /var/db/pkg/FreeBSD.meta has wrong version 2
pkg: Repository FreeBSD load error: meta cannot be loaded No error: 0
Fetching meta.txz: 100%    916 B   0.9kB/s    00:01
pkg: repository meta /var/db/pkg/FreeBSD.meta has wrong version 2
repository FreeBSD has no meta file, using default settings
Fetching packagesite.txz: 100%    6 MiB 340.2kB/s    00:19
pkg: repository meta /var/db/pkg/FreeBSD.meta has wrong version 2
pkg: Repository FreeBSD load error: meta cannot be loaded No error: 0
Unable to open created repository FreeBSD
Unable to update repository FreeBSD
Error updating repositories!
635 rmills@rmillsmm-freebsd:~/gnu/github/exiv2/0.27-maintenance/build $
```

#### Solaris

Solaris uses the package manager pkg.  To get a list of packages:

```bash
$ pkg list
```

To install a package:

```bash
$ sudo pkg install developer/gcc-7
```


[TOC](#TOC)

Written by Robin Mills<br>robin@clanmills.com<br>Updated: 2021-04-06

