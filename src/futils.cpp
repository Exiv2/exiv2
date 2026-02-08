// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "futils.hpp"

#include "config.h"
#include "enforce.hpp"
#include "image_int.hpp"

// + standard includes
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#ifdef EXV_ENABLE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <psapi.h>  // For access to GetModuleFileName
// clang-format on
#endif

#if __has_include(<unistd.h>)
#include <unistd.h>  // for getpid()
#endif

#if __has_include(<libproc.h>)
#include <libproc.h>
#endif

#if __has_include(<mach-o/dyld.h>)
#include <mach-o/dyld.h>  // for _NSGetExecutablePath()
#endif

#ifdef __FreeBSD__
// clang-format off
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <libprocstat.h>
// clang-format on
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#endif

namespace {
constexpr std::array ENVARDEF{
    "/exiv2.php",
    "40",
};  /// @brief default URL for http exiv2 handler and time-out
constexpr std::array ENVARKEY{
    "EXIV2_HTTP_POST",
    "EXIV2_TIMEOUT",
};  /// @brief request keys for http exiv2 handler and time-out

/// @brief Convert an integer value to its hex character.
char to_hex(char code) {
  static const char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/// @brief Convert a hex character to its integer value.
char from_hex(char ch) {
  return 0xF & (isdigit(ch) ? ch - '0' : static_cast<char>(tolower(ch)) - 'a' + 10);
}

constexpr char base64_encode[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};
}  // namespace

namespace Exiv2 {
// *****************************************************************************
// free functions
std::string getEnv(int env_var) {
  // this check is relying on undefined behavior and might not be effective
  if (env_var < envHTTPPOST || env_var > envTIMEOUT)
    throw std::out_of_range("Unexpected env variable");
#ifdef _WIN32
  char* buf = nullptr;
  size_t len;
  if (_dupenv_s(&buf, &len, ENVARKEY[env_var]) == 0 && buf) {
    auto ret = std::string(buf);
    free(buf);
    return ret;
  }
#else
  if (auto val = std::getenv(ENVARKEY[env_var]))
    return val;
#endif
  return ENVARDEF[env_var];
}

std::string urlencode(const std::string& str) {
  std::string encoded;
  encoded.reserve(str.size() * 3);
  for (uint8_t c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
      encoded += c;
    else if (c == ' ')
      encoded += '+';
    else {
      encoded += '%';
      encoded += to_hex(c >> 4);
      encoded += to_hex(c & 15);
    }
  }
  encoded.shrink_to_fit();
  return encoded;
}

void urldecode(std::string& str) {
  size_t idxIn{0};
  size_t idxOut{0};
  size_t sizeStr = str.size();
  while (idxIn < sizeStr) {
    if (str[idxIn] == '%') {
      if (str[idxIn + 1] && str[idxIn + 2]) {
        str[idxOut++] = from_hex(str[idxIn + 1]) << 4 | from_hex(str[idxIn + 2]);
        idxIn += 2;
      }
    } else if (str[idxIn] == '+') {
      str[idxOut++] = ' ';
    } else {
      str[idxOut++] = str[idxIn];
    }
    idxIn++;
  }
  str.erase(idxOut);
}

// https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize) {
  auto encoding_table = base64_encode;

  size_t output_length = 4 * ((dataLength + 2) / 3);
  int rc = result && data_buf && output_length < resultSize ? 1 : 0;
  if (rc) {
    const auto data = static_cast<const unsigned char*>(data_buf);
    for (size_t i = 0, j = 0; i < dataLength;) {
      uint32_t octet_a = data[i++];
      uint32_t octet_b = i < dataLength ? data[i++] : 0;
      uint32_t octet_c = i < dataLength ? data[i++] : 0;

      uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

      result[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
      result[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
      result[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
      result[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    const size_t mod_table[] = {0, 2, 1};
    for (size_t i = 0; i < mod_table[dataLength % 3]; i++)
      result[output_length - 1 - i] = '=';
    result[output_length] = 0;
  }
  return rc;
}  // base64encode

size_t base64decode(const char* in, char* out, size_t out_size) {
  size_t result = 0;
  size_t input_length = in ? ::strlen(in) : 0;
  if (!in || input_length % 4 != 0 || input_length == 0)
    return result;

  auto encoding_table = reinterpret_cast<const unsigned char*>(base64_encode);
  unsigned char decoding_table[256];
  for (unsigned char i = 0; i < 64; i++)
    decoding_table[encoding_table[i]] = i;

  size_t output_length = input_length / 4 * 3;
  const auto buff = reinterpret_cast<const unsigned char*>(in);

  if (buff[input_length - 1] == '=')
    output_length--;
  if (buff[input_length - 2] == '=')
    output_length--;

  if (output_length + 1 < out_size) {
    for (size_t i = 0, j = 0; i < input_length;) {
      uint32_t sextet_a = buff[i] == '=' ? 0 & i++ : decoding_table[buff[i++]];
      uint32_t sextet_b = buff[i] == '=' ? 0 & i++ : decoding_table[buff[i++]];
      uint32_t sextet_c = buff[i] == '=' ? 0 & i++ : decoding_table[buff[i++]];
      uint32_t sextet_d = buff[i] == '=' ? 0 & i++ : decoding_table[buff[i++]];

      uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

      if (j < output_length)
        out[j++] = (triple >> 2 * 8) & 0xFF;
      if (j < output_length)
        out[j++] = (triple >> 1 * 8) & 0xFF;
      if (j < output_length)
        out[j++] = (triple >> 0 * 8) & 0xFF;
    }
    out[output_length] = 0;
    result = output_length;
  }

  return result;
}

Protocol fileProtocol(const std::string& path) {
  Protocol result = pFile;
  constexpr struct {
    std::string_view name;
    Protocol prot;
    bool isUrl;  // path.size() > name.size()
  } prots[] = {
      {"http://", pHttp, true},    {"https://", pHttps, true},  {"ftp://", pFtp, true}, {"sftp://", pSftp, true},
      {"file://", pFileUri, true}, {"data://", pDataUri, true}, {"-", pStdin, false},
  };
  for (const auto& prot : prots) {
    if (result != pFile)
      break;

    if (path.starts_with(prot.name))
      // URL's require data.  Stdin == "-" and no further data
      if (prot.isUrl ? path.size() > prot.name.size() : path.size() == prot.name.size())
        result = prot.prot;
  }

  return result;
}  // fileProtocol

bool fileExists(const std::string& path) {
  if (fileProtocol(path) != pFile) {
    return true;
  }
#ifdef EXV_ENABLE_FILESYSTEM
  return fs::exists(path);
#else
  return false;
#endif
}

std::string strError() {
  int error = errno;
  std::string os;
#ifdef EXV_HAVE_STRERROR_R
  const size_t n = 1024;
#ifdef EXV_STRERROR_R_CHAR_P
  char buf2[n] = {};
  auto buf = strerror_r(error, buf2, n);
#else
  char buf[n] = {};
  const int ret = strerror_r(error, buf, n);
  Internal::enforce(ret != ERANGE, Exiv2::ErrorCode::kerCallFailed);
#endif
  os = buf;
  // Issue# 908.
  // report strerror() if strerror_r() returns empty
  if (os.empty()) {
    os = std::strerror(error);
  }
#elif defined(_WIN32)
  const size_t n = 1024;
  char buf[n] = {};
  const auto ret = strerror_s(buf, n, error);
  Internal::enforce(ret != ERANGE, Exiv2::ErrorCode::kerCallFailed);
  os = buf;
#else
  os = std::strerror(error);
#endif
  return stringFormat("{} (errno = {})", os, error);
}  // strError

void Uri::Decode(Uri& uri) {
  urldecode(uri.QueryString);
  urldecode(uri.Path);
  urldecode(uri.Host);
  urldecode(uri.Username);
  urldecode(uri.Password);
}

Uri Uri::Parse(const std::string& uri) {
  Uri result;

  if (uri.empty())
    return result;

  const auto uriEnd = uri.end();

  // protocol
  const auto protocolStart = uri.begin();
  auto protocolEnd = std::find(protocolStart, uriEnd, ':');  //"://");

  assert(protocolStart <= protocolEnd);

  if (protocolEnd != uriEnd) {
    auto prot = std::string(protocolEnd, uriEnd);
    if (prot.starts_with("://")) {
      result.Protocol = std::string(protocolStart, protocolEnd);
      protocolEnd += 3;  //      ://
    } else
      protocolEnd = protocolStart;  // no protocol
  } else
    protocolEnd = protocolStart;  // no protocol

  assert(protocolStart <= protocolEnd);

  // username & password
  const auto authStart = protocolEnd;

  auto authEnd = std::find(authStart, uriEnd, '@');
  assert(authStart <= authEnd);
  if (authEnd != uriEnd) {
    const auto userStart = authStart;
    if (auto userEnd = std::find(authStart, authEnd, ':'); userEnd != authEnd) {
      assert(authStart <= userEnd);
      assert(userEnd < authEnd);
      result.Username = std::string(userStart, userEnd);
      ++userEnd;
      assert(userEnd <= authEnd);
      result.Password = std::string(userEnd, authEnd);
    } else {
      result.Username = std::string(authStart, authEnd);
    }
    ++authEnd;
  } else {
    authEnd = protocolEnd;
  }

  assert(authStart <= authEnd);

  // host
  const auto hostStart = authEnd;
  const auto pathStart = std::find(hostStart, uriEnd, '/');   // get pathStart
  const auto queryStart = std::find(hostStart, uriEnd, '?');  // get query start

  assert(hostStart <= pathStart);
  assert(hostStart <= queryStart);

  const auto portEnd = (pathStart < uriEnd) ? pathStart : queryStart;
  auto hostEnd = std::find(hostStart, portEnd, ':');  // check for port

  assert(hostStart <= hostEnd);
  assert(hostEnd <= portEnd);

  if (hostStart < hostEnd) {
    result.Host = std::string(hostStart, hostEnd);
  }

  // port
  if ((hostEnd < uriEnd) && (*hostEnd == ':'))  // we have a port
  {
    ++hostEnd;
    if (hostEnd < portEnd) {
      result.Port = std::string(hostEnd, portEnd);
    }
  }
  if (result.Port.empty() && result.Protocol == "http")
    result.Port = "80";

  // path
  if (pathStart < queryStart) {
    result.Path = std::string(pathStart, queryStart);
  }

  // query
  if (queryStart < uriEnd)
    result.QueryString = std::string(queryStart, uriEnd);

  return result;
}

std::string getProcessPath() {
#ifdef EXV_ENABLE_FILESYSTEM
#ifdef __FreeBSD__
  std::string ret("unknown");
  unsigned int n;
  char buffer[PATH_MAX] = {};
  struct procstat* procstat = procstat_open_sysctl();
  struct kinfo_proc* procs = procstat ? procstat_getprocs(procstat, KERN_PROC_PID, getpid(), &n) : nullptr;
  if (procs) {
    procstat_getpathname(procstat, procs, buffer, PATH_MAX);
    ret = std::string(buffer);
  }
  // release resources
  if (procs)
    procstat_freeprocs(procstat, procs);
  if (procstat)
    procstat_close(procstat);

  const size_t idxLastSeparator = ret.find_last_of('/');
  return ret.substr(0, idxLastSeparator);
#else
  try {
#ifdef _WIN32
    TCHAR pathbuf[MAX_PATH];
    GetModuleFileName(nullptr, pathbuf, MAX_PATH);
    auto path = fs::path(pathbuf);
#elif defined(__APPLE__)
    char pathbuf[2048];
    uint32_t size = sizeof(pathbuf);
    const int get_exec_path_failure = _NSGetExecutablePath(pathbuf, &size);
    if (get_exec_path_failure)
      return "unknown";  // pathbuf not big enough
    auto path = fs::path(pathbuf);
#elif defined(__sun__)
    auto path = fs::read_symlink(stringFormat("/proc/{}/path/a.out", getpid()));
#elif defined(__unix__)
    auto path = fs::read_symlink("/proc/self/exe");
#endif
    return path.parent_path().string();
  } catch (const fs::filesystem_error&) {
    return "unknown";
  }
#endif
#else
  return "unknown";
#endif
}
}  // namespace Exiv2
