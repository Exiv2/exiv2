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
  File:      value.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
             31-Jul-04, brad: added Time, Date and String values
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "value.hpp"
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <ctime>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Value& Value::operator=(const Value& rhs)
    {
        if (this == &rhs) return *this;
        type_ = rhs.type_;
        return *this;
    }

    Value::AutoPtr Value::create(TypeId typeId)
    {
        AutoPtr value;
        switch (typeId) {
        case invalidTypeId:
            value = AutoPtr(new DataValue(invalidTypeId));
            break;
        case unsignedByte:
            value = AutoPtr(new DataValue(unsignedByte));
            break;
        case asciiString:
            value = AutoPtr(new AsciiValue);
            break;
        case unsignedShort:
            value = AutoPtr(new ValueType<uint16_t>);
            break;
        case unsignedLong:
            value = AutoPtr(new ValueType<uint32_t>);
            break;
        case unsignedRational:
            value = AutoPtr(new ValueType<URational>);
            break;
        case invalid6:
            value = AutoPtr(new DataValue(invalid6));
            break;
        case undefined:
            value = AutoPtr(new DataValue);
            break;
        case signedShort:
            value = AutoPtr(new ValueType<int16_t>);
            break;
        case signedLong:
            value = AutoPtr(new ValueType<int32_t>);
            break;
        case signedRational:
            value = AutoPtr(new ValueType<Rational>);
            break;
        case string:
            value = AutoPtr(new StringValue);
            break;
        case date:
            value = AutoPtr(new DateValue);
            break;
        case time:
            value = AutoPtr(new TimeValue);
            break;
        case comment:
            value = AutoPtr(new CommentValue);
            break;
        default:
            value = AutoPtr(new DataValue(typeId));
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

    void DataValue::read(const byte* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed
        value_.assign(buf, buf + len);
    }

    void DataValue::read(const std::string& buf)
    {
        std::istringstream is(buf);
        int tmp;
        value_.clear();
        while (is >> tmp) {
            value_.push_back(static_cast<byte>(tmp));
        }
    }

    long DataValue::copy(byte* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return static_cast<long>(
            std::copy(value_.begin(), value_.end(), buf) - buf
            );
    }

    long DataValue::size() const
    {
        return static_cast<long>(value_.size());
    }

    DataValue* DataValue::clone_() const
    {
        return new DataValue(*this);
    }

    std::ostream& DataValue::write(std::ostream& os) const
    {
        std::vector<byte>::size_type end = value_.size();
        for (std::vector<byte>::size_type i = 0; i != end; ++i) {
            os << static_cast<int>(value_[i]) << " ";
        }
        return os;
    }

    StringValueBase& StringValueBase::operator=(const StringValueBase& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        value_ = rhs.value_;
        return *this;
    }

    void StringValueBase::read(const std::string& buf)
    {
        value_ = buf;
    }

    void StringValueBase::read(const byte* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        value_ = std::string(reinterpret_cast<const char*>(buf), len);
    }

    long StringValueBase::copy(byte* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        return static_cast<long>(
            value_.copy(reinterpret_cast<char*>(buf), value_.size())
            );
    }

    long StringValueBase::size() const
    {
        return static_cast<long>(value_.size());
    }

    std::ostream& StringValueBase::write(std::ostream& os) const
    {
        return os << value_;
    }

    StringValue& StringValue::operator=(const StringValue& rhs)
    {
        if (this == &rhs) return *this;
        StringValueBase::operator=(rhs);
        return *this;
    }

    StringValue* StringValue::clone_() const
    {
        return new StringValue(*this);
    }

    AsciiValue& AsciiValue::operator=(const AsciiValue& rhs)
    {
        if (this == &rhs) return *this;
        StringValueBase::operator=(rhs);
        return *this;
    }

    void AsciiValue::read(const std::string& buf)
    {
        value_ = buf;
        if (value_[value_.size()-1] != '\0') value_ += '\0';
    }

    AsciiValue* AsciiValue::clone_() const
    {
        return new AsciiValue(*this);
    }

    std::ostream& AsciiValue::write(std::ostream& os) const
    {
        // Strip all trailing '\0's (if any)
        std::string::size_type pos = value_.find_last_not_of('\0');
        return os << value_.substr(0, pos + 1);
    }

    CommentValue::CharsetTable::CharsetTable(CharsetId charsetId,
                                             const char* name, 
                                             const char* code)
        : charsetId_(charsetId), name_(name), code_(code)
    {
    }

    //! Lookup list of supported IFD type information
    const CommentValue::CharsetTable CommentValue::CharsetInfo::charsetTable_[] = {
        CharsetTable(ascii,            "Ascii",            "ASCII\0\0\0"),
        CharsetTable(jis,              "Jis",              "JIS\0\0\0\0\0"),
        CharsetTable(unicode,          "Unicode",          "UNICODE\0"),
        CharsetTable(undefined,        "Undefined",        "\0\0\0\0\0\0\0\0"),
        CharsetTable(invalidCharsetId, "InvalidCharsetId", "\0\0\0\0\0\0\0\0"),
        CharsetTable(lastCharsetId,    "InvalidCharsetId", "\0\0\0\0\0\0\0\0")
    };

    const char* CommentValue::CharsetInfo::name(CharsetId charsetId)
    {
        return charsetTable_[ charsetId < lastCharsetId ? charsetId : undefined ].name_;
    }

    const char* CommentValue::CharsetInfo::code(CharsetId charsetId)
    {
        return charsetTable_[ charsetId < lastCharsetId ? charsetId : undefined ].code_;
    }

    CommentValue::CharsetId CommentValue::CharsetInfo::charsetIdByName(
        const std::string& name)
    {
        int i = 0;
        for (;    charsetTable_[i].charsetId_ != lastCharsetId
               && charsetTable_[i].name_ != name; ++i) {}
        return charsetTable_[i].charsetId_ == lastCharsetId ?
               invalidCharsetId : charsetTable_[i].charsetId_;
    }

    CommentValue::CharsetId CommentValue::CharsetInfo::charsetIdByCode(
        const std::string& code)
    {
        int i = 0;
        for (;    charsetTable_[i].charsetId_ != lastCharsetId
               && std::string(charsetTable_[i].code_, 8) != code; ++i) {}
        return charsetTable_[i].charsetId_ == lastCharsetId ?
               invalidCharsetId : charsetTable_[i].charsetId_;
    }

    CommentValue::CommentValue(const std::string& comment)
        : StringValueBase(Exiv2::undefined)
    {
        read(comment);
    }

    CommentValue& CommentValue::operator=(const CommentValue& rhs)
    {
        if (this == &rhs) return *this;
        StringValueBase::operator=(rhs);
        return *this;
    }

    void CommentValue::read(const std::string& comment)
    {
        std::string c = comment;
        CharsetId charsetId = undefined;
        if (comment.length() > 8 && comment.substr(0, 8) == "charset=") {
            std::string::size_type pos = comment.find_first_of(' ');
            std::string name = comment.substr(8, pos-8);
            // Strip quotes (so you can also to specify the charset without quotes)
            if (name[0] == '"') name = name.substr(1);
            if (name[name.length()-1] == '"') name = name.substr(0, name.length()-1);
            charsetId = CharsetInfo::charsetIdByName(name);
            if (charsetId == invalidCharsetId) throw Error("Invalid charset");
            c.clear();
            if (pos != std::string::npos) c = comment.substr(pos+1);
        }
        const std::string code(CharsetInfo::code(charsetId), 8);
        StringValueBase::read(code + c);
    }

    std::ostream& CommentValue::write(std::ostream& os) const
    {
        CharsetId charsetId = this->charsetId();
        if (charsetId != undefined) {
            os << "charset=\"" << CharsetInfo::name(charsetId) << "\" ";
        }
        return os << comment();
    }

    std::string CommentValue::comment() const
    {
        if (value_.length() >= 8) return value_.substr(8);
        return "";
    }

    CommentValue::CharsetId CommentValue::charsetId() const
    {
        CharsetId charsetId = undefined;
        if (value_.length() >= 8) {
            const std::string code = value_.substr(0, 8);
            charsetId = CharsetInfo::charsetIdByCode(code);
        }
        return charsetId;        
    }

    CommentValue* CommentValue::clone_() const
    {
        return new CommentValue(*this);        
    }

    DateValue::DateValue(int year, int month, int day) 
        : Value(date)
    {
        date_.year = year;
        date_.month = month;
        date_.day = day;
    }

    DateValue& DateValue::operator=(const DateValue& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        date_.year = rhs.date_.year;
        date_.month = rhs.date_.month;
        date_.day = rhs.date_.day;
        return *this;
    }

    void DateValue::read(const byte* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        // Hard coded to read Iptc style dates
        if (len != 8) throw Error("Unsupported date format");
        int scanned = sscanf(reinterpret_cast<const char*>(buf), 
                   "%4d%2d%2d", 
                   &date_.year, &date_.month, &date_.day);
        if (scanned != 3) throw Error("Unsupported date format");
    }

    void DateValue::read(const std::string& buf)
    {
        // byteOrder not needed 
        // Hard coded to read Iptc style dates
        if (buf.length() < 8) throw Error("Unsupported date format");
        int scanned = sscanf(buf.data(), 
                   "%4d-%d-%d", 
                   &date_.year, &date_.month, &date_.day);
        if (scanned != 3) throw Error("Unsupported date format");
    }

    void DateValue::setDate( const Date& src )
    {
        date_.year = src.year;
        date_.month = src.month;
        date_.day = src.day;
    }
    
    long DateValue::copy(byte* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        // sprintf wants to add the null terminator, so use oversized buffer
        char temp[9];

        int wrote = sprintf( temp, "%04d%02d%02d", 
                           date_.year, date_.month, date_.day);
        assert(wrote == 8);
        memcpy(buf, temp, 8);
        return 8;
    }

    long DateValue::size() const
    {
        return 8;
    }

    DateValue* DateValue::clone_() const
    {
        return new DateValue(*this);
    }

    std::ostream& DateValue::write(std::ostream& os) const
    {
        return os << date_.year << '-' << std::right
               << std::setw(2) << std::setfill('0') << date_.month << '-'
               << std::setw(2) << std::setfill('0') << date_.day;
    }

    long DateValue::toLong(long n) const 
    {
        // Range of tm struct is limited to about 1970 to 2038
        // This will return -1 if outside that range
        std::tm tms;
        memset(&tms, 0, sizeof(tms));
        tms.tm_mday = date_.day;
        tms.tm_mon = date_.month - 1;
        tms.tm_year = date_.year - 1900;
        return static_cast<long>(std::mktime(&tms));
    }

    TimeValue::TimeValue(int hour, int minute, 
                         int second, int tzHour, 
                         int tzMinute)
        : Value(date)
    {
        time_.hour=hour;
        time_.minute=minute;
        time_.second=second;
        time_.tzHour=tzHour;
        time_.tzMinute=tzMinute;
    }

    TimeValue& TimeValue::operator=(const TimeValue& rhs)
    {
        if (this == &rhs) return *this;
        Value::operator=(rhs);
        memcpy(&time_, &rhs.time_, sizeof(time_));
        return *this;
    }

    void TimeValue::read(const byte* buf, long len, ByteOrder byteOrder)
    {
        // byteOrder not needed 
        // Hard coded to read Iptc style times
        if (len != 11) throw Error("Unsupported time format");
        char plusMinus;
        int scanned = sscanf(reinterpret_cast<const char*>(buf), 
                   "%2d%2d%2d%1c%2d%2d", 
                   &time_.hour, &time_.minute, &time_.second, 
                   &plusMinus, &time_.tzHour, &time_.tzMinute );

        if (scanned != 6) throw Error("Unsupported time format");
        if (plusMinus == '-') {
            time_.tzHour *= -1;
            time_.tzMinute *= -1;
        }
    }

    void TimeValue::read(const std::string& buf)
    {
        // byteOrder not needed 
        // Hard coded to read Iptc style times
        if (buf.length() < 9) throw Error("Unsupported time format");
        char plusMinus;
        int scanned = sscanf(buf.data(),
                   "%d:%d:%d%1c%d:%d", 
                   &time_.hour, &time_.minute, &time_.second, 
                   &plusMinus, &time_.tzHour, &time_.tzMinute );

        if (scanned != 6) throw Error("Unsupported time format");
        if (plusMinus == '-') {
            time_.tzHour *= -1;
            time_.tzMinute *= -1;
        }
    }

    void TimeValue::setTime( const Time& src )
    {
        memcpy(&time_, &src, sizeof(time_));
    }
    
    long TimeValue::copy(byte* buf, ByteOrder byteOrder) const
    {
        // byteOrder not needed
        // sprintf wants to add the null terminator, so use oversized buffer
        char temp[12];
        char plusMinus = '+';
        if (time_.tzHour < 0 || time_.tzMinute < 0) plusMinus = '-';

        int wrote = sprintf(temp, 
                   "%02d%02d%02d%1c%02d%02d", 
                   time_.hour, time_.minute, time_.second, 
                   plusMinus, abs(time_.tzHour), abs(time_.tzMinute));

        assert(wrote == 11);
        memcpy(buf, temp, 11);
        return 11;
    }

    long TimeValue::size() const
    {
        return 11;
    }

    TimeValue* TimeValue::clone_() const
    {
        return new TimeValue(*this);
    }

    std::ostream& TimeValue::write(std::ostream& os) const
    {
        char plusMinus = '+';
        if (time_.tzHour < 0 || time_.tzMinute < 0) plusMinus = '-';
        
        return os << std::right
           << std::setw(2) << std::setfill('0') << time_.hour << ':'
           << std::setw(2) << std::setfill('0') << time_.minute << ':'
           << std::setw(2) << std::setfill('0') << time_.second << plusMinus
           << std::setw(2) << std::setfill('0') << abs(time_.tzHour) << ':'
           << std::setw(2) << std::setfill('0') << abs(time_.tzMinute);
    }

    long TimeValue::toLong(long n) const 
    {
        // Returns number of seconds in the day in UTC. 
        long result = (time_.hour - time_.tzHour) * 60 * 60;
        result += (time_.minute - time_.tzMinute) * 60;
        result += time_.second;
        if (result < 0) {
            result += 86400;
        }
        return result;
    }

}                                       // namespace Exiv2
