// ***************************************************************** -*- C++ -*-
/*
  Abstract : Simple test program to test class Ifd.

  File     : ifd-test.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu)
  History  : 15-Feb-05, ahu
 */
// *****************************************************************************
// included header files
#include "ifd.hpp"
#include "error.hpp"

#include <iostream>
#include <string>
#include <cstring>

int main()
try {
    // -------------------------------------------------------------------------
    std::cout << "Read standard Ifd from data buffer\n";

    const long len = 77;
    Exiv2::byte buf[]
        = { 0xff, // Filler
            // No
            0x00,0x04,
            // Tag       Type          Components          Offset/Data
            0x00,0x01, 0x00,0x02, 0x00,0x00,0x00,0x04, 'T', 'h', 'e', '\0',
            0x00,0x02, 0x00,0x02, 0x00,0x00,0x00,0x06, 0x00,0x00,0x00,0x37,
            0x00,0x03, 0x00,0x02, 0x00,0x00,0x00,0x07, 0x00,0x00,0x00,0x3d,
            0x00,0x04, 0x00,0x02, 0x00,0x00,0x00,0x09, 0x00,0x00,0x00,0x44,
            // Next
            0x00,0x00,0x00,0x00,
            // Data
            'K', 'u', 'a', 'l', 'a', '\0',
            'L', 'u', 'm', 'p', 'u', 'r', '\0',
            'M', 'a', 'l', 'a', 'y', 's', 'i', 'a', '\0'
        };

    Exiv2::Ifd ifd(Exiv2::ifd0Id, 0, false);
    int rc = ifd.read(buf, len, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (1) failed, rc = " << rc << "\n";
        return rc;
    }
    ifd.print(std::cout);

    Exiv2::Ifd::iterator pos = ifd.findTag(0x0004);
    if (pos == ifd.end()) {
        std::cout << "Tag not found!\n";
        return 1;
    }
    Exiv2::byte data[] = { 'T', 'H', 'R', 'E', 'E', '\0' };

    std::cout << "Setting value of entry 3...\n";
    pos->setValue(2, 6, data, 6);

    Exiv2::DataBuf db(1024);
    rc = ifd.copy(db.pData_ + 1, Exiv2::bigEndian);
    std::cout << "Wrote " << rc << " characters to data buffer\n";
    rc = ifd.read(db.pData_, len, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (1a) failed, rc = " << rc << "\n";
        return rc;
    }
    ifd.print(std::cout);

    // -------------------------------------------------------------------------
    std::cout << "\nRead non-standard Ifd from data buffer\n";

    const long len2 = 76;
    Exiv2::byte buf2[]
        = { // Data
            'K', 'u', 'a', 'l', 'a', '\0',
            'L', 'u', 'm', 'p', 'u', 'r', '\0',
            'M', 'a', 'l', 'a', 'y', 's', 'i', 'a', '\0',
            // No
            0x00,0x04,
            // Tag       Type          Components          Offset/Data
            0x00,0x01, 0x00,0x02, 0x00,0x00,0x00,0x04, 'T', 'h', 'e', '\0',
            0x00,0x02, 0x00,0x02, 0x00,0x00,0x00,0x06, 0x00,0x00,0x00,0x00,
            0x00,0x03, 0x00,0x02, 0x00,0x00,0x00,0x07, 0x00,0x00,0x00,0x06,
            0x00,0x04, 0x00,0x02, 0x00,0x00,0x00,0x09, 0x00,0x00,0x00,0x0d,
            // Next
            0x00,0x00,0x00,0x00
        };

    Exiv2::Ifd ifd2(Exiv2::ifd0Id, 0, false);
    rc = ifd2.read(buf2, len2, 22, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (2) failed, rc = " << rc << "\n";
        return rc;
    }
    ifd2.print(std::cout);

    pos = ifd2.findTag(0x0004);
    if (pos == ifd2.end()) {
        std::cout << "Tag not found!\n";
        return 1;
    }
    Exiv2::byte data2[] = { 'T', 'H', 'R', 'E', 'E', '\0' };

    std::cout << "Setting value of entry 3...\n";
    pos->setValue(2, 6, data2, 6);

    ifd2.print(std::cout);

    // -------------------------------------------------------------------------
    std::cout << "\nTest boundary checks, the following reads should generate warnings or errors\n";

    std::cout << "--- read (3)" << std::endl;
    rc = ifd.read(buf, len-1, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (3) failed, rc = " << rc << "\n";
    }

    std::cout << "--- read (4)" << std::endl;
    rc = ifd.read(buf, len-17, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (4) failed, rc = " << rc << "\n";
    }

    std::cout << "--- read (5)" << std::endl;
    rc = ifd.read(buf, len-16, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (5) failed, rc = " << rc << "\n";
    }

    std::cout << "--- read (6)" << std::endl;
    rc = ifd.read(buf, len-23, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (6) failed, rc = " << rc << "\n";
    }

    std::cout << "--- read (7)" << std::endl;
    rc = ifd2.read(buf2, len2-1, 22, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (7) failed, rc = " << rc << "\n";
    }

    // -------------------------------------------------------------------------
    std::cout << "\nCreate Ifd from scratch\n";

    Exiv2::Ifd ifd3(Exiv2::ifd0Id);
    Exiv2::Entry e;
    e.setIfdId(Exiv2::ifd0Id);
    e.setIdx(0);

    e.setTag(0x0001);
    e.setOffset(0);  // will be calculated when the IFD is written
    Exiv2::byte data0x01[] = { 'T', 'h', 'e', '\0' };
    e.setValue(2, 4, data0x01, 4);
    ifd3.add(e);

    e.setTag(0x0002);
    e.setOffset(0);  // will be calculated when the IFD is written
    Exiv2::byte data0x02[] = { 'K', 'u', 'a', 'l', 'a', '\0' };
    e.setValue(2, 6, data0x02, 6);
    ifd3.add(e);

    e.setTag(0x0003);
    e.setOffset(0);  // will be calculated when the IFD is written
    Exiv2::byte data0x03[] = { 'L', 'u', 'm', 'p', 'u', 'r', '\0' };
    e.setValue(2, 7, data0x03, 7);
    ifd3.add(e);

    e.setTag(0x0004);
    e.setOffset(0);  // will be calculated when the IFD is written
    Exiv2::byte data0x04[] = { 'M', 'a', 'l', 'a', 'y', 's', 'i', 'a', '\0' };
    e.setValue(2, 9, data0x04, 9);
    ifd3.add(e);

    Exiv2::DataBuf ibuf(256);
    long len3 = ifd3.copy(ibuf.pData_, Exiv2::bigEndian);

    Exiv2::Ifd ifd4(Exiv2::ifd0Id, 0, false);
    rc = ifd4.read(ibuf.pData_, len3, 0, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (8) failed, rc = " << rc << "\n";
    }
    ifd4.print(std::cout);

    // -------------------------------------------------------------------------
    std::cout << "\nMove data buffer\n";

    Exiv2::Ifd ifd5(Exiv2::ifd0Id, 0, false);
    rc = ifd5.read(buf, len, 1, Exiv2::bigEndian);
    if (rc) {
        std::cout << "Ifd::read (1) failed, rc = " << rc << "\n";
        return rc;
    }
    ifd5.print(std::cout);

    Exiv2::byte* newBuf = new Exiv2::byte[len];
    std::memset(newBuf, 0x00, len);
    std::memcpy(newBuf, buf, len);
    std::memset(buf, 0x0, len);
    ifd5.updateBase(newBuf);
    ifd5.print(std::cout);
    delete[] newBuf;

    return 0;
}
catch (const Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return 1;
}
