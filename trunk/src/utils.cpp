// ********************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel. All rights reserved.
 * 
 * Todo: Insert license blabla here
 *
 */
/*
  Author(s): Andreas Huggel (ahu)
  History:
   08-Dec-03, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.2 $
 */

// *********************************************************************
// included header files
#include "utils.hpp"

// + standard includes
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>                     // for getopt(), stat()
#include <errno.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>

namespace Util {

// *********************************************************************
// class Getopt
int Getopt::getopt(int argc, char* const argv[], const std::string& optstring)
{
    progname_ = Util::basename(argv[0]);

    while (true) {
        int c = ::getopt(argc, argv, optstring.c_str());
        if (c == -1) break;
        errcnt_ += option(c, ::optarg == 0 ? "" : ::optarg, ::optopt);
    }
    for (int i = ::optind; i < argc; i++) {
        errcnt_ += nonoption(argv[i]);
    }
    return errcnt_; 
}

// *********************************************************************
// free functions

    bool fileExists(const std::string& path, bool ct)
    {
        struct stat buf;
        int ret = ::stat(path.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    }

    std::string strError()
    {
        int error = errno;
        std::ostringstream os;
        os << ::strerror(error) << " (errno = " << error << ")";
        return os.str();
    }

    std::string dirname(const std::string& path)
    {
        if (path == "") return ".";
        // Strip trailing slashes
        std::string p = path;
        while (p.length() > 1 && p[p.length()-1] == '/') {
            p = p.substr(0, p.length()-1);
        }
        if (p == "/") return "/";
        std::string::size_type idx = p.rfind('/');
        if (idx == std::string::npos) return ".";
        if (idx == 0) return "/";
        p = p.substr(0, idx);
        while (p.length() > 1 && p[p.length()-1] == '/') {
            p = p.substr(0, p.length()-1);
        }
        return p;
    }

    std::string basename(const std::string& path, bool delsuffix)
    {
        if (path == "") return ".";
        // Strip trailing slashes
        std::string p = path;
        while (p.length() > 1 && p[p.length()-1] == '/') {
            p = p.substr(0, p.length()-1);
        }
        if (p == "/") return p;
        std::string::size_type idx = p.rfind('/');
        if (idx != std::string::npos) p = p.substr(idx+1);
        if (delsuffix) p = p.substr(0, p.length() - suffix(p).length());
        return p;
    }

    std::string suffix(const std::string& path)
    {
        std::string b = basename(path);    
        std::string::size_type idx = b.rfind('.');
        if (idx == std::string::npos || idx == 0 || idx == b.length()-1) {
            return "";
        }
        return b.substr(idx);
    }

    bool strtol(const char* nptr, long& n)
    {
        if (!nptr || *nptr == '\0') return false;
        char* endptr = 0;
        long tmp = ::strtol(nptr, &endptr, 10);
        if (*endptr != '\0') return false;
        if (tmp == LONG_MAX || tmp == LONG_MIN) return false;
        n = tmp;
        return true;
    }

}                                       // namespace Util
