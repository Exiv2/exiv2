#include <image.hpp> // Unit under test

/// \todo we should not need to include all these headers to be able to use the Factory
#include <jpgimage.hpp>
#include <bmpimage.hpp>
#include <cr2image.hpp>
#include <crwimage.hpp>
#include <gifimage.hpp>
#include <jp2image.hpp>
#include <mrwimage.hpp>
#include <orfimage.hpp>
#include <pgfimage.hpp>
#include <pngimage.hpp>
#include <psdimage.hpp>
#include <rafimage.hpp>
#include <rw2image.hpp>
#include <tgaimage.hpp>
#include <tiffimage.hpp>
#include <webpimage.hpp>

#include <error.hpp> // Need to include this header for the Exiv2::Error exception

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(TheImageFactory, createsInstancesForSupportedTypesInMemory)
{
    // Note that the constructor of these Image classes take an 'create' argument
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::jp2));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::jpeg));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::exv));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::pgf));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::png));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypesInMemory)
{
    // Note that the constructor of these Image classes does not take an 'create' argument

    EXPECT_THROW(ImageFactory::create(ImageTypee::bmp), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::cr2), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::crw), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::gif), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::mrw), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::orf), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::psd), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::raf), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::rw2), Error);
//    EXPECT_THROW(ImageFactory::create(ImageTypee::tga), Error); // This one crashes badly
    EXPECT_THROW(ImageFactory::create(ImageTypee::webp), Error);

    // TIFF
    EXPECT_THROW(ImageFactory::create(ImageTypee::tiff), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::dng), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::nef), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::pef), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::arw), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::sr2), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::srw), Error);
}


TEST(TheImageFactory, createsInstancesForSupportedTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes take an 'create' argument
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::jp2, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::jpeg, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::exv, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::pgf, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageTypee::png, filePath));

    EXPECT_EQ(0, std::remove(filePath.c_str()));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes does not take an 'create' argument
    EXPECT_THROW(ImageFactory::create(ImageTypee::bmp, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::cr2, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::crw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::gif, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::mrw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::orf, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::psd, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::raf, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::rw2, filePath), Error);
//    EXPECT_THROW(ImageFactory::create(ImageTypee::tga), Error); // This one crashes badly
    EXPECT_THROW(ImageFactory::create(ImageTypee::webp, filePath), Error);

    // TIFF
    EXPECT_THROW(ImageFactory::create(ImageTypee::tiff, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::dng, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::nef, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::pef, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::arw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::sr2, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageTypee::srw, filePath), Error);
}
