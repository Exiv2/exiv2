// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFBOX_INT_HPP
#define BMFFBOX_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace Exiv2 {
class BasicIo;
}

namespace Exiv2::Internal {

//! @brief Pack a four-character box/item code into its big-endian integer representation.
constexpr uint32_t bmffType(const char (&name)[5]) {
  return (uint32_t{static_cast<unsigned char>(name[0])} << 24) | (uint32_t{static_cast<unsigned char>(name[1])} << 16) |
         (uint32_t{static_cast<unsigned char>(name[2])} << 8) | uint32_t{static_cast<unsigned char>(name[3])};
}

//! @brief A validated interval in the original input. Payloads are never owned by the model.
struct BmffSpan {
  uint64_t offset{};  //!< Absolute byte offset from the start of the input.
  uint64_t size{};    //!< Length of the interval in bytes.

  //! @brief Compare the absolute offset and length of two input intervals.
  bool operator==(const BmffSpan&) const = default;
};

//! @brief Version and flags decoded from the common FullBox prefix.
struct BmffFullBox {
  uint8_t version{};  //!< FullBox version selecting its field layout.
  uint32_t flags{};   //!< Low 24 flag bits of the FullBox prefix.
};

//! @brief Input box boundaries and decoded children, with opaque payloads left in the input.
struct BmffBox {
  uint32_t type{};                     //!< Packed four-character box code.
  uint32_t size32{};                   //!< Original size field: 0 for EOF, 1 for an extended size.
  BmffSpan span;                       //!< Complete original box interval, including its header.
  uint8_t headerSize{};                //!< Includes a UUID user type, but excludes FullBox version/flags.
  bool extendsToEnd{};                 //!< True when the original size field extends to end of file.
  std::array<uint8_t, 16> userType{};  //!< UUID user type, populated only for uuid boxes.
  std::optional<BmffFullBox> fullBox;  //!< Decoded version and flags for recognized FullBox layouts.
  std::vector<BmffBox> children;       //!< Decoded immediate children in original file order.

  //! @brief Return the absolute payload interval, excluding the box header and UUID user type.
  [[nodiscard]] BmffSpan payload() const;
};

//! @brief File framing only; adapters decide which boxes contain children and what they mean.
struct BmffFile {
  uint64_t size{};             //!< Source file length in bytes.
  std::vector<BmffBox> boxes;  //!< Top-level boxes in file order, retaining source ranges.
};

//! @brief Caller-selected limits for one reader; no image-format policy is implied.
struct BmffReadLimits {
  uint64_t maxBoxes;      //!< Maximum aggregate headers decoded, including repeated visits.
  uint64_t maxBytesRead;  //!< Maximum aggregate field bytes read, excluding skipped payloads.
  unsigned maxDepth;      //!< Maximum child nesting below the root sibling range at depth zero.
};

class BmffReader;

/*!
  @brief A bounded field cursor borrowing its reader and unchanged input.

  Obtain cursors from BmffReader::cursor(). The reader and its input must outlive
  all cursors. Independent cursors share the reader's budgets and may reposition
  the underlying stream on every read. No method writes to the input.
 */
class BmffCursor {
 public:
  //! @brief Return the absolute input offset of the next field.
  [[nodiscard]] uint64_t position() const {
    return position_;
  }

  //! @brief Return the bytes still available within this interval.
  [[nodiscard]] uint64_t remaining() const {
    return end_ - position_;
  }

  //! @brief Return the exclusive absolute end of the containing interval.
  [[nodiscard]] uint64_t end() const {
    return end_;
  }

  //! @brief Skip bytes within this interval, throwing Error if count exceeds its remainder.
  void advance(uint64_t count);

  /*!
    @brief Read an exact field and charge its size to the shared structural budget.
    @param bytes Writable storage for at least size bytes; may be null only for size zero.
    @param size Number of bytes to read within the cursor's remaining interval.
    @throws Error For an invalid range, exhausted budget, or failed seek/read.

    The underlying stream is repositioned. The cursor advances only after success.
   */
  void read(uint8_t* bytes, size_t size);

  //! @brief Decode up to eight big-endian bytes; width zero returns zero without I/O.
  uint64_t number(unsigned width);

  //! @brief Reject trailing fields with Error unless the interval is exhausted.
  void finish() const;

 private:
  friend class BmffReader;

  //! @brief Borrow a reader and an interval whose bounds it has already validated.
  BmffCursor(BmffReader& reader, BmffSpan range);

  BmffReader& reader_;
  uint64_t position_;
  uint64_t end_;
};

/*!
  @brief Decode and traverse bounded BMFF box ranges without interpreting their payloads.

  The reader borrows an open, seekable BasicIo that must remain unchanged and
  outlive the reader and its cursors. Limits apply across all cursors and visits.
  It neither requires image-format boxes nor guesses which payloads are containers.
  Malformed ranges and exhausted limits throw kerCorruptedMetadata; failed I/O
  throws kerInputDataReadFailed. The underlying stream position may change.
 */
class BmffReader {
 public:
  /*!
    @brief Borrow an open input and snapshot its length and caller-selected limits.
    @param io Open, seekable input; ownership is retained by the caller.
    @param limits Bounds on aggregate structural work and nesting.
    @throws Error If the input is closed or its size exceeds signed seek offsets.
   */
  BmffReader(BasicIo& io, BmffReadLimits limits);

  //! @brief Return the unchanged input length captured at construction.
  [[nodiscard]] uint64_t fileSize() const {
    return fileSize_;
  }

  //! @brief Create a field cursor, rejecting intervals extending beyond the input.
  [[nodiscard]] BmffCursor cursor(BmffSpan range);

  /*!
    @brief Validate one header and advance input past its entire box.
    @param input A cursor belonging to this reader, positioned at a box boundary.
    @return Box boundaries and original size/UUID fields, with no decoded children.
    @throws Error For incomplete headers, invalid sizes, exceeded limits, or failed I/O.

    Size-zero boxes extend to EOF; their containing interval must end at EOF.
    Skipping the payload does not read or charge its bytes. Failure may consume
    header bytes; callers must not resume parsing after an error.
   */
  BmffBox readBox(BmffCursor& input);

  /*!
    @brief Visit sibling boxes in order, leaving container recognition to the adapter.
    @param input A cursor belonging to this reader, bounding the sibling range.
    @param depth Nesting depth; root is zero and recursive child visits add one.
    @param visitor Callback taking a mutable BmffBox and bounded payload cursor.
    @throws Error For invalid structure, depth, exhausted budgets, or failed I/O.

    The callback may read a prefix and recursively visit the remaining payload
    with depth + 1, populate children, or leave the payload opaque. It must not
    retain references to the callback arguments. Unconsumed payload bytes are
    skipped; input reaches its end on success. Callback exceptions propagate.
   */
  template <typename Visitor>
  void visit(BmffCursor& input, unsigned depth, Visitor&& visitor) {
    checkTraversal(input, depth);
    while (input.remaining() != 0) {
      auto box = readBox(input);
      auto fields = cursor(box.payload());
      visitor(box, fields);
    }
  }

 private:
  friend class BmffCursor;

  //! @brief Read an already bounded field and update the aggregate byte budget.
  void read(uint64_t offset, uint8_t* bytes, size_t size);

  //! @brief Validate cursor ownership and nesting before a sibling traversal.
  void checkTraversal(const BmffCursor& input, unsigned depth) const;

  BasicIo& io_;
  BmffReadLimits limits_;
  uint64_t fileSize_;
  uint64_t boxes_{};
  uint64_t bytes_{};
};

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // BMFFBOX_INT_HPP
