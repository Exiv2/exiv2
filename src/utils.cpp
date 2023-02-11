#include "utils.hpp"

#include <algorithm>
#include <cctype>

namespace Exiv2::Internal {

std::string upper(const std::string& str) {
  std::string result = str;
  std::transform(str.begin(), str.end(), result.begin(), [](int c) { return static_cast<char>(toupper(c)); });
  return result;
}

std::string lower(const std::string& a) {
  std::string b = a;
  std::transform(a.begin(), a.end(), b.begin(), [](int c) { return static_cast<char>(tolower(c)); });
  return b;
}

}  // namespace Exiv2::Internal
