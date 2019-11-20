#include <exiv2/basicio.hpp> // SUT

#include <gtest/gtest.h>
#include <array>

using namespace Exiv2;

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
