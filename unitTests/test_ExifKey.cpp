// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/tags.hpp>

#include <cstring>
#include <sstream>

using namespace Exiv2;

namespace {
void expectInteroperabilityVersion(const ExifKey& key) {
  std::ostringstream os;
  os << key;
  EXPECT_EQ(os.str(), "Exif.Iop.InteroperabilityVersion");
  EXPECT_STREQ(key.familyName(), "Exif");
  EXPECT_EQ(key.groupName(), "Iop");
  EXPECT_EQ(key.tagName(), "InteroperabilityVersion");
  EXPECT_EQ(key.tag(), 0x0002);
  EXPECT_STREQ(ExifTags::ifdName(key.groupName()), "Iop");
  EXPECT_STREQ(ExifTags::sectionName(key), "Interoperability");
}
}  // namespace

TEST(ExifKey, partsOfAKnownKey) {
  ExifKey key("Exif.Iop.InteroperabilityVersion");
  expectInteroperabilityVersion(key);
}

TEST(ExifKey, copyConstructor) {
  ExifKey key("Exif.Iop.InteroperabilityVersion");
  ExifKey copy(key);
  expectInteroperabilityVersion(copy);
}

TEST(ExifKey, hexTagIsConvertedToTagName) {
  ExifKey key("Exif.Image.0x0110");
  EXPECT_EQ(key.key(), "Exif.Image.Model");
  EXPECT_EQ(key.tagName(), "Model");
}

TEST(ExifKey, hexTagIsConvertedToTagNameInMakernoteGroup) {
  ExifKey key("Exif.Nikon3.0x0007");
  EXPECT_EQ(key.key(), "Exif.Nikon3.Focus");
  EXPECT_EQ(key.tagName(), "Focus");
}
