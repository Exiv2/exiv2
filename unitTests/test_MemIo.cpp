#include <exiv2/basicio.hpp>  // SUT
#include <exiv2/error.hpp>

#include <gtest/gtest.h>
#include <array>

using namespace Exiv2;

namespace
{
    struct DefaultMemIo : public testing::Test
    {
        MemIo io{};
    };

    struct BlockMemIo : public testing::Test
    {
        const std::array<byte, 8> buf{1, 2, 3, 4, 5, 6, 7, 8};
        MemIo io{buf.data(), buf.size()};
    };

    // Regression tests for bug reported in https://github.com/Exiv2/exiv2/pull/944
    struct MemIoOutOfBounds : public testing::Test
    {
        void SetUp() override
        {
            ASSERT_FALSE(io.eof());
        }

        std::array<byte, 1024> buf;
        std::array<byte, 16> tmp;
        MemIo io{buf.data(), buf.size()};
    };
}  // namespace

TEST_F(DefaultMemIo, isAlwaysOpen)
{
    ASSERT_TRUE(io.isopen());
    ASSERT_EQ(0, io.open());   // cannot be opened
    ASSERT_EQ(0, io.close());  // cannot be closed
    ASSERT_EQ(0, io.error());  // no erros
    ASSERT_TRUE(io.isopen());
}

TEST_F(DefaultMemIo, expectedValuesForAccessors)
{
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(0, io.size());
    ASSERT_FALSE(io.eof());
    ASSERT_EQ("MemIo", io.path());
}

TEST_F(DefaultMemIo, isResizedAfterWrite)
{
    const std::array<byte, 4> buff{1, 2, 3, 4};
    ASSERT_EQ(buff.size(), io.write(buff.data(), buff.size()));  // 1st version of write()
    ASSERT_EQ(buff.size(), io.size());

    io.seek(0, BasicIo::beg);  // Need to reset the pos, otherwise we do not copy anything

    MemIo io2{};
    ASSERT_EQ(buff.size(), io2.write(io));  // 2nd version of write()
    ASSERT_EQ(buff.size(), io2.size());
}

TEST_F(DefaultMemIo, isResizedByPutb)
{
    ASSERT_EQ(6, io.putb(6));
    ASSERT_EQ(1, io.size());

    ASSERT_EQ(7, io.putb(7));
    ASSERT_EQ(2, io.size());
}

TEST_F(DefaultMemIo, getbReturnsEof)
{
    ASSERT_EQ(EOF, io.getb());
}

TEST_F(DefaultMemIo, getbWorksAfterWritingSomethingAndResetingThePos)
{
    ASSERT_EQ(6, io.putb(6));
    io.seek(0, BasicIo::beg);
    ASSERT_EQ(6, io.getb());
}

TEST_F(DefaultMemIo, transfer)
{
    const std::array<byte, 4> buff{1, 2, 3, 4};
    io.write(buff.data(), buff.size());

    MemIo io2;
    io2.transfer(io);

    ASSERT_EQ(buff.size(), io2.size());
    ASSERT_EQ(0, io.size());
}

TEST_F(DefaultMemIo, mmapReturnsPointerToDataWhichIsNull)
{
    ASSERT_EQ(nullptr, io.mmap());
    ASSERT_EQ(0, io.munmap());
}

TEST_F(DefaultMemIo, populateFakeDataDoesNothing)
{
    ASSERT_NO_THROW(io.populateFakeData());
}

// ----------------------------------------------------------------------------

TEST_F(BlockMemIo, isAlwaysOpen)
{
    ASSERT_TRUE(io.isopen());
    ASSERT_EQ(0, io.open());   // cannot be opened
    ASSERT_EQ(0, io.close());  // cannot be closed
    ASSERT_EQ(0, io.error());  // no erros
    ASSERT_TRUE(io.isopen());
}

TEST_F(BlockMemIo, expectedValuesForAccessors)
{
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(buf.size(), io.size());
    ASSERT_FALSE(io.eof());
    ASSERT_EQ("MemIo", io.path());
}

TEST_F(BlockMemIo, writeIsNotAffectingInputBlock)
{
    const std::array<byte, 4> buf2{9, 9, 9, 9};
    ASSERT_EQ(buf2.size(), io.write(buf2.data(), buf2.size()));  // 1st version of write()
    ASSERT_EQ(buf.size(), io.size());

    const std::array<byte, 8> expected {9, 9, 9, 9, 5, 6, 7, 8};
    std::array<byte, 8> read{};

    io.seek(0, BasicIo::beg);
    ASSERT_EQ(read.size(), io.read(read.data(), read.size()));
    ASSERT_EQ(expected, read); // Mofifies the internal data
    ASSERT_NE(expected, buf);  // But not the input memory block
}

TEST_F(BlockMemIo, isResizedByPutb)
{
    io.seek(0, BasicIo::end);
    ASSERT_EQ(6, io.putb(6));
    ASSERT_EQ(buf.size() + 1, io.size());

    ASSERT_EQ(7, io.putb(7));
    ASSERT_EQ(buf.size() + 2, io.size());
}

TEST_F(BlockMemIo, readWorksWhenCountIsSmallerThanSize)
{
    const auto databuf = io.read(4);
    ASSERT_EQ(4, databuf.size_);
    ASSERT_TRUE(std::equal(databuf.cbegin(), databuf.cend(), buf.begin()));
}

TEST_F(BlockMemIo, readFailsWhenCountIsBiggerThanSize_versionDataBuf)
{
    const auto databuf = io.read(20);
    ASSERT_EQ(0, databuf.size_);
}

TEST_F(BlockMemIo, readFailsWhenCountIsBiggerThanSize_versionBuffer)
{
    std::array<byte, 20> buf;
    ASSERT_EQ(io.size(), io.read(buf.data(), buf.size()));
    ASSERT_TRUE(io.eof());
}

TEST_F(BlockMemIo, readOrThrowThrowsWhenCountIsBiggerThanSize)
{
    std::array<byte, 20> buf;
    ASSERT_THROW(io.readOrThrow(buf.data(), buf.size()), Error);
}

TEST_F(BlockMemIo, mmapReturnsPointerToInputBuffer)
{
    ASSERT_EQ(buf.data(), io.mmap());
    ASSERT_EQ(0, io.munmap());
}

TEST_F(BlockMemIo, canBeTransferred)
{
    MemIo io2;
    io2.transfer(io);

    ASSERT_EQ(buf.size(), io2.size());
    ASSERT_EQ(0, io.size());
}

// ----------------------------------------------------------------------------

TEST_F(MemIoOutOfBounds, seek_out_of_bounds_00)
{
    // The problem is that MemIo::seek() does not check that the new offset is in bounds.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::beg), 1);
    ASSERT_TRUE(io.eof());

    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp.data(), tmp.size()), tmp.size());
}

TEST_F(MemIoOutOfBounds, seek_out_of_bounds_01)
{
    // Seek to the end of the file.
    ASSERT_EQ(io.seek(0, BasicIo::end), 0);
    ASSERT_FALSE(io.eof());
    ASSERT_EQ(io.read(tmp.data(), tmp.size()), 0);

    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::end), 1);
    ASSERT_TRUE(io.eof());
    ASSERT_EQ(io.read(tmp.data(), tmp.size()), 0);
}

TEST_F(MemIoOutOfBounds, seek_out_of_bounds_02)
{
    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::cur), 1);
    ASSERT_TRUE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp.data(), tmp.size()), tmp.size());
}

TEST_F(MemIoOutOfBounds, seek_out_of_bounds_03)
{
    // Try to seek past the beginning of the file.
    ASSERT_EQ(io.seek(-0x10000000, BasicIo::cur), 1);
    ASSERT_FALSE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp.data(), tmp.size()), tmp.size());
}
