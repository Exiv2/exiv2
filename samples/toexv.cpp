// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  Abstract : Test application to extract metadata from image to exv file
  Version  : $Rev: 3777 $
 */
// *****************************************************************************
// included header files
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <fstream>
#include <cassert>

#include "utils.hpp"
#include "toexv.hpp"

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
	try {
		// Handle command line arguments
		Params params(":iecCahsx");
		if (params.getopt(argc, argv)) return params.usage();
		if (params.help_             ) return params.help();

		Exiv2::Image::AutoPtr readImage = Exiv2::ImageFactory::open(params.read_);
		assert(readImage.get() != 0);
		readImage->readMetadata();

		Exiv2::Image::AutoPtr writeImage = Exiv2::ImageFactory::create(Exiv2::ImageType::exv,params.write_);
		assert(writeImage.get() != 0);

		if (params.all_    ) writeImage->setMetadata  (*readImage);
		if (params.iptc_   ) writeImage->setIptcData  (readImage->iptcData());
		if (params.exif_   ) writeImage->setExifData  (readImage->exifData());
		if (params.ICC_    ) writeImage->setIccProfile(*readImage->iccProfile());
		if (params.comment_) writeImage->setComment   (readImage->comment());
		if (params.xmp_    ) writeImage->setXmpData   (readImage->xmpData());

		writeImage->writeMetadata();

		if ( params.size_ ) std::cout << params.write_ << " " << writeImage->io().size() << std::endl;
		return 0;

#if 0
		// This is prototype code for working on writing metadata to memory
		// This has been discussed with Andrea Ferrora (PhotoFlow)
		// And useful for sending the exv to a webservice.
		uint32_t     size = 54321;
		Exiv2::byte  data[size];
		Exiv2::BasicIo::AutoPtr memIo   (new Exiv2::MemIo(data,size));
		Exiv2::Image::AutoPtr   memImage(new Exiv2::ExvImage(memIo,true));
		memImage->setMetadata(*image);
		std::cout << "wrote " << memImage->writeMetadata() << std::endl;

		for ( size_t i = 0 ; i < 20 ; i++ ) {
			char c = (char) data[i] ;
			if ( c < 32 || c > 127 ) c = '.' ;
			std::cout <<  c;
		}
		std::cout << std::endl;
#endif

	} catch (Exiv2::AnyError& e) {
		std::cerr << "Caught Exiv2 exception '" << e << "'\n";
		return 3;
	}
}

Params::Params( const char* opts)
: optstring_(opts)
, first_(true)
, help_(false)
, iptc_(false)
, exif_(false)
, ICC_(false)
, all_(false)
, comment_(false)
, xmp_(false)
, size_(false)
{}


int Params::option(int opt, const std::string& /*optarg*/, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h': help_     = true ; break;
    case 'i': iptc_     = true ; break;
    case 'e': exif_     = true ; break;
    case 'c': comment_  = true ; break;
    case 'C': ICC_      = true ; break;
    case 'x': xmp_      = true ; break;
    case 'a': all_      = true ; break;
    case 's': size_     = true ; break;
    case 'p': /* ignore for backwards compatibility */ ; break;
    case ':':
        std::cerr << progname() << ": Option -" << static_cast<char>(optopt)
                  << " requires an argument\n";
        rc = 1;
        break;
    case '?':
        std::cerr << progname() << ": Unrecognized option -"
                  << static_cast<char>(optopt) << "\n";
        rc = 1;
        break;
    default:
        std::cerr << progname()
                  << ": getopt returned unexpected character code " << (char) opt
                  << " 0x" << std::hex << opt << "\n";
        rc = 1;
        break;
    }

    return rc;
}

int Params::nonoption(const std::string& argv)
{
    if (!write_.empty()) {
        std::cerr << progname() << ": Unexpected extra argument (" << argv << ")\n";
        return 1;
    }
    if (first_) read_ = argv;
    else        write_ = argv;
    first_ = false;
    return 0;
}

int Params::getopt(int argc, char* const argv[])
{
    int rc = Util::Getopt::getopt(argc, argv, optstring_);
    // Further consistency checks
    if (help_==false) {
        if (rc==0 && read_.empty() ) {
            std::cerr << progname() << ": Read and write files must be specified\n";
            rc = 1;
        }
        if (rc==0 && write_.empty() ) {
            std::cerr << progname() << ": Write file must be specified\n";
            rc = 1;
        }
    }
    if ( argc == 3 ) { all_ = true; size_ = true; }
    return rc;
}

int Params::usage(std::ostream& os) const
{
    os << "\nReads and writes raw metadata. Use -h option for help.\n"
       << "Usage: " << progname()
       << " [-" << optstring_ << "]"
       << " readfile writefile\n";
    return 2;
}

int Params::help(std::ostream& os) const
{
    usage(os);
    os << "\nOptions:\n"
          "   -i      Read Iptc data from readfile and write to writefile.\n"
          "   -e      Read Exif data from readfile and write to writefile.\n"
          "   -c      Read Jpeg comment from readfile and write to writefile.\n"
          "   -C      Read ICC profile from readfile and write to writefile.\n"
          "   -x      Read XMP data from readfile and write to writefile.\n"
          "   -a      Read all metadata from readfile and write to writefile.\n"
          "   -s      Print size of writefile.\n"
          "   -h      Display this help and exit.\n";
    return 1;
}
