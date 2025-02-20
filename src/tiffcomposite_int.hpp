// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TIFFCOMPOSITE_INT_HPP_
#define TIFFCOMPOSITE_INT_HPP_

// *****************************************************************************
// included header files
#include "tifffwd_int.hpp"

#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
class BasicIo;

namespace Internal {
// *****************************************************************************
// class definitions

//! TIFF value type.
using TiffType = uint16_t;

const TiffType ttUnsignedByte = 1;      //!< Exif BYTE type
const TiffType ttAsciiString = 2;       //!< Exif ASCII type
const TiffType ttUnsignedShort = 3;     //!< Exif SHORT type
const TiffType ttUnsignedLong = 4;      //!< Exif LONG type
const TiffType ttUnsignedRational = 5;  //!< Exif RATIONAL type
const TiffType ttSignedByte = 6;        //!< Exif SBYTE type
const TiffType ttUndefined = 7;         //!< Exif UNDEFINED type
const TiffType ttSignedShort = 8;       //!< Exif SSHORT type
const TiffType ttSignedLong = 9;        //!< Exif SLONG type
const TiffType ttSignedRational = 10;   //!< Exif SRATIONAL type
const TiffType ttTiffFloat = 11;        //!< TIFF FLOAT type
const TiffType ttTiffDouble = 12;       //!< TIFF DOUBLE type
const TiffType ttTiffIfd = 13;          //!< TIFF IFD type

//! Convert the \em tiffType of a \em tag and \em group to an Exiv2 \em typeId.
TypeId toTypeId(TiffType tiffType, uint16_t tag, IfdId group);
//! Convert the %Exiv2 \em typeId to a TIFF value type.
TiffType toTiffType(TypeId typeId);

/*!
  Special TIFF tags for the use in TIFF structures only
*/
namespace Tag {
const uint32_t none = 0x10000;   //!< Dummy tag
const uint32_t root = 0x20000;   //!< Special tag: root IFD
const uint32_t next = 0x30000;   //!< Special tag: next IFD
const uint32_t all = 0x40000;    //!< Special tag: all tags in a group
const uint32_t pana = 0x80000;   //!< Special tag: root IFD of Panasonic RAW images
const uint32_t fuji = 0x100000;  //!< Special tag: root IFD of Fujifilm RAF images
const uint32_t cmt2 = 0x110000;  //!< Special tag: root IFD of CR3 images
const uint32_t cmt3 = 0x120000;  //!< Special tag: root IFD of CR3 images
const uint32_t cmt4 = 0x130000;  //!< Special tag: root IFD of CR3 images
}  // namespace Tag

/*!
  @brief A tuple consisting of extended Tag and group used as an item in
         TIFF paths.
*/
class TiffPathItem {
 public:
  //! @name Creators
  //@{
  //! Constructor
  constexpr TiffPathItem(uint32_t extendedTag, IfdId group) : extendedTag_(extendedTag), group_(group) {
  }
  //@}

  //! @name Accessors
  //@{
  //! Return the tag corresponding to the extended tag
  [[nodiscard]] uint16_t tag() const {
    return static_cast<uint16_t>(extendedTag_ & 0xffff);
  }
  //! Return the extended tag (32 bit so that it can contain special tags)
  [[nodiscard]] uint32_t extendedTag() const {
    return extendedTag_;
  }
  //! Return the group
  [[nodiscard]] IfdId group() const {
    return group_;
  }
  //@}

 private:
  // DATA
  uint32_t extendedTag_;
  IfdId group_;
};

/*!
  @brief Simple IO wrapper to ensure that the header is only written if there is
         any other data at all.

  The wrapper is initialized with an IO reference and a pointer to a TIFF header.
  Subsequently the wrapper is used by all TIFF write methods. It takes care that
  the TIFF header is written to the IO first before any other output and only if
  there is any other data.
 */
class IoWrapper {
 public:
  //! @name Creators
  //@{
  /*!
    brief Constructor.

    The IO wrapper owns none of the objects passed in so the caller is
    responsible to keep them alive.
   */
  IoWrapper(BasicIo& io, const byte* pHeader, size_t size, OffsetWriter* pow);
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Wraps the corresponding BasicIo::write() method.

    Writes the TIFF header to the IO, if it hasn't been written yet, followed
    by the data passed in the arguments.
   */
  size_t write(const byte* pData, size_t wcount);
  /*!
    @brief Wraps the corresponding BasicIo::putb() method.

    Writes the TIFF header to the IO, if it hasn't been written yet, followed
    by the data passed in the argument.
   */
  int putb(byte data);
  //! Wrapper for OffsetWriter::setTarget(), using an int instead of the enum to reduce include deps
  void setTarget(int id, size_t target);
  //@}

 private:
  // DATA
  BasicIo& io_;              //! Reference for the IO instance.
  const byte* pHeader_;      //! Pointer to the header data.
  size_t size_;              //! Size of the header data.
  bool wroteHeader_{false};  //! Indicates if the header has been written.
  OffsetWriter* pow_;        //! Pointer to an offset-writer, if any, or 0
};

/*!
  @brief Interface class for components of a TIFF directory hierarchy
         (Composite pattern).  Both TIFF directories as well as entries
         implement this interface.  A component can be uniquely identified
         by a tag, group tuple.  This class is implemented as a NVI
         (Non-Virtual Interface) and it has an interface for visitors
         (Visitor pattern) to perform operations on all components.
 */
class TiffComponent {
 public:
  //! TiffComponent auto_ptr type
  using UniquePtr = std::unique_ptr<TiffComponent>;
  //! Container type to hold all metadata
  using Components = std::vector<UniquePtr>;

  //! @name Creators
  //@{
  //! Constructor
  constexpr TiffComponent(uint16_t tag, IfdId group) : tag_(tag), group_(group) {
  }
  //! Virtual destructor.
  virtual ~TiffComponent() = default;
  TiffComponent(const TiffComponent&) = default;
  TiffComponent& operator=(const TiffComponent&) = default;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Add a TIFF entry \em tag to the component. Components on
           the path to the entry are added if they don't exist yet.

    @param tag      The tag of the new entry
    @param tiffPath A path from the TIFF root element to a TIFF entry.
    @param pRoot    Pointer to the root component of the TIFF composite.
    @param object   TIFF component to add. If 0, the correct entry will be
                    created.

    @return A pointer to the newly added TIFF entry.
   */
  TiffComponent* addPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot, UniquePtr object = nullptr);
  /*!
    @brief Add a child to the component. Default is to do nothing.
    @param tiffComponent Auto pointer to the component to add.
    @return Return a pointer to the newly added child element or 0.
   */
  TiffComponent* addChild(UniquePtr tiffComponent);
  /*!
      @brief Add a "next" component to the component. Default is to do
             nothing.
      @param tiffComponent Auto pointer to the component to add.
      @return Return a pointer to the newly added "next" element or 0.
   */
  TiffComponent* addNext(UniquePtr tiffComponent);
  /*!
    @brief Interface to accept visitors (Visitor pattern). Visitors
           can perform operations on all components of the composite.

    @param visitor The visitor.
   */
  void accept(TiffVisitor& visitor);
  /*!
    @brief Set a pointer to the start of the binary representation of the
           component in a memory buffer. The buffer must be allocated and
           freed outside of this class.
   */
  void setStart(const byte* pStart) {
    pStart_ = const_cast<byte*>(pStart);
  }
  /*!
    @brief Write a TiffComponent to a binary image.

    @param ioWrapper  IO wrapper to which the TiffComponent is written.
    @param byteOrder  Applicable byte order (little or big endian).
    @param offset     Offset from the start of the image (TIFF header) to
                      the component.
    @param valueIdx   Index of the component to be written relative to offset.
    @param dataIdx    Index of the data area of the component relative to offset.
    @param imageIdx   Index of the image data area relative to offset.
    @return           Number of bytes written to the IO wrapper including all
                      nested components.
    @throw            Error If the component cannot be written.
   */
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
               size_t& imageIdx);
  //@}

  //! @name Accessors
  //@{
  //! Return the tag of this entry.
  [[nodiscard]] uint16_t tag() const {
    return tag_;
  }
  //! Return the group id of this component
  [[nodiscard]] IfdId group() const {
    return group_;
  }
  //! Return a pointer to the start of the binary representation of the component
  [[nodiscard]] byte* start() const {
    return pStart_;
  }
  /*!
    @brief Return an auto-pointer to a copy of itself (deep copy, but
           without any children). The caller owns this copy and the
           auto-pointer ensures that it will be deleted.
   */
  [[nodiscard]] UniquePtr clone() const;
  /*!
    @brief Write the IFD data of this component to a binary image.
           Return the number of bytes written. Components derived from
           TiffEntryBase implement this method if needed.
   */
  size_t writeData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx, size_t& imageIdx) const;
  /*!
    @brief Write the image data of this component to a binary image.
           Return the number of bytes written. TIFF components implement
           this method if needed.
   */
  size_t writeImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
  /*!
    @brief Return the size in bytes of the IFD value of this component
           when written to a binary image.
   */
  [[nodiscard]] size_t size() const;
  /*!
    @brief Return the number of components in this component.
   */
  [[nodiscard]] size_t count() const;
  /*!
    @brief Return the size in bytes of the IFD data of this component when
           written to a binary image.  This is a support function for
           write(). Components derived from TiffEntryBase implement this
           method corresponding to their implementation of writeData().
   */
  [[nodiscard]] size_t sizeData() const;
  /*!
    @brief Return the size in bytes of the image data of this component
           when written to a binary image.  This is a support function for
           write(). TIFF components implement this method corresponding to
           their implementation of writeImage().
   */
  [[nodiscard]] size_t sizeImage() const;
  /*!
    @brief Return the unique id of the entry in the image.
   */
  // Todo: This is only implemented in TiffEntryBase. It is needed here so that
  //       we can sort components by tag and idx. Something is not quite right.
  [[nodiscard]] virtual int idx() const;
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  //! Implements addPath(). The default implementation does nothing.
  virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                   TiffComponent::UniquePtr object);
  //! Implements addChild(). The default implementation does nothing.
  virtual TiffComponent* doAddChild(UniquePtr tiffComponent);
  //! Implements addNext(). The default implementation does nothing.
  virtual TiffComponent* doAddNext(UniquePtr tiffComponent);
  //! Implements accept().
  virtual void doAccept(TiffVisitor& visitor) = 0;
  //! Implements write().
  virtual size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                         size_t& imageIdx) = 0;
  //@}

  //! @name Protected Accessors
  //@{
  //! Internal virtual copy constructor, implements clone().
  [[nodiscard]] virtual TiffComponent* doClone() const = 0;
  //! Implements writeData().
  virtual size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                             size_t& imageIdx) const = 0;
  //! Implements writeImage().
  virtual size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const = 0;
  //! Implements size().
  [[nodiscard]] virtual size_t doSize() const = 0;
  //! Implements count().
  [[nodiscard]] virtual size_t doCount() const = 0;
  //! Implements sizeData().
  [[nodiscard]] virtual size_t doSizeData() const = 0;
  //! Implements sizeImage().
  [[nodiscard]] virtual size_t doSizeImage() const = 0;
  //@}

 private:
  // DATA
  uint16_t tag_;  //!< Tag that identifies the component
  IfdId group_;   //!< Group id for this component
  /*!
    Pointer to the start of the binary representation of the component in
    a memory buffer. The buffer is allocated and freed outside of this class.
   */
  byte* pStart_{};
};

//! TIFF mapping table for functions to decode special cases
struct TiffMappingInfo {
  struct Key;
  /*!
    @brief Compare a TiffMappingInfo with a TiffMappingInfo::Key.
           The two are equal if TiffMappingInfo::make_ equals a substring
           of the key of the same size. E.g., mapping info = "OLYMPUS",
           key = "OLYMPUS OPTICAL CO.,LTD" (found in the image) match,
           the extendedTag is Tag::all or equal to the extended tag of the
           key, and the group is equal to that of the key.
   */
  bool operator==(const Key& key) const;
  //! Return the tag corresponding to the extended tag
  [[nodiscard]] uint16_t tag() const {
    return static_cast<uint16_t>(extendedTag_ & 0xffff);
  }

  // DATA
  const char* make_;       //!< Camera make for which these mapping functions apply
  uint32_t extendedTag_;   //!< Tag (32 bit so that it can contain special tags)
  IfdId group_;            //!< Group that contains the tag
  DecoderFct decoderFct_;  //!< Decoder function for matching tags
  EncoderFct encoderFct_;  //!< Encoder function for matching tags

};  // struct TiffMappingInfo

//! Search key for TIFF mapping structures.
struct TiffMappingInfo::Key {
  std::string m_;  //!< Camera make
  uint32_t e_;     //!< Extended tag
  IfdId g_;        //!< %Group
};

/*!
  @brief This abstract base class provides the common functionality of an
         IFD directory entry and defines an extended interface for derived
         concrete entries, which allows access to the attributes of the
         entry.
 */
class TiffEntryBase : public TiffComponent {
  friend class TiffReader;
  friend class TiffEncoder;

 public:
  //! @name Creators
  //@{
  //! Default constructor.
  constexpr TiffEntryBase(uint16_t tag, IfdId group, TiffType tiffType = ttUndefined) :
      TiffComponent(tag, group), tiffType_(tiffType) {
  }

  //! Virtual destructor.
  ~TiffEntryBase() override = default;
  //@}

  //! @name NOT implemented
  //@{
  //! Assignment operator.
  TiffEntryBase& operator=(const TiffEntryBase&) = delete;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Encode a TIFF component from the metadatum provided and
           information from the \em encoder as needed.

    Implemented as double-dispatch calls back to one of the specific
    encoding functions at the \em encoder.
   */
  void encode(TiffEncoder& encoder, const Exifdatum* datum);
  //! Set the offset
  void setOffset(size_t offset) {
    offset_ = offset;
  }
  /*!
    @brief Set pointer and size of the entry's data (not taking ownership of the data).

    @param storage Usually, pData is a pointer into a copy of the image file, which
                   means that it points to memory which is guaranteed to live longer
                   than this class. However, sometimes pData is pointer into a
                   DataBuf that was allocated by another node in the component tree.
                   If so, we need to make sure that the DataBuf doesn't get freed too
                   early. We use a std::shared_ptr to hold a reference to the DataBuf
                   to ensure that it will be kept alive. The storage parameter is
                   assigned to the storage_ field. In the more common scenario where
                   pData points to a copy of the image, rather than a DataBuf, then
                   you should pass std::shared_ptr<DataBuf>(), which is essentially
                   a nullptr.
   */
  void setData(byte* pData, size_t size, std::shared_ptr<DataBuf> storage);
  /*!
    @brief Set the entry's data buffer. A shared_ptr is used to manage the DataBuf
           because TiffEntryBase has a clone method so it is possible (in theory) for
           the DataBuf to have multiple owners.
   */
  void setData(std::shared_ptr<DataBuf> buf);
  /*!
   @brief Update the value. Takes ownership of the pointer passed in.

   Update binary value data and call setValue().
 */
  void updateValue(Value::UniquePtr value, ByteOrder byteOrder);
  /*!
    @brief Set tag value. Takes ownership of the pointer passed in.

    Update type, count and the pointer to the value.
  */
  void setValue(Value::UniquePtr value);
  //@}

  //! @name Accessors
  //@{
  //! Return the TIFF type
  [[nodiscard]] TiffType tiffType() const {
    return tiffType_;
  }
  /*!
    @brief Return the offset to the data area relative to the base
           for the component (usually the start of the TIFF header)
   */
  [[nodiscard]] size_t offset() const {
    return offset_;
  }
  /*!
    @brief Return the unique id of the entry in the image
   */
  [[nodiscard]] int idx() const override;
  /*!
    @brief Return a pointer to the binary representation of the
           value of this component.
   */
  [[nodiscard]] const byte* pData() const {
    return pData_;
  }
  //! Return a const pointer to the converted value of this component
  [[nodiscard]] const Value* pValue() const {
    return pValue_.get();
  }
  //@}

 protected:
  //! @name Protected Creators
  //@{
  //! Copy constructor (used to implement clone()).
  TiffEntryBase(const TiffEntryBase& rhs);
  //@}

  //! @name Protected Manipulators
  //@{
  //! Implements encode().
  virtual void doEncode(TiffEncoder& encoder, const Exifdatum* datum) = 0;
  //! Set the number of components in this entry
  void setCount(size_t count) {
    count_ = count;
  }
  //! Set the unique id of the entry in the image
  void setIdx(int idx) {
    idx_ = idx;
  }
  /*!
    @brief Implements write(). Write the value of a standard TIFF entry to
           the \em ioWrapper, return the number of bytes written. Only the
           \em ioWrapper and \em byteOrder arguments are used.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  //! Implements count().
  [[nodiscard]] size_t doCount() const override;
  /*!
    @brief Implements writeData(). Standard TIFF entries have no data:
           write nothing and return 0.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  /*!
    @brief Implements writeImage(). Standard TIFF entries have no image data:
           write nothing and return 0.
   */
  size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  //! Implements size(). Return the size of a standard TIFF entry
  [[nodiscard]] size_t doSize() const override;
  //! Implements sizeData(). Return 0.
  [[nodiscard]] size_t doSizeData() const override;
  //! Implements sizeImage(). Return 0.
  [[nodiscard]] size_t doSizeImage() const override;
  //@}

  //! Helper function to write an \em offset to a preallocated binary buffer
  static size_t writeOffset(byte* buf, size_t offset, TiffType tiffType, ByteOrder byteOrder);

  //! Used (internally) to create another reference to the DataBuf reference by storage_.
  [[nodiscard]] std::shared_ptr<DataBuf> storage() const {
    return storage_;
  }

 private:
  // DATA
  TiffType tiffType_;  //!< Field TIFF type
  size_t count_{};     //!< The number of values of the indicated type
  size_t offset_{};    //!< Offset to the data area
  size_t size_{};      //!< Size of the data buffer holding the value in bytes, there is no minimum size.

  // Notes on the ownership model of pData_: pData_ is a always a
  // pointer to a buffer owned by somebody else. Usually it is a
  // pointer into a copy of the image file, but if
  // TiffEntryBase::setData is used then it is a pointer into the
  // storage_ DataBuf below.
  byte* pData_{};  //!< Pointer to the data area

  int idx_{};                      //!< Unique id of the entry in the image
  std::unique_ptr<Value> pValue_;  //!< Converted data value

  // This DataBuf is only used when TiffEntryBase::setData is called.
  // Otherwise, it remains empty. It is wrapped in a shared_ptr because
  // TiffEntryBase has a clone method, which could lead to the DataBuf
  // having multiple owners.
  std::shared_ptr<DataBuf> storage_;
};

/*!
  @brief A standard TIFF IFD entry.
 */
class TiffEntry : public TiffEntryBase {
 public:
  //! @name Creators
  //@{
  //! Constructor
  constexpr TiffEntry(uint16_t tag, IfdId group) : TiffEntryBase(tag, group) {
  }
  //@}

 protected:
  //! @name Manipulators
  //@{
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffEntry* doClone() const override;
  //@}
};

/*!
  @brief Interface for a standard TIFF IFD entry consisting of a value
         which is a set of offsets to a data area. The sizes of these "strips"
         are provided in a related TiffSizeEntry, tag and group of which are
         set in the constructor. The implementations of this interface differ
         in whether the data areas are extracted to the higher level metadata
         (TiffDataEntry) or not (TiffImageEntry).
 */
class TiffDataEntryBase : public TiffEntryBase {
 public:
  //! @name Creators
  //@{
  //! Constructor
  TiffDataEntryBase(uint16_t tag, IfdId group, uint16_t szTag, IfdId szGroup);
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Set the data areas ("strips").

    @param pSize Pointer to the Value holding the sizes corresponding
                 to this data entry.
    @param pData Pointer to the data area.
    @param sizeData Size of the data area.
    @param baseOffset Base offset into the data area.
   */
  virtual void setStrips(const Value* pSize, const byte* pData, size_t sizeData, size_t baseOffset) = 0;
  //@}

  //! @name Accessors
  //@{
  //! Return the group of the entry which has the size
  [[nodiscard]] uint16_t szTag() const {
    return szTag_;
  }
  //! Return the group of the entry which has the size
  [[nodiscard]] IfdId szGroup() const {
    return szGroup_;
  }
  //@}

 private:
  // DATA
  uint16_t szTag_;  //!< Tag of the entry with the size
  IfdId szGroup_;   //!< Group of the entry with the size
};

/*!
  @brief A standard TIFF IFD entry consisting of a value which is an offset
         to a data area and the data area. The size of the data area is
         provided in a related TiffSizeEntry, tag and group of which are set
         in the constructor.

         This component extracts the data areas ("strips") and makes them
         available in the higher level metadata. It is used, e.g., for
         \em Exif.Thumbnail.JPEGInterchangeFormat for which the size
         is provided in \em Exif.Thumbnail.JPEGInterchangeFormatLength.
 */
class TiffDataEntry : public TiffDataEntryBase {
  friend class TiffEncoder;

 public:
  using TiffDataEntryBase::TiffDataEntryBase;

  //! @name Manipulators
  //@{
  void setStrips(const Value* pSize, const byte* pData, size_t sizeData, size_t baseOffset) override;
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write(). Write pointers into the data area to the
           \em ioWrapper, relative to the offsets in the value. Return the
           number of bytes written. The \em valueIdx argument is not used.

    The number of components in the value determines how many offsets are
    written. Set the first value to 0, the second to the size of the first
    data area, etc. when creating a new data entry. Offsets will be adjusted
    on write. The type of the value can only be signed or unsigned short or
    long.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffDataEntry* doClone() const override;
  /*!
    @brief Implements writeData(). Write the data area to the \em ioWrapper.
           Return the number of bytes written.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  // Using doWriteImage from base class
  // Using doSize() from base class
  //! Implements sizeData(). Return the size of the data area.
  [[nodiscard]] size_t doSizeData() const override;
  // Using doSizeImage from base class
  //@}

 private:
  // DATA
  byte* pDataArea_{};      //!< Pointer to the data area (never alloc'd)
  size_t sizeDataArea_{};  //!< Size of the data area
};

/*!
  @brief A standard TIFF IFD entry consisting of a value which is an array
         of offsets to image data areas. The sizes of the image data areas are
         provided in a related TiffSizeEntry, tag and group of which are set
         in the constructor.

         The data is not extracted into the higher level metadata tags, it is
         only copied to the target image when the image is written.
         This component is used, e.g., for
         \em Exif.Image.StripOffsets for which the sizes are provided in
         \em Exif.Image.StripByteCounts.
 */
class TiffImageEntry : public TiffDataEntryBase {
  friend class TiffEncoder;
  using TiffDataEntryBase::TiffDataEntryBase;

 public:
  //! @name Manipulators
  //@{
  void setStrips(const Value* pSize, const byte* pData, size_t sizeData, size_t baseOffset) override;
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write(). Write pointers into the image data area to the
           \em ioWrapper. Return the number of bytes written. The \em valueIdx
           and \em dataIdx  arguments are not used.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffImageEntry* doClone() const override;
  /*!
    @brief Implements writeData(). Write the image data area to the \em ioWrapper.
           Return the number of bytes written.

    This function writes the image data to the data area of the current
    directory. It is used for TIFF image entries in the makernote (large
    preview images) so that the image data remains in the makernote IFD.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  /*!
    @brief Implements writeImage(). Write the image data area to the \em ioWrapper.
           Return the number of bytes written.
   */
  size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  //! Implements size(). Return the size of the strip pointers.
  [[nodiscard]] size_t doSize() const override;
  //! Implements sizeData(). Return the size of the image data area.
  [[nodiscard]] size_t doSizeData() const override;
  //! Implements sizeImage(). Return the size of the image data area.
  [[nodiscard]] size_t doSizeImage() const override;
  //@}

 private:
  //! Pointers to the image data (strips) and their sizes.
  using Strips = std::vector<std::pair<const byte*, size_t>>;

  // DATA
  Strips strips_;  //!< Image strips data (never alloc'd) and sizes
};

/*!
  @brief A TIFF IFD entry containing the size of a data area of a related
         TiffDataEntry. This component is used, e.g. for
         \em Exif.Thumbnail.JPEGInterchangeFormatLength, which contains the
         size of \em Exif.Thumbnail.JPEGInterchangeFormat.
 */
class TiffSizeEntry : public TiffEntryBase {
 public:
  //! @name Creators
  //@{
  //! Constructor
  constexpr TiffSizeEntry(uint16_t tag, IfdId group, uint16_t dtTag, IfdId dtGroup) :
      TiffEntryBase(tag, group), dtTag_(dtTag), dtGroup_(dtGroup) {
  }

  //@}

  //! @name Accessors
  //@{
  //! Return the group of the related entry which has the data area
  [[nodiscard]] uint16_t dtTag() const {
    return dtTag_;
  }
  //! Return the group of the related entry which has the data area
  [[nodiscard]] IfdId dtGroup() const {
    return dtGroup_;
  }
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffSizeEntry* doClone() const override;
  //@}

 private:
  // DATA
  uint16_t dtTag_;  //!< Tag of the entry with the data area
  IfdId dtGroup_;   //!< Group of the entry with the data area
};

/*!
  @brief This class models a TIFF directory (%Ifd). It is a composite
         component of the TIFF tree.
 */
class TiffDirectory : public TiffComponent {
  friend class TiffEncoder;
  friend class TiffDecoder;

 public:
  //! @name Creators
  //@{
  //! Default constructor
  TiffDirectory(uint16_t tag, IfdId group, bool hasNext = true);
  //! Virtual destructor
  ~TiffDirectory() override = default;
  //@}

  //! @name NOT implemented
  //@{
  //! Assignment operator.
  TiffDirectory& operator=(const TiffDirectory&) = delete;
  //@}

  //! @name Accessors
  //@{
  //! Return true if the directory has a next pointer
  [[nodiscard]] bool hasNext() const {
    return hasNext_;
  }
  //@}

 protected:
  //! @name Protected Creators
  //@{
  //! Copy constructor (used to implement clone()).
  TiffDirectory(const TiffDirectory& rhs);
  //@}

  //! @name Protected Manipulators
  //@{
  TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                           TiffComponent::UniquePtr object) override;
  TiffComponent* doAddChild(TiffComponent::UniquePtr tiffComponent) override;
  TiffComponent* doAddNext(TiffComponent::UniquePtr tiffComponent) override;
  void doAccept(TiffVisitor& visitor) override;
  /*!
    @brief Implements write(). Write the TIFF directory, values and
           additional data, including the next-IFD, if any, to the
           \em ioWrapper, return the number of bytes written.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffDirectory* doClone() const override;
  /*!
    @brief This class does not really implement writeData(), it only has
           write(). This method must not be called; it commits suicide.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  /*!
    @brief Implements writeImage(). Write the image data of the TIFF
           directory to the \em ioWrapper by forwarding the call to each
           component as well as the next-IFD, if there is any. Return the
           number of bytes written.
   */
  size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  /*!
    @brief Implements size(). Return the size of the TIFF directory,
           values and additional data, including the next-IFD, if any.
   */
  [[nodiscard]] size_t doSize() const override;
  /*!
    @brief Implements count(). Return the number of entries in the TIFF
           directory. Does not count entries which are marked as deleted.
   */
  [[nodiscard]] size_t doCount() const override;
  /*!
    @brief This class does not really implement sizeData(), it only has
           size(). This method must not be called; it commits suicide.
   */
  [[nodiscard]] size_t doSizeData() const override;
  /*!
    @brief Implements sizeImage(). Return the sum of the image sizes of
           all components plus that of the next-IFD, if there is any.
   */
  [[nodiscard]] size_t doSizeImage() const override;
  //@}

 private:
  //! @name Private Accessors
  //@{
  //! Write a binary directory entry for a TIFF component.
  static size_t writeDirEntry(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, TiffComponent* pTiffComponent,
                              size_t valueIdx, size_t dataIdx, size_t& imageIdx);
  //@}

  // DATA
  Components components_;  //!< List of components in this directory
  bool hasNext_;           //!< True if the directory has a next pointer
  UniquePtr pNext_;        //!< Pointer to the next IFD
};

/*!
  @brief This class models a TIFF sub-directory (sub-IFD). A sub-IFD
         is an entry with one or more values that are pointers to IFD
         structures containing an IFD. The TIFF standard defines
         some important tags to be sub-IFDs, including the %Exif and
         GPS tags.
 */
class TiffSubIfd : public TiffEntryBase {
  friend class TiffReader;

 public:
  //! @name Creators
  //@{
  //! Default constructor
  TiffSubIfd(uint16_t tag, IfdId group, IfdId newGroup);
  //! Virtual destructor
  ~TiffSubIfd() override = default;
  //@}

  //! @name Protected Creators
  //@{
  //! Copy constructor (used to implement clone()).
  TiffSubIfd(const TiffSubIfd& rhs);
  TiffSubIfd& operator=(const TiffSubIfd&) = delete;
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                           TiffComponent::UniquePtr object) override;
  TiffComponent* doAddChild(TiffComponent::UniquePtr tiffComponent) override;
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write(). Write the sub-IFD pointers to the \em ioWrapper,
           return the number of bytes written. The \em valueIdx and
           \em imageIdx arguments are not used.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffSubIfd* doClone() const override;
  /*!
    @brief Implements writeData(). Write the sub-IFDs to the \em ioWrapper.
           Return the number of bytes written.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  /*!
    @brief Implements writeImage(). Write the image data of each sub-IFD to
           the \em ioWrapper. Return the number of bytes written.
   */
  size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  //! Implements size(). Return the size of the sub-Ifd pointers.
  [[nodiscard]] size_t doSize() const override;
  //! Implements sizeData(). Return the sum of the sizes of all sub-IFDs.
  [[nodiscard]] size_t doSizeData() const override;
  //! Implements sizeImage(). Return the sum of the image sizes of all sub-IFDs.
  [[nodiscard]] size_t doSizeImage() const override;
  //@}

 private:
  //! A collection of TIFF directories (IFDs)
  using Ifds = std::vector<std::unique_ptr<TiffDirectory>>;

  // DATA
  IfdId newGroup_;  //!< Start of the range of group numbers for the sub-IFDs
  Ifds ifds_;       //!< The subdirectories
};

/*!
  @brief This class is the basis for Makernote support in TIFF. It contains
         a pointer to a concrete Makernote. The TiffReader visitor has the
         responsibility to create the correct Make/Model specific Makernote
         for a particular TIFF file. Calls to child management methods are
         forwarded to the concrete Makernote, if there is one.
 */
class TiffMnEntry : public TiffEntryBase {
  friend class TiffReader;
  friend class TiffDecoder;
  friend class TiffEncoder;

 public:
  //! @name Creators
  //@{
  //! Default constructor
  constexpr TiffMnEntry(uint16_t tag, IfdId group, IfdId mnGroup) :
      TiffEntryBase(tag, group, ttUndefined), mnGroup_(mnGroup) {
  }

 protected:
  //! @name Protected Manipulators
  //@{
  TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                           TiffComponent::UniquePtr object) override;
  TiffComponent* doAddChild(TiffComponent::UniquePtr tiffComponent) override;
  TiffComponent* doAddNext(TiffComponent::UniquePtr tiffComponent) override;
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write() by forwarding the call to the actual
           concrete Makernote, if there is one.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffMnEntry* doClone() const override;
  //! Implements count(). Return number of components in the entry.
  [[nodiscard]] size_t doCount() const override;
  // Using doWriteData from base class
  // Using doWriteImage from base class
  /*!
    @brief Implements size() by forwarding the call to the actual
           concrete Makernote, if there is one.
   */
  [[nodiscard]] size_t doSize() const override;
  // Using doSizeData from base class
  // Using doSizeImage from base class
  //@}

 private:
  // DATA
  IfdId mnGroup_;                      //!< New group for concrete mn
  std::unique_ptr<TiffComponent> mn_;  //!< The Makernote
};

/*!
  @brief Tiff IFD Makernote. This is a concrete class suitable for all
         IFD makernotes.

         Contains a makernote header (which can be 0) and an IFD and
         implements child mgmt functions to deal with the IFD entries. The
         various makernote weirdnesses are taken care of in the makernote
         header (and possibly in special purpose IFD entries).
 */
class TiffIfdMakernote : public TiffComponent {
  friend class TiffReader;

 public:
  //! @name Creators
  //@{
  //! Default constructor
  TiffIfdMakernote(uint16_t tag, IfdId group, IfdId mnGroup, std::unique_ptr<MnHeader> pHeader, bool hasNext = true);
  //! Virtual destructor
  ~TiffIfdMakernote() override;
  //@}

  /*!
    @name NOT implemented

    Implementing the copy constructor and assignment operator will require
    cloning the header, i.e., clone() functionality on the MnHeader
    hierarchy.
   */
  //@{
  //! Copy constructor.
  TiffIfdMakernote(const TiffIfdMakernote&) = delete;
  //! Assignment operator.
  TiffIfdMakernote& operator=(const TiffIfdMakernote&) = delete;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Read the header from a data buffer, return true if successful.

    The default implementation simply returns true.
   */
  bool readHeader(const byte* pData, size_t size, ByteOrder byteOrder);
  /*!
    @brief Set the byte order for the makernote.
   */
  void setByteOrder(ByteOrder byteOrder);
  /*!
    @brief Set the byte order used for the image.
   */
  void setImageByteOrder(ByteOrder byteOrder) {
    imageByteOrder_ = byteOrder;
  }
  //@}

  //! @name Accessors
  //@{
  //! Return the size of the header in bytes.
  [[nodiscard]] size_t sizeHeader() const;
  //! Write the header to a data buffer, return the number of bytes written.
  size_t writeHeader(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
  /*!
    @brief Return the offset to the makernote from the start of the
           TIFF header.
  */
  [[nodiscard]] size_t mnOffset() const;
  /*!
    @brief Return the offset to the start of the Makernote IFD from
           the start of the Makernote.
           Returns 0 if there is no header.
   */
  [[nodiscard]] size_t ifdOffset() const;
  /*!
    @brief Return the byte order for the makernote. Requires the image
           byte order to be set (setImageByteOrder()).  Returns the byte
           order for the image if there is no header or the byte order for
           the header is \c invalidByteOrder.
   */
  [[nodiscard]] ByteOrder byteOrder() const;

  /*!
    @brief Return the base offset for use with the makernote IFD entries
           relative to the start of the TIFF header.
           Returns 0 if there is no header.
   */
  [[nodiscard]] size_t baseOffset() const;
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                           TiffComponent::UniquePtr object) override;
  TiffComponent* doAddChild(TiffComponent::UniquePtr tiffComponent) override;
  TiffComponent* doAddNext(TiffComponent::UniquePtr tiffComponent) override;
  void doAccept(TiffVisitor& visitor) override;
  /*!
    @brief Implements write(). Write the Makernote header, TIFF directory,
           values and additional data to the \em ioWrapper, return the
           number of bytes written.
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffIfdMakernote* doClone() const override;
  /*!
    @brief This class does not really implement writeData(), it only has
           write(). This method must not be called; it commits suicide.
   */
  size_t doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                     size_t& imageIdx) const override;
  /*!
    @brief Implements writeImage(). Write the image data of the IFD of
           the Makernote. Return the number of bytes written.
   */
  size_t doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  /*!
    @brief Implements size(). Return the size of the Makernote header,
           TIFF directory, values and additional data.
   */
  [[nodiscard]] size_t doSize() const override;
  /*!
    @brief Implements count(). Return the number of entries in the IFD
           of the Makernote. Does not count entries which are marked as
           deleted.
   */
  [[nodiscard]] size_t doCount() const override;
  /*!
    @brief This class does not really implement sizeData(), it only has
           size(). This method must not be called; it commits suicide.
   */
  [[nodiscard]] size_t doSizeData() const override;
  /*!
    @brief Implements sizeImage(). Return the total image data size of the
           makernote IFD.
   */
  [[nodiscard]] size_t doSizeImage() const override;
  //@}

 private:
  // DATA
  std::unique_ptr<MnHeader> pHeader_;           //!< Makernote header
  TiffDirectory ifd_;                           //!< Makernote IFD
  size_t mnOffset_{};                           //!< Makernote offset
  ByteOrder imageByteOrder_{invalidByteOrder};  //!< Byte order for the image
};

/*!
  @brief Function pointer type for a function to determine which cfg + def
         of a corresponding array set to use.
 */
using CfgSelFct = int (*)(uint16_t, const byte*, size_t, TiffComponent*);

//! Function pointer type for a crypt function used for binary arrays.
using CryptFct = DataBuf (*)(uint16_t, const byte*, size_t, TiffComponent*);

//! Defines one tag in a binary array
struct ArrayDef {
  //! Comparison with idx
  bool operator==(size_t idx) const {
    return idx_ == idx;
  }
  //! Get the size in bytes of a tag.
  [[nodiscard]] size_t size(uint16_t tag, IfdId group) const;
  // DATA
  size_t idx_;         //!< Index in bytes from the start
  TiffType tiffType_;  //!< TIFF type of the element
  size_t count_;       //!< Number of components
};

//! Additional configuration for a binary array.
struct ArrayCfg {
  /*!
    @brief Return the size of the default tag, which is used
           to calculate tag numbers as idx/tagStep
   */
  [[nodiscard]] size_t tagStep() const {
    return elDefaultDef_.size(0, group_);
  }
  // DATA
  IfdId group_;            //!< Group for the elements
  ByteOrder byteOrder_;    //!< Byte order, invalidByteOrder to inherit
  TiffType elTiffType_;    //!< Type for the array entry and the size element, if any
  CryptFct cryptFct_;      //!< Crypt function, 0 if not used
  bool hasSize_;           //!< If true, first tag is the size element
  bool hasFillers_;        //!< If true, write all defined tags
  bool concat_;            //!< If true, concatenate gaps between defined tags to single tags
  ArrayDef elDefaultDef_;  //!< Default element
};

//! Combination of array configuration and definition for arrays
struct ArraySet {
  ArrayCfg cfg_;         //!< Binary array configuration
  const ArrayDef* def_;  //!< Binary array definition array
  size_t defSize_;       //!< Size of the array definition array
};

/*!
  @brief Composite to model an array of different tags. The tag types as well
         as other aspects of the array are configurable. The elements of this
         component are of type TiffBinaryElement.
 */
class TiffBinaryArray : public TiffEntryBase {
 public:
  //! @name Creators
  //@{
  //! Constructor
  TiffBinaryArray(uint16_t tag, IfdId group, const ArrayCfg& arrayCfg, const ArrayDef* arrayDef, size_t defSize);
  //! Constructor for a complex binary array
  TiffBinaryArray(uint16_t tag, IfdId group, const ArraySet* arraySet, size_t setSize, CfgSelFct cfgSelFct);
  //! Virtual destructor
  ~TiffBinaryArray() override = default;
  TiffBinaryArray& operator=(const TiffBinaryArray&) = delete;
  //@}

  //! @name Manipulators
  //@{
  //! Add an element to the binary array, return the size of the element
  size_t addElement(size_t idx, const ArrayDef& def);
  /*!
    @brief Setup cfg and def for the component, in case of a complex binary array.
           Else do nothing. Return true if the initialization succeeded, else false.

    This version of initialize() is used during intrusive writing. It determines the
    correct settings based on the \em group passed in (which is the group of the first
    tag that is added to the array). It doesn't require cfgSelFct_.

    @param group Group to setup the binary array for.
    @return true if the initialization succeeded, else false.
   */
  bool initialize(IfdId group);
  /*!
    @brief Setup cfg and def for the component, in case of a complex binary array.
           Else do nothing. Return true if the initialization succeeded, else false.

    This version of initialize() is used for reading and non-intrusive writing. It
    calls cfgSelFct_ to determine the correct settings.

    @param pRoot Pointer to the root component of the TIFF tree.
    @return true if the initialization succeeded, else false.
   */
  bool initialize(TiffComponent* pRoot);
  //! Initialize the original data buffer and its size from the base entry.
  void iniOrigDataBuf();
  //! Update the original data buffer and its size, return true if successful.
  bool updOrigDataBuf(const byte* pData, size_t size);
  //! Set a flag to indicate if the array was decoded
  void setDecoded(bool decoded) {
    decoded_ = decoded;
  }
  //@}

  //! @name Accessors
  //@{
  //! Return a pointer to the configuration
  [[nodiscard]] const ArrayCfg* cfg() const {
    return arrayCfg_;
  }
  //! Return a pointer to the definition
  [[nodiscard]] const ArrayDef* def() const {
    return arrayDef_;
  }
  //! Return the number of elements in the definition
  [[nodiscard]] size_t defSize() const {
    return defSize_;
  }
  //! Return the flag which indicates if the array was decoded
  [[nodiscard]] bool decoded() const {
    return decoded_;
  }
  //@}

 protected:
  //! @name Protected Creators
  //@{
  //! Copy constructor (used to implement clone()).
  TiffBinaryArray(const TiffBinaryArray& rhs);
  //@}

  //! @name Protected Manipulators
  //@{
  /*!
    @brief Implements addPath(). Todo: Document it!
   */
  TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                           TiffComponent::UniquePtr object) override;
  /*!
    @brief Implements addChild(). Todo: Document it!
   */
  TiffComponent* doAddChild(TiffComponent::UniquePtr tiffComponent) override;
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write(). Todo: Document it!
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffBinaryArray* doClone() const override;
  //! Implements count(). Todo: Document it!
  [[nodiscard]] size_t doCount() const override;
  // Using doWriteData from base class
  // Using doWriteImage from base class
  /*!
    @brief Implements size(). Todo: Document it!
   */
  [[nodiscard]] size_t doSize() const override;
  // Using doSizeData from base class
  // Using doSizeImage from base class
  //@}

 private:
  // DATA
  CfgSelFct cfgSelFct_{};       //!< Pointer to a function to determine which cfg to use (may be 0)
  const ArraySet* arraySet_{};  //!< Pointer to the array set, if any (may be 0)
  const ArrayCfg* arrayCfg_{};  //!< Pointer to the array configuration (must not be 0, except for
                                //!< unrecognized complex binary arrays)
  const ArrayDef* arrayDef_{};  //!< Pointer to the array definition (may be 0)
  size_t defSize_{};            //!< Size of the array definition array (may be 0)
  size_t setSize_{};            //!< Size of the array set (may be 0)
  Components elements_;         //!< List of elements in this composite
  byte* origData_{};            //!< Pointer to the original data buffer (unencrypted)
  size_t origSize_{};           //!< Size of the original data buffer
  TiffComponent* pRoot_{};      //!< Pointer to the root component of the TIFF tree. (Only used for intrusive writing.)
  bool decoded_{};              //!< Flag to indicate if the array was decoded
};

/*!
  @brief Element of a TiffBinaryArray.
 */
class TiffBinaryElement : public TiffEntryBase {
  using TiffEntryBase::TiffEntryBase;

 public:
  //! @name Manipulators
  //@{
  /*!
    @brief Set the array definition for this element.
   */
  void setElDef(const ArrayDef& def) {
    elDef_ = def;
  }
  /*!
    @brief Set the byte order of this element.
   */
  void setElByteOrder(ByteOrder byteOrder) {
    elByteOrder_ = byteOrder;
  }
  //@}

  //! @name Accessors
  //@{
  /*!
    @brief Return the array definition of this element.
   */
  [[nodiscard]] const ArrayDef* elDef() const {
    return &elDef_;
  }
  /*!
    @brief Return the byte order of this element.
   */
  [[nodiscard]] ByteOrder elByteOrder() const {
    return elByteOrder_;
  }
  //@}

 protected:
  //! @name Protected Manipulators
  //@{
  void doAccept(TiffVisitor& visitor) override;
  void doEncode(TiffEncoder& encoder, const Exifdatum* datum) override;
  /*!
    @brief Implements write(). Todo: Document it!
   */
  size_t doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                 size_t& imageIdx) override;
  //@}

  //! @name Protected Accessors
  //@{
  [[nodiscard]] TiffBinaryElement* doClone() const override;
  /*!
    @brief Implements count(). Returns the count from the element definition.
   */
  [[nodiscard]] size_t doCount() const override;
  // Using doWriteData from base class
  // Using doWriteImage from base class
  /*!
    @brief Implements size(). Returns count * type-size, both taken from
           the element definition.
   */
  [[nodiscard]] size_t doSize() const override;
  // Using doSizeData from base class
  // Using doSizeImage from base class
  //@}

 private:
  // DATA
  ArrayDef elDef_{0, ttUndefined, 0};        //!< The array element definition
  ByteOrder elByteOrder_{invalidByteOrder};  //!< Byte order to read/write the element
};

// *****************************************************************************
// template, inline and free functions

/*!
  @brief Compare two TIFF component pointers by tag. Return true if the tag
         of component lhs is less than that of rhs.
 */
bool cmpTagLt(const TiffComponent::UniquePtr& lhs, const TiffComponent::UniquePtr& rhs);

/*!
  @brief Compare two TIFF component pointers by group. Return true if the
         group of component lhs is less than that of rhs.
 */
bool cmpGroupLt(const std::unique_ptr<TiffDirectory>& lhs, const std::unique_ptr<TiffDirectory>& rhs);

//! Function to create and initialize a new TIFF entry
TiffComponent::UniquePtr newTiffEntry(uint16_t tag, IfdId group);

//! Function to create and initialize a new TIFF makernote entry
TiffComponent::UniquePtr newTiffMnEntry(uint16_t tag, IfdId group);

//! Function to create and initialize a new binary array element
TiffComponent::UniquePtr newTiffBinaryElement(uint16_t tag, IfdId group);

//! Function to create and initialize a new TIFF directory
template <IfdId newGroup>
TiffComponent::UniquePtr newTiffDirectory(uint16_t tag, IfdId /*group*/) {
  return std::make_unique<TiffDirectory>(tag, newGroup);
}

//! Function to create and initialize a new TIFF sub-directory
template <IfdId newGroup>
TiffComponent::UniquePtr newTiffSubIfd(uint16_t tag, IfdId group) {
  return std::make_unique<TiffSubIfd>(tag, group, newGroup);
}

//! Function to create and initialize a new binary array entry
template <const ArrayCfg& arrayCfg, size_t N, const ArrayDef (&arrayDef)[N]>
TiffComponent::UniquePtr newTiffBinaryArray0(uint16_t tag, IfdId group) {
  return std::make_unique<TiffBinaryArray>(tag, group, arrayCfg, arrayDef, N);
}

//! Function to create and initialize a new simple binary array entry
template <const ArrayCfg& arrayCfg>
TiffComponent::UniquePtr newTiffBinaryArray1(uint16_t tag, IfdId group) {
  return std::make_unique<TiffBinaryArray>(tag, group, arrayCfg, nullptr, 0);
}

//! Function to create and initialize a new complex binary array entry
template <size_t N, const ArraySet (&arraySet)[N], CfgSelFct cfgSelFct>
TiffComponent::UniquePtr newTiffBinaryArray2(uint16_t tag, IfdId group) {
  return std::make_unique<TiffBinaryArray>(tag, group, arraySet, N, cfgSelFct);
}

//! Function to create and initialize a new TIFF entry for a thumbnail (data)
template <uint16_t szTag, IfdId szGroup>
TiffComponent::UniquePtr newTiffThumbData(uint16_t tag, IfdId group) {
  return std::make_unique<TiffDataEntry>(tag, group, szTag, szGroup);
}

//! Function to create and initialize a new TIFF entry for a thumbnail (size)
template <uint16_t dtTag, IfdId dtGroup>
TiffComponent::UniquePtr newTiffThumbSize(uint16_t tag, IfdId group) {
  return std::make_unique<TiffSizeEntry>(tag, group, dtTag, dtGroup);
}

//! Function to create and initialize a new TIFF entry for image data
template <uint16_t szTag, IfdId szGroup>
TiffComponent::UniquePtr newTiffImageData(uint16_t tag, IfdId group) {
  return std::make_unique<TiffImageEntry>(tag, group, szTag, szGroup);
}

//! Function to create and initialize a new TIFF entry for image data (size)
template <uint16_t dtTag, IfdId dtGroup>
TiffComponent::UniquePtr newTiffImageSize(uint16_t tag, IfdId group) {
  // Todo: Same as newTiffThumbSize - consolidate (rename)?
  return std::make_unique<TiffSizeEntry>(tag, group, dtTag, dtGroup);
}

}  // namespace Internal
}  // namespace Exiv2

#endif  // #ifndef TIFFCOMPOSITE_INT_HPP_
