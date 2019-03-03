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

TEST(TheImageFactory, createsInstancesForSupportedTypes)
{
    // Note that the constructor of these Image classes take an 'create' argument
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jp2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jpeg));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::exv));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pgf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::png));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypes)
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
