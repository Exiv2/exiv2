#include <exiv2/basicio.hpp> // SUT

#include <gtest/gtest.h>
#include <array>

using namespace Exiv2;

struct DefaultMemIo: public testing::Test
{
    MemIo io{};
};

TEST_F(DefaultMemIo, isAlwaysOpen)
{
    ASSERT_TRUE(io.isopen());
    ASSERT_EQ(0, io.open());    // cannot be opened
    ASSERT_EQ(0, io.close());   // cannot be closed
    ASSERT_EQ(0, io.error());   // no erros
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
    const std::array<byte, 4> buff{1,2,3,4};
    ASSERT_EQ(buff.size(), io.write(buff.data(), buff.size()));
    ASSERT_EQ(buff.size(), io.size());

    io.seek(0, BasicIo::beg);

    MemIo io2{};
    ASSERT_EQ(buff.size(), io2.write(io));
    ASSERT_EQ(buff.size(), io2.size());
}

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
