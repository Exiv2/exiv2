#include "gtestwrapper.h"

#include <image_int.hpp>

using namespace Exiv2::Internal;
using Exiv2::makeSlice;
using Exiv2::Slice;

static const unsigned char buf[10] = {'a', 'b', 'c', 1, 4, 0, 'e', 136, 0, 'a'};

template <typename T>
void checkBinaryToString(const Exiv2::Slice<T> sl, const char* expectedOutput)
{
    // construct the helper manually so that we catch potential invalidation of
    // temporaries
    std::stringstream ss;
    const binaryToStringHelper<T> helper = binaryToString(sl);
    ss << helper;

    ASSERT_STREQ(ss.str().c_str(), expectedOutput);
}

TEST(binaryToString, zeroStart)
{
    // a, b, c are printable, 1 & 4 are not => '.', 0 at last position => skipped
    checkBinaryToString(makeSlice(buf, 0, 6), "abc..");

    // same as previous, but now last element is not ignored since it is not 0
    checkBinaryToString(makeSlice(buf, 0, 5), "abc..");

    // same as first, only now the 0 & 136 are converted to '.'
    checkBinaryToString(makeSlice(buf, 0, 8), "abc...e.");

    // should result in the same as previously, as trailing zero is ignored
    checkBinaryToString(makeSlice(buf, 0, 9), "abc...e.");

    // ensure that the function does not overflow when last element != 0
    checkBinaryToString(makeSlice(buf, 0, sizeof(buf)), "abc...e..a");
}

TEST(binaryToString, nonZeroStart)
{
    // start @ index 1, read 6 characters (until e)
    checkBinaryToString(makeSlice(buf, 1, 7), "bc...e");

    // start @ index 3, read until end
    checkBinaryToString(makeSlice(buf, 3, sizeof(buf)), "...e..a");
}

TEST(stringFormat, badInitialGuessOfBufferSize)
{
    const char fmt[] = "%s";
    const char str[] = "Long string with more than 16 characters.";
    ASSERT_EQ(stringFormat(fmt, str), std::string(str));
}
