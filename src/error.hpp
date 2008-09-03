// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    error.hpp
  @brief   Error class for exceptions
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component
 */
#ifndef ERROR_HPP_
#define ERROR_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <exception>
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Error class interface. Allows the definition and use of a hierarchy
             of error classes which can all be handled in one catch block.
             Inherits from the standard exception base-class, to make life
             easier for library users (they have the option of catching most
             things via std::exception).
     */
    class EXIV2API AnyError : public std::exception {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~AnyError() throw()
        {
        }
        //@}

        //! @name Accessors
        //@{
        //! Return the error code.
        virtual int code() const throw() =0;
    }; // AnyError

    //! %AnyBase output operator
    inline std::ostream& operator<<(std::ostream& os, const AnyError& error)
    {
        return os << error.what();
    }

    /*!
      @brief Simple error class used for exceptions. An output operator is
             provided to print errors to a stream.
     */
    class EXIV2API Error : public AnyError {
    public:
        //! @name Creators
        //@{
        //! Constructor taking only an error code
        explicit Error(int code)
            : code_(code), count_(0)
        {
            setMsg();
        }
        //! Constructor taking an error code and one argument
        template<typename A>
        Error(int code, const A& arg1)
            : code_(code), count_(1), arg1_(toString(arg1))
        {
            setMsg();
        }
        //! Constructor taking an error code and two arguments
        template<typename A, typename B>
        Error(int code, const A& arg1, const B& arg2)
            : code_(code), count_(2),
              arg1_(toString(arg1)), arg2_(toString(arg2))
        {
            setMsg();
        }
        //! Constructor taking an error code and three arguments
        template<typename A, typename B, typename C>
        Error(int code, const A& arg1, const B& arg2, const C& arg3)
            : code_(code), count_(3),
              arg1_(toString(arg1)), arg2_(toString(arg2)), arg3_(toString(arg3))
        {
            setMsg();
        }
        //! Virtual destructor.
        virtual ~Error() throw()
        {
        }
        //@}

        //! @name Accessors
        //@{
        virtual int code() const throw() { return code_; }
        /*!
          @brief Return the error message. The pointer returned by what()
                 is valid only as long as the Error object exists.
         */
        virtual const char* what() const throw() { return msg_.c_str(); }
        //@}

    private:
        //! @name Manipulators
        //@{
        void setMsg();
        //@}

        static int errorIdx(int code);

        // DATA
        int code_;                              //!< Error code
        int count_;                             //!< Number of arguments
        std::string arg1_;                      //!< First argument
        std::string arg2_;                      //!< Second argument
        std::string arg3_;                      //!< Third argument
        std::string msg_;                       //!< Complete error message

    }; // class Error

}                                       // namespace Exiv2

#endif                                  // #ifndef ERROR_HPP_
