// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  Abstract:  Command line program to display and manipulate image metadata.

  File:      exiv2.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   10-Dec-03, ahu: created
 */
// *****************************************************************************

// include local header files which are not part of libexiv2
#include <exiv2/futils.hpp>

#include "actions.hpp"
#include "i18n.h"  // NLS support.
#include "params.hpp"

int main(int argc, char* const argv[])
{
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

#ifdef EXV_ENABLE_NLS
    setlocale(LC_ALL, "");
    const std::string localeDir = EXV_LOCALEDIR[0] == '/' ? EXV_LOCALEDIR : (Exiv2::getProcessPath() + EXV_SEPARATOR_STR + EXV_LOCALEDIR);
    bindtextdomain(EXV_PACKAGE_NAME, localeDir.c_str());
    textdomain(EXV_PACKAGE_NAME);
#endif

    // Handle command line arguments
    Params& params = Params::instance();
    if (params.getopt(argc, argv)) {
        params.usage();
        return EXIT_FAILURE;
    }
    if (params.help_) {
        params.help();
        return EXIT_SUCCESS;
    }
    if (params.version_) {
        params.version(params.verbose_);
        return EXIT_SUCCESS;
    }

    int rc = EXIT_SUCCESS;

    try {
        // Create the required action class
        Action::TaskFactory& taskFactory = Action::TaskFactory::instance();
        auto task = taskFactory.create(Action::TaskType(params.action_));
        assert(task);

        // Process all files
        int n = 1;
        int s = static_cast<int>(params.files_.size());
        int w = s > 9 ? s > 99 ? 3 : 2 : 1;
        for (const auto& file : params.files_) {
            if (params.verbose_) {
                std::cout << _("File") << " " << std::setw(w) << std::right << n++ << "/" << s << ": " << file
                          << std::endl;
            }
            int ret = task->run(file);
            if (rc == EXIT_SUCCESS)
                rc = ret;
        }

        Exiv2::XmpParser::terminate();

    } catch (const std::exception& exc) {
        std::cerr << "Uncaught exception: " << exc.what() << std::endl;
        rc = EXIT_FAILURE;
    }

    // Return a positive one byte code for better consistency across platforms
    return static_cast<unsigned int>(rc) % 256;
} // main
