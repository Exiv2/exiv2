#include "utils.hpp"

#include <algorithm>
#include <cctype>

namespace Exiv2::Internal {

std::string upper(const std::string& str) {
  std::string result = str;
  std::transform(str.begin(), str.end(), result.begin(), ::toupper);
  return result;
}

std::string lower(const std::string& a) {
  std::string b = a;
  std::transform(a.begin(), a.end(), b.begin(), ::tolower);
  return b;
}

}  // namespace Exiv2::Internal
