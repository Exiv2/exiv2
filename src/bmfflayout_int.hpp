// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFLAYOUT_INT_HPP
#define BMFFLAYOUT_INT_HPP

#include "bmffbox_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

/*!
  @brief Borrowed source bytes or an immutable memory buffer in an output layout.

  A non-null bytes pointer selects memory; otherwise source selects the input
  interval. The input and every referenced vector must remain unchanged and
  outlive measurement, emission and verification. No encoded payload is owned.
 */
struct BmffOutputSegment {
  BmffSpan source;                      //!< Absolute interval in the original input.
  const std::vector<uint8_t>* bytes{};  //!< Borrowed replacement buffer, or null for source bytes.
  uint64_t position{};                  //!< Absolute output offset assigned by layoutBmff().

  //! @brief Return the selected memory buffer's length or the source interval length.
  [[nodiscard]] uint64_t size() const;
};

/*!
  @brief Output box owning its prefix and children, and borrowing segment payloads.

  Serialization order is header, prefix, children, then segments. Adapters supply
  all children and format-specific prefixes; unknown payloads are opaque. Only
  the standard uuid header receives special framing. Header promotion is
  monotonic, and terminal-size inputs are serialized with explicit lengths.
 */
struct BmffOutputBox {
  uint32_t type{};                          //!< Packed box code, with no admission policy.
  bool extended{};                          //!< Preserve or promote to a 64-bit size field.
  std::array<uint8_t, 16> uuid{};           //!< User type for a uuid box.
  std::vector<uint8_t> prefix;              //!< Owned bytes before any children or segments.
  std::vector<BmffOutputBox> children;      //!< Owned child nodes in output order.
  std::vector<BmffOutputSegment> segments;  //!< Borrowed payloads in output order.
  uint64_t position{};                      //!< Absolute output offset assigned by layoutBmff().
  uint64_t size{};                          //!< Measured size including the header.
  uint8_t header{};                         //!< Measured header length including any UUID.
};

//! @brief An adapter-approved source interval and its absolute output destination.
struct BmffRelocation {
  BmffSpan source;         //!< Retained interval in the original input.
  uint64_t destination{};  //!< Absolute output position of the interval's first byte.
};

/*!
  @brief An owned, sorted map of nonoverlapping retained source intervals.

  The adapter chooses which ranges can be relocated. Merely emitting an opaque
  box does not establish that its embedded offsets can be changed safely.
 */
class BmffRelocations {
 public:
  //! @brief Sort and take ownership of approved ranges; throw Error for overlap or overflow.
  explicit BmffRelocations(std::vector<BmffRelocation> ranges);

  /*!
    @brief Translate a source subrange into absolute output coordinates.
    @param source Interval wholly contained in one approved range.
    @return Output offset of the interval's first byte.
    @throws Error If no single range contains the interval or arithmetic overflows.
   */
  [[nodiscard]] uint64_t position(BmffSpan source) const;

 private:
  std::vector<BmffRelocation> ranges_;
};

/*!
  @brief Measure boxes, promote headers and assign all positions starting at output offset zero.
  @param boxes Adapter-built tree; owned prefixes and borrowed payloads must remain unchanged afterward.
  @return Total serialized length within BasicIo's signed seek range.
  @throws Error For sizes or source intervals outside the signed seek range.

  No I/O occurs. The caller bounds the number and nesting of nodes it constructs.
  No format markers, item tables or metadata are required or interpreted.
 */
uint64_t layoutBmff(std::vector<BmffOutputBox>& boxes);

/*!
  @brief Stream a measured layout to a distinct, empty staging stream with bounded buffers.
  @param input Borrowed, open, seekable source; it must remain unchanged and not alias output.
  @param output Borrowed, open, empty, seekable staging stream.
  @param boxes Unchanged tree already positioned by layoutBmff().
  @throws Error For inconsistent layout, overflow, failed I/O or short reads/writes.

  Stream positions change. No source transfer is performed. On failure the
  partially written output must be discarded. Every I/O request is at most 64 KiB.
 */
void writeBmffLayout(BasicIo& input, BasicIo& output, const std::vector<BmffOutputBox>& boxes);

/*!
  @brief Compare all emitted headers, prefixes and payloads with the measured layout.
  @param input Unchanged source borrowed during emission.
  @param output Prepared output, distinct from input and open for reading.
  @param boxes Unchanged layout with all borrowed buffers still alive.
  @throws Error For a byte/size mismatch, invalid layout, or failed I/O.

  Both stream positions may change; neither stream is written. Comparisons use
  two fixed 64 KiB buffers. Format-level semantic verification belongs to adapters.
 */
void verifyBmffLayout(BasicIo& input, BasicIo& output, const std::vector<BmffOutputBox>& boxes);

}  // namespace Exiv2::Internal
#endif
#endif
