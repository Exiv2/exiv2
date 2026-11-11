// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

// https://github.com/Exiv2/exiv2/issues/9562
TEST(Image, appendIccProfileFromItsOwnProfile) {
  const std::string seed = "ICC_PROFILE_SELF_ALIAS_TEST\n";
  auto image = ImageFactory::create(ImageType::jpeg);
  image->setIccProfile(DataBuf(reinterpret_cast<const byte*>(seed.data()), seed.size()), false);

  const DataBuf& profile = image->iccProfile();
  image->appendIccProfile(profile.c_data(), profile.size(), false);

  const std::string expected = seed + seed;
  ASSERT_EQ(image->iccProfile().size(), expected.size());
  EXPECT_EQ(0, image->iccProfile().cmpBytes(0, expected.data(), expected.size()));
}

// https://github.com/Exiv2/exiv2/issues/9562
TEST(Image, appendIccProfileFromMiddleOfItsOwnProfile) {
  const std::string seed = "0123456789";
  auto image = ImageFactory::create(ImageType::jpeg);
  image->setIccProfile(DataBuf(reinterpret_cast<const byte*>(seed.data()), seed.size()), false);

  const std::string addition = "ABCDEFGHIJ";
  image->appendIccProfile(reinterpret_cast<const byte*>(addition.data()), addition.size(), false);

  const DataBuf& profile = image->iccProfile();
  image->appendIccProfile(profile.c_data(2), 5, false);

  const std::string expected = seed + addition + seed.substr(2, 5);
  ASSERT_EQ(image->iccProfile().size(), expected.size());
  EXPECT_EQ(0, image->iccProfile().cmpBytes(0, expected.data(), expected.size()));
}
