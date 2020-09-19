// ***************************************************************** -*- C++ -*-
// prevtest.cpp
// Test access to preview images

#include <cassert>
#include <exiv2/exiv2.hpp>
#include <iostream>
#include <string>

int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string filename(argv[1]);

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filename);
    assert(image.get() != nullptr);
    image->readMetadata();

    Exiv2::PreviewManager loader(*image);
    Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();
    for (auto& pos : list) {
        std::cout << pos.mimeType_ << " preview, type " << pos.id_ << ", " << pos.size_ << " bytes, " << pos.width_
                  << 'x' << pos.height_ << " pixels"
                  << "\n";

        Exiv2::PreviewImage preview = loader.getPreviewImage(pos);
        preview.writeFile(filename + "_" + Exiv2::toString(pos.width_) + "x" + Exiv2::toString(pos.height_));
    }

    // Cleanup
    Exiv2::XmpParser::terminate();
    
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
