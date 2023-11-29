// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_CRWIMAGE_INT_HPP
#define EXIV2_CRWIMAGE_INT_HPP

// *****************************************************************************
// included header files
#include "image.hpp"
#include "tags_int.hpp"

// + standard includes
#include <stack>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class declarations
class CiffHeader;
class CiffComponent;
struct CrwMapping;
struct CrwSubDir {
  uint16_t dir;
  uint16_t parent;
};

// *****************************************************************************
// type definitions

//! Function pointer for functions to decode Exif tags from a CRW entry
using CrwDecodeFct = void (*)(const CiffComponent&, const CrwMapping*, Image&, ByteOrder);

//! Function pointer for functions to encode CRW entries from Exif tags
using CrwEncodeFct = void (*)(const Image&, const CrwMapping*, CiffHeader*);

//! Stack to hold a path of CRW directories
using CrwDirs = std::stack<CrwSubDir>;

//! Type to identify where the data is stored in a directory
enum class DataLocId { valueData, directoryData, lastDataLocId };

// *****************************************************************************
// class definitions

/*!
  @brief Interface class for components of the CIFF directory hierarchy of a
         CRW (Canon Raw data) image. Both CIFF directories as well as
         entries implement this interface. This class is implemented as NVI
         (non-virtual interface).
 */
class CiffComponent {
 public:
  //! CiffComponent auto_ptr type
  using UniquePtr = std::unique_ptr<CiffComponent>;
  //! Container type to hold all metadata
  using Components = std::vector<CiffComponent*>;

  //! @name Creators
  //@{
  //! Default constructor
  CiffComponent() = default;
  //! Constructor taking a tag and directory
  CiffComponent(uint16_t tag, uint16_t dir);
  CiffComponent(const CiffComponent&) = delete;
  CiffComponent& operator=(const CiffComponent&) = delete;
  //! Virtual destructor.
  virtual ~CiffComponent() = default;
  //@}

  //! @name Manipulators
  //@{
  // Default assignment operator is fine

  //! Add a component to the composition
  void add(UniquePtr component);
  /*!
    @brief Add \em crwTagId to the parse tree, if it doesn't exist
           yet. \em crwDirs contains the path of subdirectories, starting
           with the root directory, leading to \em crwTagId. Directories
           that don't exist yet are added along the way. Returns a pointer
           to the newly added component.

    @param crwDirs   Subdirectory path from root to the subdirectory
                     containing the tag to be added.
    @param crwTagId  Tag to be added.

    @return A pointer to the newly added component.
   */
  CiffComponent* add(CrwDirs& crwDirs, uint16_t crwTagId);
  /*!
    @brief Remove \em crwTagId from the parse tree, if it exists yet. \em
           crwDirs contains the path of subdirectories, starting with the
           root directory, leading to \em crwTagId.

    @param crwDirs   Subdirectory path from root to the subdirectory
                     containing the tag to be removed.
    @param crwTagId  Tag to be removed.
   */
  void remove(CrwDirs& crwDirs, uint16_t crwTagId);
  /*!
    @brief Read a component from a data buffer

    @param pData     Pointer to the data buffer.
    @param size      Number of bytes in the data buffer.
    @param start     Component starts at \em pData + \em start.
    @param byteOrder Applicable byte order (little or big endian).

    @throw Error If the component cannot be parsed.
   */
  void read(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder);
  /*!
    @brief Write the metadata from the raw metadata component to the
           binary image \em blob. This method may append to the blob.

    @param blob      Binary image to add metadata to
    @param byteOrder Byte order
    @param offset    Current offset

    @return New offset
   */
  size_t write(Blob& blob, ByteOrder byteOrder, size_t offset);

  /*!
    @brief Writes the entry's value if size is larger than eight bytes. If
           needed, the value is padded with one 0 byte to make the number
           of bytes written to the blob even. The offset of the component
           is set to the offset passed in.
    @param blob The binary image to write to.
    @param offset Offset from the start of the directory for this entry.

    @return New offset.
   */
  size_t writeValueData(Blob& blob, size_t offset);

  //! Set the directory tag for this component.
  void setDir(uint16_t dir) {
    dir_ = dir;
  }
  //! Set the data value of the entry.
  void setValue(DataBuf&& buf);
  //@}

  //! Return the type id for a tag
  static TypeId typeId(uint16_t tag);
  //! Return the data location id for a tag
  static DataLocId dataLocation(uint16_t tag);

  //! @name Accessors
  //@{
  /*!
    @brief Decode metadata from the component and add it to
           \em image.

    @param image Image to add metadata to
    @param byteOrder Byte order
   */
  void decode(Image& image, ByteOrder byteOrder) const;
  /*!
    @brief Print debug info about a component to \em os.

    @param os Output stream to write to
    @param byteOrder Byte order
    @param prefix Prefix to be written before each line of output
   */
  void print(std::ostream& os, ByteOrder byteOrder, const std::string& prefix = "") const;
  /*!
    @brief Write a directory entry for the component to the \em blob.
           If the size of the data is not larger than 8 bytes, the
           data is written to the directory entry.
   */
  void writeDirEntry(Blob& blob, ByteOrder byteOrder) const;
  //! Return the tag of the directory containing this component
  [[nodiscard]] uint16_t dir() const {
    return dir_;
  }

  //! Return the tag of this component
  [[nodiscard]] uint16_t tag() const {
    return tag_;
  }

  //! Return true if the component is empty, else false
  [[nodiscard]] bool empty() const;

  /*!
    @brief Return the data size of this component

    @note If the data is contained in the directory entry itself,
          this method returns 8, which is the maximum number
          of data bytes this component can have. The actual size,
          i.e., used data bytes, may be less than 8.
   */
  [[nodiscard]] size_t size() const {
    return size_;
  }

  //! Return the offset to the data from the start of the directory
  [[nodiscard]] size_t offset() const {
    return offset_;
  }

  //! Return a pointer to the data area of this component
  [[nodiscard]] const byte* pData() const {
    return pData_;
  }

  //! Return the tag id of this component
  [[nodiscard]] uint16_t tagId() const {
    return tag_ & 0x3fff;
  }

  //! Return the type id of this component
  [[nodiscard]] TypeId typeId() const {
    return typeId(tag_);
  }

  //! Return the data location for this component
  [[nodiscard]] DataLocId dataLocation() const {
    return dataLocation(tag_);
  }

  /*!
    @brief Finds \em crwTagId in directory \em crwDir, returning a pointer to
           the component or 0 if not found.
   */
  [[nodiscard]] CiffComponent* findComponent(uint16_t crwTagId, uint16_t crwDir) const;
  //@}

 protected:
  //! @name Manipulators
  //@{
  //! Implements add()
  virtual void doAdd(UniquePtr component) = 0;
  //! Implements add(). The default implementation does nothing.
  virtual CiffComponent* doAdd(CrwDirs& crwDirs, uint16_t crwTagId);
  //! Implements remove(). The default implementation does nothing.
  virtual void doRemove(CrwDirs& crwDirs, uint16_t crwTagId);
  //! Implements read(). The default implementation reads a directory entry.
  virtual void doRead(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder);
  //! Implements write()
  virtual size_t doWrite(Blob& blob, ByteOrder byteOrder, size_t offset) = 0;
  //! Set the size of the data area.
  void setSize(size_t size) {
    size_ = size;
  }
  //! Set the offset for this component.
  void setOffset(size_t offset) {
    offset_ = offset;
  }
  //@}

  //! @name Accessors
  //@{
  //! Implements decode()
  virtual void doDecode(Image& image, ByteOrder byteOrder) const = 0;
  //! Implements print(). The default implementation prints the entry.
  virtual void doPrint(std::ostream& os, ByteOrder byteOrder, const std::string& prefix) const;
  //! Implements empty(). Default implementation returns true if size is 0.
  [[nodiscard]] virtual bool doEmpty() const;
  //! Implements findComponent(). The default implementation checks the entry.
  [[nodiscard]] virtual CiffComponent* doFindComponent(uint16_t crwTagId, uint16_t crwDir) const;
  //@}

 private:
  // DATA
  uint16_t dir_ = 0;   //!< Tag of the directory containing this component
  uint16_t tag_ = 0;   //!< Tag of the entry
  size_t size_ = 0;    //!< Size of the data area
  size_t offset_ = 0;  //!< Offset to the data area from start of dir

  // Notes on the ownership model of pData_: pData_ is a always a read-only
  // pointer to a buffer owned by somebody else. Usually it is a pointer
  // into a copy of the image that was memory-mapped in CrwImage::readMetadata().
  // However, if CiffComponent::setValue() is used, then it is a pointer into
  // the storage_ DataBuf below.
  const byte* pData_ = nullptr;  //!< Pointer to the data area

  // This DataBuf is only used when CiffComponent::setValue is called.
  // Otherwise, it remains empty.
  DataBuf storage_;
};  // class CiffComponent

/*!
  @brief This class models one directory entry of a CIFF directory of
         a CRW (Canon Raw data) image.
 */
class CiffEntry : public CiffComponent {
  using CiffComponent::CiffComponent;
  //! @name Manipulators
  //@{
  using CiffComponent::doAdd;
  // See base class comment
  void doAdd(UniquePtr component) override;
  /*!
    @brief Implements write(). Writes only the value data of the entry,
           using writeValueData().
   */
  size_t doWrite(Blob& blob, ByteOrder byteOrder, size_t offset) override;
  //@}

  //! @name Accessors
  //@{
  // See base class comment
  void doDecode(Image& image, ByteOrder byteOrder) const override;
  //@}

};  // class CiffEntry

//! This class models a CIFF directory of a CRW (Canon Raw data) image.
class CiffDirectory : public CiffComponent {
  using CiffComponent::CiffComponent;

 public:
  //! @name Creators
  //@{
  //! Virtual destructor
  ~CiffDirectory() override;
  //@}

  CiffDirectory(const CiffDirectory&) = delete;
  CiffDirectory& operator=(const CiffDirectory&) = delete;

  //! @name Manipulators
  //@{
  // Default assignment operator is fine

  /*!
    @brief Parse a CIFF directory from a memory buffer

    @param pData     Pointer to the memory buffer containing the directory
    @param size      Size of the memory buffer
    @param byteOrder Applicable byte order (little or big endian)
   */
  void readDirectory(const byte* pData, size_t size, ByteOrder byteOrder);
  //@}

 private:
  //! @name Manipulators
  //@{
  // See base class comment
  void doAdd(UniquePtr component) override;
  // See base class comment
  CiffComponent* doAdd(CrwDirs& crwDirs, uint16_t crwTagId) override;
  // See base class comment
  void doRemove(CrwDirs& crwDirs, uint16_t crwTagId) override;
  /*!
    @brief Implements write(). Writes the complete Ciff directory to
           the blob.
   */
  size_t doWrite(Blob& blob, ByteOrder byteOrder, size_t offset) override;
  // See base class comment
  void doRead(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder) override;
  //@}

  //! @name Accessors
  //@{
  // See base class comment
  void doDecode(Image& image, ByteOrder byteOrder) const override;

  // See base class comment
  void doPrint(std::ostream& os, ByteOrder byteOrder, const std::string& prefix) const override;

  //! See base class comment. A directory is empty if it has no components.
  [[nodiscard]] bool doEmpty() const override;

  // See base class comment
  [[nodiscard]] CiffComponent* doFindComponent(uint16_t crwTagId, uint16_t crwDir) const override;
  //@}

  // DATA
  Components components_;  //!< List of components in this dir
  UniquePtr m_;            // used by recursive doAdd
  CiffComponent* cc_ = nullptr;

};  // class CiffDirectory

/*!
  @brief This class models the header of a CRW (Canon Raw data) image.  It
         is the head of a CIFF parse tree, consisting of CiffDirectory and
         CiffEntry objects. Most of its methods will walk the parse tree to
         perform the requested action.
 */
class CiffHeader {
 public:
  //! CiffHeader auto_ptr type
  using UniquePtr = std::unique_ptr<CiffHeader>;

  //! @name Creators
  //@{
  //! Default constructor
  CiffHeader() = default;
  //! Virtual destructor
  virtual ~CiffHeader() = default;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Read the CRW image from a data buffer, starting with the Ciff
           header.

    @param pData Pointer to the data buffer.
    @param size  Number of bytes in the data buffer.

    @throw Error If the image cannot be parsed.
   */
  void read(const byte* pData, size_t size);
  /*!
    @brief Set the value of entry \em crwTagId in directory \em crwDir to
           \em buf. If this tag doesn't exist, it is added along with all
           directories needed.

    @param crwTagId Tag to be added.
    @param crwDir   Parent directory of the tag.
    @param buf      Value to be set.
   */
  void add(uint16_t crwTagId, uint16_t crwDir, DataBuf&& buf);
  /*!
    @brief Remove entry \em crwTagId in directory \em crwDir from the parse
           tree. If it's the last entry in the directory, the directory is
           removed as well, etc.

    @param crwTagId Tag id to be removed.
    @param crwDir   Parent directory of the tag.
   */
  void remove(uint16_t crwTagId, uint16_t crwDir) const;
  //@}

  //! Return a pointer to the Canon CRW signature.
  static const char* signature() {
    return signature_;
  }

  //! @name Accessors
  //@{
  /*!
    @brief Write the CRW image to the binary image \em blob, starting with
           the Ciff header. This method appends to the blob.

    @param blob Binary image to add to.

    @throw Error If the image cannot be written.
   */
  void write(Blob& blob) const;
  /*!
    @brief Decode the CRW image and add it to \em image.

    Walk the parse tree and convert CIFF entries to metadata
    entries which are added to \em image.

    @param image Image to add metadata to
   */
  void decode(Image& image) const;

  //! Return the byte order (little or big endian).
  [[nodiscard]] ByteOrder byteOrder() const {
    return byteOrder_;
  }
  /*!
    @brief Finds \em crwTagId in directory \em crwDir in the parse tree,
           returning a pointer to the component or 0 if not found.
   */
  [[nodiscard]] CiffComponent* findComponent(uint16_t crwTagId, uint16_t crwDir) const;
  //@}

 private:
  // DATA
  static constexpr auto signature_ = "HEAPCCDR";  //!< Canon CRW signature

  std::unique_ptr<CiffDirectory> pRootDir_;  //!< Pointer to the root directory
  ByteOrder byteOrder_ = littleEndian;       //!< Applicable byte order
  uint32_t offset_ = 0;                      //!< Offset to the start of the root dir
  std::vector<byte> pPadding_;               //!< the (unknown) remainder
  uint32_t padded_ = 0;                      //!< Number of padding-bytes

};  // class CiffHeader

/*!
  @brief Structure for a mapping table for conversion of CIFF entries to
         image metadata and vice versa.
 */
struct CrwMapping {
  uint16_t crwTagId_;      //!< CRW tag id
  uint16_t crwDir_;        //!< CRW directory tag
  uint32_t size_;          //!< Data size (overwrites the size from the entry)
  uint16_t tag_;           //!< Exif tag to map to
  IfdId ifdId_;            //!< Exif Ifd id to map to
  CrwDecodeFct toExif_;    //!< Conversion function
  CrwEncodeFct fromExif_;  //!< Reverse conversion function
};

/*!
  @brief Static class providing mapping functionality from CRW entries
         to image metadata and vice versa
 */
class CrwMap {
 public:
  /*!
    @brief Decode image metadata from a CRW entry, convert and add it
           to the image metadata. This function converts only one CRW
           component.

    @param ciffComponent Source CIFF entry
    @param image         Destination image for the metadata
    @param byteOrder     Byte order in which the data of the entry
                         is encoded
   */
  static void decode(const CiffComponent& ciffComponent, Image& image, ByteOrder byteOrder);
  /*!
    @brief Encode image metadata from \em image into the CRW parse tree.
           This function converts all Exif metadata that %Exiv2 can
           convert to CRW format, in a loop through the entries of the
           mapping table.

    @param pHead         Destination parse tree.
    @param image         Source image containing the metadata.
   */
  static void encode(CiffHeader* pHead, const Image& image);

  /*!
    @brief Load the stack: loop through the CRW subdirs hierarchy and push
           all directories on the path from \em crwDir to root onto the
           stack \em crwDirs. Requires the subdirs array to be arranged in
           bottom-up order to be able to finish in only one pass.
   */
  static void loadStack(CrwDirs& crwDirs, uint16_t crwDir);

 private:
  //! Return conversion information for one \em crwDir and \em crwTagId
  static const CrwMapping* crwMapping(uint16_t crwDir, uint16_t crwTagId);

  /*!
    @brief Standard decode function to convert CRW entries to
           Exif metadata.

    Uses the mapping defined in the conversion structure \em pCrwMapping
    to convert the data. If the \em size field in the conversion structure
    is not 0, then it is used instead of the \em size provided by the
    entry itself.
   */
  static void decodeBasic(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                          ByteOrder byteOrder);

  //! Decode the user comment
  static void decode0x0805(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                           ByteOrder byteOrder);

  //! Decode camera Make and Model information
  static void decode0x080a(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                           ByteOrder byteOrder);

  //! Decode Canon Camera Settings 1, 2 and Custom Function arrays
  static void decodeArray(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                          ByteOrder byteOrder);

  //! Decode the date when the picture was taken
  static void decode0x180e(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                           ByteOrder byteOrder);

  //! Decode image width and height
  static void decode0x1810(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                           ByteOrder byteOrder);

  //! Decode the thumbnail image
  static void decode0x2008(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                           ByteOrder byteOrder);

  /*!
    @brief Standard encode function to convert Exif metadata to Crw
           entries.

    This is the basic encode function taking one Exif key and converting
    it to one Ciff entry. Both are available in the \em pCrwMapping passed
    in.

    @param image Image with the metadata to encode
    @param pCrwMapping Pointer to an entry into the \em crwMapping_ table
                 with information on the source and target metadata entries.
    @param pHead Pointer to the head of the CIFF parse tree into which
                 the metadata from \em image is encoded.
   */
  static void encodeBasic(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode the user comment
  static void encode0x0805(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode camera Make and Model information
  static void encode0x080a(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode Canon Camera Settings 1, 2 and Custom Function arrays
  static void encodeArray(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode the date when the picture was taken
  static void encode0x180e(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode image width and height
  static void encode0x1810(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  //! Encode the thumbnail image
  static void encode0x2008(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead);

  // DATA
  static const CrwMapping crwMapping_[];  //!< Metadata conversion table
  static const CrwSubDir crwSubDir_[];    //!< Ciff directory hierarchy

};  // class CrwMap

// *****************************************************************************
// template, inline and free functions

/*!
  @brief Pack the tag values of all \em ifdId tags in \em exifData into a
         data buffer. This function is used to pack Canon Camera Settings1,2
         and Custom Function tags.
 */
DataBuf packIfdId(const ExifData& exifData, IfdId ifdId, ByteOrder byteOrder);

}  // namespace Exiv2::Internal

#endif  // EXIV2_CRWIMAGE_INT_HPP
