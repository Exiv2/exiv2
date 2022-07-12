// SPDX-License-Identifier: GPL-2.0-or-later

#include "cr2header_int.hpp"

namespace Exiv2::Internal {
Cr2Header::Cr2Header(ByteOrder byteOrder) : TiffHeaderBase(42, 16, byteOrder, 0x00000010) {
}

bool Cr2Header::read(const byte* pData, size_t size) {
  if (!pData || size < 16) {
    return false;
  }

  if (pData[0] == 'I' && pData[0] == pData[1]) {
    setByteOrder(littleEndian);
  } else if (pData[0] == 'M' && pData[0] == pData[1]) {
    setByteOrder(bigEndian);
  } else {
    return false;
  }
  if (tag() != getUShort(pData + 2, byteOrder()))
    return false;
  setOffset(getULong(pData + 4, byteOrder()));
  if (0 != memcmp(pData + 8, cr2sig_, 4))
    return false;
  offset2_ = getULong(pData + 12, byteOrder());

  return true;
}

DataBuf Cr2Header::write() const {
  DataBuf buf(16);
  switch (byteOrder()) {
    case littleEndian:
      buf.write_uint8(0, 'I');
      break;
    case bigEndian:
      buf.write_uint8(0, 'M');
      break;
    default:
      break;
  }
  buf.write_uint8(1, buf.read_uint8(0));

  buf.write_uint16(2, tag(), byteOrder());
  buf.write_uint32(4, 0x00000010, byteOrder());
  std::copy_n(cr2sig_, 4, buf.begin() + 8);
  // Write a dummy value for the RAW IFD offset. The offset-writer is used to set this offset in a second pass.
  buf.write_uint32(12, 0x00000000, byteOrder());
  return buf;
}  // Cr2Header::write

bool Cr2Header::isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* /*pPrimaryGroups*/) const {
  // CR2 image tags are all IFD2 and IFD3 tags
  if (group == IfdId::ifd2Id || group == IfdId::ifd3Id)
    return true;
  // ...and any (IFD0) tag that is in the TIFF image tags list
  return isTiffImageTag(tag, group);
}

}  // namespace Exiv2::Internal
