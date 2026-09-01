![Exiv2](exiv2.png)

# Building Exiv2 and dependencies with Conan

Conan is a C/C++ package manager. It can fetch the dependencies needed to build Exiv2 without requiring the system package manager for each library.

This guide describes the current Conan 2 workflow used by the project.

## Install Conan 2

```bash
python -m pip install 'conan>=2,<3'
```

Verify the installation:

```bash
conan --version
```

## Create a default profile

Conan 2 stores profiles under `~/.conan2/profiles/`.

```bash
conan profile detect --name default --force
```

On Linux, a typical profile looks like this:

```ini
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=13
compiler.libcxx=libstdc++11
build_type=Release
[options]
```

On Windows, the profile is typically stored at `%USERPROFILE%\.conan2\profiles\default` and uses the MSVC compiler:

```ini
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.runtime=dynamic
compiler.runtime_type=Release
compiler.version=193
build_type=Release
[options]
```

## Configuring the Conan cache

Conan 2 keeps its layout under `~/.conan2`, and cache storage can be overridden in `~/.conan2/global.conf`.

Example:

```ini
core.cache:storage_path=/path/to/conanCache
```

## Install dependencies and configure the build

From the project root:

```bash
mkdir -p build
cd build
conan install .. --build=missing
cmake ..
cmake --build .
ctest --output-on-failure
```

## Visual Studio notes

The repository's legacy `Visual Studio` Conan 1 profiles are no longer valid for Conan 2. Use `compiler=msvc` and the matching `compiler.version` instead of `arch_build` / `os_build` / `compiler.runtime=MD*` settings.

Example profile:

```ini
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.runtime=dynamic
compiler.runtime_type=Release
compiler.version=192
os=Windows
[options]
```

For the modern GitHub Actions workflow, the project sets the profile in place after `conan profile detect` and writes cache settings to `%USERPROFILE%\.conan2\global.conf`.

## Conan recipe

The project-level recipe is in `conanfile.py`. It uses the Conan 2 import:

```python
from conan import ConanFile
```

The dependency graph remains the same as before; only the Conan API and profile/config plumbing were updated to the Conan 2 layout.
