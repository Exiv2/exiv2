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
  @file    exif.hpp
  @brief   Encoding and decoding of %Exif data
  @version $Name:  $ $Revision: 1.26 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    09-Jan-04, ahu: created
 */
#ifndef EXIF_HPP_
#define EXIF_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "error.hpp"
#include "image.hpp"
#include "value.hpp"
#include "ifd.hpp"
#include "tags.hpp"

// + standard includes
#include <string>
#include <vector>
#include <iostream>

// *****************************************************************************
// namespace extensions
//! Provides classes and functions to encode and decode %Exif data.
namespace Exif {

// *****************************************************************************
// class declarations
    class ExifData;
    class MakerNote;

// *****************************************************************************
// class definitions

    /*!
      @brief Information related to one %Exif tag.
     */
    class Metadatum {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The
                 metadatum is created from a key / value pair. %Metadatum copies
                 (clones) the value if one is provided. Alternatively, a program
                 can create an 'empty' metadatum with only a key and set the
                 value using setValue().

          @param key The key of the metadatum.
          @param value Pointer to a metadatum value.
          @param makerNote Pointer to the associated MakerNote (only needed for 
                 MakerNote tags).
          @throw Error ("Invalid key") if the key cannot be parsed and converted
                 to a tag number and an IFD id or the section name does not match.
         */
        explicit Metadatum(const std::string& key, 
                           const Value* value =0, 
                           MakerNote* makerNote =0);
        //! Constructor to build a metadatum from an IFD entry.
        Metadatum(const Entry& e, ByteOrder byteOrder);
        //! Copy constructor
        Metadatum(const Metadatum& rhs);
        //! Destructor
        ~Metadatum();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Metadatum& operator=(const Metadatum& rhs);
        /*!
          @brief Set the value. This method copies (clones) the value.
         */
        void setValue(const Value* value);
        /*!
          @brief Set the value to the string buf. 
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then an AsciiValue is created.
         */
        void setValue(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write value to a character data buffer and return the number
                 of characters (bytes) written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        long copy(char* buf, ByteOrder byteOrder) const 
            { return value_ == 0 ? 0 : value_->copy(buf, byteOrder); }
        /*!
          @brief Return the key of the metadatum. The key is of the form
                 'ifdItem.sectionName.tagName'. Note however that the key
                 is not necessarily unique.
         */
        std::string key() const { return key_; }
        //! Return the related image item (the first part of the key)
        const char* ifdItem() const { return ExifTags::ifdItem(ifdId_); }
        //! Return the name of the section (the second part of the key)
        std::string sectionName() const;
        //! Return the name of the tag (which is also the third part of the key)
        std::string tagName() const;
        //! Return the tag
        uint16 tag() const { return tag_; }
        //! Return the type id of the value
        TypeId typeId() const { return value_ == 0 ? invalid : value_->typeId(); }
        //! Return the name of the type
        const char* typeName() const { return TypeInfo::typeName(typeId()); }
        //! Return the size in bytes of one component of this type
        long typeSize() const { return TypeInfo::typeSize(typeId()); }
        //! Return the number of components in the value
        long count() const { return value_ == 0 ? 0 : value_->count(); }
        //! Return the size of the value in bytes
        long size() const { return value_ == 0 ? 0 : value_->size(); }
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId_); }
        //! Return the index (unique id of this metadatum within the original IFD)
        int idx() const { return idx_; }
        //! Return the pointer to the associated MakerNote
        MakerNote* makerNote() const { return makerNote_; }
        //! Return the value as a string.
        std::string toString() const 
            { return value_ == 0 ? "" : value_->toString(); }
        /*!
          @brief Return the n-th component of the value converted to long. The
                 return value is -1 if the value of the Metadatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        long toLong(long n =0) const 
            { return value_ == 0 ? -1 : value_->toLong(n); }
        /*!
          @brief Return the n-th component of the value converted to float.  The
                 return value is -1 if the value of the Metadatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        float toFloat(long n =0) const 
            { return value_ == 0 ? -1 : value_->toFloat(n); }
        /*!
          @brief Return the n-th component of the value converted to
                 Rational. The return value is -1/1 if the value of the
                 Metadatum is not set and the behaviour of the method is
                 undefined if there is no n-th component.
         */
        Rational toRational(long n =0) const 
            { return value_ == 0 ? Rational(-1, 1) : value_->toRational(n); }
        /*!
          @brief Return a pointer to a copy (clone) of the value. The caller
                 is responsible to delete this copy when it's no longer needed.

          This method is provided for users who need full control over the 
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.
          
          @return A pointer to a copy (clone) of the value, 0 if the value is 
                  not set.
         */
        Value* getValue() const { return value_ == 0 ? 0 : value_->clone(); }
        /*!
          @brief Return a constant reference to the value. 

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. 

          <b>Example:</b> <br>
          @code
          ExifData::const_iterator i = exifData.findKey(key);
          if (i != exifData.end()) {
              std::cout << i->key() << " " << std::hex << i->value() << "\n";
          }
          @endcode

          @return A constant reference to the value.
          @throw Error ("Value not set") if the value is not set.
         */
        const Value& value() const 
            { if (value_) return *value_; throw Error("Value not set"); }
        //@}

    private:
        uint16 tag_;                   //!< Tag value
        IfdId ifdId_;                  //!< The IFD associated with this tag
        int idx_;                      //!< Unique id of an entry within one IFD
        MakerNote* makerNote_;         //!< Pointer to the associated MakerNote
        Value* value_;                 //!< Pointer to the value
        std::string key_;              //!< Key

    }; // class Metadatum

    /*!
      @brief Output operator for Metadatum types, printing the interpreted
             tag value.
     */
    std::ostream& operator<<(std::ostream& os, const Metadatum& md);

    //! %Thumbnail data Todo: add, create, rotate, delete
    class Thumbnail {
    public:
        //! %Thumbnail image types
        enum Type { none, jpeg, tiff };

        //! @name Creators
        //@{
        //! Default constructor
        Thumbnail();
        //! Destructor
        ~Thumbnail();
        //! Copy constructor
        Thumbnail(const Thumbnail& rhs);
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Thumbnail& operator=(const Thumbnail& rhs);
        /*!
          @brief Read the thumbnail from the data buffer buf, using %Exif
                 metadata exifData. Return 0 if successful. 

          @param buf Data buffer containing the thumbnail data. The buffer must
                 start with the TIFF header.  
          @param exifData %Exif data corresponding to the data buffer.
          @param byteOrder The byte order used for the encoding of TIFF
                 thumbnails. It determines the byte order of the resulting
                 thumbnail image, if it is in TIFF format. For JPEG thumbnails
                 the byte order is not used.
          @return 0 if successful<br>
                 -1 if there is no thumbnail image according to the %Exif data<br>
                  1 in case of inconsistent JPEG thumbnail %Exif data<br>
                  2 in case of inconsistent TIFF thumbnail %Exif data<br>
         */
        int read(const char* buf, 
                 const ExifData& exifData,
                 ByteOrder byteOrder =littleEndian);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write thumbnail to file path, return 0 if successful, -1 if 
                 there is no thumbnail image to write.
         */
        int write(const std::string& path) const;
        /*!
          @brief Copy the thumbnail image data (without the IFD, if any) to the
                 data buffer buf. The user must ensure that the buffer has
                 enough memory. Otherwise the call results in undefined
                 behaviour. Return the number of characters written.
         */
        long copy(char* buf) const;
        /*!
          @brief Update the %Exif data according to the actual thumbnail image.
          
          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to 0. If the type is TIFF, StripOffsets are set to the offsets of
          the IFD of the thumbnail image itself.
         */
        void update(ExifData& exifData) const;
        /*!
          @brief Update the thumbnail data offsets in IFD1 assuming the
                 thumbnail data follows immediately after IFD1.  

          If the type of the thumbnail image is JPEG, JPEGInterchangeFormat is
          set to point directly behind the data area of IFD1. If the type is
          TIFF, StripOffsets from the thumbnail image are adjusted to point to
          the strips, which have to follow immediately after IFD1. Use copy() to
          write the thumbnail image data. The offset of IFD1 must be set
          correctly. Changing the size of IFD1 invalidates the thumbnail data
          offsets set by this method.
         */
        void setOffsets(Ifd& ifd1, ByteOrder byteOrder) const;
        /*!
          @brief Return the size of the thumbnail data (data only, without the 
                 IFD, in case of a TIFF thumbnail.
         */
        long size() const;
        //! Return the type of the thumbnail
        Type type() const { return type_; }
        //@}

    private:
        //! @name Manipulators
        //@{
        //! Read a compressed (JPEG) thumbnail image from the data buffer
        int readJpegImage(const char* buf, const ExifData& exifData);
        //! Read an uncompressed (TIFF) thumbnail image from the data buffer
        int readTiffImage(const char* buf,
                          const ExifData& exifData,
                          ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        //! Update the Exif data according to the actual JPEG thumbnail image
        void updateJpegImage(ExifData& exifData) const;
        //! Update the Exif data according to the actual TIFF thumbnail image
        void updateTiffImage(ExifData& exifData) const;
        //! Copy the JPEG thumbnail image data to the data buffer buf
        long copyJpegImage(char* buf) const;
        //! Copy the TIFF thumbnail image data to the data buffer buf
        long copyTiffImage(char* buf) const;
        //! Update the offsets to the JPEG thumbnail image in the IFD
        void setJpegImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const;
        //! Update the offsets to the TIFF thumbnail image in the IFD
        void setTiffImageOffsets(Ifd& ifd1, ByteOrder byteOrder) const;
        //@}

        // DATA
        Type type_;              // Type of thumbnail image
        long size_;              // Size of the image data
        char* image_;            // Thumbnail image data
        TiffHeader tiffHeader_;  // Thumbnail TIFF Header, only for TIFF thumbs
        Ifd ifd_;                // Thumbnail IFD, only for TIFF thumbnails

    }; // class Thumbnail

    //! Container type to hold all metadata
    typedef std::vector<Metadatum> Metadata;

    //! Unary predicate that matches a Metadatum with a given key
    class FindMetadatumByKey {
    public:
        //! Constructor, initializes the object with the tag to look for
        FindMetadatumByKey(const std::string& key) : key_(key) {}
        /*!
          @brief Returns true if the key of the argument metadatum is equal
          to that of the object.
        */
        bool operator()(const Metadatum& metadatum) const
            { return key_ == metadatum.key(); }

    private:
        std::string key_;
        
    }; // class FindMetadatumByTag

    //! Unary predicate that matches a Metadatum with a given ifd id and idx
    class FindMetadatumByIfdIdIdx {
    public:
        //! Constructor, initializes the object with the ifd id and idx to look for
        FindMetadatumByIfdIdIdx(IfdId ifdId, int idx)
            : ifdId_(ifdId), idx_(idx) {}
        /*!
          @brief Returns true if the ifd id and idx of the argument metadatum 
                 is equal to that of the object.
        */
        bool operator()(const Metadatum& metadatum) const
            { return ifdId_ == metadatum.ifdId() && idx_ == metadatum.idx(); }

    private:
        IfdId ifdId_;
        int idx_;
        
    }; // class FindMetadatumByIfdIdIdx

    /*!
      @brief A container for %Exif data. This is the top-level class of 
             the Exiv2 library.

      Provide high-level access to the %Exif data of an image:
      - read %Exif information from JPEG files
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata 
      - write %Exif data to JPEG files
      - extract %Exif metadata to files, insert from these files
      - extract and delete %Exif thumbnail (JPEG and TIFF thumbnails)
    */
    class ExifData {
        //! @name Not implemented
        //@{
        //! Copying not allowed (Todo: implement me!)
        ExifData(const ExifData& rhs);
        //! Assignment not allowed (Todo: implement me!)
        ExifData& operator=(const ExifData& rhs);
        //@}
    public:
        //! Metadata iterator type
        typedef Metadata::iterator iterator;
        //! Metadata const iterator type
        typedef Metadata::const_iterator const_iterator;

        //! @name Creators
        //@{
        //! Default constructor
        ExifData();
        //! Destructor
        ~ExifData();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the %Exif data from file path.
          @param path Path to the file
          @return 0 if successful<br>
                  the return code of readExifData(std::istream& is)
                    if the call to this function fails<br>
                  the return code of read(const char* buf, long len)
                    if the call to this function fails<br>
         */
        int read(const std::string& path);
        /*!
          @brief Read the %Exif data from a character buffer. The data buffer
                 must start with the TIFF header.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful
         */
        int read(const char* buf, long len);
        /*!
          @brief Write the %Exif data to file path. If an %Exif data section
                 already exists in the file, it is replaced. Otherwise, an
                 %Exif data section is created. See copy(char* buf) for further
                 details.

          @return 0 if successful.
         */
        int write(const std::string& path);
        /*!
          @brief Write the %Exif data to a data buffer, return number of bytes 
                 written. The copied data starts with the TIFF header.

          Tries to update the original data buffer and write it back with
          minimal changes, in a 'non-intrusive' fashion, if possible. In this
          case, tag data that ExifData does not understand stand a good chance
          to remain valid. (In particular, if the %Exif data contains a
          Makernote in IFD format, the offsets in its IFD will remain valid.)
          <BR>
          If 'non-intrusive' writing is not possible, the Exif data will be
          re-built from scratch, in which case the absolute position of the
          metadata entries within the data buffer may (and in most cases will)
          be different from their original position. Furthermore, in this case,
          the Exif data is updated with the metadata from the actual thumbnail
          image (overriding existing metadata).

          @param buf The data buffer to write to.  The user must ensure that the
                 buffer has enough memory. Otherwise the call results in
                 undefined behaviour.
          @return Number of characters written to the buffer.
         */ 
        long copy(char* buf);
        /*!
          @brief Add all (IFD) entries in the range from iterator position begin
                 to iterator position end to the %Exif metadata. No duplicate
                 checks are performed, i.e., it is possible to add multiple
                 metadata with the same key.
         */
        void add(Entries::const_iterator begin, 
                 Entries::const_iterator end,
                 ByteOrder byteOrder);
        /*!
          @brief Add a metadatum from the supplied key and value pair.  This
                 method copies (clones) the value.  No duplicate checks are
                 performed, i.e., it is possible to add multiple metadata with
                 the same key.
         */
        void add(const std::string& key, Value* value);
        /*! 
          @brief Add a copy of the metadatum to the %Exif metadata.  No
                 duplicate checks are performed, i.e., it is possible to add
                 multiple metadata with the same key.
         */
        void add(const Metadatum& metadatum);
        //! Delete the metadatum at iterator position pos
        void erase(iterator pos);
        //! Sort metadata by key
        void sortByKey();
        //! Sort metadata by tag
        void sortByTag();
        //! Begin of the metadata
        iterator begin() { return metadata_.begin(); }
        //! End of the metadata
        iterator end() { return metadata_.end(); }
        /*!
          @brief Find a metadatum with the given key, return an iterator to it.
                 If multiple metadata with the same key exist, it is undefined 
                 which of the matching metadata is found.
         */
        iterator findKey(const std::string& key);
        /*!
          @brief Find the metadatum with the given ifd id and idx, return an 
                 iterator to it. 

          This method can be used to uniquely identify a metadatum that was
          created from an IFD or from the makernote (with idx greater than
          0). Metadata created by an application (not read from an IFD or a
          makernote) all have their idx field set to 0, i.e., they cannot be
          uniquely identified with this method.  If multiple metadata with the
          same key exist, it is undefined which of the matching metadata is
          found.
         */
        iterator findIfdIdIdx(IfdId ifdId, int idx);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const { return metadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return metadata_.end(); }
        /*!
          @brief Find a metadatum with the given key, return a const iterator to
                 it.  If multiple metadata with the same key exist, it is
                 undefined which of the matching metadata is found.
         */
        const_iterator findKey(const std::string& key) const;
        /*!
          @brief Find the metadatum with the given ifd id and idx, return an 
                 iterator to it. 

          This method can be used to uniquely identify a metadatum that was
          created from an IFD or from the makernote (with idx greater than
          0). Metadata created by an application (not read from an IFD or a
          makernote) all have their idx field set to 0, i.e., they cannot be
          uniquely identified with this method.  If multiple metadata with the
          same key exist, it is undefined which of the matching metadata is
          found.
         */
        const_iterator findIfdIdIdx(IfdId ifdId, int idx) const;
        //! Get the number of metadata entries
        long count() const { return metadata_.size(); }
        /*!
          @brief Return the approximate size of all %Exif data (TIFF header plus 
                 metadata). The number returned may be bigger than the actual 
                 size of the %Exif data, but it is never smaller. Only copy()
                 returns the exact size.
         */
        long size() const;
        //! Returns the byte order as specified in the TIFF header
        ByteOrder byteOrder() const { return tiffHeader_.byteOrder(); }
        /*!
          @brief Write the thumbnail image to a file. The filename extension
                 will be set according to the image type of the thumbnail, so
                 the path should not include an extension.
         */
        int writeThumbnail(const std::string& path) const 
            { return thumbnail_.write(path); }
        //! Return the type of the thumbnail
        Thumbnail::Type thumbnailType() const { return thumbnail_.type(); }
        //! Return the size of the thumbnail data
        long thumbnailSize() const { return thumbnail_.size(); }
        //@}

    private:
        //! @name Accessors
        //@{
        /*!
          @brief Check if the metadata is compatible with the internal IFDs for
                 non-intrusive writing. Return true if compatible, false if not.

          @note This function does not detect deleted metadata as incompatible,
                although the deletion of metadata is not (yet) a supported
                non-intrusive write operation.
         */
        bool compatible() const;
        /*!
          @brief Find the IFD or makernote entry corresponding to ifd id and idx.

          @return A pair of which the first part determines if a match was found
                  and, if true, the second contains an iterator to the entry.
         */
        std::pair<bool, Entries::const_iterator> 
        findEntry(IfdId ifdId, int idx) const;
        //! Return a pointer to the internal IFD identified by its IFD id
        const Ifd* getIfd(IfdId ifdId) const;
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Check if the metadata changed and update the internal IFDs and
                 the MakerNote if the changes are compatible with the existing
                 data (non-intrusive write support). 

          @return True if only compatible changes were detected in the metadata
                  and the internal IFDs and MakerNote (and thus the data buffer)
                  were updated successfully. Return false, if non-intrusive
                  writing is not possible. The internal IFDs and the MakerNote
                  (and thus the data buffer) may or may not be modified in this
                  case.
         */
        bool updateEntries();
        /*!
          @brief Update the metadata for a range of entries. Called by
                 updateEntries() for each of the internal IFDs and the MakerNote
                 (if any).
         */
        bool updateRange(const Entries::iterator& begin,
                         const Entries::iterator& end);
        /*!
          @brief Write Exif data to a data buffer the hard way, return number of
                 bytes written.

          Rebuilds the Exif data from scratch, using the TIFF header, metadata
          container and thumbnail. In particular, the internal IFDs and the
          original data buffer are not used. Furthermore, this method updates
          the Exif data with the metadata from the actual thumbnail image
          (overriding existing metadata).
         */
        long copyFromMetadata(char* buf);
        //@}

        // DATA
        TiffHeader tiffHeader_;
        Metadata metadata_;
        Thumbnail thumbnail_;
        MakerNote* makerNote_;   // Todo: implement reference counting instead
                                 //       of making ExifData own this pointer

        Ifd ifd0_;
        Ifd exifIfd_;
        Ifd iopIfd_;
        Ifd gpsIfd_;
        Ifd ifd1_;

        bool valid_;                      // Flag if data buffer is valid
        long size_;                       // Size of the Exif raw data in bytes
        char* data_;                      // Exif raw data buffer

    }; // class ExifData

// *****************************************************************************
// free functions

    /*!
      @brief Add all metadata in the range from iterator position begin to
             iterator position end, which have an IFD id matching that of the
             IFD to the list of directory entries of ifd.  No duplicate checks
             are performed, i.e., it is possible to add multiple metadata with
             the same key to an IFD.
     */
    void addToIfd(Ifd& ifd,
                  Metadata::const_iterator begin, 
                  Metadata::const_iterator end, 
                  ByteOrder byteOrder);
    /*!
      @brief Add the metadatum to the IFD.  No duplicate checks are performed,
             i.e., it is possible to add multiple metadata with the same key to
             an IFD.
     */
    void addToIfd(Ifd& ifd, const Metadatum& metadatum, ByteOrder byteOrder);
    /*!
      @brief Compare two metadata by tag. Return true if the tag of metadatum
             lhs is less than that of rhs.
     */
    bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs);
    /*!
      @brief Compare two metadata by key. Return true if the key of metadatum
             lhs is less than that of rhs.
     */
    bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs);
    /*!
      @brief Return a key for the entry.  The key is of the form
             'ifdItem.sectionName.tagName'.  This function knows about
             MakerNotes, i.e., it will invoke MakerNote::makeKey if necessary.
    */
    std::string makeKey(const Entry& entry);
    /*!
      @brief Return the tag and IFD id pair for the key. This function knows
             about MakerNotes, i.e., it will forward the request to
             MakerNote::decomposeKey if necessary.
      @return A pair consisting of the tag and IFD id.
      @throw Error ("Invalid key") if the key cannot be parsed into
      item item, section name and tag name parts.
    */
    std::pair<uint16, IfdId> decomposeKey(const std::string& key,
                                          const MakerNote* makerNote);

}                                       // namespace Exif

#endif                                  // #ifndef EXIF_HPP_
