#include <image.hpp> // Unit under test

#include <error.hpp> // Need to include this header for the Exiv2::Error exception

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(TheImageFactory, createsInstancesForFewSupportedTypesInMemory)
{
    // Note that the constructor of these Image classes take an 'create' argument
    ASSERT_NO_THROW(ImageFactory::create(ImageType::jp2));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::jpeg));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::exv));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::pgf));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::png));
}

TEST(TheImageFactory, cannotCreateInstancesForMostTypesInMemory)
{
    // Note that the constructor of these Image classes does not take an 'create' argument

    ASSERT_THROW(ImageFactory::create(ImageType::bmp), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::cr2), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::crw), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::gif), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::mrw), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::orf), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::psd), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::raf), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::rw2), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::tga), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::webp), Error);

    // TIFF
    ASSERT_THROW(ImageFactory::create(ImageType::tiff), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::dng), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::nef), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::pef), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::arw), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::sr2), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::srw), Error);
}

TEST(TheImageFactory, throwsWithImageTypeNone)
{
    ASSERT_THROW(ImageFactory::create(ImageType::none), Error);
}

TEST(TheImageFactory, throwsWithNonExistingImageTypes)
{
    ASSERT_THROW(ImageFactory::create(static_cast<ImageType>(666)), Error);
}


TEST(TheImageFactory, createsInstancesForFewSupportedTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes take an 'create' argument
    ASSERT_NO_THROW(ImageFactory::create(ImageType::jp2, filePath));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::jpeg, filePath));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::exv, filePath));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::pgf, filePath));
    ASSERT_NO_THROW(ImageFactory::create(ImageType::png, filePath));

    ASSERT_EQ(0, std::remove(filePath.c_str()));
}

TEST(TheImageFactory, cannotCreateInstancesForSomeTypesInFiles)
{
    const std::string filePath("./here");

    // Note that the constructor of these Image classes does not take an 'create' argument
    ASSERT_THROW(ImageFactory::create(ImageType::bmp, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::cr2, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::crw, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::gif, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::mrw, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::orf, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::psd, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::raf, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::rw2, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::tga, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::webp, filePath), Error);

    // TIFF
    ASSERT_THROW(ImageFactory::create(ImageType::tiff, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::dng, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::nef, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::pef, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::arw, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::sr2, filePath), Error);
    ASSERT_THROW(ImageFactory::create(ImageType::srw, filePath), Error);
}

TEST(TheImageFactory, loadInstancesDifferentImageTypes)
{
    /// \todo use filesystem library with C++17
    std::string testData(TESTDATA_PATH);

    ASSERT_EQ(ImageType::jpeg, ImageFactory::getType(testData + "/DSC_3079.jpg"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/DSC_3079.jpg", false));

    ASSERT_EQ(ImageType::exv, ImageFactory::getType(testData + "/exiv2-bug1108.exv"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1108.exv", false));

    ASSERT_EQ(ImageType::crw, ImageFactory::getType(testData + "/exiv2-canon-powershot-s40.crw"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/exiv2-canon-powershot-s40.crw", false));

    ASSERT_EQ(ImageType::tiff, ImageFactory::getType(testData + "/exiv2-bug1044.tif"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1044.tif", false));

    ASSERT_EQ(ImageType::png, ImageFactory::getType(testData + "/exiv2-bug1074.png"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/exiv2-bug1074.png", false));

    ASSERT_EQ(ImageType::xmp, ImageFactory::getType(testData + "/BlueSquare.xmp"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/BlueSquare.xmp", false));

    ASSERT_EQ(ImageType::psd, ImageFactory::getType(testData + "/exiv2-photoshop.psd"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/exiv2-photoshop.psd", false));

    ASSERT_EQ(ImageType::webp, ImageFactory::getType(testData + "/cve_2017_1000126_stack-oob-read.webp"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/cve_2017_1000126_stack-oob-read.webp", false));

    ASSERT_EQ(ImageType::pgf, ImageFactory::getType(testData + "/imagemagick.pgf"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/imagemagick.pgf", false));

    ASSERT_EQ(ImageType::jp2, ImageFactory::getType(testData + "/Reagan.jp2"));
    ASSERT_NO_THROW(ImageFactory::open(testData + "/Reagan.jp2", false));
}

TEST(TheImageFactory, getsExpectedModesForJp2Images)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jp2, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::jp2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForJpegImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::jpeg, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdXmp));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::jpeg, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::jpeg, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForExvImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::exv, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdXmp));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::exv, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::exv, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPgfImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::pgf, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdXmp));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pgf, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::pgf, mdIccProfile));
}

#ifdef EXV_HAVE_PNG
TEST(TheImageFactory, getsExpectedModesForPngImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::png, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdXmp));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::png, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::png, mdIccProfile));
}
#endif

TEST(TheImageFactory, getsExpectedModesForBmpImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdNone));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdIptc));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bmp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForCr2Images)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::cr2, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::cr2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForCrwImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::crw, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdIptc));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdXmp));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::crw, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::crw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForGifImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdNone));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdIptc));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::gif, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForMrwImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::mrw, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::mrw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForOrfImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::orf, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::orf, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPsdImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::psd, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::psd, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForRafImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::raf, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::raf, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForRw2Images)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::rw2, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::rw2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForTgaImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdNone));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdIptc));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tga, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForWebpImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::webp, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::webp, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::webp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForTiffImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::tiff, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::tiff, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForDngImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::dng, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::dng, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForNefImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::nef, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::nef, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForPefImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::pef, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::pef, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForArwImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::arw, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::arw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForSr2Images)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::sr2, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::sr2, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForSrwImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::srw, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::srw, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForXmpImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdNone));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdExif));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdIptc));
    ASSERT_EQ(amReadWrite, ImageFactory::checkMode(ImageType::xmp, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::xmp, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForBigTiffImages)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdNone));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdExif));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdIptc));
    ASSERT_EQ(amRead, ImageFactory::checkMode(ImageType::bigtiff, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::bigtiff, mdIccProfile));
}

TEST(TheImageFactory, getsExpectedModesForNoneValue)
{
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdNone));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdExif));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdIptc));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdXmp));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdComment));
    ASSERT_EQ(amNone, ImageFactory::checkMode(ImageType::none, mdIccProfile));
}

/// \todo check why JpegBase is taking ImageType in the constructor
