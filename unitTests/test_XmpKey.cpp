// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

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
    XmpKey::UniquePtr clonedKey = key.clone();
    checkValidity(*clonedKey);
}

TEST_F(AXmpKey, throwsWithNotRegisteredWellFormedKey)
{
    ASSERT_THROW(XmpKey key(notRegisteredValidKey), std::exception);
}

TEST_F(AXmpKey, throwsWithNotRegisteredPrefix)
{
    ASSERT_THROW(XmpKey key("badPrefix", expectedProperty), std::exception);
}

TEST_F(AXmpKey, throwsWithBadFormedKey)
{
    ASSERT_THROW(XmpKey key(expectedProperty), std::exception);  // It should have the format ns.prefix.key
}
