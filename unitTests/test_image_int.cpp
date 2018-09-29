#include "gtestwrapper.h"

#include <image_int.hpp>

using namespace Exiv2::Internal;

static const unsigned char buf[10] = {'a', 'b', 'c', 1, 4, 0, 'e', 136, 0, 'a'};

TEST(binaryToString, zeroStart)
{
    // a, b, c are printable, 1 & 4 are not => '.', 0 at last position => skipped
    ASSERT_STREQ(binaryToString(buf, 6, 0).c_str(), "abc..");

    // same as previous, but now last element is not ignored since it is not 0
    ASSERT_STREQ(binaryToString(buf, 5, 0).c_str(), "abc..");

    // same as first, only now the 0 & 136 are converted to '.'
    ASSERT_STREQ(binaryToString(buf, 8, 0).c_str(), "abc...e.");

    // should result in the same as previously, as trailing zero is ignored
    ASSERT_STREQ(binaryToString(buf, 9, 0).c_str(), "abc...e.");

    // ensure that the function does not overread when last element != 0
    ASSERT_STREQ(binaryToString(buf, sizeof(buf), 0).c_str(), "abc...e..a");
}

TEST(binaryToString, nonZeroStart)
{
    // start @ index 1, read 6 characters (until e)
    ASSERT_STREQ(binaryToString(buf, 6, 1).c_str(), "bc...e");

    // start @ index 3, read until end
    ASSERT_STREQ(binaryToString(buf, sizeof(buf) - 3, 3).c_str(), "...e..a");
}

TEST(binaryToString, DataBuf)
{
    const Exiv2::DataBuf data_buf(buf, sizeof(buf));

    // same as first case in zeroStart
    ASSERT_EQ(binaryToString(data_buf, 6, 0), "abc..");

    // try to pass a too large value for size
    ASSERT_EQ(binaryToString(data_buf, 200, 0), "abc...e..a");

    // make it blow up by setting start larger than zero...
    ASSERT_EQ(binaryToString(data_buf, 200, 1), "bc...e..a");
}
