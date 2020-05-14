// ***************************************************************** -*- C++ -*-
/*
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
/*
  Abstract : Tester application for BasicIo functions. Tests MemIo primarily
        since FileIo just sits atop of FILE* streams.

  File     : iotest.cpp
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History  : 04-Dec-04, brad: created
 */
// *****************************************************************************
// included header files
#include <exiv2/exiv2.hpp>

#include <cstdio>                               // for EOF
#include <cstring>
#include <iostream>
#include <stdio.h>

using Exiv2::byte;
using Exiv2::BasicIo;
using Exiv2::MemIo;
using Exiv2::FileIo;
using Exiv2::IoCloser;
using Exiv2::Error;
using Exiv2::strError;

int WriteReadSeek(BasicIo &io);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    try {
        if (argc < 4 || argc > 5 ) {
            std::cout << "Usage: " << argv[0] << " filein fileout1 fileout2 [remote]\n";
            std::cout << "fileouts are overwritten and should match filein exactly\n";
            return 1;
        }
        const char* f0 = argv[1];
        const char* f1 = argv[2];
        const char* f2 = argv[3];
        const char* fr = argv[4];

        if ( argc == 5 ) {
            // copy fileIn from a remote location.
            FILE* f = fopen(f0,"wb");
            if ( !f ) {
                Error(Exiv2::kerFileOpenFailed, f0, "w+b", strError());
            }
            BasicIo::AutoPtr io = Exiv2::ImageFactory::createIo(fr);
            io->open();
            int         l = 0 ;
            Exiv2::byte b[10000];
            int r ;
            while ( (r=io->read(b,sizeof(b))) > 0  ) {
                l += r;
                fwrite(b,r,1,f) ;
            }
            fclose(f);
            if ( !l ) {
                Error(Exiv2::kerFileOpenFailed, fr, "w+b", strError());
            }
        }
        
        FileIo fileIn(f0);
        if (fileIn.open() != 0) {
            throw Error(Exiv2::kerDataSourceOpenFailed, fileIn.path(), strError());
        }

        FileIo fileOut1(f1);
        if (fileOut1.open("w+b") != 0) {
            throw Error(Exiv2::kerFileOpenFailed, f1, "w+b", strError());
        }

        MemIo memIo1;

        // Copy to output file through memIo
        memIo1.write(fileIn);
        memIo1.seek(0, BasicIo::beg);
        fileOut1.write(memIo1);

        // Make sure they are all the same size
        if(fileIn.size() != memIo1.size() || memIo1.size() != fileOut1.size()) {
            std::cerr << argv[0] <<
                ": Sizes do not match\n";
            return 1;
        }

        // Read writereadseek test on MemIo
        MemIo memIo2;
        int rc = WriteReadSeek(memIo2);
        if (rc != 0) return rc;

        // Read writereadseek test on FileIo
        // Create or overwrite the file, then close it
        FileIo fileTest("iotest.txt");
        if (fileTest.open("w+b") != 0) {
            throw Error(Exiv2::kerFileOpenFailed, "iotest.txt", "w+b", strError());
        }

        fileTest.close();
        rc = WriteReadSeek(fileTest);
        if (rc != 0) return rc;

        // Another test of reading and writing
        fileOut1.seek(0, BasicIo::beg);
        memIo2.seek(0, BasicIo::beg);
        FileIo fileOut2(f2);
        if (fileOut2.open("w+b") != 0) {
            throw Error(Exiv2::kerFileOpenFailed, f2, "w+b", strError());
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
    } catch (Exiv2::AnyError& e) {
        std::cerr << "Caught Exiv2 exception '" << e << "'\n";
        return 20;
    }
}


int WriteReadSeek(BasicIo &io)
{
    byte buf[4096];
    const char   tester1[] = "this is a little test of MemIo";
    const char   tester2[] = "Appending this on the end";
    const char   expect[]  = "this is a little teAppending this on the end";
    const size_t insert = 19;
    const size_t size1  = std::strlen(tester1) + 1;
    const size_t size2  = std::strlen(tester2) + 1;

    if (io.open() != 0) {
        throw Error(Exiv2::kerDataSourceOpenFailed, io.path(), strError());
    }
    IoCloser closer(io);
    if ((size_t) io.write((byte*)tester1, (long)size1) != size1) {
        std::cerr << ": WRS initial write failed\n";
        return 2;
    }

    if (io.size() != size1) {
        std::cerr << ": WRS size is not " << size1 << "\n";
        return 2;
    }
    long     backup = (long)size1;
    io.seek(-backup, BasicIo::cur);

    int c = EOF;
    std::memset(buf, -1, sizeof(buf));
    for (int i = 0; (c=io.getb()) != EOF; ++i) {
        buf[i] = (byte)c;
    }

    // Make sure we got the null back
    if(buf[size1-1] != 0) {
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
    if((size_t)io.write((byte*)tester2, (long)size2) != size2) {
        std::cerr << ": WRS bad write 1\n";
        return 9;
    }

    // open should seek to beginning
    if (io.open() != 0)  {
        throw Error(Exiv2::kerDataSourceOpenFailed, io.path(), strError());
    }
    std::memset(buf, -1, sizeof(buf));
    if ((size_t) io.read(buf, sizeof(buf)) != insert + size2) {
        std::cerr << ": WRS something went wrong\n";
        return 10;
    }

    // Make sure we got the null back
    if(buf[insert + size2 - 1] != 0) {
        std::cerr << ": WRS missing null terminator 2\n";
        return 11;
    }

    if (std::strcmp(expect, (char*)buf) != 0 ) {
        std::cerr << ": WRS strings don't match 2\n";
        return 12;
    }

    return 0;
}
