// ***************************************************************** -*- C++ -*-
/*
  Abstract : Key unit tests 

  File     : key-test.cpp
  Version  : $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 24-Aug-04, ahu: created

 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <string>

using namespace Exiv2;

int main()
{
    int tc = 0;
    int rc = 0;

    std::string key("Exif.Iop.InteroperabilityVersion");
    ExifKey ek(key);

    // operator<<
    tc += 1;
    std::ostringstream os;
    os << ek;
    if (os.str() != key) {
        std::cout << "Testcase failed (operator<<)" << std::endl;
        rc += 1; 
    }
    // familyName
    tc += 1;
    if (std::string(ek.familyName()) != "Exif") {
        std::cout << "Testcase failed (familyName)" << std::endl;
        rc += 1; 
    }
    // groupName
    tc += 1;
    if (ek.groupName() != "Iop") {
        std::cout << "Testcase failed (groupName)" << std::endl;
        rc += 1; 
    }
    // tagName
    tc += 1;
    if (ek.tagName() != "InteroperabilityVersion") {
        std::cout << "Testcase failed (tagName)" << std::endl;
        rc += 1; 
    }
    // tagName
    tc += 1;
    if (ek.tag() != 0x0002) {
        std::cout << "Testcase failed (tag)" << std::endl;
        rc += 1; 
    }
    // ifdName
    tc += 1;
    if (std::string(ek.ifdName()) != "Iop") {
        std::cout << "Testcase failed (ifdName: " << std::endl;
        rc += 1; 
    }
    // sectionName
    tc += 1;
    if (ek.sectionName() != "Interoperability") {
        std::cout << "Testcase failed (sectionName)" << std::endl;
        rc += 1; 
    }

    // -----

    // Copy constructor
    ExifKey ek2(ek);

    // operator<<
    tc += 1;
    std::ostringstream os2;
    os2 << ek2;
    if (os2.str() != key) {
        std::cout << "Testcase failed (operator<<)" << std::endl;
        rc += 1; 
    }
    // familyName
    tc += 1;
    if (std::string(ek2.familyName()) != "Exif") {
        std::cout << "Testcase failed (familyName)" << std::endl;
        rc += 1; 
    }
    // groupName
    tc += 1;
    if (ek2.groupName() != "Iop") {
        std::cout << "Testcase failed (groupName)" << std::endl;
        rc += 1; 
    }
    // tagName
    tc += 1;
    if (ek2.tagName() != "InteroperabilityVersion") {
        std::cout << "Testcase failed (tagName)" << std::endl;
        rc += 1; 
    }
    // tagName
    tc += 1;
    if (ek2.tag() != 0x0002) {
        std::cout << "Testcase failed (tag)" << std::endl;
        rc += 1; 
    }
    // ifdName
    tc += 1;
    if (std::string(ek2.ifdName()) != "Iop") {
        std::cout << "Testcase failed (ifdName: " << std::endl;
        rc += 1; 
    }
    // sectionName
    tc += 1;
    if (ek2.sectionName() != "Interoperability") {
        std::cout << "Testcase failed (sectionName)" << std::endl;
        rc += 1; 
    }

    // -----

    long len = 76;
    byte buf[] 
        = { // No
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

    Ifd ifd(Exiv2::iopIfd, 0, false);
    int ret = ifd.read(buf, len, bigEndian, 1);
    if (ret) {
        std::cout << "Ifd::read failed, ret = " << ret << "\n";
        return ret;
    }

    Ifd::const_iterator i = ifd.findTag(0x0002);
    if (i == ifd.end()) {
        std::cout << "Ifd::findTag failed" << "\n";
    }
    else {
        ExifKey ek3(*i);

        // operator<<
        tc += 1;
        std::ostringstream os2;
        os2 << ek3;
        if (os2.str() != key) {
            std::cout << "Testcase failed (operator<<)" << std::endl;
                                  rc += 1; 
        }
        // familyName
        tc += 1;
        if (std::string(ek3.familyName()) != "Exif") {
            std::cout << "Testcase failed (familyName)" << std::endl;
                                  rc += 1; 
        }
        // groupName
        tc += 1;
        if (ek3.groupName() != "Iop") {
            std::cout << "Testcase failed (groupName)" << std::endl;
                                  rc += 1; 
        }
        // tagName
        tc += 1;
        if (ek3.tagName() != "InteroperabilityVersion") {
            std::cout << "Testcase failed (tagName)" << std::endl;
                                  rc += 1; 
        }
        // tagName
        tc += 1;
        if (ek3.tag() != 0x0002) {
            std::cout << "Testcase failed (tag)" << std::endl;
                                  rc += 1; 
        }
        // ifdName
        tc += 1;
        if (std::string(ek3.ifdName()) != "Iop") {
            std::cout << "Testcase failed (ifdName: " << std::endl;
                                  rc += 1; 
        }
        // sectionName
        tc += 1;
        if (ek3.sectionName() != "Interoperability") {
            std::cout << "Testcase failed (sectionName)" << std::endl;
                                  rc += 1; 
        }

    }

    // -----

    if (rc == 0) {
        std::cout << "All " << tc << " testcases passed." << std::endl;
    }
    else {
        std::cout << rc << " of " << tc << " testcases failed." << std::endl;
    }
}

