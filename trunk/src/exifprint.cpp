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
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:
   26-Jan-04, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.1 $
 */
// *****************************************************************************
// included header files
#include "exifprint.hpp"
#include "tags.hpp"
#include "exif.hpp"
#include "utils.hpp"

// + standard includes
#include <iostream>
#include <iomanip>

#include <cstring>

using namespace Exif;

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
    // Handle command line arguments
    Params& params = Params::instance();
    if (params.getopt(argc, argv)) {
        params.usage();
        return 1;
    }
    if (params.help_) {
        params.help();
        return 0;
    }
    if (params.version_) {
        params.version();
        return 0;
    }

    ExifData exifData;
    int rc = exifData.read(argv[1]);

    if (rc == 0) {
        ExifData::const_iterator beg = exifData.begin();
        ExifData::const_iterator end = exifData.end();
        ExifData::const_iterator i = beg;
        for (; i != end; ++i) {
            std::cout << "0x" 
                      << std::hex << std::setw(4) << std::setfill('0') << std::right
                      << i->tag() << " " 
                      << std::setw(27) << std::setfill(' ') << std::left
//                      << i->tagName() << " "
                      << i->key() << " "
                      << std::setw(17) << std::setfill(' ') << std::left
                      << i->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << i->count() << "   " 
                      << std::dec << i->value() << "\n";
        }
    }

    return rc;
}

// *****************************************************************************
// class Params
Params* Params::instance_ = 0;

Params& Params::instance()
{
    if (0 == instance_) {
        instance_ = new Params;
    }
    return *instance_;
}

void Params::version(std::ostream& os) const
{
    os << "Exifprint 0.1\n\n" 
       << "Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>\n"; 
}

void Params::usage(std::ostream& os) const
{
    os << "Usage: " << progname() 
       << " [ -hv ] file ...\n\n"
       << "Print the Exif metadata of the image files.\n";
}

void Params::help(std::ostream& os) const
{
    usage(os);
    os << "\nOptions:\n"
       << "   -h      Display this help and exit.\n"
       << "   -v      Show the program version and exit.\n\n";
}

int Params::option(int opt, const std::string& optarg, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h':
        help_ = true;
        break;
    case 'v':
        version_ = true;
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
    }
    return rc;
}

int Params::nonoption(const std::string& argv)
{
    files_.push_back(argv);
    return 0;
}

int Params::getopt(int argc, char* const argv[])
{
    int rc = Util::Getopt::getopt(argc, argv, optstring_);
    // Further consistency checks
    if (help_ || version_) return 0;
    if (0 == files_.size()) {
        std::cerr << progname() << ": At least one file is required\n";
        rc = 1;
    }
    return rc;
}
