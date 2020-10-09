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
#include "enforce.hpp"

// + standard includes
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#ifdef   EXV_HAVE_UNISTD_H
#include <unistd.h>                     // for stat()
#endif

#if defined(_MSC_VER)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif

// platform specific support for getLoadedLibraries
#if defined(__CYGWIN__) || defined(__MINGW__) || defined(WIN32)
# include <windows.h>
# include <psapi.h>   // For access to GetModuleFileNameEx
# if __LP64__
#  ifdef  _WIN64
#   undef _WIN64
#  endif
#  define _WIN64 1
# endif
#elif defined(__APPLE__)
# include <mach-o/dyld.h>
# include <libproc.h>
#elif defined(__FreeBSD__)
# include <sys/param.h>
# include <sys/queue.h>
# include <sys/socket.h>
# include <sys/sysctl.h>
# include <libprocstat.h>
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
        auto buf = new char[strlen(str) * 3 + 1];
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
        auto buf = new char[(strlen(str) + 1)];
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

    int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize) {
        const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const auto data = static_cast<const uint8_t*>(data_buf);
        size_t resultIndex = 0;
        size_t x;
        size_t padCount = dataLength % 3;

        /* increment over the length of the string, three characters at a time */
        for (x = 0; x < dataLength; x += 3)
        {
            /* these three 8-bit (ASCII) characters become one 24-bit number */
            uint32_t n = data[x] << 16;

            if((x+1) < dataLength)
                n += data[x+1] << 8;

            if((x+2) < dataLength)
                n += data[x+2];

            /* this 24-bit number gets separated into four 6-bit numbers */
            uint8_t n0 = static_cast<uint8_t>(n >> 18) & 63;
            uint8_t n1 = static_cast<uint8_t>(n >> 12) & 63;
            uint8_t n2 = static_cast<uint8_t>(n >> 6) & 63;
            uint8_t n3 = static_cast<uint8_t>(n) & 63;

            /*
            * if we have one byte available, then its encoding is spread
            * out over two characters
            */
            if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n0];
            if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
            result[resultIndex++] = base64chars[n1];

            /*
            * if we have only two bytes available, then their encoding is
            * spread out over three chars
            */
            if((x+1) < dataLength)
            {
                if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
                result[resultIndex++] = base64chars[n2];
            }

            /*
            * if we have all three bytes available, then their encoding is spread
            * out over four characters
            */
            if((x+2) < dataLength)
            {
                if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
                result[resultIndex++] = base64chars[n3];
            }
        }

        /*
        * create and add padding that is required if we did not have a multiple of 3
        * number of characters available
        */
        if (padCount > 0)
        {
            for (; padCount < 3; padCount++)
            {
                if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
                result[resultIndex++] = '=';
            }
        }
        if(resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
        result[resultIndex] = 0;
        return 1;   /* indicate success */
    } // base64encode

    long base64decode(const char *in, char *out, size_t out_size) {
        static const char decode[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW"
                         "$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
        long i;
        long done = 0;
        unsigned char v;
        unsigned char quad[4];

        while (*in) {
            long len = 0;
            for (i = 0; i < 4 && *in; i++) {
                v = 0;
                while (*in && !v) {
                    v = *in++;
                    v = (v < 43 || v > 122) ? 0 : decode[v - 43];
                    if (v)
                        v = (v == '$') ? 0 : v - 61;
                    if (*in) {
                        len++;
                        if (v)
                            quad[i] = v - 1;
                    } else
                        quad[i] = 0;
                }
            }
            if (!len)
                continue;
            if (out_size < static_cast<size_t>(done + len - 1))
                /* out buffer is too small */
                return -1;
            if (len >= 2)
                *out++ = quad[0] << 2 | quad[1] >> 4;
            if (len >= 3)
                *out++ = quad[1] << 4 | quad[2] >> 2;
            if (len >= 4)
                *out++ = ((quad[2] << 6) & 0xc0) | quad[3];
            done += len - 1;
        }
        if (static_cast<size_t>(done + 1) >= out_size)
            return -1;
        *out++ = '\0';
        return done;
    } // base64decode

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
        for (const auto& prot : prots) {
            if ((result == pFile) && (path.find(prot.name) == 0))
                // URL's require data.  Stdin == "-" and no further data
                if (prot.isUrl ? path.size() > prot.name.size() : path.size() == prot.name.size())
                    result = prot.prot;
        }
        return result;
    } // fileProtocol
    /// \todo Remove code duplication
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
        for (const auto& prot : prots) {
            if ((result == pFile) && (path.find(prot.name) == 0))
                // URL's require data.  Stdin == "-" and no further data
                if (prot.isUrl ? path.size() > prot.name.size() : path.size() == prot.name.size())
                    result = prot.prot;
        }
        return result;
    } // fileProtocol
#endif
    bool fileExists(const std::string& path, bool ct)
    {
        // special case: accept "-" (means stdin)
        if (path == "-" || fileProtocol(path) != pFile) {
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
        return path.substr(found);
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
        char *buf = nullptr;
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

        using iterator_t = std::string::const_iterator;

        if ( !uri.length() )  return result;

        iterator_t uriEnd = uri.end();

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

        // get query start
        iterator_t queryStart = std::find(pathStart, uriEnd, '?');

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
        HANDLE processHandle = nullptr;
        processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
        if (processHandle != nullptr) {
            TCHAR filename[MAX_PATH];
            if (GetModuleFileNameEx(processHandle, nullptr, filename, MAX_PATH) != 0) {
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
    #elif defined(__linux__) || defined(__CYGWIN__) || defined(__MSYS__)
        // http://stackoverflow.com/questions/606041/how-do-i-get-the-path-of-a-process-in-unix-linux
        char proc[100];
        char path[500];
        sprintf(proc,"/proc/%d/exe", getpid());
        ssize_t l = readlink (proc, path,sizeof(path)-1);
        if (l>0) {
            path[l]=0;
            ret = path;
        }
    #endif

        const size_t idxLastSeparator = ret.find_last_of(EXV_SEPARATOR_STR);
        return ret.substr(0, idxLastSeparator);
    }

    static bool pushPath(std::string& path,std::vector<std::string>& libs,std::set<std::string> & paths)
    {
        bool result = Exiv2::fileExists(path,true) && paths.find(path) == paths.end() && path != "/" ;
        if ( result ) {
            paths.insert(path);
            libs.push_back(path);
        }
        return result ;
    }

    std::vector<std::string> getLoadedLibraries()
    {
        std::vector<std::string>  libs ;
        std::set<std::string>     paths;
        std::string               path ;

#if defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW__)
        // enumerate loaded libraries and determine path to executable
        std::array<HMODULE, 200> handles;
        DWORD cbNeeded;
        if (EnumProcessModules(GetCurrentProcess(), handles.data(), DWORD(handles.size()), &cbNeeded)) {
            std::array<char, _MAX_PATH> szFilename;
            for (const auto& h : handles) {
                GetModuleFileNameA(h, szFilename.data(), DWORD(szFilename.size()));
                std::string path(szFilename.data());
                pushPath(path,libs,paths);
            }
        }
#elif defined(__APPLE__)
        // man 3 dyld
        uint32_t count = _dyld_image_count();
        for (uint32_t image = 0 ; image < count ; image++ ) {
            std::string path(_dyld_get_image_name(image));
            pushPath(path,libs,paths);
        }
#elif defined(__FreeBSD__)
        unsigned int n;
        struct procstat*      procstat = procstat_open_sysctl();
        struct kinfo_proc*    procs    = procstat ? procstat_getprocs(procstat, KERN_PROC_PID, getpid(), &n) : NULL;
        struct filestat_list* files    = procs    ? procstat_getfiles(procstat, procs, true)                 : NULL;
        if ( files ) {
            filestat* entry;
            STAILQ_FOREACH(entry, files, next) {
                std::string path(entry->fs_path);
                pushPath(path,libs,paths);
            }
        }
        // free resources
        if ( files    ) procstat_freefiles(procstat, files);
        if ( procs    ) procstat_freeprocs(procstat, procs);
        if ( procstat ) procstat_close    (procstat);

#elif defined(__unix__)
        // read file /proc/self/maps which has a list of files in memory
        std::ifstream maps("/proc/self/maps",std::ifstream::in);
        std::string   string ;
        while ( std::getline(maps,string) ) {
            std::size_t pos = string.find_last_of(' ');
            if ( pos != std::string::npos ) {
                std::string path = string.substr(pos+1);
                pushPath(path,libs,paths);
            }
        }
#endif
        return libs;
    }
}                                       // namespace Exiv2
