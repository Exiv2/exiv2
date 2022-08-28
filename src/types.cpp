// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "types.hpp"

#include "enforce.hpp"
#include "futils.hpp"
#include "i18n.h"  // for _exvGettext
#include "safe_op.hpp"

// + standard includes
#include <array>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <utility>

// *****************************************************************************
namespace {
//! Information pertaining to the defined %Exiv2 value type identifiers.
struct TypeInfoTable {
  Exiv2::TypeId typeId_;  //!< Type id
  const char* name_;      //!< Name of the type
  size_t size_;           //!< Bytes per data entry
  //! Comparison operator for \em typeId
  bool operator==(Exiv2::TypeId typeId) const {
    return typeId_ == typeId;
  }
  //! Comparison operator for \em name
  bool operator==(const std::string& name) const {
    return name == name_;
  }
};  // struct TypeInfoTable

//! Lookup list with information of Exiv2 types
constexpr auto typeInfoTable = std::array{
    TypeInfoTable{Exiv2::invalidTypeId, "Invalid", 0},
    TypeInfoTable{Exiv2::unsignedByte, "Byte", 1},
    TypeInfoTable{Exiv2::asciiString, "Ascii", 1},
    TypeInfoTable{Exiv2::unsignedShort, "Short", 2},
    TypeInfoTable{Exiv2::unsignedLong, "Long", 4},
    TypeInfoTable{Exiv2::unsignedRational, "Rational", 8},
    TypeInfoTable{Exiv2::signedByte, "SByte", 1},
    TypeInfoTable{Exiv2::undefined, "Undefined", 1},
    TypeInfoTable{Exiv2::signedShort, "SShort", 2},
    TypeInfoTable{Exiv2::signedLong, "SLong", 4},
    TypeInfoTable{Exiv2::signedRational, "SRational", 8},
    TypeInfoTable{Exiv2::tiffFloat, "Float", 4},
    TypeInfoTable{Exiv2::tiffDouble, "Double", 8},
    TypeInfoTable{Exiv2::tiffIfd, "Ifd", 4},
    TypeInfoTable{Exiv2::string, "String", 1},
    TypeInfoTable{Exiv2::date, "Date", 8},
    TypeInfoTable{Exiv2::time, "Time", 11},
    TypeInfoTable{Exiv2::comment, "Comment", 1},
    TypeInfoTable{Exiv2::directory, "Directory", 1},
    TypeInfoTable{Exiv2::xmpText, "XmpText", 1},
    TypeInfoTable{Exiv2::xmpAlt, "XmpAlt", 1},
    TypeInfoTable{Exiv2::xmpBag, "XmpBag", 1},
    TypeInfoTable{Exiv2::xmpSeq, "XmpSeq", 1},
    TypeInfoTable{Exiv2::langAlt, "LangAlt", 1},
};

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
const char* TypeInfo::typeName(TypeId typeId) {
  auto tit = std::find(typeInfoTable.begin(), typeInfoTable.end(), typeId);
  if (tit == typeInfoTable.end())
    return nullptr;
  return tit->name_;
}

TypeId TypeInfo::typeId(const std::string& typeName) {
  auto tit = std::find(typeInfoTable.begin(), typeInfoTable.end(), typeName);
  if (tit == typeInfoTable.end())
    return invalidTypeId;
  return tit->typeId_;
}

size_t TypeInfo::typeSize(TypeId typeId) {
  auto tit = std::find(typeInfoTable.begin(), typeInfoTable.end(), typeId);
  if (tit == typeInfoTable.end())
    return 0;
  return tit->size_;
}

DataBuf::DataBuf(size_t size) : pData_(size) {
}

DataBuf::DataBuf(const byte* pData, size_t size) : pData_(size) {
  std::copy_n(pData, size, pData_.begin());
}

void DataBuf::alloc(size_t size) {
  pData_.resize(size);
}

void DataBuf::resize(size_t size) {
  pData_.resize(size);
}

void DataBuf::reset() {
  pData_.clear();
}

uint8_t Exiv2::DataBuf::read_uint8(size_t offset) const {
  if (offset >= pData_.size()) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::read_uint8");
  }
  return pData_[offset];
}

void Exiv2::DataBuf::write_uint8(size_t offset, uint8_t x) {
  if (offset >= pData_.size()) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::write_uint8");
  }
  pData_[offset] = x;
}

uint16_t Exiv2::DataBuf::read_uint16(size_t offset, ByteOrder byteOrder) const {
  if (pData_.size() < 2 || offset > (pData_.size() - 2)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::read_uint16");
  }
  return getUShort(&pData_[offset], byteOrder);
}

void Exiv2::DataBuf::write_uint16(size_t offset, uint16_t x, ByteOrder byteOrder) {
  if (pData_.size() < 2 || offset > (pData_.size() - 2)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::write_uint16");
  }
  us2Data(&pData_[offset], x, byteOrder);
}

uint32_t Exiv2::DataBuf::read_uint32(size_t offset, ByteOrder byteOrder) const {
  if (pData_.size() < 4 || offset > (pData_.size() - 4)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::read_uint32");
  }
  return getULong(&pData_[offset], byteOrder);
}

void Exiv2::DataBuf::write_uint32(size_t offset, uint32_t x, ByteOrder byteOrder) {
  if (pData_.size() < 4 || offset > (pData_.size() - 4)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::write_uint32");
  }
  ul2Data(&pData_[offset], x, byteOrder);
}

uint64_t Exiv2::DataBuf::read_uint64(size_t offset, ByteOrder byteOrder) const {
  if (pData_.size() < 8 || offset > (pData_.size() - 8)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::read_uint64");
  }
  return getULongLong(&pData_[offset], byteOrder);
}

void Exiv2::DataBuf::write_uint64(size_t offset, uint64_t x, ByteOrder byteOrder) {
  if (pData_.size() < 8 || offset > (pData_.size() - 8)) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::write_uint64");
  }
  ull2Data(&pData_[offset], x, byteOrder);
}

int Exiv2::DataBuf::cmpBytes(size_t offset, const void* buf, size_t bufsize) const {
  if (pData_.size() < bufsize || offset > pData_.size() - bufsize) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::cmpBytes");
  }
  return memcmp(&pData_[offset], buf, bufsize);
}

byte* Exiv2::DataBuf::data(size_t offset) {
  return const_cast<byte*>(c_data(offset));
}

const byte* Exiv2::DataBuf::c_data(size_t offset) const {
  if (pData_.empty()) {
    return nullptr;
  }
  if (offset >= pData_.size()) {
    throw std::out_of_range("Overflow in Exiv2::DataBuf::c_data");
  }
  return &pData_[offset];
}

const char* Exiv2::DataBuf::c_str(size_t offset) const {
  return reinterpret_cast<const char*>(c_data(offset));
}

// *************************************************************************
// free functions

static void checkDataBufBounds(const DataBuf& buf, size_t end) {
  enforce<std::invalid_argument>(end <= static_cast<size_t>(std::numeric_limits<long>::max()),
                                 "end of slice too large to be compared with DataBuf bounds.");
  enforce<std::out_of_range>(end <= buf.size(), "Invalid slice bounds specified");
}

Slice<byte*> makeSlice(DataBuf& buf, size_t begin, size_t end) {
  checkDataBufBounds(buf, end);
  return {buf.data(), begin, end};
}

Slice<const byte*> makeSlice(const DataBuf& buf, size_t begin, size_t end) {
  checkDataBufBounds(buf, end);
  return {buf.c_data(), begin, end};
}

std::ostream& operator<<(std::ostream& os, const Rational& r) {
  return os << r.first << "/" << r.second;
}

template <typename T>
std::istream& fromStreamToRational(std::istream& is, T& r) {
  // http://dev.exiv2.org/boards/3/topics/1912?r=1915
  if (std::tolower(is.peek()) == 'f') {
    char F = 0;
    float f = 0.F;
    is >> F >> f;
    f = 2.0F * std::log(f) / std::log(2.0F);
    r = Exiv2::floatToRationalCast(f);
  } else {
    int32_t nominator = 0;
    int32_t denominator = 0;
    char c('\0');
    is >> nominator >> c >> denominator;
    if (c != '/')
      is.setstate(std::ios::failbit);
    if (is)
      r = {nominator, denominator};
  }
  return is;
}

std::istream& operator>>(std::istream& is, Rational& r) {
  return fromStreamToRational(is, r);
}

std::ostream& operator<<(std::ostream& os, const URational& r) {
  return os << r.first << "/" << r.second;
}

std::istream& operator>>(std::istream& is, URational& r) {
  return fromStreamToRational(is, r);
}

uint16_t getUShort(const byte* buf, ByteOrder byteOrder) {
  return getUShort(makeSliceUntil(buf, 2), byteOrder);
}

uint32_t getULong(const byte* buf, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
  }
  return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}

uint64_t getULongLong(const byte* buf, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    return static_cast<uint64_t>(buf[7]) << 56 | static_cast<uint64_t>(buf[6]) << 48 |
           static_cast<uint64_t>(buf[5]) << 40 | static_cast<uint64_t>(buf[4]) << 32 |
           static_cast<uint64_t>(buf[3]) << 24 | static_cast<uint64_t>(buf[2]) << 16 |
           static_cast<uint64_t>(buf[1]) << 8 | static_cast<uint64_t>(buf[0]);
  }
  return static_cast<uint64_t>(buf[0]) << 56 | static_cast<uint64_t>(buf[1]) << 48 |
         static_cast<uint64_t>(buf[2]) << 40 | static_cast<uint64_t>(buf[3]) << 32 |
         static_cast<uint64_t>(buf[4]) << 24 | static_cast<uint64_t>(buf[5]) << 16 |
         static_cast<uint64_t>(buf[6]) << 8 | static_cast<uint64_t>(buf[7]);
}

URational getURational(const byte* buf, ByteOrder byteOrder) {
  uint32_t nominator = getULong(buf, byteOrder);
  uint32_t denominator = getULong(buf + 4, byteOrder);
  return {nominator, denominator};
}

int16_t getShort(const byte* buf, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    return buf[1] << 8 | buf[0];
  }
  return buf[0] << 8 | buf[1];
}

int32_t getLong(const byte* buf, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
  }
  return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}

Rational getRational(const byte* buf, ByteOrder byteOrder) {
  int32_t nominator = getLong(buf, byteOrder);
  int32_t denominator = getLong(buf + 4, byteOrder);
  return {nominator, denominator};
}

float getFloat(const byte* buf, ByteOrder byteOrder) {
  // This algorithm assumes that the internal representation of the float
  // type is the 4-byte IEEE 754 binary32 format, which is common but not
  // required by the C++ standard.
  union {
    uint32_t ul_;
    float f_;
  } u;
  u.ul_ = getULong(buf, byteOrder);
  return u.f_;
}

double getDouble(const byte* buf, ByteOrder byteOrder) {
  // This algorithm assumes that the internal representation of the double
  // type is the 8-byte IEEE 754 binary64 format, which is common but not
  // required by the C++ standard.
  union {
    uint64_t ull_;
    double d_;
  } u;
  u.ull_ = 0;
  if (byteOrder == littleEndian) {
    u.ull_ = static_cast<uint64_t>(buf[7]) << 56 | static_cast<uint64_t>(buf[6]) << 48 |
             static_cast<uint64_t>(buf[5]) << 40 | static_cast<uint64_t>(buf[4]) << 32 |
             static_cast<uint64_t>(buf[3]) << 24 | static_cast<uint64_t>(buf[2]) << 16 |
             static_cast<uint64_t>(buf[1]) << 8 | static_cast<uint64_t>(buf[0]);
  } else {
    u.ull_ = static_cast<uint64_t>(buf[0]) << 56 | static_cast<uint64_t>(buf[1]) << 48 |
             static_cast<uint64_t>(buf[2]) << 40 | static_cast<uint64_t>(buf[3]) << 32 |
             static_cast<uint64_t>(buf[4]) << 24 | static_cast<uint64_t>(buf[5]) << 16 |
             static_cast<uint64_t>(buf[6]) << 8 | static_cast<uint64_t>(buf[7]);
  }
  return u.d_;
}

size_t us2Data(byte* buf, uint16_t s, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    buf[0] = static_cast<byte>(s & 0x00ffU);
    buf[1] = static_cast<byte>((s & 0xff00U) >> 8);
  } else {
    buf[0] = static_cast<byte>((s & 0xff00U) >> 8);
    buf[1] = static_cast<byte>(s & 0x00ffU);
  }
  return 2;
}

size_t ul2Data(byte* buf, uint32_t l, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    buf[0] = static_cast<byte>(l & 0x000000ffU);
    buf[1] = static_cast<byte>((l & 0x0000ff00U) >> 8);
    buf[2] = static_cast<byte>((l & 0x00ff0000U) >> 16);
    buf[3] = static_cast<byte>((l & 0xff000000U) >> 24);
  } else {
    buf[0] = static_cast<byte>((l & 0xff000000U) >> 24);
    buf[1] = static_cast<byte>((l & 0x00ff0000U) >> 16);
    buf[2] = static_cast<byte>((l & 0x0000ff00U) >> 8);
    buf[3] = static_cast<byte>(l & 0x000000ffU);
  }
  return 4;
}

size_t ull2Data(byte* buf, uint64_t l, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    for (size_t i = 0; i < 8; i++) {
      buf[i] = static_cast<byte>(l & 0xff);
      l >>= 8;
    }
  } else {
    for (size_t i = 0; i < 8; i++) {
      buf[8 - i - 1] = static_cast<byte>(l & 0xff);
      l >>= 8;
    }
  }
  return 8;
}

size_t ur2Data(byte* buf, URational l, ByteOrder byteOrder) {
  size_t o = ul2Data(buf, l.first, byteOrder);
  o += ul2Data(buf + o, l.second, byteOrder);
  return o;
}

size_t s2Data(byte* buf, int16_t s, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    buf[0] = static_cast<byte>(s & 0x00ffU);
    buf[1] = static_cast<byte>((s & 0xff00U) >> 8);
  } else {
    buf[0] = static_cast<byte>((s & 0xff00U) >> 8);
    buf[1] = static_cast<byte>(s & 0x00ffU);
  }
  return 2;
}

size_t l2Data(byte* buf, int32_t l, ByteOrder byteOrder) {
  if (byteOrder == littleEndian) {
    buf[0] = static_cast<byte>(l & 0x000000ffU);
    buf[1] = static_cast<byte>((l & 0x0000ff00U) >> 8);
    buf[2] = static_cast<byte>((l & 0x00ff0000U) >> 16);
    buf[3] = static_cast<byte>((l & 0xff000000U) >> 24);
  } else {
    buf[0] = static_cast<byte>((l & 0xff000000U) >> 24);
    buf[1] = static_cast<byte>((l & 0x00ff0000U) >> 16);
    buf[2] = static_cast<byte>((l & 0x0000ff00U) >> 8);
    buf[3] = static_cast<byte>(l & 0x000000ffU);
  }
  return 4;
}

size_t r2Data(byte* buf, Rational l, ByteOrder byteOrder) {
  size_t o = l2Data(buf, l.first, byteOrder);
  o += l2Data(buf + o, l.second, byteOrder);
  return o;
}

size_t f2Data(byte* buf, float f, ByteOrder byteOrder) {
  // This algorithm assumes that the internal representation of the float
  // type is the 4-byte IEEE 754 binary32 format, which is common but not
  // required by the C++ standard.
  union {
    uint32_t ul_;
    float f_;
  } u;
  u.f_ = f;
  return ul2Data(buf, u.ul_, byteOrder);
}

size_t d2Data(byte* buf, double d, ByteOrder byteOrder) {
  // This algorithm assumes that the internal representation of the double
  // type is the 8-byte IEEE 754 binary64 format, which is common but not
  // required by the C++ standard.
  union {
    uint64_t ull_;
    double d_;
  } u;
  u.d_ = d;
  uint64_t m = 0xff;
  if (byteOrder == littleEndian) {
    buf[0] = static_cast<byte>(u.ull_ & m);
    buf[1] = static_cast<byte>((u.ull_ & (m << 8)) >> 8);
    buf[2] = static_cast<byte>((u.ull_ & (m << 16)) >> 16);
    buf[3] = static_cast<byte>((u.ull_ & (m << 24)) >> 24);
    buf[4] = static_cast<byte>((u.ull_ & (m << 32)) >> 32);
    buf[5] = static_cast<byte>((u.ull_ & (m << 40)) >> 40);
    buf[6] = static_cast<byte>((u.ull_ & (m << 48)) >> 48);
    buf[7] = static_cast<byte>((u.ull_ & (m << 56)) >> 56);
  } else {
    buf[0] = static_cast<byte>((u.ull_ & (m << 56)) >> 56);
    buf[1] = static_cast<byte>((u.ull_ & (m << 48)) >> 48);
    buf[2] = static_cast<byte>((u.ull_ & (m << 40)) >> 40);
    buf[3] = static_cast<byte>((u.ull_ & (m << 32)) >> 32);
    buf[4] = static_cast<byte>((u.ull_ & (m << 24)) >> 24);
    buf[5] = static_cast<byte>((u.ull_ & (m << 16)) >> 16);
    buf[6] = static_cast<byte>((u.ull_ & (m << 8)) >> 8);
    buf[7] = static_cast<byte>(u.ull_ & m);
  }
  return 8;
}

void hexdump(std::ostream& os, const byte* buf, size_t len, size_t offset) {
  const std::string::size_type pos = 8 + 16 * 3 + 2;
  const std::string align(pos, ' ');
  std::ios::fmtflags f(os.flags());

  size_t i = 0;
  while (i < len) {
    os << "  " << std::setw(4) << std::setfill('0') << std::hex << i + offset << "  ";
    std::ostringstream ss;
    do {
      byte c = buf[i];
      os << std::setw(2) << std::setfill('0') << std::right << std::hex << static_cast<int>(c) << " ";
      ss << (static_cast<int>(c) >= 31 && static_cast<int>(c) < 127 ? static_cast<char>(buf[i]) : '.');
    } while (++i < len && i % 16 != 0);
    std::string::size_type width = 9 + ((i - 1) % 16 + 1) * 3;
    os << (width > pos ? "" : align.substr(width)) << ss.str() << "\n";
  }
  os << std::dec << std::setfill(' ');
  os.flags(f);
}

bool isHex(const std::string& str, size_t size, const std::string& prefix) {
  if (str.size() <= prefix.size() || str.substr(0, prefix.size()) != prefix)
    return false;
  if (size > 0 && str.size() != size + prefix.size())
    return false;

  for (size_t i = prefix.size(); i < str.size(); ++i) {
    if (!isxdigit(str[i]))
      return false;
  }
  return true;
}  // isHex

int exifTime(const char* buf, struct tm* tm) {
  int rc = 1;
  int year = 0, mon = 0, mday = 0, hour = 0, min = 0, sec = 0;
  int scanned = std::sscanf(buf, "%4d:%2d:%2d %2d:%2d:%2d", &year, &mon, &mday, &hour, &min, &sec);
  if (scanned == 6) {
    tm->tm_year = year - 1900;
    tm->tm_mon = mon - 1;
    tm->tm_mday = mday;
    tm->tm_hour = hour;
    tm->tm_min = min;
    tm->tm_sec = sec;
    rc = 0;
  }
  return rc;
}  // exifTime

const char* exvGettext(const char* str) {
#ifdef EXV_ENABLE_NLS
  return _exvGettext(str);
#else
  return str;
#endif
}

template <>
bool stringTo<bool>(const std::string& s, bool& ok) {
  std::string lcs(s); /* lowercase string */
  for (size_t i = 0; i < lcs.length(); i++) {
    lcs[i] = std::tolower(s[i]);
  }
  /* handle the same values as xmp sdk */
  if (lcs == "false" || lcs == "f" || lcs == "0") {
    ok = true;
    return false;
  }
  if (lcs == "true" || lcs == "t" || lcs == "1") {
    ok = true;
    return true;
  }
  ok = false;
  return false;
}

int64_t parseInt64(const std::string& s, bool& ok) {
  auto ret = stringTo<int64_t>(s, ok);
  if (ok)
    return ret;

  auto f = stringTo<float>(s, ok);
  if (ok)
    return static_cast<int64_t>(f);

  auto [r, st] = stringTo<Rational>(s, ok);
  if (ok) {
    if (st <= 0) {
      ok = false;
      return 0;
    }
    return static_cast<int64_t>(static_cast<float>(r) / st);
  }

  bool b = stringTo<bool>(s, ok);
  if (ok)
    return b ? 1 : 0;

  // everything failed, return from stringTo<int64_t> is probably the best fit
  return ret;
}

uint32_t parseUint32(const std::string& s, bool& ok) {
  const int64_t x = parseInt64(s, ok);
  if (ok && 0 <= x && x <= std::numeric_limits<uint32_t>::max()) {
    return static_cast<uint32_t>(x);
  }
  ok = false;
  return 0;
}

float parseFloat(const std::string& s, bool& ok) {
  auto ret = stringTo<float>(s, ok);
  if (ok)
    return ret;

  auto [r, st] = stringTo<Rational>(s, ok);
  if (ok) {
    if (st == 0) {
      ok = false;
      return 0.0;
    }
    return static_cast<float>(r) / st;
  }

  bool b = stringTo<bool>(s, ok);
  if (ok)
    return b ? 1.0F : 0.0F;

  // everything failed, return from stringTo<float> is probably the best fit
  return ret;
}

Rational parseRational(const std::string& s, bool& ok) {
  auto ret = stringTo<Rational>(s, ok);
  if (ok)
    return ret;

  auto l = stringTo<long>(s, ok);
  if (ok)
    return {l, 1};

  auto f = stringTo<float>(s, ok);
  if (ok)
    return floatToRationalCast(f);

  bool b = stringTo<bool>(s, ok);
  if (ok)
    return {b ? 1 : 0, 1};

  // everything failed, return from stringTo<Rational> is probably the best fit
  return ret;
}

Rational floatToRationalCast(float f) {
  // Convert f to double because it simplifies the "in_range" check
  // below. (INT_MAX can be represented accurately as a double, but
  // gets rounded when it's converted to float.)
  const double d = f;
  // Don't allow INT_MIN (0x80000000) because it can cause a UBSAN failure in std::gcd().
  const bool in_range = std::numeric_limits<int32_t>::min() < d && d <= std::numeric_limits<int32_t>::max();
  if (!in_range) {
    return {d > 0 ? 1 : -1, 0};
  }
  // Beware: primitive conversion algorithm
  int32_t den = 1000000;
  const auto d_as_int32_t = static_cast<int32_t>(d);
  if (Safe::abs(d_as_int32_t) > 21474836) {
    den = 1;
  } else if (Safe::abs(d_as_int32_t) > 214748) {
    den = 100;
  } else if (Safe::abs(d_as_int32_t) > 2147) {
    den = 10000;
  }
  const auto nom = static_cast<int32_t>(std::round(d * den));
  const int32_t g = std::gcd(nom, den);

  return {nom / g, den / g};
}

}  // namespace Exiv2

#ifdef EXV_ENABLE_NLS
// Declaration is in i18n.h
const char* _exvGettext(const char* str) {
  static bool exvGettextInitialized = false;

  if (!exvGettextInitialized) {
    // bindtextdomain(EXV_PACKAGE_NAME, EXV_LOCALEDIR);
    const std::string localeDir =
        EXV_LOCALEDIR[0] == '/' ? EXV_LOCALEDIR : (Exiv2::getProcessPath() + EXV_SEPARATOR_STR + EXV_LOCALEDIR);
    bindtextdomain(EXV_PACKAGE_NAME, localeDir.c_str());
#ifdef EXV_HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset(EXV_PACKAGE_NAME, "UTF-8");
#endif
    exvGettextInitialized = true;
  }

  return dgettext(EXV_PACKAGE_NAME, str);
}
#endif  // EXV_ENABLE_NLS
