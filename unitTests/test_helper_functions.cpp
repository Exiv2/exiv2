// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "helper_functions.hpp"

TEST(stringFromUnterminated, terminatedArray) {
  const char data[5] = {'a', 'b', 'c', 0, 'd'};
  const std::string res = string_from_unterminated(data, 5);

  ASSERT_EQ(res.size(), 3U);
  ASSERT_STREQ(res.c_str(), "abc");
}

TEST(stringFromUnterminated, unterminatedArray) {
  const char data[4] = {'a', 'b', 'c', 'd'};
  const std::string res = string_from_unterminated(data, 4);

  ASSERT_EQ(res.size(), 4U);
  ASSERT_STREQ(res.c_str(), "abcd");
}
