#include "cr2header_int.hpp"

namespace Exiv2 {
    namespace Internal {
    Cr2Header::Cr2Header(ByteOrder byteOrder)
        : TiffHeaderBase(42, 16, byteOrder, 0x00000010),
          offset2_(0x00000000)
    {
    }

    bool Cr2Header::read(const byte* pData, uint32_t size)
    {
        if (!pData || size < 16) {
            return false;
        }

        if (pData[0] == 'I' && pData[0] == pData[1]) {
            setByteOrder(littleEndian);
        }
        else if (pData[0] == 'M' && pData[0] == pData[1]) {
            setByteOrder(bigEndian);
        }
        else {
            return false;
        }
        if (tag() != getUShort(pData + 2, byteOrder())) return false;
        setOffset(getULong(pData + 4, byteOrder()));
        if (0 != memcmp(pData + 8, cr2sig_, 4)) return false;
        offset2_ = getULong(pData + 12, byteOrder());

        return true;
    } // Cr2Header::read

    DataBuf Cr2Header::write() const
    {
        DataBuf buf(16);
        switch (byteOrder()) {
        case littleEndian:
            buf.write_uint8(0, 'I');
            break;
        case bigEndian:
            buf.write_uint8(0, 'M');
            break;
        case invalidByteOrder:
            assert(false);
            break;
        }
        buf.write_uint8(1, buf.read_uint8(0));

        buf.write_uint16(2, tag(), byteOrder());
        buf.write_uint32(4, 0x00000010, byteOrder());
        buf.copyBytes(8, cr2sig_, 4);
        // Write a dummy value for the RAW IFD offset. The offset-writer is used to set this offset in a second pass.
        buf.write_uint32(12, 0x00000000, byteOrder());
        return buf;
    } // Cr2Header::write

    bool Cr2Header::isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* /*pPrimaryGroups*/) const
    {
        // CR2 image tags are all IFD2 and IFD3 tags
        if (group == ifd2Id || group == ifd3Id) return true;
        // ...and any (IFD0) tag that is in the TIFF image tags list
        return isTiffImageTag(tag, group);
    }

    }  // namespace Internal
}  // namespace Exiv2
