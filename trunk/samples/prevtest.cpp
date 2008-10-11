// ***************************************************************** -*- C++ -*-
// prevtest.cpp, $Rev$
// Test access to preview images

#include <exiv2/image.hpp>
#include <exiv2/preview.hpp>

#include <string>
#include <iostream>
#include <cassert>

int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::PreviewImageLoader loader(*image);
    
    Exiv2::PreviewPropertiesList list = loader.getPreviewPropertiesList();

    for (Exiv2::PreviewPropertiesList::iterator pos = list.begin(); pos != list.end(); pos++)
    {
        std::cout << "found preview " << pos->id_ 
                  << ", size: " << pos->size_ 
                  << ", " << pos->width_ << 'x' << pos->height_
                  << "\n";

        Exiv2::PreviewImage image = loader.getPreviewImage(*pos);
        char buf[50];
        sprintf(buf, "%dx%d",pos->width_, pos->height_);
        image.writeFile(std::string(argv[1]) + "_" + buf);
    }
    
    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
