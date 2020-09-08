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
/*!
  @file    exiv2app.hpp
  @brief   Defines class Params, used for the command line handling of exiv2
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    08-Dec-03, ahu: created
 */
#pragma once

// *****************************************************************************
//#include <exiv2/exiv2.hpp>
#include <exiv2/version.hpp>

#include "getopt.hpp"
#include "types.hpp"
#include "utils.hpp"

// + standard includes
#include <iostream>
#include <set>
#include <string>
#include <vector>

#ifdef EXV_HAVE_UNISTD_H
#include <unistd.h>
#endif

// *****************************************************************************
// class definitions

//! Command identifiers
enum CmdId
{
    invalidCmdId,
    add,
    set,
    del,
    reg
};
//! Metadata identifiers
// enum MetadataId { invalidMetadataId, iptc, exif, xmp };
//! Metadata identifiers
// mdNone=0, mdExif=1, mdIptc=2, mdComment=4, mdXmp=8
enum MetadataId
{
    invalidMetadataId = Exiv2::mdNone,
    iptc = Exiv2::mdIptc,
    exif = Exiv2::mdExif,
    xmp = Exiv2::mdXmp
};

//! Structure for one parsed modification command
struct ModifyCmd
{
    //! C'tor
    ModifyCmd()
        : cmdId_(invalidCmdId), metadataId_(invalidMetadataId), typeId_(Exiv2::invalidTypeId), explicitType_(false)
    {
    }
    CmdId cmdId_;            //!< Command identifier
    std::string key_;        //!< Exiv2 key string
    MetadataId metadataId_;  //!< Metadata identifier
    Exiv2::TypeId typeId_;   //!< Exiv2 type identifier
    //! Flag to indicate if the type was explicitly specified (true)
    bool explicitType_;
    std::string value_;  //!< Data
};
//! Container for modification commands
typedef std::vector<ModifyCmd> ModifyCmds;
//! Structure to link command identifiers to strings
struct CmdIdAndString
{
    CmdId cmdId_;            //!< Commands identifier
    std::string cmdString_;  //!< Command string
};

/*!
  @brief Implements the command line handling for the program.

  Derives from Util::Getopt to use the command line argument parsing
  functionality provided there. This class is implemented as a singleton,
  i.e., there is only one global instance of it, which can be accessed
  from everywhere.

  <b>Usage example:</b> <br>
  @code
  #include "params.h"

  int main(int argc, char* const argv[])
  {
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

      // do something useful here...

      return 0;
  }
  @endcode
 */

class Params : public Util::Getopt
{
public:
    Params& operator=(const Params& rhs) = delete;
    Params& operator=(const Params&& rhs) = delete;
    Params(const Params& rhs) = delete;
    Params(const Params&& rhs) = delete;

    //! Container for command files
    typedef std::vector<std::string> CmdFiles;
    //! Container for commands from the command line
    typedef std::vector<std::string> CmdLines;
    //! Container to store filenames.
    typedef std::vector<std::string> Files;
    //! Container for preview image numbers
    typedef std::set<int> PreviewNumbers;
    //! Container for greps
    typedef exv_grep_keys_t Greps;
    //! Container for keys
    typedef std::vector<std::string> Keys;

    /*!
      @brief Controls all access to the global Params instance.
      @return Reference to the global Params instance.
    */
    static Params& instance();

    //! Enumerates print modes
    enum PrintMode
    {
        pmSummary,
        pmList,
        pmComment,
        pmPreview,
        pmStructure,
        pmXMP,
        pmIccProfile,
        pmRecursive
    };

    //! Individual items to print, bitmap
    enum PrintItem
    {
        prTag = 1,
        prGroup = 2,
        prKey = 4,
        prName = 8,
        prLabel = 16,
        prType = 32,
        prCount = 64,
        prSize = 128,
        prValue = 256,
        prTrans = 512,
        prHex = 1024,
        prSet = 2048
    };

    //! Enumerates common targets, bitmap
    enum CommonTarget
    {
        ctExif = 1,
        ctIptc = 2,
        ctComment = 4,
        ctThumb = 8,
        ctXmp = 16,
        ctXmpSidecar = 32,
        ctPreview = 64,
        ctIccProfile = 128,
        ctXmpRaw = 256,
        ctStdInOut = 512,
        ctIptcRaw = 1024
    };

    //! Enumerates the policies to handle existing files in rename action
    enum FileExistsPolicy
    {
        overwritePolicy,
        renamePolicy,
        askPolicy,
    };

    //! Enumerates year, month and day adjustments.
    enum Yod
    {
        yodYear,
        yodMonth,
        yodDay,
    };

    //! Structure for year, month and day adjustment command line arguments.
    struct YodAdjust
    {
        bool flag_;           //!< Adjustment flag.
        const char* option_;  //!< Adjustment option string.
        long adjustment_;     //!< Adjustment value.
    };

    /*!
      @brief Call Getopt::getopt() with optstring, to inititate command line
             argument parsing, perform consistency checks after all command line
             arguments are parsed.

      @param argc Argument count as passed to main() on program invocation.
      @param argv Argument array as passed to main() on program invocation.

      @return 0 if successful, >0 in case of errors.
     */
    int getopt(int argc, char* const argv[]);

    //! Handle options and their arguments.
    int option(int opt, const std::string& optarg, int optopt) override;

    //! Handle non-option parameters.
    int nonoption(const std::string& argv) override;

    //! Print a minimal usage note to an output stream.
    void usage(std::ostream& os = std::cout) const;

    //! Print further usage explanations to an output stream.
    void help(std::ostream& os = std::cout) const;

    //! Print version information to an output stream.
    void version(bool verbose = false, std::ostream& os = std::cout) const;

    //! getStdin binary data read from stdin to DataBuf
    /*
        stdin can be used by multiple images in the exiv2 command line:
        For example: $ cat foo.icc | exiv2 -iC- a.jpg b.jpg c.jpg will modify the ICC profile in several images.
    */
    void getStdin(Exiv2::DataBuf& buf);

private:
    /*!
      @brief Default constructor. Note that optstring_ is initialized here.
             The c'tor is private to force instantiation through instance().
     */
    Params();

    //! Destructor, frees any allocated regexes in greps_
    ~Params() override;

    //! @name Helpers
    //@{
    int setLogLevel(const std::string& optarg);
    int evalGrep(const std::string& optarg);
    int evalKey(const std::string& optarg);
    int evalRename(int opt, const std::string& optarg);
    int evalAdjust(const std::string& optarg);
    int evalYodAdjust(const Yod& yod, const std::string& optarg);
    int evalPrint(const std::string& optarg);
    int evalPrintFlags(const std::string& optarg);
    int evalDelete(const std::string& optarg);
    int evalExtract(const std::string& optarg);
    int evalInsert(const std::string& optarg);
    int evalModify(int opt, const std::string& optarg);
    //@}

    std::string optstring_;
    bool first_;

public:
    bool help_;                          //!< Help option flag.
    bool version_;                       //!< Version option flag.
    bool verbose_;                       //!< Verbose (talkative) option flag.
    bool force_;                         //!< Force overwrites flag.
    bool binary_;                        //!< Suppress long binary values.
    bool unknown_;                       //!< Suppress unknown tags.
    bool preserve_;                      //!< Preserve timestamps flag.
    bool timestamp_;                     //!< Rename also sets the file timestamp.
    bool timestampOnly_;                 //!< Rename only sets the file timestamp.
    FileExistsPolicy fileExistsPolicy_;  //!< What to do if file to rename exists.
    bool adjust_;                        //!< Adjustment flag.
    PrintMode printMode_;                //!< Print mode.
    unsigned long printItems_;           //!< Print items.
    unsigned long printTags_;            //!< Print tags (bitmap of MetadataId flags).
    //! %Action (integer rather than TaskType to avoid dependency).
    int action_;
    int target_;                         //!< What common target to process.

    long adjustment_;                    //!< Adjustment in seconds.
    YodAdjust yodAdjust_[3];             //!< Year, month and day adjustment info.
    std::string format_;                 //!< Filename format (-r option arg).
    bool formatSet_;                     //!< Whether the format is set with -r
    CmdFiles cmdFiles_;                  //!< Names of the modification command files
    CmdLines cmdLines_;                  //!< Commands from the command line
    ModifyCmds modifyCmds_;              //!< Parsed modification commands
    std::string jpegComment_;            //!< Jpeg comment to set in the image
    std::string directory_;              //!< Location for files to extract/insert
    std::string suffix_;                 //!< File extension of the file to insert
    Files files_;                        //!< List of non-option arguments.
    PreviewNumbers previewNumbers_;      //!< List of preview numbers
    Greps greps_;                        //!< List of keys to 'grep' from the metadata
    Keys keys_;                          //!< List of keys to match from the metadata
    std::string charset_;                //!< Charset to use for UNICODE Exif user comment

    Exiv2::DataBuf stdinBuf;             //!< DataBuf with the binary bytes from stdin
};                            // class Params
