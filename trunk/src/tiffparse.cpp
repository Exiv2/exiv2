// ***************************************************************** -*- C++ -*-
// tiffparse.cpp, $Rev$
// Print the structure of a TIFF file

#include "tiffparser.hpp"
#include "tiffimage.hpp"
#include "futils.hpp"

#include <iostream>

using namespace Exiv2;

int main(int argc, char* const argv[])
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        std::cout << "Print the structure of a TIFF file\n";
        return 1;
    }

    FileIo io(argv[1]);
    if(io.open() != 0) {
        throw Error(9, io.path(), strError());
    }
    IoCloser closer(io);

    // Ensure that this is the correct image type
    if (!isTiffType(io, false)) {
        if (io.error() || io.eof()) throw Error(14);
        throw Error(3, "TIFF");
    }

    // Read the image into a memory buffer
    long len = io.size();
    DataBuf buf(len);
    io.read(buf.pData_, len);
    if (io.error() || io.eof()) throw Error(14);

    const TiffStructure tiffStructure[] = {
        { Tag::root, Group::none, newTiffDirectory, Group::ifd0 },
        {    0x8769, Group::ifd0, newTiffSubIfd,    Group::exif },
        {    0x8825, Group::ifd0, newTiffSubIfd,    Group::gps  },
        {    0xa005, Group::exif, newTiffSubIfd,    Group::iop  },
        { Tag::next, Group::ifd0, newTiffDirectory, Group::ifd0 },
        // End of list marker
        { Tag::none, Group::none, 0, Group::none }
    };

    TiffHeade2 tiffHeader;
    if (!tiffHeader.read(buf.pData_, buf.size_)) throw Error(3, "TIFF");

    TiffComponent::AutoPtr rootDir 
        = TiffParser::create(Tag::root, Group::none, tiffStructure);
    if (0 == rootDir.get()) {
        throw Error(1, "No root element defined in TIFF structure");
    }
    rootDir->read(buf.pData_, buf.size_, tiffHeader.offset(), tiffHeader.byteOrder());

    tiffHeader.print(std::cerr);
    rootDir->print(std::cerr, tiffHeader.byteOrder());

    return 0;
}
catch (AnyError& e) {
    std::cerr << e << "\n";
    return -1;
}
