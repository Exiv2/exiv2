// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      makernote2.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "makernote2.hpp"
#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "tiffimage.hpp"

// + standard includes
#include <string>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const TiffMnRegistry TiffMnCreator::registry_[] = {
        { "Canon",          newCanonMn,     Group::canonmn   },
        { "FOVEON",         newSigmaMn,     Group::sigmamn   },
        { "FUJIFILM",       newFujiMn,      Group::fujimn    },
        { "KONICA MINOLTA", newMinoltaMn,   Group::minoltamn },
        { "Minolta",        newMinoltaMn,   Group::minoltamn },
        { "NIKON",          newNikonMn,     Group::nikonmn   },
        { "OLYMPUS",        newOlympusMn,   Group::olympmn   },
        { "Panasonic",      newPanasonicMn, Group::panamn    },
        { "SIGMA",          newSigmaMn,     Group::sigmamn   },
        { "SONY",           newSonyMn,      Group::sonymn    }
    };

    bool TiffMnRegistry::operator==(const TiffMnRegistry::Key& key) const
    {
        std::string make(make_);
        return make == key.make_.substr(0, make.length());
    }

    TiffComponent* TiffMnCreator::create(uint16_t           tag,
                                         uint16_t           group,
                                         const std::string& make,
                                         const byte*        pData,
                                         uint32_t           size,
                                         ByteOrder          byteOrder)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, TiffMnRegistry::Key(make));
        if (tmr) tc = tmr->newMnFct_(tag,
                                     group,
                                     tmr->mnGroup_,
                                     pData,
                                     size,
                                     byteOrder);
        return tc;
    } // TiffMnCreator::create

    TiffIfdMakernote::~TiffIfdMakernote()
    {
        delete pHeader_;
    }

    uint32_t TiffIfdMakernote::ifdOffset() const
    {
        if (!pHeader_) return 0;
        return pHeader_->ifdOffset();
    }

    ByteOrder TiffIfdMakernote::byteOrder () const
    {
        if (!pHeader_) return invalidByteOrder;
        return pHeader_->byteOrder();
    }

    uint32_t TiffIfdMakernote::baseOffset(uint32_t mnOffset) const
    {
        if (!pHeader_) return 0;
        return pHeader_->baseOffset(mnOffset);
    }

    bool TiffIfdMakernote::readHeader(const byte* pData,
                                      uint32_t    size,
                                      ByteOrder   byteOrder)
    {
        if (!pHeader_) return true;
        return pHeader_->read(pData, size, byteOrder);
    }

    void TiffIfdMakernote::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addChild(tiffComponent);
    }

    void TiffIfdMakernote::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        ifd_.addNext(tiffComponent);
    }

    void TiffIfdMakernote::doAccept(TiffVisitor& visitor)
    {
        if (visitor.go()) visitor.visitIfdMakernote(this);
        ifd_.accept(visitor);
        if (visitor.go()) visitor.visitIfdMakernoteEnd(this);
    }

    const byte OlympusMnHeader::signature_[] = {
        'O', 'L', 'Y', 'M', 'P', 0x00, 0x01, 0x00
    };
    const uint32_t OlympusMnHeader::size_ = 8;

    OlympusMnHeader::OlympusMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool OlympusMnHeader::read(const byte* pData,
                               uint32_t size,
                               ByteOrder /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;

        header_.alloc(size_);
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < size_
            || 0 != memcmp(header_.pData_, signature_, 5)) {
            return false;
        }
        return true;
    } // OlympusMnHeader::read

    const byte FujiMnHeader::signature_[] = {
        'F', 'U', 'J', 'I', 'F', 'I', 'L', 'M', 0x0c, 0x00, 0x00, 0x00
    };
    const uint32_t FujiMnHeader::size_ = 12;
    const ByteOrder FujiMnHeader::byteOrder_ = littleEndian;

    FujiMnHeader::FujiMnHeader()
    {
        read(signature_, size_, byteOrder_);
    }

    bool FujiMnHeader::read(const byte* pData,
                            uint32_t size,
                            ByteOrder /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;

        header_.alloc(size_);
        std::memcpy(header_.pData_, pData, header_.size_);

        // Read offset to the IFD relative to the start of the makernote
        // from the header. Note that we ignore the byteOrder argument
        start_ = getULong(header_.pData_ + 8, byteOrder_);

        if (   static_cast<uint32_t>(header_.size_) < size_
            || 0 != memcmp(header_.pData_, signature_, 8)) {
            return false;
        }
        return true;
    } // FujiMnHeader::read


    const byte Nikon2MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
    };
    const uint32_t Nikon2MnHeader::size_ = 8;

    Nikon2MnHeader::Nikon2MnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool Nikon2MnHeader::read(const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(size_);
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // Nikon2MnHeader::read

    const byte Nikon3MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0',
        0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
    };
    const uint32_t Nikon3MnHeader::size_ = 18;

    Nikon3MnHeader::Nikon3MnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool Nikon3MnHeader::read(const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 6)) return false;
        buf_.alloc(size_);
        std::memcpy(buf_.pData_, pData, buf_.size_);
        TiffHeade2 th;
        if (!th.read(buf_.pData_ + 10, 8)) return false;
        byteOrder_ = th.byteOrder();
        start_ = 10 + th.offset();
        return true;

    } // Nikon3MnHeader::read

    const byte PanasonicMnHeader::signature_[] = {
        'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00
    };
    const uint32_t PanasonicMnHeader::size_ = 12;

    PanasonicMnHeader::PanasonicMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool PanasonicMnHeader::read(const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, 9)) return false;
        buf_.alloc(size_);
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // PanasonicMnHeader::read

    const byte SigmaMnHeader::signature1_[] = {
        'S', 'I', 'G', 'M', 'A', '\0', '\0', '\0', 0x01, 0x00
    };
    const byte SigmaMnHeader::signature2_[] = {
        'F', 'O', 'V', 'E', 'O', 'N', '\0', '\0', 0x01, 0x00
    };
    const uint32_t SigmaMnHeader::size_ = 10;

    SigmaMnHeader::SigmaMnHeader()
    {
        read(signature1_, size_, invalidByteOrder);
    }

    bool SigmaMnHeader::read(const byte* pData,
                             uint32_t    size,
                             ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (   0 != memcmp(pData, signature1_, 8)
            && 0 != memcmp(pData, signature2_, 8)) return false;
        buf_.alloc(size_);
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // SigmaMnHeader::read

    const byte SonyMnHeader::signature_[] = {
        'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', '\0', '\0', '\0'
    };
    const uint32_t SonyMnHeader::size_ = 12;

    SonyMnHeader::SonyMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool SonyMnHeader::read(const byte* pData,
                            uint32_t    size,
                            ByteOrder   /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        if (0 != memcmp(pData, signature_, size_)) return false;
        buf_.alloc(size_);
        std::memcpy(buf_.pData_, pData, buf_.size_);
        start_ = size_;
        return true;

    } // SonyMnHeader::read

    // *************************************************************************
    // free functions

    TiffComponent* newCanonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* /*pData*/,
                              uint32_t    /*size*/,
                              ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, 0);
    }

    TiffComponent* newMinoltaMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* /*pData*/,
                                uint32_t    /*size*/,
                                ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, 0);
    }

    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* /*pData*/,
                                uint32_t    /*size*/,
                                ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new OlympusMnHeader);
    }

    TiffComponent* newFujiMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* /*pData*/,
                             uint32_t    /*size*/,
                             ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new FujiMnHeader);
    }

    TiffComponent* newNikonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    /*mnGroup*/,
                              const byte* pData,
                              uint32_t    size,
                              ByteOrder   /*byteOrder*/)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (size < 6 ||    std::string(reinterpret_cast<const char*>(pData), 6)
                        != std::string("Nikon\0", 6)) {
            return new TiffIfdMakernote(tag, group, Group::nikon1mn, 0);
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeade2 tiffHeader;
        if (   size < 18
            || !tiffHeader.read(pData + 10, size - 10)
            || tiffHeader.tag() != 0x002a) {
            return new TiffIfdMakernote(tag, group, Group::nikon2mn, new Nikon2MnHeader);
        }
        // Else we have a Nikon3 makernote
        return new TiffIfdMakernote(tag, group, Group::nikon3mn, new Nikon3MnHeader);
    }

    TiffComponent* newPanasonicMn(uint16_t    tag,
                                  uint16_t    group,
                                  uint16_t    mnGroup,
                                  const byte* /*pData*/,
                                  uint32_t    /*size*/,
                                  ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PanasonicMnHeader, false);
    }

    TiffComponent* newSigmaMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* /*pData*/,
                              uint32_t    /*size*/,
                              ByteOrder   /*byteOrder*/)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new SigmaMnHeader);
    }

    TiffComponent* newSonyMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    /*mnGroup*/,
                             const byte* pData,
                             uint32_t    size,
                             ByteOrder   /*byteOrder*/)
    {
        // If there is no "SONY DSC " string we assume it's a simple IFD Makernote
        if (size < 12 ||   std::string(reinterpret_cast<const char*>(pData), 12)
                        != std::string("SONY DSC \0\0\0", 12)) {
            return new TiffIfdMakernote(tag, group, Group::sony2mn, 0, true);
        }
        return new TiffIfdMakernote(tag, group, Group::sony1mn, new SonyMnHeader, false);
    }

}                                       // namespace Exiv2
