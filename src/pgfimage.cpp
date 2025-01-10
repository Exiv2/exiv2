// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "pgfimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"

#include <iostream>

#ifdef __cpp_lib_endian
#include <bit>
#endif

// Signature from front of PGF file
const unsigned char pgfSignature[3] = {0x50, 0x47, 0x46};

const unsigned char pgfBlank[] = {
    0x50, 0x47, 0x46, 0x36, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x03, 0x03, 0x00, 0x00, 0x00, 0x14, 0x00, 0x67, 0x08, 0x20, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x37, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x37, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

// *****************************************************************************
// class member definitions

namespace Exiv2 {
static uint32_t byteSwap_(uint32_t value, bool bSwap) {
#ifdef __cpp_lib_byteswap
  return bSwap ? std::byteswap(value) : value;
#else
  uint32_t result = 0;
  result |= (value & 0x000000FF) << 24;
  result |= (value & 0x0000FF00) << 8;
  result |= (value & 0x00FF0000) >> 8;
  result |= (value & 0xFF000000) >> 24;
  return bSwap ? result : value;
#endif
}

static uint32_t byteSwap_(Exiv2::DataBuf& buf, size_t offset, bool bSwap) {
  uint32_t v = 0;
  auto p = reinterpret_cast<byte*>(&v);
  int i;
  for (i = 0; i < 4; i++)
    p[i] = buf.read_uint8(offset + i);
  uint32_t result = byteSwap_(v, bSwap);
  p = reinterpret_cast<byte*>(&result);
  for (i = 0; i < 4; i++)
    buf.write_uint8(offset + i, p[i]);
  return result;
}

PgfImage::PgfImage(BasicIo::UniquePtr io, bool create) :
    Image(ImageType::pgf, mdExif | mdIptc | mdXmp | mdComment, std::move(io)), bSwap_(isBigEndianPlatform()) {
  if (create && io_->open() == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Exiv2::PgfImage:: Creating PGF image to memory\n";
#endif
    IoCloser closer(*io_);
    if (io_->write(pgfBlank, sizeof(pgfBlank)) != sizeof(pgfBlank)) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Exiv2::PgfImage:: Failed to create PGF image on memory\n";
#endif
    }
  }
}  // PgfImage::PgfImage

void PgfImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::PgfImage::readMetadata: Reading PGF file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isPgfType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "PGF");
  }
  clearMetadata();

  readPgfMagicNumber(*io_);

  size_t headerSize = readPgfHeaderSize(*io_);
  readPgfHeaderStructure(*io_, pixelWidth_, pixelHeight_);

  // And now, the most interesting, the user data byte array where metadata are stored as small image.

  Internal::enforce(headerSize <= std::numeric_limits<size_t>::max() - 8, ErrorCode::kerCorruptedMetadata);
  size_t size = headerSize + 8 - io_->tell();

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PgfImage::readMetadata: Found Image data (" << size << " bytes)\n";
#endif

  if (size > io_->size())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (size == 0)
    return;

  DataBuf imgData(size);
  const size_t bufRead = io_->read(imgData.data(), imgData.size());
  if (io_->error())
    throw Error(ErrorCode::kerFailedToReadImageData);
  if (bufRead != imgData.size())
    throw Error(ErrorCode::kerInputDataReadFailed);

  auto image = Exiv2::ImageFactory::open(imgData.c_data(), imgData.size());
  image->readMetadata();
  exifData() = image->exifData();
  iptcData() = image->iptcData();
  xmpData() = image->xmpData();
}

void PgfImage::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  MemIo tempIo;

  doWriteMetadata(tempIo);  // may throw
  io_->close();
  io_->transfer(tempIo);  // may throw

}  // PgfImage::writeMetadata

void PgfImage::doWriteMetadata(BasicIo& outIo) {
  if (!io_->isopen())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (!outIo.isopen())
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PgfImage::doWriteMetadata: Writing PGF file " << io_->path() << "\n";
  std::cout << "Exiv2::PgfImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

  // Ensure that this is the correct image type
  if (!isPgfType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerInputDataReadFailed);
    throw Error(ErrorCode::kerNoImageInInputData);
  }

  // Ensure PGF version.
  byte mnb = readPgfMagicNumber(*io_);

  readPgfHeaderSize(*io_);

  uint32_t w = 0;
  uint32_t h = 0;
  DataBuf header = readPgfHeaderStructure(*io_, w, h);

  auto img = ImageFactory::create(ImageType::png);

  img->setExifData(exifData_);
  img->setIptcData(iptcData_);
  img->setXmpData(xmpData_);
  img->writeMetadata();
  size_t imgSize = img->io().size();
  DataBuf imgBuf = img->io().read(imgSize);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PgfImage::doWriteMetadata: Creating image to host metadata (" << imgSize << " bytes)\n";
#endif

  //---------------------------------------------------------------

  // Write PGF Signature.
  if (outIo.write(pgfSignature, 3) != 3)
    throw Error(ErrorCode::kerImageWriteFailed);

  // Write Magic number.
  if (outIo.putb(mnb) == EOF)
    throw Error(ErrorCode::kerImageWriteFailed);

  // Write new Header size.
  auto newHeaderSize = static_cast<uint32_t>(header.size() + imgSize);
  DataBuf buffer(4);
  std::copy_n(&newHeaderSize, sizeof(uint32_t), buffer.data());
  byteSwap_(buffer, 0, bSwap_);
  if (outIo.write(buffer.c_data(), 4) != 4)
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PgfImage: new PGF header size : " << newHeaderSize << " bytes\n";

  printf("%x\n", buffer.read_uint8(0));
  printf("%x\n", buffer.read_uint8(1));
  printf("%x\n", buffer.read_uint8(2));
  printf("%x\n", buffer.read_uint8(3));
#endif

  // Write Header data.
  if (outIo.write(header.c_data(), header.size()) != header.size())
    throw Error(ErrorCode::kerImageWriteFailed);

  // Write new metadata byte array.
  if (outIo.write(imgBuf.c_data(), imgBuf.size()) != imgBuf.size())
    throw Error(ErrorCode::kerImageWriteFailed);

  // Copy the rest of PGF image data.

  DataBuf buf(4096);
  size_t readSize = io_->read(buf.data(), buf.size());
  while (readSize != 0) {
    if (outIo.write(buf.c_data(), readSize) != readSize)
      throw Error(ErrorCode::kerImageWriteFailed);
    readSize = io_->read(buf.data(), buf.size());
  }
  if (outIo.error())
    throw Error(ErrorCode::kerImageWriteFailed);

}  // PgfImage::doWriteMetadata

byte PgfImage::readPgfMagicNumber(BasicIo& iIo) {
  auto b = static_cast<byte>(iIo.getb());
  if (iIo.error())
    throw Error(ErrorCode::kerFailedToReadImageData);

  if (b < 0x36)  // 0x36 = '6'.
  {
    // Not right Magick version.
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::PgfImage::readMetadata: wrong Magick number\n";
#endif
  }

  return b;
}  // PgfImage::readPgfMagicNumber

size_t PgfImage::readPgfHeaderSize(BasicIo& iIo) const {
  DataBuf buffer(4);
  const size_t bufRead = iIo.read(buffer.data(), buffer.size());
  if (iIo.error())
    throw Error(ErrorCode::kerFailedToReadImageData);
  if (bufRead != buffer.size())
    throw Error(ErrorCode::kerInputDataReadFailed);

  auto headerSize = static_cast<size_t>(byteSwap_(buffer, 0, bSwap_));
  if (headerSize == 0)
    throw Error(ErrorCode::kerNoImageInInputData);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PgfImage: PGF header size : " << headerSize << " bytes\n";
#endif

  return headerSize;
}

DataBuf PgfImage::readPgfHeaderStructure(BasicIo& iIo, uint32_t& width, uint32_t& height) const {
  DataBuf header(16);
  size_t bufRead = iIo.read(header.data(), header.size());
  if (iIo.error())
    throw Error(ErrorCode::kerFailedToReadImageData);
  if (bufRead != header.size())
    throw Error(ErrorCode::kerInputDataReadFailed);

  DataBuf work(8);  // don't disturb the binary data - doWriteMetadata reuses it
  std::copy_n(header.c_data(), 8, work.begin());
  width = byteSwap_(work, 0, bSwap_);
  height = byteSwap_(work, 4, bSwap_);

  /* NOTE: properties not yet used
  byte nLevels  = buffer.pData_[8];
  byte quality  = buffer.pData_[9];
  byte bpp      = buffer.pData_[10];
  byte channels = buffer.pData_[11];
  */

  if (header.read_uint8(12) == 2)  // Indexed color image. We pass color table (256 * 3 bytes).
  {
    header.alloc(16 + (256 * 3));

    bufRead = iIo.read(header.data(16), 256 * 3);
    if (iIo.error())
      throw Error(ErrorCode::kerFailedToReadImageData);
    if (bufRead != 256 * 3)
      throw Error(ErrorCode::kerInputDataReadFailed);
  }

  return header;
}  // PgfImage::readPgfHeaderStructure

// *************************************************************************
// free functions
Image::UniquePtr newPgfInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<PgfImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isPgfType(BasicIo& iIo, bool advance) {
  const int32_t len = 3;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  int rc = memcmp(buf, pgfSignature, 3);
  if (!advance || rc != 0) {
    iIo.seek(-len, BasicIo::cur);
  }

  return rc == 0;
}
}  // namespace Exiv2
