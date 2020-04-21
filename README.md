| Travis        | AppVeyor      | GitLab| Codecov| Repology|
|:-------------:|:-------------:|:-----:|:------:|:-------:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=0.27-maintenance)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/0.27-maintenance?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/0.27-maintenance) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/0.27-maintenance/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/0.27-maintenance) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/0.27-maintenance/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) | [![Packaging status](https://repology.org/badge/tiny-repos/exiv2.svg)](https://repology.org/metapackage/exiv2/versions) |
<div id="TOC">

### TABLE  OF  CONTENTS

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
3. [License and Support](#3)
    1. [License](#3-1)
    2. [Support](#3-2)
4. [Test Suit](#4)
    1. [Running tests on a UNIX-like system](#4-1)
    2. [Running tests on Visual Studio builds](#4-2)
    3. [Unit tests](#4-3)
5. [Platform Notes](#5)
    1. [Linux](#5-1)
    2. [macOS](#5-2)
    3. [MinGW](#5-3)
    4. [Cygwin](#5-4)
    5. [Microsoft Visual C++](#5-5)
    6. [Unix](#5-6)

<div id="1">

# Welcome to Exiv2

![Exiv2](exiv2.png)

Exiv2 is a C++ library and a command line utility to read,
write, delete and modify Exif, IPTC, XMP and ICC image metadata.

| Exiv2 Resource              | Location |
|:------                      |:----     |
| Releases and Documentation  | [https://exiv2.org](https://exiv2.org) |
| Prereleases:                | [https://pre-release.exiv2.org](https://pre-release.exiv2.org) |
| Project Resources           | [https://github.com/Exiv2/exiv2](https://github.com/Exiv2/exiv2) |
| License (GPLv2)             | [COPYING](COPYING) |
| CMake Downloads             | [https://cmake.org/download/](https://cmake.org/download/) |

The file ReadMe.txt in a build bundle describes how to install the library on the platform.  ReadMe.txt also documents how to compile and link code on the platform.

[TOC](#TOC)
<div id="2">

## 2 Building, Installing, Using and Uninstalling Exiv2

You need [CMake](https://cmake.org/download/) to configure the Exiv2 project and the GCC or Clang compiler and associated tool chain.

<div id="2-1">

### 2.1 Build, Install, Use Exiv2 on a UNIX-like system

```bash
cd ~/gnu/github/exiv2  # location of the project code
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
make tests
sudo make install
```

This will install the library into the "standard locations".  The library will be installed in `/usr/local/lib`, executables (including the exiv2 command-line program) in `/usr/local/bin/` and header files in `/usr/local/include/exiv2`

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



[TOC](#TOC)
<div id="2-2">

### 2.2 Build and Install Exiv2 with Visual Studio

We recommend that you use conan to download the Exiv2 external dependencies on Windows (On Linux/OSX you can use or install system packages).
Apart from handling the dependencies, to configure and compile the project is pretty similar to the UNIX like systems.
See [README-CONAN](README-CONAN.md) for more information

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
577 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $
```

Options are defined on the CMake command line:
```bash
$ cmake -DBUILD_SHARED_LIBS=On -DEXIV2_ENABLE_NLS=OFF
```

[TOC](#TOC)
<div id="2-4">

### 2.4 Dependencies

The following Exiv2 features require external libraries:

| Feature                     | Package   |  Default | To change default             | Availability |
|:--------------------------  |:--------  |:--------:| :---------------------------- |:-----------  |
| PNG image support           | zlib      | ON       | -DEXIV2\_ENABLE\_PNG=Off      | [http://zlib.net/](http://zlib.net/) |
| XMP support                 | expat     | ON       | -DEXIV2\_ENABLE\_XMP=Off      | [http://expat.sourceforge.net](http://expat.sourceforge.net)/<br/>Use _**Expat 2.2.6**_ and later |
| Natural language system     | gettext   | OFF      | -DEXIV2\_ENABLE\_NLS=On       | [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) |

On UNIX systems, you may install the dependencies using the distribution's package management system. Install the
development package of a dependency to install the header files and libraries required to build Exiv2. In the file
`ci/install_dependencies.sh` you can check to the list of packages we install on different Linux distributions. This
file is used to setup some CI images in which we try out the Exiv2 compilation.

Natural language system is discussed in more detail here: [Localisation](#2-8)

Notes about different platforms are included here: [Platform Notes](#5)

You may choose to install dependences with conan.  This is supported on all platforms and is especially useful for users of Visual Studio.
See [README-CONAN](README-CONAN.md) for more information.

[TOC](#TOC)

<div id="2-5">

### 2.5 Building and linking your code with Exiv2

There are detailed platform notes about compiling and linking in `releasenotes/{platform}/ReadMe.txt`

where `platform: { CYGWIN | macOS | Linux | MinGW | msvc }`

In general you need to do the following:

1) Application code should be written in C++98 and include exiv2 headers:

```C++
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
$ *** EDIT CMakeLists.txt ***
$ cat CMakeLists.txt
cmake_minimum_required(VERSION 3.8)
project(exifprint VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(exiv2 REQUIRED CONFIG NAMES exiv2)    # search ${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2/
add_executable(exifprint ../samples/exifprint.cpp) # compile this
target_link_libraries(exifprint exiv2lib)          # link exiv2lib

$ cmake .                                          # generate the makefile
$ make                                             # build the code
$ ./exifprint                                      # test your executable
Usage: ./exifprint [ file | --version || --version-test ]
$
```

This [repository](https://github.com/piponazo/exiv2Consumer) shows an example of how to consume Exiv2 with CMake.

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

To build localisation support, use the CMake option `-DEXIV2_ENABLE_NLS=ON`.  You must install the `gettext` package with your package manager or from source.  The `gettext` package is available from [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) and includes the library `libintl` and utilities to build localisation files.  If CMake produces error messages which mention libintl or gettext, you should verify that the package `gettext` has been correctly built and installed.

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
$ cmake ..options.. -DEXIV2_BUILD_DOC=ON
$ make doc
```

To build the documentation, you must install the following products:

| Product      | Availability |
|:------------ |:------------ |
| doxygen<br/>graphviz<br/>python<br/>xsltproc<br/>md5sum  | [http://www.doxygen.org/](http://www.doxygen.org/)<br/>[http://www.graphviz.org/](http://www.graphviz.org/)<br/>[http://www.python.org/](http://www.python.org/)<br/>[http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/)<br/>[http://www.microbrew.org/tools/md5sha1sum/](http://www.microbrew.org/tools/md5sha1sum/) |

[TOC](#TOC)
<div id="2-10">

### 2.10 Building Exiv2 Packages

To enable the building of Exiv2 packages, use the CMake option `-DEXIV2_TEAM_PACKAGING=ON`.

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

```C++
#ifdef EXIV2_DEBUG_MESSAGES
....
#endif
```

Those blocks of code are not compiled unless you define `EXIV2_DEBUG_MESSAGES` by yourself. They are provided for additional debugging information. For example, if you are interested in additional output from webpimage.cpp, you can update your build as follows:

```bash
$ cd <exiv2dir>
$ touch src/webpimage.cpp
$ make CXXFLAGS=-DEXIV2_DEBUG_MESSAGESDEBUG
$ bin/exiv2 ...
-- or --
$ sudo make install
$ exiv2     ...
```

If you are debugging library code, it is recommended that you use the exiv2 command-line as your test harness as Team Exiv2 is very familiar with this tool and able to give support.

[TOC](#TOC)

3) Starting the debugger

This is platform specific.  On Linux:

```bash
$ gdb exiv2
```

[TOC](#TOC)

4) Using Debugger IDEs such as Xcode, CLion, Visual Studio, Eclipse or QtCreator

I have used all those IDEs to debug the Exiv2 library and applications.  All of them work.  You may find it takes initial effort, however I assure you that they all work well.

I personally use CLion which has excellent integration with CMake.  It will automatically add **`-DCMAKE_BUILD_TYPE=Debug`** to the cmake command.  It keeps build types in separate directories such as **`<exiv2dir>/cmake-build-debug`**.

[TOC](#TOC)

5) cmake --build . options **`--config Release|Debug`** and **`--target install`**

Visual Studio and Xcode can build debug or release builds without using the option **`-DCMAKE_BUILD_TYPE`** because the generated project files can build multiple types.  The option **`--config Debug`** can be specified on the command-line to specify the build type.  Alternatively, if you prefer to build in the IDE, the UI provides options to select the configuration and target.

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

Therefore, multi-threaded applications need to ensure that these two XMP functions are serialized, e.g., by calling them from an initialization section which is run before any threads are started.

[TOC](#TOC)

<div id="2-15">

### 2.15 Library Initialisation and Cleanup

As discussed in the section on Thread Safety, Exiv2 classes for Exif and IPTC metadata are fully reentrant and require no initialisation or cleanup.

Adobe's XMPsdk is generally thread-safe, however it has to be initialized and terminated before and after starting any threads to access XMP metadata. The Exiv2 library will initialize this if necessary, however it does not terminate the XMPsdk.

The Exiv2 command-line and the sample applications call the following at the outset:

```
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
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
$ mingw64-cmake .. -DEXIV2_TEAM_EXTRA_WARNINGS=ON \
                   -DEXIV2_ENABLE_VIDEO=ON        \
                   -DEXIV2_ENABLE_WEBREADY=ON     \
                   -DEXIV2_ENABLE_WIN_UNICODE=ON  \
                   -DBUILD_SHARED_LIBS=OFF
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

On a default wine installation, you are in the MSDOS/cmd prompt.  You cannot execute the exiv2 test suite in this environment as you require python3 and MSYS/bash to run the suite.

You should mount the your Fedora exiv2/ directory on a Windows machine on which you have installed MSYS2.  You will need python3 and make.

My build machines is a MacMini with VMs for Windows, Fedora and other platforms.  On Fedora, I build in a Mac directory which is shared to all VMs.

```bash
[rmills@rmillsmm-fedora 0.27-maintenance]$ pwd
/media/psf/Home/gnu/github/exiv2/0.27-maintenance
[rmills@rmillsmm-fedora 0.27-maintenance]$ ls -l build_mingw_fedora/bin/exiv2.exe
-rwxrwxr-x. 1 rmills rmills 754944 Apr 10 07:44 build_mingw_fedora/bin/exiv2.exe
[rmills@rmillsmm-fedora 0.27-maintenance]$
```

On MSYS2, I can directly access the share:

```bash
$ cd //Mac/Home/gnu/github/exiv2/0.27/maintenance/build_mingw_fedora
$ export EXIV2_BINDIR=$pwd/bin
$ export EXIV2_EXT=.exe
$ cd ../test
$ make tests
```

You will find that 3 tests fail at the end of the test suite.  It is safe to ignore those minor exceptions.

[TOC](#TOC)

<div id="3">

## 3 License and Support

All project resources are accessible from the project website.
    https://github.com/Exiv2/exiv2

<div id="3-1">

### 3.1 License

Copyright (C) 2004-2019 Exiv2 authors.
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
For new bug reports and feature requests, please open an issue in Github.

[TOC](#TOC)
<div id="4">

## 4 Running the test suite

There are different kinds of tests:

| Description        | Language | Location | Command<br>_(in build or test directory)_ | CMake Option to Build |
|:--                 |:--        |:--      |:--                                        |:--   |
| Run all tests      |           |                      | $ make tests     | |
| Bash scripts       | bash   | \<exiv2dir\>/test    | $ make bash_tests   | -DEXIV2\_BUILD\_SAMPLES=On |
| Python scripts     | python | \<exiv2dir\>/tests   | $ make python_tests  | -DEXIV2\_BUILD\_SAMPLES=On |
| Unit tests         | C++    | \<exiv2dir\>/unitTests   | $ make unit_test | -DEXIV2\_BUILD\_UNIT\_TESTS=On |
| Version test       | C++    | \<exiv2dir\>/src/version.cpp | $ make version_test | Always in library |

**Caution Visual Studio Users using cmd.exe**<br>_You may use `make` to to execute tests in the test directory.  To execute tests from the build directory, use `cmake`._  This is discussed in detail below: [Running tests on Visual Studio builds](#4-2)

Environment Variables used by test suite

| Variable        | Default | Platforms | Purpose |
|:--                 |:--        |:--     |:-- |
| EXIV2_BINDIR       | **\<exiv2dir\>/build/bin** | All Platforms | Locatation of built binary object (exiv2.exe) |
| EXIV2_EXT          | **.exe**  | msvc<br>Cygwin<br>Msys<br>MinGW | Extension used by executable binaries |
| EXIV2_EXT          | _**not set**_  | Linux<br>macOS<br>Unix|  |
| EXIV2_ECHO          | _**not set**_ | All Platforms | For debugging Bash scripts |
| VALGRIND            | _**not set**_ | All Platforms | For debugging Bash scripts |

<div id="4-1">

### 4.1 Running tests on a UNIX-like system

You can run the suite directly from the build:

```bash
$ cmake .. -G "Unix Makefiles"
$ make
...
$ make tests
... lots of output ...
Summary report
```

You can run individual tests in the `test` directory using the environment variable EXIV2\_BINDIR to specify the location of the build artifacts.  For Windows builds (msvc, Cygwin, Msys, MinGW), set EXIV2_EXT=.exe

```bash
$ cd <exiv2dir>/build
$ cd ../test
$ env EXIV2_BINDIR=${PWD}/../build/bin ./icc-test.sh
ICC jpg md5 webp md5 png md5 jpg md5
all testcases passed.

$ env EXIV2_BINDIR=${PWD}/../build/bin make python_tests
... lots of output ...
test_run (tiff_test.test_tiff_test_program.TestTiffTestProg) ... ok
----------------------------------------------------------------------
Ran 176 tests in 9.526s
OK (skipped=6)
```

[TOC](#TOC)
<div id="4-2">

### 4.2 Running tests on Visual Studio builds

To run the bash scripts you will need to install MinGW/msys2 which provides you with the bash interpreter.  You can run the test suite from bash, or from cmd.exe.

##### Running tests from MinGW/msys2 bash

Use the bash interpreter for MinGW/msys2 to run the test suite.  It's essential to have a DOS Python3 interpreter on your path called `python3.exe`  The variables EXIV2\_BINDIR and EXIV2\_EXT enable the test suite to locate the MSVC build artifacts.

```bash
$ cd <exiv2dir>/build
$ cd ../test
$ PATH="/c/Python37:$PATH"
$ export EXIV2_EXT=.exe
$ export EXIV2_BINDIR=${PWD}/../build/bin
```

**Caution:** _The python3 interpreter must be for DOS and called python3.exe.  I copied the python.exe program:_

```
..>copy c:\Python37\python.exe c:\Python37\python3.exe
```

Once you have modified the PATH and exported EXIV2\_BINDIR and EXIV2\_EXT, you can execute the test suite as described for UNIX-like systems:

```bash
$ cd <exiv2dir>/test
$ make tests
$ make python_tests
$ ./icc-test.sh
```
##### Running tests suite from cmd.exe

You can build with Visual Studio using Conan.  The is described in detail in [README-CONAN.md](README-CONAN.md)

As a summary, the procedure is:

```
c:\...\exiv2> mkdir build
c:\...\exiv2\build>conan install .. --build missing --profile msvc2019Release
c:\...\exiv2\build>cmake .. -DEXIV2_BUILD_UNIT_TESTS=On -G "Visual Studio 16 2019"
c:\...\exiv2\build>cmake --build . --config release
... lots of output from compiler and linker ...
c:\...\exiv2\build>
```

**Caution:** _You will need a DOS python3 interpreter which must be called python3.exe.  I copied the python.exe program:_

```
c:\...\exiv2\build>copy c:\Python37\python.exe c:\Python37\python3.exe
```

You must set the environment strings EXIV2\_BINDIR, EXIV2\_EXT and modify PATH.  You will need a DOS Python3 interpreter on your path, and you'll need the bash interpreter.  By careful to ensure the DOS python3.exe is found before the MingW/msys2 python3.

```
c:\...\exiv2\build> set EXIV2_BINDIR=%CD%
c:\...\exiv2\build> set EXIV2_EXT=.exe
c:\...\exiv2\build\bin> set "PATH=c:\Python37;c:\Python37\Scripts;c:\msys64\usr\bin;%PATH%"
```
Move to the test directory and use make (which is in c:\msys64\usr\bin) to drive the test procedures.  You cannot run the tests in the build directory because there is no Makefile in the build directory.

```
c:\...\exiv2\build>cd ..\test
c:\...\exiv2\test>make bash_tests
...
c:\...\exiv2\test>make python_tests   # or unit_test or version_test
...
c:\...\exiv2\test>make tests          # run all the tests
...
```

I use the following batch file _cmd64.bat_ to set up a special path for cmd.exe.  This ensures that I can jump instantly to the test directory with all the correct tools (DOS python, DOS cmake, msys/bash etc) on the PATH.

```
@echo off
setlocal
set "P="
set "P=%P%C:\Python37\;C:\Python37\Scripts;" # DOS Python3
set "P=%P%c:\Program Files\cmake\bin;"       # DOS cmake
set "P=%P%c:\msys64\usr\bin;"                # msys2 make, bash etc
set "P=%P%c:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin;"
set "P=%P%c:\Windows\System32;"              # windows
set "P=%P%%USERPROFILE%\com;"                # my home-made magic
echo %P%
set "PATH=%P%"
set "EXIV2_EXT=.exe"
set "EXIV2_BINDIR=%USERPROFILE%\gnu\github\exiv2\0.27-maintenance\build\bin"
color 0d
cmd /S /K cd "%EXIV2_BINDIR%\..\.."
color
endlocal
```

When you have the PATH constructed is this way, you can use the cmake command to run tests directly from the build directory as follows:

```
c:\...\exiv2\test>cd ..\build
c:\...\exiv2\build>cmake --build . --config Release --target tests
```


[TOC](#TOC)
<div id="4-3">

### 4.3 Unit tests

The code for the unit tests is in `<exiv2dir>/unitTests`.  To include unit tests in the build, use the *cmake* option `-DEXIV2_BUILD_UNIT_TESTS=ON`.

There is a discussion on the web about installing GTest: [https://github.com/Exiv2/exiv2/issues/575](https://github.com/Exiv2/exiv2/issues/575)

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

### 5.3 MinGW

Please note that the platform MinGW/msys2 32 is obsolete and superceded by MinGW/msys2 64.

There is a discussion on the web about installing GTest: [https://github.com/Exiv2/exiv2/issues/575](https://github.com/Exiv2/exiv2/issues/575)

#### MinGW/msys2 64 bit
Install: [http://repo.msys2.org/distrib/x86_64/msys2-x86\_64-20190524.exe](http://repo.msys2.org/distrib/x86_64/msys2-x86_64-20190524.exe)

I use the following batch file to start the MinGW/msys2 64 bit bash shell from the Dos Command Prompt (cmd.exe)

```bat
@echo off
setlocal
set "PS1=\! MSYS64:\u@\h:\w \$ "
set  PATH="/usr/local/bin/:/usr/bin:/mingw64/bin:/bin:/usr/sbin:/sbin"
set "HOME=c:\msys64\home\%USERNAME%"
if NOT EXIST %HOME% mkdir %HOME%
cd  %HOME%
c:\msys64\usr\bin\bash.exe -norc
endlocal
```

#### Install MinGW Dependencies

Install tools and dependencies:

```bash
$ for i in base-devel git cmake coreutils python3 man gcc gdb make dos2unix diffutils zlib-devel libexpat-devel libiconv-devel gettext-devel; do (echo y|pacman -S $i); done
```

#### Download exiv2 from github and build

```bash
$ mkdir -p ~/gnu/github/exiv2
$ cd       ~/gnu/github/exiv2
$ git clone https://github.com/exiv2/exiv2
$ cd exiv2
$ mkdir build ; cd build ;
$ cmake .. -G "Unix Makefiles"
$ make
```

#### MinGW and Regex

The exiv2 command line program provides an option **`--grep`** to filter output.  The implementation requires the header file **`<regex.h>`** and supporting library to be available during the build.  When not available, the option **`--grep`** degrades to a substring match.  Because there are several versions of **`<regex.h>`** available on the MinGW platform, detection of regex is always disabled on this platform and uses substring match.  The following command reveals if regex is included in your build:

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
make, cmake, gcc, gettext-devel pkg-config, dos2unix, zlib-devel, libexpat1-devel, git, python3-interpreter, libiconv, libxml2-utils, libncurses.

Download and build libiconv-1.15: [https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz](https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz)

There is a discussion on the web about installing GTest: [https://github.com/Exiv2/exiv2/issues/575](https://github.com/Exiv2/exiv2/issues/575)

Download and build cmake from source because I can't get the cygwin installed cmake 3.6.2 to work.
To build cmake from source, you need libncurses. [https://cmake.org/download/](https://cmake.org/download/)

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

### 5.5 Microsoft Visual C++

We recommend that you use Conan to build Exiv2 using Microsoft Visual C++. Exiv2 v0.27 can be built with Visual Studio versions 2008 and later.  We actively support and build with Visual Studio 2015, 2017 and 2019.

As well as Microsoft Visual Studio, you will need to install CMake, Python3, and Conan.

1) Binary installers for CMake on Windows are availably from [https://cmake.org/download/](https://cmake.org/download/).<br/>
2) Binary installers for Python3 are available from [python.org](https://python.org)<br/>
3) Conan can be installed using python/pip.  Details in [README-CONAN.md](README-CONAN.md)

I use the following batch file `cmd64.bat` to start cmd.exe.  I do this to reduce the complexity of the path which grows as various tools are installed on Windows.  As well as providing a "stripped down path", it also ensures the DOS python3 and DOS bash are on the path.

```bat
@echo off
setlocal
set "P="
set "P=%P%C:\Python37\;C:\Python37\Scripts;" # DOS Python3
set "P=%P%c:\Program Files\cmake\bin;"       # DOS cmake
set "P=%P%c:\msys64\usr\bin;"                # msys2 make, bash etc
set "P=%P%c:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin;"
set "P=%P%c:\Windows\System32;"              # windows
set "P=%P%%USERPROFILE%\com;"                # my home-made magic
echo %P%
set "PATH=%P%"
set "EXIV2_EXT=.exe"
set "EXIV2_BINDIR=%USERPROFILE%\gnu\github\exiv2\0.27-maintenance\build\bin"
color 0d
cmd /S /K cd "%EXIV2_BINDIR%\..\.."
color
endlocal
```

**Caution:** _The python3 interpreter must be for DOS and called python3.exe.  I copied the python.exe program:_

```
..>copy c:\Python37\python.exe c:\Python37\python3.exe
```


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

Written by Robin Mills<br>robin@clanmills.com<br>Updated: 2020-04-21
