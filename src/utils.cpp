#include "utils.hpp"

#include <cctype>
#include <string>

namespace Exiv2::Internal {

std::string upper(std::string_view str) {
  std::string result;
  result.reserve(str.size());
  for (auto c : str)
    result.push_back(std::toupper(static_cast<unsigned char>(c)));
  return result;
}

std::string lower(std::string_view a) {
  std::string b;
  b.reserve(a.size());
  for (auto c : a)
    b.push_back(std::tolower(static_cast<unsigned char>(c)));
  return b;
}

}  // namespace Exiv2::Internal
