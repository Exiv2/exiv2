// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
  File:      value.cpp
  Version:   $Name:  $ $Revision: 1.7 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.7 $ $RCSfile: value.cpp,v $")

// *****************************************************************************
// included header files
#include "value.hpp"
#include "types.hpp"

// + standard includes
#include <iostream>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Value& Value::operator=(const Value& rhs)
    {
        if (this == &rhs) return *this;
        type_ = rhs.type_;
        return *this;
    }

    Value* Value::create(TypeId typeId)
    {
        Value* value = 0;
        switch (typeId) {
        case invalidTypeId:
            value = new DataValue(invalidTypeId);
            break;
        case unsignedByte:
            value = new DataValue(unsignedByte);
            break;
        case asciiString:
            value =  new AsciiValue;
            break;
        case unsignedShort:
            value = new ValueType<uint16>;
            break;
        case unsignedLong:
            value = new ValueType<uint32>;
            break;
        case unsignedRational:
            value = new ValueType<URational>;
            break;
        case invalid6:
            value = new DataValue(invalid6);
            break;
        case undefined:
            value = new DataValue;
            break;
        case signedShort:
            value = new ValueType<int16>;
            break;
        case signedLong:
            value = new ValueType<int32>;
            break;
        case signedRational:
            value = new ValueType<Rational>;
            break;
        default:
            value = new DataValue(typeId);
            break;
        }
        return value;
    } // Value::create

    std::string Value::toString() const
    {
        std::ostringstream os;
        write(os);
        return os.str();
    }

    DataValue& DataValue::operator=(const DataValue& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        value_ = rhs.value_;
        return *this;
    }

    void DataValue::read(const char* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        value_ = std::string(buf, len);
    }

    void DataValue::read(const std::string& buf)
    {
        std::istringstream is(buf);
        int tmp;
        value_.clear();
        while (is >> tmp) {
            value_ += static_cast<char>(tmp);
        }
    }

    long DataValue::copy(char* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return static_cast<long>(value_.copy(buf, value_.size()));
    }

    long DataValue::size() const
    {
        return static_cast<long>(value_.size());
    }

    DataValue* DataValue::clone() const
    {
        return new DataValue(*this);
    }

    std::ostream& DataValue::write(std::ostream& os) const
    {
        std::string::size_type end = value_.size();
        for (std::string::size_type i = 0; i != end; ++i) {
            os << static_cast<int>(static_cast<unsigned char>(value_[i])) 
               << " ";
        }
        return os;
    }

    AsciiValue& AsciiValue::operator=(const AsciiValue& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        value_ = rhs.value_;
        return *this;
    }

    void AsciiValue::read(const char* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        value_ = std::string(buf, len);
    }

    void AsciiValue::read(const std::string& buf)
    {
        value_ = buf;
        if (value_[value_.size()-1] != '\0') value_ += '\0';
    }

    long AsciiValue::copy(char* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return static_cast<long>(value_.copy(buf, value_.size()));
    }

    long AsciiValue::size() const
    {
        return static_cast<long>(value_.size());
    }

    AsciiValue* AsciiValue::clone() const
    {
        return new AsciiValue(*this);
    }

    std::ostream& AsciiValue::write(std::ostream& os) const
    {
        // Strip all trailing '\0's (if any)
        std::string::size_type pos = value_.find_last_not_of('\0');
        return os << value_.substr(0, pos + 1);
    }

}                                       // namespace Exiv2
