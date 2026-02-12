if not expat_dep.found()
  xmp_dep = disabler()
  xmp_public_args = []
  subdir_done()
endif

sdk = 'XMP-Toolkit-SDK'

# The SDK includes expat as "third-party/expat/lib/expat.h".
# A wrapper header at xmpsdk/third-party/expat/lib/expat.h redirects
# this to the system expat via #include_next.
# Note: sdk / 'source' is intentionally NOT on the include path.
# SDK files use "source/..." relative includes (resolved via the SDK root),
# and adding source/ directly causes a case-insensitive collision on Windows
# (Cygwin/MSYS2) between SDK's Endian.h and system <endian.h>.
xmp_inc = include_directories(
  '.',
  sdk / 'public/include',
  sdk,
  sdk / 'third-party/zuid',
  sdk / 'third-party/zuid/interfaces',
)

xmp_sources = files(
  # XMPCore sources
  sdk / 'XMPCore/source/ExpatAdapter.cpp',
  sdk / 'XMPCore/source/ParseRDF.cpp',
  sdk / 'XMPCore/source/WXMPMeta.cpp',
  sdk / 'XMPCore/source/WXMPUtils.cpp',
  sdk / 'XMPCore/source/WXMPIterator.cpp',
  sdk / 'XMPCore/source/XMPCore_Impl.cpp',
  sdk / 'XMPCore/source/XMPIterator.cpp',
  sdk / 'XMPCore/source/XMPMeta.cpp',
  sdk / 'XMPCore/source/XMPMeta-GetSet.cpp',
  sdk / 'XMPCore/source/XMPMeta-Parse.cpp',
  sdk / 'XMPCore/source/XMPMeta-Serialize.cpp',
  sdk / 'XMPCore/source/XMPUtils.cpp',
  sdk / 'XMPCore/source/XMPUtils-FileInfo.cpp',
  # Shared sources
  sdk / 'source/IOUtils.cpp',
  sdk / 'source/PerfUtils.cpp',
  sdk / 'source/SafeStringAPIs.cpp',
  sdk / 'source/UnicodeConversions.cpp',
  sdk / 'source/XIO.cpp',
  sdk / 'source/XML_Node.cpp',
  sdk / 'source/XMPFiles_IO.cpp',
  sdk / 'source/XMP_LibUtils.cpp',
  sdk / 'source/XMP_ProgressTracker.cpp',
  # MD5
  sdk / 'third-party/zuid/interfaces/MD5.cpp',
)

# Public args propagate to consumers (exiv2lib) via xmp_dep.
# Private args apply only to XMP SDK compilation.
xmp_public_args = ['-DBanAllEntityUsage=1', '-DEXV_ADOBE_XMPSDK=2016']
xmp_private_args = []

if host_machine.system() == 'windows'
  xmp_sources += files(sdk / 'source/Host_IO-Win.cpp')
  xmp_public_args += '-DWIN_ENV'
  # UNICODE/_UNICODE: The XMP SDK uses wide-char Windows APIs internally
  # (e.g. FindFirstFileW, WIN32_FIND_DATAW in Host_IO-Win.cpp) but also
  # calls generic macros like FindNextFile() that resolve to the A (ANSI)
  # or W (wide) variant based on the UNICODE define. Without these defines,
  # FindNextFile resolves to FindNextFileA, causing a type mismatch with
  # the WIN32_FIND_DATAW struct the SDK code passes to it.
  # These are PRIVATE because UNICODE affects Windows API macros globally
  # and would break exiv2 code (e.g. gai_strerror → gai_strerrorW).
  xmp_private_args += ['-DXML_STATIC', '-DUNICODE', '-D_UNICODE', '-DWIN32',
                       '-D_CRT_SECURE_NO_WARNINGS', '-D_SCL_SECURE_NO_WARNINGS']
  if cpp.get_id() not in ['msvc', 'clang-cl']
    # Non-MSVC compilers (MinGW-GCC, MSYS2-Clang) on Windows lack MSVC's
    # <sal.h> header which defines SAL annotations used by the XMP SDK's
    # SafeStringAPIs.h. Force-include our compatibility shim that defines
    # them as empty. clang-cl uses the MSVC toolchain and has <sal.h>.
    # See sal_compat.h for details.
    xmp_private_args += ['-include', meson.current_source_dir() / 'sal_compat.h']
    # https://stackoverflow.com/questions/18551409/localtime-r-support-on-mingw
    xmp_private_args += '-D_POSIX_THREAD_SAFE_FUNCTIONS'
  endif
elif host_machine.system() == 'darwin'
  xmp_sources += files(sdk / 'source/Host_IO-POSIX.cpp')
  xmp_public_args += '-DMAC_ENV'
else
  xmp_sources += files(sdk / 'source/Host_IO-POSIX.cpp')
  xmp_public_args += '-DUNIX_ENV'
  # The SDK's EndianUtils.hpp only auto-detects endianness for GCC on
  # x86/x86_64/SPARC under UNIX_ENV. For other architectures (ARM, WASM,
  # etc.) or non-GCC compilers, define it explicitly using meson's host info.
  if host_machine.endian() == 'big'
    xmp_private_args += '-DkBigEndianHost=1'
  else
    xmp_private_args += '-DkBigEndianHost=0'
  endif
endif

# Third-party SDK: suppress warnings and errors from third-party code.
# - warning_level=0: avoid project-level -Wall/-Wextra/-Wpedantic
# - cpp_std=c++20: the SDK uses bitwise ops between different unnamed enum
#   types (e.g. kXMP_PropValueIsStruct | kRDF_HasValueElem in ParseRDF.cpp).
#   In C++20 this is deprecated (a warning), but in C++26 (c++latest) it is
#   ill-formed (a hard error that -Wno flags cannot suppress). Capping at
#   C++20 keeps it as a suppressible warning. Future SDK versions will likely
#   fix this; revisit the cap when updating the submodule.
# - Wno-deprecated-enum-enum-conversion: suppresses the C++20 deprecation
#   warning for the enum-enum bitwise ops (defense-in-depth for -Werror)
xmp_warn_args = []
if cpp.has_argument('-Wno-deprecated-enum-enum-conversion')
  xmp_warn_args += '-Wno-deprecated-enum-enum-conversion'
endif

xmp_static = static_library('exiv2-xmp', xmp_sources,
  cpp_args: [xmp_public_args, xmp_private_args, xmp_warn_args],
  include_directories: xmp_inc,
  dependencies: expat_dep,
  override_options: ['warning_level=0', 'cpp_std=c++20'],
)

xmp_dep = declare_dependency(
  compile_args: xmp_public_args,
  include_directories: xmp_inc,
  link_with: xmp_static,
)
