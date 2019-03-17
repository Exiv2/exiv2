# Cross-compilation

In this document we provide some guidelines to cross-compile Exiv2. Since there could be several ways to achieve
cross-compilation, we will have different sections for each of them.

Something important to keep in mind is that since Exiv2 have some 3rd party dependencies, we would need to have
such dependencies compiled for the target system. This can be normally achieved by installing system packages, 
by using **conan**, or compiling them by yourselves.

## CMake Toolchain files

Exiv2 is configured through CMake and we can easily achieve cross-comilation via its toolchain files feature. The idea is to
have different toolchains indicating to CMake where to find the tools to cross-compile the project (compiler, linker, etc).
We will place such files under `cmake/toolchain`.

The most important step to remember is to use such a toolchain file when running CMake for the first time to configure the
project (in a clean build directory). In case a CMake cache exists already in the build directory and you try to run
cmake again with a different tooldchain file, things will go wrong.

### Linux-to-Win64

Many Linux distributions provide MinGW packages which provide a GCC compiler to cross-compile from Linux to
windows. In some of these distributions there even exist mingw packages with popular libraries compiled for the target
system (Windows). However, that is not always the case (For example, in Ubuntu 18.04 we cannot find the expat package for
MinGW).

Nonetheless, with conan we can easily compile those 3rd party dependencies. You just need to have a proper profile
as we explain in [README-CONAN.md](README-CONAN.md).

The following code shows the full process to cross compile Exiv2:

```bash
cd $EXIV2_ROOT
mkdir buildMinGW && cd buildMinGW
# Get 3rd party dependencies with conan
conan install ../ --profile linux-to-win64 --build missing
# Configure project to cross compile from linux to windows
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolschains/ubuntu-mingw64.cmake ../
# Compile project
cmake --build . --config Release
```

#### Required packages

- Ubuntu: `mingw-w64`.


#### Issues found

##### mutex in namespace std does not name a type

If you get an error like this:

```bash
[ 11%] Building CXX object src/CMakeFiles/exiv2lib_int.dir/cr2header_int.cpp.obj
In file included from /media/linuxDev/programming/exiv2/include/exiv2/xmp_exiv2.hpp:35:0,
                 from /media/linuxDev/programming/exiv2/include/exiv2/image.hpp:33,
                 from /media/linuxDev/programming/exiv2/src/tiffimage_int.hpp:34,
                 from /media/linuxDev/programming/exiv2/src/cr2header_int.hpp:32,
                 from /media/linuxDev/programming/exiv2/src/cr2header_int.cpp:1:
/media/linuxDev/programming/exiv2/include/exiv2/properties.hpp:215:21: error: ‘mutex’ in namespace ‘std’ does not name a type
         static std::mutex mutex_;
```

It can be probably caused because you are not using the right variant of the MinGW compiler in the toolchain file.
MinGW in Ubuntu comes with two flavours: `posix` and `win32`. When choosing the `posix` flavour, these errors should
dissapear.