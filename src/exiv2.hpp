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
/*!
  @file    exiv2.hpp
  @brief   Defines class Params, used for the command line handling of exiv2
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    08-Dec-03, ahu: created
 */
#ifndef EXIV2_HPP_
#define EXIV2_HPP_

// *****************************************************************************
// included header files
#include "utils.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iostream>

// *****************************************************************************
// class definitions
/*!
  @brief Implements the command line handling for the program. 

  Derives from Util::Getopt to use the command line argument parsing
  functionalty provided there. This class is implemented as a Singleton,
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
class Params : public Util::Getopt {
private:
    std::string optstring_;

public:
    /*!
      @brief Controls all access to the global Params instance.
      @return Reference to the global Params instance.
    */
    static Params& instance();

    bool help_;                         //!< Help option flag.
    bool version_;                      //!< Version option flag.
    bool verbose_;                      //!< Verbose (talkative) option flag.
    bool force_;                        //!< Force overwrites flag. 
    bool adjust_;                       //!< Adjustment flag.
    //! %Action (integer rather than TaskType to avoid dependency).
    int  action_;                       

    long adjustment_;                   //!< Adjustment in seconds.
    std::string format_;                //!< Filename format (-r option arg).

    //! Container to store filenames.
    typedef std::vector<std::string> Files;

    Files files_;                       //!< List of non-option arguments.

private:
    /*!
      @brief Default constructor. Note that optstring_ is initialized here.
             Private to force instantiation through instance().
     */
    Params() : optstring_(":hVvfa:r:"),
               help_(false), 
               version_(false),
               verbose_(false), 
               force_(false), 
               adjust_(false),
               action_(0),
               adjustment_(0),
               format_("%Y%m%d_%H%M%S"),
               first_(true) {}

    //! Prevent copy-construction: not implemented.
    Params(const Params& rhs);

    //! Pointer to the global Params object.
    static Params* instance_;

    bool first_;

public:
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
    virtual int option(int opt, const std::string& optarg, int optopt);

    //! Handle non-option parameters.
    virtual int nonoption(const std::string& argv);

    //! Print a minimal usage note to an output stream.
    void usage(std::ostream& os =std::cout) const;

    //! Print further usage explanations to an output stream.
    void help(std::ostream& os =std::cout) const;

    //! Print version information to an output stream.
    void version(std::ostream& os =std::cout) const;
}; // class Params

#endif                                  // #ifndef EXIV2_HPP_
