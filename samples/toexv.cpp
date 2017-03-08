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

static size_t exifMetadataCount(Exiv2::Image::AutoPtr& image)
{
	size_t result = 0 ;
	Exiv2::ExifData&                  exif = image->exifData();
	Exiv2::ExifData::const_iterator    end = exif.end();
	for (Exiv2::ExifData::const_iterator i = exif.begin(); i != end; ++i) result++;
	return result;
}

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

		if ( params.write_ == "+" ) {
			std::cout << "exifMetadataCount = " << exifMetadataCount(readImage) << std::endl;

			// create an in-memory file and write the metadata
			Exiv2::BasicIo::AutoPtr memIo   (new Exiv2::MemIo());
			Exiv2::Image::AutoPtr   memImage(new Exiv2::ExvImage(memIo,true));
			memImage->setMetadata  (*readImage);
			memImage->writeMetadata();

            // serialize the in-memory file into buff
            size_t       size = memImage->io().size();
			Exiv2::byte  buff[size];
			memImage->io().seek(0,Exiv2::BasicIo::beg);
			memImage->io().read(buff,size);

			std::cout << "size = " << size << std::endl;

			// create an in-memory file with buff and read the metadata into buffImage
			Exiv2::BasicIo::AutoPtr buffIo   (new Exiv2::MemIo(buff,size));
			Exiv2::Image::AutoPtr   buffImage(new Exiv2::ExvImage(buffIo,false));
			assert(buffImage.get() != 0);
			buffImage->readMetadata();

			std::cout << "exifMetadataCount = " << exifMetadataCount(buffImage) << std::endl;

		} else if ( params.write_ != "-" ) {
			// create a file and write the metadata
			Exiv2::Image::AutoPtr writeImage = Exiv2::ImageFactory::create(Exiv2::ImageType::exv,params.write_);
			params.copyMetadata(readImage,writeImage);
		} else {
			// create an in-memory file
			Exiv2::BasicIo::AutoPtr memIo   (new Exiv2::MemIo());
			Exiv2::Image::AutoPtr   memImage(new Exiv2::ExvImage(memIo,true));
			params.copyMetadata(readImage,memImage);

			// read a few bytes from the in-memory file
            size_t       size = memImage->io().size();
            if (size>32) size = 32;
			Exiv2::byte  data[size];

			memImage->io().seek(0,Exiv2::BasicIo::beg);
			memImage->io().read(data,size);

			// dump the bytes
			for ( size_t i = 0 ; i < size ; i++ ) {
				char c = (char) data[i] ;
				if ( !isascii(c) ) c = '.' ;
				std::cout << c ;
			}
			std::cout << std::endl;
		}

		return 0;

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
, usage_(false)
{}

void Params::copyMetadata(Exiv2::Image::AutoPtr& readImage,Exiv2::Image::AutoPtr& writeImage)
{
	if (all_    ) writeImage->setMetadata  (*readImage);
	if (iptc_   ) writeImage->setIptcData  ( readImage->iptcData());
	if (exif_   ) writeImage->setExifData  ( readImage->exifData());
	if (ICC_    ) writeImage->setIccProfile(*readImage->iccProfile());
	if (comment_) writeImage->setComment   ( readImage->comment());
	if (xmp_    ) writeImage->setXmpData   ( readImage->xmpData());

	writeImage->writeMetadata();
	if ( size_ ) std::cout << write_ << " " << writeImage->io().size() << std::endl;
}

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
    if ( argc == 1 ) usage_ = true;
    // Further consistency checks
    if ( !help_ && !usage_ ) {
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
    if ( usage_ ) return 2 ;
    return rc;
}

int Params::usage(std::ostream& os) const
{
    os << "Reads and writes raw metadata. Use -h option for help.\n"
       << "Usage: " << progname()
       << " [-" << optstring_ << "]"
       << " readfile {-|+|writefile}\n";
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
