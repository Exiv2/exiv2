#include "utils.hpp"

namespace Exiv2 {
bool startsWith(const std::string_view& s, const std::string_view& start) {
  return s.find(start) == 0;
}
}  // namespace Exiv2
