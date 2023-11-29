// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/types.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

using namespace Exiv2;

// More info about tm : http://www.cplusplus.com/reference/ctime/tm/

TEST(ExivTime, getsTimeFromValidString) {
  tm tmInstance;
  ASSERT_EQ(0, exifTime("2007:05:24 12:31:55", &tmInstance));
  ASSERT_EQ(107, tmInstance.tm_year);  // Years since 1900
  ASSERT_EQ(4, tmInstance.tm_mon);
  ASSERT_EQ(24, tmInstance.tm_mday);
  ASSERT_EQ(12, tmInstance.tm_hour);
  ASSERT_EQ(31, tmInstance.tm_min);
  ASSERT_EQ(55, tmInstance.tm_sec);
}

TEST(ExivTime, doesNotGetTimeWithBadFormedString) {
  tm tmInstance;
  ASSERT_EQ(1, exifTime("007:a5:24 aa:bb:cc", &tmInstance));
}

TEST(DataBuf, defaultInstanceIsEmpty) {
  DataBuf instance;
  ASSERT_TRUE(instance.empty());
}

TEST(DataBuf, allocatesDataWithNonEmptyConstructor) {
  DataBuf instance(5);
  ASSERT_NE(nullptr, instance.c_data());
  ASSERT_EQ(5, instance.size());
}

TEST(DataBuf, canBeConstructedFromExistingData) {
  const std::array<byte, 4> data{'h', 'o', 'l', 'a'};
  DataBuf instance(data.data(), data.size());
  ASSERT_TRUE(std::equal(data.begin(), data.end(), instance.begin()));
}

TEST(DataBuf, tryingToAccessTooFarElementThrows) {
  const std::array<byte, 4> data{'h', 'o', 'l', 'a'};
  DataBuf instance(data.data(), data.size());
  ASSERT_THROW([[maybe_unused]] auto d = instance.data(5), std::out_of_range);
  ASSERT_THROW([[maybe_unused]] auto d = instance.c_data(5), std::out_of_range);
}

TEST(DataBuf, readUintFunctionsWorksOnExistingData) {
  const std::array<byte, 8> data{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  DataBuf instance(data.data(), data.size());
  ASSERT_EQ(data[0], instance.read_uint8(0));
  ASSERT_EQ(data[1], instance.read_uint16(0, bigEndian));
  ASSERT_EQ(0x00010203, instance.read_uint32(0, bigEndian));
  ASSERT_EQ(0x0001020304050607, instance.read_uint64(0, bigEndian));
}

TEST(DataBuf, readUintFunctionsThrowsOnTooFarElements) {
  const std::array<byte, 8> data{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  DataBuf instance(data.data(), data.size());
  ASSERT_THROW([[maybe_unused]] auto d = instance.read_uint8(data.size()), std::out_of_range);
  ASSERT_THROW([[maybe_unused]] auto d = instance.read_uint16(data.size(), bigEndian), std::out_of_range);
  ASSERT_THROW([[maybe_unused]] auto d = instance.read_uint32(data.size(), bigEndian), std::out_of_range);
  ASSERT_THROW([[maybe_unused]] auto d = instance.read_uint64(data.size(), bigEndian), std::out_of_range);
}

TEST(DataBuf, writeUintFunctionsWorksWhenThereIsEnoughData) {
  DataBuf instance(8);
  std::uint64_t val{0x0102030405060708};
  ASSERT_NO_THROW(instance.write_uint8(0, (val >> 56)));
  ASSERT_EQ(0x01, instance.read_uint8(0));

  ASSERT_NO_THROW(instance.write_uint16(0, (val >> 48), bigEndian));
  ASSERT_EQ(0x0102, instance.read_uint16(0, bigEndian));

  ASSERT_NO_THROW(instance.write_uint32(0, (val >> 32), bigEndian));
  ASSERT_EQ(0x01020304, instance.read_uint32(0, bigEndian));

  ASSERT_NO_THROW(instance.write_uint64(0, val, bigEndian));
  ASSERT_EQ(val, instance.read_uint64(0, bigEndian));
}

TEST(DataBuf, writeUintFunctionsThrowsIfTryingToWriteOutOfBounds) {
  DataBuf instance(8);
  std::uint64_t val{0x0102030405060708};
  ASSERT_THROW(instance.write_uint8(8, (val >> 56)), std::out_of_range);
  ASSERT_THROW(instance.write_uint16(7, (val >> 48), bigEndian), std::out_of_range);
  ASSERT_THROW(instance.write_uint32(5, (val >> 32), bigEndian), std::out_of_range);
  ASSERT_THROW(instance.write_uint64(1, (val >> 32), bigEndian), std::out_of_range);
}

// Test methods like DataBuf::read_uint32 and DataBuf::write_uint32.
TEST(DataBuf, readWriteEndianess) {
  DataBuf buf(4 + 1 + 2 + 4 + 8);

  // Big endian.
  buf.write_uint8(4, 0x01);
  buf.write_uint16(4 + 1, 0x0203, bigEndian);
  buf.write_uint32(4 + 1 + 2, 0x04050607, bigEndian);
  buf.write_uint64(4 + 1 + 2 + 4, 0x08090a0b0c0d0e0fULL, bigEndian);
  static const uint8_t expected_le[4 + 1 + 2 + 4 + 8] = {0,   0,   0,   0,   0x1, 0x2, 0x3, 0x4, 0x5, 0x6,
                                                         0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
  ASSERT_EQ(0, buf.cmpBytes(0, expected_le, buf.size()));
  ASSERT_EQ(buf.read_uint8(4), 0x01);
  ASSERT_EQ(buf.read_uint16(4 + 1, bigEndian), 0x0203);
  ASSERT_EQ(buf.read_uint32(4 + 1 + 2, bigEndian), 0x04050607);
  ASSERT_EQ(buf.read_uint64(4 + 1 + 2 + 4, bigEndian), 0x08090a0b0c0d0e0fULL);

  // Little endian.
  buf.write_uint8(4, 0x01);
  buf.write_uint16(4 + 1, 0x0203, littleEndian);
  buf.write_uint32(4 + 1 + 2, 0x04050607, littleEndian);
  buf.write_uint64(4 + 1 + 2 + 4, 0x08090a0b0c0d0e0fULL, littleEndian);
  static const uint8_t expected_be[4 + 1 + 2 + 4 + 8] = {0,   0,   0,   0,   0x1, 0x3, 0x2, 0x7, 0x6, 0x5,
                                                         0x4, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8};
  ASSERT_EQ(0, buf.cmpBytes(0, expected_be, buf.size()));
  ASSERT_EQ(buf.read_uint8(4), 0x01);
  ASSERT_EQ(buf.read_uint16(4 + 1, littleEndian), 0x0203);
  ASSERT_EQ(buf.read_uint32(4 + 1 + 2, littleEndian), 0x04050607);
  ASSERT_EQ(buf.read_uint64(4 + 1 + 2 + 4, littleEndian), 0x08090a0b0c0d0e0fULL);
}

TEST(Rational, floatToRationalCast) {
  static const float floats[] = {0.5F, 0.015F, 0.0000625F};

  for (float i : floats) {
    const Rational r = floatToRationalCast(i);
    const float fraction = static_cast<float>(r.first) / static_cast<float>(r.second);
    ASSERT_TRUE(std::fabs((i - fraction) / i) < 0.01F);
  }

  const Rational plus_inf = floatToRationalCast(std::numeric_limits<float>::infinity());
  ASSERT_EQ(plus_inf.first, 1);
  ASSERT_EQ(plus_inf.second, 0);

  const Rational minus_inf = floatToRationalCast(-1 * std::numeric_limits<float>::infinity());
  ASSERT_EQ(minus_inf.first, -1);
  ASSERT_EQ(minus_inf.second, 0);
}

TEST(Rational, toStream) {
  Rational r = {1, 2};
  std::stringstream str;
  str << r;
  ASSERT_EQ("1/2", str.str());
}

TEST(Rational, readRationalFromStream) {
  Rational r;
  std::istringstream input("1/2");
  input >> r;
  ASSERT_EQ(1, r.first);
  ASSERT_EQ(2, r.second);
}

TEST(Rational, parseRationalFromStringSuccessfully) {
  bool ok{false};
  Rational rational = parseRational("1/2", ok);
  ASSERT_EQ(std::make_pair(1, 2), rational);
  ASSERT_TRUE(ok);
}

TEST(Rational, parseRationalFromLongIsOK) {
  bool ok{true};
  Rational rational = parseRational("12", ok);
  ASSERT_EQ(std::make_pair(12, 1), rational);
  ASSERT_TRUE(ok);
}

TEST(Rational, parseRationalFromBoolIsOK) {
  bool ok{true};
  Rational rational = parseRational("true", ok);
  ASSERT_EQ(std::make_pair(1, 1), rational);
  ASSERT_TRUE(ok);

  rational = parseRational("false", ok);
  ASSERT_EQ(std::make_pair(0, 1), rational);
  ASSERT_TRUE(ok);
}

TEST(Rational, parseRationalFromFloatIsOK) {
  bool ok{true};
  Rational rational = parseRational("1.2", ok);
  ASSERT_EQ(std::make_pair(6, 5), rational);
  ASSERT_TRUE(ok);

  rational = parseRational("1.4", ok);
  ASSERT_EQ(std::make_pair(7, 5), rational);
  ASSERT_TRUE(ok);
}

TEST(Rational, parseRationalFromFloatWithFCharIsNoOK) {
  bool ok{true};
  Rational rational = parseRational("1.2f", ok);
  ASSERT_EQ(std::make_pair(0, 0), rational);
  ASSERT_FALSE(ok);
}

TEST(Rational, floatToRationalCastWorks) {
  ASSERT_EQ(std::make_pair(6, 5), floatToRationalCast(1.2f));
  ASSERT_EQ(std::make_pair(11001, 5), floatToRationalCast(2200.2f));
  ASSERT_EQ(std::make_pair(1100001, 5), floatToRationalCast(220000.2f));
  ASSERT_EQ(std::make_pair(22000000, 1), floatToRationalCast(22000000.2f));
  ASSERT_EQ(std::make_pair(22000000, 1), floatToRationalCast(22000000.2f));
}

TEST(Rational, floatToRationalCastWithIntegersOutOfLimits) {
  ASSERT_EQ(std::make_pair(1, 0), floatToRationalCast(2247483647.f));
  ASSERT_EQ(std::make_pair(-1, 0), floatToRationalCast(-2247483647.f));
}

TEST(URational, toStream) {
  URational r = {1, 2};
  std::stringstream str;
  str << r;
  ASSERT_EQ("1/2", str.str());
}

TEST(URational, readRationalFromStream) {
  URational r;
  std::istringstream input("1/2");
  input >> r;
  ASSERT_EQ(1, r.first);
  ASSERT_EQ(2, r.second);
}

// --------------------

TEST(parseUint32, withNumberInRangeReturnsOK) {
  bool ok{false};
  ASSERT_EQ(123456, parseUint32("123456", ok));
  ASSERT_TRUE(ok);
}

TEST(parseUint32, withNumberOutOfRangeReturnsFalse) {
  bool ok{false};
  ASSERT_EQ(0, parseUint32("4333333333", ok));
  ASSERT_FALSE(ok);
}
