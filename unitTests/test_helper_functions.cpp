#include "helper_functions.hpp"

#include "gtestwrapper.h"

TEST(string_from_unterminated, terminatedArray)
{
    const char data[5] = {'a', 'b', 'c', 0, 'd'};
    const std::string res = string_from_unterminated(data, 5);

    ASSERT_EQ(res.size(), 3);
    ASSERT_STREQ(res.c_str(), "abc");
}

TEST(string_from_unterminated, unterminatedArray)
{
    const char data[4] = {'a', 'b', 'c', 'd'};
    const std::string res = string_from_unterminated(data, 4);

    ASSERT_EQ(res.size(), 4);
    ASSERT_STREQ(res.c_str(), "abcd");
}
