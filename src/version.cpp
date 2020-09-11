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

#include "config.h"

#ifdef EXV_USE_CURL
#include <curl/curl.h>
#endif

#include "http.hpp"
#include "version.hpp"
#include "makernote_int.hpp"
#include "futils.hpp"

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
#include "xmp_exiv2.hpp"
#endif

// + standard includes
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#if ! defined(WIN32) && ! defined(__CYGWIN__) && ! defined(__MINGW__)
#include <unistd.h>
#include <sys/types.h>
#endif

std::string Exiv2::versionString()
{
    std::ostringstream os;
    os << EXIV2_MAJOR_VERSION << '.' << EXIV2_MINOR_VERSION << '.' << EXIV2_PATCH_VERSION;
    return os.str();
}

std::string Exiv2::versionNumberHexString()
{
    std::ostringstream os;
    os << std::hex << std::setw(6) << std::setfill('0') << Exiv2::versionNumber();
    return os.str();
}

static bool shouldOutput(const exv_grep_keys_t& greps,const char* key,const std::string& value)
{
    bool bPrint = greps.empty();
    for (auto g = greps.begin(); !bPrint && g != greps.end(); ++g) {
        re::smatch m;
        const std::string Key(key);

        bPrint = (  re::regex_search( Key, m, *g)
                 || re::regex_search( value, m, *g)
                 );
    }
    return bPrint;
}

static void output(std::ostream& os,const exv_grep_keys_t& greps,const char* name,const std::string& value)
{
    if ( shouldOutput(greps,name,value) ) os << name << "=" << value << std::endl;
}

static void output(std::ostream& os,const exv_grep_keys_t& greps,const char* name,int value)
{
    std::ostringstream stringStream;
    stringStream << value;
    output(os,greps,name,stringStream.str());
}

void Exiv2::dumpLibraryInfo(std::ostream& os,const exv_grep_keys_t& keys)
{
    constexpr int bits = 8 * sizeof(void*);
#ifdef NDEBUG
    constexpr int debug = 0;
#else
    constexpr int debug = 1;
#endif

#ifdef exiv2lib_EXPORTS
    constexpr int dll = 1;
#else
    constexpr int dll = 0;
#endif

    constexpr const char* compiler =
#if defined(_MSC_VER)
    "MSVC"    ;

#ifndef __VERSION__
    char  version[40];
#if _MSC_VER == 1900
    sprintf(version, "14.00 (2015/%s)", bits == 64 ? "x64" : "x86");
#elif _MSC_VER >= 1910 && _MSC_VER < 1920
    sprintf(version, "14.%02d (2017/%s)", _MSC_VER % 100, bits == 64 ? "x64" : "x86");
#elif _MSC_VER >= 1920
    sprintf(version, "14.%02d (2019/%s)", _MSC_VER % 100, bits == 64 ? "x64" : "x86");
#else
    version[0] = '\0';
#endif
#define __VERSION__ version
#endif

#elif defined(__clang__)
    "Clang"   ;
#elif defined(__GNUG__)
    "G++"     ;
#elif defined(__GNUC__)
    "GCC"     ;
#elif defined(__SUNPRO_CC)
    "CC (oracle)";
#elif defined (__SUNPRO_C)
    "cc (oracle)";
#else
    "unknown" ;
#endif

#if defined(__SUNPRO_CC) || defined (__SUNPRO_C)
#define __oracle__
#endif

#ifndef __VERSION__
#ifdef  __clang__version__
#define __VERSION__ __clang__version__
#else
#define __VERSION__ "unknown"
#endif
#endif

    constexpr const char* platform =
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
#elif defined(__FreeBSD__)
    "freebsd";
#elif defined(__NetBSD__)
    "netbsd";
#elif defined(__linux__)
    "linux";
#else
    "unknown";
#endif

    constexpr int have_strings_h = 0;

#ifdef EXV_HAVE_GMTIME_R
    constexpr int have_gmtime_r = 1;
#else
    constexpr int have_gmtime_r = 0;
#endif

#ifdef EXV_HAVE_LIBINTL_H
    constexpr int have_libintl = 1;
#else
    constexpr int have_libintl = 0;
#endif

#ifdef EXV_HAVE_LENSDATA
    constexpr int have_lensdata = 1;
#else
    constexpr int have_lensdata = 0;
#endif

#ifdef EXV_HAVE_ICONV
    constexpr int have_iconv = 1;
#else
    constexpr int have_iconv = 0;
#endif

#ifdef EXV_HAVE_LSTAT
    constexpr int have_lstat = 1;
#else
    constexpr int have_lstat = 0;
#endif

#ifdef EXV_HAVE_STRERROR_R
    constexpr int have_strerror_r = 1;
#else
    constexpr int have_strerror_r = 0;
#endif

#ifdef EXV_HAVE_MMAP
    constexpr int have_mmap = 1;
#else
    constexpr int have_mmap = 0;
#endif

#ifdef EXV_HAVE_MUNMAP
    constexpr int have_munmap = 1;
#else
    constexpr int have_munmap = 0;
#endif

#ifdef EXV_HAVE_UNISTD_H
    constexpr int have_unistd_h = 1;
#else
    constexpr int have_unistd_h = 0;
#endif

#ifdef EXV_HAVE_SYS_MMAN_H
    constexpr int have_sys_mman = 1;
#else
    constexpr int have_sys_mman = 0;
#endif

#ifdef EXIV2_ENABLE_PNG
    constexpr int have_libz = 1;
#else
    constexpr int have_libz = 0;
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
    constexpr int have_xmptoolkit = 1;
#else
    constexpr int have_xmptoolkit = 0;
#endif

#ifdef EXV_ADOBE_XMPSDK
    constexpr int adobe_xmpsdk = EXV_ADOBE_XMPSDK;
#else
    constexpr int adobe_xmpsdk = 0;
#endif

#ifdef EXV_UNICODE_PATH
    constexpr int have_unicode_path = 1;
#else
    constexpr int have_unicode_path = 0;
#endif

#ifdef EXV_ENABLE_WEBREADY
    constexpr int enable_webready = 1;
#else
    constexpr int enable_webready = 0;
#endif

#ifdef EXV_ENABLE_NLS
    constexpr int enable_nls = 1;
#else
    constexpr int enable_nls = 0;
#endif

#ifdef EXV_USE_CURL
    constexpr int use_curl = 1;
#else
    constexpr int use_curl = 0;
#endif

    std::vector<std::string> libs =Exiv2::getLoadedLibraries(); // libs[0] == executable

    output(os,keys,"exiv2",Exiv2::versionString());
    output(os,keys,"platform"       , platform   );
    output(os,keys,"compiler"       , compiler   );
    output(os,keys,"bits"           , bits       );
    output(os,keys,"dll"            , dll        );
    output(os,keys,"debug"          , debug      );
    output(os,keys,"cplusplus"      , __cplusplus);
    output(os,keys,"version"        , __VERSION__);
    output(os,keys,"date"           , __DATE__   );
    output(os,keys,"time"           , __TIME__   );
    output(os,keys,"processpath"    , Exiv2::getProcessPath());
#ifdef EXV_ENABLE_NLS
    output(os,keys,"localedir"      , EXV_LOCALEDIR);
#endif
    output(os,keys,"package_name"   , EXV_PACKAGE_NAME);

#ifdef EXV_USE_CURL
    std::string curl_protocols;
    curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);
    for (int i = 0; vinfo->protocols[i]; i++) {
        curl_protocols += vinfo->protocols[i];
        curl_protocols += " " ;
    }
    output(os,keys,"curlprotocols" ,curl_protocols);
#endif

    output(os,keys,"curl"          , use_curl);
    if ( libs.begin() != libs.end() ) {
        output(os,keys,"executable" ,*libs.begin());
        for (auto lib = libs.begin() + 1; lib != libs.end(); ++lib)
            output(os,keys,"library",*lib);
    }

    output(os,keys,"have_strerror_r"   ,have_strerror_r  );
    output(os,keys,"have_gmtime_r"     ,have_gmtime_r    );
    output(os,keys,"have_libintl"      ,have_libintl     );
    output(os,keys,"have_lensdata"     ,have_lensdata    );
    output(os,keys,"have_iconv"        ,have_iconv       );
    output(os,keys,"have_lstat"        ,have_lstat       );
    output(os,keys,"have_strerror_r"   ,have_strerror_r  );
    output(os,keys,"have_strings_h"    ,have_strings_h   );
    output(os,keys,"have_mmap"         ,have_mmap        );
    output(os,keys,"have_munmap"       ,have_munmap      );
    output(os,keys,"have_unistd_h"     ,have_unistd_h    );
    output(os,keys,"have_sys_mman"     ,have_sys_mman    );
    output(os,keys,"have_libz"         ,have_libz        );
    output(os,keys,"have_xmptoolkit"   ,have_xmptoolkit  );
    output(os,keys,"adobe_xmpsdk"      ,adobe_xmpsdk     );
    output(os,keys,"have_unicode_path" ,have_unicode_path);
    output(os,keys,"enable_webready"   ,enable_webready  );
    output(os,keys,"enable_nls"        ,enable_nls       );
    output(os,keys,"use_curl"          ,use_curl         );

    output(os,keys,"config_path"       ,Exiv2::Internal::getExiv2ConfigPath());

#if ! defined(WIN32) && ! defined(__CYGWIN__) && ! defined(__MINGW__)
    uid_t uid  = getuid();
    uid_t euid = geteuid();
    uid_t gid  = getgid();

    output(os,keys,"uid" ,  uid  );
    output(os,keys,"euid", euid  );
    output(os,keys,"gid" ,  gid  );
#endif

#ifdef EXV_HAVE_XMP_TOOLKIT
    constexpr const char* name = "xmlns";

    Exiv2::Dictionary ns;
    Exiv2::XmpProperties::registeredNamespaces(ns);
    for (const auto& dict : ns) {
        std::string xmlns = dict.first;
        std::string uri = dict.second;
        output(os, keys, name, xmlns.append(":").append(uri));
    }
#endif
}
