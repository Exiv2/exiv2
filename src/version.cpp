// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#ifdef EXV_USE_CURL
#include <curl/curl.h>
#endif

#include "futils.hpp"
#include "makernote_int.hpp"
#include "version.hpp"

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
#include "xmp_exiv2.hpp"
#endif

// + standard includes
#include <array>
#include <fstream>
#include <set>

// #1147
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif

// platform specific support for getLoadedLibraries
#if defined(_WIN32) || defined(__CYGWIN__)
// clang-format off
#include <winapifamily.h>
#include <windows.h>
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_APP)
  #include <psapi.h>
#endif
// clang-format on
#if __LP64__
#ifdef _WIN64
#undef _WIN64
#endif
#define _WIN64 1
#endif
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__FreeBSD__)
// clang-format off
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <libprocstat.h>
// clang-format on
#elif defined(__sun__)
#include <dlfcn.h>
#include <link.h>
#endif

namespace Exiv2 {
uint32_t versionNumber() {
  return EXIV2_MAKE_VERSION(EXIV2_MAJOR_VERSION, EXIV2_MINOR_VERSION, EXIV2_PATCH_VERSION);
}

std::string versionString() {
  std::ostringstream os;
  os << EXIV2_MAJOR_VERSION << '.' << EXIV2_MINOR_VERSION << '.' << EXIV2_PATCH_VERSION;
  return os.str();
}

std::string versionNumberHexString() {
  std::ostringstream os;
  os << std::hex << std::setw(6) << std::setfill('0') << Exiv2::versionNumber();
  return os.str();
}

const char* version() {
  return EXV_PACKAGE_VERSION;
}

bool testVersion(uint32_t major, uint32_t minor, uint32_t patch) {
  return versionNumber() >= EXIV2_MAKE_VERSION(major, minor, patch);
}
}  // namespace Exiv2

static bool shouldOutput(const std::vector<std::regex>& greps, const char* key, const std::string& value) {
  bool bPrint = greps.empty();
  for (auto const& g : greps) {
    bPrint = std::regex_search(key, g) || std::regex_search(value, g);
    if (bPrint) {
      break;
    }
  }
  return bPrint;
}

static void output(std::ostream& os, const std::vector<std::regex>& greps, const char* name, const std::string& value) {
  if (shouldOutput(greps, name, value))
    os << name << "=" << value << '\n';
}

static void output(std::ostream& os, const std::vector<std::regex>& greps, const char* name, int value) {
  std::ostringstream stringStream;
  stringStream << value;
  output(os, greps, name, stringStream.str());
}

static bool pushPath(const std::string& path, std::vector<std::string>& libs, std::set<std::string>& paths) {
  bool result = Exiv2::fileExists(path) && paths.find(path) == paths.end() && path != "/";
  if (result) {
    paths.insert(path);
    libs.push_back(path);
  }
  return result;
}

static std::vector<std::string> getLoadedLibraries() {
  std::vector<std::string> libs;
  std::set<std::string> paths;
  std::string path;

#if defined(_WIN32) || defined(__CYGWIN__)
// enumerate loaded libraries and determine path to executable (unsupported on UWP)
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_APP)
  HMODULE handles[200];
  DWORD cbNeeded;
  if (EnumProcessModules(GetCurrentProcess(), handles, static_cast<DWORD>(std::size(handles)), &cbNeeded)) {
    char szFilename[_MAX_PATH];
    for (DWORD h = 0; h < cbNeeded / sizeof(handles[0]); h++) {
      GetModuleFileNameA(handles[h], szFilename, static_cast<DWORD>(std::size(szFilename)));
      pushPath(szFilename, libs, paths);
    }
  }
#endif
#elif defined(__APPLE__)
  // man 3 dyld
  uint32_t count = _dyld_image_count();
  for (uint32_t image = 0; image < count; image++) {
    std::string path(_dyld_get_image_name(image));
    pushPath(path, libs, paths);
  }
#elif defined(__FreeBSD__)
  {
    unsigned int n;
    struct procstat* procstat = procstat_open_sysctl();
    struct kinfo_proc* procs = procstat ? procstat_getprocs(procstat, KERN_PROC_PID, getpid(), &n) : nullptr;
    struct filestat_list* files = procs ? procstat_getfiles(procstat, procs, true) : nullptr;
    if (files) {
      filestat* entry;
      STAILQ_FOREACH(entry, files, next) {
        if (entry && PS_FST_TYPE_VNODE == entry->fs_type && entry->fs_path) {
          std::string path(entry->fs_path);
          pushPath(path, libs, paths);
        }
      }
    }
    // free resources
    if (files)
      procstat_freefiles(procstat, files);
    if (procs)
      procstat_freeprocs(procstat, procs);
    if (procstat)
      procstat_close(procstat);
  }
#elif defined(__sun__) || defined(__unix__)
  // http://stackoverflow.com/questions/606041/how-do-i-get-the-path-of-a-process-in-unix-linux
  char procsz[100];
  char pathsz[500];
  snprintf(procsz, sizeof(procsz), "/proc/%d/path/a.out", getpid());
  if (auto l = readlink(procsz, pathsz, sizeof(pathsz) - 1); l > 0) {
    pathsz[l] = '\0';
    path.assign(pathsz);
    libs.push_back(path);
  }

  // read file /proc/self/maps which has a list of files in memory
  // (this doesn't yield anything on __sun__)
  std::ifstream maps("/proc/self/maps");
  std::string string;
  while (std::getline(maps, string)) {
    std::size_t pos = string.find_last_of(' ');
    if (pos != std::string::npos) {
      path = string.substr(pos + 1);
      pushPath(path, libs, paths);
    }
  }
#endif
  if (libs.empty())
    libs.emplace_back("unknown");

  return libs;
}

void Exiv2::dumpLibraryInfo(std::ostream& os, const std::vector<std::regex>& keys) {
  int bits = 8 * sizeof(void*);
#ifdef NDEBUG
  int debug = 0;
#else
  int debug = 1;
#endif

#ifdef exiv2lib_EXPORTS
  int dll = 1;
#else
  int dll = 0;
#endif

  const char* compiler =
#if defined(_MSC_VER)
      "MSVC";

#ifndef __VERSION__
  char version[40];
  snprintf(version, sizeof(version), "%d.%02d", (_MSC_VER - 600) / 100, _MSC_VER % 100);

  // add edition in brackets
  // 7.10 = 2003 8.00 = 2005 etc 12.00 = 2013 13.00 = 2015 (yet the installer labels it as 14.0!)
  size_t edition = (_MSC_VER - 600) / 100;
  constexpr auto editions = std::array{
      "0", "1", "2", "3", "4", "5", "6", "2003", "2005", "2008", "2010", "2012", "2013", "2015", "2017", "2019",
  };
  if (edition == 13 && _MSC_VER >= 1910)
    edition++;  // 2017 _MSC_VAR  == 1910
  if (edition == 14 && _MSC_VER >= 1920)
    edition++;  // 2019 _MSC_VAR  == 1920

  if (edition > editions.size())
    edition = 0;
  if (edition) {
    const size_t len = ::strlen(version);
    snprintf(version + len, sizeof(version) - len, " (%s/%s)", editions[edition], bits == 64 ? "x64" : "x86");
  }
#define __VERSION__ version
#endif

#elif defined(__clang__)
      "Clang";
#elif defined(__GNUG__)
      "G++";
#elif defined(__GNUC__)
      "GCC";
#elif defined(__SUNPRO_CC)
      "CC (oracle)";
#elif defined(__SUNPRO_C)
      "cc (oracle)";
#elif defined(__sun__)
      "cc (solaris)";
#else
      "unknown";
#endif

#ifndef __VERSION__
#ifdef __clang__version__
#define __VERSION__ __clang__version__
#else
#define __VERSION__ "unknown"
#endif
#endif

  const char* platform =
#if defined(__MSYS__)
      "msys";
#elif defined(__CYGWIN__)
      "cygwin";
#elif defined(_MSC_VER)
      "windows";
#elif defined(__APPLE__)
      "apple";
#elif defined(__MINGW64__)
      "mingw64";
#elif defined(__MINGW32__)
      "mingw32";
#elif defined(__sun__)
      "solaris";
#elif defined(__NetBSD__)
      "netbsd";
#elif defined(__FreeBSD__)
          "freebsd";
#elif defined(__linux__)
          "linux";
#else
          "unknown";
#endif

  int have_inttypes = 0;
  int have_libintl = 0;
  int have_lensdata = 0;
  int have_iconv = 0;
  int have_memory = 0;
  int have_stdbool = 0;
  int have_stdint = 0;
  int have_stdlib = 0;
  int have_strlib = 0;
  int have_strerror_r = 0;
  int have_strings_h = 0;
  int have_mmap = 0;
  int have_munmap = 0;
  int have_sys_stat = 0;
  int have_unistd_h = 0;
  int have_sys_mman = 0;
  int have_libz = 0;
  int have_brotli = 0;
  int have_xmptoolkit = 0;
  int adobe_xmpsdk = 0;
  int have_bool = 0;
  int have_strings = 0;
  int have_sys_types = 0;
  int have_unistd = 0;

  int enable_bmff = 0;
  int enable_webready = 0;
  int enable_nls = 0;
  int enable_video = 0;
  int use_curl = 0;

#if __has_include(<inttypes.h>)
  have_inttypes = 1;
#endif

#ifdef EXV_HAVE_LENSDATA
  have_lensdata = 1;
#endif

#ifdef EXV_HAVE_ICONV
  have_iconv = 1;
#endif

#if __has_include(<libintl.h>)
  have_libintl = 1;
#endif

#if __has_include(<memory.h>)
  have_memory = 1;
#endif

#if __has_include(<stdbool.h>)
  have_stdbool = 1;
#endif

#if __has_include(<stdint.h>)
  have_stdint = 1;
#endif

#if __has_include(<stdlib.h>)
  have_stdlib = 1;
#endif

#ifdef EXV_HAVE_STRERROR_R
  have_strerror_r = 1;
#endif

#if __has_include(<strings.h>)
  have_strings = 1;
#endif

#if __has_include(<sys/mman.h>)
  have_mmap = 1;
#endif

#if __has_include(<sys/mman.h>)
  have_munmap = 1;
#endif

#if __has_include(<sys/stat.h>)
  have_sys_stat = 1;
#endif

#if __has_include(<sys/types.h>)
  have_sys_types = 1;
#endif

#if __has_include(<unistd.h>)
  have_unistd = 1;
#endif

#if __has_include(<sys/mman.h>)
  have_sys_mman = 1;
#endif

#ifdef EXV_HAVE_LIBZ
  have_libz = 1;
#endif

#ifdef EXV_HAVE_BROTLI
  have_brotli = 1;
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
  have_xmptoolkit = 1;
#endif

#ifdef EXV_ADOBE_XMPSDK
  adobe_xmpsdk = EXV_ADOBE_XMPSDK;
#endif

#ifdef EXV_ENABLE_BMFF
  enable_bmff = 1;
#endif

#ifdef EXV_ENABLE_WEBREADY
  enable_webready = 1;
#endif

#ifdef EXV_ENABLE_NLS
  enable_nls = 1;
#endif

#ifdef EXV_ENABLE_VIDEO
  enable_video = 1;
#endif

#ifdef EXV_USE_CURL
  use_curl = 1;
#endif

  std::vector<std::string> libs = getLoadedLibraries();

  output(os, keys, "exiv2", Exiv2::versionString());
  output(os, keys, "platform", platform);
  output(os, keys, "compiler", compiler);
  output(os, keys, "bits", bits);
  output(os, keys, "dll", dll);
  output(os, keys, "debug", debug);
  output(os, keys, "cplusplus", __cplusplus);
  output(os, keys, "version", __VERSION__);
  output(os, keys, "date", __DATE__);
  output(os, keys, "time", __TIME__);
  output(os, keys, "processpath", Exiv2::getProcessPath());
#ifdef EXV_ENABLE_NLS
  output(os, keys, "localedir", EXV_LOCALEDIR);
#endif
  output(os, keys, "package_name", EXV_PACKAGE_NAME);

#ifdef EXV_USE_CURL
  std::string curl_protocols;
  auto vinfo = curl_version_info(CURLVERSION_NOW);
  for (int i = 0; vinfo->protocols[i]; i++) {
    curl_protocols += vinfo->protocols[i];
    curl_protocols += " ";
  }
  output(os, keys, "curlprotocols", curl_protocols);
#endif

  output(os, keys, "curl", use_curl);
  if (libs.begin() != libs.end()) {
    output(os, keys, "executable", *libs.begin());
    for (auto lib = libs.begin() + 1; lib != libs.end(); ++lib)
      output(os, keys, "library", *lib);
  }

  output(os, keys, "have_inttypes", have_inttypes);
  output(os, keys, "have_libintl", have_libintl);
  output(os, keys, "have_lensdata", have_lensdata);
  output(os, keys, "have_iconv", have_iconv);
  output(os, keys, "have_memory", have_memory);
  output(os, keys, "have_stdbool", have_stdbool);
  output(os, keys, "have_stdint", have_stdint);
  output(os, keys, "have_stdlib", have_stdlib);
  output(os, keys, "have_strlib", have_strlib);
  output(os, keys, "have_strerror_r", have_strerror_r);
  output(os, keys, "have_strings_h", have_strings_h);
  output(os, keys, "have_mmap", have_mmap);
  output(os, keys, "have_munmap", have_munmap);
  output(os, keys, "have_sys_stat", have_sys_stat);
  output(os, keys, "have_unistd_h", have_unistd_h);
  output(os, keys, "have_sys_mman", have_sys_mman);
  output(os, keys, "have_libz", have_libz);
  output(os, keys, "have_brotli", have_brotli);
  output(os, keys, "have_xmptoolkit", have_xmptoolkit);
  output(os, keys, "adobe_xmpsdk", adobe_xmpsdk);
  output(os, keys, "have_bool", have_bool);
  output(os, keys, "have_strings", have_strings);
  output(os, keys, "have_sys_types", have_sys_types);
  output(os, keys, "have_unistd", have_unistd);
  output(os, keys, "enable_bmff", enable_bmff);
  output(os, keys, "enable_webready", enable_webready);
  output(os, keys, "enable_nls", enable_nls);
  output(os, keys, "enable_video", enable_video);
  output(os, keys, "use_curl", use_curl);

  output(os, keys, "config_path", Exiv2::Internal::getExiv2ConfigPath());

// #1147
#ifndef _WIN32
  uid_t uid = getuid();
  output(os, keys, "uid", uid);
  uid_t euid = geteuid();
  output(os, keys, "euid", euid);
  uid_t gid = getgid();
  output(os, keys, "gid", gid);
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
  const char* name = "xmlns";

  Exiv2::Dictionary ns;
  Exiv2::XmpProperties::registeredNamespaces(ns);
  for (const auto& [xmlns, uri] : ns) {
    output(os, keys, name, xmlns + ":" + uri);
  }
#endif
}
