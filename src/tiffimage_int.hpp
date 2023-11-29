// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TIFFIMAGE_INT_HPP_
#define TIFFIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include <unordered_map>
#include "image.hpp"
#include "tiffcomposite_int.hpp"
#include "tifffwd_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
/*!
  @brief Contains internal objects which are not published and are not part
         of the <b>libexiv2</b> API.
 */

// *****************************************************************************
// class definitions

/*!
  @brief Abstract base class defining the interface of an image header.
         Used internally by classes for TIFF-based images.  Default
         implementation is for the regular TIFF header.
 */
class TiffHeaderBase {
 public:
  //! @name Creators
  //@{
  //! Constructor taking \em tag, \em size and default \em byteOrder and \em offset.
  TiffHeaderBase(uint16_t tag, uint32_t size, ByteOrder byteOrder, uint32_t offset);
  //! Virtual destructor.
  virtual ~TiffHeaderBase() = default;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Read the image header from a data buffer. Return false if the
           data buffer does not contain an image header of the expected
           format, else true.

    @param pData Pointer to the data buffer.
    @param size  Number of bytes in the data buffer.
    @return True if the TIFF header was read successfully. False if the
           data buffer does not contain a valid TIFF header.
   */
  virtual bool read(const byte* pData, size_t size);
  //! Set the byte order.
  virtual void setByteOrder(ByteOrder byteOrder);
  //! Set the offset to the start of the root directory.
  virtual void setOffset(uint32_t offset);
  //@}

  //! @name Accessors
  //@{
  /*!
    @brief Return the image header in binary format.
           The caller owns this data and %DataBuf ensures that it will be deleted.

    @return Binary header data.
   */
  [[nodiscard]] virtual DataBuf write() const;
  /*!
    @brief Print debug info for the image header to \em os.

    @param os Output stream to write to.
    @param prefix Prefix to be written before each line of output.
   */
  virtual void print(std::ostream& os, const std::string& prefix = "") const;
  //! Return the byte order (little or big endian).
  [[nodiscard]] virtual ByteOrder byteOrder() const;
  //! Return the offset to the start of the root directory.
  [[nodiscard]] virtual uint32_t offset() const;
  //! Return the size (in bytes) of the image header.
  [[nodiscard]] virtual uint32_t size() const;
  //! Return the tag value (magic number) which identifies the buffer as TIFF data.
  [[nodiscard]] virtual uint16_t tag() const;
  /*!
    @brief Return \c true if the %Exif \em tag from \em group is an image tag.

    Certain tags of TIFF and TIFF-like images are required to correctly
    display the primary image. These image tags contain image data rather
    than metadata.

    @param tag Tag number.
    @param group Group identifier.
    @param pPrimaryGroups Pointer to a list of TIFF groups that contain
           primary images, empty if none are marked.

    @return The default implementation returns \c false.
   */
  virtual bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const;
  //@}

 private:
  // DATA
  uint16_t tag_;         //!< Tag to identify the buffer as TIFF data
  uint32_t size_;        //!< Size of the header
  ByteOrder byteOrder_;  //!< Applicable byte order
  uint32_t offset_;      //!< Offset to the start of the root dir
};

//! Convenience function to check if tag, group is in the list of TIFF image tags.
bool isTiffImageTag(uint16_t tag, IfdId group);

/*!
  @brief Standard TIFF header structure.
 */
class TiffHeader : public TiffHeaderBase {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  explicit TiffHeader(ByteOrder byteOrder = littleEndian, uint32_t offset = 0x00000008, bool hasImageTags = true);
  //@}
  //@{
  //! @name Accessors
  bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const override;
  //@}

 private:
  // DATA
  bool hasImageTags_;  //!< Indicates if image tags are supported
};

/*!
  @brief Data structure used to list image tags for TIFF and TIFF-like images.
 */
using TiffGroupKey = std::pair<uint32_t, IfdId>;

struct TiffGroupKey_hash {
  std::size_t operator()(const TiffGroupKey& pair) const noexcept {
    return std::hash<uint64_t>{}(static_cast<uint64_t>(pair.first) << 32 | static_cast<uint64_t>(pair.second));
  }
};

/*!
  @brief Data structure used as a row (element) of a table (array)
         defining the TIFF component used for each tag in a group.
 */
using TiffGroupTable = std::unordered_map<TiffGroupKey, NewTiffCompFct, TiffGroupKey_hash>;

/*!
  @brief Data structure used as a row of the table which describes TIFF trees.
         Multiple trees are needed as TIFF-based RAW image formats do not always
         use standard TIFF layout.
*/
using TiffTreeParent = std::pair<IfdId, uint32_t>;  // Parent group, parent tag
using TiffTreeTable = std::unordered_map<TiffGroupKey, TiffTreeParent, TiffGroupKey_hash>;

/*!
  @brief TIFF component factory.
 */
class TiffCreator {
 public:
  /*!
    @brief Create the TiffComponent for TIFF entry \em extendedTag and
           \em group. The embedded lookup table is used to find the correct
           component creation function. If the pointer that is returned
           is 0, then the TIFF entry should be ignored.
  */
  static std::unique_ptr<TiffComponent> create(uint32_t extendedTag, IfdId group);
  /*!
    @brief Get the path, i.e., a list of extended tag and group pairs, from
           the \em root TIFF element to the TIFF entry \em extendedTag and
           \em group.
  */
  static TiffPath getPath(uint32_t extendedTag, IfdId group, uint32_t root);

 private:
  static const TiffTreeTable tiffTreeTable_;    ///< TIFF tree structure
  static const TiffGroupTable tiffGroupTable_;  ///< TIFF group structure
};

/*!
  @brief Stateless parser class for data in TIFF format. Images use this
         class to decode and encode TIFF-based data.
 */
class TiffParserWorker {
 public:
  /*!
    @brief Decode TIFF metadata from a data buffer \em pData of length
           \em size into the provided metadata containers.

    This is the entry point to access image data in TIFF format. The
    parser uses classes TiffHeader and the TiffComponent and TiffVisitor
    hierarchies.

    @param exifData  Exif metadata container.
    @param iptcData  IPTC metadata container.
    @param xmpData   XMP metadata container.
    @param pData     Pointer to the data buffer. Must point to data
                     in TIFF format; no checks are performed.
    @param size      Length of the data buffer.
    @param root      Root tag of the TIFF tree for new TIFF components.
    @param findDecoderFct Function to access special decoding info.
    @param pHeader   Optional pointer to a TIFF header. If not provided,
                     a standard TIFF header is used.

    @return Byte order in which the data is encoded, invalidByteOrder if
            decoding failed.
  */
  static ByteOrder decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData, size_t size,
                          uint32_t root, FindDecoderFct findDecoderFct, TiffHeaderBase* pHeader = nullptr);
  /*!
    @brief Encode TIFF metadata from the metadata containers into a
           memory block \em blob.

    1) Parse the binary image, if one is provided, and
    2) attempt updating the parsed tree in-place ("non-intrusive writing")
    3) else, create a new tree and write a new TIFF structure ("intrusive
       writing"). If there is a parsed tree, it is only used to access the
       image data in this case.
   */
  static WriteMethod encode(BasicIo& io, const byte* pData, size_t size, ExifData& exifData, IptcData& iptcData,
                            XmpData& xmpData, uint32_t root, FindEncoderFct findEncoderFct, TiffHeaderBase* pHeader,
                            OffsetWriter* pOffsetWriter);

 private:
  /*!
    @brief Parse TIFF metadata from a data buffer \em pData of length
           \em size into a TIFF composite structure.

    @param pData     Pointer to the data buffer. Must point to data
                     in TIFF format; no checks are performed.
    @param size      Length of the data buffer.
    @param root      Root tag of the TIFF tree.
    @param pHeader   Pointer to a TIFF header.
    @return          An auto pointer with the root element of the TIFF
                     composite structure. If \em pData is 0 or \em size
                     is 0, the return value is a 0 pointer.
   */
  static std::unique_ptr<TiffComponent> parse(const byte* pData, size_t size, uint32_t root, TiffHeaderBase* pHeader);
  /*!
    @brief Find primary groups in the source tree provided and populate
           the list of primary groups.

    @param pSourceDir Pointer to the source composite tree to search (may be 0)
    @return List of primary groups which is populated
   */
  static PrimaryGroups findPrimaryGroups(TiffComponent* pSourceDir);
};

/*!
  @brief Table of TIFF decoding and encoding functions and find functions.
         This class is separated from the metadata decoder and encoder
         visitors so that the parser can be parametrized with a different
         table if needed. This is used, eg., for CR2 format, which uses a
         different decoder table.
 */
class TiffMapping {
 public:
  /*!
    @brief Find the decoder function for a key.

    If the returned pointer is 0, the tag should not be decoded,
    else the decoder function should be used.

    @param make Camera make
    @param extendedTag Extended tag
    @param group %Group

    @return Pointer to the decoder function
   */
  static DecoderFct findDecoder(const std::string& make, uint32_t extendedTag, IfdId group);
  /*!
    @brief Find special encoder function for a key.

    If the returned pointer is 0, the tag should be encoded with the
    encoder function of the TIFF component, else the encoder function
    should be used.

    @param make Camera make
    @param extendedTag Extended tag
    @param group %Group

    @return Pointer to the encoder function
   */
  static EncoderFct findEncoder(const std::string& make, uint32_t extendedTag, IfdId group);

 private:
  static const TiffMappingInfo tiffMappingInfo_[];  ///< TIFF mapping table
};

/*!
  @brief Class to insert pointers or offsets to computed addresses at
         specific locations in an image. Used for offsets which are
         best computed during the regular write process. They are
         written in a second pass, using the writeOffsets() method.
 */
class OffsetWriter {
 public:
  //! Identifiers for supported offsets
  enum OffsetId {
    cr2RawIfdOffset  //!< CR2 RAW IFD offset, a pointer in the CR2 header to the 4th IFD in a CR2 image
  };
  //! @name Manipulators
  //@{
  /*!
    @brief Set the \em origin of the offset for \em id, i.e., the location in the image where the offset is,
           and the byte order to encode the offset.

    If the list doesn't contain an entry for \em id yet, this function will create one.
  */
  void setOrigin(OffsetId id, uint32_t origin, ByteOrder byteOrder);
  /*!
    @brief Set the \em target for offset \em id, i.e., the address to which the offset points.

    If the list doesn't contain an entry with \em id yet, this function won't do anything.
  */
  void setTarget(OffsetId id, uint32_t target);
  //@}

  //! @name Accessors
  //@{
  //! Write the offsets to the IO instance \em io.
  void writeOffsets(BasicIo& io) const;
  //@}
 private:
  //! Data structure for the offset list.
  struct OffsetData {
    uint32_t origin_{};                  //!< Origin address
    uint32_t target_{};                  //!< Target address
    ByteOrder byteOrder_{littleEndian};  //!< Byte order to use to encode target address
  };
  //! Type of the list containing an identifier and an address pair.
  using OffsetList = std::map<OffsetId, OffsetData>;

  // DATA
  OffsetList offsetList_;  //!< List of the offsets to replace

};  // class OffsetWriter

// Todo: Move this class to metadatum_int.hpp or tags_int.hpp
//! Unary predicate that matches an Exifdatum with a given IfdId.
class FindExifdatum {
 public:
  //! Constructor, initializes the object with the IfdId to look for.
  explicit FindExifdatum(Exiv2::IfdId ifdId) : ifdId_(ifdId) {
  }
  //! Returns true if IFD id matches.
  bool operator()(const Exiv2::Exifdatum& md) const {
    return ifdId_ == md.ifdId();
  }

 private:
  Exiv2::IfdId ifdId_;

};  // class FindExifdatum

}  // namespace Exiv2::Internal

#endif  // #ifndef TIFFIMAGE_INT_HPP_
