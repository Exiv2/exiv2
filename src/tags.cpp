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
/*
  File:      tags.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier dot gilles at gmail dot com>
  History:   15-Jan-04, ahu: created
             21-Jan-05, ahu: added MakerNote TagInfo registry and related code
 */
// *****************************************************************************
// included header files
#include "types.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "value.hpp"
#include "convert.hpp"
#include "i18n.h"                // NLS support.

#include "canonmn_int.hpp"
#include "casiomn_int.hpp"
#include "fujimn_int.hpp"
#include "minoltamn_int.hpp"
#include "nikonmn_int.hpp"
#include "olympusmn_int.hpp"
#include "panasonicmn_int.hpp"
#include "pentaxmn_int.hpp"
#include "samsungmn_int.hpp"
#include "sigmamn_int.hpp"
#include "sonymn_int.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstring>


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;


    //! List of all defined Exif sections.
    extern const SectionInfo sectionInfo[] = {
        { sectionIdNotSet, "(UnknownSection)",     N_("Unknown section")              },
        { imgStruct,       "ImageStructure",       N_("Image data structure")         },
        { recOffset,       "RecordingOffset",      N_("Recording offset")             },
        { imgCharacter,    "ImageCharacteristics", N_("Image data characteristics")   },
        { otherTags,       "OtherTags",            N_("Other data")                   },
        { exifFormat,      "ExifFormat",           N_("Exif data structure")          },
        { exifVersion,     "ExifVersion",          N_("Exif version")                 },
        { imgConfig,       "ImageConfig",          N_("Image configuration")          },
        { userInfo,        "UserInfo",             N_("User information")             },
        { relatedFile,     "RelatedFile",          N_("Related file")                 },
        { dateTime,        "DateTime",             N_("Date and time")                },
        { captureCond,     "CaptureConditions",    N_("Picture taking conditions")    },
        { gpsTags,         "GPS",                  N_("GPS information")              },
        { iopTags,         "Interoperability",     N_("Interoperability information") },
        { mpfTags,         "MPF",                  N_("CIPA Multi-Picture Format")    },
        { makerTags,       "Makernote",            N_("Vendor specific information")  },
        { dngTags,         "DngTags",              N_("Adobe DNG tags")               },
        { panaRaw,         "PanasonicRaw",         N_("Panasonic RAW tags")           },
        { tiffEp,          "TIFF/EP",              N_("TIFF/EP tags")                 },
        { tiffPm6,         "TIFF&PM6",             N_("TIFF PageMaker 6.0 tags")      },
        { adobeOpi,        "AdobeOPI",             N_("Adobe OPI tags")               },
        { lastSectionId,   "(LastSection)",        N_("Last section")                 }
    };

} // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    bool TagVocabulary::operator==(const std::string& key) const
    {
        if (strlen(voc_) > key.size()) return false;
        return 0 == strcmp(voc_, key.c_str() + key.size() - strlen(voc_));
    }

    // Unknown Tag
    static const TagInfo unknownTag(0xffff, "Unknown tag", N_("Unknown tag"),
                                    N_("Unknown tag"),
                                    ifdIdNotSet, sectionIdNotSet, asciiString, -1, printValue);




}}                                      // namespace Internal, Exiv2

namespace Exiv2 {

    //! @cond IGNORE
    GroupInfo::GroupName::GroupName(const std::string& groupName)
    {
        g_ = groupName;
    }
    //! @endcond

    bool GroupInfo::operator==(int ifdId) const
    {
        return ifdId_ == ifdId;
    }

    bool GroupInfo::operator==(const GroupName& groupName) const
    {
        return 0 == strcmp(groupName.g_.c_str(), groupName_);
    }

    TagInfo::TagInfo(
        uint16_t tag,
        const char* name,
        const char* title,
        const char* desc,
        int ifdId,
        int sectionId,
        TypeId typeId,
        int16_t count,
        PrintFct printFct
    )
        : tag_(tag), name_(name), title_(title), desc_(desc), ifdId_(ifdId),
          sectionId_(sectionId), typeId_(typeId), count_(count), printFct_(printFct)
    {
    }

    const char* ExifTags::sectionName(const ExifKey& key)
    {
        const TagInfo* ti = tagInfo(key.tag(), static_cast<Internal::IfdId>(key.ifdId()));
        if (ti == 0) return sectionInfo[unknownTag.sectionId_].name_;
        return sectionInfo[ti->sectionId_].name_;
    }

    uint16_t ExifTags::defaultCount(const ExifKey& key)
    {
        const TagInfo* ti = tagInfo(key.tag(), static_cast<Internal::IfdId>(key.ifdId()));
        if (ti == 0) return unknownTag.count_;
        return ti->count_;
    }

    const char* ExifTags::ifdName(const std::string& groupName)
    {
        IfdId ifdId = Internal::groupId(groupName);
        return Internal::ifdName(ifdId);
    }

    bool ExifTags::isMakerGroup(const std::string& groupName)
    {
        IfdId ifdId = Internal::groupId(groupName);
        return Internal::isMakerIfd(ifdId);
    }

    bool ExifTags::isExifGroup(const std::string& groupName)
    {
        IfdId ifdId = Internal::groupId(groupName);
        return Internal::isExifIfd(ifdId);
    }

    void ExifTags::taglist(std::ostream& os)
    {
        const TagInfo* ifd = ifdTagList();
        const TagInfo* exif = exifTagList();
        const TagInfo* gps = gpsTagList();
        const TagInfo* iop = iopTagList();

        for (int i=0; ifd[i].tag_ != 0xffff; ++i) {
            os << ifd[i] << "\n";
        }
        for (int i=0; exif[i].tag_ != 0xffff; ++i) {
            os << exif[i] << "\n";
        }
        for (int i=0; iop[i].tag_ != 0xffff; ++i) {
            os << iop[i] << "\n";
        }
        for (int i=0; gps[i].tag_ != 0xffff; ++i) {
            os << gps[i] << "\n";
        }
    } // ExifTags::taglist

    void ExifTags::taglist(std::ostream& os, const std::string& groupName)
    {
        IfdId ifdId = Internal::groupId(groupName);
        Internal::taglist(os, ifdId);
    }

    //! %Internal Pimpl structure with private members and data of class ExifKey.
    struct ExifKey::Impl {
        //! @name Creators
        //@{
        Impl();                         //!< Default constructor
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Set the key corresponding to the \em tag, \em ifdId and \em tagInfo.
                 The key is of the form '<b>Exif</b>.groupName.tagName'.
         */
        void makeKey(uint16_t tag, IfdId ifdId, const TagInfo* tagInfo);
        /*!
          @brief Parse and convert the key string into tag and IFD Id.
                 Updates data members if the string can be decomposed,
                 or throws \em Error .

          @throw Error if the key cannot be decomposed.
         */
        void decomposeKey(const std::string& key);
        //@}

        //! @name Accessors
        //@{
        //! Return the name of the tag
        std::string tagName() const;
        //@}

        // DATA
        static const char* familyName_; //!< "Exif"

        const TagInfo* tagInfo_;        //!< Tag info
        uint16_t tag_;                  //!< Tag value
        IfdId ifdId_;                   //!< The IFD associated with this tag
        int idx_;                       //!< Unique id of the Exif key in the image
        std::string groupName_;         //!< The group name
        std::string key_;               //!< %Key
    };

    const char* ExifKey::Impl::familyName_ = "Exif";

    ExifKey::Impl::Impl()
        : tagInfo_(0), tag_(0), ifdId_(ifdIdNotSet), idx_(0)
    {
    }

    std::string ExifKey::Impl::tagName() const
    {
        if (tagInfo_ != 0 && tagInfo_->tag_ != 0xffff) {
            return tagInfo_->name_;
        }
        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << tag_;
        return os.str();
    }

    void ExifKey::Impl::decomposeKey(const std::string& key)
    {
        // Get the family name, IFD name and tag name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error(kerInvalidKey, key);
        std::string familyName = key.substr(0, pos1);
        if (0 != strcmp(familyName.c_str(), familyName_)) {
            throw Error(kerInvalidKey, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(kerInvalidKey, key);
        std::string groupName = key.substr(pos0, pos1 - pos0);
        if (groupName.empty()) throw Error(kerInvalidKey, key);
        std::string tn = key.substr(pos1 + 1);
        if (tn.empty()) throw Error(kerInvalidKey, key);

        // Find IfdId
        IfdId ifdId = groupId(groupName);
        if (ifdId == ifdIdNotSet) throw Error(kerInvalidKey, key);
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(kerInvalidKey, key);
        }
        // Convert tag
        uint16_t tag = tagNumber(tn, ifdId);
        // Get tag info
        tagInfo_ = tagInfo(tag, ifdId);
        if (tagInfo_ == 0) throw Error(kerInvalidKey, key);

        tag_ = tag;
        ifdId_ = ifdId;
        groupName_ = groupName;
        // tagName() translates hex tag name (0xabcd) to a real tag name if there is one
        key_ = familyName + "." + groupName + "." + tagName();
    }

    void ExifKey::Impl::makeKey(uint16_t tag, IfdId ifdId, const TagInfo* tagInfo)
    {
        assert(tagInfo != 0);

        tagInfo_ = tagInfo;
        tag_ = tag;
        ifdId_ = ifdId;
        key_ = std::string(familyName_) + "." + groupName_ + "." + tagName();
    }

    ExifKey::ExifKey(uint16_t tag, const std::string& groupName)
        : p_(new Impl)
    {
        IfdId ifdId = groupId(groupName);
        // Todo: Test if this condition can be removed
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(kerInvalidIfdId, ifdId);
        }
        const TagInfo* ti = tagInfo(tag, ifdId);
        if (ti == 0) {
            throw Error(kerInvalidIfdId, ifdId);
        }
        p_->groupName_ = groupName;
        p_->makeKey(tag, ifdId, ti);
    }

    ExifKey::ExifKey(const TagInfo& ti)
        : p_(new Impl)
    {
        IfdId ifdId = static_cast<IfdId>(ti.ifdId_);
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(kerInvalidIfdId, ifdId);
        }
        p_->groupName_ = Exiv2::groupName(ifdId);
        p_->makeKey(ti.tag_, ifdId, &ti);
    }

    ExifKey::ExifKey(const std::string& key)
        : p_(new Impl)
    {
        p_->decomposeKey(key);
    }

    ExifKey::ExifKey(const ExifKey& rhs)
        : Key(rhs), p_(new Impl(*rhs.p_))
    {
    }

    ExifKey::~ExifKey() {}

    ExifKey& ExifKey::operator=(const ExifKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        *p_ = *rhs.p_;
        return *this;
    }

    void ExifKey::setIdx(int idx)
    {
        p_->idx_ = idx;
    }

    std::string ExifKey::key() const
    {
        return p_->key_;
    }

    const char* ExifKey::familyName() const
    {
        return p_->familyName_;
    }

    std::string ExifKey::groupName() const
    {
        return p_->groupName_;
    }

    std::string ExifKey::tagName() const
    {
        return p_->tagName();
    }

    std::string ExifKey::tagLabel() const
    {
        if (p_->tagInfo_ == 0 || p_->tagInfo_->tag_ == 0xffff) return "";
        return _(p_->tagInfo_->title_);
    }

    std::string ExifKey::tagDesc() const
    {
        if (p_->tagInfo_ == 0 || p_->tagInfo_->tag_ == 0xffff) return "";
        return _(p_->tagInfo_->desc_);
    }

    TypeId ExifKey::defaultTypeId() const
    {
        if (p_->tagInfo_ == 0) return unknownTag.typeId_;
        return p_->tagInfo_->typeId_;
    }

    uint16_t ExifKey::tag() const
    {
        return p_->tag_;
    }

    ExifKey::AutoPtr ExifKey::clone() const
    {
        return AutoPtr(clone_());
    }

    ExifKey* ExifKey::clone_() const
    {
        return new ExifKey(*this);
    }

    int ExifKey::ifdId() const
    {
        return p_->ifdId_;
    }

    int ExifKey::idx() const
    {
        return p_->idx_;
    }

    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const TagInfo& ti)
    {
        std::ios::fmtflags f( os.flags() );
        ExifKey exifKey(ti);
        os << exifKey.tagName() << ",\t"
           << std::dec << exifKey.tag() << ",\t"
           << "0x" << std::setw(4) << std::setfill('0')
           << std::right << std::hex << exifKey.tag() << ",\t"
           << exifKey.groupName() << ",\t"
           << exifKey.key() << ",\t"
           << TypeInfo::typeName(exifKey.defaultTypeId()) << ",\t"
           << exifKey.tagDesc();

        os.flags(f);
        return os;
    }

}                                       // namespace Exiv2

