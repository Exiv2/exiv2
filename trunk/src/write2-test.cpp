// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData write unit tests for Exif data created from scratch

  File     : write2-test.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 26-Jun-04, ahu: created

 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>
#include <string>

void write(const std::string& file, Exiv2::ExifData& ed);
void print(const std::string& file);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string file(argv[1]);

    std::cout <<"----- Two IFD0 tags\n";
    Exiv2::ExifData ed1;
    Exiv2::Exifdatum md1(Exiv2::ExifKey("Exif.Image.Model"));
    md1.setValue("Test 1");
    ed1.add(md1);

    Exiv2::Value::AutoPtr v1 = Exiv2::Value::create(Exiv2::unsignedShort);
    v1->read("160 161 162 163");
    ed1.add(Exiv2::ExifKey("Exif.Image.SamplesPerPixel"), v1.get());

    Exiv2::Value::AutoPtr v2 = Exiv2::Value::create(Exiv2::signedLong);
    v2->read("-2 -1 0 1");
    ed1.add(Exiv2::ExifKey("Exif.Image.XResolution"), v2.get());

    Exiv2::Value::AutoPtr v3 = Exiv2::Value::create(Exiv2::signedRational);
    v3->read("-2/3 -1/3 0/3 1/3");
    ed1.add(Exiv2::ExifKey("Exif.Image.YResolution"), v3.get());

    Exiv2::Value::AutoPtr v4 = Exiv2::Value::create(Exiv2::undefined);
    v4->read("255 254 253 252");
    ed1.add(Exiv2::ExifKey("Exif.Image.WhitePoint"), v4.get());

    write(file, ed1);
    print(file);

    std::cout <<"\n----- One Exif tag\n";
    Exiv2::ExifData ed2;
    Exiv2::Exifdatum md2(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
    md2.setValue("Test 2");
    ed2.add(md2);
    write(file, ed2);
    print(file);

    std::cout <<"\n----- One Canon MakerNote tag\n";
    Exiv2::ExifData edMn1;
    Exiv2::Exifdatum mdMn1(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn1.setValue("Canon");
    edMn1.add(mdMn1);
    Exiv2::Exifdatum mdMn2(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn2.setValue("Canon PowerShot S40");
    edMn1.add(mdMn2);
    Exiv2::Exifdatum mdMn3(Exiv2::ExifKey("Exif.Canon.0xabcd"));
    mdMn3.setValue("A Canon makernote tag");
    edMn1.add(mdMn3);
    write(file, edMn1);
    print(file);

    std::cout <<"\n----- One Fujifilm MakerNote tag\n";
    Exiv2::ExifData edMn2;
    Exiv2::Exifdatum mdMn4(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn4.setValue("FUJIFILM");
    edMn2.add(mdMn4);
    Exiv2::Exifdatum mdMn5(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn5.setValue("FinePixS2Pro");
    edMn2.add(mdMn5);
    Exiv2::Exifdatum mdMn6(Exiv2::ExifKey("Exif.Fujifilm.0x1000"));
    mdMn6.setValue("A Fujifilm QUALITY tag");
    edMn2.add(mdMn6);
    write(file, edMn2);
    print(file);

    std::cout <<"\n----- One Sigma/Foveon MakerNote tag\n";
    Exiv2::ExifData edMn3;
    Exiv2::Exifdatum mdMn7(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn7.setValue("SIGMA");
    edMn3.add(mdMn7);
    Exiv2::Exifdatum mdMn8(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn8.setValue("SIGMA SD10");
    edMn3.add(mdMn8);
    Exiv2::Exifdatum mdMn9(Exiv2::ExifKey("Exif.Sigma.0x0018"));
    mdMn9.setValue("Software? Exiv2!");
    edMn3.add(mdMn9);
    write(file, edMn3);
    print(file);

    std::cout <<"\n----- One Nikon1 MakerNote tag\n";
    Exiv2::ExifData edMn4;
    Exiv2::Exifdatum mdMn10(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn10.setValue("NIKON");
    edMn4.add(mdMn10);
    Exiv2::Exifdatum mdMn11(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn11.setValue("E990");
    edMn4.add(mdMn11);
    Exiv2::Exifdatum mdMn12(Exiv2::ExifKey("Exif.Nikon1.0x0080"));
    mdMn12.setValue("ImageAdjustment by Exiv2");
    edMn4.add(mdMn12);
    write(file, edMn4);
    print(file);

    std::cout <<"\n----- One Nikon2 MakerNote tag\n";
    Exiv2::ExifData edMn5;
    Exiv2::Exifdatum mdMn13(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn13.setValue("NIKON");
    edMn5.add(mdMn13);
    Exiv2::Exifdatum mdMn14(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn14.setValue("E950");
    edMn5.add(mdMn14);
    Exiv2::Exifdatum mdMn15(Exiv2::ExifKey("Exif.Nikon2.0xffff"));
    mdMn15.setValue("An obscure Nikon2 tag");
    edMn5.add(mdMn15);
    write(file, edMn5);
    print(file);

    std::cout <<"\n----- One Nikon3 MakerNote tag\n";
    Exiv2::ExifData edMn6;
    Exiv2::Exifdatum mdMn16(Exiv2::ExifKey("Exif.Image.Make"));
    mdMn16.setValue("NIKON CORPORATION");
    edMn6.add(mdMn16);
    Exiv2::Exifdatum mdMn17(Exiv2::ExifKey("Exif.Image.Model"));
    mdMn17.setValue("NIKON D70");
    edMn6.add(mdMn17);
    Exiv2::Exifdatum mdMn18(Exiv2::ExifKey("Exif.Nikon3.0x0004"));
    mdMn18.setValue("A boring Nikon3 Quality tag");
    edMn6.add(mdMn18);
    write(file, edMn6);
    print(file);

    std::cout <<"\n----- One IOP tag\n";
    Exiv2::ExifData ed3;
    Exiv2::Exifdatum md3(Exiv2::ExifKey("Exif.Iop.InteroperabilityVersion"));
    md3.setValue("Test 3");
    ed3.add(md3);
    write(file, ed3);
    print(file);

    std::cout <<"\n----- One GPS tag\n";
    Exiv2::ExifData ed4;
    Exiv2::Exifdatum md4(Exiv2::ExifKey("Exif.GPSInfo.GPSVersionID"));
    md4.setValue("Test 4");
    ed4.add(md4);
    write(file, ed4);
    print(file);

    // Todo: Fix this
    std::cout <<"\n----- One IFD1 tag\n";
    Exiv2::ExifData ed5;
    Exiv2::Exifdatum md5(Exiv2::ExifKey("Exif.Thumbnail.Artist"));
    md5.setValue("Test 5");
    ed5.add(md5);

    Exiv2::Exifdatum md6(Exiv2::ExifKey("Exif.Image.Model"));
    md6.setValue("Test 5 (Fix me!)");
    ed5.add(md6);

    write(file, ed5);
    print(file);

    return 0;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
}

void write(const std::string& file, Exiv2::ExifData& ed)
{
    int rc = ed.write(file);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, file);
        throw Exiv2::Error(error);
    }
}

void print(const std::string& file)
{
    Exiv2::ExifData ed;
    int rc = ed.read(file);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, file);
        throw Exiv2::Error(error);
    }

    Exiv2::ExifData::const_iterator end = ed.end();
    for (Exiv2::ExifData::const_iterator i = ed.begin(); i != end; ++i) {
        std::cout << std::setw(45) << std::setfill(' ') << std::left
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
