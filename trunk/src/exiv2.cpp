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
  Abstract:  Command line program to display and manipulate image %Exif data

  File:      exiv2.cpp
  Version:   $Name:  $ $Revision: 1.8 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   10-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.8 $ $RCSfile: exiv2.cpp,v $")

// *****************************************************************************
// included header files
#include "exiv2.hpp"
#include "actions.hpp"
#include "utils.hpp"

#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>

// *****************************************************************************
// local declarations
namespace {

    // Evaluate [-]HH[:MM[:SS]], returns true and sets time to the value
    // in seconds if successful, else returns false.
    bool parseTime(const std::string& ts, long& time);
}

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

    // Create the required action class
    Action::TaskFactory& taskFactory = Action::TaskFactory::instance();
    Action::Task::AutoPtr task 
        = taskFactory.create(Action::TaskType(params.action_));
    assert(task.get());

    // Process all files
    int n = 1;
    int s = params.files_.size();
    int w = s > 9 ? s > 99 ? 3 : 2 : 1;
    Params::Files::const_iterator e = params.files_.end();
    for (Params::Files::const_iterator i = params.files_.begin(); i != e; ++i) {
        if (params.verbose_) {
            std::cout << "File " << std::setw(w) << n++ << "/" << s << ": " 
                      << *i << "\n";
        }
        task->run(*i);
    }
    return 0;
} // main

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
    os << "Exiv2 version 0.4-pre1, " 
       << "Copyright (C) 2004 Andreas Huggel.\n\n"
       << "This is free software; see the source for copying conditions.  "
       << "There is NO \nwarranty; not even for MERCHANTABILITY or FITNESS FOR "
       << "A PARTICULAR PURPOSE.\n";
}

void Params::usage(std::ostream& os) const
{
    os << "Usage: " << progname() 
       << " [ options ] [ action ] file ...\n\n"
       << "Manipulate the Exif metadata of images.\n";
}

void Params::help(std::ostream& os) const
{
    usage(os);
    os << "\nActions:\n"
       << "  ad | adjust   Adjust the metadata timestamp by the given time. This\n"
       << "                action requires the option -a time.\n"
       << "  pr | print    Print the Exif (or other) image metadata.\n"
       << "  rm | delete   Delete the Exif section or thumbnail from the files.\n"
       << "  ex | extract  Extract the Exif data or Exif thumbnail to files.\n"
       << "  in | insert   Insert the Exif data from corresponding *.exv files.\n"
       << "  mv | rename   Rename files according to the Exif create timestamp.\n"
       << "                The filename format can be set with -r format.\n"
       << "\nOptions:\n"
       << "   -h      Display this help and exit.\n"
       << "   -V      Show the program version and exit.\n"
       << "   -v      Be verbose during the program run.\n"
       << "   -f      Do not prompt before overwriting existing files (force).\n"
       << "   -a time Time adjustment in the format [-]HH[:MM[:SS]]. This option\n"
       << "           is only used with the `adjust' action.\n"
       << "   -p mode Print mode for the `print' action. Possible modes are `s'\n"
       << "           for a summary (the default), `i' for interpreted data, `v'\n"
       << "           for uninterpreted data values and `h' for a hexdump.\n"
       << "   -d tgt  Delete target for the `delete' action. Possible targets are\n"
       << "           `e' to delete the whole Exif section (the default) and `t'\n"
       << "           to delete only the Exif thumbnail from the files.\n"
       << "   -e tgt  Extract target for the `extract' action. Possible targets\n"
       << "           are `e' to extract the Exif data to a binary file (the\n"
       << "           default) and `t' to extract only the Exif thumbnail.\n"
       << "   -r fmt  Filename format for the `rename' action. The format string\n"
       << "           follows strftime(3). Default filename format is " 
       <<             format_ << ".\n\n";
} // Params::help

int Params::option(int opt, const std::string& optarg, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h': help_ = true; break;
    case 'V': version_ = true; break;
    case 'v': verbose_ = true; break;
    case 'f': force_ = true; break;
    case 'r': 
        switch (action_) {
        case Action::none:
            action_ = Action::rename;
            format_ = optarg; 
            break;
        case Action::rename:
            std::cerr << progname() 
                      << ": Ignoring surplus option -r \"" << optarg << "\"\n";
            break;
        default:
            std::cerr << progname() 
                      << ": Option -r is not compatible with a previous option\n";
            rc = 1;
            break;
        }
        break;
    case 'a':
        switch (action_) {
        case Action::none:
            action_ = Action::adjust;
            adjust_ = parseTime(optarg, adjustment_);
            if (!adjust_) {
                std::cerr << progname() << ": Error parsing -a option argument `" 
                          << optarg << "'\n";
                rc = 1;
            }
            break;
        case Action::adjust:
            std::cerr << progname() 
                      << ": Ignoring surplus option -a " << optarg << "\n";
            break;
        default:
            std::cerr << progname() 
                      << ": Option -a is not compatible with a previous option\n";
            rc = 1;
            break;
        }
        break;
    case 'p':
        switch (action_) {
        case Action::none:
            action_ = Action::print;
            switch (optarg[0]) {
            case 's': printMode_ = summary; break;
            case 'i': printMode_ = interpreted; break;
            case 'v': printMode_ = values; break;
            case 'h': printMode_ = hexdump; break;
            default:
                std::cerr << progname() << ": Unrecognized print mode `"
                          << optarg << "'\n";
                rc = 1;
                break;
            }
            break;
        case Action::print:
            std::cerr << progname() 
                      << ": Ignoring surplus option -p" << optarg << "\n";
            break;
        default:
            std::cerr << progname() 
                      << ": Option -p is not compatible with a previous option\n";
            rc = 1;
            break;
        }
        break;
    case 'd':
        switch (action_) {
        case Action::none:
            action_ = Action::erase;
            switch (optarg[0]) {
            case 'e': delTarget_ = delExif; break;
            case 't': delTarget_ = delThumb; break;
            default:
                std::cerr << progname() << ": Unrecognized delete target `"
                          << optarg << "'\n";
                rc = 1;
                break;
            }
            break;
        case Action::erase:
            std::cerr << progname() 
                      << ": Ignoring surplus option -d" << optarg << "\n";
            break;
        default:
            std::cerr << progname() 
                      << ": Option -d is not compatible with a previous option\n";
            rc = 1;
            break;
        }
        break;
    case 'e':
        switch (action_) {
        case Action::none:
            action_ = Action::extract;
            switch (optarg[0]) {
            case 'e': extractTarget_ = extExif; break;
            case 't': extractTarget_ = extThumb; break;
            default:
                std::cerr << progname() << ": Unrecognized extract target `"
                          << optarg << "'\n";
                rc = 1;
                break;
            }
        case Action::extract:
            std::cerr << progname() 
                      << ": Ignoring surplus option -e" << optarg << "\n";
            break;
        default:
            std::cerr << progname() 
                      << ": Option -e is not compatible with a previous option\n";
            rc = 1;
            break;
        }
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
} // Params::option

int Params::nonoption(const std::string& argv)
{
    int rc = 0;
    bool action = false;
    if (first_) {
        // The first non-option argument must be the action
        first_ = false;
        if (argv == "ad" || argv == "adjust") {
            if (action_ != Action::none && action_ != Action::adjust) {
                std::cerr << progname() << ": Action adjust is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::adjust;
        }
        if (argv == "pr" || argv == "print") {
            if (action_ != Action::none && action_ != Action::print) {
                std::cerr << progname() << ": Action print is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::print;
        }
        if (argv == "rm" || argv == "delete") {
            if (action_ != Action::none && action_ != Action::erase) {
                std::cerr << progname() << ": Action delete is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::erase;
        }
        if (argv == "ex" || argv == "extract") {
            if (action_ != Action::none && action_ != Action::extract) {
                std::cerr << progname() << ": Action extract is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::extract;
        }
        if (argv == "in" || argv == "insert") {
            if (action_ != Action::none && action_ != Action::insert) {
                std::cerr << progname() << ": Action insert is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::insert;
        }
        if (argv == "mv" || argv == "rename") {
            if (action_ != Action::none && action_ != Action::rename) {
                std::cerr << progname() << ": Action rename is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::rename;
        }
        if (action_ == Action::none) {
            // if everything else fails, assume print as the default action
            action_ = Action::print;
        }
    }
    if (!action) {
        files_.push_back(argv);
    }
    return rc;
} // Params::nonoption

int Params::getopt(int argc, char* const argv[])
{ 
    int rc = Util::Getopt::getopt(argc, argv, optstring_);
    // Further consistency checks
    if (help_ || version_) return 0;
    if (action_ == Action::none) {
        // This shouldn't happen since print is taken as default action
        std::cerr << progname() << ": An action must be specified\n";
        rc = 1;
    }
    if (action_ == Action::adjust && !adjust_) {
        std::cerr << progname() 
                  << ": Adjust action requires option -a time\n";
        rc = 1;
    }
    if (0 == files_.size()) {
        std::cerr << progname() << ": At least one file is required\n";
        rc = 1;
    }
    return rc;
} // Params::getopt

// *****************************************************************************
// local implementations
namespace {

    bool parseTime(const std::string& ts, long& time)
    {
        std::string hstr, mstr, sstr;
        char *cts = new char[ts.length() + 1];
        strcpy(cts, ts.c_str());
        char *tmp = ::strtok(cts, ":");
        if (tmp) hstr = tmp;
        tmp = ::strtok(0, ":");
        if (tmp) mstr = tmp;
        tmp = ::strtok(0, ":");
        if (tmp) sstr = tmp;
        delete[] cts;

        int sign = 1;
        long hh(0), mm(0), ss(0);
        // [-]HH part
        if (!Util::strtol(hstr.c_str(), hh)) return false;
        if (hh < 0) {
            sign = -1;
            hh *= -1;
        }
        // check for the -0 special case
        if (hh == 0 && hstr.find('-') != std::string::npos) sign = -1;
        // MM part, if there is one
        if (mstr != "") {
            if (!Util::strtol(mstr.c_str(), mm)) return false;
            if (mm > 59) return false;
            if (mm < 0) return false;
        }
        // SS part, if there is one
        if (sstr != "") {
            if (!Util::strtol(sstr.c_str(), ss)) return false;
            if (ss > 59) return false;
            if (ss < 0) return false;
        }

        time = sign * (hh * 3600 + mm * 60 + ss);
        return true;
    } // parseTime

}
