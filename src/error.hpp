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
  @file    error.hpp
  @brief   Error class for exceptions
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
 */
#ifndef ERROR_HPP_
#define ERROR_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

// *****************************************************************************
// class definitions

    /*!
      @brief Very simple error class used for exceptions. It contains just an
             error message. An output operator is provided to print
             errors to a stream.
     */
    class Error {
    public:
        //! Constructor taking a (short) error message as argument
        Error(const std::string& message) : message_(message) {}
        /*!
          @brief Return the error message. Consider using the output operator
                 operator<<(std::ostream &os, const Error& error) instead.
         */
        std::string message() const { return message_; }
    private:
        std::string message_;
    };

    //! %Error output operator
    inline std::ostream& operator<<(std::ostream& os, const Error& error)
    {
        return os << error.message();
    }

}                                       // namespace Exif

#endif                                  // #ifndef ERROR_HPP_
