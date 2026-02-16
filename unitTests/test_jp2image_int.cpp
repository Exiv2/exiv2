// SPDX-License-Identifier: GPL-2.0-or-later

#include "jp2image_int.hpp"  // Internals of JPEG-2000 standard

#include <gtest/gtest.h>

using namespace Exiv2::Internal;

namespace {
void setValidValues(std::vector<uint8_t>& boxData) {
  // The first 4 bytes correspond to the BR (Brand). It must have the value 'jp2\040'
  boxData[0] = 'j';
  boxData[1] = 'p';
  boxData[2] = '2';
  boxData[3] = '\040';

  // The next 4 bytes correspond to the MinV (Minor version). It is a 4-byte unsigned int with value 0

  // The only available Compatibility list also has the value 'jp2\040'
  boxData[8] = 'j';
  boxData[9] = 'p';
  boxData[10] = '2';
  boxData[11] = '\040';
}
}  // namespace

TEST(Jp2_FileTypeBox, isNotValidWithoutProperValuesSet) {
  const std::vector<uint8_t> boxData(12);
  ASSERT_FALSE(isValidBoxFileType(boxData));
}

TEST(Jp2_FileTypeBox, isValidWithMinimumPossibleSizeAndValidValues) {
  std::vector<uint8_t> boxData(12);
  setValidValues(boxData);
  ASSERT_TRUE(isValidBoxFileType(boxData));
}

TEST(Jp2_FileTypeBox, isNotValidWithMinimumPossibleSizeButInvalidBrand) {
  std::vector<uint8_t> boxData(12);
  setValidValues(boxData);
  boxData[2] = '3';  // Change byte in the brand field

  ASSERT_FALSE(isValidBoxFileType(boxData));
}

TEST(Jp2_FileTypeBox, isNotValidWithMinimumPossibleSizeButInvalidCL1) {
  std::vector<uint8_t> boxData(12);
  setValidValues(boxData);
  boxData[10] = '3';  // Change byte in the CL1

  ASSERT_FALSE(isValidBoxFileType(boxData));
}

// ----------------------------------------------------------

TEST(Jp2_FileTypeBox, withInvalidBoxDataSizeIsInvalid) {
  std::vector<uint8_t> boxData(13);  // 12 + 1 (the extra byte causes problems)
  ASSERT_FALSE(isValidBoxFileType(boxData));
}

TEST(Jp2_FileTypeBox, withSmallBoxDataSizeIsInvalid) {
  std::vector<uint8_t> boxData(7);  // Minimum size is 8
  ASSERT_FALSE(isValidBoxFileType(boxData));
}

TEST(Jp2_FileTypeBox, with2CLs_lastOneWithBrandValue_isValid) {
  std::vector<uint8_t> boxData(16);
  // The first 4 bytes correspond to the BR (Brand). It must have the value 'jp2\040'
  boxData[0] = 'j';
  boxData[1] = 'p';
  boxData[2] = '2';
  boxData[3] = '\040';

  // The next 4 bytes correspond to the MinV (Minor version). It is a 4-byte unsigned int with value 0

  // The 2nd Compatibility list has the value 'jp2\040'
  boxData[12] = 'j';
  boxData[13] = 'p';
  boxData[14] = '2';
  boxData[15] = '\040';

  ASSERT_TRUE(isValidBoxFileType(boxData));
}
