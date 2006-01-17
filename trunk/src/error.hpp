// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! Helper structure defining an error message
    struct ErrMsg {
        //! Constructor
        ErrMsg(int code, const char* message)
            : code_(code), message_(message)
        {
        }
        int code_;                             //!< Error code
        const char* message_;                   //!< Error message
    };

    /*!
      @brief Error class interface. Allows the definition and use of a hierarchy
             of error classes which can all be handled in one catch block.
     */
    class AnyError {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~AnyError()
        {
        }
        //@}

        //! @name Accessors
        //@{
        //! Return the error code.
        virtual int code() const =0;
        /*!
          @brief Return the error message. Consider using the output operator
                 operator<<(std::ostream &os, const AnyError& error) instead.
          @note  Unlike std::exception::what(), this function returns an
                 std::string.
         */
        virtual std::string what() const =0;
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
    class Error : public AnyError {
    public:
        //! @name Creators
        //@{
        //! Constructor taking only an error code
        explicit Error(int code)
            : code_(code), count_(0)
        {
        }
        //! Constructor taking an error code and one argument
        template<typename A>
        Error(int code, const A& arg1)
            : code_(code), count_(1), arg1_(toString(arg1))
        {
        }
        //! Constructor taking an error code and two arguments
        template<typename A, typename B>
        Error(int code, const A& arg1, const B& arg2)
            : code_(code), count_(2),
              arg1_(toString(arg1)), arg2_(toString(arg2))
        {
        }
        //! Constructor taking an error code and three arguments
        template<typename A, typename B, typename C>
        Error(int code, const A& arg1, const B& arg2, const C& arg3)
            : code_(code), count_(3),
              arg1_(toString(arg1)), arg2_(toString(arg2)), arg3_(toString(arg3))
        {
        }
        //@}

        //! @name Accessors
        //@{
        virtual int code() const { return code_; }
        virtual std::string what() const;
        //@}

    private:
        static int errorIdx(int code);

        // DATA
        int code_;                              //!< Error code
        int count_;                             //!< Number of arguments
        std::string arg1_;                      //!< First argument
        std::string arg2_;                      //!< Second argument
        std::string arg3_;                      //!< Third argument

        static const ErrMsg errMsg_[];          //!< List of error messages
    }; // class Error

}                                       // namespace Exiv2

#endif                                  // #ifndef ERROR_HPP_
