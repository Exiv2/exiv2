// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFSAMPLE_INT_HPP
#define BMFFSAMPLE_INT_HPP

#include "bmffbox_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

//! @brief One locally stored sample; the unchanged input retains ownership of its bytes.
struct BmffSingleSample {
  BmffSpan data;                //!< Absolute input range, validated against the input length.
  uint32_t descriptionIndex{};  //!< One-based stsd index supplied by the sample-to-chunk entry.
};

/*!
  @brief Resolve a single-sample, single-chunk standard sample table.
  @param reader Reader borrowing the open, unchanged input; its budgets apply.
  @param table Validated stbl box whose immediate tables are inspected.
  @param depth Nesting depth of the table's immediate children.
  @return The sample range and description index, or no value for missing or
          unsupported tables, including multiple samples/chunks.
  @throws Error For malformed/ambiguous supported tables, exhausted budgets, or I/O failure.

  Supports version-zero stsc/stsz and stco/co64, including a variable stsz entry.
  No descriptions, codecs, brands or media ownership are interpreted here. The
  caller validates the description and whether the range is in a media payload.
  The stream position may change. No payload bytes are copied or retained.
 */
std::optional<BmffSingleSample> readBmffSingleSample(BmffReader& reader, const BmffBox& table, unsigned depth);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // BMFFSAMPLE_INT_HPP
