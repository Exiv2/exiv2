// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      types.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cctype>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    TypeInfoTable::TypeInfoTable(TypeId typeId, const char* name, long size)
        : typeId_(typeId), name_(name), size_(size)
    {
    }

    //! Lookup list of supported IFD type information
    const TypeInfoTable TypeInfo::typeInfoTable_[] = {
        TypeInfoTable(invalidTypeId,    "Invalid",     0),
        TypeInfoTable(unsignedByte,     "Byte",        1),
        TypeInfoTable(asciiString,      "Ascii",       1),
        TypeInfoTable(unsignedShort,    "Short",       2),
        TypeInfoTable(unsignedLong,     "Long",        4),
        TypeInfoTable(unsignedRational, "Rational",    8),
        TypeInfoTable(invalid6,         "Invalid(6)",  1),
        TypeInfoTable(undefined,        "Undefined",   1),
        TypeInfoTable(signedShort,      "SShort",      2),
        TypeInfoTable(signedLong,       "SLong",       4),
        TypeInfoTable(signedRational,   "SRational",   8),
        TypeInfoTable(string,           "String",      1),
        TypeInfoTable(date,             "Date",        8),
        TypeInfoTable(time,             "Time",        11),
        TypeInfoTable(comment,          "Comment",     1),
        // End of list marker
        TypeInfoTable(lastTypeId,       "(Unknown)",   0)
    };

    const char* TypeInfo::typeName(TypeId typeId)
    {
        return typeInfoTable_[ typeId < lastTypeId ? typeId : 0 ].name_;
    }

    TypeId TypeInfo::typeId(const std::string& typeName)
    {
        int i = 0;
        for (;    typeInfoTable_[i].typeId_ != lastTypeId
               && typeInfoTable_[i].name_ != typeName; ++i) {}
        return typeInfoTable_[i].typeId_ == lastTypeId ?
               invalidTypeId : typeInfoTable_[i].typeId_;
    }

    long TypeInfo::typeSize(TypeId typeId)
    {
        return typeInfoTable_[ typeId < lastTypeId ? typeId : 0 ].size_;
    }
    
    DataBuf::DataBuf(DataBuf& rhs)
        : pData_(rhs.pData_), size_(rhs.size_)
    {
        rhs.release();
    }

    DataBuf::DataBuf(byte* pData, long size) 
        : pData_(0), size_(0)
    {
        if (size > 0) {
            pData_ = new byte[size];
            memcpy(pData_, pData, size);
            size_ = size;
        }
    }

    DataBuf& DataBuf::operator=(DataBuf& rhs)
    {
        if (this == &rhs) return *this;
        reset(rhs.release());
        return *this;
    }

    void DataBuf::alloc(long size)
    { 
        if (size > size_) {
            delete[] pData_; 
            size_ = size; 
            pData_ = new byte[size];
        } 
    }

    std::pair<byte*, long> DataBuf::release()
    {
        std::pair<byte*, long> p = std::make_pair(pData_, size_);
        pData_ = 0;
        size_ = 0;
        return p;
    }

    void DataBuf::reset(std::pair<byte*, long> p)
    {
        if (pData_ != p.first) {
            delete[] pData_;
            pData_ = p.first;
        }
        size_ = p.second;
    }

    // *************************************************************************
    // free functions

    uint16_t getUShort(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (byte)buf[1] << 8 | (byte)buf[0];
        }
        else {
            return (byte)buf[0] << 8 | (byte)buf[1];
        }
    }

    uint32_t getULong(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (byte)buf[3] << 24 | (byte)buf[2] << 16 
                   | (byte)buf[1] <<  8 | (byte)buf[0];
        }
        else {
            return   (byte)buf[0] << 24 | (byte)buf[1] << 16 
                   | (byte)buf[2] <<  8 | (byte)buf[3];
        }
    }

    URational getURational(const byte* buf, ByteOrder byteOrder)
    {
        uint32_t nominator = getULong(buf, byteOrder);
        uint32_t denominator = getULong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    int16_t getShort(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return (byte)buf[1] << 8 | (byte)buf[0];
        }
        else {
            return (byte)buf[0] << 8 | (byte)buf[1];
        }
    }

    int32_t getLong(const byte* buf, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            return   (byte)buf[3] << 24 | (byte)buf[2] << 16 
                   | (byte)buf[1] <<  8 | (byte)buf[0];
        }
        else {
            return   (byte)buf[0] << 24 | (byte)buf[1] << 16 
                   | (byte)buf[2] <<  8 | (byte)buf[3];
        }
    }

    Rational getRational(const byte* buf, ByteOrder byteOrder)
    {
        int32_t nominator = getLong(buf, byteOrder);
        int32_t denominator = getLong(buf + 4, byteOrder);
        return std::make_pair(nominator, denominator);
    }

    long us2Data(byte* buf, uint16_t s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(s & 0x00ff);
            buf[1] = (byte)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (byte)((s & 0xff00) >> 8);
            buf[1] =  (byte)(s & 0x00ff);
        }
        return 2;
    }

    long ul2Data(byte* buf, uint32_t l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(l & 0x000000ff);
            buf[1] = (byte)((l & 0x0000ff00) >> 8);
            buf[2] = (byte)((l & 0x00ff0000) >> 16);
            buf[3] = (byte)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (byte)((l & 0xff000000) >> 24);
            buf[1] = (byte)((l & 0x00ff0000) >> 16);
            buf[2] = (byte)((l & 0x0000ff00) >> 8);
            buf[3] =  (byte)(l & 0x000000ff);
        }
        return 4;
    }

    long ur2Data(byte* buf, URational l, ByteOrder byteOrder)
    {
        long o = ul2Data(buf, l.first, byteOrder);
        o += ul2Data(buf+o, l.second, byteOrder);
        return o;
    }

    long s2Data(byte* buf, int16_t s, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(s & 0x00ff);
            buf[1] = (byte)((s & 0xff00) >> 8);
        }
        else {
            buf[0] = (byte)((s & 0xff00) >> 8);
            buf[1] =  (byte)(s & 0x00ff);
        }
        return 2;
    }

    long l2Data(byte* buf, int32_t l, ByteOrder byteOrder)
    {
        if (byteOrder == littleEndian) {
            buf[0] =  (byte)(l & 0x000000ff);
            buf[1] = (byte)((l & 0x0000ff00) >> 8);
            buf[2] = (byte)((l & 0x00ff0000) >> 16);
            buf[3] = (byte)((l & 0xff000000) >> 24);
        }
        else {
            buf[0] = (byte)((l & 0xff000000) >> 24);
            buf[1] = (byte)((l & 0x00ff0000) >> 16);
            buf[2] = (byte)((l & 0x0000ff00) >> 8);
            buf[3] =  (byte)(l & 0x000000ff);
        }
        return 4;
    }

    long r2Data(byte* buf, Rational l, ByteOrder byteOrder)
    {
        long o = l2Data(buf, l.first, byteOrder);
        o += l2Data(buf+o, l.second, byteOrder);
        return o;
    }

    void hexdump(std::ostream& os, const byte* buf, long len, long offset)
    {
        const std::string::size_type pos = 8 + 16 * 3 + 2; 
        const std::string align(pos, ' '); 

        long i = 0;
        while (i < len) {
            os << "  " 
               << std::setw(4) << std::setfill('0') << std::hex 
               << i + offset << "  ";
            std::ostringstream ss;
            do {
                byte c = buf[i];
                os << std::setw(2) << std::setfill('0') 
                   << std::hex << (int)c << " ";
                ss << ((int)c >= 31 && (int)c < 127 ? char(buf[i]) : '.');
            } while (++i < len && i%16 != 0);
            std::string::size_type width = 9 + ((i-1)%16 + 1) * 3;
            os << (width > pos ? "" : align.substr(width)) << ss.str() << "\n";
        }
        os << std::dec << std::setfill(' ');
    } // hexdump

    int gcd(int a, int b)
    {
        int temp;
        if (a < b) {
            temp = a;
            a = b; 
            b = temp; 
        }
        while ((temp = a % b) != 0) {
            a = b;
            b = temp;
        }
        return b;
    } // gcd

    long lgcd(long a, long b)
    {
        long temp;
        if (a < b) {
            temp = a;
            a = b; 
            b = temp; 
        }
        while ((temp = a % b) != 0) {
            a = b;
            b = temp;
        }
        return b;
    } // lgcd

    bool isHex(const std::string& str, size_t size, const std::string& prefix)
    {
        if (   str.size() <= prefix.size() 
            || str.substr(0, prefix.size()) != prefix) return false;
        if (   size > 0
            && str.size() != size + prefix.size()) return false;
        
        for (size_t i = prefix.size(); i < str.size(); ++i) {
            if (!isxdigit(str[i])) return false;
        }
        return true;
    } // isHex

}                                       // namespace Exiv2
