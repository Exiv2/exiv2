#include <exiv2/types.hpp>

#include <clocale>
#include <cmath>
#include <limits>
#include <system_error>

#include <gtest/gtest.h>

using namespace Exiv2;

// More info about tm : http://www.cplusplus.com/reference/ctime/tm/

TEST(ExivTime, getsTimeFromValidString)
{
    struct tm tmInstance;
    ASSERT_EQ(0, exifTime("2007:05:24 12:31:55", &tmInstance));
    ASSERT_EQ(107,  tmInstance.tm_year); // Years since 1900
    ASSERT_EQ(4,    tmInstance.tm_mon);
    ASSERT_EQ(24,   tmInstance.tm_mday);
    ASSERT_EQ(12,   tmInstance.tm_hour);
    ASSERT_EQ(31,   tmInstance.tm_min);
    ASSERT_EQ(55,   tmInstance.tm_sec);
}

TEST(ExivTime, doesNotGetTimeWithBadFormedString)
{
    struct tm tmInstance;
    ASSERT_EQ(1, exifTime("007:a5:24 aa:bb:cc", &tmInstance));
}

TEST(DataBuf, pointsToNullByDefault)
{
    DataBuf instance;
    ASSERT_EQ(nullptr, instance.pData_);
    ASSERT_EQ(0,    instance.size_);
}

TEST(DataBuf, allocatesDataWithNonEmptyConstructor)
{
    DataBuf instance (5);
    ASSERT_NE(static_cast<byte *>(nullptr), instance.pData_); /// \todo use nullptr once we move to c++11
    ASSERT_EQ(5,    instance.size_);
}

TEST(Rational, floatToRationalCast)
{
    static const float floats[] = {0.5f, 0.015f, 0.0000625f};

    for (size_t i = 0; i < sizeof(floats) / sizeof(*floats); ++i) {
        const Rational r = floatToRationalCast(floats[i]);
        const float fraction = static_cast<float>(r.first) / static_cast<float>(r.second);
        ASSERT_TRUE(fabs((floats[i] - fraction) / floats[i]) < 0.01f);
    }

    const Rational plus_inf = floatToRationalCast(std::numeric_limits<float>::infinity());
    ASSERT_EQ(plus_inf.first, 1);
    ASSERT_EQ(plus_inf.second, 0);

    const Rational minus_inf = floatToRationalCast(-1 * std::numeric_limits<float>::infinity());
    ASSERT_EQ(minus_inf.first, -1);
    ASSERT_EQ(minus_inf.second, 0);
}

struct WideStrTest : public ::testing::Test
{
    WideStrTest()
    {
#if (!defined(_MSC_VER)) && (!defined(__CYGWIN__)) && (!defined(__MINGW__))
        if (std::setlocale(LC_ALL, "en_US.UTF-8") == nullptr) {
            if (std::setlocale(LC_ALL, "en_US") == nullptr) {
                throw std::system_error(errno, std::system_category(),
                                        "Could not set locale to either 'en_US.UTF-8' or 'en_US'");
            }
        }
#endif
    }

    ~WideStrTest()
    {
#if (!defined(_MSC_VER)) && (!defined(__CYGWIN__)) && (!defined(__MINGW__))
        std::setlocale(LC_ALL, "C");
#endif
    }
};

TEST_F(WideStrTest, asciiWideStringToUtf8)
{
    const std::wstring wStr{L"this is an ordinary ASCII string"};
    const std::string res = ws2s(wStr);

    ASSERT_STREQ(res.c_str(), "this is an ordinary ASCII string");
}

TEST_F(WideStrTest, UtfWideStringToUtf8)
{
    const std::wstring wStr{L"This String cöntains Special characters: zß水"};
    const std::string res = ws2s(wStr);

    // clang-format off
    // clang format adds a space between u8 and the string
    ASSERT_STREQ(res.c_str(), u8"This String cöntains Special characters: zß水");
    // clang-format on
}

TEST_F(WideStrTest, invalidUnicodeWideStringToUtf8)
{
    const std::wstring wStr{L"\000"};
    const std::string res = ws2s(wStr);

    ASSERT_STREQ(res.c_str(), "");
}

TEST_F(WideStrTest, AsciiStringToWide)
{
    const std::string str{"this is an ordinary ASCII string"};
    const std::wstring res = s2ws(str);

    ASSERT_STREQ(res.c_str(), L"this is an ordinary ASCII string");
}

TEST_F(WideStrTest, Utf8WithSpecialCharsToWide)
{
    // clang-format off
    // clang format adds a space between u8 and the string
    const std::string str{u8"This String cöntains Special characters: zß水"};
    // clang-format on
    const std::wstring res = s2ws(str);

    ASSERT_STREQ(res.c_str(), L"This String cöntains Special characters: zß水");
}

TEST_F(WideStrTest, invalidUtf8ToWide)
{
    const std::string str{"\000"};
    const std::wstring res = s2ws(str);

    ASSERT_STREQ(res.c_str(), L"");
}
