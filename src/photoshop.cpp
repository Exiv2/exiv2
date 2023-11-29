#include "photoshop.hpp"

#include "enforce.hpp"
#include "image.hpp"
#include "safe_op.hpp"

#include <iostream>

namespace Exiv2 {

bool Photoshop::isIrb(const byte* data) {
  if (data == nullptr) {
    return false;
  }
  return std::any_of(irbId_.begin(), irbId_.end(), [data](auto id) { return memcmp(data, id, 4) == 0; });
}

bool Photoshop::valid(const byte* pPsData, size_t sizePsData) {
  const byte* record = nullptr;
  uint32_t sizeIptc = 0;
  uint32_t sizeHdr = 0;
  const byte* pCur = pPsData;
  const byte* pEnd = pPsData + sizePsData;
  int ret = 0;
  while (pCur < pEnd && 0 == (ret = Photoshop::locateIptcIrb(pCur, (pEnd - pCur), &record, sizeHdr, sizeIptc))) {
    pCur = record + sizeHdr + sizeIptc + (sizeIptc & 1);
  }
  return ret >= 0;
}

// Todo: Generalised from JpegBase::locateIptcData without really understanding
//       the format (in particular the header). So it remains to be confirmed
//       if this also makes sense for psTag != Photoshop::iptc
int Photoshop::locateIrb(const byte* pPsData, size_t sizePsData, uint16_t psTag, const byte** record, uint32_t& sizeHdr,
                         uint32_t& sizeData) {
  if (sizePsData < 12) {
    return 3;
  }

  // Used for error checking
  size_t position = 0;
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Photoshop::locateIrb: ";
#endif
  // Data should follow Photoshop format, if not exit
  while (position <= (sizePsData - 12) && isIrb(pPsData + position)) {
    const byte* hrd = pPsData + position;
    position += 4;
    uint16_t type = getUShort(pPsData + position, bigEndian);
    position += 2;
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "0x" << std::hex << type << std::dec << " ";
#endif
    // Pascal string is padded to have an even size (including size byte)
    byte psSize = pPsData[position] + 1;
    psSize += (psSize & 1);
    position += psSize;
    if (position + 4 > sizePsData) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Warning: "
                << "Invalid or extended Photoshop IRB\n";
#endif
      return -2;
    }
    uint32_t dataSize = getULong(pPsData + position, bigEndian);
    position += 4;
    if (dataSize > (sizePsData - position)) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Warning: "
                << "Invalid Photoshop IRB data size " << dataSize << " or extended Photoshop IRB\n";
#endif
      return -2;
    }
#ifdef EXIV2_DEBUG_MESSAGES
    if ((dataSize & 1) && position + dataSize == sizePsData) {
      std::cerr << "Warning: "
                << "Photoshop IRB data is not padded to even size\n";
    }
#endif
    if (type == psTag) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "ok\n";
#endif
      sizeData = dataSize;
      sizeHdr = psSize + 10;
      *record = hrd;
      return 0;
    }
    // Data size is also padded to be even
    position += dataSize + (dataSize & 1);
  }
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "pPsData doesn't start with '8BIM'\n";
#endif
  if (position < sizePsData) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Warning: "
              << "Invalid or extended Photoshop IRB\n";
#endif
    return -2;
  }
  return 3;
}

int Photoshop::locateIptcIrb(const byte* pPsData, size_t sizePsData, const byte** record, uint32_t& sizeHdr,
                             uint32_t& sizeData) {
  return locateIrb(pPsData, sizePsData, iptc_, record, sizeHdr, sizeData);
}

int Photoshop::locatePreviewIrb(const byte* pPsData, size_t sizePsData, const byte** record, uint32_t& sizeHdr,
                                uint32_t& sizeData) {
  return locateIrb(pPsData, sizePsData, preview_, record, sizeHdr, sizeData);
}

DataBuf Photoshop::setIptcIrb(const byte* pPsData, size_t sizePsData, const IptcData& iptcData) {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "IRB block at the beginning of Photoshop::setIptcIrb\n";
  if (sizePsData == 0)
    std::cerr << "  None.\n";
  else
    hexdump(std::cerr, pPsData, sizePsData);
#endif
  const byte* record = pPsData;
  uint32_t sizeIptc = 0;
  uint32_t sizeHdr = 0;
  DataBuf rc;
  if (0 > Photoshop::locateIptcIrb(pPsData, sizePsData, &record, sizeHdr, sizeIptc)) {
    return rc;
  }

  Blob psBlob;
  const auto sizeFront = static_cast<size_t>(record - pPsData);
  // Write data before old record.
  if (sizePsData > 0 && sizeFront > 0) {
    append(psBlob, pPsData, sizeFront);
  }

  // Write new iptc record if we have it
  if (DataBuf rawIptc = IptcParser::encode(iptcData); !rawIptc.empty()) {
    std::array<byte, 12> tmpBuf;
    std::copy_n(Photoshop::irbId_.front(), 4, tmpBuf.data());
    us2Data(tmpBuf.data() + 4, iptc_, bigEndian);
    tmpBuf[6] = 0;
    tmpBuf[7] = 0;
    ul2Data(tmpBuf.data() + 8, static_cast<uint32_t>(rawIptc.size()), bigEndian);
    append(psBlob, tmpBuf.data(), 12);
    append(psBlob, rawIptc.c_data(), rawIptc.size());
    // Data is padded to be even (but not included in size)
    if (rawIptc.size() & 1)
      psBlob.push_back(0x00);
  }

  // Write existing stuff after record, skip the current and all remaining IPTC blocks
  size_t pos = sizeFront;
  auto nextSizeData = Safe::add<long>(static_cast<long>(sizePsData), -static_cast<long>(pos));
  Internal::enforce(nextSizeData >= 0, ErrorCode::kerCorruptedMetadata);
  while (0 == Photoshop::locateIptcIrb(pPsData + pos, nextSizeData, &record, sizeHdr, sizeIptc)) {
    const auto newPos = static_cast<size_t>(record - pPsData);
    if (newPos > pos) {  // Copy data up to the IPTC IRB
      append(psBlob, pPsData + pos, newPos - pos);
    }
    pos = newPos + sizeHdr + sizeIptc + (sizeIptc & 1);  // Skip the IPTC IRB
    nextSizeData = Safe::add<long>(static_cast<long>(sizePsData), -static_cast<long>(pos));
    Internal::enforce(nextSizeData >= 0, ErrorCode::kerCorruptedMetadata);
  }
  if (pos < sizePsData) {
    append(psBlob, pPsData + pos, sizePsData - pos);
  }

  // Data is rounded to be even
  if (!psBlob.empty())
    rc = DataBuf(psBlob.data(), psBlob.size());
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "IRB block at the end of Photoshop::setIptcIrb\n";
  if (rc.empty())
    std::cerr << "  None.\n";
  else
    hexdump(std::cerr, rc.c_data(), rc.size());
#endif
  return rc;
}

}  // namespace Exiv2
