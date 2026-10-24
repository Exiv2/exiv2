// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <sstream>
#include "canonmn_int.hpp"

TEST(MakerNoteConversions, keepsValidCanonSerialNumbers) {
  Exiv2::AsciiValue value;
  ASSERT_EQ(0, value.read("65537"));
  std::ostringstream output;
  Exiv2::Internal::CanonMakerNote::print0x000a(output, value, nullptr);
  EXPECT_EQ("000100001", output.str());
}

TEST(MakerNoteConversions, doesNotTruncateOversizedCanonSerialNumbers) {
  Exiv2::AsciiValue value;
  ASSERT_EQ(0, value.read("4294967296"));
  std::ostringstream output;
  Exiv2::Internal::CanonMakerNote::print0x000a(output, value, nullptr);
  EXPECT_EQ("4294967296", output.str());
}

TEST(MakerNoteConversions, doesNotTruncateOversizedD30SerialNumbers) {
  Exiv2::AsciiValue value;
  ASSERT_EQ(0, value.read("4294967296"));
  Exiv2::ExifData metadata;
  metadata["Exif.Canon.ModelID"] = uint32_t{0x01140000};
  std::ostringstream output;
  Exiv2::Internal::CanonMakerNote::print0x000c(output, value, &metadata);
  EXPECT_EQ("4294967296", output.str());
}

TEST(MakerNoteConversions, printsCasioFirmwareDateWithHighBitTrailingBytes) {
  for (const int trailingByte : {0, 0x80, 0xff}) {
    SCOPED_TRACE(trailingByte);
    std::string firmwareDate = "2401011200";
    firmwareDate.push_back(static_cast<char>(trailingByte));
    Exiv2::AsciiValue value;
    ASSERT_EQ(0, value.read(firmwareDate));
    Exiv2::ExifData metadata;
    auto& datum = metadata["Exif.Casio.FirmwareDate"];
    datum.setValue(&value);
    EXPECT_EQ("2024:01:01 12:00", datum.print(&metadata));
  }
}

TEST(MakerNoteConversions, printsCasio2FirmwareDateWithHighBitTrailingBytes) {
  for (const int trailingByte : {0, 0x80, 0xff}) {
    SCOPED_TRACE(trailingByte);
    std::string firmwareDate = "2401011200";
    firmwareDate.push_back(static_cast<char>(trailingByte));
    Exiv2::AsciiValue value;
    ASSERT_EQ(0, value.read(firmwareDate));
    Exiv2::ExifData metadata;
    auto& datum = metadata["Exif.Casio2.FirmwareDate"];
    datum.setValue(&value);
    EXPECT_EQ("2024:01:01 12:00", datum.print(&metadata));
  }
}
