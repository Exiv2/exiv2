// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "futils.hpp"
#include "datasets.hpp"
#include "enforce.hpp"
#include "image_int.hpp"

// + standard includes
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cerrno>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#ifdef   EXV_HAVE_UNISTD_H
#include <unistd.h>                     // for stat()
#endif

#if defined(WIN32)
#include <windows.h>
#include <psapi.h>  // For access to GetModuleFileNameEx
#endif

#if defined(_MSC_VER)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#elif defined(__APPLE__)
#include <libproc.h>
#endif

#if defined(__FreeBSD__)
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <libprocstat.h>
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

namespace Exiv2 {
    const char* ENVARDEF[] = {"/exiv2.php", "40"}; //!< @brief default URL for http exiv2 handler and time-out
    const char* ENVARKEY[] = {"EXIV2_HTTP_POST", "EXIV2_TIMEOUT"}; //!< @brief request keys for http exiv2 handler and time-out
// *****************************************************************************
// free functions
    std::string getEnv(int env_var)
    {
        // this check is relying on undefined behavior and might not be effective
        if (env_var < envHTTPPOST || env_var > envTIMEOUT) {
            throw std::out_of_range("Unexpected env variable");
        }
        return getenv(ENVARKEY[env_var]) ? getenv(ENVARKEY[env_var]) : ENVARDEF[env_var];
    }

    /// @brief Convert an integer value to its hex character.
    char to_hex(char code) {
        static char hex[] = "0123456789abcdef";
        return hex[code & 15];
    }

    /// @brief Convert a hex character to its integer value.
    char from_hex(char ch) {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
    }

    std::string urlencode(const char* str) {
        const char* pstr = str;
        // \todo try to use std::string for buf and avoid the creation of another string for just
        // returning the final value
        char* buf  = new char[strlen(str) * 3 + 1];
        char* pbuf = buf;
        while (*pstr) {
            if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
                *pbuf++ = *pstr;
            else if (*pstr == ' ')
                *pbuf++ = '+';
            else
                *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
            pstr++;
        }
        *pbuf = '\0';
        std::string ret(buf);
        delete [] buf;
        return ret;
    }

    char* urldecode(const char* str) {
        const char* pstr = str;
        char* buf  = new char [(strlen(str) + 1)];
        char* pbuf = buf;
        while (*pstr) {
            if (*pstr == '%') {
                if (pstr[1] && pstr[2]) {
                    *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
                    pstr += 2;
                }
            } else if (*pstr == '+') {
                *pbuf++ = ' ';
            } else {
                *pbuf++ = *pstr;
            }
            pstr++;
        }
        *pbuf = '\0';
        return buf;
    }

    void urldecode(std::string& str) {
        char* decodeStr = Exiv2::urldecode(str.c_str());
        str = std::string(decodeStr);
        delete [] decodeStr;
    }

    // https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
    static const std::string base64_chars =
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz"
                 "0123456789+/";

    static inline bool is_base64(unsigned char c) {
      return (isalnum(c) || (c == '+') || (c == '/'));
    }

    static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
      std::string ret;
      int i = 0;
      int j = 0;
      unsigned char char_array_3[3];
      unsigned char char_array_4[4];

      while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
          char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
          char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
          char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
          char_array_4[3] = char_array_3[2] & 0x3f;

          for(i = 0; (i <4) ; i++)
            ret += base64_chars[char_array_4[i]];
          i = 0;
        }
      }

      if (i) {
        for(j = i; j < 3; j++)
          char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
          ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
          ret += '=';
      }
      return ret;
    }

    static std::string base64_decode(std::string const& encoded_string) {
      int in_len = encoded_string.size();
      int i = 0;
      int j = 0;
      int in_ = 0;
      unsigned char char_array_4[4], char_array_3[3];
      std::string ret;

      while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
          for (i = 0; i <4; i++)
            char_array_4[i] = base64_chars.find(char_array_4[i]);

          char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
          char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
          char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

          for (i = 0; (i < 3); i++)
            ret += char_array_3[i];
          i = 0;
        }
      }

      if (i) {
        for (j = i; j <4; j++)
          char_array_4[j] = 0;

        for (j = 0; j <4; j++)
          char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
      }

      return ret;
    }

    int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize) {
        int rc = 0 ; // failed
        std::string encoded = base64_encode((unsigned char const*)data_buf, (unsigned int) dataLength) ;
        if (encoded.size() < resultSize ) {
            memcpy(result,encoded.c_str(),encoded.size());
            rc = 1 ; // indicate success
        }
        return rc;
    } // base64encode

    long base64decode(const char *in, char *out, size_t out_size)
    {
       std::string in_string(in);
       std::string out_string = base64_decode(in_string);
       long result = (long) out_string.size();
       if ( (long) out_size > result ) {
           memcpy(out,out_string.c_str(),result);
           out[result] = 0;
       } else result = -1 ;
       return result;
    }

    Protocol fileProtocol(const std::string& path) {
        Protocol result = pFile ;
        struct {
            std::string name ;
            Protocol    prot ;
            bool        isUrl; // path.size() > name.size()
        } prots[] =
        { { "http://"   ,pHttp     , true  }
        , { "https://"  ,pHttps    , true  }
        , { "ftp://"    ,pFtp      , true  }
        , { "sftp://"   ,pSftp     , true  }
        , { "ssh://"    ,pSsh      , true  }
        , { "file://"   ,pFileUri  , true  }
        , { "data://"   ,pDataUri  , true  }
        , { "-"         ,pStdin    , false }
        };
        for ( size_t i = 0 ; result == pFile && i < sizeof(prots)/sizeof(prots[0]) ; i ++ )
            if ( path.find(prots[i].name) == 0 )
                // URL's require data.  Stdin == "-" and no further data
                if ( prots[i].isUrl ? path.size() > prots[i].name.size() : path.size() == prots[i].name.size() )
                    result = prots[i].prot;

        return result;
    } // fileProtocol
#ifdef EXV_UNICODE_PATH
    Protocol fileProtocol(const std::wstring& path) {
        Protocol result = pFile ;
        struct {
            std::wstring  name ;
            Protocol      prot ;
            bool          isUrl; // path.size() > name.size()
        } prots[] =
        { { L"http://"   ,pHttp     , true  }
        , { L"https://"  ,pHttps    , true  }
        , { L"ftp://"    ,pFtp      , true  }
        , { L"sftp://"   ,pSftp     , true  }
        , { L"ssh://"    ,pSsh      , true  }
        , { L"file://"   ,pFileUri  , true  }
        , { L"data://"   ,pDataUri  , true  }
        , { L"-"         ,pStdin    , false }
        };
        for ( size_t i = 0 ; result == pFile && i < sizeof(prots)/sizeof(prots[0]) ; i ++ )
            if ( path.find(prots[i].name) == 0 )
                // URL's require data.  Stdin == "-" and no further data
                if ( prots[i].isUrl ? path.size() > prots[i].name.size() : path.size() == prots[i].name.size() )
                    result = prots[i].prot;

        return result;
    } // fileProtocol
#endif
    bool fileExists(const std::string& path, bool ct)
    {
        // special case: accept "-" (means stdin)
        if (path.compare("-") == 0 || fileProtocol(path) != pFile) {
            return true;
        }

        struct stat buf;
        int ret = ::stat(path.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

#ifdef EXV_UNICODE_PATH
    bool fileExists(const std::wstring& wpath, bool ct)
    {
        // special case: accept "-" (means stdin)
        if (wpath.compare(L"-") == 0 || fileProtocol(wpath) != pFile) {
            return true;
        }

        struct _stat buf;
        int ret = _wstat(wpath.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

#endif
    std::string pathOfFileUrl(const std::string& url) {
        std::string path = url.substr(7);
        size_t found = path.find('/');
        if (found == std::string::npos) return path;
        else return path.substr(found);
    }
#ifdef EXV_UNICODE_PATH
    std::wstring pathOfFileUrl(const std::wstring& wurl) {
        std::wstring path = wurl.substr(7);
        size_t found = path.find('/');
        if (found == std::wstring::npos) return path;
        else return path.substr(found);
    }
#endif

    std::string strError()
    {
        int error = errno;
        std::ostringstream os;
#ifdef EXV_HAVE_STRERROR_R
        const size_t n = 1024;
#ifdef EXV_STRERROR_R_CHAR_P
        char *buf = 0;
        char buf2[n];
        std::memset(buf2, 0x0, n);
        buf = strerror_r(error, buf2, n);
#else
        char buf[n];
        std::memset(buf, 0x0, n);
        const int ret = strerror_r(error, buf, n);
        enforce(ret != ERANGE, Exiv2::kerCallFailed);
#endif
        os << buf;
        // Issue# 908.
        // report strerror() if strerror_r() returns empty
        if ( !buf[0] ) {
            os << strerror(error);
        }
#else
        os << std::strerror(error);
#endif
        os << " (errno = " << error << ")";
        return os.str();
    } // strError

    void Uri::Decode(Uri& uri)
    {
        urldecode(uri.QueryString);
        urldecode(uri.Path);
        urldecode(uri.Host);
        urldecode(uri.Username);
        urldecode(uri.Password);
    }

    Uri Uri::Parse(const std::string &uri)
    {
        Uri result;

        typedef std::string::const_iterator iterator_t;

        if ( !uri.length() )  return result;

        iterator_t uriEnd = uri.end();

        // get query start
        iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

        // protocol
        iterator_t protocolStart = uri.begin();
        iterator_t protocolEnd   = std::find(protocolStart, uriEnd, ':');            //"://");

        if (protocolEnd != uriEnd)
        {
            std::string prot = &*(protocolEnd);
            if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
            {
                result.Protocol = std::string(protocolStart, protocolEnd);
                protocolEnd += 3;   //      ://
            }
            else
                protocolEnd = uri.begin();  // no protocol
        }
        else
            protocolEnd = uri.begin();  // no protocol

        //username & password
        iterator_t authStart = protocolEnd;
        iterator_t authEnd = std::find(protocolEnd, uriEnd, '@');
        if (authEnd != uriEnd) {
            iterator_t userStart = authStart;
            iterator_t userEnd   = std::find(authStart, authEnd, ':');
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
        iterator_t hostStart = authEnd;
        iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

        iterator_t hostEnd = std::find(authEnd,
            (pathStart != uriEnd) ? pathStart : queryStart,
            ':');  // check for port

        result.Host = std::string(hostStart, hostEnd);

        // port
        if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
        {
            ++hostEnd;
            iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
            result.Port = std::string(hostEnd, portEnd);
        }
        if ( !result.Port.length() && result.Protocol == "http" ) result.Port = "80";

        // path
        if (pathStart != uriEnd)
            result.Path = std::string(pathStart, queryStart);

        // query
        if (queryStart != uriEnd)
            result.QueryString = std::string(queryStart, uri.end());

        return result;
    }

    std::string getProcessPath()
    {
        std::string ret("unknown");
    #if defined(WIN32)
        HANDLE processHandle = NULL;
        processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
        if (processHandle != NULL) {
            TCHAR filename[MAX_PATH];
            if (GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH) != 0) {
                ret = filename;
            }
            CloseHandle(processHandle);
        }
    #elif defined(__APPLE__)
        const int pid = getpid();
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath (pid, pathbuf, sizeof(pathbuf)) > 0) {
            ret = pathbuf;
        }
    #elif defined(__FreeBSD__)
        unsigned int       n;
        char               buffer[PATH_MAX] = {};
        struct procstat*   procstat = procstat_open_sysctl();
        struct kinfo_proc* procs    = procstat ? procstat_getprocs(procstat, KERN_PROC_PID, getpid(), &n) : NULL;
        if ( procs ) {
            procstat_getpathname(procstat, procs, buffer, PATH_MAX);
            ret = std::string(buffer);
        }
        // release resources
        if ( procs    ) procstat_freeprocs(procstat, procs);
        if ( procstat ) procstat_close(procstat);
    #elif defined(__sun__)
        // https://stackoverflow.com/questions/47472762/on-solaris-how-to-get-the-full-path-of-executable-of-running-process-programatic        
        const char* proc = Internal::stringFormat("/proc/%d/path/a.out",getpid()).c_str();
        char        path[500];
        ssize_t     l = readlink (proc,path,sizeof(path)-1);
        if (l>0) {
            path[l]=0;
            ret = path;
        }
    #elif defined(__unix__)
        // http://stackoverflow.com/questions/606041/how-do-i-get-the-path-of-a-process-in-unix-linux
        char path[500];
        ssize_t l = readlink ("/proc/self/exe", path,sizeof(path)-1);
        if (l>0) {
            path[l]=0;
            ret = path;
        }
    #endif

        const size_t idxLastSeparator = ret.find_last_of(EXV_SEPARATOR_STR);
        return ret.substr(0, idxLastSeparator);
    }
}                                       // namespace Exiv2
