#include <exiv2/bmpimage.hpp>

#include <gtest/gtest.h>

#include <array>

using namespace Exiv2;

TEST(BmpImage, canBeOpenedWithEmptyMemIo)
{
    auto memIo = std::make_unique<MemIo>();
    ASSERT_NO_THROW(BmpImage bmp(std::move(memIo)));
}

TEST(BmpImage, mimeTypeIsBmp)
{
    auto memIo = std::make_unique<MemIo>();
    BmpImage bmp(std::move(memIo));

    ASSERT_EQ("image/x-ms-bmp", bmp.mimeType());
}
