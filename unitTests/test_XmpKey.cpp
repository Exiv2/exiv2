#include <exiv2/error.hpp>
#include <exiv2/properties.hpp>

#include "gtestwrapper.h"

using namespace Exiv2;

namespace
{
    const std::string expectedFamily("Xmp");
    const std::string expectedPrefix("prefix");
    const std::string expectedProperty("prop");
    const std::string expectedKey(expectedFamily + "." + expectedPrefix + "." + expectedProperty);
    const std::string notRegisteredValidKey("Xmp.noregistered.prop");
}

// Test Fixture which register a namespace with a prefix. This is needed to test the correct
// behavior of the XmpKey class
class AXmpKey : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        XmpProperties::registerNs(expectedFamily, expectedPrefix);
    }

    static void TearDownTestCase()
    {
        XmpProperties::unregisterNs();
    }

    void checkValidity(const XmpKey& key)
    {
        ASSERT_EQ(expectedKey, key.key());
        ASSERT_EQ(expectedFamily, key.familyName());
        ASSERT_EQ(expectedPrefix, key.groupName());
        ASSERT_EQ(expectedProperty, key.tagName());
        ASSERT_EQ(expectedProperty, key.tagLabel());
        ASSERT_EQ(0, key.tag());
        ASSERT_STREQ("Xmp/", key.ns().c_str());
    }
};

TEST_F(AXmpKey, correctlyInstantiateWithValidKey)
{
    XmpKey key(expectedKey);
    checkValidity(key);
}

TEST_F(AXmpKey, correctlyInstantiatedWithValidPrefixAndProperty)
{
    XmpKey key(expectedPrefix, expectedProperty);
    checkValidity(key);
}

TEST_F(AXmpKey, canBeCopiedConstructed)
{
    XmpKey key(expectedPrefix, expectedProperty);
    XmpKey copiedKey(key);
    checkValidity(copiedKey);
}

TEST_F(AXmpKey, canBeCopied)
{
    XmpKey key(expectedPrefix, expectedProperty);
    XmpKey copiedKey("Xmp.prefix.prop2");
    copiedKey = key;
    checkValidity(copiedKey);
}

TEST_F(AXmpKey, canBeCloned)
{
    XmpKey key(expectedPrefix, expectedProperty);
    XmpKey::AutoPtr clonedKey = key.clone();
    checkValidity(*clonedKey);
}

TEST_F(AXmpKey, throwsWithNotRegisteredWellFormedKey)
{
    ASSERT_THROW(XmpKey key(notRegisteredValidKey), Error);
}

TEST_F(AXmpKey, throwsWithNotRegisteredPrefix)
{
    ASSERT_THROW(XmpKey key("badPrefix", expectedProperty), Error);
}

TEST_F(AXmpKey, throwsWithBadFormedKey)
{
    ASSERT_THROW(XmpKey key(expectedProperty), Error);  // It should have the format ns.prefix.key
}
