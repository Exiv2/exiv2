// SPDX-License-Identifier: GPL-2.0-or-later

#include "orfimage_int.hpp"

namespace Exiv2::Internal {
OrfHeader::OrfHeader(ByteOrder byteOrder) : TiffHeaderBase(0x4f52, 8, byteOrder, 0x00000008) {
}

bool OrfHeader::read(const byte* pData, size_t size) {
  if (size < 8)
    return false;

  if (pData[0] == 'I' && pData[0] == pData[1]) {
    setByteOrder(littleEndian);
  } else if (pData[0] == 'M' && pData[0] == pData[1]) {
    setByteOrder(bigEndian);
  } else {
    return false;
  }

  uint16_t sig = getUShort(pData + 2, byteOrder());
  if (tag() != sig && 0x5352 != sig)
    return false;  // #658: Added 0x5352 "SR" for SP-560UZ
  sig_ = sig;
  setOffset(getULong(pData + 4, byteOrder()));
  return true;
}

DataBuf OrfHeader::write() const {
  DataBuf buf(8);
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

  buf.write_uint16(2, sig_, byteOrder());
  buf.write_uint32(4, 0x00000008, byteOrder());
  return buf;
}

}  // namespace Exiv2::Internal
