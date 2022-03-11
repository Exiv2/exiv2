// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SONYMN_INT_HPP_
#define SONYMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffcomposite_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {

    // *****************************************************************************
    // class definitions

    //! MakerNote for Sony cameras
    class SonyMakerNote {
    public:
        //! Return read-only list of built-in Sony tags
        static const TagInfo* tagList();
        //! Return read-only list of built-in Sony Standard Camera Settings tags
        static const TagInfo* tagListCs();
        //! Return read-only list of built-in Sony Standard Camera Settings version 2 tags
        static const TagInfo* tagListCs2();
        //! Return read-only list of built-in Sony FocusPosition tags
        static const TagInfo* tagListFp();
        //! Return read-only list of built-in Sony Misc1 tags (Tag 9403)
        static const TagInfo* tagListSonyMisc1();
        //! Return read-only list of built-in Sony Misc2b tags (Tag 9404)
        static const TagInfo* tagListSonyMisc2b();
        //! Return read-only list of built-in Sony Misc3c tags (Tag 9400)
        static const TagInfo* tagListSonyMisc3c();
        //! Return read-only list of built-in Sony SInfo1 tags (Tag 3000)
        static const TagInfo* tagListSonySInfo1();
        //! Return read-only list of built-in Sony 2010e tags (Tag 2010)
        static const TagInfo* tagList2010e();

        //! @name Print functions for Sony %MakerNote tags
        //@{
        //! Print Sony SonyMisc1 CameraTemperature values (in degrees Celsius)
        static std::ostream& printSonyMisc1CameraTemperature(std::ostream&, const Value&, const ExifData*);
        //! Print Sony2Fp Focus Mode value
        static std::ostream& printSony2FpFocusMode(std::ostream&, const Value&, const ExifData*);
        //! Print Sony2Fp Focus Position 2 value
        static std::ostream& printSony2FpFocusPosition2(std::ostream&, const Value&, const ExifData* metadata);
        //! Print Sony 2Fp AmbientTemperature values (in degrees Celsius)
        static std::ostream& printSony2FpAmbientTemperature(std::ostream&, const Value&, const ExifData*);
        //! Print SonyMisc2b Lens Zoom Position value
        static std::ostream& printSonyMisc2bLensZoomPosition(std::ostream&, const Value&, const ExifData* metadata);
        //! Print SonyMisc2b Focus Position 2 value
        static std::ostream& printSonyMisc2bFocusPosition2(std::ostream&, const Value&, const ExifData* metadata);
        //! Print SonyMisc3c shot number since power up value
        static std::ostream& printSonyMisc3cShotNumberSincePowerUp(std::ostream&, const Value&, const ExifData*);
        //! Print SonyMisc3c sequence number
        static std::ostream& printSonyMisc3cSequenceNumber(std::ostream&, const Value&, const ExifData*);
        //! Print SonyMisc3c Sony image height value
        static std::ostream& printSonyMisc3cSonyImageHeight(std::ostream&, const Value&, const ExifData* metadata);
        //! Print SonyMisc3c model release year value
        static std::ostream& printSonyMisc3cModelReleaseYear(std::ostream&, const Value&, const ExifData* metadata);
        //! Print SonyMisc3c quality 2 value
        static std::ostream& printSonyMisc3cQuality2(std::ostream&, const Value&, const ExifData* metadata);
        //! Print Sony Camera Model
        static std::ostream& print0xb000(std::ostream&, const Value&, const ExifData*);
        //! Print Full and Preview Image size
        static std::ostream& printImageSize(std::ostream&, const Value&, const ExifData*);

    private:
        //! Tag information
        static const TagInfo tagInfo_[];
        static const TagInfo tagInfoCs_[];
        static const TagInfo tagInfoCs2_[];
        static const TagInfo tagInfoFp_[];
        static const TagInfo tagInfoSonyMisc1_[];
        static const TagInfo tagInfoSonyMisc2b_[];
        static const TagInfo tagInfoSonyMisc3c_[];
        static const TagInfo tagInfoSonySInfo1_[];
        static const TagInfo tagInfo2010e_[];

    }; // class SonyMakerNote

    DataBuf sonyTagDecipher(uint16_t, const byte*, size_t, TiffComponent* const);
    DataBuf sonyTagEncipher(uint16_t, const byte*, size_t, TiffComponent* const);

}  // namespace Exiv2::Internal

#endif                                  // #ifndef SONYMN_INT_HPP_
