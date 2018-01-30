// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
 *
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
#include "error.hpp"
#include "futils.hpp"
#include "value.hpp"
#include "convert.hpp"
#include "i18n.h"                // NLS support.

#define  TAG_DEFINITIONS
#include "tags_int.hpp"

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
// local declarations

namespace {
    // Print version string from an intermediate string
    std::ostream& printVersion(std::ostream& os, const std::string& str);
}


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    //! List of all known Exif groups. Important: Group name (3rd column) must be unique!
    extern const GroupInfo groupInfo[] = {
        { ifdIdNotSet,     "Unknown IFD", "Unknown", 0 },
        { ifd0Id,          "IFD0",      "Image",        ifdTagList                     },
        { ifd1Id,          "IFD1",      "Thumbnail",    ifdTagList                     },
        { ifd2Id,          "IFD2",      "Image2",       ifdTagList                     },
        { ifd3Id,          "IFD3",      "Image3",       ifdTagList                     },
        { exifId,          "Exif",      "Photo",        exifTagList                    },
        { gpsId,           "GPSInfo",   "GPSInfo",      gpsTagList                     },
        { iopId,           "Iop",       "Iop",          iopTagList                     },
        { mpfId,           "MPF",       "MpfInfo",      mpfTagList                     },
        { subImage1Id,     "SubImage1", "SubImage1",    ifdTagList                     },
        { subImage2Id,     "SubImage2", "SubImage2",    ifdTagList                     },
        { subImage3Id,     "SubImage3", "SubImage3",    ifdTagList                     },
        { subImage4Id,     "SubImage4", "SubImage4",    ifdTagList                     },
        { subImage5Id,     "SubImage5", "SubImage5",    ifdTagList                     },
        { subImage6Id,     "SubImage6", "SubImage6",    ifdTagList                     },
        { subImage7Id,     "SubImage7", "SubImage7",    ifdTagList                     },
        { subImage8Id,     "SubImage8", "SubImage8",    ifdTagList                     },
        { subImage9Id,     "SubImage9", "SubImage9",    ifdTagList                     },
        { subThumb1Id,     "SubThumb1", "SubThumb1",    ifdTagList                     },
        { panaRawId,       "PanaRaw",   "PanasonicRaw", PanasonicMakerNote::tagListRaw },
        { mnId,            "Makernote", "MakerNote",    mnTagList                      },
        { canonId,         "Makernote", "Canon",        CanonMakerNote::tagList        },
        { canonCsId,       "Makernote", "CanonCs",      CanonMakerNote::tagListCs      },
        { canonSiId,       "Makernote", "CanonSi",      CanonMakerNote::tagListSi      },
        { canonCfId,       "Makernote", "CanonCf",      CanonMakerNote::tagListCf      },
        { canonPiId,       "Makernote", "CanonPi",      CanonMakerNote::tagListPi      },
        { canonTiId,       "Makernote", "CanonTi",      CanonMakerNote::tagListTi      },
        { canonFiId,       "Makernote", "CanonFi",      CanonMakerNote::tagListFi      },
        { canonPaId,       "Makernote", "CanonPa",      CanonMakerNote::tagListPa      },
        { canonPrId,       "Makernote", "CanonPr",      CanonMakerNote::tagListPr      },
        { casioId,         "Makernote", "Casio",        CasioMakerNote::tagList        },
        { casio2Id,        "Makernote", "Casio2",       Casio2MakerNote::tagList       },
        { fujiId,          "Makernote", "Fujifilm",     FujiMakerNote::tagList         },
        { minoltaId,       "Makernote", "Minolta",      MinoltaMakerNote::tagList      },
        { minoltaCs5DId,   "Makernote", "MinoltaCs5D",  MinoltaMakerNote::tagListCs5D  },
        { minoltaCs7DId,   "Makernote", "MinoltaCs7D",  MinoltaMakerNote::tagListCs7D  },
        { minoltaCsOldId,  "Makernote", "MinoltaCsOld", MinoltaMakerNote::tagListCsStd },
        { minoltaCsNewId,  "Makernote", "MinoltaCsNew", MinoltaMakerNote::tagListCsStd },
        { nikon1Id,        "Makernote", "Nikon1",       Nikon1MakerNote::tagList       },
        { nikon2Id,        "Makernote", "Nikon2",       Nikon2MakerNote::tagList       },
        { nikon3Id,        "Makernote", "Nikon3",       Nikon3MakerNote::tagList       },
        { nikonPvId,       "Makernote", "NikonPreview", ifdTagList                     },
        { nikonVrId,       "Makernote", "NikonVr",      Nikon3MakerNote::tagListVr     },
        { nikonPcId,       "Makernote", "NikonPc",      Nikon3MakerNote::tagListPc     },
        { nikonWtId,       "Makernote", "NikonWt",      Nikon3MakerNote::tagListWt     },
        { nikonIiId,       "Makernote", "NikonIi",      Nikon3MakerNote::tagListIi     },
        { nikonAfId,       "Makernote", "NikonAf",      Nikon3MakerNote::tagListAf     },
        { nikonAf2Id,      "Makernote", "NikonAf2",     Nikon3MakerNote::tagListAf2    },
        { nikonAFTId,      "Makernote", "NikonAFT",     Nikon3MakerNote::tagListAFT    },
        { nikonFiId,       "Makernote", "NikonFi",      Nikon3MakerNote::tagListFi     },
        { nikonMeId,       "Makernote", "NikonMe",      Nikon3MakerNote::tagListMe     },
        { nikonFl1Id,      "Makernote", "NikonFl1",     Nikon3MakerNote::tagListFl1    },
        { nikonFl2Id,      "Makernote", "NikonFl2",     Nikon3MakerNote::tagListFl2    },
        { nikonFl3Id,      "Makernote", "NikonFl3",     Nikon3MakerNote::tagListFl3    },
        { nikonSi1Id,      "Makernote", "NikonSiD80",   Nikon3MakerNote::tagListSi1    },
        { nikonSi2Id,      "Makernote", "NikonSiD40",   Nikon3MakerNote::tagListSi2    },
        { nikonSi3Id,      "Makernote", "NikonSiD300a", Nikon3MakerNote::tagListSi3    },
        { nikonSi4Id,      "Makernote", "NikonSiD300b", Nikon3MakerNote::tagListSi4    },
        { nikonSi5Id,      "Makernote", "NikonSi02xx",  Nikon3MakerNote::tagListSi5    },
        { nikonSi6Id,      "Makernote", "NikonSi01xx",  Nikon3MakerNote::tagListSi5    },
        { nikonCb1Id,      "Makernote", "NikonCb1",     Nikon3MakerNote::tagListCb1    },
        { nikonCb2Id,      "Makernote", "NikonCb2",     Nikon3MakerNote::tagListCb2    },
        { nikonCb2aId,     "Makernote", "NikonCb2a",    Nikon3MakerNote::tagListCb2a   },
        { nikonCb2bId,     "Makernote", "NikonCb2b",    Nikon3MakerNote::tagListCb2b   },
        { nikonCb3Id,      "Makernote", "NikonCb3",     Nikon3MakerNote::tagListCb3    },
        { nikonCb4Id,      "Makernote", "NikonCb4",     Nikon3MakerNote::tagListCb4    },
        { nikonLd1Id,      "Makernote", "NikonLd1",     Nikon3MakerNote::tagListLd1    },
        { nikonLd2Id,      "Makernote", "NikonLd2",     Nikon3MakerNote::tagListLd2    },
        { nikonLd3Id,      "Makernote", "NikonLd3",     Nikon3MakerNote::tagListLd3    },
        { olympusId,       "Makernote", "Olympus",      OlympusMakerNote::tagList      },
        { olympus2Id,      "Makernote", "Olympus2",     OlympusMakerNote::tagList      },
        { olympusCsId,     "Makernote", "OlympusCs",    OlympusMakerNote::tagListCs    },
        { olympusEqId,     "Makernote", "OlympusEq",    OlympusMakerNote::tagListEq    },
        { olympusRdId,     "Makernote", "OlympusRd",    OlympusMakerNote::tagListRd    },
        { olympusRd2Id,    "Makernote", "OlympusRd2",   OlympusMakerNote::tagListRd2   },
        { olympusIpId,     "Makernote", "OlympusIp",    OlympusMakerNote::tagListIp    },
        { olympusFiId,     "Makernote", "OlympusFi",    OlympusMakerNote::tagListFi    },
        { olympusFe1Id,    "Makernote", "OlympusFe1",   OlympusMakerNote::tagListFe    },
        { olympusFe2Id,    "Makernote", "OlympusFe2",   OlympusMakerNote::tagListFe    },
        { olympusFe3Id,    "Makernote", "OlympusFe3",   OlympusMakerNote::tagListFe    },
        { olympusFe4Id,    "Makernote", "OlympusFe4",   OlympusMakerNote::tagListFe    },
        { olympusFe5Id,    "Makernote", "OlympusFe5",   OlympusMakerNote::tagListFe    },
        { olympusFe6Id,    "Makernote", "OlympusFe6",   OlympusMakerNote::tagListFe    },
        { olympusFe7Id,    "Makernote", "OlympusFe7",   OlympusMakerNote::tagListFe    },
        { olympusFe8Id,    "Makernote", "OlympusFe8",   OlympusMakerNote::tagListFe    },
        { olympusFe9Id,    "Makernote", "OlympusFe9",   OlympusMakerNote::tagListFe    },
        { olympusRiId,     "Makernote", "OlympusRi",    OlympusMakerNote::tagListRi    },
        { panasonicId,     "Makernote", "Panasonic",    PanasonicMakerNote::tagList    },
        { pentaxDngId,     "Makernote", "PentaxDng",    PentaxMakerNote::tagList       },
        { pentaxId,        "Makernote", "Pentax",       PentaxMakerNote::tagList       },
        { samsung2Id,      "Makernote", "Samsung2",     Samsung2MakerNote::tagList     },
        { samsungPvId,     "Makernote", "SamsungPreview", ifdTagList                   },
        { samsungPwId,     "Makernote", "SamsungPictureWizard", Samsung2MakerNote::tagListPw },
        { sigmaId,         "Makernote", "Sigma",        SigmaMakerNote::tagList        },
        { sony1Id,         "Makernote", "Sony1",        SonyMakerNote::tagList         },
        { sony2Id,         "Makernote", "Sony2",        SonyMakerNote::tagList         },
        { sonyMltId,       "Makernote", "SonyMinolta",  MinoltaMakerNote::tagList      },
        { sony1CsId,       "Makernote", "Sony1Cs",      SonyMakerNote::tagListCs       },
        { sony1Cs2Id,      "Makernote", "Sony1Cs2",     SonyMakerNote::tagListCs2      },
        { sony1MltCs7DId,  "Makernote", "Sony1MltCs7D", MinoltaMakerNote::tagListCs7D  },
        { sony1MltCsOldId, "Makernote", "Sony1MltCsOld",MinoltaMakerNote::tagListCsStd },
        { sony1MltCsNewId, "Makernote", "Sony1MltCsNew",MinoltaMakerNote::tagListCsStd },
        { sony1MltCsA100Id,"Makernote","Sony1MltCsA100",MinoltaMakerNote::tagListCsA100},
        { sony2CsId,       "Makernote", "Sony2Cs",      SonyMakerNote::tagListCs       },
        { sony2Cs2Id,      "Makernote", "Sony2Cs2",     SonyMakerNote::tagListCs2      },
        { lastId,          "(Last IFD info)", "(Last IFD item)", 0 }
    };

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

    const TagInfo* ifdTagList()
    {
        return ifdTagInfo;
    }

    const TagInfo* exifTagList()
    {
        return exifTagInfo;
    }

    const TagInfo* gpsTagList()
    {
        return gpsTagInfo;
    }

    const TagInfo* iopTagList()
    {
        return iopTagInfo;
    }

    const TagInfo* mpfTagList()
    {
        return mpfTagInfo;
    }

    const TagInfo* mnTagList()
    {
        return mnTagInfo;
    }

    std::ostream& printValue(std::ostream& os, const Value& value, const ExifData*)
    {
        return os << value;
    }

    IfdId groupId(const std::string& groupName)
    {
        IfdId ifdId = ifdIdNotSet;
        const GroupInfo* ii = find(groupInfo, GroupInfo::GroupName(groupName));
        if (ii != 0) ifdId = static_cast<IfdId>(ii->ifdId_);
        return ifdId;
    }

    const char* ifdName(IfdId ifdId)
    {
        const GroupInfo* ii = find(groupInfo, ifdId);
        if (ii == 0) return groupInfo[0].ifdName_;
        return ii->ifdName_;
    } // ifdName

    const char* groupName(IfdId ifdId)
    {
        const GroupInfo* ii = find(groupInfo, ifdId);
        if (ii == 0) return groupInfo[0].groupName_;
        return ii->groupName_;
    } // groupName

    bool isMakerIfd(IfdId ifdId)
    {
        bool rc = false;
        const GroupInfo* ii = find(groupInfo, ifdId);
        if (ii != 0 && 0 == strcmp(ii->ifdName_, "Makernote")) {
            rc = true;
        }
        return rc;
    } // isMakerIfd

    bool isExifIfd(IfdId ifdId)
    {
        bool rc;
        switch (ifdId) {
        case ifd0Id:
        case exifId:
        case gpsId:
        case iopId:
        case ifd1Id:
        case ifd2Id:
        case ifd3Id:
        case mpfId:
        case subImage1Id:
        case subImage2Id:
        case subImage3Id:
        case subImage4Id:
        case subImage5Id:
        case subImage6Id:
        case subImage7Id:
        case subImage8Id:
        case subImage9Id:
        case subThumb1Id:
        case panaRawId: rc = true; break;
        default:           rc = false; break;
        }
        return rc;
    } // isExifIfd

    void taglist(std::ostream& os, IfdId ifdId)
    {
        const TagInfo* ti = Internal::tagList(ifdId);
        if (ti != 0) {
            for (int k = 0; ti[k].tag_ != 0xffff; ++k) {
                os << ti[k] << "\n";
            }
        }
    } // taglist

    const TagInfo* tagList(IfdId ifdId)
    {
        const GroupInfo* ii = find(groupInfo, ifdId);
        if (ii == 0 || ii->tagList_ == 0) return 0;
        return ii->tagList_();
    } // tagList

    const TagInfo* tagInfo(uint16_t tag, IfdId ifdId)
    {
        const TagInfo* ti = tagList(ifdId);
        if (ti == 0) return 0;
        int idx = 0;
        for (idx = 0; ti[idx].tag_ != 0xffff; ++idx) {
            if (ti[idx].tag_ == tag) break;
        }
        return &ti[idx];
    } // tagInfo

    const TagInfo* tagInfo(const std::string& tagName, IfdId ifdId)
    {
        const TagInfo* ti = tagList(ifdId);
        if (ti == 0) return 0;
        const char* tn = tagName.c_str();
        if (tn == 0) return 0;
        for (int idx = 0; ti[idx].tag_ != 0xffff; ++idx) {
            if (0 == strcmp(ti[idx].name_, tn)) {
                return &ti[idx];
            }
        }
        return 0;
    } // tagInfo

    uint16_t tagNumber(const std::string& tagName, IfdId ifdId)
    {
        const TagInfo* ti = tagInfo(tagName, ifdId);
        if (ti != 0 && ti->tag_ != 0xffff) return ti->tag_;
        if (!isHex(tagName, 4, "0x")) throw Error(7, tagName, ifdId);
        std::istringstream is(tagName);
        uint16_t tag;
        is >> std::hex >> tag;
        return tag;
    } // tagNumber

    std::ostream& printLong(std::ostream& os, const Value& value, const ExifData*)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<long>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printLong

    std::ostream& printFloat(std::ostream& os, const Value& value, const ExifData*)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<float>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printFloat

    std::ostream& printDegrees(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (value.count() == 3) {
            std::ostringstream oss;
            oss.copyfmt(os);
            static const char* unit[] = { "deg", "'", "\"" };
            static const int prec[] = { 7, 5, 3 };
            int n;
            for (n = 2; n > 0; --n) {
                if (value.toRational(n).first != 0) break;
            }
            for (int i = 0; i < n + 1; ++i) {
                const uint32_t z = (uint32_t) value.toRational(i).first;
                const uint32_t d = (uint32_t) value.toRational(i).second;
                if (d == 0)
                {
                    os << "(" << value << ")";
                    os.flags(f);
                    return os;
                }
                // Hack: Need Value::toDouble
                double b = static_cast<double>(z)/d;
                const int p = z % d == 0 ? 0 : prec[i];
                os << std::fixed << std::setprecision(p) << b
                   << unit[i] << " ";
            }
            os.copyfmt(oss);
        }
        else {
            os << value;
        }
        os.flags(f);
        return os;
    } // printDegrees

    std::ostream& printUcs2(std::ostream& os, const Value& value, const ExifData*)
    {
        bool cnv = false;
        if (value.typeId() == unsignedByte && value.size() > 0) {
            DataBuf buf(value.size());
            value.copy(buf.pData_, invalidByteOrder);
            // Strip trailing odd byte due to failing UCS-2 conversion
            if (buf.size_ % 2 == 1)  buf.size_ -=1;

            // Strip trailing UCS-2 0-characters
            while (buf.size_ >= 2) {
                if (buf.pData_[buf.size_ - 1] == 0 && buf.pData_[buf.size_ - 2] == 0) {
                    buf.size_ -= 2;
                } else {
                    break;
                }
            }

            std::string str((const char*)buf.pData_, buf.size_);
            cnv = convertStringCharset(str, "UCS-2LE", "UTF-8");
            if (cnv) os << str;
        }
        if (!cnv) os << value;
        return os;
    } // printUcs2

    std::ostream& printExifUnit(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifUnit)(os, value, metadata);
    }

    std::ostream& print0x0000(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.size() != 4 || value.typeId() != unsignedByte) {
            return os << value;
        }

        for (int i = 0; i < 3; i++) {
            os << value.toLong(i);
            os << ".";
        }
        os << value.toLong(3);

        return os;
    }

    std::ostream& print0x0005(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSAltitudeRef)(os, value, metadata);
    }

    std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        std::ostringstream oss;
        oss.copyfmt(os);
        const int32_t d = value.toRational().second;
        if (d == 0) return os << "(" << value << ")";
        const int p = d > 1 ? 1 : 0;
        os << std::fixed << std::setprecision(p) << value.toFloat() << " m";
        os.copyfmt(oss);

        os.flags(f);
        return os;
    }

    std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (value.count() == 3) {
            for (int i = 0; i < 3; ++i) {
                if (value.toRational(i).second == 0) {
                    return os << "(" << value << ")";
                }
            }
            std::ostringstream oss;
            oss.copyfmt(os);
            const float sec = 3600 * value.toFloat(0)
                              + 60 * value.toFloat(1)
                              + value.toFloat(2);
            int p = 0;
            if (sec != static_cast<int>(sec)) p = 1;

            const int hh = static_cast<int>(sec / 3600);
            const int mm = static_cast<int>((sec - 3600 * hh) / 60);
            const float ss = sec - 3600 * hh - 60 * mm;

            os << std::setw(2) << std::setfill('0') << std::right << hh << ":"
               << std::setw(2) << std::setfill('0') << std::right << mm << ":"
               << std::setw(2 + p * 2) << std::setfill('0') << std::right
               << std::fixed << std::setprecision(p) << ss;

            os.copyfmt(oss);
        }
        else {
            os << value;
        }

        os.flags(f);
        return os;
    }

    std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSStatus)(os, value, metadata);
    }

    std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSMeasureMode)(os, value, metadata);
    }

    std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSSpeedRef)(os, value, metadata);
    }

    std::ostream& print0x0019(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSDestDistanceRef)(os, value, metadata);
    }

    std::ostream& print0x001e(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSDifferential)(os, value, metadata);
    }

    std::ostream& print0x0112(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifOrientation)(os, value, metadata);
    }

    std::ostream& print0x0213(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifYCbCrPositioning)(os, value, metadata);
    }

    std::ostream& print0x8298(std::ostream& os, const Value& value, const ExifData*)
    {
        // Print the copyright information in the format Photographer, Editor
        std::string val = value.toString();
        std::string::size_type pos = val.find('\0');
        if (pos != std::string::npos) {
            std::string photographer(val, 0, pos);
            if (photographer != " ") os << photographer;
            std::string editor(val, pos + 1);
            if (editor != "") {
                if (photographer != " ") os << ", ";
                os << editor;
            }
        }
        else {
            os << val;
        }
        return os;
    }

    std::ostream& print0x829a(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.count() == 0) return os;
        if (value.typeId() != unsignedRational) return os << "(" << value << ")";

        URational t = value.toRational();
        if (t.first == 0 || t.second == 0) {
            os << "(" << t << ")";
        }
        else if (t.second == t.first) {
            os << "1 s";
        }
        else if (t.second % t.first == 0) {
            t.second = t.second / t.first;
            t.first = 1;
            os << t << " s";
        }
        else {
            os << static_cast<float>(t.first) / t.second << " s";
        }
        return os;
    }

    std::ostream& print0x829d(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        Rational fnumber = value.toRational();
        if (fnumber.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fnumber.first) / fnumber.second;
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        os.flags(f);
        return os;
    }

    std::ostream& print0x8822(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifExposureProgram)(os, value, metadata);
    }

    std::ostream& print0x8827(std::ostream& os, const Value& value, const ExifData*)
    {
        return os << value.toLong();
    }

    std::ostream& print0x9101(std::ostream& os, const Value& value, const ExifData*)
    {
        for (long i = 0; i < value.count(); ++i) {
            long l = value.toLong(i);
            switch (l) {
            case 0:  break;
            case 1:  os << "Y"; break;
            case 2:  os << "Cb"; break;
            case 3:  os << "Cr"; break;
            case 4:  os << "R"; break;
            case 5:  os << "G"; break;
            case 6:  os << "B"; break;
            default: os << "(" << l << ")"; break;
            }
        }
        return os;
    }

    std::ostream& print0x9201(std::ostream& os, const Value& value, const ExifData*)
    {
        Rational r = value.toRational();
        if (!value.ok() || r.second == 0) return os << "(" << value << ")";

        URational ur = exposureTime(static_cast<float>(r.first) / r.second);
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

    std::ostream& print0x9202(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.count() == 0
            || value.toRational().second == 0) {
            return os << "(" << value << ")";
        }
        std::ostringstream oss;
        oss.copyfmt(os);
        os << "F" << std::setprecision(2) << fnumber(value.toFloat());
        os.copyfmt(oss);
        os.flags(f);
        return os;
    }

    std::ostream& print0x9204(std::ostream& os, const Value& value, const ExifData*)
    {
        Rational bias = value.toRational();

        if (bias.first == 0 || bias.first == (int32_t)0x80000000 ) {
            os << "0 EV";
        } else if (bias.second <= 0) {
            os << "(" << bias.first << "/" << bias.second << ")";
        } else {
            int32_t d = gcd(bias.first, bias.second);
            int32_t num = std::abs(bias.first) / d;
            int32_t den = bias.second / d;
            os << (bias.first < 0 ? "-" : "+") << num;
            if (den != 1) {
                os << "/" << den;
            }
            os << " EV";
        }
        return os;
    }

    std::ostream& print0x9206(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
        }
        else if (static_cast<uint32_t>(distance.first) == 0xffffffff) {
            os << _("Infinity");
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        os.flags(f);
        return os;
    }

    std::ostream& print0x9207(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifMeteringMode)(os, value, metadata);
    }

    std::ostream& print0x9208(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifLightSource)(os, value, metadata);
    }

    std::ostream& print0x920a(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        Rational length = value.toRational();
        if (length.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)length.first / length.second
               << " mm";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        os.flags(f);
        return os;
    }

    std::ostream& print0x9286(std::ostream& os, const Value& value, const ExifData*)
    {
        const CommentValue* pcv = dynamic_cast<const CommentValue*>(&value);
        if (pcv) {
            os << pcv->comment();
        }
        else {
            os << value;
        }
        return os;
    }

    std::ostream& print0xa001(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifColorSpace)(os, value, metadata);
    }

    std::ostream& print0xa217(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifSensingMethod)(os, value, metadata);
    }

    std::ostream& print0xa300(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifFileSource)(os, value, metadata);
    }

    std::ostream& print0xa301(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifSceneType)(os, value, metadata);
    }

    std::ostream& print0xa401(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifCustomRendered)(os, value, metadata);
    }

    std::ostream& print0xa402(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifExposureMode)(os, value, metadata);
    }

    std::ostream& print0xa403(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifWhiteBalance)(os, value, metadata);
    }

    std::ostream& print0xa404(std::ostream& os, const Value& value, const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        Rational zoom = value.toRational();
        if (zoom.second == 0) {
            os << _("Digital zoom not used");
        }
        else {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second;
            os.copyfmt(oss);
        }
        os.flags(f);
        return os;
    }

    std::ostream& print0xa405(std::ostream& os, const Value& value, const ExifData*)
    {
        long length = value.toLong();
        if (length == 0) {
            os << _("Unknown");
        }
        else {
            os << length << ".0 mm";
        }
        return os;
    }

    std::ostream& print0xa406(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifSceneCaptureType)(os, value, metadata);
    }

    std::ostream& print0xa407(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGainControl)(os, value, metadata);
    }

    std::ostream& print0xa409(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifSaturation)(os, value, metadata);
    }

    std::ostream& print0xa40c(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifSubjectDistanceRange)(os, value, metadata);
    }

    std::ostream& printGPSDirRef(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifGPSDirRef)(os, value, metadata);
    }

    std::ostream& printNormalSoftHard(std::ostream& os, const Value& value, const ExifData* metadata)
    {
        return EXV_PRINT_TAG(exifNormalSoftHard)(os, value, metadata);
    }

    std::ostream& printExifVersion(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.size() != 4 || value.typeId() != undefined) {
            return os << "(" << value << ")";
        }

        char s[5];
        for (int i = 0; i < 4; ++i) {
            s[i] = static_cast<char>(value.toLong(i));
        }
        s[4] = '\0';

        return printVersion(os, s);
    }

    std::ostream& printXmpVersion(std::ostream& os, const Value& value, const ExifData*)
    {
        if (value.size() != 4 || value.typeId() != xmpText) {
            return os << "(" << value << ")";
        }

        return printVersion(os, value.toString());
    }

    std::ostream& printXmpDate(std::ostream& os, const Value& value, const ExifData*)
    {
        if (!(value.size() == 19 || value.size() == 20) || value.typeId() != xmpText) {
            return os << value;
        }

        std::string stringValue = value.toString();
        if (stringValue[19] == 'Z') {
            stringValue = stringValue.substr(0, 19);
        }
        for (unsigned int i = 0; i < stringValue.length(); ++i) {
            if (stringValue[i] == 'T') stringValue[i] = ' ';
            if (stringValue[i] == '-') stringValue[i] = ':';
        }

        return os << stringValue;
    }

    float fnumber(float apertureValue)
    {
        return static_cast<float>(std::exp(std::log(2.0) * apertureValue / 2));
    }

    URational exposureTime(float shutterSpeedValue)
    {
        URational ur(1, 1);
        double tmp = std::exp(std::log(2.0) * shutterSpeedValue);
        if (tmp > 1) {
            ur.second = static_cast<long>(tmp + 0.5);
        }
        else {
            ur.first = static_cast<long>(1/tmp + 0.5);
        }
        return ur;
    }

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

    const GroupInfo* ExifTags::groupList()
    {
        return groupInfo + 1; // +1 to skip the first (ifdIdNotSet) entry
    }

    const TagInfo* ExifTags::tagList(const std::string& groupName)
    {
        const GroupInfo* ii = find(groupInfo, GroupInfo::GroupName(groupName));
        if (ii == 0 || ii->tagList_ == 0) return 0;
        return ii->tagList_();
    } // ExifTags::tagList

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
        for (int i=0; ifdTagInfo[i].tag_ != 0xffff; ++i) {
            os << ifdTagInfo[i] << "\n";
        }
        for (int i=0; exifTagInfo[i].tag_ != 0xffff; ++i) {
            os << exifTagInfo[i] << "\n";
        }
        for (int i=0; iopTagInfo[i].tag_ != 0xffff; ++i) {
            os << iopTagInfo[i] << "\n";
        }
        for (int i=0; gpsTagInfo[i].tag_ != 0xffff; ++i) {
            os << gpsTagInfo[i] << "\n";
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
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string familyName = key.substr(0, pos1);
        if (0 != strcmp(familyName.c_str(), familyName_)) {
            throw Error(6, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string groupName = key.substr(pos0, pos1 - pos0);
        if (groupName.empty()) throw Error(6, key);
        std::string tn = key.substr(pos1 + 1);
        if (tn.empty()) throw Error(6, key);

        // Find IfdId
        IfdId ifdId = groupId(groupName);
        if (ifdId == ifdIdNotSet) throw Error(6, key);
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(6, key);
        }
        // Convert tag
        uint16_t tag = tagNumber(tn, ifdId);
        // Get tag info
        tagInfo_ = tagInfo(tag, ifdId);
        if (tagInfo_ == 0) throw Error(6, key);

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
            throw Error(23, ifdId);
        }
        const TagInfo* ti = tagInfo(tag, ifdId);
        if (ti == 0) {
            throw Error(23, ifdId);
        }
        p_->groupName_ = groupName;
        p_->makeKey(tag, ifdId, ti);
    }

    ExifKey::ExifKey(const TagInfo& ti)
        : p_(new Impl)
    {
        IfdId ifdId = static_cast<IfdId>(ti.ifdId_);
        if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
            throw Error(23, ifdId);
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

    ExifKey::~ExifKey()
    {
        delete p_;
    }

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


namespace {
    std::ostream& printVersion(std::ostream& os, const std::string& str)
    {
        if (str.size() != 4) {
            return os << "(" << str << ")";
        }
        if (str[0] != '0') os << str[0];
        return os << str[1] << "." << str[2] << str[3];
    }
}

