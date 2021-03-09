#include <exiv2/exiv2.hpp>

#include "gtestwrapper.h"

using namespace Exiv2;

// The tests corrispond to those in issue https://github.com/Exiv2/exiv2/issues/1481


// 1. No language value
TEST(LangAltValueReadTest, noLangugeValBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang= test1-1";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, quoteThenNoLangugeValBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=\" test1-2";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

// 2. Empty language value
TEST(LangAltValueReadTest, emptyDoubleQuotesLanguageValBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=\"\" test2";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

// 3. Mismatched and/or incorrect positioning of quotation marks
TEST(LangAltValueReadTest, emptyDoubleQuotesLanguageValNoSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=\"\"test3-1";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, oneDoubleQuotesLanguageValNoSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=\"test3-2";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, oneDoubleQuotesLanguageValBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=\"en-UK test3-3";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, languageValOneDoubleQuotesBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=en-US\" test3-4";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, languageValOneDoubleQuotesNoSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=test3-5\"";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, languageValTwoDoubleQuotesNoSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=test3-6\"\"";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

// 4. Invalid characters in language part
TEST(LangAltValueReadTest, languageValExtraHyphenBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=en-UK- test4-1";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}

TEST(LangAltValueReadTest, languageValWithInvalidCharBeforeSpace)
{
    XmpParser::initialize();
    ::atexit(XmpParser::terminate);
 
    Exiv2::XmpData xmpData;
    try {
        xmpData["Xmp.dc.title"] = "lang=en=UK test4-2";
    }
    catch (AnyError& e) {
        ASSERT_EQ(e.code(),Exiv2::kerInvalidLangAltValue);
    }
    catch (...) {
        ASSERT_TRUE(false);
    }
}
