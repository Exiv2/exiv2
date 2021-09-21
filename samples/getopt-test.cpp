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
// getopt-test.cpp
// Sample program to test getopt()
// Command:
// $ getopt-test -v pr -P EIXxgklnycsvth file1 file2
//
// Output:
// standard getopt()
// 118 = v optind = 2 opterr = 1 optopt = 118 optarg = unknown
// -1 optind = 2 opterr = 1 optopt = 118 optarg = unknown
//
// homemade getopt()
// 118 = v optind = 2 opterr = 1 optopt = 118 optarg = unknown
// -1 optind = 2 opterr = 1 optopt = 118 optarg = unknown
//
// Params::option() opt = 118 optarg =  optopt = 118
// Params::nonoption() pr
// Params::nonoption() -P
// Params::nonoption() EIXxgklnycsvth
// Params::nonoption() file1
// Params::nonoption() file2
// Params::getopt() rc = 0

#include <exiv2/exiv2.hpp>

// getopt.{cpp|hpp} is not part of libexiv2
#include "getopt.hpp"

#ifdef EXV_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <iostream>
#include <iomanip>
#include <cassert>

#define Safe(x) (x?x:"unknown")
const char* optstring = ":hVvqfbuktTFa:Y:O:D:r:p:P:d:e:i:c:m:M:l:S:g:K:n:Q:";

// *****************************************************************************
// class Params
class Params : public Util::Getopt {

public:
    /*!
      @brief Call Getopt::getopt() with optstring, to inititate command line
             argument parsing, perform consistency checks after all command line
             arguments are parsed.

      @param argc Argument count as passed to main() on program invocation.
      @param argv Argument array as passed to main() on program invocation.

      @return 0 if successful, >0 in case of errors.
     */
    int getopt(int argc, char** const argv)
    {
        int rc = Util::Getopt::getopt(argc, argv, ::optstring);
    	std::cout << "Params::getopt()"
    	          << " rc = " << rc
    	          << std::endl;
    	return rc ;
    }

    //! Handle options and their arguments.
    int option(int opt, const std::string& optarg, int optopt) override
    {
    	std::cout << "Params::option()"
    	          << " opt = "    << opt
    	          << " optarg = " << optarg
    	          << " optopt = " << optopt
    	          << std::endl;
    	return 0;
    }

    //! Handle non-option parameters.
    int nonoption(const std::string& argv) override
    {
    	std::cout << "Params::nonoption()"
    	          << " " << argv
    	          << std::endl;
    	return 0 ;
    }
}; // class Params

int main(int argc, char** const argv)
{
	Exiv2::XmpParser::initialize();
	::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

	int n;

#ifdef EXV_HAVE_UNISTD_H
	std::cout << "standard getopt()" << std::endl;
	do {
	    n = ::getopt(argc,argv,::optstring);
	    if ( n >= 0 ) {
            char N = static_cast<char>(n);
            std::cout << n << " = " << N;
        } else {
            std::cout << n ;
        }
        std::cout << " optind = " << ::optind
				  << " opterr = " << ::opterr
				  << " optopt = " << ::optopt
				  << " optarg = " << Safe(::optarg)
				  << std::endl;
	} while ( n >= 0 );
	std::cout << std::endl;
#endif

	std::cout << "homemade getopt()" << std::endl;
	do {
	    n = Util::getopt(argc,argv,::optstring);
	    if ( n >= 0 ) {
            char N = static_cast<char>(n);
            std::cout << n << " = " << N;
        } else {
            std::cout << n ;
        }
        std::cout << " optind = " << Util::optind
				  << " opterr = " << Util::opterr
				  << " optopt = " << Util::optopt
				  << " optarg = " << Safe(Util::optarg)
				  << std::endl;

	} while ( n >= 0 );
	std::cout << std::endl;

    // Handle command line arguments
    Params params;
    params.getopt(argc, argv);

    return 0;
}

