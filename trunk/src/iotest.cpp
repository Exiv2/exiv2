// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
 * 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  Abstract : Tester application for BasicIo functions. Tests MemIo primarily
        since FileIo just sits atop of FILE* streams.

  File     : iotest.cpp
  Version  : $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History  : 13-Jul-04, brad: created
 */
// *****************************************************************************
// included header files
#include "types.hpp"
#include "error.hpp"
#include "basicio.hpp"
#include <iostream>

using Exiv2::byte;
using Exiv2::BasicIo;
using Exiv2::MemIo;
using Exiv2::FileIo;
using Exiv2::IoCloser;

int WriteReadSeek(BasicIo &io);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " filein fileout1 fileout2\n";
        std::cout << "fileouts are overwritten and should match filein exactly\n";
        return 1;
    }

    FileIo fileIn(argv[1]);
    if (fileIn.open() != 0) {
        std::cerr << argv[0] << 
            ": Could not open input file (" << argv[1] << ")\n";
        return 1;
    }

    FileIo fileOut1(argv[2]);
    if (fileOut1.open("w+b") != 0) {
        std::cerr << argv[0] << 
            ": Could not open output file 1 (" << argv[2] << ")\n";
        return 1;
    }

    MemIo memIo1;

    // Copy to output file through memIo
    memIo1.write(fileIn);
    memIo1.seek(0, BasicIo::beg);
    fileOut1.write(memIo1);

    // Read writereadseek test on MemIo
    MemIo memIo2;
    int rc = WriteReadSeek(memIo2);
    if (rc != 0) return rc;

    // Read writereadseek test on FileIo
    // Create or overwrite the file, then close it
    FileIo fileTest("iotest.txt");
    if (fileTest.open("w+b") != 0) {
        std::cerr << argv[0] << 
            ": Could not create test file iotest.txt\n";
        return 1;
    }
    fileTest.close();
    rc = WriteReadSeek(fileTest);
    if (rc != 0) return rc;
    
    // Another test of reading and writing
    fileOut1.seek(0, BasicIo::beg);
    memIo2.seek(0, BasicIo::beg);
    FileIo fileOut2(argv[3]);
    if (fileOut2.open("w+b") != 0) {
        std::cerr << argv[0] << 
            ": Could not open output file 2 (" << argv[3] << ")\n";
        return 1;
    }

    long readCount = 0;
    byte buf[32];
    while ((readCount=fileOut1.read(buf, sizeof(buf)))) {
        if (memIo2.write(buf, readCount) != readCount) {
            std::cerr << argv[0] << 
                ": MemIo bad write 2\n";
            return 13;
        }
        if (fileOut2.write(buf, readCount) != readCount) {
            std::cerr << argv[0] << 
                ": FileIo bad write 2\n";
            return 14;
        }
    }

    return 0;
}
catch (Exiv2::Error& e) {
    std::cerr << "Caught Exiv2 exception '" << e << "'\n";
    return 20;
}
}


int WriteReadSeek(BasicIo &io)
{
    byte buf[4096];
    const char tester1[] = "this is a little test of MemIo";
    const char tester2[] = "Appending this on the end";
    const char expect[] = "this is a little teAppending this on the end";
    const long insert = 19;
    const long len1 = (long)strlen(tester1) + 1;
    const long len2 = (long)strlen(tester2) + 1;

    IoCloser closer(io);
    if (io.open() != 0) {
        std::cerr << ": WRS could not open IO\n";
        return 2;
    }

    if (io.write((byte*)tester1, len1) != len1) {
        std::cerr << ": WRS initial write failed\n";
        return 2;
    }
    io.seek(-len1, BasicIo::cur);

    int c = EOF;
    memset(buf, -1, sizeof(buf));
    for (int i = 0; (c=io.getb()) != EOF; ++i) {
        buf[i] = (byte)c;
    }

    // Make sure we got the null back
    if(buf[len1-1] != 0) {
        std::cerr << ": WRS missing null terminator 1\n";
        return 3;
    }

    if (strcmp(tester1, (char*)buf) != 0 ) {
        std::cerr << ": WRS strings don't match 1\n";
        return 4;
    }

    io.seek(-2, BasicIo::end);
    if (io.getb() != 'o') {
        std::cerr << ": WRS bad getb o\n";
        return 5;
    }
    
    io.seek(-2, BasicIo::cur);
    if (io.getb() != 'I') {
        std::cerr << ": WRS bad getb I\n";
        return 6;
    }

    if (io.putb('O') != 'O') {
        std::cerr << ": WRS bad putb\n";
        return 7;
    }
    
    io.seek(-1, BasicIo::cur);
    if (io.getb() != 'O') {
        std::cerr << ": WRS bad getb O\n";
        return 8;
    }

    io.seek(insert, BasicIo::beg);
    if(io.write((byte*)tester2, len2) != len2) {
        std::cerr << ": WRS bad write 1\n";
        return 9;
    }

    // open should seek to beginning
    io.open();
    memset(buf, -1, sizeof(buf));
    if (io.read(buf, sizeof(buf)) != insert + len2) {
        std::cerr << ": WRS something went wrong\n";
        return 10;
    }
    
    // Make sure we got the null back
    if(buf[insert + len2 - 1] != 0) {
        std::cerr << ": WRS missing null terminator 2\n";
        return 11;
    }

    if (strcmp(expect, (char*)buf) != 0 ) {
        std::cerr << ": WRS strings don't match 2\n";
        return 12;
    }

    return 0;
}

