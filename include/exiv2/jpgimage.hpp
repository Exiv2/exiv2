// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JPGIMAGE_HPP_
#define JPGIMAGE_HPP_

// *****************************************************************************
#include "exiv2lib_export.h"

#include <array>

// included header files
#include "error.hpp"
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
// *****************************************************************************
// class definitions

/*!
  @brief Helper class, has methods to deal with %Photoshop "Information
         Resource Blocks" (IRBs).
 */
struct EXIV2API Photoshop {
  // Todo: Public for now
  static constexpr std::array irbId_{"8BIM", "AgHg", "DCSR", "PHUT"};  //!< %Photoshop IRB markers
  static constexpr auto ps3Id_ = "Photoshop 3.0\0";                    //!< %Photoshop marker
  static constexpr auto bimId_ = "8BIM";                               //!< %Photoshop IRB marker (deprecated)
  static constexpr uint16_t iptc_ = 0x0404;                            //!< %Photoshop IPTC marker
  static constexpr uint16_t preview_ = 0x040c;                         //!< %Photoshop preview marker

  /*!
    @brief Checks an IRB

    @param pPsData        Existing IRB buffer
    @param sizePsData     Size of the IRB buffer
    @return true  if the IRB marker is known and the buffer is big enough to check this;<BR>
            false otherwise
  */
  static bool isIrb(const byte* pPsData, size_t sizePsData);
  /*!
    @brief Validates all IRBs

    @param pPsData        Existing IRB buffer
    @param sizePsData     Size of the IRB buffer, may be 0
    @return true  if all IRBs are valid;<BR>
            false otherwise
  */
  static bool valid(const byte* pPsData, size_t sizePsData);
  /*!
    @brief Locates the data for a %Photoshop tag in a %Photoshop formated memory
        buffer. Operates on raw data to simplify reuse.
    @param pPsData Pointer to buffer containing entire payload of
        %Photoshop formated data, e.g., from APP13 Jpeg segment.
    @param sizePsData Size in bytes of pPsData.
    @param psTag %Tag number of the block to look for.
    @param record Output value that is set to the start of the
        data block within pPsData (may not be null).
    @param sizeHdr Output value that is set to the size of the header
        within the data block pointed to by record (may not be null).
    @param sizeData Output value that is set to the size of the actual
        data within the data block pointed to by record (may not be null).
    @return 0 if successful;<BR>
            3 if no data for psTag was found in pPsData;<BR>
           -2 if the pPsData buffer does not contain valid data.
  */
  static int locateIrb(const byte* pPsData, size_t sizePsData, uint16_t psTag, const byte** record,
                       uint32_t* const sizeHdr, uint32_t* const sizeData);
  /*!
    @brief Forwards to locateIrb() with \em psTag = \em iptc_
   */
  static int locateIptcIrb(const byte* pPsData, size_t sizePsData, const byte** record, uint32_t* const sizeHdr,
                           uint32_t* const sizeData);
  /*!
    @brief Forwards to locatePreviewIrb() with \em psTag = \em preview_
   */
  static int locatePreviewIrb(const byte* pPsData, size_t sizePsData, const byte** record, uint32_t* const sizeHdr,
                              uint32_t* const sizeData);
  /*!
    @brief Set the new IPTC IRB, keeps existing IRBs but removes the
           IPTC block if there is no new IPTC data to write.

    @param pPsData    Existing IRB buffer
    @param sizePsData Size of the IRB buffer, may be 0
    @param iptcData   Iptc data to embed, may be empty
    @return A data buffer containing the new IRB buffer, may have 0 size
  */
  static DataBuf setIptcIrb(const byte* pPsData, size_t sizePsData, const IptcData& iptcData);

};  // class Photoshop

/*!
  @brief Abstract helper base class to access JPEG images.
 */
class EXIV2API JpegBase : public Image {
 public:
  //! @name Manipulators
  //@{
  void readMetadata() override;
  void writeMetadata() override;
  void printStructure(std::ostream& out, PrintStructureOption option, int depth) override;
  //@}

  //! @name NOT implemented
  //@{
  //! Copy constructor
  JpegBase(const JpegBase&) = delete;
  //! Assignment operator
  JpegBase& operator=(const JpegBase&) = delete;
  //@}

 protected:
  //! @name Creators
  //@{
  /*!
    @brief Constructor that can either open an existing image or create
        a new image from scratch. If a new image is to be created, any
        existing data is overwritten.
    @param type Image type.
    @param io An auto-pointer that owns a BasicIo instance used for
        reading and writing image metadata. \b Important: The constructor
        takes ownership of the passed in BasicIo instance through the
        auto-pointer. Callers should not continue to use the BasicIo
        instance after it is passed to this method.  Use the Image::io()
        method to get a temporary reference.
    @param create Specifies if an existing image should be read (false)
        or if a new image should be created (true).
    @param initData Data to initialize newly created images. Only used
        when \em create is true. Should contain data for the smallest
        valid image of the calling subclass.
    @param dataSize Size of initData in bytes.
   */
  JpegBase(ImageType type, BasicIo::UniquePtr io, bool create, const byte initData[], size_t dataSize);
  //@}

  //! @name Accessors
  //@{
  /*!
    @brief Determine if the content of the BasicIo instance is of the
        type supported by this class.

    The advance flag determines if the read position in the stream is
    moved (see below). This applies only if the type matches and the
    function returns true. If the type does not match, the stream
    position is not changed. However, if reading from the stream fails,
    the stream position is undefined. Consult the stream state to obtain
    more information in this case.

    @param iIo BasicIo instance to read from.
    @param advance Flag indicating whether the position of the io
        should be advanced by the number of characters read to
        analyse the data (true) or left at its original
        position (false). This applies only if the type matches.
    @return  true  if the data matches the type of this class;<BR>
             false if the data does not match
   */
  virtual bool isThisType(BasicIo& iIo, bool advance) const = 0;
  //@}

  //! @name Manipulators
  //@{
  /*!
    @brief Writes the image header (aka signature) to the BasicIo instance.
    @param oIo BasicIo instance that the header is written to.
    @return 0 if successful;<BR>
            4 if the output file can not be written to
   */
  virtual int writeHeader(BasicIo& oIo) const = 0;
  //@}

  // Constant Data
  static constexpr byte dht_ = 0xc4;    //!< JPEG DHT marker
  static constexpr byte dqt_ = 0xdb;    //!< JPEG DQT marker
  static constexpr byte dri_ = 0xdd;    //!< JPEG DRI marker
  static constexpr byte sos_ = 0xda;    //!< JPEG SOS marker
  static constexpr byte eoi_ = 0xd9;    //!< JPEG EOI marker
  static constexpr byte app0_ = 0xe0;   //!< JPEG APP0 marker
  static constexpr byte app1_ = 0xe1;   //!< JPEG APP1 marker
  static constexpr byte app2_ = 0xe2;   //!< JPEG APP2 marker
  static constexpr byte app13_ = 0xed;  //!< JPEG APP13 marker
  static constexpr byte com_ = 0xfe;    //!< JPEG Comment marker

  // Start of Frame markers, nondifferential Huffman-coding frames
  static constexpr byte sof0_ = 0xc0;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof1_ = 0xc1;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof2_ = 0xc2;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof3_ = 0xc3;  //!< JPEG Start-Of-Frame marker

  // Start of Frame markers, differential Huffman-coding frames
  static constexpr byte sof5_ = 0xc5;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof6_ = 0xc6;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof7_ = 0xc7;  //!< JPEG Start-Of-Frame marker

  // Start of Frame markers, nondifferential arithmetic-coding frames
  static constexpr byte sof9_ = 0xc9;   //!< JPEG Start-Of-Frame marker
  static constexpr byte sof10_ = 0xca;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof11_ = 0xcb;  //!< JPEG Start-Of-Frame marker

  // Start of Frame markers, differential arithmetic-coding frames
  static constexpr byte sof13_ = 0xcd;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof14_ = 0xce;  //!< JPEG Start-Of-Frame marker
  static constexpr byte sof15_ = 0xcf;  //!< JPEG Start-Of-Frame marker

  static constexpr auto exifId_ = "Exif\0\0";                       //!< Exif identifier
  static constexpr auto jfifId_ = "JFIF\0";                         //!< JFIF identifier
  static constexpr auto xmpId_ = "http://ns.adobe.com/xap/1.0/\0";  //!< XMP packet identifier
  static constexpr auto iccId_ = "ICC_PROFILE\0";                   //!< ICC profile identifier

 private:
  //! @name Manipulators
  //@{
  /*!
    @brief Initialize the image with the provided data.
    @param initData Data to be written to the associated BasicIo
    @param dataSize Size in bytes of data to be written
    @return 0 if successful;<BR>
            4 if the image can not be written to.
   */
  int initImage(const byte initData[], size_t dataSize);
  /*!
    @brief Provides the main implementation of writeMetadata() by
          writing all buffered metadata to the provided BasicIo.
    @param oIo BasicIo instance to write to (a temporary location).

    @return 4 if opening or writing to the associated BasicIo fails
   */
  void doWriteMetadata(BasicIo& outIo);
  //@}

  //! @name Accessors
  //@{
  /*!
    @brief Advances associated io instance to one byte past the next
        Jpeg marker and returns the marker. This method should be called
        when the BasicIo instance is positioned one byte past the end of a
        Jpeg segment.
    @param err the error code to throw if no marker is found
    @return the next Jpeg segment marker if successful;<BR>
            throws an Error if not successful
   */
  [[nodiscard]] byte advanceToMarker(ErrorCode err) const;
  //@}

  DataBuf readNextSegment(byte marker);

  /*!
    @brief Is the marker followed by a non-zero payload?
    @param marker The marker at the start of a segment
    @return true if the marker is followed by a non-zero payload
   */
  static bool markerHasLength(byte marker);
};  // class JpegBase

/*!
  @brief Class to access JPEG images
 */
class EXIV2API JpegImage : public JpegBase {
  friend EXIV2API bool isJpegType(BasicIo& iIo, bool advance);

 public:
  //! @name Creators
  //@{
  /*!
    @brief Constructor that can either open an existing Jpeg image or create
        a new image from scratch. If a new image is to be created, any
        existing data is overwritten. Since the constructor can not return
        a result, callers should check the good() method after object
        construction to determine success or failure.
    @param io An auto-pointer that owns a BasicIo instance used for
        reading and writing image metadata. \b Important: The constructor
        takes ownership of the passed in BasicIo instance through the
        auto-pointer. Callers should not continue to use the BasicIo
        instance after it is passed to this method.  Use the Image::io()
        method to get a temporary reference.
    @param create Specifies if an existing image should be read (false)
        or if a new file should be created (true).
   */
  JpegImage(BasicIo::UniquePtr io, bool create);
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] std::string mimeType() const override;
  //@}

  // NOT Implemented
  //! Copy constructor
  JpegImage(const JpegImage&) = delete;
  //! Assignment operator
  JpegImage& operator=(const JpegImage&) = delete;

 protected:
  //! @name Accessors
  //@{
  bool isThisType(BasicIo& iIo, bool advance) const override;
  //@}
  //! @name Manipulators
  //@{
  /*!
    @brief Writes a Jpeg header (aka signature) to the BasicIo instance.
    @param oIo BasicIo instance that the header is written to.
    @return 0 if successful;<BR>
           2 if the input image is invalid or can not be read;<BR>
           4 if the temporary image can not be written to;<BR>
          -3 other temporary errors
   */
  int writeHeader(BasicIo& outIo) const override;
  //@}

 private:
  // Constant data
  static constexpr byte soi_ = 0xd8;  // SOI marker
  static const byte blank_[];         // Minimal Jpeg image
};                                    // class JpegImage

//! Helper class to access %Exiv2 files
class EXIV2API ExvImage : public JpegBase {
  friend EXIV2API bool isExvType(BasicIo& iIo, bool advance);

 public:
  //! @name Creators
  //@{
  /*!
    @brief Constructor that can either open an existing EXV image or create
        a new image from scratch. If a new image is to be created, any
        existing data is overwritten. Since the constructor can not return
        a result, callers should check the good() method after object
        construction to determine success or failure.
    @param io An auto-pointer that owns a BasicIo instance used for
        reading and writing image metadata. \b Important: The constructor
        takes ownership of the passed in BasicIo instance through the
        auto-pointer. Callers should not continue to use the BasicIo
        instance after it is passed to this method.  Use the Image::io()
        method to get a temporary reference.
    @param create Specifies if an existing image should be read (false)
           or if a new file should be created (true).
   */
  ExvImage(BasicIo::UniquePtr io, bool create);
  //@}
  //! @name Accessors
  //@{
  [[nodiscard]] std::string mimeType() const override;
  //@}

  // NOT Implemented
  //! Copy constructor
  ExvImage(const ExvImage&) = delete;
  //! Assignment operator
  ExvImage& operator=(const ExvImage&) = delete;

 protected:
  //! @name Accessors
  //@{
  bool isThisType(BasicIo& iIo, bool advance) const override;
  //@}
  //! @name Manipulators
  //@{
  int writeHeader(BasicIo& outIo) const override;
  //@}

 private:
  // Constant data
  static constexpr char exiv2Id_[] = "Exiv2";                                          // EXV identifier
  static constexpr byte blank_[] = {0xff, 0x01, 'E', 'x', 'i', 'v', '2', 0xff, 0xd9};  // Minimal exiv2 file

};  // class ExvImage

// *****************************************************************************
// template, inline and free functions

// These could be static private functions on Image subclasses but then
// ImageFactory needs to be made a friend.
/*!
  @brief Create a new JpegImage instance and return an auto-pointer to it.
         Caller owns the returned object and the auto-pointer ensures that
         it will be deleted.
 */
EXIV2API Image::UniquePtr newJpegInstance(BasicIo::UniquePtr io, bool create);
//! Check if the file iIo is a JPEG image.
EXIV2API bool isJpegType(BasicIo& iIo, bool advance);
/*!
  @brief Create a new ExvImage instance and return an auto-pointer to it.
         Caller owns the returned object and the auto-pointer ensures that
         it will be deleted.
 */
EXIV2API Image::UniquePtr newExvInstance(BasicIo::UniquePtr io, bool create);
//! Check if the file iIo is an EXV file
EXIV2API bool isExvType(BasicIo& iIo, bool advance);

}  // namespace Exiv2

#endif  // #ifndef JPGIMAGE_HPP_
