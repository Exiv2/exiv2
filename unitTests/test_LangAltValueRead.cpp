// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

// The tests correspond to those in issue https://github.com/Exiv2/exiv2/issues/1481

// 1. No language value
TEST(LangAltValueReadTest, noLanguageValBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang= test1-1";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, quoteThenNoLanguageValBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=\" test1-2";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

// 2. Empty language value
TEST(LangAltValueReadTest, emptyDoubleQuotesLanguageValBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=\"\" test2";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

// 3. Mismatched and/or incorrect positioning of quotation marks
TEST(LangAltValueReadTest, emptyDoubleQuotesLanguageValNoSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=\"\"test3-1";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, oneDoubleQuotesLanguageValNoSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=\"test3-2";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, oneDoubleQuotesLanguageValBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=\"en-UK test3-3";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, languageValOneDoubleQuotesBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=en-US\" test3-4";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, languageValOneDoubleQuotesNoSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=test3-5\"";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, languageValTwoDoubleQuotesNoSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=test3-6\"\"";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

// 4. Invalid characters in language part
TEST(LangAltValueReadTest, languageValExtraHyphenBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=en-UK- test4-1";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(LangAltValueReadTest, languageValWithInvalidCharBeforeSpace) {
  Exiv2::XmpData xmpData;
  try {
    xmpData["Xmp.dc.title"] = "lang=en=UK test4-2";
  } catch (Error& e) {
    ASSERT_EQ(e.code(), Exiv2::ErrorCode::kerInvalidLangAltValue);
  } catch (...) {
    ASSERT_TRUE(false);
  }
}
