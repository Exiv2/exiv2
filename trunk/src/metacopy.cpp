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
  Abstract : Tester application for image file handling

  File     : metacopy.cpp
  Version  : $Name:  $ $Revision: 1.2 $
  Author(s): Brad Schick (brad) <schick@robotbattle.com>
  History  : 13-Jul-04, brad: created
 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include "types.hpp"
#include "metacopy.hpp"
#include <iostream>
#include <fstream>

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {
    // Handle command line arguments
    Params params;
    if (params.getopt(argc, argv)) {
        params.usage();
        return 1;
    }
    if (params.help_) {
        params.help();
        return 2;
    }

    Exiv2::Image::AutoPtr readImg 
        = Exiv2::ImageFactory::instance().open(params.read_);
    if (readImg.get() == 0) {
        std::cerr << params.progname() << 
            ": Could not read file (" << params.read_ << ")\n";
        return 4;
    }
    if (readImg->readMetadata()) {
        std::cerr << params.progname() << 
            ": Could not read metadata from (" << params.read_ << ")\n";
        return 5;
    }

    Exiv2::Image::AutoPtr writeImg 
        = Exiv2::ImageFactory::instance().open(params.write_);
    if (writeImg.get() == 0) {
        std::cerr << params.progname() << 
            ": Could not read file (" << params.write_ << ")\n";
        return 6;
    }

    if (params.preserve_) {
        if (writeImg->readMetadata()) {
            std::cerr << params.progname() << 
                ": Could not read metadata from (" << params.write_ << ")\n";
            return 7;
        }
    }
    if (params.iptc_) {
        writeImg->setIptcData(readImg->iptcData(), readImg->sizeIptcData());
    }
    if (params.exif_) {
        writeImg->setExifData(readImg->exifData(), readImg->sizeExifData());
    }
    if (params.comment_) {
        writeImg->setComment(readImg->comment());
    }

    if (writeImg->writeMetadata()) {
        std::cerr << params.progname() << 
            ": Could not write metadata to (" << params.write_ << ")\n";
        return 8;
    }

    return 0;
}
catch (Exiv2::Error& e) {
    std::cerr << "Caught Exiv2 exception '" << e << "'\n";
    return 10;
}
}

int Params::option(int opt, const std::string& optarg, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h': help_ = true; break;
    case 'i': iptc_ = true; break;
    case 'e': exif_ = true; break;
    case 'c': comment_ = true; break;
    case 'p': preserve_ = true; break;
    case 'a': 
        iptc_ =true; 
        exif_ =true; 
        comment_ =true; 
        break;
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
                  << ": getopt returned unexpected character code " 
                  << std::hex << opt << "\n";
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
    else write_ = argv;
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
        if (preserve_ && iptc_ && exif_ && comment_ ) {
            std::cerr << progname() << ": Option -p has no effect when all metadata types are specified.\n";
            rc = 1;
        }
    }
    return rc;
} // Params::getopt


void Params::usage(std::ostream& os) const
{
    os << "\nReads and writes raw metadata. Use -h option for help.\n"
       << "Usage: " << progname() 
       << " [-iecaph] readfile writefile\n";
}

void Params::help(std::ostream& os) const
{
    usage(os);
    os << "\nOptions:\n"
       << "   -i      Read Iptc data from readfile and write to writefile.\n"
       << "   -e      Read Exif data from readfile and write to writefile.\n"
       << "   -c      Read Jpeg comment from readfile and write to writefile.\n"
       << "   -a      Read all metadata from readfile and write to writefile.\n"
       << "   -p      Preserve existing metadata in writefile if not replaced.\n"
       << "   -h      Display this help and exit.\n\n";
} // Params::help

