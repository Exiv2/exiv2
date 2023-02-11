#ifndef EXIV2_UTILS_HPP
#define EXIV2_UTILS_HPP

#include <string>
#include <string_view>

namespace Exiv2::Internal {

template <typename T>
constexpr bool startsWith(std::string_view s, T start) {
#ifdef __cpp_lib_starts_ends_with
  return s.starts_with(start);
#else
  return s.find(start) == 0;
#endif
}

/// @brief Returns the uppercase version of \b str
std::string upper(const std::string& str);

/// @brief Returns the lowercase version of \b str
std::string lower(const std::string& a);

}  // namespace Exiv2::Internal

#endif  // EXIV2_UTILS_HPP
