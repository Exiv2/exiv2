#include <exiv2/basicio.hpp>

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(MemIo, seek_out_of_bounds)
{
    byte buf[1024];
    memset(buf, 0, sizeof(buf));

    MemIo io(buf, sizeof(buf));
    ASSERT_FALSE(io.eof());

    // Regression test for bug reported in https://github.com/Exiv2/exiv2/pull/945
    // The problem is that MemIo::seek() does not check that the new offset is
    // in bounds.
    byte tmp[16];
    ASSERT_EQ(io.seek(0x10000000, BasicIo::beg), 1);
    ASSERT_TRUE(io.eof());

    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), sizeof(tmp));

    // Reset the file.
    ASSERT_EQ(io.seek(0, BasicIo::beg), 0);
    ASSERT_FALSE(io.eof());

    // Seek to the end of the file.
    ASSERT_EQ(io.seek(0, BasicIo::end), 0);
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), 0);

    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::end), 1);
    ASSERT_TRUE(io.eof());
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), 0);

    // Reset the file.
    ASSERT_EQ(io.seek(100, BasicIo::beg), 0);
    ASSERT_FALSE(io.eof());

    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::cur), 1);
    ASSERT_TRUE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), sizeof(tmp));

    // Reset the file.
    ASSERT_EQ(io.seek(100, BasicIo::beg), 0);
    ASSERT_FALSE(io.eof());

    // Try to seek past the beginning of the file.
    ASSERT_EQ(io.seek(-0x10000000, BasicIo::cur), 1);
    ASSERT_FALSE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), sizeof(tmp));
}
