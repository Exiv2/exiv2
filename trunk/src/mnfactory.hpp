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
  @file    mnfactory.hpp
  @brief   Factory for MakerNote classes
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created
           07-Mar-04, ahu: isolated as a separate component
 */
#ifndef MNFACTORY_HPP_
#define MNFACTORY_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <utility>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exif {

    class MakerNote;

// *****************************************************************************
// class definitions

    /*!
      @brief %MakerNote factory.

      Creates an instance of the %MakerNote for one camera model. The factory is
      implemented as a singleton, which can be accessed only through the static
      member function instance().
    */
    class MakerNoteFactory {
    public:
        /*!
          @brief Access the makerNote factory. Clients access the task factory
                 exclusively through this method.
        */
        static MakerNoteFactory& instance();

        //! @name Manipulators
        //@{        
        /*!
          @brief Register a %MakerNote prototype for a camera make and model.

          Registers a %MakerNote for a given make and model combination with the
          factory. Both the make and model strings may contain wildcards ('*',
          e.g., "Canon*").  The method adds a new makerNote pointer to the
          registry with the make and model strings provided. It takes ownership
          of the object pointed to by the maker note pointer provided. If the
          make already exists, then a new branch for the model is added to the
          registry. If the model also already exists, then the new makerNote
          pointer replaces the old one and the maker note pointed to by the old
          pointer is deleted.

          @param make Camera manufacturer. (Typically the string from the %Exif
                 make tag.)
          @param model Camera model. (Typically the string from the %Exif
                 model tag.)
          @param makerNote Pointer to the prototype. Ownership is transfered to the
                 %MakerNote factory.
        */
        void registerMakerNote(const std::string& make, 
                               const std::string& model, 
                               MakerNote* makerNote);
        //@}

        //! @name Accessors
        //@{        
        /*!
          @brief Create the appropriate %MakerNote based on camera make and
                 model, return a pointer to the newly created MakerNote
                 instance. Return 0 if no %MakerNote is defined for the camera
                 model.

          The method searches the make-model tree for a make and model
          combination in the registry that matches the search key. The search is
          case insensitive (Todo: implement case-insensitive comparisons) and
          wildcards in the registry entries are supported. First the best
          matching make is searched, then the best matching model for this make
          is searched. If there is no matching make or no matching model within
          the models registered for the best matching make, then no maker note
          is created and the function returns 0. If a match is found, the
          function returns a pointer to a clone of the registered prototype. The
          maker note pointed to is owned by the caller of the function, i.e.,
          the caller is responsible to delete the returned make note when it is
          no longer needed.
          The best match is the match with the most matching characters.

          @param make Camera manufacturer. (Typically the string from the %Exif
                 make tag.)
          @param model Camera model. (Typically the string from the %Exif
                 model tag.)
          @return A pointer that owns a %MakerNote for the camera model.  If
                 the camera is not supported, the pointer is 0.
         */
        MakerNote* create(const std::string& make, 
                          const std::string& model) const;
        //@}

        /*!
          @brief Match a registry entry with a key (used for make and model).

          The matching algorithm is case insensitive and wildcards ('*') in the
          registry entry are supported. The best match is the match with the
          most matching characters.

          @return A pair of which the first component indicates whether or not
                  the key matches and the second component contains the number
                  of matching characters.
         */
        static std::pair<bool, int> match(const std::string& regEntry, 
                                          const std::string& key);

    private:
        //! @name Creators
        //@{                
        //! Prevent construction other than through instance().
        MakerNoteFactory();
        //! Prevent copy construction: not implemented.
        MakerNoteFactory(const MakerNoteFactory& rhs);
        //@}

        //! Type used to store model labels and %MakerNote prototype classes
        typedef std::vector<std::pair<std::string, MakerNote*> > ModelRegistry;
        //! Type used to store a list of make labels and model registries
        typedef std::vector<std::pair<std::string, ModelRegistry*> > Registry;

        // DATA
        //! Pointer to the one and only instance of this class.
        static MakerNoteFactory* instance_;
        //! List of makernote types and corresponding prototypes.
        Registry registry_;

    }; // class MakerNoteFactory
   
}                                       // namespace Exif

#endif                                  // #ifndef MNFACTORY_HPP_
