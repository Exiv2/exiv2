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
  @file    canonmn.hpp
  @brief   Canon MakerNote implemented according to the specification
           "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
           by David Burren
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created
           07-Mar-04, ahu: isolated as a separate component
 */
#ifndef CANONMN_HPP_
#define CANONMN_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "makernote.hpp"

// + standard includes
#include <string>
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

    class Value;

// *****************************************************************************
// class definitions

    //! MakerNote for Canon cameras
    class CanonMakerNote : public IfdMakerNote {
    public:
        //! @name Creators
        //@{        
        //! Default constructor
        CanonMakerNote();
        //! Virtual destructor
        virtual ~CanonMakerNote() {}
        //@}

        //! @name Accessors
        //@{        
        MakerNote* clone() const;
        //! Return the name of the makernote section ("Canon")
        std::string sectionName(uint16 tag) const { return sectionName_; }
        std::ostream& printTag(std::ostream& os,
                               uint16 tag, 
                               const Value& value) const;
        //@}

        //! @name Print functions for Canon %MakerNote tags 
        //@{
        //! Print various camera settings, part 1
        std::ostream& print0x0001(std::ostream& os, const Value& value) const;
        //! Print various camera settings, part 2
        std::ostream& print0x0004(std::ostream& os, const Value& value) const;
        //! Print the image number
        std::ostream& print0x0008(std::ostream& os, const Value& value) const;
        //! Print the serial number of the camera
        std::ostream& print0x000c(std::ostream& os, const Value& value) const;
        //! Print EOS D30 custom functions
        std::ostream& print0x000f(std::ostream& os, const Value& value) const;
        //@}

    private:
        std::string sectionName_;

    }; // class CanonMakerNote
   
}                                       // namespace Exif

#endif                                  // #ifndef CANONMN_HPP_
