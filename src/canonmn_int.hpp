// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    canonmn_int.hpp
  @brief   Canon makernote tags.<BR>References:<BR>
           [1] <a href="http://www.burren.cx/david/canon.html">EXIF MakerNote of Canon</a> by David Burren<br>
           [2] <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/Canon.html">Canon makernote tags</a> by Phil Harvey
  @author  <a href="mailto:ahuggel@gmx.net">Andreas Huggel (ahu)</a><br>
           <a href="mailto:david@edeca.net">David Cannings (dc)</a>
           <a href="mailto:andi.clemens@gmx.net">Andi Clemens (ac)</a>
  @date    18-Feb-04, ahu: created<BR>
           07-Mar-04, ahu: isolated as a separate component<BR>
           12-Aug-06,  dc: started updating all tags
 */
#ifndef CANONMN_INT_HPP_
#define CANONMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    class Value;

    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Canon cameras
    class CanonMakerNote {
    public:
        //! Return read-only list of built-in Canon tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Canon Camera Settings tags
        static const TagInfo* tagListCs();
        //! Return read-only list of built-in Canon Shot Info tags
        static const TagInfo* tagListSi();
        //! Return read-only list of built-in Canon Panorama tags
        static const TagInfo* tagListPa();
        //! Return read-only list of built-in Canon Custom Function tags
        static const TagInfo* tagListCf();
        //! Return read-only list of built-in Canon Picture Info tags
        static const TagInfo* tagListPi();
        //! Return read-only list of built-in Canon Time Info tags
        static const TagInfo* tagListTi();
        //! Return read-only list of built-in Canon File Info tags
        static const TagInfo* tagListFi();
        //! Return read-only list of built-in Canon Processing Info tags
        static const TagInfo* tagListPr();
        //! Return read-only list of built-in Canon Movie Info tags
        static const TagInfo* tagListMv();
        //! Return read-only list of built-in Canon My Colors Info tags
        static const TagInfo* tagListMc();
        //! Return read-only list of built-in Canon AFInfo3 Info tags
        static const TagInfo* tagListAf3();
        //! Return read-only list of built-in Canon Face detection Info tags
        static const TagInfo* tagListFcd3();
        //! Return read-only list of built-in Canon White balance Info tags
        static const TagInfo* tagListWbi();
        //! Return read-only list of built-in Canon Contrast Info tags
        static const TagInfo* tagListCo();
        //! Return read-only list of built-in Canon Face detection 1 Info tags
        static const TagInfo* tagListFcd2();
        //! Return read-only list of built-in Canon Face detection 2 Info tags
        static const TagInfo* tagListFcd1();
        //! Return read-only list of built-in Canon Aspect Info tags
        static const TagInfo* tagListAs();
        //! Return read-only list of built-in Canon Balance Info tags
        static const TagInfo* tagListCbi();
        //! Return read-only list of built-in Canon Flags Info tags
        static const TagInfo* tagListFl();
        //! Return read-only list of built-in Canon Modified On Info tags
        static const TagInfo* tagListMo();
        //! Return read-only list of built-in Canon Preview Image Info tags
        static const TagInfo* tagListPreI();
        //! Return read-only list of built-in Canon Color Info tags
        static const TagInfo* tagListCi();
        //! Return read-only list of built-in Canon AFMicroAdjMode Quality Info tags
        static const TagInfo* tagListAfMiAdj();
        //! Return read-only list of built-in Canon VignettingCorr Info tags
        static const TagInfo* tagListVigCor();
        //! Return read-only list of built-in Canon VignettingCorr2 Info tags
        static const TagInfo* tagListVigCor2();
        //! Return read-only list of built-in Canon LightingOpt Info tags
        static const TagInfo* tagListLiOp();
        //! Return read-only list of built-in Canon LensInfo Info tags
        static const TagInfo* tagListLe();
        //! Return read-only list of built-in Canon Ambience Info tags
        static const TagInfo* tagListAm();
        //! Return read-only list of built-in Canon MultiExposureControl Info tags
        static const TagInfo* tagListMe();
        //! Return read-only list of built-in Canon Filter Info tags
        static const TagInfo* tagListFil();
        //! Return read-only list of built-in Canon HDR Info tags
        static const TagInfo* tagListHdr();
        //! Return read-only list of built-in Canon AFConfig Info tags
        static const TagInfo* tagListAfC();
        //! Return read-only list of built-in Canon RawBurstInfo Info tags
        static const TagInfo* tagListRawB();
        //! Return read-only list of built-in Canon AFInfo2 Info tags
        static const TagInfo* tagListAf2();
        

        
        

        
        

        //! @name Print functions for Canon %MakerNote tags
        //@{
        //! Print the FileInfo FileNumber
        static std::ostream& printFiFileNumber(std::ostream& os, const Value& value, const ExifData* metadata);
        //! Print the focal length
        static std::ostream& printFocalLength(std::ostream& os, const Value& value, const ExifData*);
        //! Print the image number
        static std::ostream& print0x0008(std::ostream& os, const Value& value, const ExifData*);
        //! Print the serial number of the camera
        static std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData*);
        //! Self timer
        static std::ostream& printCs0x0002(std::ostream& os, const Value& value, const ExifData*);
        //! Camera lens type. For some values, the exact type can only be determined if \em metadata is provided.
        static std::ostream& printCsLensType(std::ostream& os, const Value& value, const ExifData* metadata);
        //! Camera lens information
        static std::ostream& printCsLens(std::ostream& os, const Value& value, const ExifData*);
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
        static const TagInfo tagInfoAf2_[];
        static const TagInfo tagInfoAf3_[];
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
        static const TagInfo tagInfoRawB_ [];
        

    }; // class CanonMakerNote

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
    float canonEv(long val);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef CANONMN_INT_HPP_


