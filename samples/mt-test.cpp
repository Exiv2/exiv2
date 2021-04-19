// ***************************************************************** -*- C++ -*-
// mt-test.cpp
// Sample multi-threading program
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

// Discussion:          http://dev.exiv2.org/issues/1207
// Caution:             This code isn't currently exercised by any bash script in the test suite
// This code is here for use in development when multi-threading issues
// are being discussed.  For example #1207 and #1187
// It may be brought into use when Exiv2 support C++11 (#1188)
//

// WARNING:             Only builds with clang and gcc < 4.9.  I've never tried to build with Visual Studio
// requires C++11
// On Mac               #define  __cplusplus 201103L
// Older compilers      #define __cplusplus  199711
// Compiler switches:   -std=c++11 (set in samples/Makefile)

// WARNING:             auto_ptr is not supported in C++11 implemented by gcc 4.9/C++11 and later

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>
#include <thread>
#include <mutex>

// mutex to for exclusive reporting
std::mutex m;

void reportExifMetadataCount(int n,const char* argv[])
{
	int count = 0 ;
	std::string what;

	// count the exif metadata in the file
	try {
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[n]);
		assert(image.get() != 0);
		image->readMetadata();

		Exiv2::ExifData &exifData = image->exifData();
		if (!exifData.empty()) {
			Exiv2::ExifData::const_iterator end = exifData.end();
			for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i)
				count++;
		}
	} catch (Exiv2::Error& e) {
		what = e.what() ;
		count = -1;
	}

	// report to the user
	m.lock();
	std::cout << "file: "  << argv[n] << " "
	          << "n: "     << n       << " "
	          << "count: " << count   << " "
	          << (count < 0 ? "exception: " : what)
	          << what                 << std::endl;
	m.unlock();
}

int main(int argc,const char* argv[])
{
	Exiv2::XmpParser::initialize();
	::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

	int result = 0;

	if ( argc < 2 ) {
		std::cerr << "syntax: " << argv[0] << " [path]+" << std::endl;
		result = 1 ;
	} else {
		// Initialize XmpParser before starting threads
		Exiv2::XmpParser::initialize();

		// bucket of threads
		std::thread* threads = new std::thread[argc+1];

		// spin up the treads
		for ( int arg = 1 ; arg < argc ; arg++ ) {
			threads[arg] = std::thread(reportExifMetadataCount,arg,argv);
		}

		// wait for them to finish
		for ( int arg = 1 ; arg < argc ; arg++ ) {
			if ( threads[arg].joinable() )
				threads[arg].join();
		}
		delete [] threads;
	}

	return result;
}

// That's all Folks!
////
