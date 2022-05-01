// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "futils.hpp"

#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "utils.hpp"

// + standard includes
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <stdexcept>

#ifdef EXV_HAVE_UNISTD_H
#include <unistd.h>  // for stat()
#endif

namespace fs = std::filesystem;

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <psapi.h>  // For access to GetModuleFileNameEx
// clang-format on
#endif

#if defined(__APPLE__) && defined(EXV_HAVE_LIBPROC_H)
#include <libproc.h>
#endif

#if defined(__FreeBSD__)
#include <libprocstat.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace Exiv2 {
constexpr std::array<const char*, 2> ENVARDEF{
    "/exiv2.php",
    "40",
};  //!< @brief default URL for http exiv2 handler and time-out
constexpr std::array<const char*, 2> ENVARKEY{
    "EXIV2_HTTP_POST",
    "EXIV2_TIMEOUT",
};  //!< @brief request keys for http exiv2 handler and time-out

// *****************************************************************************
// free functions
std::string getEnv(int env_var) {
  // this check is relying on undefined behavior and might not be effective
  if (env_var < envHTTPPOST || env_var > envTIMEOUT) {
    throw std::out_of_range("Unexpected env variable");
  }
  return getenv(ENVARKEY[env_var]) ? getenv(ENVARKEY[env_var]) : ENVARDEF[env_var];
}

/// @brief Convert an integer value to its hex character.
char to_hex(char code) {
  static const char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/// @brief Convert a hex character to its integer value.
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
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
  size_t idxIn{0}, idxOut{0};
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
static constexpr char base64_encode[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

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
  if (!in || input_length % 4 != 0)
    return result;

  auto encoding_table = reinterpret_cast<const unsigned char*>(base64_encode);
  unsigned char decoding_table[256];
  for (unsigned char i = 0; i < 64; i++)
    decoding_table[encoding_table[i]] = i;

  size_t output_length = input_length / 4 * 3;
  const auto buff = reinterpret_cast<const unsigned char*>(in);

  if (buff[input_length - 1] == '=')
    (output_length)--;
  if (buff[input_length - 2] == '=')
    (output_length)--;

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
  struct {
    std::string name;
    Protocol prot;
    bool isUrl;  // path.size() > name.size()
  } prots[] = {{"http://", pHttp, true}, {"https://", pHttps, true},  {"ftp://", pFtp, true},
               {"sftp://", pSftp, true}, {"file://", pFileUri, true}, {"data://", pDataUri, true},
               {"-", pStdin, false}};
  for (const auto& prot : prots) {
    if (result != pFile)
      break;

    if (Exiv2::Internal::startsWith(path, prot.name))
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
  return fs::exists(path);
}

std::string strError() {
  int error = errno;
  std::ostringstream os;
#ifdef EXV_HAVE_STRERROR_R
  const size_t n = 1024;
#ifdef EXV_STRERROR_R_CHAR_P
  char* buf = nullptr;
  char buf2[n] = {};
  buf = strerror_r(error, buf2, n);
#else
  char buf[n] = {};
  const int ret = strerror_r(error, buf, n);
  enforce(ret != ERANGE, Exiv2::ErrorCode::kerCallFailed);
#endif
  os << buf;
  // Issue# 908.
  // report strerror() if strerror_r() returns empty
  if (!buf[0]) {
    os << strerror(error);
  }
#else
  os << std::strerror(error);
#endif
  os << " (errno = " << error << ")";
  return os.str();
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

  if (!uri.length())
    return result;

  auto uriEnd = uri.end();

  // get query start
  auto queryStart = std::find(uri.begin(), uriEnd, '?');

  // protocol
  auto protocolStart = uri.begin();
  auto protocolEnd = std::find(protocolStart, uriEnd, ':');  //"://");

  if (protocolEnd != uriEnd) {
    auto prot = std::string(protocolEnd, uriEnd);
    if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      result.Protocol = std::string(protocolStart, protocolEnd);
      protocolEnd += 3;  //      ://
    } else
      protocolEnd = uri.begin();  // no protocol
  } else
    protocolEnd = uri.begin();  // no protocol

  // username & password
  auto authStart = protocolEnd;
  auto authEnd = std::find(protocolEnd, uriEnd, '@');
  if (authEnd != uriEnd) {
    auto userStart = authStart;
    auto userEnd = std::find(authStart, authEnd, ':');
    if (userEnd != authEnd) {
      result.Username = std::string(userStart, userEnd);
      ++userEnd;
      result.Password = std::string(userEnd, authEnd);
    } else {
      result.Username = std::string(authStart, authEnd);
    }
    ++authEnd;
  } else {
    authEnd = protocolEnd;
  }

  // host
  auto hostStart = authEnd;
  auto pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

  auto hostEnd = std::find(authEnd, (pathStart != uriEnd) ? pathStart : queryStart,
                           ':');  // check for port

  result.Host = std::string(hostStart, hostEnd);

  // port
  if ((hostEnd != uriEnd) && (*hostEnd == ':'))  // we have a port
  {
    ++hostEnd;
    auto portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
    result.Port = std::string(hostEnd, portEnd);
  }
  if (!result.Port.length() && result.Protocol == "http")
    result.Port = "80";

  // path
  if (pathStart != uriEnd)
    result.Path = std::string(pathStart, queryStart);

  // query
  if (queryStart != uriEnd)
    result.QueryString = std::string(queryStart, uri.end());

  return result;
}

std::string getProcessPath() {
  std::string ret("unknown");
#if defined(_WIN32)
  HANDLE processHandle = nullptr;
  processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
  if (processHandle) {
    TCHAR filename[MAX_PATH];
    if (GetModuleFileNameEx(processHandle, nullptr, filename, MAX_PATH) != 0) {
      ret = filename;
    }
    CloseHandle(processHandle);
  }
#elif defined(__APPLE__)
#ifdef EXV_HAVE_LIBPROC_H
  const int pid = getpid();
  char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
  if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0) {
    ret = pathbuf;
  }
#endif
#elif defined(__FreeBSD__)
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
#elif defined(__sun__)
  // https://stackoverflow.com/questions/47472762/on-solaris-how-to-get-the-full-path-of-executable-of-running-process-programatic
  const char* proc = Internal::stringFormat("/proc/%d/path/a.out", getpid()).c_str();
  char path[500];
  ssize_t l = readlink(proc, path, sizeof(path) - 1);
  if (l > 0) {
    path[l] = 0;
    ret = path;
  }
#elif defined(__unix__)
  ret = std::filesystem::read_symlink("/proc/self/exe");
#endif

  const size_t idxLastSeparator = ret.find_last_of(EXV_SEPARATOR_CHR);
  return ret.substr(0, idxLastSeparator);
}
}  // namespace Exiv2
