#include <pngchunk_int.hpp> // SUT
#include <error.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

struct PngChunkIHDR : ::testing::Test
{
    Internal::PngImageHeader inputData, output;

    void checkThrows()
    {
        DataBuf buf (reinterpret_cast<byte*>(&inputData), 13);
        ASSERT_THROW(Internal::PngChunk::decodeIHDRChunk(buf, output), Exiv2::Error);
    }
};

TEST_F(PngChunkIHDR, DecodeValidhunk)
{
    l2Data(reinterpret_cast<byte *>(&inputData.width), 720, bigEndian);
    l2Data(reinterpret_cast<byte *>(&inputData.height), 1280, bigEndian);
    inputData.bitDepth = 8;
    inputData.colorType = 2;
    inputData.compressionMethod = 0;
    inputData.filterMethod = 0;
    inputData.interlaceMethod = 1;

    DataBuf buf (reinterpret_cast<byte*>(&inputData), 13);

    Internal::PngChunk::decodeIHDRChunk(buf, output);

    ASSERT_EQ(output.width, 720);
    ASSERT_EQ(output.height, 1280);
    ASSERT_EQ(output.bitDepth, 8);
    ASSERT_EQ(output.colorType, 2);
    ASSERT_EQ(output.compressionMethod, 0);
    ASSERT_EQ(output.filterMethod, 0);
    ASSERT_EQ(output.interlaceMethod, 1);
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidBitDepth)
{
    inputData.bitDepth = 3;
    checkThrows();
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidBitDepthForCertainColorType)
{
    inputData.bitDepth = 4;  // valid value in its own
    inputData.colorType = 2; // For this colorType only 8 and 16 are valid bitDepths
    checkThrows();
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidColorType)
{
    inputData.colorType = 1;
    checkThrows();
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidCompressionMethod)
{
    inputData.compressionMethod = 1;
    checkThrows();
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidFilterMethod)
{
    inputData.filterMethod = 1;
    checkThrows();
}

TEST_F(PngChunkIHDR, ThrowsOnInvalidInterlaceMethod)
{
    inputData.interlaceMethod = 2;
    checkThrows();
}
