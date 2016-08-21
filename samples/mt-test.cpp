// ***************************************************************** -*- C++ -*-
// mt-test.cpp $Rev: 3090 $
// Sample multi-threading program

// requires C++11
// On Mac               #define  __cplusplus 201103L
// Older compilers      #define __cplusplus  199711
// Compiler switches:   -std=c++11 -stdlib=libc++ (set in samples/Makefile)
// Discussion:          http://dev.exiv2.org/issues/1207
// Caution:             This code isn't currently exercised any bash script in the test suite

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
	int result = 0;

	if ( argc < 2 ) {
		std::cerr << "syntax: " << argv[0] << " [path]+" << std::endl;
		result = 1 ;
	} else {
		// Initialize XmpParser before starting threads
		Exiv2::XmpParser::initialize();

		// bucket of threads
		const int           TMAX=1000;
		std::thread threads[TMAX];
		int           argm = argc > TMAX ? TMAX : argc;

		// spin up the treads
		for ( int arg = 1 ; arg < argm ; arg++ ) {
			threads[arg] = std::thread(reportExifMetadataCount,arg,argv);
		}

		// wait for them to finish
		for ( int arg = 1 ; arg < argm ; arg++ ) {
			if ( threads[arg].joinable() )
				threads[arg].join();
		}
	}

	return result;
}

// That's all Folks!
////
