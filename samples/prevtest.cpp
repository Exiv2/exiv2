// SPDX-License-Identifier: GPL-2.0-or-later
// Test access to preview images

#include <exiv2/exiv2.hpp>

#include <iostream>

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
    std::string filename(argv[1]);

    auto image = Exiv2::ImageFactory::open(filename);
    image->readMetadata();

    Exiv2::PreviewManager loader(*image);
    Exiv2::PreviewPropertiesList list = loader.getPreviewProperties();
    for (auto&& pos : list) {
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
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
