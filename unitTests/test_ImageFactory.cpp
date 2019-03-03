#include <image.hpp> // Unit under test

#include <error.hpp> // Need to include this header for the Exiv2::Error exception

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(TheImageFactory, createsInstancesForFewSupportedTypesInMemory)
{
    // Note that the constructor of these Image classes take an 'create' argument
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jp2));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::jpeg));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::exv));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::pgf));
    EXPECT_NO_THROW(ImageFactory::create(ImageType::png));
}

TEST(TheImageFactory, cannotCreateInstancesForMostTypesInMemory)
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
    EXPECT_THROW(ImageFactory::create(ImageType::tga), Error);
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

TEST(TheImageFactory, throwsWithImageTypeNone)
{
    EXPECT_THROW(ImageFactory::create(ImageType::none), Error);
}

TEST(TheImageFactory, throwsWithNonExistingImageTypes)
{
    EXPECT_THROW(ImageFactory::create(static_cast<ImageType>(666)), Error);
}


TEST(TheImageFactory, createsInstancesForFewSupportedTypesInFiles)
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
    EXPECT_THROW(ImageFactory::create(ImageType::tga, filePath), Error);
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

TEST(TheImageFactory, loadInstancesDifferentImageTypes)
{
    /// \todo use filesystem library with C++17
    std::string testData(TESTDATA_PATH);

    EXPECT_EQ(ImageType::jpeg, ImageFactory::getType(testData + "/DSC_3079.jpg"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/DSC_3079.jpg", false));

    EXPECT_EQ(ImageType::exv, ImageFactory::getType(testData + "/exiv2-bug1108.exv"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1108.exv", false));

    EXPECT_EQ(ImageType::crw, ImageFactory::getType(testData + "/exiv2-canon-powershot-s40.crw"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/exiv2-canon-powershot-s40.crw", false));

    EXPECT_EQ(ImageType::tiff, ImageFactory::getType(testData + "/exiv2-bug1044.tif"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1044.tif", false));

    EXPECT_EQ(ImageType::png, ImageFactory::getType(testData + "/exiv2-bug1074.png"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1074.png", false));

    EXPECT_EQ(ImageType::xmp, ImageFactory::getType(testData + "/BlueSquare.xmp"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/BlueSquare.xmp", false));

    EXPECT_EQ(ImageType::psd, ImageFactory::getType(testData + "/exiv2-photoshop.psd"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/exiv2-photoshop.psd", false));

    EXPECT_EQ(ImageType::webp, ImageFactory::getType(testData + "/cve_2017_1000126_stack-oob-read.webp"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/cve_2017_1000126_stack-oob-read.webp", false));

    EXPECT_EQ(ImageType::pgf, ImageFactory::getType(testData + "/imagemagick.pgf"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/imagemagick.pgf", false));

    EXPECT_EQ(ImageType::jp2, ImageFactory::getType(testData + "/Reagan.jp2"));
    EXPECT_NO_THROW(ImageFactory::open(testData + "/Reagan.jp2", false));
}
