// ***************************************************************** -*- C++ -*-
/*
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
  @file    metacopy.hpp
  @brief   Defines class Params, used for the command line handling
  @author  Brad Schick (brad) <brad@robotbattle.com>
  @date    13-Jul-04, brad: created
 */
#pragma once

class Params : public Util::Getopt {
private:
    std::string optstring_;
    bool first_{true};

  public:
    bool help_{false};             //!< Help option flag.
    bool iptc_{false};             //!< Iptc option flag.
    bool exif_{false};             //!< Exif option flag.
    bool comment_{false};          //!< JPEG comment option flag.
    bool xmp_{false};              //!< XMP option flag.
    bool preserve_{false};         //!< Preserve existing metadata option flag.
    std::string read_;             //!< Source file
    std::string write_;            //!< Destination file

public:
    /*!
      @brief Default constructor. Note that optstring_ is initialized here.
     */
  Params()
      : optstring_(":iecaph")

  {}

  /*!
    @brief Call Getopt::getopt() with optstring, to initiate command line
           argument parsing, perform consistency checks after all command line
           arguments are parsed.

    @param argc Argument count as passed to main() on program invocation.
    @param argv Argument array as passed to main() on program invocation.

    @return 0 if successful, >0 in case of errors.
   */
  int getopt(int argc, char *const argv[]);

  //! Handle options and their arguments.
  int option(int opt, const std::string &optarg, int optopt) override;

  //! Handle non-option parameters.
  int nonoption(const std::string &argv) override;

  //! Print a minimal usage note to an output stream.
  void usage(std::ostream &os = std::cout) const;

  //! Print further usage explanations to an output stream.
  void help(std::ostream &os = std::cout) const;

}; // class Params
