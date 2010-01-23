// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
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

#ifdef _MSC_VER
// Disable MSVC warnings "non - DLL-interface classkey 'identifier' used as base
// for DLL-interface classkey 'identifier'"
# pragma warning( disable : 4275 )
#endif

    //! Generalised toString function
    template<typename charT, typename T>
    std::basic_string<charT> toBasicString(const T& arg)
    {
        std::basic_ostringstream<charT> os;
        os << arg;
        return os.str();
    }

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
        virtual ~AnyError() throw();
        //@}

        //! @name Accessors
        //@{
        //! Return the error code.
        virtual int code() const throw() =0;
        //@}
    }; // AnyError

    //! %AnyError output operator
    inline std::ostream& operator<<(std::ostream& os, const AnyError& error)
    {
        return os << error.what();
    }

    /*!
      @brief Simple error class used for exceptions. An output operator is
             provided to print errors to a stream.
     */
    template<typename charT>
    class EXV_DLLPUBLIC BasicError : public AnyError {
    public:
        //! @name Creators
        //@{
        //! Constructor taking only an error code
        EXV_DLLLOCAL explicit BasicError(int code);
        //! Constructor taking an error code and one argument
        template<typename A>
        EXV_DLLLOCAL BasicError(int code, const A& arg1);
        //! Constructor taking an error code and two arguments
        template<typename A, typename B>
        EXV_DLLLOCAL BasicError(int code, const A& arg1, const B& arg2);
        //! Constructor taking an error code and three arguments
        template<typename A, typename B, typename C>
        EXV_DLLLOCAL BasicError(int code, const A& arg1, const B& arg2, const C& arg3);
        //! Virtual destructor. (Needed because of throw())
        EXV_DLLLOCAL virtual ~BasicError() throw();
        //@}

        //! @name Accessors
        //@{
        EXV_DLLLOCAL virtual int code() const throw();
        /*!
          @brief Return the error message as a C-string. The pointer returned by what()
                 is valid only as long as the BasicError object exists.
         */
        EXIV2API virtual const char* what() const throw();
        /*!
          @brief Return the error message as a wchar_t-string. The pointer returned by
                 wwhat() is valid only as long as the BasicError object exists.
         */
        EXIV2API virtual const wchar_t* wwhat() const throw();
        //@}

    private:
        //! @name Manipulators
        //@{
        EXV_DLLLOCAL void setMsg();
        //@}

        // DATA
        int code_;                              //!< Error code
        int count_;                             //!< Number of arguments
        std::basic_string<charT> arg1_;         //!< First argument
        std::basic_string<charT> arg2_;         //!< Second argument
        std::basic_string<charT> arg3_;         //!< Third argument
        std::basic_string<charT> msg_;          //!< Complete error message

    }; // class BasicError

    typedef BasicError<char> Error;
    typedef BasicError<wchar_t> WError;

// *****************************************************************************
// free functions, template and inline definitions

    //! Return the error message for the error with code \em code.
    EXIV2API const char* errMsg(int code);

    template<typename charT>
    BasicError<charT>::BasicError(int code)
        : code_(code), count_(0)
    {
        setMsg();
    }

    template<typename charT>
    BasicError<charT>::~BasicError() throw()
    {
    }

    template<typename charT>
    int BasicError<charT>::code() const throw()
    {
        return code_;
    }

    template<typename charT> template<typename A>
    BasicError<charT>::BasicError(int code, const A& arg1)
        : code_(code), count_(1), arg1_(toBasicString<charT>(arg1))
    {
        setMsg();
    }

    template<typename charT> template<typename A, typename B>
    BasicError<charT>::BasicError(int code, const A& arg1, const B& arg2)
        : code_(code), count_(2),
          arg1_(toBasicString<charT>(arg1)),
          arg2_(toBasicString<charT>(arg2))
    {
        setMsg();
    }

    template<typename charT> template<typename A, typename B, typename C>
    BasicError<charT>::BasicError(int code, const A& arg1, const B& arg2, const C& arg3)
        : code_(code), count_(3),
          arg1_(toBasicString<charT>(arg1)),
          arg2_(toBasicString<charT>(arg2)),
          arg3_(toBasicString<charT>(arg3))
    {
        setMsg();
    }

    template<typename charT>
    void BasicError<charT>::setMsg()
    {
        std::string s(exvGettext(errMsg(code_)));
        msg_.assign(s.begin(), s.end());
        std::string ph("%0");
        std::basic_string<charT> tph(ph.begin(), ph.end());
        size_t pos = msg_.find(tph);
        if (pos != std::basic_string<charT>::npos) {
            msg_.replace(pos, 2, toBasicString<charT>(code_));
        }
        if (count_ > 0) {
            ph = "%1";
            tph.assign(ph.begin(), ph.end());
            pos = msg_.find(tph);
            if (pos != std::basic_string<charT>::npos) {
                msg_.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            ph = "%2";
            tph.assign(ph.begin(), ph.end());
            pos = msg_.find(tph);
            if (pos != std::basic_string<charT>::npos) {
                msg_.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            ph = "%3";
            tph.assign(ph.begin(), ph.end());
            pos = msg_.find(tph);
            if (pos != std::basic_string<charT>::npos) {
                msg_.replace(pos, 2, arg3_);
            }
        }
    }

#ifdef _MSC_VER
# pragma warning( default : 4275 )
#endif

}                                       // namespace Exiv2
#endif                                  // #ifndef ERROR_HPP_
