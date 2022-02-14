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

#include <filesystem>

namespace fs = std::filesystem;

namespace Util {

// *****************************************************************************
// free functions

    std::string dirname(const std::string& path)
    {
        auto p = fs::path(path).parent_path();
        if (p.empty())
            return ".";
        return p.string();
    }

    std::string basename(const std::string& path, bool delsuffix)
    {
        auto p = fs::path(path);
        if (delsuffix)
            return p.stem().string();
        return p.filename().string();
    }

    std::string suffix(const std::string& path)
    {
        return fs::path(path).extension().string();
    }

}                                       // namespace Util
