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

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(TheImageFactory, createsInstancesOfAllSupportedTypes)
{
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jp2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jpeg));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::exv));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pgf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::png));


    // The next expectations fail ... O_O ...

    EXPECT_NO_THROW(ImageFactory::create(ImageType::bmp));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::cr2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::crw));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::gif));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::mrw));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::orf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::psd));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::raf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::rw2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::tga));

    // TIFF
    EXPECT_NO_THROW(ImageFactory::create(ImageType::tiff));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::dng));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::nef));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pef));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::arw));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::sr2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::srw));

    EXPECT_NO_THROW(ImageFactory::create(ImageType::webp));
}
