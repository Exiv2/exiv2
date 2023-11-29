// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_MAKERNOTE_INT_HPP
#define EXIV2_MAKERNOTE_INT_HPP

// *****************************************************************************
// included header files
#include "tags_int.hpp"
#include "types.hpp"

// namespace extensions
namespace Exiv2::Internal {
class IoWrapper;
class TiffComponent;
// *****************************************************************************
// function prototypes
/*!
  @brief Determine the path to the Exiv2 configuration file
 */
std::string getExiv2ConfigPath();

/*!
  @brief Read value from Exiv2 configuration file
 */
std::string readExiv2Config(const std::string& section, const std::string& value, const std::string& def);

// *****************************************************************************
// class definitions

//! Type for a pointer to a function creating a makernote (image)
using NewMnFct = TiffComponent* (*)(uint16_t, IfdId, IfdId, const byte*, size_t, ByteOrder);

//! Type for a pointer to a function creating a makernote (group)
using NewMnFct2 = TiffComponent* (*)(uint16_t tag, IfdId group, IfdId mnGroup);

//! Makernote registry structure
struct TiffMnRegistry {
  struct MakeKey;
  /*!
    @brief Compare a TiffMnRegistry structure with a key being the make
           string from the image. The two are equal if
           TiffMnRegistry::make_ equals a substring of the key of the
           same size. E.g., registry = "OLYMPUS",
           key = "OLYMPUS OPTICAL CO.,LTD" (found in the image) match.
   */
  bool operator==(const std::string& key) const;

  //! Compare a TiffMnRegistry structure with a makernote group
  bool operator==(IfdId key) const;

  // DATA
  const char* make_;     //!< Camera make
  IfdId mnGroup_;        //!< Group identifier
  NewMnFct newMnFct_;    //!< Makernote create function (image)
  NewMnFct2 newMnFct2_;  //!< Makernote create function (group)
};

/*!
  @brief TIFF makernote factory for concrete TIFF makernotes.
 */
class TiffMnCreator {
 public:
  /*!
    @brief Create the Makernote for camera \em make and details from
           the makernote entry itself if needed. Return a pointer to
           the newly created TIFF component. Set tag and group of the
           new component to \em tag and \em group. This method is used
           when a makernote is parsed from the Exif block.
    @note  Ownership for the component is transferred to the caller,
           who is responsible to delete the component. No smart pointer
           is used to indicate this transfer here in order to reduce
           file dependencies.
  */
  static TiffComponent* create(uint16_t tag, IfdId group, const std::string& make, const byte* pData, size_t size,
                               ByteOrder byteOrder);
  /*!
    @brief Create the Makernote for a given group. This method is used
           when a makernote is written back from Exif tags.
   */
  static TiffComponent* create(uint16_t tag, IfdId group, IfdId mnGroup);

  ~TiffMnCreator() = default;
  //! Prevent destruction (needed if used as a policy class)
  TiffMnCreator(const TiffComponent&) = delete;
  TiffMnCreator& operator=(const TiffComponent&) = delete;

 private:
  static const TiffMnRegistry registry_[];  //!< List of makernotes
};

//! Makernote header interface. This class is used with TIFF makernotes.
class MnHeader {
 public:
  //! @name Creators
  //@{
  //! Virtual destructor.
  virtual ~MnHeader() = default;
  //@}
  //! @name Manipulators
  //@{
  //! Read the header from a data buffer, return true if ok
  virtual bool read(const byte* pData, size_t size, ByteOrder byteOrder) = 0;
  /*!
    @brief Set the byte order for the makernote.
   */
  virtual void setByteOrder(ByteOrder byteOrder);
  //@}
  //! @name Accessors
  //@{
  //! Return the size of the header (in bytes).
  [[nodiscard]] virtual size_t size() const = 0;
  //! Write the header to a data buffer, return the number of bytes written.
  virtual size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const = 0;
  /*!
    @brief Return the offset to the start of the Makernote IFD from
           the start of the Makernote (= the start of the header).
   */
  [[nodiscard]] virtual size_t ifdOffset() const;
  /*!
    @brief Return the byte order for the makernote. If the return value is
           invalidByteOrder, this means that the byte order of the the
           image should be used for the makernote.
   */
  [[nodiscard]] virtual ByteOrder byteOrder() const;
  /*!
    @brief Return the base offset for the makernote IFD entries relative
           to the start of the TIFF header. \em mnOffset is the offset
           to the makernote from the start of the TIFF header.
   */
  [[nodiscard]] virtual size_t baseOffset(size_t mnOffset) const;
  //@}

};  // class MnHeader

//! Header of an Olympus Makernote
class OlympusMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  OlympusMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                 //!< Data buffer for the makernote header
  static const byte signature_[];  //!< Olympus makernote header signature

};  // class OlympusMnHeader

//! Header of an Olympus II Makernote
class Olympus2MnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  Olympus2MnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                 //!< Data buffer for the makernote header
  static const byte signature_[];  //!< Olympus makernote header signature

};  // class Olympus2MnHeader

//! Header of an OM Digital Solutions (ex Olympus) Makernote
class OMSystemMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  OMSystemMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                 //!< Data buffer for the makernote header
  static const byte signature_[];  //!< Olympus makernote header signature

};  // class OMSystemMnHeader

//! Header of a Fujifilm Makernote
class FujiMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  FujiMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  // setByteOrder not implemented
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] ByteOrder byteOrder() const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                    //!< Data buffer for the makernote header
  static const byte signature_[];     //!< Fujifilm makernote header signature
  static const ByteOrder byteOrder_;  //!< Byteorder for makernote (always II)
  size_t start_{0};                   //!< Start of the mn IFD rel. to mn start

};  // class FujiMnHeader

//! Header of a Nikon 2 Makernote
class Nikon2MnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  Nikon2MnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                    //!< Raw header data
  size_t start_{0};                //!< Start of the mn IFD rel. to mn start
  static const byte signature_[];  //!< Nikon 2 makernote header signature

};  // class Nikon2MnHeader

//! Header of a Nikon 3 Makernote
class Nikon3MnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  Nikon3MnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  void setByteOrder(ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] ByteOrder byteOrder() const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                            //!< Raw header data
  ByteOrder byteOrder_{invalidByteOrder};  //!< Byteorder for makernote
  size_t start_;                           //!< Start of the mn IFD rel. to mn start
  static const byte signature_[];          //!< Nikon 3 makernote header signature

};  // class Nikon3MnHeader

//! Header of a Panasonic Makernote
class PanasonicMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  PanasonicMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                    //!< Raw header data
  size_t start_{0};                //!< Start of the mn IFD rel. to mn start
  static const byte signature_[];  //!< Panasonic makernote header signature

};  // class PanasonicMnHeader

//! Header of an Pentax DNG Makernote
class PentaxDngMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  PentaxDngMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                 //!< Data buffer for the makernote header
  static const byte signature_[];  //!< Pentax DNG makernote header signature

};  // class PentaxDngMnHeader

//! Header of an Pentax Makernote
class PentaxMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  PentaxMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf header_;                 //!< Data buffer for the makernote header
  static const byte signature_[];  //!< Pentax makernote header signature

};  // class PentaxMnHeader

//! Header of a Samsung Makernote, only used for the relative offset
class SamsungMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  SamsungMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t baseOffset(size_t mnOffset) const override;
  //@}

};  // class SamsungMnHeader

//! Header of a Sigma Makernote
class SigmaMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  SigmaMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                     //!< Raw header data
  size_t start_{0};                 //!< Start of the mn IFD rel. to mn start
  static const byte signature1_[];  //!< Sigma makernote header signature 1
  static const byte signature2_[];  //!< Sigma makernote header signature 2

};  // class SigmaMnHeader

//! Header of a Sony Makernote
class SonyMnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  SonyMnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                    //!< Raw header data
  size_t start_{0};                //!< Start of the mn IFD rel. to mn start
  static const byte signature_[];  //!< Sony makernote header signature

};  // class SonyMnHeader

//! Header of a Casio2 Makernote
class Casio2MnHeader : public MnHeader {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  Casio2MnHeader();
  //@}
  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size, ByteOrder byteOrder) override;
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] size_t size() const override;
  size_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const override;
  [[nodiscard]] size_t ifdOffset() const override;
  [[nodiscard]] ByteOrder byteOrder() const override;
  //@}
  //! Return the size of the makernote header signature
  static size_t sizeOfSignature();

 private:
  DataBuf buf_;                       //!< Raw header data
  size_t start_{0};                   //!< Start of the mn IFD rel. to mn start
  static const byte signature_[];     //!< Casio makernote header signature
  static const ByteOrder byteOrder_;  //!< Byteorder for makernote (always big endian)

};  // class Casio2MnHeader

// *****************************************************************************
// template, inline and free functions

//! Function to create a simple IFD makernote (Canon, Minolta, Nikon1)
TiffComponent* newIfdMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size, ByteOrder byteOrder);

//! Function to create a simple IFD makernote (Canon, Minolta, Nikon1)
TiffComponent* newIfdMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create an Olympus makernote
TiffComponent* newOlympusMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                            ByteOrder byteOrder);

//! Function to create an Olympus makernote
TiffComponent* newOlympusMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create an Olympus II makernote
TiffComponent* newOlympus2Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create an OM Digital Solutions makernote
TiffComponent* newOMSystemMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                             ByteOrder byteOrder);

//! Function to create an OM Digital Solutions makernote
TiffComponent* newOMSystemMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Fujifilm makernote
TiffComponent* newFujiMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size, ByteOrder byteOrder);

//! Function to create a Fujifilm makernote
TiffComponent* newFujiMn2(uint16_t tag, IfdId group, IfdId mnGroup);

/*!
  @brief Function to create a Nikon makernote. This will create the
         appropriate Nikon 1, 2 or 3 makernote, based on the arguments.
 */
TiffComponent* newNikonMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                          ByteOrder byteOrder);

//! Function to create a Nikon2 makernote
TiffComponent* newNikon2Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Nikon3 makernote
TiffComponent* newNikon3Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Panasonic makernote
TiffComponent* newPanasonicMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                              ByteOrder byteOrder);

//! Function to create a Panasonic makernote
TiffComponent* newPanasonicMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create an Pentax makernote
TiffComponent* newPentaxMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                           ByteOrder byteOrder);

//! Function to create an Pentax makernote
TiffComponent* newPentaxMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create an Pentax DNG makernote
TiffComponent* newPentaxDngMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Samsung makernote
TiffComponent* newSamsungMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                            ByteOrder byteOrder);

//! Function to create a Samsung makernote
TiffComponent* newSamsungMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Sigma makernote
TiffComponent* newSigmaMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                          ByteOrder byteOrder);

//! Function to create a Sigma makernote
TiffComponent* newSigmaMn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Sony makernote
TiffComponent* newSonyMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size, ByteOrder byteOrder);

//! Function to create a Sony1 makernote
TiffComponent* newSony1Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Sony2 makernote
TiffComponent* newSony2Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

//! Function to create a Casio2 makernote
TiffComponent* newCasioMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                          ByteOrder byteOrder);

//! Function to create a Casio2 makernote
TiffComponent* newCasio2Mn2(uint16_t tag, IfdId group, IfdId mnGroup);

/*!
  @brief Function to select cfg + def of the Sony Camera Settings complex binary array.

  @param tag Tag number of the binary array
  @param pData Pointer to the raw array data.
  @param size Size of the array data.
  @param pRoot Pointer to the root component of the TIFF tree.
  @return An index into the array set, -1 if no match was found.
 */
int sonyCsSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
    @brief Function to select cfg + def of the Sony 2010 Miscellaneous Information complex binary array.

    @param tag Tag number of the binary array
    @param pData Pointer to the raw array data.
    @param size Size of the array data.
    @param pRoot Pointer to the root component of the TIFF tree.
    @return An index into the array set, -1 if no match was found.
*/
int sony2010eSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
    @brief Function to select cfg + def of the Sony2Fp (tag 9402) complex binary array.

    @param tag Tag number of the binary array
    @param pData Pointer to the raw array data.
    @param size Size of the array data.
    @param pRoot Pointer to the root component of the TIFF tree.
    @return An index into the array set, -1 if no match was found.
*/
int sony2FpSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
    @brief Function to select cfg + def of the SonyMisc2b (tag 9404b) complex binary array.

    @param tag Tag number of the binary array
    @param pData Pointer to the raw array data.
    @param size Size of the array data.
    @param pRoot Pointer to the root component of the TIFF tree.
    @return An index into the array set, -1 if no match was found.
*/
int sonyMisc2bSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
    @brief Function to select cfg + def of the SonyMisc3c (tag 9400) complex binary array.

    @param tag Tag number of the binary array
    @param pData Pointer to the raw array data.
    @param size Size of the array data.
    @param pRoot Pointer to the root component of the TIFF tree.
    @return An index into the array set, -1 if no match was found.
*/
int sonyMisc3cSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
  @brief Function to select cfg + def of a Nikon complex binary array.

  @param tag Tag number of the binary array
  @param pData Pointer to the raw array data.
  @param size Size of the array data.
  @param pRoot Pointer to the root component of the TIFF tree.
  @return An index into the array set, -1 if no match was found.
 */
int nikonSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

/*!
  @brief Encrypt and decrypt Nikon data.

  Checks the version of the Nikon data array and en/decrypts (portions of) it as
  needed. (The Nikon encryption algorithm is symmetric.)

  @note This function requires access to other components of the composite, it
        should only be called after all other components are read.

  @param tag Tag number of the binary array
  @param pData Pointer to the start of the data to en/decrypt.
  @param size Size of the data buffer.
  @param pRoot Pointer to the root element of the composite.
  @return En/decrypted data. Ownership of the memory is passed to the caller.
          The buffer may be empty in case no decryption was needed.
 */
DataBuf nikonCrypt(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot);

}  // namespace Exiv2::Internal

#endif  // EXIV2_MAKERNOTE_INT_HPP
