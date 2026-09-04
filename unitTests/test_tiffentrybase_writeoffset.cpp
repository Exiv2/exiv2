// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/error.hpp>
#include <gtest/gtest.h>
#include <tiffcomposite_int.hpp>

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
// TiffEntryBase::writeOffset is protected, but static, so a derived class
// with a `using` declaration is enough to reach it in a test without ever
// needing to build an actual TiffEntryBase instance.
class TestableTiffEntryBase : public TiffEntryBase {
 public:
  using TiffEntryBase::writeOffset;
};
}  // namespace

TEST(ATiffEntryBaseWriteOffset, acceptsAMaximalUint32Offset) {
  byte buf[4] = {};
  EXPECT_NO_THROW(TestableTiffEntryBase::writeOffset(buf, std::numeric_limits<uint32_t>::max(), ttUnsignedLong,
                                                      littleEndian));
}

TEST(ATiffEntryBaseWriteOffset, rejectsAnOffsetPastUint32MaxForUnsignedLong) {
  const size_t tooLarge = static_cast<size_t>(std::numeric_limits<uint32_t>::max()) + 1;
  byte buf[4] = {};
  EXPECT_THROW(TestableTiffEntryBase::writeOffset(buf, tooLarge, ttUnsignedLong, littleEndian), Error);
}

TEST(ATiffEntryBaseWriteOffset, rejectsAnOffsetPastUint32MaxForSignedLong) {
  const size_t tooLarge = static_cast<size_t>(std::numeric_limits<uint32_t>::max()) + 1;
  byte buf[4] = {};
  EXPECT_THROW(TestableTiffEntryBase::writeOffset(buf, tooLarge, ttSignedLong, littleEndian), Error);
}

TEST(ATiffEntryBaseWriteOffset, stillRejectsAnOffsetPastUint16MaxForUnsignedShort) {
  // Pre-existing behavior for the 16-bit case, kept as a control so this
  // file also catches a regression in the other direction.
  const size_t tooLarge = static_cast<size_t>(std::numeric_limits<uint16_t>::max()) + 1;
  byte buf[2] = {};
  EXPECT_THROW(TestableTiffEntryBase::writeOffset(buf, tooLarge, ttUnsignedShort, littleEndian), Error);
}
