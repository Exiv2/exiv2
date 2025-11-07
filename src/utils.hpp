#ifndef EXIV2_UTILS_HPP
#define EXIV2_UTILS_HPP

#include <string>
#include <string_view>

namespace Exiv2::Internal {

template <typename T>
constexpr bool contains(std::string_view s, T c) {
#ifdef __cpp_lib_string_contains
  return s.contains(c);
#else
  return s.find(c) != std::string_view::npos;
#endif
}

/// @brief Returns the uppercase version of \b str
std::string upper(std::string_view str);

/// @brief Returns the lowercase version of \b str
std::string lower(std::string_view a);

}  // namespace Exiv2::Internal

#endif  // EXIV2_UTILS_HPP
