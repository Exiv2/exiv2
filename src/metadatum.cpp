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
/*
  File:      metadatum.cpp
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <schick@robotbattle.com>
  History:   26-Jan-04, ahu: created
             31-Jul-04, brad: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: metadatum.cpp,v $");

// *****************************************************************************
// included header files
#include "metadatum.hpp"

// + standard includes
#include <iostream>
#include <iomanip>


// *****************************************************************************
// class member definitions
namespace Exiv2 {

    std::ostream& operator<<(std::ostream& os, const Metadatum& md)
    {
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << md.tag() << " " 
                  << std::setw(40) << std::setfill(' ') << std::left
                  << md.key() << " "
                  << std::setw(9) << std::setfill(' ') << std::left
                  << md.typeName() << " "
                  << std::dec << md.value() 
                  << "\n";
        return os;
    }


    bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.tag() < rhs.tag();
    }


    bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.key() < rhs.key();
    }

}                                       // namespace Exiv2

