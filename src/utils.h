// ********************************************************* -*- C++ -*-
/*
 * Copyright (c) 2003, 2004 Andreas Huggel. All rights reserved.
 * 
 * This file is part of the Exiv distribution.
 *
 * This file may be distributed and/or modified under the terms of the
 * Common Public License version 1.0 as published by IBM and appearing
 * in the file license-cpl.txt included in the packaging of this file.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation and appearing in the file license-gpl.txt included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*!
  @file    utils.h
  @brief   A collection of utility functions
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
  @date    12-Dec-03, ahu: created
 */
#ifndef _UTILS_H_
#define _UTILS_H_

// *********************************************************************
// included header files
// + standard includes
#include <string>

// *********************************************************************
// namespace extensions
/*!
  @brief Contains utility classes and functions. Most of these are 
         wrappers for common C functions that do not require pointers 
         and memory considerations.
*/
namespace Util {

// *********************************************************************
// class definitions

/*!
  @brief Parse the command line options of a program.

  A wrapper around the POSIX %getopt(3) function.  Parses the command line
  options and passes each option to virtual option().  A derived class
  implements this method to handle options as needed.  Similarly,
  remaining non-option parameters are passed to the virtual nonoption()
  method.
 */
class Getopt {
public:
    //! Default constructor.
    Getopt() : errcnt_(0) {}

    //! Destructor.
    virtual ~Getopt() {}

    /*!
      @brief Parse command line arguments.

      Parses the command line arguments. Calls option() with the
      character value of the option and its argument (if any) for each
      recognized option and with ':' or '?' for unrecognized options.
      See the manual pages for %getopt(3) for details.  In addition,
      nonoption() is invoked for each remaining non-option parameter on
      the command line.

      @param argc Argument count as passed to main() on  program invocation.
      @param argv Argument array as passed to main() on  program invocation.
      @param optstring String containing the legitimate option characters.

      @return Number of errors (the sum of the return values from option() 
              and nonoption()).
     */
    int getopt(int argc, char* const argv[], const std::string& optstring);

    /*!
      @brief Callback used by getopt() to pass on each option and its 
             argument (if any).

      Implement this method in a derived class to handle the options as
      needed. See the manual pages for %getopt(3) for further details, in
      particular, the semantics of optarg and optopt.

      @param opt Value of the option character as returned by %getopt(3).
      @param optarg The corresponding option argument.
      @param optopt The actual option character in case of an unrecognized
             option or a missing option argument (opt is '?' or ':').

      @return 0 if successful, 1 in case of an error.
     */
    virtual int option(int opt, const std::string& optarg, int optopt) = 0;

    /*!
      @brief Callback used by getopt() to pass on each non-option parameter
             found on the command line.

      Implement this method in a derived class to handle the non-option
      parameters as needed. The default implementation ignores all non-option
      parameters.

      @param argv The non-option parameter from the command line.

      @return 0 if successful, 1 in case of an error.
     */
    virtual int nonoption(const std::string& argv) { return 0; }

    //! Program name (argv[0])
    const std::string& progname() const { return progname_; }

    //! Total number of errors returned by calls to option()
    int errcnt() const { return errcnt_; }

private:
    std::string progname_;
    int errcnt_;
};

// *********************************************************************
// free functions

    /*!
      @brief Test if a file exists.
  
      @param  path Name of file to verify.
      @param  ct   Flag to check if <i>path</i> is a regular file.
      @return true if <i>path</i> exists and, if <i>ct</i> is set,
      is a regular file, else false.
  
      @note The function calls <b>stat()</b> test for <i>path</i>
      and its type, see stat(2). <b>errno</b>
      is left unchanged in case of an error.
     */
    bool fileExists(const std::string& path, bool ct =false);

    //! Get a system error message and the error code. See %strerror(2).
    std::string strError();

    //! Get the directory component from the path string. See %dirname(3).
    std::string dirname(const std::string& path);

    /*!
      @brief Get the filename component from the path string. See %basename(3).
             If the delsuffix parameter is true, the suffix will be removed.
     */
    std::string basename(const std::string& path, bool delsuffix =false);

    /*!
      @brief Get the suffix from the path string. Normally, the suffix
             is the substring of the basename of path from the last '.' 
             to the end of the string.
     */
    std::string suffix(const std::string& path);

    /*! 
      @brief Convert a C string to a long value, which is returned in n.
             Returns true if the conversion is successful, else false.
             n is not modified if the conversion is unsuccessful. See strtol(2).
     */
    bool strtol(const char* nptr, long& n);

}                                       // namespace Util

#endif                                  // #ifndef _UTILS_H_
