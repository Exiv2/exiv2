#include "orfimage_int.hpp"

namespace Exiv2 {
    namespace Internal {

    OrfHeader::OrfHeader(ByteOrder byteOrder)
        : TiffHeaderBase(0x4f52, 8, byteOrder, 0x00000008),
          sig_(0x4f52)
    {
    }

    OrfHeader::~OrfHeader()
    {
    }

    bool OrfHeader::read(const byte* pData, uint32_t size)
    {
        if (size < 8) return false;

        if (pData[0] == 'I' && pData[0] == pData[1]) {
            setByteOrder(littleEndian);
        }
        else if (pData[0] == 'M' && pData[0] == pData[1]) {
            setByteOrder(bigEndian);
        }
        else {
            return false;
        }

        uint16_t sig = getUShort(pData + 2, byteOrder());
        if (tag() != sig && 0x5352 != sig) return false; // #658: Added 0x5352 "SR" for SP-560UZ
        sig_ = sig;
        setOffset(getULong(pData + 4, byteOrder()));
        return true;
    } // OrfHeader::read

    DataBuf OrfHeader::write() const
    {
        DataBuf buf(8);
        switch (byteOrder()) {
        case littleEndian:
            buf.pData_[0] = 'I';
            break;
        case bigEndian:
            buf.pData_[0] = 'M';
            break;
        case invalidByteOrder:
            assert(false);
            break;
        }
        buf.pData_[1] = buf.pData_[0];

        us2Data(buf.pData_ + 2, sig_, byteOrder());
        ul2Data(buf.pData_ + 4, 0x00000008, byteOrder());
        return buf;
    }

}}                                      // namespace Internal, Exiv2
