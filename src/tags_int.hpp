// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TAGS_INT_HPP_
#define TAGS_INT_HPP_

// *****************************************************************************
// included header files
#include "error.hpp"
#include "tags.hpp"

// *****************************************************************************
// namespace extensions

namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

//! The details of a section.
struct SectionInfo {
  SectionId sectionId_;  //!< Section id
  const char* name_;     //!< Section name (one word)
  const char* desc_;     //!< Section description
};

/*!
  @brief Helper structure for lookup tables for translations of numeric
         tag values to human readable labels.
 */
struct TagDetails {
  int64_t val_;        //!< Tag value
  const char* label_;  //!< Translation of the tag value

  //! Comparison operator for use with the find template
  bool operator==(int64_t key) const {
    return val_ == key;
  }
};  // struct TagDetails

/*!
  @brief Helper structure for lookup tables for translations of string
         tag values to human readable labels.
 */
struct StringTagDetails {
  const char* val_;    //!< Tag value
  const char* label_;  //!< Translation of the tag value

  //! Comparison operator for use with the find template
  bool operator==(const std::string& key) const {
    return (key == val_);
  }
};  // struct TagDetails

/*!
  @brief Helper structure for lookup tables for translations of bitmask
         values to human readable labels.
 */
using TagDetailsBitmask = std::pair<uint32_t, const char*>;

/*!
  @brief Helper structure for lookup tables for translations of lists of
         individual bit values to human readable labels.
         Required to be sorted by the first element (e.g.,
         {{0, N_("Center")}, {1, N_("Top")}, {2, N_("Upper-right")},
         {3, N_("Right")}})
 */
using TagDetailsBitlistSorted = std::pair<uint32_t, const char*>;

/*!
  @brief Helper structure for lookup tables for translations of controlled
         vocabulary strings to their descriptions.
 */
struct TagVocabulary {
  const char* voc_;    //!< Vocabulary string
  const char* label_;  //!< Description of the vocabulary string

  /*!
    @brief Comparison operator for use with the find template

    Compare vocabulary strings like "PR-NON" with keys like
    "http://ns.useplus.org/ldf/vocab/PR-NON" and return true if the vocabulary
    string matches the end of the key.
   */
  bool operator==(const std::string& key) const;
};  // struct TagDetails

/*!
  @brief Generic pretty-print function to translate a full string value to a description
         by looking up a reference table.
 */
template <size_t N, const StringTagDetails (&array)[N]>
std::ostream& printTagString(std::ostream& os, const std::string& value, const ExifData*) {
  if (auto td = Exiv2::find(array, value)) {
    os << exvGettext(td->label_);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

/*!
  @brief Generic pretty-print function to translate the full string value in Value, to a description
         by looking up a reference table.
 */
template <size_t N, const StringTagDetails (&array)[N]>
std::ostream& printTagString(std::ostream& os, const Value& value, const ExifData* data) {
  return printTagString<N, array>(os, value.toString(0), data);
}

//! Shortcut for the printStringTag template which requires typing the array name only once.
#define EXV_PRINT_STRING_TAG_1(array) printTagString<std::size(array), array>

/*!
  @brief Generic pretty-print function to translate the first 2 values in Value as a string,
         to a description by looking up a reference table.
 */
template <size_t N, const StringTagDetails (&array)[N]>
std::ostream& printTagString2(std::ostream& os, const Value& value, const ExifData* data) {
  if (value.count() < 2)
    return os << "(" << value << ")";
  std::string temp = value.toString(0) + " " + value.toString(1);
  return printTagString<N, array>(os, temp, data);
}

//! Shortcut for the printTagString2 template which requires typing the array name only once.
#define EXV_PRINT_STRING_TAG_2(array) printTagString2<std::size(array), array>

/*!
  @brief Generic pretty-print function to translate the first 4 values in Value as a string,
         to a description by looking up a reference table.
 */
template <size_t N, const StringTagDetails (&array)[N]>
std::ostream& printTagString4(std::ostream& os, const Value& value, const ExifData* data) {
  if (value.count() < 4)
    return os << "(" << value << ")";
  std::string temp = value.toString(0) + " " + value.toString(1) + " " + value.toString(2) + " " + value.toString(3);
  return printTagString<N, array>(os, temp, data);
}

//! Shortcut for the printTagString4 template which requires typing the array name only once.
#define EXV_PRINT_STRING_TAG_4(array) printTagString4<std::size(array), array>

/*!
  @brief Generic pretty-print function to translate a long value to a description
         by looking up a reference table. Unknown values are passed through without error.
 */
template <size_t N, const TagDetails (&array)[N]>
std::ostream& printTagNoError(std::ostream& os, const int64_t value, const ExifData*) {
  if (auto td = Exiv2::find(array, value)) {
    os << exvGettext(td->label_);
  } else {
    os << value;
  }
  return os;
}

/*!
  @brief Generic pretty-print function to translate the full string value in Value, to a description
         by looking up a reference table.
 */
template <size_t N, const TagDetails (&array)[N]>
std::ostream& printTagNoError(std::ostream& os, const Value& value, const ExifData* data) {
  return printTagNoError<N, array>(os, value.toInt64(), data);
}

//! Shortcut for the printStringTag template which requires typing the array name only once.
#define EXV_PRINT_TAG_NO_ERROR(array) printTagNoError<std::size(array), array>

/*!
  @brief Generic pretty-print function to translate a long value to a description
         by looking up a reference table.
 */
template <size_t N, const TagDetails (&array)[N]>
std::ostream& printTag(std::ostream& os, const int64_t value, const ExifData*) {
  if (auto td = Exiv2::find(array, value)) {
    os << exvGettext(td->label_);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

/*!
  @brief Generic pretty-print function to translate the first long value in Value, to a description
         by looking up a reference table.
 */
template <size_t N, const TagDetails (&array)[N]>
std::ostream& printTag(std::ostream& os, const Value& value, const ExifData* data) {
  return printTag<N, array>(os, value.toInt64(), data);
}

//! Shortcut for the printTag template which requires typing the array name only once.
#define EXV_PRINT_TAG(array) printTag<std::size(array), array>

/*!
  @brief Generic print function to translate a long value to a description
         by looking up bitmasks in a reference table.
 */
template <size_t N, const TagDetailsBitmask (&array)[N]>
std::ostream& printTagBitmask(std::ostream& os, const Value& value, const ExifData*) {
  const auto val = value.toUint32();
  if (val == 0 && N > 0) {
    auto [mask, label] = *array;
    if (mask == 0)
      return os << exvGettext(label);
  }
  bool sep = false;
  for (size_t i = 0; i < N; ++i) {
    auto [mask, label] = *(array + i);

    if (val & mask) {
      if (sep) {
        os << ", " << exvGettext(label);
      } else {
        os << exvGettext(label);
        sep = true;
      }
    }
  }
  return os;
}

//! Shortcut for the printTagBitmask template which requires typing the array name only once.
#define EXV_PRINT_TAG_BITMASK(array) printTagBitmask<std::size(array), array>

/*!
  @brief Generic print function to translate the bits in the values
         by looking up the indices in a reference table. The function
         processes the values using little endian format. Any bits not
         found in the array are also output.
 */
template <size_t N, const TagDetailsBitlistSorted (&array)[N]>
std::ostream& printTagBitlistAllLE(std::ostream& os, const Value& value, const ExifData*) {
  if constexpr (N == 0)
    throw Error(ErrorCode::kerErrorMessage, std::string("Passed zero length TagDetailsBitlistSorted"));

  uint32_t vN = 0;
  uint32_t currentVNBit = 0;
  size_t lastArrayPos = 0;  // Prevents unneeded searching of array
  constexpr auto maxArrayBit = (array + N - 1)->first;
  auto allVNZero = true;
  auto useSep = false;

  // For each value
  for (size_t i = 0; i < value.count(); i++) {
    vN = value.toUint32(i);
    if (vN == 0) {  // If all bits zero, then nothing to process
      currentVNBit += 8;
      continue;
    }
    allVNZero = false;
    // Cycle through every bit in that byte
    for (auto j = 0; j < 8; j++, currentVNBit++) {
      if ((vN >> j & 0x01) == 0) {  // If bit not set, then nothing to process
        continue;
      }
      if (currentVNBit > maxArrayBit) {  // If beyond array values, output unknown index
        os << ", [" << currentVNBit << "]";
        continue;
      }

      // Check to see if the numbered bit is found in the array
      for (size_t k = lastArrayPos; k < N; ++k) {
        auto [bit, label] = *(array + k);

        if (currentVNBit == bit) {
          lastArrayPos = k;
          if (useSep) {
            os << ", " << exvGettext(label);
          } else {
            os << exvGettext(label);
            useSep = true;
          }
          break;
        }
      }
    }
  }
  if (allVNZero)
    os << exvGettext("None");
  return os;
}

//! Shortcut for the printTagBitlistAllLE template which requires typing the array name only once.
#define EXV_PRINT_TAG_BITLIST_ALL_LE(array) printTagBitlistAllLE<std::size(array), array>

/*!
  @brief Generic pretty-print function to translate a controlled vocabulary value (string)
         to a description by looking up a reference table.
 */
template <size_t N, const TagVocabulary (&array)[N]>
std::ostream& printTagVocabulary(std::ostream& os, const Value& value, const ExifData*) {
  if (auto td = Exiv2::find(array, value.toString())) {
    os << exvGettext(td->label_);
  } else {
    os << "(" << value << ")";
  }
  return os;
}

//! Shortcut for the printTagVocabulary template which requires typing the array name only once.
#define EXV_PRINT_VOCABULARY(array) printTagVocabulary<std::size(array), array>

template <size_t N, const TagVocabulary (&array)[N]>
std::ostream& printTagVocabularyMulti(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() == 0) {
    os << "(" << value << ")";
    return os;
  }

  for (size_t i = 0; i < value.count(); i++) {
    if (i != 0)
      os << ", ";
    auto td = Exiv2::find(array, value.toString(i));
    if (td) {
      os << exvGettext(td->label_);
    } else {
      os << "(" << value.toString(i) << ")";
    }
  }

  return os;
}

//! Shortcut for the printTagVocabularyMulti template which requires typing the array name only once.
#define EXV_PRINT_VOCABULARY_MULTI(array) printTagVocabularyMulti<std::size(array), array>

// *****************************************************************************
// free functions

//! Return read-only list of built-in IFD0/1 tags
const TagInfo* ifdTagList();
//! Return read-only list of built-in Exif IFD tags
const TagInfo* exifTagList();
//! Return read-only list of built-in IOP tags
const TagInfo* iopTagList();
//! Return read-only list of built-in GPS tags
const TagInfo* gpsTagList();
//! Return read-only list of built-in Exiv2 Makernote info tags
const TagInfo* mnTagList();
//! Return read-only list of built-in mfp Tags http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/MPF.html
const TagInfo* mpfTagList();

const GroupInfo* groupList();
const TagInfo* tagList(const std::string& groupName);

//! Return the group id for a group name
IfdId groupId(const std::string& groupName);
//! Return the name of the IFD
const char* ifdName(IfdId ifdId);
//! Return the group name for a group id
const char* groupName(IfdId ifdId);

//! Return true if \em ifdId is a makernote IFD id. (Note: returns false for makerIfd)
bool isMakerIfd(IfdId ifdId);
//! Return true if \em ifdId is an %Exif IFD id.
bool isExifIfd(IfdId ifdId);

//! Print the list of tags for \em ifdId to the output stream \em os
void taglist(std::ostream& os, IfdId ifdId);
//! Return the tag list for \em ifdId
const TagInfo* tagList(IfdId ifdId);
//! Return the tag info for \em tag and \em ifdId
const TagInfo* tagInfo(uint16_t tag, IfdId ifdId);
//! Return the tag info for \em tagName and \em ifdId
const TagInfo* tagInfo(const std::string& tagName, IfdId ifdId);
/*!
  @brief Return the tag number for one combination of IFD id and tagName.
         If the tagName is not known, it expects tag names in the
         form "0x01ff" and converts them to unsigned integer.

  @throw Error if the tagname or ifdId is invalid
 */
uint16_t tagNumber(const std::string& tagName, IfdId ifdId);

//! @name Functions printing interpreted tag values
//@{
//! Default print function, using the Value output operator
std::ostream& printValue(std::ostream& os, const Value& value, const ExifData*);
//! Print the value converted to a int64_t
std::ostream& printInt64(std::ostream& os, const Value& value, const ExifData*);
//! Print a Rational or URational value in floating point format
std::ostream& printFloat(std::ostream& os, const Value& value, const ExifData*);
//! Print a longitude or latitude value
std::ostream& printDegrees(std::ostream& os, const Value& value, const ExifData*);
//! Print function converting from UCS-2LE to UTF-8
std::ostream& printUcs2(std::ostream& os, const Value& value, const ExifData*);
//! Print function for Exif units
std::ostream& printExifUnit(std::ostream& os, const Value& value, const ExifData*);
//! Print function for lens specification
std::ostream& printLensSpecification(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS version
std::ostream& print0x0000(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS altitude ref
std::ostream& print0x0005(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS altitude
std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS timestamp
std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS status
std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS measurement mode
std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS speed ref
std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS destination distance ref
std::ostream& print0x0019(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS differential correction
std::ostream& print0x001e(std::ostream& os, const Value& value, const ExifData*);
//! Print orientation
std::ostream& print0x0112(std::ostream& os, const Value& value, const ExifData*);
//! Print YCbCrPositioning
std::ostream& print0x0213(std::ostream& os, const Value& value, const ExifData*);
//! Print the copyright
std::ostream& print0x8298(std::ostream& os, const Value& value, const ExifData*);
//! Print the exposure time
std::ostream& print0x829a(std::ostream& os, const Value& value, const ExifData*);
//! Print the f-number
std::ostream& print0x829d(std::ostream& os, const Value& value, const ExifData*);
//! Print exposure program
std::ostream& print0x8822(std::ostream& os, const Value& value, const ExifData*);
//! Print ISO speed ratings
std::ostream& print0x8827(std::ostream& os, const Value& value, const ExifData*);
//! Print components configuration specific to compressed data
std::ostream& print0x9101(std::ostream& os, const Value& value, const ExifData*);
//! Print exposure time converted from APEX shutter speed value
std::ostream& print0x9201(std::ostream& os, const Value& value, const ExifData*);
//! Print f-number converted from APEX aperture value
std::ostream& print0x9202(std::ostream& os, const Value& value, const ExifData*);
//! Print the exposure bias value
std::ostream& print0x9204(std::ostream& os, const Value& value, const ExifData*);
//! Print the subject distance
std::ostream& print0x9206(std::ostream& os, const Value& value, const ExifData*);
//! Print metering mode
std::ostream& print0x9207(std::ostream& os, const Value& value, const ExifData*);
//! Print light source
std::ostream& print0x9208(std::ostream& os, const Value& value, const ExifData*);
//! Print the actual focal length of the lens
std::ostream& print0x920a(std::ostream& os, const Value& value, const ExifData*);
//! Print color space
std::ostream& print0xa001(std::ostream& os, const Value& value, const ExifData*);
//! Print sensing method
std::ostream& print0xa217(std::ostream& os, const Value& value, const ExifData*);
//! Print file source
std::ostream& print0xa300(std::ostream& os, const Value& value, const ExifData*);
//! Print scene type
std::ostream& print0xa301(std::ostream& os, const Value& value, const ExifData*);
//! Print custom rendered
std::ostream& print0xa401(std::ostream& os, const Value& value, const ExifData*);
//! Print exposure mode
std::ostream& print0xa402(std::ostream& os, const Value& value, const ExifData*);
//! Print white balance
std::ostream& print0xa403(std::ostream& os, const Value& value, const ExifData*);
//! Print digital zoom ratio
std::ostream& print0xa404(std::ostream& os, const Value& value, const ExifData*);
//! Print 35mm equivalent focal length
std::ostream& print0xa405(std::ostream& os, const Value& value, const ExifData*);
//! Print scene capture type
std::ostream& print0xa406(std::ostream& os, const Value& value, const ExifData*);
//! Print gain control
std::ostream& print0xa407(std::ostream& os, const Value& value, const ExifData*);
//! Print saturation
std::ostream& print0xa409(std::ostream& os, const Value& value, const ExifData*);
//! Print subject distance range
std::ostream& print0xa40c(std::ostream& os, const Value& value, const ExifData*);
//! Print GPS direction ref
std::ostream& printGPSDirRef(std::ostream& os, const Value& value, const ExifData*);
//! Print contrast, sharpness (normal, soft, hard)
std::ostream& printNormalSoftHard(std::ostream& os, const Value& value, const ExifData*);
//! Print any version packed in 4 Bytes format : major major minor minor
std::ostream& printExifVersion(std::ostream& os, const Value& value, const ExifData*);
//! Print any version encoded in the ASCII string majormajorminorminor
std::ostream& printXmpVersion(std::ostream& os, const Value& value, const ExifData*);
//! Print a date following the format YYYY-MM-DDTHH:MM:SSZ
std::ostream& printXmpDate(std::ostream& os, const Value& value, const ExifData*);
//! Print a bitmask as (none) | n | n,m... where: (none) = no bits set | n = bit n from left (0=left-most) | n,m.. =
//! multiple bits "
std::ostream& printBitmask(std::ostream& os, const Value& value, const ExifData*);
//@}

//! Calculate F number from an APEX aperture value
float fnumber(float apertureValue);

//! Calculate the exposure time from an APEX shutter speed value
URational exposureTime(float shutterSpeedValue);

}  // namespace Exiv2::Internal

#endif  // #ifndef TAGS_INT_HPP_
