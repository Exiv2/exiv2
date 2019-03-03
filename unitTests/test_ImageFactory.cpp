#include <image.hpp>  // Unit under test

#include <error.hpp>  // Need to include this header for the Exiv2::Error exception

#include <gtest/gtest.h>
#include <filesystem>

using namespace Exiv2;
namespace fs = std::filesystem;

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
    fs::path testData(TESTDATA_PATH);

    std::string imagePath = (testData / "DSC_3079.jpg").string();
    EXPECT_EQ(ImageType::jpeg, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "exiv2-bug1108.exv").string();
    EXPECT_EQ(ImageType::exv, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "exiv2-canon-powershot-s40.crw").string();
    EXPECT_EQ(ImageType::crw, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "exiv2-bug1044.tif").string();
    EXPECT_EQ(ImageType::tiff, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "exiv2-bug1074.png").string();
    EXPECT_EQ(ImageType::png, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "BlueSquare.xmp").string();
    EXPECT_EQ(ImageType::xmp, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "exiv2-photoshop.psd").string();
    EXPECT_EQ(ImageType::psd, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "cve_2017_1000126_stack-oob-read.webp").string();
    EXPECT_EQ(ImageType::webp, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "imagemagick.pgf").string();
    EXPECT_EQ(ImageType::pgf, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));

    imagePath = (testData / "Reagan.jp2").string();
    EXPECT_EQ(ImageType::jp2, ImageFactory::getType(imagePath));
    EXPECT_NO_THROW(ImageFactory::open(imagePath, false));
}
