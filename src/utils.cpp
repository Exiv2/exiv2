// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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

#include "utils.hpp"


namespace Util {

// *****************************************************************************
// free functions

    std::string dirname(const std::string& path)
    {
        if (path.empty())
            return ".";
        // Strip trailing slashes or backslashes
        std::string p = path;
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        if (p == "\\" || p == "/") return p;
        if (p.length() == 2 && p[1] == ':') return p; // For Windows paths
        std::string::size_type idx = p.find_last_of("\\/");
        if (idx == std::string::npos) return ".";
        if (idx == 1 && p.at(0) == '\\' && p.at(1) == '\\') return p; // For Windows paths
        p = p.substr(0, idx == 0 ? 1 : idx);
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        return p;
    }

    std::string basename(const std::string& path, bool delsuffix)
    {
        if (path.empty())
            return ".";
        // Strip trailing slashes or backslashes
        std::string p = path;
        while (   p.length() > 1
               && (p[p.length()-1] == '\\' || p[p.length()-1] == '/')) {
            p = p.substr(0, p.length()-1);
        }
        if (p.length() == 2 && p[1] == ':') return ""; // For Windows paths
        std::string::size_type idx = p.find_last_of("\\/");
        if (idx == 1 && p.at(0) == '\\' && p.at(1) == '\\') return ""; // For Windows paths
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

}                                       // namespace Util
