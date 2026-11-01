// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_VALUE_INT_HPP
#define EXIV2_VALUE_INT_HPP

#include "value.hpp"

namespace Exiv2::Internal {

//! @brief Preserve exact array items internally without changing the public setter's empty-input behavior.
struct XmpArrayValueAccess {
  //! @brief Append one parsed or converted text item, including an empty string.
  static void append(XmpArrayValue& value, const std::string& text) {
    value.value_.push_back(text);
  }
};

}  // namespace Exiv2::Internal

#endif  // EXIV2_VALUE_INT_HPP
