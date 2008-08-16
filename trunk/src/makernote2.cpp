// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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

#include "makernote2_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"

// + standard includes
#include <string>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    const TiffMnRegistry TiffMnCreator::registry_[] = {
        { "Canon",          Group::canonmn,   newIfdMn,       newIfdMn2       },
        { "FOVEON",         Group::sigmamn,   newSigmaMn,     newSigmaMn2     },
        { "FUJIFILM",       Group::fujimn,    newFujiMn,      newFujiMn2      },
        { "KONICA MINOLTA", Group::minoltamn, newIfdMn,       newIfdMn2       },
        { "Minolta",        Group::minoltamn, newIfdMn,       newIfdMn2       },
        { "NIKON",          Group::nikonmn,   newNikonMn,     0               },
        { "OLYMPUS",        Group::olympmn,   newOlympusMn,   newOlympusMn2   },
        { "Panasonic",      Group::panamn,    newPanasonicMn, newPanasonicMn2 },
        { "PENTAX",         Group::pentaxmn,  newPentaxMn,    newPentaxMn2    },
        { "SIGMA",          Group::sigmamn,   newSigmaMn,     newSigmaMn2     },
        { "SONY",           Group::sonymn,    newSonyMn,      0               },
        // Entries below are only used for lookup by group
        { "-",              Group::nikon1mn,  0,              newIfdMn2       },
        { "-",              Group::nikon2mn,  0,              newNikon2Mn2    },
        { "-",              Group::nikon3mn,  0,              newNikon3Mn2    },
        { "-",              Group::sony1mn,   0,              newSony1Mn2     },
        { "-",              Group::sony2mn,   0,              newSony2Mn2     }
    };

    bool TiffMnRegistry::operator==(const std::string& key) const
    {
        std::string make(make_);
        return make == key.substr(0, make.length());
    }

    bool TiffMnRegistry::operator==(const uint16_t& key) const
    {
        return mnGroup_ == key;
    }

    TiffComponent* TiffMnCreator::create(uint16_t           tag,
                                         uint16_t           group,
                                         const std::string& make,
                                         const byte*        pData,
                                         uint32_t           size,
                                         ByteOrder          byteOrder)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, make);
        if (tmr) {
            assert(tmr->newMnFct_);
            tc = tmr->newMnFct_(tag,
                                group,
                                tmr->mnGroup_,
                                pData,
                                size,
                                byteOrder);
        }
        return tc;
    } // TiffMnCreator::create

    TiffComponent* TiffMnCreator::create(uint16_t           tag,
                                         uint16_t           group,
                                         uint16_t           mnGroup)
    {
        TiffComponent* tc = 0;
        const TiffMnRegistry* tmr = find(registry_, mnGroup);
        if (tmr) {
            assert(tmr->newMnFct2_);
            tc = tmr->newMnFct2_(tag, group, mnGroup);
        }
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

    uint32_t TiffIfdMakernote::sizeHeader() const
    {
        if (!pHeader_) return 0;
        return pHeader_->size();
    }

    uint32_t TiffIfdMakernote::writeHeader(Blob& blob, ByteOrder byteOrder) const
    {
        if (!pHeader_) return 0;
        return pHeader_->write(blob, byteOrder);
    }

    TiffComponent* TiffIfdMakernote::doAddPath(uint16_t tag, TiffPath& tiffPath)
    {
        return ifd_.addPath(tag, tiffPath);
    }

    TiffComponent* TiffIfdMakernote::doAddChild(TiffComponent::AutoPtr tiffComponent)
    {
        return ifd_.addChild(tiffComponent);
    }

    TiffComponent* TiffIfdMakernote::doAddNext(TiffComponent::AutoPtr tiffComponent)
    {
        return ifd_.addNext(tiffComponent);
    }

    void TiffIfdMakernote::doAccept(TiffVisitor& visitor)
    {
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitIfdMakernote(this);
        ifd_.accept(visitor);
        if (visitor.go(TiffVisitor::geTraverse)) visitor.visitIfdMakernoteEnd(this);
    }

    uint32_t TiffIfdMakernote::doWrite(Blob&     blob,
                                       ByteOrder byteOrder,
                                       int32_t   offset,
                                       uint32_t  /*valueIdx*/,
                                       uint32_t  /*dataIdx*/,
                                       uint32_t& imageIdx)
    {
        if (this->byteOrder() != invalidByteOrder) {
            byteOrder = this->byteOrder();
        }
        uint32_t len = writeHeader(blob, byteOrder);
        len += ifd_.write(blob, byteOrder,
                          offset - baseOffset(offset) + len,
                          uint32_t(-1), uint32_t(-1),
                          imageIdx);
        return len;
    } // TiffIfdMakernote::doWrite

    uint32_t TiffIfdMakernote::doWriteData(Blob&     /*blob*/,
                                           ByteOrder /*byteOrder*/,
                                           int32_t   /*offset*/,
                                           uint32_t  /*dataIdx*/,
                                           uint32_t& /*imageIdx*/) const
    {
        assert(false);
        return 0;
    } // TiffIfdMakernote::doWriteData

    uint32_t TiffIfdMakernote::doWriteImage(Blob&     blob,
                                            ByteOrder byteOrder) const
    {
        if (this->byteOrder() != invalidByteOrder) {
            byteOrder = this->byteOrder();
        }
        uint32_t len = ifd_.writeImage(blob, byteOrder);
        return len;
    } // TiffIfdMakernote::doWriteImage

    uint32_t TiffIfdMakernote::doSize() const
    {
        return sizeHeader() + ifd_.size();
    } // TiffIfdMakernote::doSize

    uint32_t TiffIfdMakernote::doCount() const
    {
        return ifd_.count();
    } // TiffIfdMakernote::doCount

    uint32_t TiffIfdMakernote::doSizeData() const
    {
        assert(false);
        return 0;
    } // TiffIfdMakernote::doSizeData

    uint32_t TiffIfdMakernote::doSizeImage() const
    {
        return ifd_.sizeImage();
    } // TiffIfdMakernote::doSizeImage

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

    uint32_t OlympusMnHeader::write(Blob&     blob,
                                    ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // OlympusMnHeader::write

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

    uint32_t FujiMnHeader::write(Blob&     blob,
                                 ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // FujiMnHeader::write

    const byte Nikon2MnHeader::signature_[] = {
        'N', 'i', 'k', 'o', 'n', '\0', 0x01, 0x00
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

    uint32_t Nikon2MnHeader::write(Blob&     blob,
                                   ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // Nikon2MnHeader::write

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

    uint32_t Nikon3MnHeader::write(Blob&     blob,
                                   ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // Nikon3MnHeader::write

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

    uint32_t PanasonicMnHeader::write(Blob&     blob,
                                      ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // PanasonicMnHeader::write

    const byte PentaxMnHeader::signature_[] = {
        'A', 'O', 'C', 0x00, 'M', 'M'
    };
    const uint32_t PentaxMnHeader::size_ = 6;

    PentaxMnHeader::PentaxMnHeader()
    {
        read(signature_, size_, invalidByteOrder);
    }

    bool PentaxMnHeader::read(const byte* pData,
                              uint32_t size,
                              ByteOrder /*byteOrder*/)
    {
        assert (pData != 0);

        if (size < size_) return false;
        header_.alloc(size_);
        std::memcpy(header_.pData_, pData, header_.size_);
        if (   static_cast<uint32_t>(header_.size_) < size_
            || 0 != memcmp(header_.pData_, signature_, 3)) {
            return false;
        }
        return true;
    } // PentaxMnHeader::read

    uint32_t PentaxMnHeader::write(Blob&     blob,
                                   ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // PentaxMnHeader::write

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

    uint32_t SigmaMnHeader::write(Blob&     blob,
                                  ByteOrder /*byteOrder*/) const
    {
        append(blob, signature1_, size_);
        return size_;
    } // SigmaMnHeader::write

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

    uint32_t SonyMnHeader::write(Blob&     blob,
                                 ByteOrder /*byteOrder*/) const
    {
        append(blob, signature_, size_);
        return size_;
    } // SonyMnHeader::write

    // *************************************************************************
    // free functions

    TiffComponent* newIfdMn(uint16_t    tag,
                            uint16_t    group,
                            uint16_t    mnGroup,
                            const byte* /*pData*/,
                            uint32_t    /*size*/,
                            ByteOrder   /*byteOrder*/)
    {
        return newIfdMn2(tag, group, mnGroup);
    }

    TiffComponent* newIfdMn2(uint16_t tag,
                             uint16_t group,
                             uint16_t mnGroup)
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
        return newOlympusMn2(tag, group, mnGroup);
    }

    TiffComponent* newOlympusMn2(uint16_t tag,
                                 uint16_t group,
                                 uint16_t mnGroup)
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
        return newFujiMn2(tag, group, mnGroup);
    }

    TiffComponent* newFujiMn2(uint16_t tag,
                              uint16_t group,
                              uint16_t mnGroup)
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
            return newIfdMn2(tag, group, Group::nikon1mn);
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeade2 tiffHeader;
        if (   size < 18
            || !tiffHeader.read(pData + 10, size - 10)
            || tiffHeader.tag() != 0x002a) {
            return newNikon2Mn2(tag, group, Group::nikon2mn);
        }
        // Else we have a Nikon3 makernote
        return newNikon3Mn2(tag, group, Group::nikon3mn);
    }

    TiffComponent* newNikon2Mn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new Nikon2MnHeader);
    }

    TiffComponent* newNikon3Mn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new Nikon3MnHeader);
    }

    TiffComponent* newPanasonicMn(uint16_t    tag,
                                  uint16_t    group,
                                  uint16_t    mnGroup,
                                  const byte* /*pData*/,
                                  uint32_t    /*size*/,
                                  ByteOrder   /*byteOrder*/)
    {
        return newPanasonicMn2(tag, group, mnGroup);
    }

    TiffComponent* newPanasonicMn2(uint16_t tag,
                                   uint16_t group,
                                   uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PanasonicMnHeader, false);
    }

    TiffComponent* newPentaxMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* /*pData*/,
                                uint32_t    /*size*/,
                                ByteOrder   /*byteOrder*/)
    {
        return newPentaxMn2(tag, group, mnGroup);
    }

    TiffComponent* newPentaxMn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new PentaxMnHeader);
    }

    TiffComponent* newSigmaMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* /*pData*/,
                              uint32_t    /*size*/,
                              ByteOrder   /*byteOrder*/)
    {
        return newSigmaMn2(tag, group, mnGroup);
    }

    TiffComponent* newSigmaMn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup)
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
            return newSony2Mn2(tag, group, Group::sony2mn);
        }
        return newSony1Mn2(tag, group, Group::sony1mn);
    }

    TiffComponent* newSony1Mn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, new SonyMnHeader, false);
    }

    TiffComponent* newSony2Mn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup)
    {
        return new TiffIfdMakernote(tag, group, mnGroup, 0, true);
    }

}}                                      // namespace Internal, Exiv2
