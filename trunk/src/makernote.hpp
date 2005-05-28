// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @file    makernote.hpp
  @brief   Contains the Exif %MakerNote interface, IFD %MakerNote and a 
           MakerNote factory
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    18-Feb-04, ahu: created
 */
#ifndef MAKERNOTE_HPP_
#define MAKERNOTE_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "ifd.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <utility>
#include <vector>
#include <map>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class Value;

// *****************************************************************************
// class definitions

    /*!
      @brief Exif makernote interface

      %MakerNote is a low-level container for makernote entries. The ExifData
      container uses makernote entries just like the other Exif metadata. Thus,
      clients can access Exif and makernote tags and their values uniformly
      through the ExifData interface. The role of %MakerNote is very similar to
      that of class Ifd (but makernotes do not need to be in IFD format, see
      below). In addition, it provides %MakerNote specific tag descriptions and
      print functions to interpret the makernote values.

      MakerNote holds methods and functionality to
      - read the makernote from a character buffer
      - copy the makernote to a character buffer
      - maintain a list of makernote entries (similar to IFD entries) 
      - interpret (print) the values of makernote tags

      Makernotes can be added to the system by subclassing %MakerNote and
      registering a create function for the new subclass together with the
      camera make and model (which may contain wildcards) in the
      MakerNoteFactory. Since the majority of makernotes are in IFD format,
      subclass IfdMakerNote is provided. It contains an IFD container and
      implements all interface methods related to the makernote entries. <BR>

      To implement a new IFD makernote, all that you need to do is 
      - subclass %IfdMakerNote, 
      - implement methods to read and check the header (if any) as well as
        clone and create functions,
      - add a list of tag descriptions and appropriate print functions and 
      - register the camera make/model and create function in the makernote factory. 
      .
      See existing makernote implementations for examples, e.g., CanonMakerNote
      or FujiMakerNote.

      Finally, the header file which defines the static variable 
      \em register*MakerNote needs to be included from mn.hpp, to ensure that 
      the makernote is automatically registered in the factory.
     */
    class MakerNote {
        //! @name Not implemented
        //@{
        //! Assignment not allowed (memory management mode alloc_ is const)
        MakerNote& operator=(const MakerNote& rhs);
        //@}

    public:
        //! Shortcut for a %MakerNote auto pointer.
        typedef std::auto_ptr<MakerNote> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor. Allows to choose whether or not memory management 
                 is required for the Entries.
         */
        explicit MakerNote(bool alloc =true);
        //! Virtual destructor.
        virtual ~MakerNote() {}
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the makernote, including the makernote header, from
                 character buffer buf of length len at position offset (from the
                 start of the TIFF header) and encoded in byte order byteOrder.
                 Return 0 if successful.
         */
        virtual int read(const byte* buf, 
                         long len, 
                         ByteOrder byteOrder,
                         long offset) =0;
        /*!
          @brief Copy (write) the makerNote to the character buffer buf at 
                 position offset (from the start of the TIFF header), encoded
                 in byte order byteOrder. Update internal offsets if necessary.
                 Return the number of bytes written.
         */
        virtual long copy(byte* buf, ByteOrder byteOrder, long offset) =0;
        /*!
          @brief Add the entry to the makernote. No duplicate-check is performed,
                 i.e., it is possible to add multiple entries with the same tag.
                 The memory allocation mode of the entry to be added must be the 
                 same as that of the makernote and the IFD id of the entry must
                 be set to 'makerIfd'.
         */
        virtual void add(const Entry& entry) =0;
        //! The first makernote entry
        virtual Entries::iterator begin() =0;
        //! End of the makernote entries
        virtual Entries::iterator end() =0;
        /*!
          @brief Update the base pointer of the %MakerNote and all its entries 
                 to \em pNewBase.

          Allows to re-locate the underlying data buffer to a new location
          \em pNewBase. This method only has an effect in non-alloc mode.
         */
        virtual void updateBase(byte* pNewBase) =0;
        //@}

        //! @name Accessors
        //@{
        //! Return the byte order (little or big endian).
        ByteOrder byteOrder() const { return byteOrder_; }
        //! Return the offset of the makernote from the start of the TIFF header
        long offset() const  { return offset_; }
        /*!
          @brief Return an auto-pointer to an newly created, empty instance of
                 the same type as this. The makernote entries are <B>not</B>
                 copied.  The caller owns the new object and the auto-pointer
                 ensures that it will be deleted.

          @param alloc Memory management model for the newly created object. 
                 Indicates if memory required to store data should be allocated
                 and deallocated (true) or not (false). If false, only pointers
                 to the buffer provided to read() will be kept. See Ifd for more
                 background on this concept.
         */
        AutoPtr create(bool alloc =true) const;
        /*!
          @brief Return an auto-pointer to a clone of this object. The caller
                 owns the new object and the auto-pointer ensures that it will
                 be deleted.

          @note  In non-alloc mode the clone potentially contains pointers to 
                 the same data buffer as the original. 
                 Use updateBase(byte* pNewBase) to adjust them.
         */
        AutoPtr clone() const;
        //! The first makernote entry
        virtual Entries::const_iterator begin() const =0;
        //! End of the makernote entries
        virtual Entries::const_iterator end() const =0;
        //! Find an entry by idx, return a const iterator to the record
        virtual Entries::const_iterator findIdx(int idx) const =0;
        //! Return the size of the makernote in bytes
        virtual long size() const =0;
        //@}

    protected:
        // DATA
        /*!
          @brief Flag to control the memory management: <BR>
                 True:  requires memory allocation and deallocation, <BR>
                 False: no memory management needed.
         */
        const bool alloc_; 
        /*! 
          @brief Offset of the makernote from the start of the TIFF header 
                 (for offset()).
         */
        long offset_;
        /*!
          @brief Alternative byte order to use, invalid if the byte order of the
                 Exif block can be used
         */
        ByteOrder byteOrder_;

    private:
        //! Internal virtual create function.
        virtual MakerNote* create_(bool alloc =true) const =0;
        //! Internal virtual copy constructor.
        virtual MakerNote* clone_() const =0;

    }; // class MakerNote

    //! Type for a pointer to a function creating a makernote
    typedef MakerNote::AutoPtr (*CreateFct)(bool, const byte*, long, ByteOrder, long);

    /*!
      @brief Interface for MakerNotes in IFD format. See MakerNote.
     */
    class IfdMakerNote : public MakerNote {
        //! @name Not implemented
        //@{
        //! Assignment not allowed (Ifd does not have an assignment operator)
        IfdMakerNote& operator=(const IfdMakerNote& rhs);
        //@}

    public:
        //! Shortcut for an %IfdMakerNote auto pointer.
        typedef std::auto_ptr<IfdMakerNote> AutoPtr;

        //! @name Creators
        //@{        
        /*!
          @brief Constructor. Requires an %Ifd id and allows to choose whether 
                 or not memory management is needed for the Entries and whether
                 the IFD has a next pointer.
        */
        explicit IfdMakerNote(IfdId ifdId, bool alloc =true, bool hasNext =true);
        //! Copy constructor
        IfdMakerNote(const IfdMakerNote& rhs);
        //! Virtual destructor
        virtual ~IfdMakerNote() {}
        //@}

        //! @name Manipulators
        //@{
        virtual int read(const byte* buf, 
                         long len, 
                         ByteOrder byteOrder, 
                         long offset);
        /*!
          @brief Read the makernote header from the makernote databuffer.  This
                 method must set the offset adjustment (adjOffset_), if needed
                 (assuming that the required information is in the header).
                 Return 0 if successful.          
          @note  The default implementation does nothing, assuming there is no
                 header
         */
        virtual int readHeader(const byte* buf, 
                               long len,
                               ByteOrder byteOrder);
        virtual long copy(byte* buf, ByteOrder byteOrder, long offset);
        virtual void add(const Entry& entry) { ifd_.add(entry); }
        virtual Entries::iterator begin() { return ifd_.begin(); }
        virtual Entries::iterator end() { return ifd_.end(); }
        virtual void updateBase(byte* pNewBase);
        //@}

        //! @name Accessors
        //@{
        virtual Entries::const_iterator begin() const { return ifd_.begin(); }
        virtual Entries::const_iterator end() const { return ifd_.end(); }
        virtual Entries::const_iterator findIdx(int idx) const;
        virtual long size() const;
        AutoPtr create(bool alloc =true) const;
        AutoPtr clone() const;
        /*!
          @brief Check the makernote header. This will typically check if a
                 required prefix string is present in the header. Return 0 if
                 successful.
          @note  The default implementation does nothing, assuming there is no
                 header
         */
        virtual int checkHeader() const;
        /*!
          @brief Write the makernote header to a character buffer, return the 
                 number of characters written.
          @note  The default implementation copies the header_ buffer.
         */
        virtual long copyHeader(byte* buf) const;
        /*! 
          @brief Return the size of the makernote header in bytes.
          @note  The default implementation returns the size of the header_ 
                 buffer.
         */
        virtual long headerSize() const;
        //@}

    protected:
        // DATA
        /*!
          @brief True:  Adjustment of the IFD offsets is to be added to the
                        offset from the start of the TIFF header (i.e., the
                        start of the Exif data section),
                 False: Adjustment of the IFD offsets is a suitable absolute 
                        value. Ignore the offset from the start of the TIFF 
                        header.
         */
        bool absOffset_;
        /*!
          @brief Adjustment of the IFD offsets relative to the start of the 
                 TIFF header or to the start of the makernote, depending on 
                 the setting of absOffset_.
         */
        long adjOffset_;
        //! Data buffer for the makernote header
        DataBuf header_;
        //! The makernote IFD
        Ifd ifd_;

    private:
        virtual IfdMakerNote* create_(bool alloc =true) const =0;
        virtual IfdMakerNote* clone_() const =0;

    }; // class IfdMakerNote

    /*!
      @brief Factory for MakerNote objects.

      Maintains an associative list (tree) of camera makes/models and
      corresponding %MakerNote create functions. Creates an instance of the
      %MakerNote for one camera make/model. The factory is implemented as a
      static class.
    */
    class MakerNoteFactory {
    public:
        /*!
          @brief Register a %MakerNote create function for a camera make and
                 model.

          Registers a create function for a %MakerNote for a given make and
          model combination with the factory. Both the make and model strings
          may contain wildcards ('*', e.g., "Canon*").  If the make already
          exists in the registry, then a new branch for the model is added. If
          the model also already exists, then the new create function replaces
          the old one.

          @param make Camera manufacturer. (Typically the string from the Exif
                 make tag.)
          @param model Camera model. (Typically the string from the Exif
                 model tag.)
          @param createMakerNote Pointer to a function to create a new 
                 %MakerNote of a particular type.
        */
        static void registerMakerNote(const std::string& make, 
                                      const std::string& model, 
                                      CreateFct createMakerNote);
        
        //! Register a %MakerNote prototype in the IFD id registry.
        static void registerMakerNote(IfdId ifdId, MakerNote::AutoPtr makerNote);

        /*!
          @brief Create the appropriate %MakerNote based on camera make and
                 model and possibly the contents of the makernote itself, return
                 an auto-pointer to the newly created MakerNote instance. Return
                 0 if no %MakerNote is defined for the camera model.

          The method searches the make-model tree for a make and model
          combination in the registry that matches the search key. The search is
          case insensitive (Todo: implement case-insensitive comparisons) and
          wildcards in the registry entries are supported. First the best
          matching make is searched, then the best matching model for this make
          is searched. If there is no matching make or no matching model within
          the models registered for the best matching make, then no makernote
          is created and the function returns 0. If a match is found, the
          function invokes the registered create function and returns an
          auto-pointer to the newly created MakerNote. The makernote pointed to
          is owned by the caller of the function and the auto-pointer ensures
          that it is deleted.  The best match is an exact match, then a match is
          rated according to the number of matching characters. The makernote
          buffer is passed on to the create function, which can based on its
          content, automatically determine the correct version or flavour of the
          makernote required. This is used, e.g., to determine which of the
          three Nikon makernotes to create.

          @param make Camera manufacturer. (Typically the string from the Exif
                 make tag.)
          @param model Camera model. (Typically the string from the Exif
                 model tag.)
          @param alloc Memory management model for the new MakerNote. Determines
                 if memory required to store data should be allocated and
                 deallocated (true) or not (false). If false, only pointers to
                 the buffer provided to read() will be kept. See Ifd for more
                 background on this concept. 
          @param buf Pointer to the makernote character buffer. 
          @param len Length of the makernote character buffer. 
          @param byteOrder Byte order in which the Exif data (and possibly the 
                 makernote) is encoded.
          @param offset Offset from the start of the TIFF header of the makernote
                 buffer.

          @return An auto-pointer that owns a %MakerNote for the camera model.  
                 If the camera is not supported, the pointer is 0.
         */
        static MakerNote::AutoPtr create(const std::string& make, 
                                         const std::string& model, 
                                         bool alloc, 
                                         const byte* buf, 
                                         long len, 
                                         ByteOrder byteOrder, 
                                         long offset); 

        //! Create a %MakerNote for an IFD id.
        static MakerNote::AutoPtr create(IfdId ifdId, bool alloc =true);

        /*!
          @brief Match a registry entry with a key (used for make and model).

          The matching algorithm is case insensitive and wildcards ('*') in the
          registry entry are supported. The best match is an exact match, then
          a match is rated according to the number of matching characters.

          @return A score value indicating how good the key and registry entry 
                  match. 0 means no match, values greater than 0 indicate a
                  match, larger values are better matches:<BR>
                  0: key and registry entry do not match<BR>
                  1: a pure wildcard match, i.e., the registry entry is just 
                     a wildcard.<BR>
                  Score values greater than 1 are computed by adding 1 to the 
                  number of matching characters, except for an exact match, 
                  which scores 2 plus the number of matching characters.
         */
        static int match(const std::string& regEntry, const std::string& key);

    private:
        //! @name Creators
        //@{                
        //! Prevent construction: not implemented.
        MakerNoteFactory() {}
        //! Prevent copy construction: not implemented.
        MakerNoteFactory(const MakerNoteFactory& rhs);
        //@}

        //! Creates the private static instance
        static void init();

        //! Type used to store model labels and %MakerNote create functions
        typedef std::vector<std::pair<std::string, CreateFct> > ModelRegistry;
        //! Type used to store a list of make labels and model registries
        typedef std::vector<std::pair<std::string, ModelRegistry*> > Registry;
        //! Type used to store a list of IFD ids and %MakerNote prototypes
        typedef std::map<IfdId, MakerNote*> IfdIdRegistry;

        // DATA
        //! List of makernote types and corresponding makernote create functions.
        static Registry* pRegistry_;
        //! List of makernote IFD ids and corresponding create functions.
        static IfdIdRegistry* pIfdIdRegistry_;

    }; // class MakerNoteFactory
   
}                                       // namespace Exiv2

#endif                                  // #ifndef MAKERNOTE_HPP_
