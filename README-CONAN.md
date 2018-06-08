# How to use conan to build Exiv2 and dependencies

Conan is a portable package manager for C/C++ libraries. It can be used to bring all the dependencies needed to
build Exiv2 into local directories, without needing to install system packages.

This document provides a step-by-step guide to show you the basic usage of conan. For more details about the tool, please visit the [Conan  documentation website](http://docs.conan.io/en/latest/).

Although we provide step-by-step instructions to enable you to build Exiv2 with conan, we strongly recommend that you
read conan's documentation to understand the main concepts: [Getting started with Conan](http://docs.conan.io/en/latest/getting_started.html)

To build Exiv2 with conan, you will also need to install CMake.  For more information see [README-CMAKE](README-CMAKE)

<name id="TOC"></a>
----
### T A B L E &nbsp;&nbsp;&nbsp; O&nbsp;&nbsp;F &nbsp;&nbsp;&nbsp; C O N T E N T S

1. [Step by Step Guide](#1)
    1. [Install conan](#1-1)
    2. [Test conan installation](#1-2)
    3. [Create a build directory](#1-3)
    4. [Build dependencies and install conan artefacts in your build directory](#1-4)
    5. [Execute cmake to generate build files for your environment:](#1-5)
2. [Platform Notes](#2)
    1. [Linux Notes](#2-1)
    2. [Visual Studio Notes](#2-2)
    3. [Cygwin Notes](#2-3)
    4. [MinGW Notes](#2-4)
3. [Conan Architecture](#3)
    1. [conanfile.py](#3-1)
    2. [Conan Recipes](#3-2)
    3. [Conan server search path](#3-3)
    4. [Configuring conan on your machine](#3-4)
4. [Building Exiv2 with Adobe XMPsdk 2016](#4)
    1. [Add a remote directory to conan's recipe search path](#4-1)
    2. [Build dependencies and install conan artefacts in your build directory](#4-2)
    3. [Execute cmake to generate build files for your environment](#4-3)
    4. [Build Exiv2 and link Adobe XMPsdk library](#4-4)

<name id="1"></a>
----
# 1 Step by Step Guide

<name id="1-1"></a>
##### 1.1) </a>Install conan:

```bash
$ pip install conan
```
For other installation methods (brew, installers, from sources), visit this [link]([install
conan](http://docs.conan.io/en/latest/installation.html)).

To upgrade the version of conan:

```bash
$ pip install conan --upgrade
```

<name id="1-2"></a>
##### 1.2) Test conan installation

```bash
$ conan --version
Conan version 1.4.1
```

<name id="1-3"></a>
##### 1.3) Create a build directory<name id="1-3"></a>

Create a build directory and run the `$ conan profile list` command:

```bash
$ mkdir build
$ cd build
$ conan profile list
```

**IMPORTANT** _**Visual Studio Users**_ require the profile msvc2017Release64 in %HOMEPATH%/.conan/profile/msvc2017Release64

```ini
[build_requires]
[settings]
arch=x86_64
build_type=Release
compiler=Visual Studio
compiler.runtime=MD
compiler.version=15
os=Windows
arch_build=x86_64
os_build=Windows
[options]
[env]
```

<name id="1-4"></a>
##### 1.4) Build dependencies and install conan artefacts in your build directory</a>

Execute `$ conan install` pointing to the directory containing `conanfile.py`.

```bash
$ conan install .. --build missing  # --profile msvc2017Release64
```

_**Visual Studio Users**_ should use `--profile msvc2017Release64`

The output from this command is quite long as conan downloads or builds zlib, expat, curl and other dependencies.

<name id="1-5"></a>
##### 1.5) Execute cmake to generate build files for your environment.

```bash
$ cmake ..  # -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
```

_**Visual Studio Users**_ should use `-G xxx -DCMAKE_BUILD_TYPE=Release`.

More information about CMake/Exiv2: [README-CMAKE](README-CMAKE).

<name id="1-6"></a>
##### 1.6) Build Exiv2:

```bash
$ cmake --build . --config Release
```

[TOC](#TOC)

<name id="2"></a>
## 2) Platform Notes

<name id="2-1"></a>
### 2.1) Linux Notes

##### Default Profile

When you run conan install for the first time, it will detect and write the default profile ~/.conan/profile/default.  On my Ubuntu system with GCC 4.9, this is:

```bash
[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=gcc
compiler.version=4.9
compiler.libcxx=libstdc++
build_type=Release
[options]
[build_requires]
[env]
```

##### Changing profile settings

One of the most important **profile** settings to be adjusted in your conan profile when working on Linux is the field

```bash
compiler.libcxx=libstdc++11  # Possible values: libstdc++, libstdc++11, libc++
```
With the arrival of the c++11 standard, and the growing popularity of the *clang* compiler, it is increasingly important which version of the standard library to use (corresponds to the `-stdlib` compiler flag).  We recommend the following values of **compiler.libcxx**:

```bash
compiler.libcxx=libstdc++11  # will use -stdlib=libstdc++ and define _GLIBCXX_USE_CXX11_ABI=1
compiler.libcxx=libstdc++    # will use -stdlib=libstdc++ and define _GLIBCXX_USE_CXX11_ABI=0
compiler.libcxx=libc++       # will use -stdlib=libc++

```

As a rule of thumb, set `compiler.libcxx=libstdc++11` when using a version of gcc >= 5.1.

More information about the standard library and GCC [dual ABI in gcc](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html) with GCC works.

Please, be aware that normally when using gcc >= 5.1, \_GLIBCXX\_USE\_CXX11\_ABI is set to 1 by default. However some linux
distributions might set that definition to 0 by default. In case you get linking errors about standard c++ containers or
algorithms when bringing the Exiv2 dependencies with conan, this might indicate a mismatch between the value set in
**compiler.libcxx** and the default values used in your distribution.

<name id="2-2"></a>
### 2.2) Visual Studio Notes

### Profiles for Visual Studio

You can build Exiv2 with either Visual Studio 2015 (version 14) or 2017 (version 15).  You create profiles in %HOMEPATH%\.conan\profiles with a text editor.

If you have an installation of Visual Studio 2017, the profile msvc2017Release64 is as follows:

```ini
[build_requires]
[settings]
arch=x86_64
build_type=Release
compiler=Visual Studio
compiler.runtime=MD
compiler.version=15
os=Windows
arch_build=x86_64
os_build=Windows
[options]
[env]
```

### CMake Generators for Visual Studio

In the step-by-step guide, the command `$ cmake ..` uses
the default CMake generator.  Always use the generator for your version of Visual Studio.  For example:

```bash
c:\....\exiv2\build> conan install .. --profile msvc2017Release64 --build missing
c:\....\exiv2\build> cmake         .. -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
c:\....\exiv2\build> cmake --build .  --config Release
```

CMake provides 4 Generators.  The 64 and 32 bit Generators have different names:

| Architecture | Visual Studio 2015 | Visual Studio 2017|
|:---------|--------------------|--------------------|
| 64 bit | "Visual Studio 14 2015 Win64"    | "Visual Studio 15 2017 Win64" |
| 32 bit | "Visual Studio 14 2015"    | "Visual Studio 15 2017" |

### Recommended settings for Visual Studio


##### 64 bit Release Build

| | Visual Studio 2015 | Visual Studio 2017|
|:---------|--------------------|--------------------|
| _**conan install .. --profile**_ | msvc2015Release64 | msvc2017Release64 |
| _**cmake -G**_                   |  "Visual Studio 14 2015 Win64"    | "Visual Studio 15 2017 Win64" |
| _**cmake**_                      | -DCMAKE\_BUILD\_TYPE=Release | -DCMAKE\_BUILD\_TYPE=Release |
| _**profile**_ | arch=x86\_64  | arch=x86\_64 |
| | arch\_build=x86\_64 | arch\_build=x86\_64 |
| | build\_type=Release | build\_type=Release |
| | compiler.runtime=MD | compiler.runtime=MD |
| | compiler.version=14 | compiler.version=15 |
| | compiler=Visual Studio | compiler=Visual Studio |
| | os=Windows | os=Windows |
| | os\_build=Windows | os\_build=Windows |

##### Debug Builds

|| Visual Studio 2015 | Visual Studio 2017|
|:-------|-------|------|
| _**conan install .. --profile**_ | msvc2015Debug64 | msvc2017Debug64 |
| _**cmake**_ | -DCMAKE\_BUILD\_TYPE=Debug | -DCMAKE\_BUILD\_TYPE=Debug |
| _**profile**_ | build_type=Debug | build_type=Debug |
| | compiler.runtime=MDd | compiler.runtime=MDd |

##### 32bit Builds

|| Visual Studio 2015 | Visual Studio 2017|
|:-----------|--------------------|--------------------|
| _**conan install .. --profile**_ | msvc2015Release32 | msvc2017Release32 |
| _**cmake -G**_ | "Visual Studio 14 2015" | "Visual Studio 15 2017" |
| _**profile**_ | arch=x86 | arch=x86 |
| | arch_build=x86 | arch_build=x86 |

### Changing profile settings with the conan command

It is recommended that you create 8 profiles for msvc{2017|2015}{Release|Debug}{64|32}.

You can modify profile settings on the command line.
The following example demonstrates making substantial changes to profile settings by performing a 32 bit build using Visual Studio 2015 with a 2017 profile!

```bash
$ conan install .. --profile msvc2017Release64 -s arch_build=x86 -s arch=x86 -s compiler.version=14
$ cmake         .. -G "Visual Studio 2015"
$ cmake --build .  --config Release
```

### CMake/Cygwin Collisions on the build machine

If you have Cygwin installed on your build machine, you may encounter the situation
that CMake erroneously finds library files in Cygwin directories and adds `c:\\cygwin64\\usr\\include` to the
compiler header search path.  FindIntl is a prime suspect and believe it's caused by %PATH%.

If necessary, temporarily rename c:\\cygwin64\\usr\\include as c:\\cygwin64\\usr\\uncle to hide those files when working with CMake.

<name id="2-3"></a>
### 2.3) Cygwin Notes

Do not use conan on the Cygwin Platform.  To build Exiv2 for Cygwin use CMake.  See [README-CMAKE](README-CMAKE)

<name id="2-4"></a>
### 2.4) MinGW Notes

Team Exiv2 does not provide support for MinGW.

Users have reported success with CMake (without conan) on both msys/1.0 and msys/2.0.

[TOC](#TOC)

<name id="3">
## 3 Conan Architecture

<name id="3-1">
##### 3.1) conanfile.py

In the root level of the **Exiv2** repository, the file `conanfile.py` defines C/C++ dependencies with the syntax: `Library/version@user/channel`

For example, **zlib**:

```python
self.requires('self.requires('zlib/1.2.11@conan/stable')')
```

<name id="3-2">
##### 3.2) Conan _**Recipes**_

Conan searches remote servers for a _**recipe**_ to build a dependency.

A _**recipe**_ is a python file which indicates how to build a library from sources. The recipe
understands configurations: Platform/Compiler/Settings.  If the remote server has a pre-compiled package for
your configuration, it will be downloaded.  Otherwise, conan will compile the libraries on your machine using instructions in the recipe.

To illustrate, here is a list of packages that returned by the command `$ conan search`

```bash
$ conan search --remote conan-center zlib/1.2.11@conan/stable
```

The output should be:

```bash
Existing packages for recipe zlib/1.2.11@conan/stable:

    Package_ID: 0000193ac313953e78a4f8e82528100030ca70ee
        [options]
            shared: False
        [settings]
            arch: x86_64
            build_type: Debug
            compiler: gcc
            compiler.version: 4.9
            os: Linux
        Outdated from recipe: False

    Package_ID: 014be746b283391f79d11e4e8af3154344b58223
        [options]
            shared: False
        [settings]
            arch: x86_64
            build_type: Debug
            compiler: gcc
            compiler.exception: seh
            compiler.threads: posix
            compiler.version: 5
            os: Windows
        Outdated from recipe: False

... deleted ....

```

<name id="3-3">
##### 3.3) Conan server search path

Conan searches remote servers for a _**recipe**_ to build the dependency.  You can list them with the command:

```bash
$ conan remote list
```

You can add servers to the conan server search path:

```bash
$ conan remote add conan-piponazo https://api.bintray.com/conan/piponazo/piponazo
```

<name id="3-4">
##### 3.4) Configuring conan on your machine

Conan stores its configuration and local builds in the directory ~/.conan (%HOMEPATH%\\.conan on Windows).

Conan installs several files and two directories:

```bash
$HOME/.conan/profiles	Configuration files for compilers/platforms
$HOME/.conan/data	    Dependencies are built/stored in this directory
```

<name id="3-5">
##### 3.5) Running `conan install` for the first time

The first time you run `$ conan install`, it will auto-detect your configuration and store a default profile in the file
$HOME/.conan/profiles/default

Normally you will want to define new profiles for choosing different compilers (msvc, gcc, clang), different
build_type (Release, Debug), runtimes (MD, MT, MDd, MTd)

The expected output should be something like this, in case it's the first time you run conan:

```bash
$ conan install ..  --build missing
Expat/2.2.5@pix4d/stable: Retrieving from predefined remote 'conan-center'
Expat/2.2.5@pix4d/stable: Trying with 'conan-center'...
Downloading conanmanifest.txt
[==================================================] 220B/220B
Downloading conanfile.py
[==================================================] 1.7KB/1.7KB
zlib/1.2.11@conan/stable: Retrieving from predefined remote 'conan-center'
zlib/1.2.11@conan/stable: Trying with 'conan-center'...
Downloading conanmanifest.txt
[==================================================] 121B/121B
Downloading conanfile.py
[==================================================] 5.7KB/5.7KB
libcurl/7.56.1@bincrafters/stable: Retrieving from predefined remote 'bincrafters'
libcurl/7.56.1@bincrafters/stable: Trying with 'bincrafters'...
Downloading conanmanifest.txt
...
PROJECT: Installing D:\Dev\Windows\projects\exiv2\conanfile.py
Requirements
    Expat/2.2.5@pix4d/stable from 'conan-center'
    OpenSSL/1.0.2n@conan/stable from 'conan-center'
    gtest/1.8.0@bincrafters/stable from 'conan-center'
    libcurl/7.56.1@bincrafters/stable from 'bincrafters'
    zlib/1.2.11@conan/stable from 'conan-center'
Packages
    Expat/2.2.5@pix4d/stable:6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7
    OpenSSL/1.0.2n@conan/stable:606fdb601e335c2001bdf31d478826b644747077
    gtest/1.8.0@bincrafters/stable:a35f8fa327837a5f1466eaf165e1b6347f6e1e51
    libcurl/7.56.1@bincrafters/stable:e37838f02fd790447943465f1c9317fd1c59b95c
    zlib/1.2.11@conan/stable:6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7

PROJECT: Retrieving package 6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7
Expat/2.2.5@pix4d/stable: Looking for package 6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7 in remote 'conan-center'
Downloading conanmanifest.txt
[==================================================] 323B/323B
Downloading conaninfo.txt
[==================================================] 438B/438B
Downloading conan_package.tgz
[==================================================] 133.6KB/133.6KB
Expat/2.2.5@pix4d/stable: Package installed 6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7
PROJECT: Retrieving package a35f8fa327837a5f1466eaf165e1b6347f6e1e51
gtest/1.8.0@bincrafters/stable: Looking for package a35f8fa327837a5f1466eaf165e1b6347f6e1e51 in remote 'conan-center'
Downloading conanmanifest.txt
[==================================================] 3.5KB/3.5KB
Downloading conaninfo.txt
[==================================================] 478B/478B
Downloading conan_package.tgz
[==================================================] 1001.1KB/1001.1KB
gtest/1.8.0@bincrafters/stable: Package installed a35f8fa327837a5f1466eaf165e1b6347f6e1e51
PROJECT: Retrieving package 6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7
...
PROJECT: Generator cmake created conanbuildinfo.cmake
PROJECT: Generator txt created conanbuildinfo.txt
PROJECT: Generated conaninfo.txt
PROJECT imports(): Copied 5 '.dll' files
(conan)
```

The message `Copied 5 '.dll' files` indicates that the dependent libraries were successfully copied from the local cache on your machine to your build directory.

Note that it first downloads the recipes, and then the binary packages. When everything goes well, conan found
the recipes in the remotes, and it also found packages for our configuration (msvc2017, Release, MD).

However, if you use other configuration for which there are no packages in the remotes, you will get an error such as:

```bash
    PROJECT: WARN: Can't find a 'zlib/1.2.11@conan/stable' package for the specified options and settings:
    - Settings: arch=x86_64, build_type=Release, compiler=clang, compiler.version=3.9, os=Macos
    - Options: shared=False

    ERROR: Missing prebuilt package for 'zlib/1.2.11@conan/stable'
    Try to build it from sources with "--build zlib"
    Or read "http://docs.conan.io/en/latest/faq/troubleshooting.html#error-missing-prebuilt-package"
```

In that case, we can tell conan to build the library:

```bash
$ conan install .. --profile MyEsotericProfile --build missing
```

Once the command succeeds, we will have the libraries in our system (you can find the recipes and packages in
`$HOME/.conan/data`). When you execute the command `conan install` with the same profile, the following output is typical:

```bash
$ conan install ..
PROJECT: Installing D:\Dev\Windows\projects\exiv2\conanfile.py
Requirements
    Expat/2.2.5@pix4d/stable from 'conan-center'
    OpenSSL/1.0.2n@conan/stable from 'conan-center'
    gtest/1.8.0@bincrafters/stable from 'conan-center'
    libcurl/7.56.1@bincrafters/stable from 'bincrafters'
    zlib/1.2.11@conan/stable from 'conan-center'
Packages
    Expat/2.2.5@pix4d/stable:6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7
    OpenSSL/1.0.2n@conan/stable:606fdb601e335c2001bdf31d478826b644747077
    gtest/1.8.0@bincrafters/stable:a35f8fa327837a5f1466eaf165e1b6347f6e1e51
    libcurl/7.56.1@bincrafters/stable:e37838f02fd790447943465f1c9317fd1c59b95c
    zlib/1.2.11@conan/stable:6cc50b139b9c3d27b3e9042d5f5372d327b3a9f7

Expat/2.2.5@pix4d/stable: Already installed!
gtest/1.8.0@bincrafters/stable: Already installed!
zlib/1.2.11@conan/stable: Already installed!
OpenSSL/1.0.2n@conan/stable: Already installed!
libcurl/7.56.1@bincrafters/stable: Already installed!
PROJECT: Generator cmake created conanbuildinfo.cmake
PROJECT: Generator txt created conanbuildinfo.txt
PROJECT: Generated conaninfo.txt
PROJECT imports(): Copied 5 '.dll' files
```

Indicating that the packages were found in the local cache.

[TOC](#TOC)

<name id="4">
## 4 Building Exiv2 with Adobe XMPsdk 2016

With Exiv2 v0.27, you can build Exiv2 with Adobe XMPsdk 2016 on Linux/GCC, Mac/clang and Visual Studio 2017.
Other platforms such as Cygwin are not supported by Adobe.  Adobe/XMPsdk is built as a external library.
Applications which wish use the Adobe XMPsdk directly should build Exiv2 in this configuration and the
library can be used by the application and Exiv2.  The Adobe XMPsdk can be built as a static or shared library (.DLL)

To build Exiv2 with Adobe XMPsdk 2016, perform steps 1.1, 1.2 and 1.3 described above, then perform the following:

<name id="4-1">
##### 4.1) Add a remote directory to conan's recipe search path

By default, conan knows about several public conan repositories. Exiv2 requires
the **piponazo** repository to find the XmpSdk dependency which is not available from **conan-center** repository.

```bash
$ conan remote add conan-piponazo https://api.bintray.com/conan/piponazo/piponazo
```

<name id="4-2">
##### 4.2) Build dependencies and install conan artefacts in your build directory

```bash
$ conan install .. --options xmp=True --build missing
```

<name id="4-3">
##### 4.3) Execute cmake to generate build files for your environment:

You must tell CMake to link Adobe's library:

```bash
$ cmake .. -DEXIV2_ENABLE_EXTERNAL_XMP=On # -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
```
**MacOS-X** users should use the cmake _**Xcode**_ Generator

```bash
$ cmake .. -DEXIV2_ENABLE_EXTERNAL_XMP=On -G Xcode
```

<name id="4-4">
##### 4.4) Build Exiv2 and link Adobe XMPsdk library

```bash
$ cmake --build . --config Release
```
[TOC](#TOC)

