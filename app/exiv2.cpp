// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

// include local header files which are not part of libexiv2
#include "actions.hpp"
#include "app_utils.hpp"
#include "exiv2app.hpp"

#include "convert.hpp"
#include "getopt.hpp"
#include "i18n.h"  // NLS support.
#include "utils.hpp"
#include "xmp_exiv2.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#endif

// *****************************************************************************
// local declarations
namespace {
constexpr auto emptyYodAdjust_ = std::array{
    Params::YodAdjust{false, "-Y", 0},
    Params::YodAdjust{false, "-O", 0},
    Params::YodAdjust{false, "-D", 0},
};

//! List of all command identifiers and corresponding strings
constexpr struct CmdIdAndString {
  CmdId cmdId_;
  const char* string_;
  //! Comparison operator for \em string
  bool operator==(const std::string& string) const {
    return string == string_;
  }
} cmdIdAndString[] = {
    {CmdId::add, "add"},
    {CmdId::set, "set"},
    {CmdId::del, "del"},
    {CmdId::reg, "reg"},
    {CmdId::invalid, "invalidCmd"},  // End of list marker
};

// Return a command Id for a command string
CmdId commandId(const std::string& cmdString);

// Evaluate [-]HH[:MM[:SS]], returns true and sets time to the value
// in seconds if successful, else returns false.
bool parseTime(const std::string& ts, int64_t& time);

/*!
  @brief Parse the oparg string into a bitmap of common targets.
  @param optArg Option arguments
  @param action Action being processed
  @return A bitmap of common targets or -1 in case of a parse error
 */
int64_t parseCommonTargets(const std::string& optArg, const std::string& action);

/*!
  @brief Parse numbers separated by commas into container
  @param previewNumbers Container for the numbers
  @param optArg Option arguments
  @param j Starting index into optArg
  @return Number of characters processed
 */
int parsePreviewNumbers(Params::PreviewNumbers& previewNumbers, const std::string& optArg, int j);

/*!
  @brief Parse metadata modification commands from multiple files
  @param modifyCmds Reference to a structure to store the parsed commands
  @param cmdFiles Container with the file names
 */
bool parseCmdFiles(ModifyCmds& modifyCmds, const Params::CmdFiles& cmdFiles);

/*!
  @brief Parse metadata modification commands from a container of commands
  @param modifyCmds Reference to a structure to store the parsed commands
  @param cmdLines Container with the commands
 */
bool parseCmdLines(ModifyCmds& modifyCmds, const Params::CmdLines& cmdLines);

/*!
  @brief Parse one line of the command file
  @param modifyCmd Reference to a command structure to store the parsed
         command
  @param line Input line
  @param num Line number (used for error output)
 */
bool parseLine(ModifyCmd& modifyCmd, const std::string& line, int num);

/*!
  @brief Parses a string containing backslash-escapes
  @param input Input string, assumed to be UTF-8
 */
std::string parseEscapes(const std::string& input);
}  // namespace

// *****************************************************************************
// Main
int main(int argc, char* const argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

#ifdef EXV_ENABLE_NLS
  setlocale(LC_ALL, "");
  auto localeDir = []() -> std::string {
    if constexpr (EXV_LOCALEDIR[0] == '/')
      return EXV_LOCALEDIR;
    else
      return Exiv2::getProcessPath() + EXV_SEPARATOR_STR + EXV_LOCALEDIR;
  }();
  bindtextdomain(EXV_PACKAGE_NAME, localeDir.c_str());
  textdomain(EXV_PACKAGE_NAME);
#endif

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
    Params::version(params.verbose_);
    return 0;
  }

  int returnCode = EXIT_SUCCESS;

  try {
    // Create the required action class
    auto task = Action::TaskFactory::instance().create(static_cast<Action::TaskType>(params.action_));

    // Process all files
    auto filesCount = params.files_.size();
    if (params.action_ & Action::extract && params.target_ & Params::ctStdInOut && filesCount > 1) {
      std::cerr << params.progname() << ": " << _("Only one file is allowed when extracting to stdout") << '\n';
      returnCode = EXIT_FAILURE;
    } else {
      int w = [=]() {
        if (filesCount > 9) {
          if (filesCount > 99)
            return 3;
          return 2;
        }
        return 1;
      }();
      int n = 1;
      for (const auto& file : params.files_) {
        // If extracting to stdout then ignore verbose
        if (params.verbose_ && !(params.action_ & Action::extract && params.target_ & Params::ctStdInOut)) {
          std::cout << _("File") << " " << std::setw(w) << std::right << n++ << "/" << filesCount << ": " << file
                    << '\n';
        }
        task->setBinary(params.binary_);
        int ret = task->run(file);
        if (returnCode == EXIT_SUCCESS)
          returnCode = ret;
      }

      Action::TaskFactory::instance().cleanup();
      Exiv2::XmpParser::terminate();
    }
  } catch (const std::exception& exc) {
    std::cerr << "Uncaught exception: " << exc.what() << '\n';
    returnCode = EXIT_FAILURE;
  }

  // Return a positive one byte code for better consistency across platforms
  return static_cast<unsigned int>(returnCode) % 256;
}  // main

// *****************************************************************************
// class Params

Params::Params() :
    optstring_(":hVvqfbuktTFa:Y:O:D:r:p:P:d:e:i:c:m:M:l:S:g:K:n:Q:"),
    target_(ctExif | ctIptc | ctComment | ctXmp),
    yodAdjust_(emptyYodAdjust_),
    format_("%Y%m%d_%H%M%S") {
}

Params& Params::instance() {
  static Params instance_;
  return instance_;
}

void Params::version(bool verbose, std::ostream& os) {
  os << EXV_PACKAGE_STRING << '\n';
  if (Params::instance().greps_.empty() && !verbose) {
    os << "\n"
       << _("This program is free software; you can redistribute it and/or\n"
            "modify it under the terms of the GNU General Public License\n"
            "as published by the Free Software Foundation; either version 2\n"
            "of the License, or (at your option) any later version.\n")
       << "\n"
       << _("This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "GNU General Public License for more details.\n")
       << "\n"
       << _("You should have received a copy of the GNU General Public\n"
            "License along with this program; if not, write to the Free\n"
            "Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
            "Boston, MA 02110-1301 USA\n");
  }

  if (verbose)
    Exiv2::dumpLibraryInfo(os, Params::instance().greps_);
}

void Params::usage(std::ostream& os) const {
  os << _("Usage:") << " " << progname() << " " << _("[ option [ arg ] ]+ [ action ] file ...\n\n")
     << _("Image metadata manipulation tool.\n");
}

void Params::help(std::ostream& os) const {
  usage(os);
  os << _("\nWhere file is one or more files, optionally containing a URL\n"
          "(http, https, ftp, sftp, data or file) or wildcard\n")
     << _("\nActions:\n")
     << _("  pr | print    Print image metadata (default is a summary). This is the default\n"
          "                action\n")
     << _("  ad | adjust   Adjust Exif timestamps by the given time. Requires\n"
          "                at least one of -a, -Y, -O or -D\n")
     << _("  rm | delete   Deletes image metadata, use -d to choose type to delete\n"
          "                (default is all)\n")
     << _("  in | insert   Insert metadata from .exv, .xmp, thumbnail or .icc file.\n"
          "                Use option -S to change the suffix of the input files and\n"
          "                -l to change the location\n")
     << _("  ex | extract  Extract metadata to .exv, .xmp, preview image, thumbnail,\n"
          "                or ICC profile. Use option -S to change the suffix of the input\n"
          "                files and -l to change the location\n")
     << _("  mv | rename   Rename files and/or set file timestamps according to the\n"
          "                Exif timestamps. The filename format can be set with\n"
          "                -r format, timestamp options are controlled with -t and -T\n")
     << _("  mo | modify   Apply commands to modify the Exif, IPTC and XMP metadata.\n"
          "                Requires option -m or -M\n")
     << _("  fi | fixiso   Copy ISO setting from Canon and Nikon makernotes, to the\n"
          "                standard Exif tag\n")
     << _("  fc | fixcom   Convert the Unicode Exif user comment to UCS-2. The current\n"
          "                character encoding can be specified with the -n option\n")
     << _("\nOptions:\n") << _("   -h      Display this help and exit\n")
     << _("   -V      Show the program version and exit\n") << _("   -v      Be verbose during the program run\n")
     << _("   -q      Silence warnings and error messages (quiet)\n")
     << _("   -Q lvl  Set log-level to d(ebug), i(nfo), w(arning), e(rror) or m(ute)\n")
     << _("   -b      Obsolete, reserved for use with the test suit\n")
     << _("   -u      Show unknown tags (e.g., Exif.SonyMisc3c.0x022b)\n")
     << _("   -g str  Only output where 'str' matches in output text (grep)\n"
          "           Append /i to 'str' for case insensitive\n")
     << _("   -K key  Only output where 'key' exactly matches tag's key\n")
     << _("   -n enc  Character set to decode Exif Unicode user comments\n")
     << _("   -k      Preserve file timestamps when updating files (keep)\n")
     << _("   -t      Set the file timestamp from Exif metadata when renaming (overrides -k)\n")
     << _("   -T      Only set the file timestamp from Exif metadata ('rename' action)\n")
     << _("   -f      Do not prompt before overwriting existing files (force)\n")
     << _("   -F      Do not prompt before renaming files (Force)\n")
     << _("   -a time Time adjustment in the format [+|-]HH[:MM[:SS]]. For 'adjust' action\n")
     << _("   -Y yrs  Year adjustment with the 'adjust' action\n")
     << _("   -O mon  Month adjustment with the 'adjust' action\n")
     << _("   -D day  Day adjustment with the 'adjust' action\n")
     << _("   -p mode Print mode for the 'print' action. Possible modes are:\n")
     << _("             s : A summary of the Exif metadata (the default)\n")
     << _("             a : Exif, IPTC and XMP tags (shortcut for -Pkyct)\n")
     << _("             e : Exif tags (shortcut for -PEkycv)\n")
     << _("             t : Interpreted (translated) Exif tags (-PEkyct)\n")
     << _("             v : Plain (untranslated) Exif tags values (-PExgnycv)\n")
     << _("             h : Hex dump of the Exif tags (-PExgnycsh)\n") << _("             i : IPTC tags (-PIkyct)\n")
     << _("             x : XMP tags (-PXkyct)\n") << _("             c : JPEG comment\n")
     << _("             p : List available image preview, sorted by size\n")
     << _("             C : Print ICC profile\n")
     << _("             R : Recursive print structure of image (debug build only)\n")
     << _("             S : Print structure of image (limited file types)\n")
     << _("             X : Extract \"raw\" XMP\n")
     << _("   -P flgs Print flags for fine control of tag lists ('print' action):\n")
     << _("             E : Exif tags\n") << _("             I : IPTC tags\n") << _("             X : XMP tags\n")
     << _("             x : Tag number for Exif or IPTC tags (in hexadecimal)\n")
     << _("             g : Group name (e.g. Exif.Photo.UserComment, Photo)\n")
     << _("             k : Key (e.g. Exif.Photo.UserComment)\n")
     << _("             l : Tag label (e.g. Exif.Photo.UserComment, 'User comment')\n")
     << _("             d : Tag description\n")
     << _("             n : Tag name (e.g. Exif.Photo.UserComment, UserComment)\n") << _("             y : Type\n")
     << _("             y : Type\n") << _("             c : Number of components (count)\n")
     << _("             s : Size in bytes of vanilla value (may include NULL)\n")
     << _("             v : Plain data value of untranslated (vanilla)\n")
     << _("             V : Plain data value, data type and the word 'set'\n")
     << _("             t : Interpreted (translated) human readable values\n")
     << _("             h : Hex dump of the data\n")
     << _("   -d tgt1  Delete target(s) for the 'delete' action. Possible targets are:\n")
     << _("             a : All supported metadata (the default)\n") << _("             e : Exif tags\n")
     << _("             t : Exif thumbnail only\n") << _("             i : IPTC tags\n")
     << _("             x : XMP tags\n") << _("             c : JPEG comment\n") << _("             C : ICC Profile\n")
     << _("             c : All IPTC data (any broken multiple IPTC blocks)\n")
     << _("             - : Input from stdin\n")
     << _("   -i tgt2 Insert target(s) for the 'insert' action. Possible targets are\n")
     << _("             a : All supported metadata (the default)\n") << _("             e : Exif tags\n")
     << _("             t : Exif thumbnail only (JPEGs only from <file>-thumb.jpg)\n")
     << _("             i : IPTC tags\n") << _("             x : XMP tags\n") << _("             c : JPEG comment\n")
     << _("             C : ICC Profile, from <file>.icc\n") << _("             X : XMP sidecar from file <file>.xmp\n")
     << _("             XX: \"raw\" metadata from <file>.exv. XMP default, optional Exif and IPTC\n")
     << _("             - : Input from stdin\n")
     << _("   -e tgt3 Extract target(s) for the 'extract' action. Possible targets\n")
     << _("             a : All supported metadata (the default)\n") << _("             e : Exif tags\n")
     << _("             t : Exif thumbnail only (to <file>-thumb.jpg)\n") << _("             i : IPTC tags\n")
     << _("             x : XMP tags\n") << _("             c : JPEG comment\n")
     << _("             pN: Extract N'th preview image to <file>-preview<N>.<ext>\n")
     << _("             C : ICC Profile, to <file>.icc\n") << _("             X : XMP sidecar to <file>.xmp\n")
     << _("             XX: \"raw\" metadata to <file>.exv. XMP default, optional Exif and IPTC\n")
     << _("             - : Output to stdin\n")
     << _("   -r fmt  Filename format for the 'rename' action. The format string\n")
     << _("           follows strftime(3). The following keywords are also supported:\n")
     << _("             :basename:   - original filename without extension\n")
     << _("             :basesuffix: - suffix in original filename, starts with first dot and ends before extension\n")
     << _("             :dirname:    - name of the directory holding the original file\n")
     << _("             :parentname: - name of parent directory\n") << _("           Default 'fmt' is %Y%m%d_%H%M%S\n")
     << _("   -c txt  JPEG comment string to set in the image.\n")
     << _("   -m cmdf Applies commands in 'cmdf' file, for the modify action (see -M for format).\n")
     << _("   -M cmd  Command line for the modify action. The format is:\n")
     << _("           ( (set | add) <key> [[<type>] <value>] |\n") << _("             del <key> [<type>] |\n")
     << _("             reg prefix namespace )\n")
     << _("   -l dir  Location (directory) for files to be inserted from or extracted to.\n")
     << _("   -S suf Use suffix 'suf' for source files for insert action.\n") << _("\nExamples:\n")
     << _("   exiv2 -pe image.dng *.jp2\n"
          "           Print all Exif tags in image.dng and all .jp2 files\n")
     << _("   exiv2 -g date/i https://clanmills.com/Stonehenge.jpg\n"
          "           Print all tags in file, where key contains 'date' (case insensitive)\n")
     << _("   exiv2 -M\"set Xmp.dc.subject XmpBag Sky\" image.tiff\n"
          "           Set (or add if missing) value to tag in file\n\n");
}  // Params::help

int Params::option(int opt, const std::string& optArg, int optOpt) {
  int rc = 0;
  switch (opt) {
    case 'h':
      help_ = true;
      break;
    case 'V':
      version_ = true;
      break;
    case 'v':
      verbose_ = true;
      break;
    case 'q':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);
      break;
    case 'Q':
      rc = setLogLevel(optArg);
      break;
    case 'k':
      preserve_ = true;
      break;
    case 'b':
      binary_ = true;
      break;
    case 'u':
      unknown_ = false;
      break;
    case 'f':
      force_ = true;
      fileExistsPolicy_ = overwritePolicy;
      break;
    case 'F':
      force_ = true;
      fileExistsPolicy_ = renamePolicy;
      break;
    case 'g':
      rc = evalGrep(optArg);
      break;
    case 'K':
      rc = evalKey(optArg);
      printMode_ = pmList;
      break;
    case 'n':
      charset_ = optArg;
      break;
    case 'r':
      rc = evalRename(opt, optArg);
      break;
    case 't':
      rc = evalRename(opt, optArg);
      break;
    case 'T':
      rc = evalRename(opt, optArg);
      break;
    case 'a':
      rc = evalAdjust(optArg);
      break;
    case 'Y':
      rc = evalYodAdjust(yodYear, optArg);
      break;
    case 'O':
      rc = evalYodAdjust(yodMonth, optArg);
      break;
    case 'D':
      rc = evalYodAdjust(yodDay, optArg);
      break;
    case 'p':
      rc = evalPrint(optArg);
      break;
    case 'P':
      rc = evalPrintFlags(optArg);
      break;
    case 'd':
      rc = evalDelete(optArg);
      break;
    case 'e':
      rc = evalExtract(optArg);
      break;
    case 'C':
      rc = evalExtract(optArg);
      break;
    case 'i':
      rc = evalInsert(optArg);
      break;
    case 'c':
      rc = evalModify(opt, optArg);
      break;
    case 'm':
      rc = evalModify(opt, optArg);
      break;
    case 'M':
      rc = evalModify(opt, optArg);
      break;
    case 'l':
      directory_ = optArg;
      break;
    case 'S':
      suffix_ = optArg;
      break;
    case ':':
      std::cerr << progname() << ": " << _("Option") << " -" << static_cast<char>(optOpt) << " "
                << _("requires an argument\n");
      rc = 1;
      break;
    case '?':
      std::cerr << progname() << ": " << _("Unrecognized option") << " -" << static_cast<char>(optOpt) << "\n";
      rc = 1;
      break;
    default:
      std::cerr << progname() << ": " << _("getopt returned unexpected character code") << " " << std::hex << opt
                << "\n";
      rc = 1;
      break;
  }
  return rc;
}  // Params::option

int Params::setLogLevel(const std::string& optArg) {
  int rc = 0;
  const auto logLevel = static_cast<char>(tolower(optArg[0]));
  switch (logLevel) {
    case 'd':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::debug);
      break;
    case 'i':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::info);
      break;
    case 'w':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::warn);
      break;
    case 'e':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::error);
      break;
    case 'm':
      Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);
      break;
    default:
      std::cerr << progname() << ": " << _("Option") << " -Q: " << _("Invalid argument") << " \"" << optArg << "\"\n";
      rc = 1;
      break;
  }
  return rc;
}  // Params::setLogLevel

int Params::evalGrep(const std::string& optArg) {
  // check that string ends in "/i"
  bool bIgnoreCase = optArg.size() > 2 && optArg.back() == 'i' && optArg[optArg.size() - 2] == '/';
  auto pattern = bIgnoreCase ? optArg.substr(0, optArg.size() - 2) : optArg;

  try {
    // use POSIX syntax, optimize for faster matching, treat all sub expressions as unnamed
    auto flags = std::regex::basic | std::regex::optimize | std::regex::nosubs;
    flags = bIgnoreCase ? flags | std::regex::icase : flags;
    // try and emplace regex into vector
    // might throw if invalid pattern
    greps_.emplace_back(pattern, flags);
  } catch (std::regex_error const&) {
    // there was an error compiling the regexp
    std::cerr << progname() << ": " << _("Option") << " -g: " << _("Invalid regexp") << " \"" << optArg << "\n";
    return 1;
  }

  return 0;
}  // Params::evalGrep

int Params::evalKey(const std::string& optArg) {
  int result = 0;
  keys_.push_back(optArg);
  return result;
}  // Params::evalKey

int Params::evalRename(int opt, const std::string& optArg) {
  int rc = 0;
  switch (action_) {
    case Action::none:
      action_ = Action::rename;
      switch (opt) {
        case 'r':
          format_ = optArg;
          formatSet_ = true;
          break;
        case 't':
          timestamp_ = true;
          break;
        case 'T':
          timestampOnly_ = true;
          break;
      }
      break;
    case Action::rename:
      if (opt == 'r' && (formatSet_ || timestampOnly_)) {
        std::cerr << progname() << ": " << _("Ignoring surplus option") << " -r \"" << optArg << "\"\n";
      } else {
        format_ = optArg;
        formatSet_ = true;
      }
      break;
    default:
      std::cerr << progname() << ": " << _("Option") << " -" << static_cast<char>(opt) << " "
                << _("is not compatible with a previous option\n");
      rc = 1;
      break;
  }
  return rc;
}  // Params::evalRename

int Params::evalAdjust(const std::string& optArg) {
  int rc = 0;
  switch (action_) {
    case Action::none:
    case Action::adjust:
      if (adjust_) {
        std::cerr << progname() << ": " << _("Ignoring surplus option -a") << " " << optArg << "\n";
        break;
      }
      action_ = Action::adjust;
      adjust_ = parseTime(optArg, adjustment_);
      if (!adjust_) {
        std::cerr << progname() << ": " << _("Error parsing -a option argument") << " `" << optArg << "'\n";
        rc = 1;
      }
      break;
    default:
      std::cerr << progname() << ": " << _("Option -a is not compatible with a previous option\n");
      rc = 1;
      break;
  }
  return rc;
}  // Params::evalAdjust

int Params::evalYodAdjust(const Yod& yod, const std::string& optArg) {
  int rc = 0;
  switch (action_) {
    case Action::none:
    case Action::adjust:
      if (yodAdjust_[yod].flag_) {
        std::cerr << progname() << ": " << _("Ignoring surplus option") << " " << yodAdjust_[yod].option_ << " "
                  << optArg << "\n";
        break;
      }
      action_ = Action::adjust;
      yodAdjust_[yod].flag_ = true;
      if (!Util::strtol(optArg.c_str(), yodAdjust_[yod].adjustment_)) {
        std::cerr << progname() << ": " << _("Error parsing") << " " << yodAdjust_[yod].option_ << " "
                  << _("option argument") << " `" << optArg << "'\n";
        rc = 1;
      }
      break;
    default:
      std::cerr << progname() << ": " << _("Option") << " " << yodAdjust_[yod].option_ << " "
                << _("is not compatible with a previous option\n");
      rc = 1;
      break;
  }
  return rc;
}  // Params::evalYodAdjust

int Params::evalPrint(const std::string& optArg) {
  int rc = 0;
  switch (action_) {
    case Action::none:
      switch (optArg[0]) {
        case 's':
          action_ = Action::print;
          printMode_ = pmSummary;
          break;
        case 'a':
          rc = evalPrintFlags("kyct");
          break;
        case 'e':
          rc = evalPrintFlags("Ekycv");
          break;
        case 't':
          rc = evalPrintFlags("Ekyct");
          break;
        case 'v':
          rc = evalPrintFlags("Exgnycv");
          break;
        case 'h':
          rc = evalPrintFlags("Exgnycsh");
          break;
        case 'i':
          rc = evalPrintFlags("Ikyct");
          break;
        case 'x':
          rc = evalPrintFlags("Xkyct");
          break;
        case 'c':
          action_ = Action::print;
          printMode_ = pmComment;
          break;
        case 'p':
          action_ = Action::print;
          printMode_ = pmPreview;
          break;
        case 'C':
          action_ = Action::print;
          printMode_ = pmIccProfile;
          break;
        case 'R':
#ifdef NDEBUG
          std::cerr << progname() << ": " << _("Action not available in Release mode") << ": '" << optArg << "'\n";
          rc = 1;
#else
          action_ = Action::print;
          printMode_ = pmRecursive;
#endif
          break;
        case 'S':
          action_ = Action::print;
          printMode_ = pmStructure;
          break;
        case 'X':
          action_ = Action::print;
          printMode_ = pmXMP;
          break;
        default:
          std::cerr << progname() << ": " << _("Unrecognized print mode") << " `" << optArg << "'\n";
          rc = 1;
          break;
      }
      break;
    case Action::print:
      std::cerr << progname() << ": " << _("Ignoring surplus option -p") << optArg << "\n";
      break;
    default:
      std::cerr << progname() << ": " << _("Option -p is not compatible with a previous option\n");
      rc = 1;
      break;
  }
  return rc;
}  // Params::evalPrint

int Params::evalPrintFlags(const std::string& optArg) {
  int rc = 0;
  switch (action_) {
    case Action::none:
      action_ = Action::print;
      printMode_ = pmList;
      for (auto&& i : optArg) {
        switch (i) {
          case 'E':
            printTags_ |= MetadataId::exif;
            break;
          case 'I':
            printTags_ |= MetadataId::iptc;
            break;
          case 'X':
            printTags_ |= MetadataId::xmp;
            break;
          case 'x':
            printItems_ |= prTag;
            break;
          case 'g':
            printItems_ |= prGroup;
            break;
          case 'k':
            printItems_ |= prKey;
            break;
          case 'l':
            printItems_ |= prLabel;
            break;
          case 'n':
            printItems_ |= prName;
            break;
          case 'y':
            printItems_ |= prType;
            break;
          case 'c':
            printItems_ |= prCount;
            break;
          case 's':
            printItems_ |= prSize;
            break;
          case 'v':
            printItems_ |= prValue;
            break;
          case 't':
            printItems_ |= prTrans;
            break;
          case 'h':
            printItems_ |= prHex;
            break;
          case 'V':
            printItems_ |= prSet | prKey | prType | prValue;
            break;
          case 'd':
            printItems_ |= prDesc;
            break;
          default:
            std::cerr << progname() << ": " << _("Unrecognized print item") << " `" << i << "'\n";
            rc = 1;
            break;
        }
      }
      break;
    case Action::print:
      std::cerr << progname() << ": " << _("Ignoring surplus option -P") << optArg << "\n";
      break;
    default:
      std::cerr << progname() << ": " << _("Option -P is not compatible with a previous option\n");
      rc = 1;
      break;
  }
  return rc;
}  // Params::evalPrintFlags

int Params::evalDelete(const std::string& optArg) {
  switch (action_) {
    case Action::none:
      action_ = Action::erase;
      target_ = static_cast<CommonTarget>(0);
      [[fallthrough]];
    case Action::erase: {
      const auto rc = parseCommonTargets(optArg, "erase");
      if (rc > 0) {
        target_ |= static_cast<CommonTarget>(rc);
        return 0;
      }
      return 1;
    }
    default:
      std::cerr << progname() << ": " << _("Option -d is not compatible with a previous option\n");
      return 1;
  }
}  // Params::evalDelete

int Params::evalExtract(const std::string& optArg) {
  switch (action_) {
    case Action::none:
    case Action::modify:
      action_ = Action::extract;
      target_ = static_cast<CommonTarget>(0);
      [[fallthrough]];
    case Action::extract: {
      const auto rc = parseCommonTargets(optArg, "extract");
      if (rc > 0) {
        target_ |= static_cast<CommonTarget>(rc);
        return 0;
      }
      return 1;
    }
    default:
      std::cerr << progname() << ": " << _("Option -e is not compatible with a previous option\n");
      return 1;
  }
}  // Params::evalExtract

int Params::evalInsert(const std::string& optArg) {
  switch (action_) {
    case Action::none:
    case Action::modify:
      action_ = Action::insert;
      target_ = static_cast<CommonTarget>(0);
      [[fallthrough]];
    case Action::insert: {
      const auto rc = parseCommonTargets(optArg, "insert");
      if (rc > 0) {
        target_ |= static_cast<CommonTarget>(rc);
        return 0;
      }
      return 1;
    }
    default:
      std::cerr << progname() << ": " << _("Option -i is not compatible with a previous option\n");
      return 1;
  }
}  // Params::evalInsert

int Params::evalModify(int opt, const std::string& optArg) {
  switch (action_) {
    case Action::none:
      action_ = Action::modify;
      [[fallthrough]];
    case Action::modify:
    case Action::extract:
    case Action::insert:
      if (opt == 'c')
        jpegComment_ = parseEscapes(optArg);
      if (opt == 'm')
        cmdFiles_.push_back(optArg);  // parse the files later
      if (opt == 'M')
        cmdLines_.push_back(optArg);  // parse the commands later
      return 0;
    default:
      std::cerr << progname() << ": " << _("Option") << " -" << static_cast<char>(opt) << " "
                << _("is not compatible with a previous option\n");
      return 1;
  }
}  // Params::evalModify

int Params::nonoption(const std::string& argv) {
  int rc = 0;
  bool action = false;
  if (first_) {
    // The first non-option argument must be the action
    first_ = false;
    if (argv == "ad" || argv == "adjust") {
      if (action_ != Action::none && action_ != Action::adjust) {
        std::cerr << progname() << ": " << _("Action adjust is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::adjust;
    }
    if (argv == "pr" || argv == "print") {
      if (action_ != Action::none && action_ != Action::print) {
        std::cerr << progname() << ": " << _("Action print is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::print;
    }
    if (argv == "rm" || argv == "delete") {
      if (action_ != Action::none && action_ != Action::erase) {
        std::cerr << progname() << ": " << _("Action delete is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::erase;
    }
    if (argv == "ex" || argv == "extract") {
      if (action_ != Action::none && action_ != Action::extract && action_ != Action::modify) {
        std::cerr << progname() << ": " << _("Action extract is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::extract;
    }
    if (argv == "in" || argv == "insert") {
      if (action_ != Action::none && action_ != Action::insert && action_ != Action::modify) {
        std::cerr << progname() << ": " << _("Action insert is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::insert;
    }
    if (argv == "mv" || argv == "rename") {
      if (action_ != Action::none && action_ != Action::rename) {
        std::cerr << progname() << ": " << _("Action rename is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::rename;
    }
    if (argv == "mo" || argv == "modify") {
      if (action_ != Action::none && action_ != Action::modify) {
        std::cerr << progname() << ": " << _("Action modify is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::modify;
    }
    if (argv == "fi" || argv == "fixiso") {
      if (action_ != Action::none && action_ != Action::fixiso) {
        std::cerr << progname() << ": " << _("Action fixiso is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::fixiso;
    }
    if (argv == "fc" || argv == "fixcom" || argv == "fixcomment") {
      if (action_ != Action::none && action_ != Action::fixcom) {
        std::cerr << progname() << ": " << _("Action fixcom is not compatible with the given options\n");
        rc = 1;
      }
      action = true;
      action_ = Action::fixcom;
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
}  // Params::nonoption

static size_t readFileToBuf(FILE* f, Exiv2::DataBuf& buf) {
  const int buff_size = 4 * 1028;
  std::vector<Exiv2::byte> bytes(buff_size);
  size_t nBytes = 0;
  bool more{true};
  std::array<char, buff_size> buff;
  while (more) {
    auto n = fread(buff.data(), 1, buff_size, f);
    more = n > 0;
    if (more) {
      bytes.resize(nBytes + n);
      std::copy_n(buff.begin(), n, bytes.begin() + nBytes);
      nBytes += n;
    }
  }

  if (nBytes) {
    buf.alloc(nBytes);
    std::copy(bytes.begin(), bytes.end(), buf.begin());
  }
  return nBytes;
}

void Params::getStdin(Exiv2::DataBuf& buf) {
  // copy stdin to stdinBuf
  if (stdinBuf.empty()) {
#if defined(_WIN32)
    DWORD fdwMode;
    _setmode(fileno(stdin), O_BINARY);
    Sleep(300);
    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &fdwMode)) {  // failed: stdin has bytes!
#else
    // http://stackoverflow.com/questions/34479795/make-c-not-wait-for-user-input/34479916#34479916
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    timeval timeout = {1, 0};  // yes: set timeout seconds,microseconds

    // if we have something in the pipe, read it
    if (select(1, &readfds, nullptr, nullptr, &timeout)) {
#endif
#ifdef DEBUG
      std::cerr << "stdin has data" << '\n';
#endif
      readFileToBuf(stdin, stdinBuf);
    }
#ifdef DEBUG
    // this is only used to simulate reading from stdin when debugging
    // to simulate exiv2 -pX foo.jpg                | exiv2 -iXX- bar.jpg
    //             exiv2 -pX foo.jpg > ~/temp/stdin ; exiv2 -iXX- bar.jpg
    if (stdinBuf.empty()) {
      const char* path = "/Users/rmills/temp/stdin";
      FILE* f = fopen(path, "rb");
      if (f) {
        readFileToBuf(f, stdinBuf);
        fclose(f);
        std::cerr << "read stdin from " << path << '\n';
      }
    }
#endif
#ifdef DEBUG
    std::cerr << "getStdin stdinBuf.size_ = " << stdinBuf.size() << '\n';
#endif
  }

  // copy stdinBuf to buf
  if (!stdinBuf.empty()) {
    buf.alloc(stdinBuf.size());
    std::copy(stdinBuf.begin(), stdinBuf.end(), buf.begin());
  }
#ifdef DEBUG
  std::cerr << "getStdin stdinBuf.size_ = " << stdinBuf.size() << '\n';
#endif

}  // Params::getStdin()

int Params::getopt(int argc, char* const Argv[]) {
  std::vector<char*> argv(argc + 1);
  argv[argc] = nullptr;

  const std::unordered_map<std::string, std::string> longs{
      {"--adjust", "-a"},    {"--binary", "-b"},  {"--comment", "-c"}, {"--delete", "-d"},   {"--days", "-D"},
      {"--extract", "-e"},   {"--force", "-f"},   {"--Force", "-F"},   {"--grep", "-g"},     {"--help", "-h"},
      {"--insert", "-i"},    {"--keep", "-k"},    {"--key", "-K"},     {"--location", "-l"}, {"--modify", "-m"},
      {"--Modify", "-M"},    {"--encode", "-n"},  {"--months", "-O"},  {"--print", "-p"},    {"--Print", "-P"},
      {"--quiet", "-q"},     {"--log", "-Q"},     {"--rename", "-r"},  {"--suffix", "-S"},   {"--timestamp", "-t"},
      {"--Timestamp", "-T"}, {"--unknown", "-u"}, {"--verbose", "-v"}, {"--Version", "-V"},  {"--version", "-V"},
      {"--years", "-Y"},
  };

  for (int i = 0; i < argc; i++) {
    std::string arg(Argv[i]);
    if (longs.find(arg) != longs.end()) {
      argv[i] = ::strdup(longs.at(arg).c_str());
    } else {
      argv[i] = ::strdup(Argv[i]);
    }
  }

  int rc = Util::Getopt::getopt(argc, argv.data(), optstring_);
  // Further consistency checks
  if (help_ || version_) {
    goto cleanup;
  }
  if (action_ == Action::none) {
    // This shouldn't happen since print is taken as default action
    std::cerr << progname() << ": " << _("An action must be specified\n");
    rc = 1;
  }
  if (action_ == Action::adjust && !adjust_ && !yodAdjust_[yodYear].flag_ && !yodAdjust_[yodMonth].flag_ &&
      !yodAdjust_[yodDay].flag_) {
    std::cerr << progname() << ": " << _("Adjust action requires at least one -a, -Y, -O or -D option\n");
    rc = 1;
  }
  if (action_ == Action::modify && cmdFiles_.empty() && cmdLines_.empty() && jpegComment_.empty()) {
    std::cerr << progname() << ": " << _("Modify action requires at least one -c, -m or -M option\n");
    rc = 1;
  }
  if (files_.empty()) {
    std::cerr << progname() << ": " << _("At least one file is required\n");
    rc = 1;
  }
  // Parse command files
  if (rc == 0 && !cmdFiles_.empty() && !parseCmdFiles(modifyCmds_, cmdFiles_)) {
    std::cerr << progname() << ": " << _("Error parsing -m option arguments\n");
    rc = 1;
  }
  // Parse command lines
  if (rc == 0 && !cmdLines_.empty() && !parseCmdLines(modifyCmds_, cmdLines_)) {
    std::cerr << progname() << ": " << _("Error parsing -M option arguments\n");
    rc = 1;
  }
  if (rc == 0 && (!cmdFiles_.empty() || !cmdLines_.empty())) {
    // We'll set them again, after reading the file
    Exiv2::XmpProperties::unregisterNs();
  }
  if (!directory_.empty() && action_ != Action::insert && action_ != Action::extract) {
    std::cerr << progname() << ": " << _("-l option can only be used with extract or insert actions\n");
    rc = 1;
  }
  if (!suffix_.empty() && action_ != Action::insert) {
    std::cerr << progname() << ": " << _("-S option can only be used with insert action\n");
    rc = 1;
  }
  if (timestamp_ && action_ != Action::rename) {
    std::cerr << progname() << ": " << _("-t option can only be used with rename action\n");
    rc = 1;
  }
  if (timestampOnly_ && action_ != Action::rename) {
    std::cerr << progname() << ": " << _("-T option can only be used with rename action\n");
    rc = 1;
  }

cleanup:
  // cleanup the argument vector
  for (int i = 0; i < argc; i++)
    ::free(argv[i]);

  return rc;
}

// *****************************************************************************
// local implementations
namespace {
bool parseTime(const std::string& ts, int64_t& time) {
  std::istringstream sts(ts);
  int sign = 1;
  int64_t hh = 0;
  int64_t mm = 0;
  int64_t ss = 0;

  // [-]HH part
  if (!(sts >> hh))
    return false;
  if (hh < 0) {
    sign = -1;
    hh *= -1;
  }
  // check for the -0 special case
  if (hh == 0 && Exiv2::Internal::contains(ts, '-'))
    sign = -1;
  // MM part, if there is one
  if (sts.peek() == ':') {
    sts.ignore();
    if (!(sts >> mm) || mm > 59 || mm < 0)
      return false;
  }
  // SS part, if there is one
  if (sts.peek() == ':') {
    sts.ignore();
    if (!(sts >> ss) || ss > 59 || ss < 0)
      return false;
  }

  time = sign * (hh * 3600 + mm * 60 + ss);
  return true;
}  // parseTime

void printUnrecognizedArgument(const char argc, const std::string& action) {
  std::cerr << Params::instance().progname() << ": " << _("Unrecognized ") << action << " " << _("target") << " `"
            << argc << "'\n";
}

int64_t parseCommonTargets(const std::string& optArg, const std::string& action) {
  int64_t rc = 0;
  auto target = static_cast<Params::CommonTarget>(0);
  Params::CommonTarget all = Params::ctExif | Params::ctIptc | Params::ctComment | Params::ctXmp;
  Params::CommonTarget extra = Params::ctXmpSidecar | Params::ctExif | Params::ctIptc | Params::ctXmp;
  for (size_t i = 0; rc == 0 && i < optArg.size(); ++i) {
    switch (optArg[i]) {
      case 'e':
        target |= Params::ctExif;
        break;
      case 'i':
        target |= Params::ctIptc;
        break;
      case 'x':
        target |= Params::ctXmp;
        break;
      case 'c':
        target |= Params::ctComment;
        break;
      case 't':
        target |= Params::ctThumb;
        break;
      case 'C':
        target |= Params::ctIccProfile;
        break;
      case 'I':
        target |= Params::ctIptcRaw;
        break;
      case '-':
        target |= Params::ctStdInOut;
        break;
      case 'a':
        target |= all;
        break;
      case 'X':
        target |= extra;  // -eX
        if (i > 0) {      // -eXX or -iXX
          target |= Params::ctXmpRaw;
          target = static_cast<Params::CommonTarget>(target & ~extra);  // turn off those bits
        }
        break;

      case 'p': {
        if (action == "extract") {
          i += static_cast<size_t>(
              parsePreviewNumbers(Params::instance().previewNumbers_, optArg, static_cast<int>(i) + 1));
          target |= Params::ctPreview;
          break;
        }
        printUnrecognizedArgument(optArg[i], action);
        rc = -1;
        break;
      }
      default:
        printUnrecognizedArgument(optArg[i], action);
        rc = -1;
        break;
    }
  }
  return rc ? rc : static_cast<int64_t>(target);
}

int parsePreviewNumbers(Params::PreviewNumbers& previewNumbers, const std::string& optArg, int j) {
  size_t k = j;
  for (size_t i = j; i < optArg.size(); ++i) {
    std::ostringstream os;
    for (k = i; k < optArg.size() && isdigit(optArg[k]); ++k) {
      os << optArg[k];
    }
    if (k > i) {
      bool ok = false;
      auto num = Exiv2::stringTo<int>(os.str(), ok);
      if (ok && num >= 0) {
        previewNumbers.insert(num);
      } else {
        std::cerr << Params::instance().progname() << ": " << _("Invalid preview number") << ": " << num << "\n";
      }
      i = k;
    }
    if (k >= optArg.size() || optArg[i] != ',')
      break;
  }
  auto ret = static_cast<int>(k - j);
  if (ret == 0) {
    previewNumbers.insert(0);
  }
#ifdef DEBUG
  std::cout << "\nThe set now contains: ";
  for (auto&& number : previewNumbers) {
    std::cout << number << ", ";
  }
  std::cout << '\n';
#endif
  return static_cast<int>(k - j);
}  // parsePreviewNumbers

bool parseCmdFiles(ModifyCmds& modifyCmds, const Params::CmdFiles& cmdFiles) {
  for (auto&& filename : cmdFiles) {
    try {
      std::ifstream file(filename.c_str());
      bool bStdin = filename == "-";
      if (!file && !bStdin) {
        std::cerr << filename << ": " << _("Failed to open command file for reading\n");
        return false;
      }
      int num = 0;
      std::string line;
      while (bStdin ? std::getline(std::cin, line) : std::getline(file, line)) {
        ModifyCmd modifyCmd;
        if (parseLine(modifyCmd, line, ++num)) {
          modifyCmds.push_back(std::move(modifyCmd));
        }
      }
    } catch (const Exiv2::Error& error) {
      std::cerr << filename << ", " << _("line") << " " << error << "\n";
      return false;
    }
  }
  return true;
}  // parseCmdFile

bool parseCmdLines(ModifyCmds& modifyCmds, const Params::CmdLines& cmdLines) {
  try {
    int num = 0;
    for (auto&& line : cmdLines) {
      ModifyCmd modifyCmd;
      if (parseLine(modifyCmd, line, ++num)) {
        modifyCmds.push_back(std::move(modifyCmd));
      }
    }
    return true;
  } catch (const Exiv2::Error& error) {
    std::cerr << _("-M option") << " " << error << "\n";
    return false;
  }
}  // parseCmdLines

#ifdef _WIN32
std::string formatArg(const char* arg) {
  std::string result;
  char b = ' ';
  char e = '\\';
  std::string E = std::string("\\");
  char q = '\'';
  std::string Q = std::string("'");
  bool qt = false;
  char* a = const_cast<char*>(arg);
  while (*a) {
    if (*a == b || *a == e || *a == q)
      qt = true;
    if (*a == q)
      result += E;
    if (*a == e)
      result += E;
    result += std::string(a, 1);
    a++;
  }
  if (qt)
    result = Q + result + Q;

  return result;
}
#endif

bool parseLine(ModifyCmd& modifyCmd, const std::string& line, int num) {
  const std::string delim = " \t";

  // Skip empty lines and comments
  std::string::size_type cmdStart = line.find_first_not_of(delim);
  if (cmdStart == std::string::npos || line[cmdStart] == '#')
    return false;

  // Get command and key
  std::string::size_type cmdEnd = line.find_first_of(delim, cmdStart + 1);
  std::string::size_type keyStart = line.find_first_not_of(delim, cmdEnd + 1);
  std::string::size_type keyEnd = line.find_first_of(delim, keyStart + 1);
  if (cmdEnd == std::string::npos || keyStart == std::string::npos) {
    std::string cmdLine;
#ifdef _WIN32
    for (int i = 1; i < __argc; i++) {
      cmdLine += std::string(" ") + formatArg(__argv[i]);
    }
#endif
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                       std::to_string(num) + ": " + _("Invalid command line:") + cmdLine);
  }

  std::string cmd(line.substr(cmdStart, cmdEnd - cmdStart));
  CmdId cmdId = commandId(cmd);
  if (cmdId == CmdId::invalid) {
    throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                       std::to_string(num) + ": " + _("Invalid command") + " `" + cmd + "'");
  }

  Exiv2::TypeId defaultType = Exiv2::invalidTypeId;
  std::string key(line.substr(keyStart, keyEnd - keyStart));
  MetadataId metadataId = MetadataId::invalid;
  if (cmdId != CmdId::reg) {
    try {
      Exiv2::IptcKey iptcKey(key);
      metadataId = MetadataId::iptc;
      defaultType = Exiv2::IptcDataSets::dataSetType(iptcKey.tag(), iptcKey.record());
    } catch (const Exiv2::Error&) {
    }
    if (metadataId == MetadataId::invalid) {
      try {
        Exiv2::ExifKey exifKey(key);
        metadataId = MetadataId::exif;
        defaultType = exifKey.defaultTypeId();
      } catch (const Exiv2::Error&) {
      }
    }
    if (metadataId == MetadataId::invalid) {
      try {
        Exiv2::XmpKey xmpKey(key);
        metadataId = MetadataId::xmp;
        defaultType = Exiv2::XmpProperties::propertyType(xmpKey);
      } catch (const Exiv2::Error&) {
      }
    }
    if (metadataId == MetadataId::invalid) {
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                         std::to_string(num) + ": " + _("Invalid key") + " `" + key + "'");
    }
  }
  std::string value;
  Exiv2::TypeId type = defaultType;
  bool explicitType = false;
  if (cmdId != CmdId::del) {
    // Get type and value
    std::string::size_type typeStart = std::string::npos;
    if (keyEnd != std::string::npos)
      typeStart = line.find_first_not_of(delim, keyEnd + 1);
    std::string::size_type typeEnd = std::string::npos;
    if (typeStart != std::string::npos)
      typeEnd = line.find_first_of(delim, typeStart + 1);
    std::string::size_type valStart = typeStart;
    std::string::size_type valEnd = std::string::npos;
    if (valStart != std::string::npos)
      valEnd = line.find_last_not_of(delim);

    if (cmdId == CmdId::reg && (keyEnd == std::string::npos || valStart == std::string::npos)) {
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                         std::to_string(num) + ": " + _("Invalid command line") + " ");
    }

    if (cmdId != CmdId::reg && typeStart != std::string::npos && typeEnd != std::string::npos) {
      std::string typeStr(line.substr(typeStart, typeEnd - typeStart));
      Exiv2::TypeId tmpType = Exiv2::TypeInfo::typeId(typeStr);
      if (tmpType != Exiv2::invalidTypeId) {
        valStart = line.find_first_not_of(delim, typeEnd + 1);
        if (valStart == std::string::npos) {
          throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                             std::to_string(num) + ": " + _("Invalid command line") + " ");
        }
        type = tmpType;
        explicitType = true;
      }
    }

    if (valStart != std::string::npos) {
      value = parseEscapes(line.substr(valStart, valEnd + 1 - valStart));
      std::string::size_type last = value.length() - 1;
      if ((value.at(0) == '"' && value.at(last) == '"') || (value.at(0) == '\'' && value.at(last) == '\'')) {
        value = value.substr(1, value.length() - 2);
      }
    }
  }

  modifyCmd.cmdId_ = cmdId;
  modifyCmd.key_ = key;
  modifyCmd.metadataId_ = metadataId;
  modifyCmd.typeId_ = type;
  modifyCmd.explicitType_ = explicitType;
  modifyCmd.value_ = value;

  if (cmdId == CmdId::reg) {
    if (value.empty()) {
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage,
                         std::to_string(num) + ": " + _("Empty value for key") + +" `" + key + "'");
    }

    // Registration needs to be done immediately as the new namespaces are
    // looked up during parsing of subsequent lines (to validate XMP keys).
    Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
  }

  return true;
}  // parseLine

CmdId commandId(const std::string& cmdString) {
  if (auto it = Exiv2::find(cmdIdAndString, cmdString))
    return it->cmdId_;
  return CmdId::invalid;
}

std::string parseEscapes(const std::string& input) {
  std::string result;
  for (size_t i = 0; i < input.length(); ++i) {
    char ch = input[i];
    if (ch != '\\') {
      result.push_back(ch);
      continue;
    }
    size_t escapeStart = i;
    if (input.length() - 1 <= i) {
      result.push_back(ch);
      continue;
    }
    ++i;
    ch = input[i];
    switch (ch) {
      case '\\':  // Escaping of backslash
        result.push_back('\\');
        break;
      case 'r':  // Escaping of carriage return
        result.push_back('\r');
        break;
      case 'n':  // Escaping of newline
        result.push_back('\n');
        break;
      case 't':  // Escaping of tab
        result.push_back('\t');
        break;
      case 'u':  // Escaping of unicode
        if (input.length() >= 4 && input.length() - 4 > i) {
          uint32_t acc = 0;
          for (int j = 0; j < 4; ++j) {
            ++i;
            acc <<= 4;
            if (input[i] >= '0' && input[i] <= '9') {
              acc |= input[i] - '0';
            } else if (input[i] >= 'a' && input[i] <= 'f') {
              acc |= input[i] - 'a' + 10;
            } else if (input[i] >= 'A' && input[i] <= 'F') {
              acc |= input[i] - 'A' + 10;
            } else {
              acc = 0xFFFFFFFF;
              break;
            }
          }
          if (acc == 0xFFFFFFFF) {
            result.push_back('\\');
            i = escapeStart;
            break;
          }

          std::string ucs2toUtf8;
          ucs2toUtf8.push_back(static_cast<char>((acc & 0xff00U) >> 8));
          ucs2toUtf8.push_back(static_cast<char>(acc & 0x00ffU));

          if (Exiv2::convertStringCharset(ucs2toUtf8, "UCS-2BE", "UTF-8")) {
            result.append(ucs2toUtf8);
          }
        } else {
          result.push_back('\\');
          result.push_back(ch);
        }
        break;
      default:
        result.push_back('\\');
        result.push_back(ch);
    }
  }
  return result;
}

}  // namespace
