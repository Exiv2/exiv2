// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffbox_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"

#include <limits>

namespace Exiv2::Internal {
namespace {

// Report malformed boundaries and exhausted budgets consistently.
void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

// Bound cumulative work before adding to a counter.
void consume(uint64_t& used, uint64_t amount, uint64_t limit) {
  require(used <= limit && amount <= limit - used);
  used += amount;
}

}  // namespace

BmffSpan BmffBox::payload() const {
  require(headerSize <= span.size && span.size <= std::numeric_limits<uint64_t>::max() - span.offset);
  return {span.offset + headerSize, span.size - headerSize};
}

BmffCursor::BmffCursor(BmffReader& reader, BmffSpan range) :
    reader_(reader), position_(range.offset), end_(range.offset + range.size) {
}

void BmffCursor::advance(uint64_t count) {
  require(count <= remaining());
  position_ += count;
}

void BmffCursor::read(uint8_t* bytes, size_t size) {
  require(size <= remaining());
  reader_.read(position_, bytes, size);
  position_ += size;
}

uint64_t BmffCursor::number(unsigned width) {
  require(width <= 8);
  std::array<uint8_t, 8> bytes{};
  if (width != 0)
    read(bytes.data(), width);

  uint64_t result = 0;
  for (unsigned i = 0; i < width; ++i)
    result = (result << 8) | bytes[i];
  return result;
}

void BmffCursor::finish() const {
  require(remaining() == 0);
}

BmffReader::BmffReader(BasicIo& io, BmffReadLimits limits) : io_(io), limits_(limits), fileSize_(0) {
  require(io.isopen());
  fileSize_ = io.size();
  require(fileSize_ <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
}

BmffCursor BmffReader::cursor(BmffSpan range) {
  require(range.offset <= fileSize_ && range.size <= fileSize_ - range.offset);
  return BmffCursor(*this, range);
}

void BmffReader::read(uint64_t offset, uint8_t* bytes, size_t size) {
  consume(bytes_, size, limits_.maxBytesRead);
  if (size == 0)
    return;

  io_.seekOrThrow(static_cast<int64_t>(offset), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
  io_.readOrThrow(bytes, size, ErrorCode::kerInputDataReadFailed);
  if (io_.error())
    throw Error(ErrorCode::kerInputDataReadFailed);
}

BmffBox BmffReader::readBox(BmffCursor& input) {
  require(&input.reader_ == this);
  consume(boxes_, 1, limits_.maxBoxes);
  require(input.remaining() >= 8);

  BmffBox box;
  box.span.offset = input.position();
  const auto available = input.remaining();
  box.size32 = static_cast<uint32_t>(input.number(4));
  box.span.size = box.size32;
  box.type = static_cast<uint32_t>(input.number(4));
  box.headerSize = 8;

  // Resolve extended and terminal sizes against the immediate containing range.
  if (box.size32 == 1) {
    box.span.size = input.number(8);
    box.headerSize = 16;
  } else if (box.size32 == 0) {
    require(input.end() == fileSize_);
    box.span.size = available;
    box.extendsToEnd = true;
  }

  // UUID user types are header bytes, not part of the adapter's payload cursor.
  if (box.type == bmffType("uuid")) {
    box.headerSize += 16;
    require(box.span.size >= box.headerSize && box.span.size <= available);
    input.read(box.userType.data(), box.userType.size());
  }

  // Parent progress is independent of reads through the child payload cursor.
  require(box.span.size >= box.headerSize && box.span.size <= available);
  input.advance(box.span.size - box.headerSize);
  return box;
}

void BmffReader::checkTraversal(const BmffCursor& input, unsigned depth) const {
  require(&input.reader_ == this && depth <= limits_.maxDepth);
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
