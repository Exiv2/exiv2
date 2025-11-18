// SPDX-License-Identifier: GPL-2.0-or-later
/*!
  @file    canonmn_int.hpp
  @brief   Canon makernote tags.<BR>References:<BR>
           [1] <a href="http://www.burren.cx/david/canon.html">EXIF MakerNote of Canon</a> by David Burren<br>
           [2] <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Canon.html">Canon makernote tags</a> by
  Phil Harvey
  @author  <a href="mailto:ahuggel@gmx.net">Andreas Huggel (ahu)</a><br>
           <a href="mailto:david@edeca.net">David Cannings (dc)</a>
           <a href="mailto:andi.clemens@gmx.net">Andi Clemens (ac)</a>
  @date    18-Feb-04, ahu: created<BR>
           07-Mar-04, ahu: isolated as a separate component<BR>
           12-Aug-06,  dc: started updating all tags
 */
#ifndef EXIV2_CANONMN_INT_HPP
#define EXIV2_CANONMN_INT_HPP

// *****************************************************************************
// included header files
#include <cstdint>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
class ExifData;
class Value;
struct TagInfo;

namespace Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Canon cameras
class CanonMakerNote {
 public:
  //! Return read-only list of built-in Canon tags
  static constexpr auto tagList() {
    return tagInfo_;
  }
  //! Return read-only list of built-in Canon Camera Settings tags
  static constexpr auto tagListCs() {
    return tagInfoCs_;
  }
  //! Return read-only list of built-in Canon Shot Info tags
  static constexpr auto tagListSi() {
    return tagInfoSi_;
  }
  //! Return read-only list of built-in Canon Panorama tags
  static constexpr auto tagListPa() {
    return tagInfoPa_;
  }
  //! Return read-only list of built-in Canon Custom Function tags
  static constexpr auto tagListCf() {
    return tagInfoCf_;
  }
  //! Return read-only list of built-in Canon Picture Info tags
  static constexpr auto tagListPi() {
    return tagInfoPi_;
  }
  //! Return read-only list of built-in Canon Time Info tags
  static constexpr auto tagListTi() {
    return tagInfoTi_;
  }
  //! Return read-only list of built-in Canon File Info tags
  static constexpr auto tagListFi() {
    return tagInfoFi_;
  }
  //! Return read-only list of built-in Canon Processing Info tags
  static constexpr auto tagListPr() {
    return tagInfoPr_;
  }
  //! Return read-only list of built-in Canon Movie Info tags
  static constexpr auto tagListMv() {
    return tagInfoMv_;
  }
  //! Return read-only list of built-in Canon My Colors Info tags
  static constexpr auto tagListMc() {
    return tagInfoMc_;
  }
  //! Return read-only list of built-in Canon Face detection Info tags
  static constexpr auto tagListFcd3() {
    return tagInfoFcd3_;
  }
  //! Return read-only list of built-in Canon White balance Info tags
  static constexpr auto tagListWbi() {
    return tagInfoWbi_;
  }
  //! Return read-only list of built-in Canon Contrast Info tags
  static constexpr auto tagListCo() {
    return tagInfoCo_;
  }
  //! Return read-only list of built-in Canon Face detection 1 Info tags
  static constexpr auto tagListFcd2() {
    return tagInfoFcd2_;
  }
  //! Return read-only list of built-in Canon Face detection 2 Info tags
  static constexpr auto tagListFcd1() {
    return tagInfoFcd1_;
  }
  //! Return read-only list of built-in Canon Aspect Info tags
  static constexpr auto tagListAs() {
    return tagInfoAs_;
  }
  //! Return read-only list of built-in Canon Balance Info tags
  static constexpr auto tagListCbi() {
    return tagInfoCbi_;
  }
  //! Return read-only list of built-in Canon Flags Info tags
  static constexpr auto tagListFl() {
    return tagInfoFl_;
  }
  //! Return read-only list of built-in Canon Modified On Info tags
  static constexpr auto tagListMo() {
    return tagInfoMo_;
  }
  //! Return read-only list of built-in Canon Preview Image Info tags
  static constexpr auto tagListPreI() {
    return tagInfoPreI_;
  }
  //! Return read-only list of built-in Canon Color Info tags
  static constexpr auto tagListCi() {
    return tagInfoCi_;
  }
  //! Return read-only list of built-in Canon AFMicroAdjMode Quality Info tags
  static constexpr auto tagListAfMiAdj() {
    return tagInfoAfMiAdj_;
  }
  //! Return read-only list of built-in Canon VignettingCorr Info tags
  static constexpr auto tagListVigCor() {
    return tagInfoVigCor_;
  }
  //! Return read-only list of built-in Canon VignettingCorr2 Info tags
  static constexpr auto tagListVigCor2() {
    return tagInfoVigCor2_;
  }
  //! Return read-only list of built-in Canon LightingOpt Info tags
  static constexpr auto tagListLiOp() {
    return tagInfoLiOp_;
  }
  //! Return read-only list of built-in Canon LensInfo Info tags
  static constexpr auto tagListLe() {
    return tagInfoLe_;
  }
  //! Return read-only list of built-in Canon Ambience Info tags
  static constexpr auto tagListAm() {
    return tagInfoAm_;
  }
  //! Return read-only list of built-in Canon MultiExposureControl Info tags
  static constexpr auto tagListMe() {
    return tagInfoMe_;
  }
  //! Return read-only list of built-in Canon Filter Info tags
  static constexpr auto tagListFil() {
    return tagInfoFil_;
  }
  //! Return read-only list of built-in Canon HDR Info tags
  static constexpr auto tagListHdr() {
    return tagInfoHdr_;
  }
  //! Return read-only list of built-in Canon AFConfig Info tags
  static constexpr auto tagListAfC() {
    return tagInfoAfC_;
  }
  //! Return read-only list of built-in Canon RawBurstInfo Info tags
  static constexpr auto tagListRawB() {
    return tagInfoRawB_;
  }

  //! @name Print functions for Canon %MakerNote tags
  //@{
  //! Print the FileInfo FileNumber
  static std::ostream& printFiFileNumber(std::ostream& os, const Value& value, const ExifData* metadata);
  //! Print the focal length
  static std::ostream& printFocalLength(std::ostream& os, const Value& value, const ExifData*);
  //! Print the image number
  static std::ostream& print0x0008(std::ostream& os, const Value& value, const ExifData*);
  //! Print 0x000a
  static std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData*);
  //! Print the serial number of the camera
  static std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData*);
  //! Self timer
  static std::ostream& printCs0x0002(std::ostream& os, const Value& value, const ExifData*);
  //! Camera lens type. For some values, the exact type can only be determined if \em metadata is provided.
  static std::ostream& printCsLensType(std::ostream& os, const Value& value, const ExifData* metadata);
  //! Camera lens information
  static std::ostream& printCsLens(std::ostream& os, const Value& value, const ExifData*);
  //! CanonLe lens serial number
  static std::ostream& printLe0x0000(std::ostream& os, const Value& value, const ExifData*);
  //! AutoISO speed used
  static std::ostream& printSi0x0001(std::ostream& os, const Value& value, const ExifData*);
  //! ISO speed used
  static std::ostream& printSi0x0002(std::ostream& os, const Value& value, const ExifData*);
  //! MeasuredEV
  static std::ostream& printSi0x0003(std::ostream& os, const Value& value, const ExifData*);
  //! Sequence number
  static std::ostream& printSi0x0009(std::ostream& os, const Value& value, const ExifData*);
  //! Ambient Temperature
  static std::ostream& printSi0x000c(std::ostream& os, const Value& value, const ExifData*);
  //! Flash Guide Number
  static std::ostream& printSi0x000d(std::ostream& os, const Value& value, const ExifData*);
  //! AF point used
  static std::ostream& printSi0x000e(std::ostream& os, const Value& value, const ExifData* pExifData);
  //! Subject distance
  static std::ostream& printSi0x0013(std::ostream& os, const Value& value, const ExifData*);
  //! Aperture
  static std::ostream& printSi0x0015(std::ostream& os, const Value& value, const ExifData*);
  //! Shutter speed
  static std::ostream& printSi0x0016(std::ostream& os, const Value& value, const ExifData*);
  //! MeasuredEV2
  static std::ostream& printSi0x0017(std::ostream& os, const Value& value, const ExifData*);
  //! Bulb Duration
  static std::ostream& printSi0x0018(std::ostream& os, const Value& value, const ExifData*);
  //! Focus Distance
  static std::ostream& printFiFocusDistance(std::ostream& os, const Value& value, const ExifData*);
  //@}

 private:
  // DATA

  //! Tag information
  static const TagInfo tagInfo_[];
  static const TagInfo tagInfoAfC_[];
  static const TagInfo tagInfoAfMiAdj_[];
  static const TagInfo tagInfoAm_[];
  static const TagInfo tagInfoAs_[];
  static const TagInfo tagInfoCs_[];
  static const TagInfo tagInfoCbi_[];
  static const TagInfo tagInfoSi_[];
  static const TagInfo tagInfoCf_[];
  static const TagInfo tagInfoCi_[];
  static const TagInfo tagInfoCo_[];
  static const TagInfo tagInfoFl_[];
  static const TagInfo tagInfoFil_[];
  static const TagInfo tagInfoLiOp_[];
  static const TagInfo tagInfoLe_[];
  static const TagInfo tagInfoHdr_[];
  static const TagInfo tagInfoMe_[];
  static const TagInfo tagInfoMo_[];
  static const TagInfo tagInfoMv_[];
  static const TagInfo tagInfoMc_[];
  static const TagInfo tagInfoFcd1_[];
  static const TagInfo tagInfoFcd2_[];
  static const TagInfo tagInfoFcd3_[];
  static const TagInfo tagInfoPi_[];
  static const TagInfo tagInfoTi_[];
  static const TagInfo tagInfoFi_[];
  static const TagInfo tagInfoPa_[];
  static const TagInfo tagInfoPr_[];
  static const TagInfo tagInfoPreI_[];
  static const TagInfo tagInfoVigCor_[];
  static const TagInfo tagInfoVigCor2_[];
  static const TagInfo tagInfoWbi_[];
  static const TagInfo tagInfoRawB_[];

};  // class CanonMakerNote

// *****************************************************************************
// template, inline and free functions

/*!
   @brief Convert Canon hex-based EV (modulo 0x20) to real number
          Ported from Phil Harvey's Image::ExifTool::Canon::CanonEv
          by Will Stokes

   0x00 -> 0
   0x0c -> 0.33333
   0x10 -> 0.5
   0x14 -> 0.66666
   0x20 -> 1
   ..
   160 -> 5
   128 -> 4
   143 -> 4.46875
 */
float canonEv(int64_t val);

}  // namespace Internal
}  // namespace Exiv2

#endif  // EXIV2_CANONMN_INT_HPP
