#ifndef EXIV2_UTILS_HPP
#define EXIV2_UTILS_HPP

#include <string_view>

namespace Exiv2 {
bool startsWith(const std::string_view& s, const std::string_view& start);
}

#endif  // EXIV2_UTILS_HPP
