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
    io.seek(0x10000000, BasicIo::beg);
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), 0);
}
