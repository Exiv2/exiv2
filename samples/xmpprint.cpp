// ***************************************************************** -*- C++ -*-
// xmpprint.cpp
// Read an XMP from a video or graphic file, parse it and print 
// all (known) properties.
// ========================================================================
// Linux standalone compilation : 
//      g++ -o xmprint xmprint.cpp `pkg-config --cflags --libs exiv2`
// ========================================================================

#include <cassert>
#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  try
  {
    if (argc != 2) 
      {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
      }

    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(argv[1]);
    assert (image.get() != nullptr);
    image->readMetadata();

    Exiv2::XmpData &xmpData = image->xmpData();
    if (xmpData.empty()) {
        std::string error(argv[1]);
        error += ": No XMP data found in the file";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
    }
    if (xmpData.empty()) 
      {
        std::string error(argv[1]);
        error += ": No XMP properties found in the XMP packet";
        throw Exiv2::Error(Exiv2::kerErrorMessage, error);
      }

      for (const auto& md : xmpData) {
          std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key() << " " << std::setw(9)
                    << std::setfill(' ') << std::left << md.typeName() << " " << std::dec << std::setw(3)
                    << std::setfill(' ') << std::right << md.count() << "  " << std::dec << md.value() << std::endl;
      }

    Exiv2::XmpParser::terminate();

    return 0;
  }
catch (Exiv2::AnyError& e) 
  {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
  }
}

