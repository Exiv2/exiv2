#include <image.hpp> // Unit under test

#include <gtest/gtest.h>

using namespace Exiv2;

namespace
{
    const std::string testData(TESTDATA_PATH); /// \todo use filesystem library with C++17
}

TEST(AJpegImage, canReadMetadata)
{
    auto image = ImageFactory::open(testData + "/DSC_3079.jpg", false);
    ASSERT_NO_THROW(image->readMetadata());

    ASSERT_EQ(bigEndian, image->byteOrder());
    ASSERT_TRUE(image->good());
    ASSERT_EQ(720, image->pixelWidth());
    ASSERT_EQ(1280, image->pixelHeight());

    ASSERT_TRUE(image->comment().empty());
    ASSERT_FALSE(image->xmpPacket().empty());
    ASSERT_FALSE(image->iccProfileDefined());

    ASSERT_FALSE(image->supportsMetadata(mdNone));
    ASSERT_TRUE(image->supportsMetadata(mdExif));
    ASSERT_TRUE(image->supportsMetadata(mdIptc));
    ASSERT_TRUE(image->supportsMetadata(mdComment));
    ASSERT_TRUE(image->supportsMetadata(mdXmp));
    ASSERT_FALSE(image->supportsMetadata(mdIccProfile));

    ASSERT_TRUE(image->nativePreviews().empty());

    ASSERT_EQ(ImageType::jpeg, image->imageType());

    ASSERT_EQ(14, image->exifData().count());
}

TEST(AJpegImage, hasNoMetadataWhenCreatedFromScratch)
{
    const std::string filePath("./here.jpeg");
    auto image = ImageFactory::create(ImageType::jpeg, filePath);
    ASSERT_NO_THROW(image->readMetadata());

    ASSERT_EQ(invalidByteOrder, image->byteOrder());
    ASSERT_TRUE(image->good());
    ASSERT_EQ(1, image->pixelWidth());
    ASSERT_EQ(1, image->pixelHeight());

    ASSERT_TRUE(image->comment().empty());
    ASSERT_TRUE(image->xmpPacket().empty());
    ASSERT_FALSE(image->iccProfileDefined());

    ASSERT_EQ(0, image->exifData().count());

    ASSERT_EQ(0, std::remove(filePath.c_str()));
}
