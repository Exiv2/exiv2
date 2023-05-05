// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <image_int.hpp>

using namespace Exiv2::Internal;
using Exiv2::makeSlice;
using Exiv2::Slice;

static const unsigned char b[10] = {'a', 'b', 'c', 1, 4, 0, 'e', 136, 0, 'a'};

template <typename T>
void checkBinaryToString(Exiv2::Slice<T>&& sl, const char* expectedOutput) {
  // construct the helper manually so that we catch potential invalidation of
  // temporaries
  std::stringstream ss;
  auto helper = binaryToString(std::move(sl));
  ss << helper;

  ASSERT_STREQ(ss.str().c_str(), expectedOutput);
}

TEST(binaryToString, zeroStart) {
  // a, b, c are printable, 1 & 4 are not => '.', 0 at last position => skipped
  checkBinaryToString(makeSlice(b, 0, 6), "abc..");

  // same as previous, but now last element is not ignored since it is not 0
  checkBinaryToString(makeSlice(b, 0, 5), "abc..");

  // same as first, only now the 0 & 136 are converted to '.'
  checkBinaryToString(makeSlice(b, 0, 8), "abc...e.");

  // should result in the same as previously, as trailing zero is ignored
  checkBinaryToString(makeSlice(b, 0, 9), "abc...e.");

  // ensure that the function does not overread when last element != 0
  checkBinaryToString(makeSlice(b, 0, sizeof(b)), "abc...e..a");
}

TEST(binaryToString, nonZeroStart) {
  // start @ index 1, read 6 characters (until e)
  checkBinaryToString(makeSlice(b, 1, 7), "bc...e");

  // start @ index 3, read until end
  checkBinaryToString(makeSlice(b, 3, sizeof(b)), "...e..a");
}
