#include "utils.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace Exiv2 {
bool startsWith(std::string_view s, std::string_view start) {
  return s.find(start) == 0;
}

std::string upper(const std::string& str) {
  std::string result;
  std::transform(str.begin(), str.end(), std::back_inserter(result), ::toupper);
  return result;
}

std::string lower(const std::string& a) {
  std::string b = a;
  std::transform(a.begin(), a.end(), b.begin(), ::tolower);
  return b;
}

}  // namespace Exiv2
