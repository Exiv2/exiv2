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
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   10-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "exiv2.hpp"
#include "actions.hpp"
#include "utils.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cassert>

// *****************************************************************************
// local declarations
namespace {

    //! List of all command identifiers and corresponding strings
    static const CmdIdAndString cmdIdAndString[] = {
        add, "add", 
        set, "set", 
        del, "del", 
        invalidCmdId, "invalidCmd"                  // End of list marker
    };

    // Return a command Id for a command string 
    CmdId commandId(const std::string& cmdString);

    // Evaluate [-]HH[:MM[:SS]], returns true and sets time to the value
    // in seconds if successful, else returns false.
    bool parseTime(const std::string& ts, long& time);

    /*!
      @brief Parse the oparg string into a bitmap of common targets.
      @param optarg Option arguments
      @param action Action being processed
      @return A bitmap of common targets or -1 in case of a parse error 
     */
    int parseCommonTargets(const std::string& optarg,
                           const std::string& action);

    /*!
      @brief Parse metadata modification commands from multiple files
      @param modifyCmds Reference to a structure to store the parsed commands
      @param cmdFiles Container with the file names
     */
    bool parseCmdFiles(ModifyCmds& modifyCmds, 
                       const Params::CmdFiles& cmdFiles);

    /*!
      @brief Parse metadata modification commands from a container of commands
      @param modifyCmds Reference to a structure to store the parsed commands
      @param cmdLines Container with the commands
     */
    bool parseCmdLines(ModifyCmds& modifyCmds, 
                       const Params::CmdLines& cmdLines);

    /*!
      @brief Parse one line of the command file
      @param modifyCmd Reference to a command structure to store the parsed 
             command
      @param line Input line
      @param num Line number (used for error output)
     */
    bool parseLine(ModifyCmd& modifyCmd,
                   const std::string& line, int num);

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
    int s = static_cast<int>(params.files_.size());
    int w = s > 9 ? s > 99 ? 3 : 2 : 1;
    Params::Files::const_iterator e = params.files_.end();
    for (Params::Files::const_iterator i = params.files_.begin(); i != e; ++i) {
        if (params.verbose_) {
            std::cout << "File " << std::setw(w) << n++ << "/" << s << ": " 
                      << *i << std::endl;
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
    os << PACKAGE_STRING << ", " 
       << "Copyright (C) 2004, 2005 Andreas Huggel.\n\n"
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
       << "  ad | adjust   Adjust Exif timestamps by the given time. This\n"
       << "                action requires the option -a time.\n"
       << "  pr | print    Print image metadata.\n"
       << "  rm | delete   Delete image metadata from the files.\n"
       << "  in | insert   Insert metadata from corresponding *.exv files.\n"
       << "  ex | extract  Extract metadata to *.exv and thumbnail image files.\n"
       << "  mv | rename   Rename files according to the Exif create timestamp.\n"
       << "                The filename format can be set with -r format.\n"
       << "  mo | modify   Apply commands to modify (add, set, delete) the Exif\n"
       << "                and Iptc metadata of image files. Requires option -m or -M\n"
       << "\nOptions:\n"
       << "   -h      Display this help and exit.\n"
       << "   -V      Show the program version and exit.\n"
       << "   -v      Be verbose during the program run.\n"
       << "   -f      Do not prompt before overwriting existing files (force).\n"
       << "   -a time Time adjustment in the format [-]HH[:MM[:SS]]. This option\n"
       << "           is only used with the `adjust' action.\n"
       << "   -p mode Print mode for the `print' action. Possible modes are:\n"
       << "             s : print a summary of the Exif metadata (the default)\n"
       << "             t : interpreted (translated) Exif data\n"
       << "             v : plain Exif data values\n"
       << "             h : hexdump of the Exif data\n"
       << "             i : Iptc data values\n"
       << "             c : Jpeg comment\n"
       << "   -d tgt  Delete target(s) for the `delete' action. Possible targets are:\n"
       << "             a : all supported metadata (the default)\n"
       << "             e : Exif section\n"
       << "             t : Exif thumbnail only\n"
       << "             i : Iptc data\n"
       << "             c : Jpeg comment\n"
       << "   -i tgt  Insert target(s) for the `insert' action. Possible targets are\n"
       << "           the same as those for the -d option. Only Jpeg thumbnails can\n"
       << "           be inserted, they need to be named <file>-thumb.jpg\n"
       << "   -e tgt  Extract target(s) for the `extract' action. Possible targets\n"
       << "           are the same as those for the -d option.\n"
       << "   -r fmt  Filename format for the `rename' action. The format string\n"
       << "           follows strftime(3). Default filename format is " 
       <<             format_ << ".\n"
       << "   -m file Command file for the modify action. The format for commands is\n"
       << "           set|add|del <key> [[<type>] <value>].\n"
       << "   -M cmd  Command line for the modify action. The format for the\n"
       << "           commands is the same as that of the lines of a command file.\n"
       << "   -l dir  Location (directory) for files to be inserted or extracted.\n\n";
} // Params::help

int Params::option(int opt, const std::string& optarg, int optopt)
{
    int rc = 0;
    switch (opt) {
    case 'h': help_ = true; break;
    case 'V': version_ = true; break;
    case 'v': verbose_ = true; break;
    case 'f': force_ = true; break;
    case 'r': rc = evalRename(optarg); break;
    case 'a': rc = evalAdjust(optarg); break;
    case 'p': rc = evalPrint(optarg); break;
    case 'd': rc = evalDelete(optarg); break;
    case 'e': rc = evalExtract(optarg); break;
    case 'i': rc = evalInsert(optarg); break;
    case 'm': rc = evalModify(opt, optarg); break;
    case 'M': rc = evalModify(opt, optarg); break;
    case 'l': directory_ = optarg; break;
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

int Params::evalRename(const std::string& optarg)
{
    int rc = 0;
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
    return rc;
} // Params::evalRename

int Params::evalAdjust(const std::string& optarg)
{
    int rc = 0;
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
    return rc;
} // Params::evalAdjust

int Params::evalPrint(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::print;
        switch (optarg[0]) {
        case 's': printMode_ = pmSummary; break;
        case 't': printMode_ = pmInterpreted; break;
        case 'v': printMode_ = pmValues; break;
        case 'h': printMode_ = pmHexdump; break;
        case 'i': printMode_ = pmIptc; break;
        case 'c': printMode_ = pmComment; break;
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
    return rc;
} // Params::evalPrint

int Params::evalDelete(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::erase;
        target_ = 0;
        // fallthrough
    case Action::erase:
        rc = parseCommonTargets(optarg, "erase");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {    
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() 
                  << ": Option -d is not compatible with a previous option\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::evalDelete

int Params::evalExtract(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::extract;
        target_ = 0;
        // fallthrough
    case Action::extract:
        rc = parseCommonTargets(optarg, "extract");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {    
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() 
                  << ": Option -e is not compatible with a previous option\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::evalExtract

int Params::evalInsert(const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::insert;
        target_ = 0;
        // fallthrough
    case Action::insert:
        rc = parseCommonTargets(optarg, "insert");
        if (rc > 0) {
            target_ |= rc;
            rc = 0;
        }
        else {    
            rc = 1;
        }
        break;
    default:
        std::cerr << progname() 
                  << ": Option -i is not compatible with a previous option\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::evalInsert

int Params::evalModify(int opt, const std::string& optarg)
{
    int rc = 0;
    switch (action_) {
    case Action::none:
        action_ = Action::modify;
        // fallthrough
    case Action::modify:
        if (opt == 'm') cmdFiles_.push_back(optarg);  // parse the files later
        if (opt == 'M') cmdLines_.push_back(optarg);  // parse the commands later
        break;
    default:
        std::cerr << progname() 
                  << ": Option -" << (char)opt
                  << " is not compatible with a previous option\n";
        rc = 1;
        break;
    }
    return rc;
} // Params::evalModify

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
        if (argv == "mo" || argv == "modify") {
            if (action_ != Action::none && action_ != Action::modify) {
                std::cerr << progname() << ": Action modify is not "
                          << "compatible with the given options\n";
                rc = 1;
            }
            action = true;
            action_ = Action::modify;
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
    if (action_ == Action::modify && cmdFiles_.empty() && cmdLines_.empty()) {
        std::cerr << progname() 
                  << ": Modify action requires at least one -m or -M option\n";
        rc = 1;
    }
    if (0 == files_.size()) {
        std::cerr << progname() << ": At least one file is required\n";
        rc = 1;
    }
    if (rc == 0 && action_ == Action::modify) {
        // Parse command files
        if (!parseCmdFiles(modifyCmds_, cmdFiles_)) {
            std::cerr << progname() << ": Error parsing -m option arguments\n"; 
            rc = 1;
        }
    }
    if (rc ==0 && action_ == Action::modify) {
        // Parse command lines
        if (!parseCmdLines(modifyCmds_, cmdLines_)) {
            std::cerr << progname() << ": Error parsing -M option arguments\n";
            rc = 1;
        }
    }
    if (!directory_.empty() && !(action_ == Action::insert || action_ == Action::extract)) {
        std::cerr << progname() << ": -l option can only be used with extract or insert actions\n"; 
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

    int parseCommonTargets(const std::string& optarg, 
                           const std::string& action)
    {
        int rc = 0;
        int target = 0;
        for (size_t i = 0; rc == 0 && i < optarg.size(); ++i) {
            switch (optarg[i]) {
            case 'e': target |= Params::ctExif; break;
            case 'i': target |= Params::ctIptc; break;
            case 'c': target |= Params::ctComment; break;
            case 't': target |= Params::ctThumb; break;
            case 'a': target |=   Params::ctExif 
                                | Params::ctIptc
                                | Params::ctComment; break;
            default:
                std::cerr << Params::instance().progname() << ": Unrecognized " 
                          << action << " target `" << optarg[i] << "'\n";
                rc = -1;
                break;
            }
        }
        return rc ? rc : target;
    } // parseCommonTargets

    bool parseCmdFiles(ModifyCmds& modifyCmds, 
                       const Params::CmdFiles& cmdFiles)
    {
        Params::CmdFiles::const_iterator end = cmdFiles.end();
        Params::CmdFiles::const_iterator filename = cmdFiles.begin();
        for ( ; filename != end; ++filename) {
            try {
                std::ifstream file(filename->c_str());
                if (!file) {
                    std::cerr << *filename 
                              << ": Failed to open command file for reading\n";
                    return false;
                }
                int num = 0;
                std::string line;
                while (std::getline(file, line)) {
                    ModifyCmd modifyCmd;
                    if (parseLine(modifyCmd, line, ++num)) {
                        modifyCmds.push_back(modifyCmd);
                    }
                }
            }
            catch (const Exiv2::Error& error) {
                std::cerr << *filename << ", line " << error << "\n";
                return false;
            }
        }
        return true;
    } // parseCmdFile

    bool parseCmdLines(ModifyCmds& modifyCmds, 
                       const Params::CmdLines& cmdLines)
    {
        try {
            int num = 0;
            Params::CmdLines::const_iterator end = cmdLines.end();
            Params::CmdLines::const_iterator line = cmdLines.begin();
            for ( ; line != end; ++line) {
                ModifyCmd modifyCmd;
                if (parseLine(modifyCmd, *line, ++num)) {
                    modifyCmds.push_back(modifyCmd);
                }
            }
            return true;
        }
        catch (const Exiv2::Error& error) {
            std::cerr << "-M option " << error << "\n";
            return false;
	}
    } // parseCmdLines

    bool parseLine(ModifyCmd& modifyCmd, const std::string& line, int num)
    {
        const std::string delim = " \t";

        // Skip empty lines and comments
        std::string::size_type cmdStart = line.find_first_not_of(delim);
        if (cmdStart == std::string::npos || line[cmdStart] == '#') return false;

        // Get command and key 
        std::string::size_type cmdEnd = line.find_first_of(delim, cmdStart+1);
        std::string::size_type keyStart = line.find_first_not_of(delim, cmdEnd+1);
        std::string::size_type keyEnd = line.find_first_of(delim, keyStart+1);
        if (   cmdStart == std::string::npos
            || cmdEnd == std::string::npos
            || keyStart == std::string::npos) {
            throw Exiv2::Error(Exiv2::toString(num) 
                               + ": Invalid command line");
        }

        std::string cmd(line.substr(cmdStart, cmdEnd-cmdStart));
        CmdId cmdId = commandId(cmd);
        if (cmdId == invalidCmdId) {
            throw Exiv2::Error(Exiv2::toString(num) 
                               + ": Invalid command `" + cmd + "'");
        }

        Exiv2::TypeId defaultType = Exiv2::invalidTypeId;
        std::string key(line.substr(keyStart, keyEnd-keyStart));
        MetadataId metadataId = invalidMetadataId;
        try {
            Exiv2::IptcKey iptcKey(key);
            metadataId = iptc;
            defaultType = Exiv2::IptcDataSets::dataSetType(iptcKey.tag(), 
                                                           iptcKey.record());
        }
        catch (const Exiv2::Error&) {}
        if (metadataId == invalidMetadataId) {
            try {
                Exiv2::ExifKey exifKey(key);
                metadataId = exif;
                defaultType = Exiv2::asciiString;
            }
            catch (const Exiv2::Error&) {}
        }
        if (metadataId == invalidMetadataId) {
            throw Exiv2::Error(Exiv2::toString(num) 
                               + ": Invalid key `" + key + "'");
        }

        std::string value;
        Exiv2::TypeId type = Exiv2::invalidTypeId;
        bool explicitType = true;
        if (cmdId != del) {
            // Get type and value
            std::string::size_type typeStart 
                = line.find_first_not_of(delim, keyEnd+1);
            std::string::size_type typeEnd 
                = line.find_first_of(delim, typeStart+1);
            std::string::size_type valStart = typeStart;
            std::string::size_type valEnd = line.find_last_not_of(delim);

            if (   keyEnd == std::string::npos 
                || typeStart == std::string::npos
                || typeEnd == std::string::npos
                || valStart == std::string::npos) {
                throw Exiv2::Error(Exiv2::toString(num) 
                                   + ": Invalid command line");
            }

            std::string typeStr(line.substr(typeStart, typeEnd-typeStart));
            type = Exiv2::TypeInfo::typeId(typeStr);
            if (type != Exiv2::invalidTypeId) {
                valStart = line.find_first_not_of(delim, typeEnd+1);
                if (valStart == std::string::npos) {
                    throw Exiv2::Error(Exiv2::toString(num) 
                                       + ": Invalid command line");
                }
            }
            else {
                type = defaultType;
                explicitType = false;
            }
            if (type == Exiv2::invalidTypeId) {
                throw Exiv2::Error(Exiv2::toString(num) 
                                   + ": Invalid type");
            }

            value = line.substr(valStart, valEnd+1-valStart);
            std::string::size_type last = value.length()-1;
            if (  (value[0] == '"' || value[last] == '"') 
                && value[0] != value[last]) {
                throw Exiv2::Error(Exiv2::toString(num) 
                                   + ": Unbalanced quotes");
            }
            if (value[0] == '"') {
                value = value.substr(1, value.length()-2);
            }
        }

        modifyCmd.cmdId_ = cmdId;
        modifyCmd.key_ = key;
        modifyCmd.metadataId_ = metadataId;
        modifyCmd.typeId_ = type;
        modifyCmd.explicitType_ = explicitType;
        modifyCmd.value_ = value;

        return true;
    } // parseLine

    CmdId commandId(const std::string& cmdString)
    {
        int i = 0;
        for (;   cmdIdAndString[i].cmdId_ != invalidCmdId
                 && cmdIdAndString[i].cmdString_ != cmdString; ++i) {}
        return cmdIdAndString[i].cmdId_;
    }

}
