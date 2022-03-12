// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/jp2image.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(Jp2Image, canBeCreatedFromScratch)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    ASSERT_NO_THROW(Jp2Image image(std::move(memIo), create));
}

TEST(Jp2Image, canBeOpenedEvenWithAnEmptyMemIo)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {false};
    ASSERT_NO_THROW(Jp2Image image(std::move(memIo), create));
}

TEST(Jp2Image, mimeTypeIsPng)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    Jp2Image image(std::move(memIo), create);

    ASSERT_EQ("image/jp2", image.mimeType());
}

TEST(Jp2Image, printStructurePrintsNothingWithKpsNone)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    Jp2Image image(std::move(memIo), create);

    std::ostringstream stream;
    image.printStructure(stream, Exiv2::kpsNone, 1);

    ASSERT_TRUE(stream.str().empty());
}

TEST(Jp2Image, printStructurePrintsDataWithKpsBasic)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    Jp2Image image(std::move(memIo), create);

    std::ostringstream stream;
    image.printStructure(stream, Exiv2::kpsBasic, 1);

    ASSERT_FALSE(stream.str().empty());
}
