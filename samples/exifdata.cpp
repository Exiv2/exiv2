// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
// exifdata.cpp
// Sample program to format exif data in various external formats

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>

using format_t = std::map<std::string, int>;
using format_i = format_t::const_iterator;
enum format_e
{
    wolf,
    csv,
    json,
    xml
};

void syntax(const char* argv[],format_t& formats)
{
	std::cout << "Usage: " << argv[0] << " file format" << std::endl;
	int count = 0;
	std::cout << "formats: ";
    for (auto&& format : formats) {
        std::cout << (count++ ? " | " : "") << format.first;
    }
    std::cout << std::endl;
}

size_t formatInit(Exiv2::ExifData& exifData)
{
    return std::distance(exifData.begin(), exifData.end());
}

///////////////////////////////////////////////////////////////////////
std::string escapeCSV(Exiv2::ExifData::const_iterator  it,bool bValue)
{
	std::string   result ;

	std::ostringstream os;
	if ( bValue ) os << it->value() ; else os << it->key() ;

	std::string s = os.str();
    for (auto&& c : s) {
        if (c == ',')
            result += '\\';
        result += c;
    }

    return result;
}

std::string formatCSV(Exiv2::ExifData& exifData)
{
	size_t count  = 0;
	size_t length = formatInit(exifData);
	std::ostringstream result;

	for (auto i = exifData.begin(); count++ < length; ++i) {
		result << escapeCSV(i,false) << (count != length ? ", " : "" ) ;
	}
	result << std::endl;

	count = 0;
	for (auto i = exifData.begin(); count++ < length ; ++i) {
		result << escapeCSV(i,true) << (count != length ? ", " : "" ) ;
	}
	return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string formatWolf(Exiv2::ExifData& exifData)
{
	size_t count  = 0;
	size_t length = formatInit(exifData);
	std::ostringstream result;

	result << "{ " << std::endl;
	for (auto i = exifData.begin(); count++ < length ; ++i) {
		result << "  " << i->key()  << " -> " << i->value()  << (count != length ? "," : "" ) << std::endl ;
	}
	result << "}";
	return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string escapeJSON(Exiv2::ExifData::const_iterator  it,bool bValue=true)
{
	std::string   result ;

	std::ostringstream os;
	if ( bValue ) os << it->value() ; else os << it->key() ;

	std::string s = os.str();
    for (auto&& c : s) {
        if (c == '"')
            result += "\\\"";
        result += c;
    }

    std::string q = "\"";
    return q + result + q;
}

std::string formatJSON(Exiv2::ExifData& exifData)
{
	size_t count  = 0;
	size_t length = formatInit(exifData);
	std::ostringstream result;

	result << "{" << std::endl ;
	for (auto i = exifData.begin(); count++ < length ; ++i) {
		result << "  " << escapeJSON(i,false)  << ":" << escapeJSON(i,true) << ( count != length ? "," : "" ) << std::endl ;
	}
	result << "}";
	return result.str();
}

///////////////////////////////////////////////////////////////////////
std::string escapeXML(Exiv2::ExifData::const_iterator  it,bool bValue=true)
{
	std::string   result ;

	std::ostringstream os;
	if ( bValue ) os << it->value() ; else os << it->key() ;

	std::string s = os.str();
    for (auto&& c : s) {
        if (c == '<')
            result += "&lg;";
        if (c == '>')
            result += "&gt;";
        result += c;
    }

    return result;
}

std::string formatXML(Exiv2::ExifData& exifData)
{
	size_t count  = 0;
	size_t length = formatInit(exifData);
	std::ostringstream result;

	result << "<exif>" << std::endl;
	for (auto i = exifData.begin(); count++ < length ; ++i) {
		std::string key   = escapeXML(i,false);
		std::string value = escapeXML(i,true);
		result << "  <" << key << ">" << value << "<" << key << "/>" << std::endl ;
	}
	result << "</exif>" << std::endl;
	return result.str();
}

///////////////////////////////////////////////////////////////////////
int main(int argc,const char* argv[])
{
	Exiv2::XmpParser::initialize();
	::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

	format_t formats;
	formats["wolf"] = wolf;
	formats["csv" ] = csv ;
	formats["json"] = json;
	formats["xml" ] = xml ;

	int result = 0 ;
    if (argc != 3) {
    	syntax(argv,formats) ;
        result = 1;
    }

    const char* file   = argv[1];
    const char* format = argv[2];

	if ( !result && formats.find(format) == formats.end() ) {
        std::cout << "Unrecognised format " << format << std::endl;
        syntax(argv,formats);
        result = 2;
	}

	if ( !result ) try {
		Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(file);
		assert(image.get() != 0);
		image->readMetadata();
		Exiv2::ExifData &exifData = image->exifData();

		switch ( formats.find(format)->second ) {
			case wolf : std::cout << formatWolf(exifData) << std::endl; break;
			case csv  : std::cout << formatCSV (exifData) << std::endl; break;
			case json : std::cout << formatJSON(exifData) << std::endl; break;
			case xml  : std::cout << formatXML (exifData) << std::endl; break;

			default   : std::cout << "*** error: format not implemented yet: " << format << " ***" << std::endl;
			            result = 3;
			break;
		}
	} catch (Exiv2::AnyError& e) {
    	std::cerr << "*** error exiv2 exception '" << e << "' ***" << std::endl;
    	result = 4;
	} catch ( ... ) {
    	std::cerr << "*** error exception" << std::endl;
    	result = 5;
	}

    return result;
}
