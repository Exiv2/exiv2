|                           Codecov                            |                           OSS-Fuzz                           |                           Repology                           |                             Chat                             |
| :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/main/graph/badge.svg?token=O9G7Iswx26)](https://codecov.io/gh/Exiv2/exiv2) | [![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/exiv2.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:exiv2) | [![Packaging status](https://repology.org/badge/tiny-repos/exiv2.svg)](https://repology.org/metapackage/exiv2/versions) | [![#exiv2-chat on matrix.org](matrix-standard-vector-logo-xs.png)](https://matrix.to/#/#exiv2-chat:matrix.org) |



| **CI Status:**    |      |      |
|:--                |:--   |:--   |
| [![Basic jobs for all platforms](https://github.com/Exiv2/exiv2/actions/workflows/on_push_BasicWinLinMac.yml/badge.svg?branch=main)](https://github.com/Exiv2/exiv2/actions/workflows/on_push_BasicWinLinMac.yml) |  [![Nightly jobs for Linux distributions](https://github.com/Exiv2/exiv2/actions/workflows/nightly_Linux_distributions.yml/badge.svg?branch=main)](https://github.com/Exiv2/exiv2/actions/workflows/nightly_Linux_distributions.yml) | [![On PUSH - Linux Special Builds for main branch](https://github.com/Exiv2/exiv2/actions/workflows/on_push_ExtraJobsForMain.yml/badge.svg)](https://github.com/Exiv2/exiv2/actions/workflows/on_push_ExtraJobsForMain.yml) |

<div id="Welcome">

# Welcome to Exiv2

![Exiv2](exiv2.png)

Exiv2 is a C++ library and a command-line utility to read,
write, delete and modify Exif, IPTC, XMP and ICC image metadata.

| Exiv2 Resource | Location |
|:----------     |:------    |
| Releases and Documentation<br>Prereleases:<br>Project Resources<br>License (GPLv2)<br>CMake Downloads  | [https://exiv2.org](https://exiv2.org)<br>[https://pre-release.exiv2.org](https://pre-release.exiv2.org)<br>[https://github.com/Exiv2/exiv2](https://github.com/Exiv2/exiv2)<br>[COPYING](COPYING)<br>[https://cmake.org/download/](https://cmake.org/download/) |
| README.md<br>README-CONAN.md<br>README-SAMPLES.md | User Manual. _This document_<br>Conan User Manual _[click here](README-CONAN.md)_<br>Sample Code Manual. _[click here](README-SAMPLES.md)_ |

The file ReadMe.txt in a build bundle describes how to install the library on the platform.  ReadMe.txt also documents how to compile and link code on the platform.

<div id="TOC">

# TABLE  OF  CONTENTS

- [Welcome to Exiv2](#Welcome)
- [Building, Installing, Using and Uninstalling Exiv2](#B_I_U)
    - [Build, Install, Use and Uninstall Exiv2 on a UNIX-like system](#B_I_U_Unix)
    - [Build and Install Exiv2 with Visual Studio](#B_I_U_VisualStudio)
    - [Configure the project with CMake presets](#CMakePresets)
    - [Build Options](#BuildOptions)
    - [Dependencies](#Dependencies)
    - [Building and linking your code with Exiv2](#BuildAndLinkYourCode)
    - [Consuming Exiv2 with CMake](#ConsumeExiv2WithCmake)
    - [Using pkg-config to compile and link your code with Exiv2](#ConsumeWithPkgConfig)
    - [Localisation](#Localisation)
    - [Building Exiv2 Documentation](#BuildDoc)
    - [Building Exiv2 Tag Webpages](#BuildTagWebpages)
    - [Building Exiv2 Packages](#GeneratePackages)
    - [Debugging Exiv2](#Debugging)
    - [Building  Exiv2 with Clang and other build chains](#BuildWithClangAndOthers)
    - [Building  Exiv2 with ccache](#CCache)
    - [Thread Safety](#ThreadSafety)
    - [Library Initialisation and Cleanup](#InitAndCleanup)
    - [Cross Platform Build and Test on Linux for MinGW](#CrossPlatformSupport)
    - [Static and Shared Libraries](#StaticShared)
    - [Support for BMFF files (e.g., CR3, HEIF, HEIC, AVIF, and JPEG XL)](#BMFF)
- [License and Support](#LicenseSupport)
    - [License](#License)
    - [Support](#Support)
- [Test Suite](#TestSuite)
    - [Exiv2 Environment Variables](#EnvironmentVariables)
    - [Running tests on a UNIX-like system](#TestsOnUnix)
    - [Running tests on Visual Studio builds](#TestsOnVisualStudio)
    - [Unit Tests](#UnitTests)
    - [Bugfix Tests](#BugfixTests)
    - [Fuzzing](#FuzzingTests)
        - [OSS-Fuzz](#OssFuzz)
- [Platform Notes](#PlatformNotes)
    - [Linux](#PlatformLinux)
    - [macOS](#PlatformMacOs)
    - [MinGW/msys2](#PlatformMinGWMSYS)
    - [Cygwin](#PlatformCygwin)
    - [Visual Studio](#PlatformVisualStudio)
    - [Unix](#PlatformUnix)

[TOC](#TOC)
<div id="B_I_U">

# Building, Installing, Using and Uninstalling Exiv2

You need [CMake](https://cmake.org/download/) to configure the Exiv2 project, any C++ compiler implementing the C++ 17 standard and the associated tool chain.

<div id="B_I_U_Unix">

## Build, Install, Use Exiv2 on a UNIX-like system

```bash
$ cd ~/gnu/github/exiv2                     # Location of the project code
$ mkdir build && cd build                   # Create a build directory
$ cmake -DCMAKE_BUILD_TYPE=Release ..       # Configure the project with CMake
$ cmake --build .                           # Compile the project
$ ctest --verbose                           # Run tests
$ cmake --install .                         # Run the install target (install library, public headers, application and CMake files)
```

This will install the library into the "standard locations".  The library will be installed in `/usr/local/lib`, executables (including the exiv2 command-line program) in `/usr/local/bin/` and header files in `/usr/local/include/exiv2`. The target directory for the installation can be modified by using the CMake option `-DCMAKE_INSTALL_PREFIX`. 

CMake analyzes the project configuration from the source code directory and generates files into the build directory. It generates the project/solution/makefiles required to build the exiv2 library and command line application (and optionally sample applications and test runners). CMake also creates the files `exv_conf.h` and `exiv2lib_export.h` which contain compiler directives about the build options you have chosen and the availability of libraries on your machine.

### Using the exiv2 command-line program

To execute the exiv2 command line program, you should update your path to search /usr/local/bin/

```bash
$ export PATH="/usr/local/bin:$PATH"
```

You will also need to locate libexiv2 at run time:

```bash
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"      # Linux, Cygwin, MinGW/msys2
$ export DYLD_LIBRARY_PATH="/usr/local/lib:$DYLD_LIBRARY_PATH"  # macOS
```

### Uninstall

I don't know why anybody would uninstall Exiv2.

```bash
$ cd ~/gnu/github/exiv2  # location of the project code
$ cd build
$ cmake --build . --target uninstall
```

These commands will run the `uninstall` target and remove all the files which were installed by the `install` target.
Note that this mechanism is not perfect and it is not able to remove the sub-directories created in the installation
path.

[TOC](#TOC)
<div id="B_I_U_VisualStudio">

## Build and Install Exiv2 with Visual Studio

We recommend to use conan to download the Exiv2 external dependencies on Windows. On other platforms (macOS, Linux and others), traditionally the platform package managers have been used. However, conan can be used in any platform/architecture to bring the project dependencies. These are discussed at [Platform Notes](#PlatformNotes). The options to configure and compile the project using Visual Studio are similar to UNIX like systems.
See [README-CONAN](README-CONAN.md) for more information about Conan.

When you build, you may install with the following command.

```cmd
> cmake --install .
```
This will create and copy the exiv2 build artefacts to `%ProgramFiles%/exiv2`. To be able to run the `exiv2` command line application from any terminal you should modify your path to include `%ProgramFiles%/exiv2/bin`.

[TOC](#TOC)
<div id="CMakePresets">

## Configure the project with the CMake presets

CMake presets (see documentation [here](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)) were added recently to the project to ease the CMake configuration process for typical configurations. The presets are defined in the file `CMakePresets.json` and they can be used from the terminal or interpreted by different IDEs. Please note that one needs to use a recent version of CMake (>= 3.21) supporting the presets feature.

One can list the available presets using the `--list-presets` option:

```bash
# Running the command from a Windows terminal
$ cmake --list-presets
Available configure presets:

  "msvc"        - Visual Studio cl toolchain (also usable from VS Code)
  "win-debug"   - Windows Debug with configured architecture
  "win-release" - Windows Release with configured architecture

# Running the command from a Linux terminal
$ cmake --list-presets
Available configure presets:

  "linux-debug"   - Linux Debug with default architecture
  "linux-release" - Linux Release with default architecture
```

The project configuration with a specific preset can be chosen with the CMake `--preset` option. In the following terminal output we comment out some interesting things happening during the project configuration:

```bash
# Configuring the project using a preset
$ cmake --preset win-release
Preset CMake variables:

# Note that with the usage of a preset, we pass many different options to CMake.
  BUILD_SHARED_LIBS:BOOL="TRUE"
  CMAKE_BUILD_TYPE="Release"
  CMAKE_INSTALL_PREFIX:PATH="C:/dev/personal/exiv2/build-win-release/install"
  # A build & install directory are configured with the preset
  CONAN_AUTO_INSTALL:BOOL="TRUE"
  EXIV2_BUILD_SAMPLES:BOOL="TRUE"
  EXIV2_BUILD_UNIT_TESTS:BOOL="TRUE"
  EXIV2_ENABLE_BMFF:BOOL="TRUE"
  EXIV2_ENABLE_CURL:BOOL="TRUE"
  EXIV2_ENABLE_NLS:BOOL="FALSE"
  EXIV2_ENABLE_VIDEO:BOOL="TRUE"
  EXIV2_ENABLE_PNG:BOOL="TRUE"
  EXIV2_ENABLE_WEBREADY:BOOL="TRUE"
  EXIV2_TEAM_WARNINGS_AS_ERRORS:BOOL="TRUE"

# Conan can be automatically detected in your system and it is run automatically to bring the
# project dependencies
-- Conan: Detected VS runtime: MD
-- Conan: checking conan executable
-- Conan: Found program C:/dev/envs/conan/Scripts/conan.exe
-- Conan: Version found Conan version 1.47.0
-- Conan executing: C:/dev/envs/conan/Scripts/conan.exe install .. --remote conancenter --build missing --options webready=True --settings arch=x86_64 --settings build_type=Release --settings compiler=Visual Studio --settings compiler.version=17 --settings compiler.runtime=MD
...

# CMake finds the project dependencies which were automatically handled by conan
-- Conan: Using autogenerated FindZLIB.cmake
-- Library zlib found C:/Users/luis/.conan/data/zlib/1.2.11/_/_/package/
-- Conan: Using autogenerated FindCURL.cmake
-- Library libcurl_imp found C:/Users/luis/.conan/data/libcurl/7.79.0/_/_/package/
...

# CMake finish the project configuration and prints a report
-- Install prefix:    C:/dev/personal/exiv2/build-win-release/install
-- ------------------------------------------------------------------
-- CMake Generator:   Ninja
-- CMAKE_BUILD_TYPE:  Release
-- Compiler info: MSVC (C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC/14.30.30705/bin/Hostx64/x64/cl.exe) ; version: 19.30.30705.0
-- CMAKE_CXX_STANDARD:17
--  --- Compiler flags ---
-- General:           /DWIN32 /D_WINDOWS /W3 /GR /EHsc
         /MP
         /utf-8
         /WX
-- Extra:
-- Debug:             /MDd /Zi /Ob0 /Ox /Zo
-- Release:           /MD /O2  /DNDEBUG
-- RelWithDebInfo:    /MD /Zi /O2  /DNDEBUG
-- MinSizeRel:        /MD /O1  /DNDEBUG
--  --- Linker flags ---
-- General:           /machine:x64 /WX
-- Debug:             /debug /INCREMENTAL
-- Release:           /INCREMENTAL:NO
-- RelWithDebInfo:    /debug /INCREMENTAL
-- MinSizeRel:        /INCREMENTAL:NO
--
...
-- Build files have been written to: C:/dev/personal/exiv2/build-win-release
```

Note that the usage of CMake presets allow the project contributors to use the same set of options easily in different environments (using terminal, IDEs or CI).

[TOC](#TOC)
<div id="BuildOptions">

## Build options

There are two groups of CMake options which are relevant to the project: global CMake options and project specific ones. Here are some of the global options which are particularly useful:

| Options       | Purpose (_default_)       |
|:------------- |:------------- |
| CMAKE\_INSTALL\_PREFIX<br/>CMAKE\_BUILD\_TYPE<br/>BUILD\_SHARED\_LIBS | Where to install on your computer _**(/usr/local)**_<br/>Type of build _**(Release)**_ See: [Debugging Exiv2](#Debugging) <br/>Build exiv2lib as SHARED or STATIC |

Options defined at `exiv2/CMakeLists.txt` include:

```bash
576 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $ grep ^option CMakeLists.txt
option( BUILD_SHARED_LIBS          "Build exiv2lib as a shared library"              ON  )
option( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                 ON  )
option( EXIV2_ENABLE_EXTERNAL_XMP  "Use external version of XMP"                     OFF )
option( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"          ON  )
...
option( EXIV2_ENABLE_BMFF          "Build with BMFF support (brotli recommended)"    ON  )
option( EXIV2_ENABLE_BROTLI        "Use Brotli for JPEG XL compressed boxes (BMFF)"  ON  )
577 rmills@rmillsmm:~/gnu/github/exiv2/exiv2 $
```

Using the command-line, these variables can be set/updated using the option `-D`:

```bash
$ cmake -DBUILD_SHARED_LIBS=ON -DEXIV2_ENABLE_NLS=OFF
```

[TOC](#TOC)
<div id="Dependencies">

## Dependencies

The following Exiv2 features require external libraries:

| Feature                  | Package  | Default | To change default           | Availability |
|:------------------------ |:-------- |:-------:|:--------------------------- |:------------ |
| PNG image support        | zlib     | ON      | -DEXIV2\_ENABLE\_PNG=OFF    | [http://zlib.net/](http://zlib.net/) |
| XMP support              | expat    | ON      | -DEXIV2\_ENABLE\_XMP=OFF    | [http://expat.sourceforge.net](http://expat.sourceforge.net)/<br/>Use _**Expat 2.2.6**_ and later |
| Natural language system  | gettext  | OFF     | -DEXIV2\_ENABLE\_NLS=ON     | [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) |
| JPEG XL brob support     | brotli   | ON      | -DEXIV2\_ENABLE\_BROTLI=OFF | [https://github.com/google/brotli](https://github.com/google/brotli) |
| Character set conversion | libiconv |         | Disabled for Visual Studio.<br>Linked when installed on UNIX like platforms. | [https://www.gnu.org/software/libiconv/](https://www.gnu.org/software/libiconv/) |

On UNIX systems, you may install the dependencies using the distribution's package management system. Install the
development package of a dependency to install the header files and libraries required to build Exiv2. The script
`ci/install_dependencies.sh` is used to setup the CI images on which we build and test Exiv2.  You may find that helpful in setting up your platform dependencies.

Natural language system is discussed in more detail here: [Localisation](#Localisation)

Notes about different platforms are included here: [Platform Notes](#PlatformNotes)

You may choose to install dependences with conan.  This is supported on all platforms and is especially useful for users of Visual Studio.
See [README-CONAN](README-CONAN.md) for more information.

### Libiconv

The library libiconv is used to perform character set encoding in the tags Exif.Photo.UserComment, Exif.GPSInfo.GPSProcessingMethod and Exif.GPSInfo.GPSAreaInformation.  This is documented in the [exiv2 man page](exiv2.md).

CMake will detect libiconv of all UNIX like systems including Linux, macOS, UNIX, Cygwin64 and MinGW/msys2.  If you have installed libiconv on your machine, Exiv2 will link and use it.

The library libiconv is a GNU library and we do not recommend using libiconv with Exiv2 when building with Visual Studio.

Exiv2 includes the file cmake/FindIconv.cmake which contains a guard to prevent CMake from finding libiconv when you build with Visual Studio.  This was added because of issues reported when Visual Studio attempted to link libiconv libraries installed by Cygwin, or MinGW or gnuwin32. [https://github.com/Exiv2/exiv2/issues/1250](https://github.com/Exiv2/exiv2/issues/1250)

There are build instructions about Visual Studio in libiconv-1.16/INSTALL.window require you to install Cygwin.  There is an article here about building libiconv with Visual Studio. [https://www.codeproject.com/Articles/302012/How-to-Build-libiconv-with-Microsoft-Visual-Studio](https://www.codeproject.com/Articles/302012/How-to-Build-libiconv-with-Microsoft-Visual-Studio).  

If you wish to use libiconv with Visual Studio you will have to build libiconv and remove the "guard" in cmake/FindIconv.cmake.  Team Exiv2 will not provide support concerning libiconv and Visual Studio.

[TOC](#TOC)
<div id="BuildAndLinkYourCode">

## Building and linking your code with Exiv2

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
$ g++ -std=c++17 myprog.cpp -o myprog -I/usr/local/include -L/usr/local/lib -lexiv2
```

[TOC](#TOC)
<div id="ConsumeExiv2WithCmake">

## Consuming Exiv2 with CMake

When exiv2 is installed, the files required to consume Exiv2 with CMake are installed in `${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2`

You can build samples/exifprint.cpp as follows:

```bash
$ cd <exiv2dir>
$ mkdir exifprint
$ cd    exifprint
$ cat - > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.11)
project(exifprint VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(exiv2 REQUIRED CONFIG NAMES exiv2)    # search ${CMAKE_INSTALL_PREFIX}/lib/cmake/exiv2/
add_executable(exifprint ../samples/exifprint.cpp) # Create exifprint target
target_link_libraries(exifprint PRIVATE Exiv2::exiv2lib)  # link exiv2lib
EOF
$ cmake .                                          # generate the makefile
$ cmake --build .                                  # build the code
$ ./exifprint                                      # test your executable
Usage: bin/exifprint [ path | --version | --version-test ]
$
```

[TOC](#TOC)
<div id="ConsumeWithPkgConfig">

## Using pkg-config to compile and link your code with Exiv2

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
g++ -std=c++17 myprogram.cpp -o myprogram $(pkg-config exiv2 --libs --cflags)
```

[TOC](#TOC)
<div id="Localisation">

## Localisation

Localisation is supported on a UNIX-like platform:  Linux, macOS, Cygwin and MinGW/msys2.  Localisation is not supported for Visual Studio builds.

Crowdin have provided Exiv2 with a free open-source license to use their services.  The Exiv2 localisation project is located at [https://crowdin.com/project/exiv2](https://crowdin.com/project/exiv2).  You will also need to register to have a free user account on Crowdin.  The Crowdin setup is discussed here: [https://github.com/Exiv2/exiv2/issues/1510](https://github.com/Exiv2/exiv2/issues/1510).  It is recommended that you coordinate with Leonardo before contributing localisation changes on Crowdin.  You can contact Leonardo by via GitHub.

To build localisation support, use the CMake option `-DEXIV2_ENABLE_NLS=ON`.  You must install the `gettext` package with your package manager or from source.  The `gettext` package is available from [http://www.gnu.org/software/gettext/](http://www.gnu.org/software/gettext/) and includes the library `libintl` and utilities to build localisation files.  If CMake produces error messages which mention libintl or gettext, you should verify that the package `gettext` has been correctly built and installed.

You must install the build to test localisation.  This ensures that the localisation message files can be found at run-time.  You cannot test localisation in the directory `build\bin`.

1) Running exiv2 in another language

```bash
$ env LANG=fr_FR exiv2    # env LANGUAGE=fr_FR exiv2 on Linux!
exiv2: Une action doit être spécifié
exiv2: Au moins un fichier est nécessaire
Utilisation : exiv2 [ option [ arg ] ]+ [ action ] fichier ...

Image metadata manipulation tool.
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
msgid "Image metadata manipulation tool.\n"
msgstr ""
```
to:

```bash
#: src/exiv2.cpp:237
msgid "Image metadata manipulation tool.\n"
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
Usage: exiv2 [ option [ arg ] ]+ [ action ] file ...

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
<div id="BuildDoc">

## Building Exiv2 Documentation

Building documentation requires installing special tools.  You will probably prefer to
read the documentation on-line from the project website: https://exiv2.org

To build documentation, use the CMake option **`-DEXIV2_BUILD_DOC=ON`**.
Additionally, you will require an additional build step to actually build the documentation.

```bash
$ cmake ..options.. -DEXIV2_BUILD_DOC=ON
$ cmake --build . --target doc
```

To build the documentation, you must install the following products:

| Product      | Availability |
|:------------ |:------------ |
| doxygen<br/>graphviz<br/>python<br/>xsltproc<br/>md5sum  | [http://www.doxygen.org/](http://www.doxygen.org/)<br/>[http://www.graphviz.org/](http://www.graphviz.org/)<br/>[http://www.python.org/](http://www.python.org/)<br/>[http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/)<br/>[http://www.microbrew.org/tools/md5sha1sum/](http://www.microbrew.org/tools/md5sha1sum/) |

[TOC](#TOC)
<div id="BuildTagWebpages">

## Building Exiv2 Tag Webpages

Exiv2 provides many built-in metadata tags which are listed in the sub-pages of https://exiv2.org/metadata.html 
and https://pre-release.exiv2.org/metadata.html. Those tag webpages are generated using tag information 
extracted from the Exiv2 source code.

The tag webpage build files are in the `<exiv2dir>/doc/templates` directory. If changes are made to 
tag groups in the Exiv2 source code then the build files need to be updated. Any changes made 
to individual tags in an existing tag group are automatically included.

Building the tag webpages requires building the Exiv2 sample programs and using scripts which have additional dependencies on 
[BASH](https://www.gnu.org/software/bash/), [make](https://manpages.org/make), [xsltproc](https://manpages.org/xsltproc) 
and [Python3](https://www.python.org/).

To build the tag webpages, first [build Exiv2 from source](#TOC) with the `-DEXIV2_BUILD_SAMPLES=ON` 
option enabled. This is required as the [taglist](README-SAMPLES.md#taglist) sample program is used by one of the scripts.

Next, set the `EXIV2_BINDIR` environment variable (see [Exiv2 environment variables](#EnvironmentVariables)).

Then, change directory to `doc/templates` and run `make`.

```bash
$ cd <exiv2dir>/doc/templates
$ make
```

After processing, the generated webpages are stored in the `<exiv2dir>/doc/templates` directory. 
When the Exiv2 websites are updated, the generated tag webpages are reformatted before use.

[TOC](#TOC)
<div id="GeneratePackages">

## Building Exiv2 Packages

To enable the building of Exiv2 packages, use the CMake option `-DEXIV2_TEAM_PACKAGING=ON`.

You should not build Exiv2 Packages.  This feature is intended for use by Team Exiv2 to create Platform and Source Packages on the buildserver.

There are two types of Exiv2 packages which are generated by cpack from the CMake command-line.

1) Platform Package (header files, binary library and samples.  Some documentation and release notes)

Create and build exiv2 for your platform.

```bash
$ git clone https://github.com/exiv2/exiv2
$ mkdir -p exiv2/build
$ cd       exiv2/build
$ cmake .. -G "Unix Makefiles" -DEXIV2_TEAM_PACKAGING=ON
...
-- Build files have been written to: .../build
$ cmake --build . --config Release
...
[100%] Built target addmoddel
$ cmake --build . --target package
...
CPack: - package: /path/to/exiv2/build/exiv2-0.27.1-Linux.tar.gz generated.
```

2) Source Package

```bash
$ cmake --build . --target package_source
Run CPack packaging tool for source...
...
CPack: - package: /path/to/exiv2/build/exiv2-0.27.1-Source.tar.gz generated.
```

[TOC](#TOC)
<div id="Debugging">

## Debugging Exiv2

1) Generating and installing a debug library

In general to generate a debug library, you should use the *CMake* option `-DCMAKE_RELEASE_TYPE=Debug` and build in the usual way.

```bash
$ cd <exiv2dir>
$ mkdir build
$ cd build
$ cmake .. -G "Unix Makefiles" "-DCMAKE_BUILD_TYPE=Debug"
$ cmake --build .

```

You must install the library to ensure that your code is linked to the debug library.

You can check that you have generated a debug build with the command:

```bash
$ exiv2 -vVg debug
exiv2 0.27.1
debug=1
$
```
2) About preprocessor symbols `NDEBUG` and `EXIV2_DEBUG_MESSAGES`

Exiv2 respects the symbol `NDEBUG` which is set only for Release builds. There are sequences of code which are defined within:

```cpp
#ifdef EXIV2_DEBUG_MESSAGES
....
#endif
```

Those blocks of code are not compiled unless you define `EXIV2_DEBUG_MESSAGES`. They are provided for additional debugging information. For example, if you are interested in additional output from webpimage.cpp, you can update your build as follows:

```bash
$ cd <exiv2dir> && cd build
$ cmake -DCMAKE_CXX_FLAGS=-DEXIV2_DEBUG_MESSAGES ..
$ cmake --build .
$ bin/exiv2 ...
-- or --
$ cmake --install .
$ exiv2     ...
```

If you are debugging library code, it is recommended that you use the exiv2 command-line program as your test harness as Team Exiv2 is very familiar with this tool and able to give support.

3) Starting the debugger

This is platform specific.  On Linux:

```bash
$ gdb exiv2
```

4) Using Debugger IDEs such as Xcode, CLion, Visual Studio, Eclipse or QtCreator

I have used all those IDEs to debug the Exiv2 library and applications.  All of them work.  You may find it takes initial effort, however I assure you that they all work well.

I work on macOS and use Xcode to develop Exiv2.  For a couple of years, Team Exiv2 had free
open-source licences from JetBrains for CLion.  I really liked CLion as it is cross platform
and runs on Windows, Mac and Linux.  It has excellent integration with CMake and will automatically
add **`-DCMAKE_BUILD_TYPE=Debug`** to the CMake command.  It keeps build types in separate directories
such as **`<exiv2dir>/cmake-build-debug`**.

5) cmake --build . options **`--config Release|Debug`** and **`--target install`**

Visual Studio and Xcode can build debug or release builds without using the option **`-DCMAKE_BUILD_TYPE`** because the generated project files can build multiple types.  The option **`--config Debug`** can be specified on the CMake command-line to specify the build type.  Alternatively, if you prefer to build in the IDE, the UI provides options to select the configuration and target.

With the Unix Makefile generator, the targets can be listed:

```bash
$ cmake --build . --target help
The following are some of the valid targets for this Makefile:
... all (the default if no target is provided)
... clean
... depend
... install/local
.........
```

[TOC](#TOC)
<div id="BuildWithClangAndOthers">

## Building Exiv2 with **clang** and other build chains

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
<div id="CCache">

## Building Exiv2 with ccache

To speed up compilation, the utility ccache can be installed to cache the output of the compiler.  This greatly speeds up the build when you frequently built code that has not been modified.

Installing and using ccache (and other similar utilities), is platform dependent.  On Ubuntu:

```bash
$ sudo apt install --yes ccache
```

To build with ccache, use the CMake option **-DBUILD\_WITH\_CCACHE=ON**

```bash
$ cd <exiv2dir>
$ mkdir build ; cd build ; cd build
$ cmake .. -G "Unix Makefiles" -DBUILD_WITH_CCACHE=ON
$ cmake --build .
# Build again to appreciate the performance gain
$ cmake --build . --target clean
$ cmake --build .
```

Due to the way in which ccache is installed in Fedora (and other Linux distros), ccache effectively replaces the compiler.  A default build or **-DBUILD\_WITH\_CCACHE=OFF** is not effective and the environment variable CCACHE_DISABLE is required to disable ccache. [https://github.com/Exiv2/exiv2/issues/361](https://github.com/Exiv2/exiv2/issues/361)

[TOC](#TOC)
<div id="ThreadSafety">

## Thread Safety

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
The use of the _**thread unsafe function**_ Exiv2::enableBMFF(true) is discussed in [Support for BMFF files (e.g., CR3, HEIF, HEIC, AVIF, and JPEG XL)](#BMFF)

[TOC](#TOC)
<div id="InitAndCleanup">

## Library Initialisation and Cleanup

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
<div id="CrossPlatformSupport">

## Cross Platform Build and Test on Linux for MinGW

You can cross compile Exiv2 on Linux for MinGW.  We have used the following method on **Fedora** and believe this is also possible on Ubuntu and other distros.  Detailed instructions are provided here for **Fedora**.

### Cross Build and Test On Fedora

#### 1 Install the cross platform build tools

```bash
$ sudo dnf install mingw64-gcc-c++ mingw64-filesystem mingw64-expat mingw64-zlib cmake make
```

#### 2 Install Dependencies

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

#### 3 Get the code and build

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
                   -DEXIV2_ENABLE_WEBREADY=ON     \
                   -DBUILD_SHARED_LIBS=OFF
```
The options available for cross-compiling are the same as provided for all builds.  See: [Build Options](#BuildOptions)


#### 4 Copy "system dlls" in the bin directory

These DLLs are required to execute the cross-platform build in the bin from Windows

```bash
for i in libexpat-1.dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll zlib1.dll ; do
    cp -v /usr/x86_64-w64-mingw32/sys-root/mingw/bin/$i bin
done
```

#### 5 Executing exiv2 in wine

You may wish to use wine to execute exiv2 from the command prompt.  To do this:

```bash
[rmills@rmillsmm-fedora build_mingw_fedora]$ wine cmd
Microsoft Windows 6.1.7601

Z:\Home\gnu\github\exiv2\main\build_mingw_fedora>bin\exiv2
exiv2: An action must be specified
exiv2: At least one file is required
Usage: exiv2 [ option [ arg ] ]+ [ action ] file ...

Image metadata manipulation tool.
```

If you have not installed wine, Fedora will offer to install it for you.

#### 6 Running the test suite

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
$ mingw64-ctest
```

You will find that 3 tests fail at the end of the test suite.  It is safe to ignore those minor exceptions.

[TOC](#TOC)
<div id="StaticShared">

## Static and Shared Libraries

You can build either static or shared libraries.  Both can be linked with either static or shared run-time libraries.  You specify the shared/static with the option `-BUILD_SHARED_LIBS=ON|OFF` You specify the run-time with the option `-DEXIV2_ENABLE_DYNAMIC_RUNTIME=ON|OFF`.  The default for both options default is ON.  So you build shared and use the shared libraries which are `.dll` on Windows (msvc, Cygwin and MinGW/msys), `.dylib` on macOS and `.so` on Linux and UNIX.  

CMake creates your build artefacts in the directories `bin` and `lib`.  The `bin` directory contains your executables and .DLLs.  The `lib` directory contains your static libraries.  When you install exiv2, the build artefacts are copied to your system's prefix directory which by default is `/usr/local/`.  If you wish to test and use your build without installing, you will have to set you PATH appropriately.  Linux/Unix users should also set `LD_LIBRARY_PATH` and macOS users should set `DYLD_LIBRARY_PATH`.

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
<div id="BMFF">

## Support for BMFF files (e.g., CR3, HEIF, HEIC, AVIF, and JPEG XL)

**Attention is drawn to the possibility that BMFF support may be the subject of patent rights. _Exiv2 shall not be held responsible for identifying any or all such patent rights.  Exiv2 shall not be held responsible for the legal consequences of the use of this code_.**

Access to the BMFF code is guarded in two ways.  Firstly, you have to build the library with the CMake option: `-DEXIV2_ENABLE_BMFF=ON`.  Secondly, the application must enable BMFF support at run-time by calling the following function.

```cpp
EXIV2API bool enableBMFF(bool enable);
```

The return value from `enableBMFF()` is true if the library has been build with BMFF support (CMake option -DEXIV2_ENABLE_BMFF=ON).

Applications may wish to provide a preference setting to enable BMFF support and thereby place the responsibility for the use of this code with the user of the application.

[TOC](#TOC)
<div id="LicenseSupport">

# License and Support

All project resources are accessible from the project website.
    https://github.com/Exiv2/exiv2

<div id="License">

## License

Copyright (C) 2004-2023 Exiv2 authors.
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
<div id="Support">

## Support

For new bug reports, feature requests and support:  Please open an issue in Github.  [https://github.com/exiv2/exiv2](https://github.com/exiv2/exiv2)

[TOC](#TOC)
<div id="TestSuite">

# Test Suite

You execute the Test Suite using CTest with the command `$ ctest`.

The build creates 6 tests: bashTests, bugfixTests, lensTests, tiffTests, unitTests and versionTests.  You can run all tests or a subset. To list all available tests, execute ctest with the `-N` or `--show-only` option, which disables execution:

```bash
.../main/build $ ctest -N
Test project ...main/build
  Test #1: bashTests
  Test #2: bugfixTests
  Test #3: lensTests
  Test #4: tiffTests
  Test #5: versionTests
  Test #6: unitTests

Total Tests: 6
.../main/build $
```

ctest provides many option and the following show common use-case scenarios:

```bash
$ ctest                              # run all tests and display summary
$ ctest --output-on-failure          # run all tests and output failures
$ ctest -R bugfix                    # run only bugfixTests and display summary
$ ctest -R bugfix --verbose          # run only bugfixTests and display all output
```

Except for the `unitTests`,  CMake needs to find a python3 interpreter in the system to be able to run the rest of the test targets with CTest:

| Name               | Language  | Location    | Command<br>_(in build directory)_ | CMake Option to Build          |
|:--                 |:--        |:--                      |:--                    |:--                             |
| bashTests          | python    | tests/bash_tests       | $ ctest -R bash        | -DEXIV2_BUILD_SAMPLES=ON       |
| bugfixTests        | python    | tests/bugfixes          | $ ctest -R bugfix     | -DEXIV2_ENBALE_VIDEO=ON        |
| lensTest           | C++       | tests/lens_tests        | $ ctest -R lens       |                                |
| tiffTests          | python    | tests/tiff_test         | $ ctest -R tiff       |                                |
| unitTests          | C++       | unitTests/              | $ ctest -R unit       | -DEXIV2_BUILD_UNIT_TESTS=ON    |
| versionTests       | C++       | src/version.cpp         | $ ctest -R version    | Always in library              |

The term _**bashTests**_ is historical.  These tests were originally bash scripts and have been rewritten in python.
Visual Studio Users will appreciate the python implementation as it avoids the installation of mingw/cygwin and special PATH settings.

If you build the code in the directory `<exiv2dir>/build`, tests will run using the default values of Environment Variables.

[TOC](#TOC)
<div id="EnvironmentVariables">

## Exiv2 Environment Variables

Exiv2 optionally uses several different environment variables when building or testing.

| Variable           | Default                    | Platforms          | Purpose |
|:--                 |:--                         |:--                 |:--      |
| EXIV2_BINDIR       | **\<exiv2dir\>/build/bin** | All Platforms      | Path of built binaries (e.g., exiv2.exe) |
| EXIV2_PORT         | **12762**<br>**12671**<br>**12760**             | Cygwin<br>MinGW/msys2<br>Other Platforms | Test TCP/IP Port   |
| EXIV2_HTTP         | **http://localhost**       | All Platforms      | Test http server   |
| EXIV2_ECHO         | _**not set**_              | All Platforms      | For debugging bashTests |
| VALGRIND           | _**not set**_              | All Platforms      | For debugging bashTests |
| VERBOSE            | _**not set**_              | Makefile platforms | Instructs make to report its actions |
| PATH<br>DYLD\_LIBRARY\_PATH<br>LD\_LIBRARY\_PATH    | $EXIV2\_BINDIR/../lib | Windows<br>macOS<br>Other platforms | Path of dynamic libraries |

The Variable EXIV2\_PORT or EXIV2\_HTTP can be set to None to skip http tests.  The http server is started with the command `python3 -m http.server $port`.  On Windows, you will need to run this manually _**once**_ to authorise the firewall to permit python to use the port.

[TOC](#TOC)
<div id="TestsOnUnix">

## Running tests on Unix-like systems

You can run tests directly from the build:

```bash
$ cmake .. -G "Unix Makefiles" -DEXIV2_BUILD_UNIT_TESTS=ON 
... lots of output and build summary ...
$ cmake --build .
... lots of output ...
$ ctest
... test summary ...
$
```

You can run individual tests in the `test` directory.  **Caution:** If you build in a directory other than \<exiv2dir\>/build, you must set EXIV2\_BINDIR to run tests from the `test` directory.

```bash
$ cd <exiv2dir>/build
$ ctest -R bash --verbose
addmoddel_test (testcases.TestCases) ... ok
....
Ran 176 tests in 9.526s
OK (skipped=6)

$ ctest -R bugfix --verbose
... lots of output ...
test_run (tiff_test.test_tiff_test_program.TestTiffTestProg) ... ok
----------------------------------------------------------------------
Ran 176 tests in 9.526s
OK (skipped=6)
$
```

[TOC](#TOC)
<div id="TestsOnVisualStudio">

## Running tests on Visual Studio builds from cmd.exe

**Caution:** _The python3 interpreter must be on the PATH, build for DOS, and called python3.exe._  I copied the python.exe program:

```cmd
> copy c:\Python37\python.exe c:\Python37\python3.exe
> set PATH=c:\Python37;%PATH%
```

You can execute the test suite in a similar manner to that described for UNIX-like systems.  You _**must**_ provide the `-C` config option to ctest for Visual Studio builds.  

```cmd
> cd <exiv2dir>/build
> ctest -C Release
> ctest -C Release -R bugfix --verbose
```
Visual Studio can build different configs as follows:

```cmd
> cmake --build . --config Release        # or Debug or MinSizeRel or RelWithDebInfo
> ctest -C Release
```
The default for **CMake** config option `--config` is `Release`.  **ctest** does not have a default for config option `-C`.

### Running tests from cmd.exe

You can build with Visual Studio using Conan.  The is described in detail in [README-CONAN.md](README-CONAN.md)

As a summary, the procedure is:

```
c:\...\exiv2>mkdir build
c:\...\exiv2>cd build
c:\...\exiv2\build>conan install .. --build missing --profile msvc2019Release
c:\...\exiv2\build>cmake .. -DEXIV2_BUILD_UNIT_TESTS=ON -G "Visual Studio 16 2019"
c:\...\exiv2\build>cmake --build . --config Release
... lots of output from compiler and linker ...
c:\...\exiv2\build>ctest -C Release
```

If you wish to use an environment variables, use set:

```
set EXIV2_PORT=54321
ctest -C Release --verbose -R bash
set EXIV2_PORT=
```

[TOC](#TOC)
<div id="UnitTests">

## Unit Tests

The code for the unit tests is in `<exiv2dir>/unitTests`.  To include unit tests in the build, use the *CMake* option `-DEXIV2_BUILD_UNIT_TESTS=ON`.

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
<div id="BugfixTests">

## Bugfix Tests

You can run the bugfix tests from the build directory:

```bash
$ cd <exiv2dir>/build
$ ctest -R bugfix  
```

If you wish to run in verbose mode:

```bash
$ cd <exiv2dir>/build
$ ctest -R bugfix --verbose
```

The bugfix tests are stored in directory tests/ and you can run them all with the command:

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
$ ctest -R bugfix --verbose 2>&1 | grep FAIL
```

[TOC](#TOC)
<div id="FuzzingTests">

## Fuzzing

The code for the fuzzers is in `exiv2dir/fuzz`

To build the fuzzers, use the *cmake* option `-DEXIV2_BUILD_FUZZ_TESTS=ON` and `-DEXIV2_TEAM_USE_SANITIZERS=ON`.
Note that it only works with clang compiler as libFuzzer is integrated with clang > 6.0

To build the fuzzers:

```bash
$ cd <exiv2dir>
$ rm -rf build-fuzz ; mkdir build-fuzz ; cd build-fuzz
$ cmake .. -DCMAKE_CXX_COMPILER=$(which clang++) -DEXIV2_BUILD_FUZZ_TESTS=ON -DEXIV2_TEAM_USE_SANITIZERS=ON
$ cmake --build .
```

To execute a fuzzer:

```bash
cd <exiv2dir>/build-fuzz
mkdir corpus
./bin/fuzz-read-print-write corpus ../test/data/ -jobs=$(nproc) -workers=$(nproc) -max_len=4096
```

For more information about fuzzing see [`fuzz/README.md`](fuzz/README.md).

[TOC](#TOC)
<div id="OssFuzz">

### OSS-Fuzz

Exiv2 is enrolled in [OSS-Fuzz](https://google.github.io/oss-fuzz/), which is a fuzzing service for open-source projects, run by Google.

The build script used by OSS-Fuzz to build Exiv2 can be found [here](https://github.com/google/oss-fuzz/tree/master/projects/exiv2/build.sh). It uses the same fuzz target ([`fuzz-read-print-write`](fuzz/fuzz-read-print-write.cpp)) as mentioned above, but with a slightly different build configuration to integrate with OSS-Fuzz. In particular, it uses the CMake option `-DEXIV2_TEAM_OSS_FUZZ=ON`, which builds the fuzz target without adding the `-fsanitize=fuzzer` flag, so that OSS-Fuzz can control the sanitizer flags itself.

[TOC](#TOC)
<div id="PlatformNotes">

# Platform Notes

There are many ways to set up and configure your platform.  The following notes are provided as a guide.

<div id="PlatformLinux">

## Linux

Update your system and install the build tools and dependencies (zlib, expat, gtest and others)

```bash
$ sudo apt --yes update
$ sudo apt install --yes build-essential git clang ccache python3 libxml2-utils cmake python3 libexpat1-dev libz-dev zlib1g-dev libbrotli-dev libssh-dev libcurl4-openssl-dev libgtest-dev google-mock libinih-dev
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
<div id="PlatformMacOs">

## macOS

You will need to install Xcode and the Xcode command-line tools to build on macOS.

You should build and install libexpat and zlib.  You may use brew, macports, build from source, or use conan.

I recommend that you build and install CMake from source.

[TOC](#TOC)
<div id="PlatformMinGWMSYS">

## MinGW/msys2

Please note that the 32bit MinGW platform is obsolete and superceded by the 64bit MSYS2 distribution. It is important to highlight that we rely on using the Universal C Runtime (UCRT) and its relatively new support for UTF-8. Check this [PR](https://github.com/Exiv2/exiv2/pull/2090) for more information. Therefore you will need to use the [MSYS2 URCT64 environment](https://www.msys2.org/docs/environments/).

Install the latest version of [MSYS2](https://repo.msys2.org/distrib/msys2-x86_64-latest.exe), and follow the installation instructions available [here](https://www.msys2.org/).

The CI workflow file `.github/workflows/on_PR_windows_matrix.yml` has a build job named `msys2` with instructions showing how to configure Exiv2 on MSYS2.

### Install exiv2 Dependencies

Please note that you will need to install the `ucrt-x86_64` package version of the exiv2 dependencies:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-{brotli,cc,cmake,curl,expat,gettext,gtest,libiconv,libwinpthread,ninja,zlib}
```

### Download exiv2 from github and build

Use the Windows start menu to open the terminal customized for the UCRT64 environment: `MSYS2 MinGW UCRT x64`. Then run the following commands to download exiv2, configure the project and build it:

```bash
mkdir -p ~/gnu/github/exiv2
cd       ~/gnu/github/exiv2
git clone https://github.com/exiv2/exiv2
cd exiv2
mkdir build && cd build
cmake -G Ninja
      -DCMAKE_CXX_FLAGS=-Wno-deprecated
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_SHARED_LIBS=ON
      -DEXIV2_BUILD_SAMPLES=ON
      -DEXIV2_ENABLE_NLS=OFF
      -DEXIV2_ENABLE_WEBREADY=ON
      -DEXIV2_ENABLE_BMFF=ON
      -DEXIV2_BUILD_UNIT_TESTS=ON
      ..

cmake --build .
```

The binaries generated at this point can be executed from the MSYS2 UCRT64 terminal, but they will not run from a Windows Command Prompt or PowerShell. The reason is that the MSYS2 UCRT64 terminal is properly configured to find some needed DLLs. In case you want to be able to run the generated **exiv2** binary from any Windows terminal, you'll need to deploy the needed DLLs with the application.

[TOC](#TOC)
<div id="PlatformCygwin">

## Cygwin/64

Please note that the platform Cygwin/32 is obsolete and superceded by Cygwin/64.

Download: [https://cygwin.com/install.html](https://cygwin.com/install.html) and run setup-x86_64.exe.  I install into c:\\cygwin64

You need:
make, cmake, curl, gcc, gettext-devel pkg-config, dos2unix, tar, zlib-devel, libexpat1-devel, git, libxml2-devel python3-interpreter, libiconv, libxml2-utils, libncurses, libxml2-devel libxslt-devel python38 python38-pip python38-libxml2

The CI workflow file `.github/workflows/on_PR_windows_matrix.yml` has a build job named `cygwin` with instructions showing how to configure Exiv2 on  Cygwin/64.

To build unit tests, you should install googletest-release-1.8.0 as discussed at [Unit tests](#UnitTests)

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
<div id="PlatformVisualStudio">

## Visual Studio

We recommend that you use Conan to get the Exiv2 dependencies when using Visual Studio. Exiv2 v0.27 can be built with Visual Studio versions 2008 and later. For the `main` branch we actively support and build with Visual Studio 2019 and 2022.

As well as Visual Studio, you will need to install CMake, Python3, and Conan.

1) Binary installers for CMake on Windows are availably from [https://cmake.org/download/](https://cmake.org/download/).<br/>
2) Binary installers for Python3 are available from [python.org](https://python.org)<br/>
3) Conan can be installed using python/pip.  Details in [README-CONAN.md](README-CONAN.md)


```
..>copy c:\Python37\python.exe c:\Python37\python3.exe
```

The python3 interpreter must be on your PATH.

It is important to highlight that we rely on using of the Universal C Runtime (UCRT) and its relatively new support for UTF-8. Check this [PR](https://github.com/Exiv2/exiv2/pull/2090) for more information.

[TOC](#TOC)
<div id="PlatformUnix">

## Unix

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

### NetBSD

You can build exiv2 from source using the methods described for linux.  I built and installed exiv2 using "Pure CMake" and didn't require conan.

You will want to use the package manager `pkgsrc` to build/install the build and test components listed above.

I entered links into the file system

```
# ln -s /usr/pkg/bin/python37 /usr/local/bin/python3
# ln -s /usr/pkg/bin/bash /bin/bash`
```

It's important to ensure that `LD_LIBRARY_PATH` includes `/usr/local/lib` and `/usr/pkg/lib`.

It's important to ensure that `PATH` includes `/usr/local/bin`, `/usr/pkg/bin` and `/usr/pkg/sbin`.

### FreeBSD

Clang is pre-installed as ``/usr/bin/{cc|c++}` as well as libz and expat.  FreeBSD uses pkg as the package manager which I used to install CMake and git.

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

### Solaris

Solaris uses the package manager pkg.  To get a list of packages:

```bash
$ pkg list
```

To install a package:

```bash
$ sudo pkg install developer/gcc-7
```


[TOC](#TOC)

Written by Robin Mills<br>robin@clanmills.com<br>Updated: 2022-02-22

