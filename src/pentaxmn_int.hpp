// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PENTAXMN_INT_HPP_
#define PENTAXMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Pentaxfilm cameras
class PentaxMakerNote {
 public:
  //! Return read-only list of built-in Pentaxfilm tags
  static const TagInfo* tagList();

  //! Print Pentax version
  static std::ostream& printVersion(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax resolution
  static std::ostream& printResolution(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax date
  static std::ostream& printDate(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax time
  static std::ostream& printTime(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax exposure
  static std::ostream& printExposure(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax F value
  static std::ostream& printFValue(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax focal length
  static std::ostream& printFocalLength(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax compensation
  static std::ostream& printCompensation(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax temperature
  static std::ostream& printTemperature(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax flash compensation
  static std::ostream& printFlashCompensation(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax bracketing
  static std::ostream& printBracketing(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax shutter count
  static std::ostream& printShutterCount(std::ostream& os, const Value& value, const ExifData*);

 private:
  //! Tag information
  static const TagInfo tagInfo_[];
};  // class PentaxMakerNote

/*!
  @brief Print function to translate Pentax "combi-values" to a description
         by looking up a reference table.
 */
template <int N, const TagDetails (&array)[N], int count, int ignoredcount, int ignoredcountmax>
std::ostream& printCombiTag(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if ((value.count() != count &&
       (value.count() < (count + ignoredcount) || value.count() > (count + ignoredcountmax))) ||
      count > 4) {
    return printValue(os, value, metadata);
  }
  uint32_t l = 0;
  for (int c = 0; c < count; ++c) {
    if (value.toInt64(c) < 0 || value.toInt64(c) > 255) {
      return printValue(os, value, metadata);
    }
    l += (value.toUint32(c) << ((count - c - 1) * 8));
  }
  if (auto td = Exiv2::find(array, l)) {
    os << exvGettext(td->label_);
  } else {
    os << exvGettext("Unknown") << " (0x" << std::setw(2 * count) << std::setfill('0') << std::hex << l << std::dec
       << ")";
  }

  os.flags(f);
  return os;
}

//! Shortcut for the printCombiTag template which requires typing the array name only once.
#define EXV_PRINT_COMBITAG(array, count, ignoredcount) \
  printCombiTag<std::size(array), array, count, ignoredcount, ignoredcount>
//! Shortcut for the printCombiTag template which requires typing the array name only once.
#define EXV_PRINT_COMBITAG_MULTI(array, count, ignoredcount, ignoredcountmax) \
  printCombiTag<std::size(array), array, count, ignoredcount, ignoredcountmax>

}  // namespace Exiv2::Internal

#endif  // #ifndef PENTAXMN_INT_HPP_
