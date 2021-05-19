// ***************************************************************** -*- C++ -*-
// mmap-test.cpp
// Simple mmap tests
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

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <cstring>

using namespace Exiv2;

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    const char* path = argv[1];

    FileIo file(path);
    // Open the file in read mode
    if (file.open("rb") != 0) {
        throw Error(kerFileOpenFailed, path, "rb", strError());
    }
    // Map it to memory
    const Exiv2::byte* pData = file.mmap();
    long size = static_cast<long>(file.size());
    DataBuf buf(size);
    // Read from the memory mapped region
    memcpy(buf.pData_, pData, buf.size_);
    // Reopen file in write mode and write to it
    file.write(buf.pData_, buf.size_);
    // Read from the mapped region again
    memcpy(buf.pData_, pData, buf.size_);
    file.close();

    return 0;
}
catch (const AnyError& e) {
    std::cout << e << "\n";
}
