#include "ifd.hpp"
#include "error.hpp"

#include <iostream>
#include <string>

/*
  Test cases:
  - IFD entry offset is 0
  - Wrong endianness



*/

int main()
try {
    long len = 76;
    Exiv2::byte buf[] 
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

    Exiv2::Ifd ifd(Exiv2::ifd0, 0, false);
    int rc = ifd.read(buf, len, Exiv2::bigEndian, 1);
    if (rc) {
        std::cout << "Ifd::read failed, rc = " << rc << "\n";
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


    ifd.print(std::cout);


    return 0;
}
catch (const Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return 1;
}
