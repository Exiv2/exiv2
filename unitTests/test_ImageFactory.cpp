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

TEST(TheImageFactory, getsExpectedModesForJp2Images)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForJpegImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::jpeg, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdXmp));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::jpeg, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForExvImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::exv, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdXmp));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::exv, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPgfImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::pgf, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdXmp));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::pgf, mdIccProfile));
}

#ifdef EXV_HAVE_LIBZ
TEST(TheImageFactory, getsExpectedModesForPngImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::png, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdXmp));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::png, mdIccProfile));
}
#endif

TEST(TheImageFactory, getsExpectedModesForBmpImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdNone));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdIptc));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForCr2Images)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForCrwImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::crw, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdIptc));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdXmp));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::crw, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForGifImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdNone));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdIptc));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForMrwImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForOrfImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPsdImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForRafImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForRw2Images)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForTgaImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdNone));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdIptc));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForWebpImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::webp, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::webp, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForTiffImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForDngImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForNefImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPefImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForArwImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForSr2Images)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForSrwImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForXmpImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdNone));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdExif));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdIptc));
    EXPECT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForBigTiffImages)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdNone));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdExif));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdIptc));
    EXPECT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForNoneValue)
{
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdNone));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdExif));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdIptc));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdXmp));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdComment));
    EXPECT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdIccProfile));
}

/// \todo check why JpegBase is taking ImageType in the constructor
