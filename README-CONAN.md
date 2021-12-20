![Exiv2](exiv2.png)

# Building Exiv2 and dependencies with conan

Conan is a portable package manager for C/C++ libraries. It can be used to create all  dependencies needed to build Exiv2, without needing to install system packages.

This document provides a step-by-step guide to show you the basic usage of conan. For more details about the tool,
please visit the [Conan documentation website](http://docs.conan.io/en/latest/).

Although we provide step-by-step instructions to enable you to build Exiv2 with conan, we recommend that you read conan's documentation to understand the main concepts: [Getting started with Conan](http://docs.conan.io/en/latest/getting_started.html)

To build Exiv2 with conan, you will also need to install CMake.  https://cmake.org/download/

_**We do not recommend using conan on MinGW, Cygwin, Unix or to cross compile from Linux to those platforms.**<br>
The build procedures for those platforms are discussed here: See [README.md](README.md)_

<name id="TOC"></a>
----
### TABLE OF CONTENTS

1. [Step by Step Guide](#1)
    1. [Install conan](#1-1)
    2. [Test conan installation](#1-2)
    3. [Create a build directory](#1-3)
    4. [Build dependencies, create build environment, build and test](#1-4)
2. [Platform Notes](#2)
    1. [Linux Notes](#2-1)
    2. [Visual Studio Notes](#2-2)
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
5. [Webready Support](#5)

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
Conan version 1.23.0
```

<name id="1-3"></a>

##### 1.3) Create a build directory<name id="1-3"></a>

Create a build directory and run the conan commands:

```bash
$ mkdir build
$ cd build
$ conan profile list
```
_**Visual Studio Users**_

_The profile msvc2019Release96 in `%USERPROFILE%\.conan\profiles\msvc2019Release64` is:_

```ini
[build_requires]
[settings]
arch=x86_64
build_type=Release
compiler=Visual Studio
compiler.runtime=MD
compiler.version=16
os=Windows
arch_build=x86_64
os_build=Windows
[options]
[env]
```

_Profiles for Visual Studio are discussed in detail here: [Visual Studio Notes](#2-2)_

<name id="1-4"></a>

##### 1.4) Build dependencies, create build environment, build and test</a>


|         | Build Steps                                                              | Linux and macOS       | Visual Studio |
|:--      |:-------------------------------------------------------------------------|-----------------------|------------------------------|
| _**1**_ | Get conan to fetch dependencies<br><br>The output can be quite<br>long as conan downloads and/or builds<br>zlib, expat, curl and other dependencies.| $ conan install ..<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;--build missing       | c:\\..\\build> conan install .. --build missing<br>&nbsp;&nbsp;&nbsp;&nbsp;--profile msvc2019Release64 |
| _**2**_ | Get cmake to generate<br>makefiles or sln/vcxproj                        | $ cmake ..            | c:\\..\\build> cmake&nbsp;..&nbsp;-G&nbsp;"Visual Studio 16 2019"
| _**3**_ | Build                                                                    | $ cmake --build .     | c:\\..\\build>&nbsp;cmake&nbsp;--build&nbsp;.&nbsp;--config&nbsp;Release<br>You may prefer to open exiv2.sln and build using the IDE. |
| _**4**_ | Optionally Run Test Suite<br/>Test documentation: [README.md](README.md) | $ ctest               | c:\\..\\build>&nbsp;ctest -C Release |

[TOC](#TOC)
<name id="2"></a>

## 2) Platform Notes

<name id="2-1"></a>

### 2.1) Linux Notes

##### Default Profile

When you run conan install for the first time, it will detect and write the default profile ~/.conan/profile/default.  On my Ubuntu system with GCC 4.9, this is:

```ini
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

One of the most important **profile** settings to be adjusted in your conan profile when working on Linux is the field:

```bash
compiler.libcxx=libstdc++11  # Possible values: libstdc++, libstdc++11, libc++
```
With the arrival of the c++11 standard, and the growing popularity of the *clang* compiler, it is increasingly important which version of the standard library to use (corresponds to the `-stdlib` compiler flag).

Recommended **libcxx**
settings works with conan and different compilers:

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

[TOC](#TOC)
<name id="2-2"></a>

### 2.2) Visual Studio Notes

We recommend that you install python as discussed here:  [https://github.com/Exiv2/exiv2/pull/1403#issuecomment-731836146](https://github.com/Exiv2/exiv2/pull/1403#issuecomment-731836146)

### Profiles for Visual Studio

Exiv2 v0.27 can be built with VS 2008, 2010, 2012, 2013, 2015 , 2017 and 2019.

Exiv2 v1.0 is being "modernised" to C++11 and will not support C++98.
We don't expect Exiv2 v1.0 to build with VS versions earlier than VS 2015.

You create profiles in %HOMEPATH%\.conan\profiles with a text editor.  For your convenience, you'll find profiles in `<exiv2dir>\cmake\msvc_conan_profiles`. 

```
Profile :=    msvc{Edition}{Type}{Bits}
Edition :=  { 2019    | 2017  |  2015  }
Type    :=  { Release | Debug }
Bits    :=  { 64      | 32    }  
Examples:     msvc2019Release64 msvc2017Release32  msvc2015Debug32
```

The profile msvc2019Release64 is as follows:

```ini
[build_requires]
[settings]
arch=x86_64
build_type=Release
compiler=Visual Studio
compiler.runtime=MD
compiler.version=16
os=Windows
arch_build=x86_64
os_build=Windows
[options]
[env]
```

### Tools for Visual Studio 2019

You will need cmake version 3.14 (and up) and conan 1.14 (and up).

Additionally, when I upgraded to conan 1.14.3, I had to manually update the file `settings.yml` as follows. For me: `%USERPROFILE% == C:\Users\rmills`:

```bat
copy/y %USERPROFILE%\.conan\settings.yml.new %USERPROFILE%\.conan\settings.yml
```

### CMake Generators for Visual Studio

In the step-by-step guide, the command `$ cmake ..` uses
the default CMake generator.  Always use the generator for your version of Visual Studio.  For example:

```bat
c:\..\build> conan install .. --build missing --profile msvc2019Release64
c:\..\build> cmake         .. -G "Visual Studio 16 2019" -A x64
c:\..\build> cmake --build .  --config Release
```

CMake provides Generators for different editions of Visual Studio.  The 64 and 32 bit Generators have different names:

| Architecture | Visual Studio 2019 | Visual Studio 2017 | Visual Studio 2015 |
|:---------    |--------------------|--------------------|--------------------|--------------------|
| 64 bit       | -G "Visual Studio 16 2019" -A x64          | -G "Visual Studio 15 2017 Win64" | -G "Visual Studio 14 2015 Win64"     |
| 32 bit       | -G "Visual Studio 16 2019" -A Win32        | -G "Visual Studio 15 2017"       | -G "Visual Studio 14 2015"           |

### Recommended settings for Visual Studio

##### 64 bit Release Build

| | Visual Studio 2019 | Visual Studio 2017 | Visual Studio 2015|
|:---------|--------------------|--------------------|--------------|
| _**conan install .. --profile**_ | msvc2019Release64 | msvc2017Release64 | msvc2015Release64 |
| _**cmake**_                   |  -G "Visual Studio 16 2019" -A x64   |  -G "Visual Studio 15 2017 Win64"    | -G "Visual Studio 14 2015 Win64" |
| _**profile**_<br><br><br><br><br><br><br>_ | arch=x86\_64<br>arch\_build=x86\_64<br>build\_type=Release<br>compiler.runtime=MD<br>compiler.version=16<br>compiler=Visual Studio<br>os=Windows<br>os\_build=Windows  | arch=x86\_64<br>arch\_build=x86\_64<br>build\_type=Release<br>compiler.runtime=MD<br>compiler.version=15<br>compiler=Visual Studio<br>os=Windows<br>os\_build=Windows  | arch=x86\_64<br>arch\_build=x86\_64<br>build\_type=Release<br>compiler.runtime=MD<br>compiler.version=14 <br>compiler=Visual Studio<br>os=Windows<br>os\_build=Windows |

##### Debug Builds

|| Visual Studio 2019 | Visual Studio 2017 | Visual Studio 2015 |
|:-------|-------|------|--------------|
| _**conan install .. --profile**_ | msvc2019Debug64 | msvc2017Debug64 | msvc2015Debug64 |
| _**profile**_<br>_ | build\_type=Debug<br>compiler.runtime=MDd | build\_type=Debug<br>compiler.runtime=MDd | build_type=Debug<br>compiler.runtime=MDd |

##### 32bit Builds

|| Visual Studio 2019 | Visual Studio 2017 | Visual Studio 2015 |
|:-----------|--------------------|--------------------|--------------------|
| _**conan install .. --profile**_ | msvc2019Release32 | msvc2017Release32 | msvc2015Release32 |
| _**cmake**_ | -G "Visual Studio 16 2019" -A Win32 | -G "Visual Studio 15 2017" | -G "Visual Studio 14 2015" |
| _**profile**_<br>_ | arch=x86<br>arch\_build=x86 | arch=x86<br>arch\_build=x86 | arch=x86<br>arch\_build=x86 |

##### Static Builds

The default builds of Exiv2 and sample applications build and use DLLs.

To build static libraries, use the cmake option `-DBUILD_SHARED_LIBS=Off`.  You will probably also want to use the static run-time.  The default is to use the dynamic run-time library.

```bash
$ cmake .. -DBUILD_SHARED_LIBS=Off -DEXIV2_ENABLE_DYNAMIC_RUNTIME=Off
```

If you wish to use the static C run-time library, use the following option in the conan profile.

|                      | Static Release      | Static Debug |
|:---                  |:---------           |:-------------------|
| **profile setting**  | compiler.runtime=MT | compiler.runtime=MTd |

If you receive a linker warning concerning `LIBCMT`, it is because you are attempting to link libraries which have been built with different run-time libraries.
You should link everything with the dynamic or static run-time. You can link a static library with the dynamic run-time if you wish.

### Changing profile settings with the conan command

It is recommended that you use profiles provided in `<exiv2dir>\cmake\msvc_conan_profiles`.

You can modify profile settings on the command line.
The following example demonstrates making substantial changes to profile settings by performing a 32 bit build using Visual Studio 2015 with a 2017 profile!  This example is not considered good practice, it is an illustration of some conan flexibility which may be useful when your build environment is automated.

```bash
$ conan install .. --profile msvc2017Release64 -s arch_build=x86 -s arch=x86 -s compiler.version=14
$ cmake         .. -G "Visual Studio 2015"
$ cmake --build .  --config Release
```

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

[TOC](#TOC)
<name id="3-2">

##### 3.2) Conan _**Recipes**_

Conan searches remote servers for a _**recipe**_ to build a dependency.

A _**recipe**_ is a python file which indicates how to build a library from sources. The recipe
understands configurations: Platform/Compiler/Settings.  If the remote server has a pre-compiled package for
your configuration, it will be downloaded.  Otherwise, conan will compile the libraries on your machine using instructions in the recipe.

To illustrate, here is list of packages that returned by the command `$ conan search`

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

[TOC](#TOC)
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

[TOC](#TOC)
<name id="3-4">

##### 3.4) Configuring conan on your machine

Conan stores its configuration and local builds in the directory ~/.conan (%HOMEPATH%\\.conan on Windows).

Conan installs several files and two directories:

```bash
$HOME/.conan/profiles   Configuration files for compilers/platforms
$HOME/.conan/data       Dependencies are built/stored in this directory
```

[TOC](#TOC)
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
**macOS** users should use the cmake _**Xcode**_ Generator

```bash
$ cmake .. -DEXIV2_ENABLE_EXTERNAL_XMP=On -G Xcode
```

<name id="4-4">

##### 4.4) Build Exiv2 and link Adobe XMPsdk library

```bash
$ cmake --build . --config Release
```

[TOC](#TOC)
<name id="5">

## 5 Webready Support

Exiv2 can perform I/O using internet protocols such as http, https and ftp.

The feature is disabled by default.  You will need to instruct conan to build/download necessary libraries (curl and openssl) and tell CMake to link to the libraries.

```bash
$ conan install .. --options webready=True
$ cmake -DEXIV2_ENABLE_WEBREADY=ON -DEXIV2_ENABLE_CURL=ON ..
```

[TOC](#TOC)

Written by Robin Mills<br>robin@clanmills.com<br>Updated: 2021-12-17
