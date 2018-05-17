# How to use conan

Conan is a portable package manager for C/C++ libraries. It can be used for bringing all the dependencies needed to
build Exiv2 into local directories, without needing to install system packages.

This document provides a step-by-step guide to show you the basic usage of conan. For more details about the tool,
please visit their [documentation website](http://docs.conan.io/en/latest/).


## Main concepts

Even when the following instructions will allow you to use conan directly, we strongly recommend to take a look to their
"Getting started" [page](http://docs.conan.io/en/latest/getting_started.html), to better understand the main concepts.

In the root level of the **Exiv2** repository, there is a file named `conanfile.py` that defines the C/C++ dependencies
of the project. One of the dependencies is **Expat**, and it is stated like this:

```python
self.requires('Expat/2.2.5@pix4d/stable')
```

The syntax stands for: `Library/version@user/channel`.

Conan will search for these dependencies in a list of remotes. In those servers there are recipes (the python code that
explains how to build a library from its sources) and binary packages (different versions of the library for several
combinations of configurations: Platform/Compiler/Settings). If there is already a packages for your configuration,
conan will fetch them. Otherwise, it will compile the libraries for you by using the instructions provided in the
recipe. To better illustrate this idea, here we show a list of packages that is returned by the command `conan search`

```bash
conan search -r conan-center zlib/1.2.11@conan/stable

   # conan-center zlib/1.2.11@conan/stable (This is the recipe for which we were searching packages)

   Package_ID: ff0b79bd9b0fc1d915200da2fe02ae326cef4b1b
       [options]
           shared: False
       [settings]
           arch: x86_64
           build_type: Debug
           compiler: gcc
           compiler.exception: seh
           compiler.threads: posix
           compiler.version: 4.9
           os: Windows
       Outdated from recipe: False

   Package_ID: ff5ec8938c949149b25b89d656bed297cd5d5aad
       [options]
           shared: False
       [settings]
           arch: x86
           build_type: Debug
           compiler: clang
           compiler.version: 3.9
           os: Linux
       Outdated from recipe: False

   Package_ID: ff82a1e70ba8430648a79986385b20a3648f8c19
       [options]
           shared: True
       [settings]
           arch: x86_64
           build_type: Debug
           compiler: Visual Studio
           compiler.runtime: MDd
           compiler.version: 15
           os: Windows
       Outdated from recipe: False
```

## Installation

There are several ways to install conan, but the recommended one is using **pip**.

```bash
pip install conan
```

For other installation methods (brew, installers, from sources), visit this [link]([install
conan](http://docs.conan.io/en/latest/installation.html)).

By default, conan only point to few public conan repositories. However you can add more easily. We rely in the usage of
the **bincrafters** and **piponazo** repositories to find some of the dependencies (libcurl and XmpSdk) . Therefore, we
will need to add it to the list of remotes
(we only need to do this once):

```bash
conan remote add conan-bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan remote add conan-piponazo https://api.bintray.com/conan/piponazo/piponazo
```

## Basic usage

The first thing we need to do, is to check that conan was properly installed and you can access it:
```bash
$ conan --version
Conan version 1.1.1
```

The first time you run conan, it will auto-detect your configuration and store it in a default profile in the file
$HOME/.conan/profiles/default. In case you are using Windows, and have a installation of Visual Studio 2017, the default
profile will look like this:

```Ini
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

But normally you will want to define new profiles for choosing different compilers (msvc, gcc, clang), different
build_type (Release, Debug), runtimes (MD, MT, MDd, MTd), etc. Let´s create a copy of the default profile and name if
**msvc2017Release**.

Then, we will create a build directory where we will run the conan commands and we access it:
```bash
$ mkdir build
$ cd build
```

Finally, we use the `conan install` command pointing to the directory where the `conanfile.py` is, to bring the
dependencies:
```bash
$ conan install .. --profile msvc2017Release
```

The expected output should be something like this, in case it's the first time you run conan:
```bash
$ conan install ..
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

Note that it first downloads the recipes, and then the binary packages. In this case everything went well, conan found
the recipes in the remotes, and it also found packages for our configuration (msvc2017, Release, MD).

However, if you use other configuration for which there are not packages in the remotes, you will get an error similar
to this:
```bash
    PROJECT: WARN: Can't find a 'zlib/1.2.11@conan/stable' package for the specified options and settings:
    - Settings: arch=x86_64, build_type=Release, compiler=clang, compiler.version=3.9, os=Macos
    - Options: shared=False

    ERROR: Missing prebuilt package for 'zlib/1.2.11@conan/stable'
    Try to build it from sources with "--build zlib"
    Or read "http://docs.conan.io/en/latest/faq/troubleshooting.html#error-missing-prebuilt-package"
```

In that case, we still have the chance to compile the library by ourselves by telling conan to do so:

```bash
    $ conan install ../ --profile MyEsotericProfile --build missing
```

Once the command succeeds, we will have the libraries in our system (you can find the recipes and packages in
`$HOME/.conan/data`). Any new call to `conan install` with the same profile, will produce this output:

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

That´s all. After a successful call to `conan install`, some files will be generated in the `build` folder that will
allow to CMake (or other systems) to find the headers and libraries of the dependencies handled by conan. By simply
running:

```bash
$ cmake ..
```

CMake should manage to configure the project finding all the dependencies. If you want to know more about how to use
CMake in this project, refer to [README-CMAKE](README-CMAKE).

### Windows specific notes

Some people have difficulties when starting to use conan + CMake + Visual Studio, but most of them are due to the lack
of awareness in some details off the tool chain. In the previous section, the call `cmake ..` from a terminal, will pick
a default CMake generator, that will depend on different factors.

To choose a specific generator you can use the option `-G "Generator Name"`. If in your conan profile, you use a 64bits
architecture  (`arch: x86_64`), and you want to use a Visual Studio generator, you will need to indicate it in the first
CMake call:

```bash
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release ...
```

It is also important to remark that, even when you can change the build type within Visual Studio once you open the
solution generated by CMake, the dependencies brought by conan will use a specific configuration. If you try to mix
conan dependencies that use a certain Visual Studio Runtime (example: `MD`), and try to compile Exiv2 with other one
(example: `MT`), you will obtain linking errors. In order to make experiments with different runtime options, it is
preferable to have different build folders.
