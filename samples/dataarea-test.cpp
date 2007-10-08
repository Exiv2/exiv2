// ***************************************************************** -*- C++ -*-
/*
  Abstract : Tests for dataArea related methods

  File     : dataarea-test.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 12-Nov-04, ahu: created

 */
// *****************************************************************************
// included header files
#include <exiv2/image.hpp>
#include <exiv2/jpgimage.hpp>
#include <exiv2/exif.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>

void write(const std::string& file, Exiv2::ExifData& ed);
void print(const std::string& file);
int read(const std::string& path);

using namespace Exiv2;

// *****************************************************************************
// Main
int main(int /*argc*/, char* const /*argv*/[])
{
try {
    byte da1[]
        = { 0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,
            0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,
            0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,
            0xaa,0xbb,0xaa,0xbb,0xaa,0xbb,0xaa,0xbb
        };

    long len1 = 32;

    byte da2[]
        = { 0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
            0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc
        };
    long len2 = 16;

    Value::AutoPtr v1 = Value::create(unsignedLong);
    v1->setDataArea(da1, len1);
    v1->read("0");

    Value::AutoPtr v2 = Value::create(undefined);
    v2->read("238 238 238 238 238 238 238 238");

    Value::AutoPtr v3 = Value::create(unsignedShort);
    v3->setDataArea(da2, len2);
    v3->read("0 16");

    ExifData ed;
    ed.add(ExifKey("Exif.Image.Copyright"), v1.get());
    ed.add(ExifKey("Exif.Image.Software"), v2.get());
    ed.add(ExifKey("Exif.Image.Artist"), v3.get());

    std::string file("dataarea.exv");
    std::cout << "Writing file " << file << "\n";
    write(file, ed);

    std::cout << "\nReading IFD from file\n";
    read(file);

    std::cout << "\nReading metadata from file\n";
    print(file);

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
}

void write(const std::string& file, Exiv2::ExifData& ed)
{
    Image::AutoPtr image = ImageFactory::create(Exiv2::ImageType::exv, file);
    assert(image.get() != 0);

    image->setExifData(ed);
    image->writeMetadata();
}

void print(const std::string& file)
{
    Image::AutoPtr image = ImageFactory::open(file);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &ed = image->exifData();
    Exiv2::ExifData::const_iterator end = ed.end();
    for (Exiv2::ExifData::const_iterator i = ed.begin(); i != end; ++i) {
        std::cout << std::setw(35) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << " "
                  << std::setw(12) << std::setfill(' ') << std::left
                  << i->ifdName() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << i->typeName() << " "
                  << std::dec << std::setw(3)
                  << std::setfill(' ') << std::right
                  << i->count() << " "
                  << std::dec << i->value()
                  << "\n";
    }
}

int read(const std::string& path)
{
    Image::AutoPtr image = ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();

    if (!image->exifData().empty()) {
        DataBuf exifData = image->exifData().copy();
        long size = exifData.size_;

        // Read the TIFF header
        TiffHeader tiffHeader;
        int rc = tiffHeader.read(exifData.pData_);
        if (rc) return rc;

        // Read IFD0
        Ifd ifd0(ifd0Id);
        rc = ifd0.read(exifData.pData_,
                       size,
                       tiffHeader.offset(),
                       tiffHeader.byteOrder());
        if (rc) return rc;
        ifd0.print(std::cout);

        Ifd::const_iterator i = ifd0.findTag(0x8298);
        assert(i != ifd0.end());

        Value::AutoPtr v = Value::create(TypeId(i->type()));
        v->read(i->data(), i->count() * i->typeSize(), tiffHeader.byteOrder());
        v->setDataArea(exifData.pData_ + v->toLong(), 32);

        std::cout << "Value of tag 0x8298: " << std::hex;
        v->write(std::cout);
        std::cout << std::endl;

        DataBuf buf = v->dataArea();
        for (int i = 0; i< buf.size_; ++i) {
            std::cout << std::hex << (int)buf.pData_[i] << " ";
        }
        std::cout << std::endl;

        // --------

        i = ifd0.findTag(0x013b);
        assert(i != ifd0.end());

        v = Value::create(TypeId(i->type()));
        v->read(i->data(), i->count() * i->typeSize(), tiffHeader.byteOrder());
        v->setDataArea(exifData.pData_ + v->toLong(), 16);

        std::cout << "Value of tag 0x013b: ";
        v->write(std::cout);
        std::cout << std::endl;

        buf = v->dataArea();
        for (int i = 0; i< buf.size_; ++i) {
            std::cout << std::hex << (int)buf.pData_[i] << " ";
        }
        std::cout << std::endl;

    }
    return 0;
}
