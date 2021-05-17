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

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <string>
#include <cstring>

using namespace Exiv2;

int main()
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

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
    if (std::string(ExifTags::ifdName(ek.groupName())) != "Iop") {
        std::cout << "Testcase failed (ifdName: " << std::endl;
        rc += 1;
    }
    // sectionName
    tc += 1;
    if (strcmp(ExifTags::sectionName(ek), "Interoperability") != 0) {
        std::cout << "Testcase failed (sectionName)" << std::endl;
        rc += 1;
    }

    // -----

    // Copy constructor
    const ExifKey& ek2(ek);

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
    if (std::string(ExifTags::ifdName(ek2.groupName())) != "Iop") {
        std::cout << "Testcase failed (ifdName: " << std::endl;
        rc += 1;
    }
    // sectionName
    tc += 1;
    if (strcmp(ExifTags::sectionName(ek2), "Interoperability") != 0) {
        std::cout << "Testcase failed (sectionName)" << std::endl;
        rc += 1;
    }

    // -----

    ExifKey ek4("Exif.Image.0x0110");
    tc += 1;
    if (ek4.key() != "Exif.Image.Model") {
        std::cout << "Testcase failed (converted key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ek4.tagName() != "Model") {
        std::cout << "Testcase failed (converted tagName)" << std::endl;
        rc += 1;
    }

    // -----

    ExifKey ek5("Exif.Nikon3.0x0007");
    tc += 1;
    if (ek5.key() != "Exif.Nikon3.Focus") {
        std::cout << "Testcase failed (converted key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ek5.tagName() != "Focus") {
        std::cout << "Testcase failed (converted tagName)" << std::endl;
        rc += 1;
    }

    // -----

    IptcKey ik1("Iptc.Envelope.0x0005");
    tc += 1;
    if (ik1.key() != "Iptc.Envelope.Destination") {
        std::cout << "Testcase failed (converted Iptc key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik1.tagName() != "Destination") {
        std::cout << "Testcase failed (converted tagName)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik1.recordName() != "Envelope") {
        std::cout << "Testcase failed (converted recordName)" << std::endl;
        rc += 1;
    }

    // -----

    IptcKey ik2(0xabcd, 0x1234);
    tc += 1;
    if (ik2.key() != "Iptc.0x1234.0xabcd") {
        std::cout << "Testcase failed (unknown Iptc key)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik2.tagName() != "0xabcd") {
        std::cout << "Testcase failed (converted tagName)" << std::endl;
        rc += 1;
    }
    tc += 1;
    if (ik2.recordName() != "0x1234") {
        std::cout << "Testcase failed (converted recordName)" << std::endl;
        rc += 1;
    }

    // -----

    if (rc == 0) {
        std::cout << "All " << tc << " testcases passed." << std::endl;
    }
    else {
        std::cout << rc << " of " << tc << " testcases failed." << std::endl;
    }
}

