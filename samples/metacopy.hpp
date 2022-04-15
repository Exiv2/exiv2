// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef METACOPY_HPP_
#define METACOPY_HPP_

#include "getopt.hpp"

#include <iostream>

class Params : public Util::Getopt {
 private:
  std::string optstring_;
  bool first_{true};

 public:
  bool help_{false};      //!< Help option flag.
  bool iptc_{false};      //!< Iptc option flag.
  bool exif_{false};      //!< Exif option flag.
  bool comment_{false};   //!< JPEG comment option flag.
  bool xmp_{false};       //!< XMP option flag.
  bool preserve_{false};  //!< Preserve existing metadata option flag.
  std::string read_;      //!< Source file
  std::string write_;     //!< Destination file

  /*!
    @brief Default constructor. Note that optstring_ is initialized here.
   */
  Params() : optstring_(":iecxaph") {
  }

  /*!
    @brief Call Getopt::getopt() with optstring, to initiate command line
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

};  // class Params

#endif  // METACOPY_HPP_
