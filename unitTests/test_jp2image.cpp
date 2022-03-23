// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/jp2image.hpp>
#include <exiv2/error.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(Jp2Image, canBeCreatedFromScratch)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    ASSERT_NO_THROW(newJp2Instance(memIo, create));
}

TEST(Jp2Image, canBeOpenedEvenWithAnEmptyMemIo)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = false;
    ASSERT_NO_THROW(newJp2Instance(memIo, create));
}

TEST(Jp2Image, mimeTypeIsPng)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    Image::AutoPtr image = newJp2Instance(memIo, create);
    ASSERT_EQ("image/jp2", image->mimeType());
}

TEST(Jp2Image, printStructurePrintsNothingWithKpsNone)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    Image::AutoPtr image = newJp2Instance(memIo, create);

    std::ostringstream stream;
    image->printStructure(stream, Exiv2::kpsNone, 1);

    ASSERT_TRUE(stream.str().empty());
}

TEST(Jp2Image, printStructurePrintsDataWithKpsBasic)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    Image::AutoPtr image = newJp2Instance(memIo, create);

    std::ostringstream stream;
    image->printStructure(stream, Exiv2::kpsBasic, 1);

    ASSERT_FALSE(stream.str().empty());
}

TEST(Jp2Image, cannotReadMetadataFromEmptyIo)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = false;
    Image::AutoPtr image = newJp2Instance(memIo, create);
    ASSERT_TRUE(image.get() == NULL);
}

TEST(Jp2Image, cannotReadMetadataFromIoWhichCannotBeOpened)
{
    Exiv2::BasicIo::AutoPtr io (new Exiv2::FileIo("NonExistingPath.jp2"));
    const bool create = false;
    Image::AutoPtr image = newJp2Instance(io, create);
    ASSERT_TRUE(image.get() == NULL);
}

TEST(Jp2Image, cannotWriteMetadataToEmptyIo)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = false;
    Image::AutoPtr image = newJp2Instance(memIo, create);
    ASSERT_TRUE(image.get() == NULL);
}

TEST(Jp2Image, canWriteMetadataFromCreatedJp2Image)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    Image::AutoPtr image = newJp2Instance(memIo, create);
    ASSERT_NO_THROW(image->writeMetadata());
}

TEST(Jp2Image, canWriteMetadataAndReadAfterwards)
{
    Exiv2::BasicIo::AutoPtr memIo (new Exiv2::MemIo);
    const bool create = true;
    Image::AutoPtr image = newJp2Instance(memIo, create);

    ASSERT_NO_THROW(image->writeMetadata());
    ASSERT_NO_THROW(image->readMetadata());
}
