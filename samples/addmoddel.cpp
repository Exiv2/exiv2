// SPDX-License-Identifier: GPL-2.0-or-later

// Sample program showing how to add, modify and delete Exif metadata.

#include <exiv2/exiv2.hpp>

#include <iostream>

int main(int argc, char* const argv[]) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }
    std::string file(argv[1]);

    // Container for exif metadata. This is an example of creating
    // exif metadata from scratch. If you want to add, modify, delete
    // metadata that exists in an image, start with ImageFactory::open
    Exiv2::ExifData exifData;

    // *************************************************************************
    // Add to the Exif data

    // This is the quickest way to add (simple) Exif data. If a metadatum for
    // a given key already exists, its value is overwritten. Otherwise a new
    // tag is added.
    exifData["Exif.Image.Model"] = "Test 1";                              // AsciiValue
    exifData["Exif.Image.SamplesPerPixel"] = static_cast<uint16_t>(162);  // UShortValue
    exifData["Exif.Image.XResolution"] = -2;                              // LongValue
    exifData["Exif.Image.YResolution"] = Exiv2::Rational(-2, 3);          // RationalValue
    std::cout << "Added a few tags the quick way.\n";

    // Create a ASCII string value (note the use of create)
    auto v = Exiv2::Value::create(Exiv2::asciiString);
    // Set the value to a string
    v->read("1999:12:31 23:59:59");
    // Add the value together with its key to the Exif data container
    Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
    exifData.add(key, v.get());
    std::cout << "Added key \"" << key << "\", value \"" << *v << "\"\n";

    // Now create a more interesting value (without using the create method)
    Exiv2::URationalValue rv;
    // Set two rational components from a string
    rv.read("1/2 1/3");
    // Add more elements through the extended interface of rational value
    rv.value_.emplace_back(2, 3);
    rv.value_.emplace_back(3, 4);
    // Add the key and value pair to the Exif data
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    exifData.add(key, &rv);
    std::cout << "Added key \"" << key << "\", value \"" << rv << "\"\n";

    // *************************************************************************
    // Modify Exif data

    // Since we know that the metadatum exists (or we don't mind creating a new
    // tag if it doesn't), we can simply do this:
    Exiv2::Exifdatum& tag = exifData["Exif.Photo.DateTimeOriginal"];
    std::string date = tag.toString();
    date.replace(0, 4, "2000");
    tag.setValue(date);
    std::cout << "Modified key \"" << tag.key() << "\", new value \"" << tag.value() << "\"\n";

    // Alternatively, we can use findKey()
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    auto pos = exifData.findKey(key);
    if (pos == exifData.end())
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Key not found");

    // Get a pointer to a copy of the value
    v = pos->getValue();
    // Downcast the Value pointer to its actual type
    auto prv = dynamic_cast<Exiv2::URationalValue*>(v.get());
    if (!prv)
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Downcast failed");

    rv = Exiv2::URationalValue(*prv);
    // Modify the value directly through the interface of URationalValue
    rv.value_.at(2) = {88, 77};
    // Copy the modified value back to the metadatum
    pos->setValue(&rv);
    std::cout << "Modified key \"" << key << "\", new value \"" << pos->value() << "\"\n";

    // *************************************************************************
    // Delete metadata from the Exif data container

    // Delete the metadatum at iterator position pos
    key = Exiv2::ExifKey("Exif.Image.PrimaryChromaticities");
    pos = exifData.findKey(key);
    if (pos == exifData.end())
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Key not found");
    exifData.erase(pos);
    std::cout << "Deleted key \"" << key << "\"\n";

    // *************************************************************************
    // Finally, write the remaining Exif data to the image file
    auto image = Exiv2::ImageFactory::open(file);
    image->setExifData(exifData);
    image->writeMetadata();

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
