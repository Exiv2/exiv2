// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "value.hpp"
using namespace Exiv2;

TEST(ATimeValue, isDefaultConstructed) {
  const TimeValue value;
  ASSERT_EQ(0, value.getTime().hour);
  ASSERT_EQ(0, value.getTime().minute);
  ASSERT_EQ(0, value.getTime().second);
  ASSERT_EQ(0, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, isConstructedWithArgs) {
  const TimeValue value(23, 55, 2);
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(2, value.getTime().second);
  ASSERT_EQ(0, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);
}

/// \todo add tests to check what happen with values out of valid ranges

TEST(ATimeValue, canBeReadFromCompleteBasicFormatString) {
  TimeValue value;
  const std::string hms("235502");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(2, value.getTime().second);
}

TEST(ATimeValue, canBeReadFromReducedBasicFormatStringHHMM) {
  TimeValue value;
  const std::string hms("2355");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(0, value.getTime().second);
}

TEST(ATimeValue, canBeReadFromReducedBasicFormatStringHH) {
  TimeValue value;
  const std::string hms("23");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(0, value.getTime().minute);
  ASSERT_EQ(0, value.getTime().second);
}

TEST(ATimeValue, canBeReadFromCompleteExtendedFormatString) {
  TimeValue value;
  const std::string hms("23:55:02");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(2, value.getTime().second);
}

TEST(ATimeValue, canBeReadFromReducedExtendedFormatStringHHMM) {
  TimeValue value;
  const std::string hms("23:55");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(0, value.getTime().second);
}

TEST(ATimeValue, canBeReadFromBasicStringWithTimeZoneDesignatorPositive) {
  TimeValue value;
  std::string hms("152746+0100");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(15, value.getTime().hour);
  ASSERT_EQ(27, value.getTime().minute);
  ASSERT_EQ(46, value.getTime().second);
  ASSERT_EQ(1, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);

  value = TimeValue();
  hms = "152746+02";
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(15, value.getTime().hour);
  ASSERT_EQ(27, value.getTime().minute);
  ASSERT_EQ(46, value.getTime().second);
  ASSERT_EQ(2, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, canBeReadFromExtendedStringWithTimeZoneDesignatorPositive) {
  TimeValue value;
  std::string hms("23:55:02+04:04");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(2, value.getTime().second);
  ASSERT_EQ(4, value.getTime().tzHour);
  ASSERT_EQ(4, value.getTime().tzMinute);

  value = TimeValue();
  hms = "23:44:03+04";
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(44, value.getTime().minute);
  ASSERT_EQ(3, value.getTime().second);
  ASSERT_EQ(4, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, canBeReadFromExtendedStringWithTimeZoneDesignatorNegative) {
  TimeValue value;
  std::string hms("23:55:02-04:04");
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(55, value.getTime().minute);
  ASSERT_EQ(2, value.getTime().second);
  ASSERT_EQ(-4, value.getTime().tzHour);
  ASSERT_EQ(-4, value.getTime().tzMinute);

  value = TimeValue();
  hms = "23:44:03-04";
  ASSERT_EQ(0, value.read(hms));
  ASSERT_EQ(23, value.getTime().hour);
  ASSERT_EQ(44, value.getTime().minute);
  ASSERT_EQ(3, value.getTime().second);
  ASSERT_EQ(-4, value.getTime().tzHour);
  ASSERT_EQ(0, value.getTime().tzMinute);
}

TEST(ATimeValue, cannotBeReadFromStringWithTimeZoneDesignatorWithoutSymbol) {
  TimeValue value;
  const std::string hms("23:55:02?04:04");
  ASSERT_EQ(1, value.read(hms));
}

TEST(ATimeValue, cannotReadFromStringWithBadFormat) {
  TimeValue value;
  ASSERT_EQ(1, value.read("aa:55:02"));        // String with non-digit chars
  ASSERT_EQ(1, value.read("25:55:02"));        // Hours >= 24
  ASSERT_EQ(1, value.read("23:65:02"));        // Minutes >= 60
  ASSERT_EQ(1, value.read("23:55:62"));        // Seconds >= 60
  ASSERT_EQ(1, value.read("23:55:02+25:04"));  // tzHour >= 24
  ASSERT_EQ(1, value.read("23:55:02+04:66"));  // tzMinutes >= 60

  /// \todo This one does not fail
  // ASSERT_EQ(1, value.read("23:55:02+04:06:06")); // More components than expected
}

TEST(ATimeValue, isCopiedToBuffer) {
  const TimeValue value(23, 55, 2);
  byte buffer[11];
  ASSERT_EQ(11, value.copy(buffer));

  const byte expectedDate[11] = {'2', '3', '5', '5', '0', '2', '+', '0', '0', '0', '0'};
  for (int i = 0; i < 11; ++i) {
    ASSERT_EQ(expectedDate[i], buffer[i]) << "i: " << i;
  }
}
