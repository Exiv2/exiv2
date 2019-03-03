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
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jp2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jpeg));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::exv));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pgf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::png));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypesInMemory)
{
    // Note that the constructor of these Image classes does not take an 'create' argument

    EXPECT_THROW(ImageFactory::create(ImageType::bmp), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::cr2), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::crw), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::gif), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::mrw), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::orf), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::psd), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::raf), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::rw2), Error);
//    EXPECT_THROW(ImageFactory::create(ImageType::tga), Error); // This one crashes badly
    EXPECT_THROW(ImageFactory::create(ImageType::webp), Error);

    // TIFF
    EXPECT_THROW(ImageFactory::create(ImageType::tiff), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::dng), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::nef), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::pef), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::arw), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::sr2), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::srw), Error);
}


TEST(TheImageFactory, createsInstancesForSupportedTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes take an 'create' argument
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jp2, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jpeg, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::exv, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pgf, filePath));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::png, filePath));

    EXPECT_EQ(0, std::remove(filePath.c_str()));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes does not take an 'create' argument
    EXPECT_THROW(ImageFactory::create(ImageType::bmp, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::cr2, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::crw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::gif, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::mrw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::orf, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::psd, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::raf, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::rw2, filePath), Error);
//    EXPECT_THROW(ImageFactory::create(ImageType::tga), Error); // This one crashes badly
    EXPECT_THROW(ImageFactory::create(ImageType::webp, filePath), Error);

    // TIFF
    EXPECT_THROW(ImageFactory::create(ImageType::tiff, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::dng, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::nef, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::pef, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::arw, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::sr2, filePath), Error);
    EXPECT_THROW(ImageFactory::create(ImageType::srw, filePath), Error);
}
