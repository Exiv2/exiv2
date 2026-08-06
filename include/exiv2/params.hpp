// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_PARAMS_HPP
#define EXIV2_PARAMS_HPP

// *****************************************************************************
#include "exiv2lib_export.h"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

/*!
  @brief Parameters for the "decode" functions. There are a fairly large
  number of static "decode" functions. Examples are `ExifParser::decode`,
  `TiffParser::decode`, and `XmpParser::decode`. This class is a common
  set of parameters for those functions. It currently only contains a
  `max_recursion_depth_` field, but it will make it easier to add new
  parameters in the future.
 */
class EXIV2API DecodeParams {
 public:
  explicit DecodeParams(size_t max_recursion_depth);

  size_t max_recursion_depth() const {
    return max_recursion_depth_;
  }

 private:
  const size_t max_recursion_depth_;
};

}  // namespace Exiv2

#endif  // EXIV2_PARAMS_HPP
