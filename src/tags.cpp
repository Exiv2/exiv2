// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      tags.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   15-Jan-04, ahu: created
             21-Jan-05, ahu: added MakerNote TagInfo registry and related code
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "ifd.hpp"
#include "value.hpp"
#include "makernote.hpp"
#include "mn.hpp"                // To ensure that all makernotes are registered
#include "i18n.h"                // NLS support.

#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <cassert>
#include <cmath>

#ifdef EXV_HAVE_ICONV
# include <iconv.h>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    // Todo: Allow to register new IfdInfo entries from elsewhere (the makernotes)
    // Important: IFD item must be unique!
    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(ifdIdNotSet, "(Unknown IFD)", "(Unknown item)"),
        IfdInfo(ifd0Id, "IFD0", "Image"),
        IfdInfo(exifIfdId, "Exif", "Photo"),  // just to avoid 'Exif.Exif.*' keys
        IfdInfo(gpsIfdId, "GPSInfo", "GPSInfo"),
        IfdInfo(iopIfdId, "Iop", "Iop"),
        IfdInfo(ifd1Id, "IFD1", "Thumbnail"),
        IfdInfo(canonIfdId, "Makernote", "Canon"),
        IfdInfo(canonCsIfdId, "Makernote", "CanonCs"),
        IfdInfo(canonSiIfdId, "Makernote", "CanonSi"),
        IfdInfo(canonCfIfdId, "Makernote", "CanonCf"),
        IfdInfo(canonPiIfdId, "Makernote", "CanonPi"),
        IfdInfo(canonPaIfdId, "Makernote", "CanonPa"),
        IfdInfo(fujiIfdId, "Makernote", "Fujifilm"),
        IfdInfo(minoltaIfdId, "Makernote", "Minolta"),
        IfdInfo(minoltaCs5DIfdId, "Makernote", "MinoltaCs5D"),
        IfdInfo(minoltaCs7DIfdId, "Makernote", "MinoltaCs7D"),
        IfdInfo(minoltaCsOldIfdId, "Makernote", "MinoltaCsOld"),
        IfdInfo(minoltaCsNewIfdId, "Makernote", "MinoltaCsNew"),
        IfdInfo(nikon1IfdId, "Makernote", "Nikon1"),
        IfdInfo(nikon2IfdId, "Makernote", "Nikon2"),
        IfdInfo(nikon3IfdId, "Makernote", "Nikon3"),
        IfdInfo(olympusIfdId, "Makernote", "Olympus"),
        IfdInfo(panasonicIfdId, "Makernote", "Panasonic"),
        IfdInfo(sigmaIfdId, "Makernote", "Sigma"),
        IfdInfo(sonyIfdId, "Makernote", "Sony"),
        IfdInfo(lastIfdId, "(Last IFD info)", "(Last IFD item)")
    };

    SectionInfo::SectionInfo(
        SectionId sectionId,
        const char* name,
        const char* desc
    )
        : sectionId_(sectionId), name_(name), desc_(desc)
    {
    }

    const SectionInfo ExifTags::sectionInfo_[] = {
        SectionInfo(sectionIdNotSet, "(UnknownSection)", N_("Unknown section")),
        SectionInfo(imgStruct, "ImageStructure", N_("Image data structure")),
        SectionInfo(recOffset, "RecordingOffset", N_("Recording offset")),
        SectionInfo(imgCharacter, "ImageCharacteristics", N_("Image data characteristics")),
        SectionInfo(otherTags, "OtherTags", N_("Other data")),
        SectionInfo(exifFormat, "ExifFormat", N_("Exif data structure")),
        SectionInfo(exifVersion, "ExifVersion", N_("Exif version")),
        SectionInfo(imgConfig, "ImageConfig", N_("Image configuration")),
        SectionInfo(userInfo, "UserInfo", N_("User information")),
        SectionInfo(relatedFile, "RelatedFile", N_("Related file")),
        SectionInfo(dateTime, "DateTime", N_("Date and time")),
        SectionInfo(captureCond, "CaptureConditions", N_("Picture taking conditions")),
        SectionInfo(gpsTags, "GPS", N_("GPS information")),
        SectionInfo(iopTags, "Interoperability", N_("Interoperability information")),
        SectionInfo(makerTags, "Makernote", N_("Vendor specific information")),
        SectionInfo(lastSectionId, "(LastSection)", N_("Last section"))
    };

    TagInfo::TagInfo(
        uint16_t tag,
        const char* name,
        const char* title,
        const char* desc,
        IfdId ifdId,
        SectionId sectionId,
        TypeId typeId,
        PrintFct printFct
    )
        : tag_(tag), name_(name), title_(title), desc_(desc), ifdId_(ifdId),
          sectionId_(sectionId), typeId_(typeId), printFct_(printFct)
    {
    }

    //! NewSubfileType, TIFF tag 0x00fe - this is actually a bitmask
    extern const TagDetails exifNewSubfileType[] = {
        {  0, N_("Primary image")                                               },
        {  1, N_("Thumbnail/Preview image")                                     },
        {  2, N_("Primary image, Multi page file")                              },
        {  3, N_("Thumbnail/Preview image, Multi page file")                    },
        {  4, N_("Primary image, Transparency mask")                            },
        {  5, N_("Thumbnail/Preview image, Transparency mask")                  },
        {  6, N_("Primary image, Multi page file, Transparency mask")           },
        {  7, N_("Thumbnail/Preview image, Multi page file, Transparency mask") }
    };

    //! Units for measuring X and Y resolution, tags 0x0128, 0xa210
    extern const TagDetails exifUnit[] = {
        { 1, N_("none") },
        { 2, N_("inch") },
        { 3, N_("cm")   }
    };

    //! Compression, tag 0x0103
    extern const TagDetails exifCompression[] = {
        {     1, N_("Uncompressed")             },
        {     2, N_("CCITT RLE")                },
        {     3, N_("T4/Group 3 Fax")           },
        {     4, N_("T6/Group 4 Fax")           },
        {     5, N_("LZW")                      },
        {     6, N_("JPEG (old-style)")         },
        {     7, N_("JPEG")                     },
        {     8, N_("Adobe Deflate")            },
        {     9, N_("JBIG B&W")                 },
        {    10, N_("JBIG Color")               },
        { 32766, N_("Next 2-bits RLE")          },
        { 32771, N_("CCITT RLE 1-word")         },
        { 32773, N_("PackBits (Macintosh RLE)") },
        { 32809, N_("Thunderscan RLE")          },
        { 32895, N_("IT8 CT Padding")           },
        { 32896, N_("IT8 Linework RLE")         },
        { 32897, N_("IT8 Monochrome Picture")   },
        { 32898, N_("IT8 Binary Lineart")       },
        { 32908, N_("Pixar Film (10-bits LZW)") },
        { 32909, N_("Pixar Log (11-bits ZIP)")  },
        { 32946, N_("Pixar Deflate")            },
        { 32947, N_("Kodak DCS Encoding")       },
        { 34661, N_("ISO JBIG")                 },
        { 34676, N_("SGI Log Luminance RLE")    },
        { 34677, N_("SGI Log 24-bits packed")   },
        { 34712, N_("Leadtools JPEG 2000")      },
        { 34713, N_("Nikon NEF Compressed")     }
    };

    //! PhotometricInterpretation, tag 0x0106
    extern const TagDetails exifPhotometricInterpretation[] = {
        {     0, N_("White Is Zero")      },
        {     1, N_("Black Is Zero")      },
        {     2, N_("RGB")                },
        {     3, N_("RGB Palette")        },
        {     4, N_("Transparency Mask")  },
        {     5, N_("CMYK")               },
        {     6, N_("YCbCr")              },
        {     8, N_("CIELab")             },
        {     9, N_("ICCLab")             },
        {    10, N_("ITULab")             },
        { 32803, N_("Color Filter Array") },
        { 32844, N_("Pixar LogL")         },
        { 32845, N_("Pixar LogLuv")       },
        { 34892, N_("Linear Raw")         }
    };

    //! Orientation, tag 0x0112
    extern const TagDetails exifOrientation[] = {
        { 1, N_("top, left")     },
        { 2, N_("top, right")    },
        { 3, N_("bottom, right") },
        { 4, N_("bottom, left")  },
        { 5, N_("left, top")     },
        { 6, N_("right, top")    },
        { 7, N_("right, bottom") },
        { 8, N_("left, bottom")  }
    };

    //! YCbCrPositioning, tag 0x0213
    extern const TagDetails exifYCbCrPositioning[] = {
        { 1, N_("Centered") },
        { 2, N_("Co-sited") }
    };

    //! Base IFD Tags (IFD0 and IFD1)
    static const TagInfo ifdTagInfo[] = {
        TagInfo(0x00fe, "NewSubfileType", N_("New Subfile Type"),
                N_("A general indication of the kind of data contained in this subfile."),
                ifd0Id, imgStruct, unsignedLong, EXV_PRINT_TAG(exifNewSubfileType)), // TIFF tag
        TagInfo(0x0100, "ImageWidth", N_("Image Width"),
                N_("The number of columns of image data, equal to the number of "
                "pixels per row. In JPEG compressed data a JPEG marker is "
                "used instead of this tag."),
                ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0101, "ImageLength", N_("Image Length"),
                N_("The number of rows of image data. In JPEG compressed data a "
                "JPEG marker is used instead of this tag."),
                ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0102, "BitsPerSample", N_("Bits per Sample"),
                N_("The number of bits per image component. In this standard each "
                "component of the image is 8 bits, so the value for this "
                "tag is 8. See also <SamplesPerPixel>. In JPEG compressed data "
                "a JPEG marker is used instead of this tag."),
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0103, "Compression", N_("Compression"),
                N_("The compression scheme used for the image data. When a "
                "primary image is JPEG compressed, this designation is "
                "not necessary and is omitted. When thumbnails use JPEG "
                "compression, this tag value is set to 6."),
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifCompression)),
        TagInfo(0x0106, "PhotometricInterpretation", N_("Photometric Interpretation"),
                N_("The pixel composition. In JPEG compressed data a JPEG "
                "marker is used instead of this tag."),
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifPhotometricInterpretation)),
        TagInfo(0x010a, "FillOrder", N_("Fill Order"),
                N_("The logical order of bits within a byte"),
                ifd0Id, imgStruct, unsignedShort, printValue), // TIFF tag
        TagInfo(0x010d, "DocumentName", N_("Document Name"),
                N_("The name of the document from which this image was scanned"),
                ifd0Id, imgStruct, asciiString, printValue), // TIFF tag
        TagInfo(0x010e, "ImageDescription", N_("Image Description"),
                N_("A character string giving the title of the image. It may be "
                "a comment such as \"1988 company picnic\" or "
                "the like. Two-bytes character codes cannot be used. "
                "When a 2-bytes code is necessary, the Exif Private tag "
                "<UserComment> is to be used."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x010f, "Make", N_("Manufacturer"),
                N_("The manufacturer of the recording "
                "equipment. This is the manufacturer of the DSC, scanner, "
                "video digitizer or other equipment that generated the "
                "image. When the field is left blank, it is treated as unknown."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0110, "Model", N_("Model"),
                N_("The model name or model number of the equipment. This is the "
                "model name or number of the DSC, scanner, video digitizer "
                "or other equipment that generated the image. When the field "
                "is left blank, it is treated as unknown."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0111, "StripOffsets", N_("Strip Offsets"),
                N_("For each strip, the byte offset of that strip. It is "
                "recommended that this be selected so the number of strip "
                "bytes does not exceed 64 Kbytes. With JPEG compressed "
                "data this designation is not needed and is omitted. See also "
                "<RowsPerStrip> and <StripByteCounts>."),
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0112, "Orientation", N_("Orientation"),
                N_("The image orientation viewed in terms of rows and columns."),
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifOrientation)),
        TagInfo(0x0115, "SamplesPerPixel", N_("Samples per Pixel"),
                N_("The number of components per pixel. Since this standard applies "
                "to RGB and YCbCr images, the value set for this tag is 3. "
                "In JPEG compressed data a JPEG marker is used instead of this tag."),
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0116, "RowsPerStrip", N_("Rows per Strip"),
                N_("The number of rows per strip. This is the number of rows "
                "in the image of one strip when an image is divided into "
                "strips. With JPEG compressed data this designation is not "
                "needed and is omitted. See also <StripOffsets> and <StripByteCounts>."),
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0117, "StripByteCounts", N_("Strip Byte Count"),
                N_("The total number of bytes in each strip. With JPEG compressed "
                "data this designation is not needed and is omitted."),
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x011a, "XResolution", N_("X-Resolution"),
                N_("The number of pixels per <ResolutionUnit> in the <ImageWidth> "
                "direction. When the image resolution is unknown, 72 [dpi] is designated."),
                ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011b, "YResolution", N_("Y-Resolution"),
                N_("The number of pixels per <ResolutionUnit> in the <ImageLength> "
                "direction. The same value as <XResolution> is designated."),
                ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011c, "PlanarConfiguration", N_("Planar Configuration"),
                N_("Indicates whether pixel components are recorded in a chunky "
                "or planar format. In JPEG compressed files a JPEG marker "
                "is used instead of this tag. If this field does not exist, "
                "the TIFF default of 1 (chunky) is assumed."),
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0128, "ResolutionUnit", N_("Resolution Unit"),
                N_("The unit for measuring <XResolution> and <YResolution>. The same "
                "unit is used for both <XResolution> and <YResolution>. If "
                "the image resolution is unknown, 2 (inches) is designated."),
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifUnit)),
        TagInfo(0x012d, "TransferFunction", N_("Transfer Function"),
                N_("A transfer function for the image, described in tabular style. "
                "Normally this tag is not necessary, since color space is "
                "specified in the color space information tag (<ColorSpace>)."),
                ifd0Id, imgCharacter, unsignedShort, printValue),
        TagInfo(0x0131, "Software", N_("Software"),
                N_("This tag records the name and version of the software or "
                "firmware of the camera or image input device used to "
                "generate the image. The detailed format is not specified, but "
                "it is recommended that the example shown below be "
                "followed. When the field is left blank, it is treated as unknown."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0132, "DateTime", N_("Date and Time"),
                N_("The date and time of image creation. In Exif standard, "
                "it is the date and time the file was changed."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013b, "Artist", N_("Artist"),
                N_("This tag records the name of the camera owner, photographer or "
                "image creator. The detailed format is not specified, but it is "
                "recommended that the information be written as in the example "
                "below for ease of Interoperability. When the field is "
                "left blank, it is treated as unknown."),
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013e, "WhitePoint", N_("White Point"),
                N_("The chromaticity of the white point of the image. Normally "
                "this tag is not necessary, since color space is specified "
                "in the colorspace information tag (<ColorSpace>)."),
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x013f, "PrimaryChromaticities", N_("Primary Chromaticities"),
                N_("The chromaticity of the three primary colors of the image. "
                "Normally this tag is not necessary, since colorspace is "
                "specified in the colorspace information tag (<ColorSpace>)."),
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x014a, "SubIFDs", N_("SubIFD Offsets"),
                N_("Defined by Adobe Corporation to enable TIFF Trees within a TIFF file."),
                ifd0Id, otherTags, unsignedLong, printValue),
        TagInfo(0x0156, "TransferRange", N_("Transfer Range"),
                N_("Expands the range of the TransferFunction"),
                ifd0Id, imgCharacter, unsignedShort, printValue), // TIFF tag
        TagInfo(0x0200, "JPEGProc", N_("JPEG Process"),
                N_("This field indicates the process used to produce the compressed data"),
                ifd0Id, recOffset, unsignedLong, printValue), // TIFF tag
        TagInfo(0x0201, "JPEGInterchangeFormat", N_("JPEG Interchange Format"),
                N_("The offset to the start byte (SOI) of JPEG compressed "
                "thumbnail data. This is not used for primary image JPEG data."),
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", N_("JPEG Interchange Format Length"),
                N_("The number of bytes of JPEG compressed thumbnail data. This "
                "is not used for primary image JPEG data. JPEG thumbnails "
                "are not divided but are recorded as a continuous JPEG "
                "bitstream from SOI to EOI. Appn and COM markers should "
                "not be recorded. Compressed thumbnails must be recorded in no "
                "more than 64 Kbytes, including all other data to be recorded in APP1."),
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0211, "YCbCrCoefficients", N_("YCbCr Coefficients"),
                N_("The matrix coefficients for transformation from RGB to YCbCr "
                "image data. No default is given in TIFF; but here the "
                "value given in Appendix E, \"Color Space Guidelines\", is used "
                "as the default. The color space is declared in a "
                "color space information tag, with the default being the value "
                "that gives the optimal image characteristics "
                "Interoperability this condition."),
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x0212, "YCbCrSubSampling", N_("YCbCr Sub-Sampling"),
                N_("The sampling ratio of chrominance components in relation to the "
                "luminance component. In JPEG compressed data a JPEG marker "
                "is used instead of this tag."),
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0213, "YCbCrPositioning", N_("YCbCr Positioning"),
                N_("The position of chrominance components in relation to the "
                "luminance component. This field is designated only for "
                "JPEG compressed data or uncompressed YCbCr data. The TIFF "
                "default is 1 (centered); but when Y:Cb:Cr = 4:2:2 it is "
                "recommended in this standard that 2 (co-sited) be used to "
                "record data, in order to improve the image quality when viewed "
                "on TV systems. When this field does not exist, the reader shall "
                "assume the TIFF default. In the case of Y:Cb:Cr = 4:2:0, the "
                "TIFF default (centered) is recommended. If the reader "
                "does not have the capability of supporting both kinds of "
                "<YCbCrPositioning>, it shall follow the TIFF default regardless "
                "of the value in this field. It is preferable that readers "
                "be able to support both centered and co-sited positioning."),
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifYCbCrPositioning)),
        TagInfo(0x0214, "ReferenceBlackWhite", N_("Reference Black/White"),
                N_("The reference black point value and reference white point "
                "value. No defaults are given in TIFF, but the values "
                "below are given as defaults here. The color space is declared "
                "in a color space information tag, with the default "
                "being the value that gives the optimal image characteristics "
                "Interoperability these conditions."),
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x02bc, "XMLPacket", N_("XML Packet"),
                N_("XMP Metadata (Adobe technote 9-14-02)"),
                ifd0Id, otherTags, unsignedByte, printValue),
        TagInfo(0x4746, "Rating", N_("Windows Rating"),
                N_("Rating tag used by Windows"),
                ifd0Id, otherTags, unsignedShort, printValue), // Windows Tag
        TagInfo(0x4749, "RatingPercent", N_("Windows Rating Percent"),
                N_("Rating tag used by Windows, value in percent"),
                ifd0Id, otherTags, unsignedShort, printValue), // Windows Tag
        TagInfo(0x828d, "CFARepeatPatternDim", N_("CFA Repeat Pattern Dimension"),
                N_("Contains two values representing the minimum rows and columns "
                "to define the repeating patterns of the color filter array"),
                ifd0Id, otherTags, unsignedShort, printValue), // TIFF/EP Tag
        TagInfo(0x828e, "CFAPattern", N_("CFA Pattern"),
                N_("Indicates the color filter array (CFA) geometric pattern of the image "
                "sensor when a one-chip color area sensor is used. It does not apply to "
                "all sensing methods"),
                ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        TagInfo(0x828f, "BatteryLevel", N_("Battery Level"),
                "Contains a value of the battery level as a fraction or string",
                ifd0Id, otherTags, unsignedRational, printValue), // TIFF/EP Tag
        TagInfo(0x83bb, "IPTCNAA", N_("IPTC/NAA"),
                N_("Contains an IPTC/NAA record"),
                ifd0Id, otherTags, unsignedLong, printValue), // TIFF/EP Tag
        TagInfo(0x8298, "Copyright", N_("Copyright"),
                N_("Copyright information. In this standard the tag is used to "
                "indicate both the photographer and editor copyrights. It is "
                "the copyright notice of the person or organization claiming "
                "rights to the image. The Interoperability copyright "
                "statement including date and rights should be written in this "
                "field; e.g., \"Copyright, John Smith, 19xx. All rights "
                "reserved.\". In this standard the field records both the "
                "photographer and editor copyrights, with each recorded in a "
                "separate part of the statement. When the field "
                "is left blank, it is treated as unknown."),
                ifd0Id, otherTags, asciiString, print0x8298),
        TagInfo(0x8649, "ImageResources", N_("Image Resources Block"),
                N_("Contains information embedded by the Adobe Photoshop application"),
                ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8769, "ExifTag", N_("Exif IFD Pointer"),
                N_("A pointer to the Exif IFD. Interoperability, Exif IFD has the "
                "same structure as that of the IFD specified in TIFF. "
                "ordinarily, however, it does not contain image data as in "
                "the case of TIFF."),
                ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x8773, "InterColorProfile", N_("Inter Color Profile"),
                N_("Contains an InterColor Consortium (ICC) format color space characterization/profile"),
                ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8825, "GPSTag", N_("GPS Info IFD Pointer"),
                N_("A pointer to the GPS Info IFD. The "
                "Interoperability structure of the GPS Info IFD, like that of "
                "Exif IFD, has no image data."),
                ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x9216, "TIFFEPStandardID", N_("TIFF/EP Standard ID"),
                N_("Contains four ASCII characters representing the TIFF/EP standard "
                "version of a TIFF/EP file, eg '1', '0', '0', '0'"),
                ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        TagInfo(0x9c9b, "XPTitle", N_("Windows Title"),
                N_("Title tag used by Windows, encoded in UCS2"),
                ifd0Id, otherTags, unsignedByte, printUcs2), // Windows Tag
        TagInfo(0x9c9c, "XPComment", N_("Windows Comment"),
                N_("Comment tag used by Windows, encoded in UCS2"),
                ifd0Id, otherTags, unsignedByte, printUcs2), // Windows Tag
        TagInfo(0x9c9d, "XPAuthor", N_("Windows Author"),
                N_("Author tag used by Windows, encoded in UCS2"),
                ifd0Id, otherTags, unsignedByte, printUcs2), // Windows Tag
        TagInfo(0x9c9e, "XPKeywords", N_("Windows Keywords"),
                N_("Keywords tag used by Windows, encoded in UCS2"),
                ifd0Id, otherTags, unsignedByte, printUcs2), // Windows Tag
        TagInfo(0x9c9f, "XPSubject", N_("Windows Subject"),
                N_("Subject tag used by Windows, encoded in UCS2"),
                ifd0Id, otherTags, unsignedByte, printUcs2), // Windows Tag
        // End of list marker
        TagInfo(0xffff, "(UnknownIfdTag)", N_("Unknown IFD tag"),
                N_("Unknown IFD tag"),
                ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    const TagInfo* ExifTags::ifdTagList()
    {
        return ifdTagInfo;
    }

    //! ExposureProgram, tag 0x8822
    extern const TagDetails exifExposureProgram[] = {
        { 0, N_("Not defined")       },
        { 1, N_("Manual")            },
        { 2, N_("Auto")              },
        { 3, N_("Aperture priority") },
        { 4, N_("Shutter priority")  },
        { 5, N_("Creative program")  },
        { 6, N_("Action program")    },
        { 7, N_("Portrait mode")     },
        { 8, N_("Landscape mode")    }
    };

    //! MeteringMode, tag 0x9207
    extern const TagDetails exifMeteringMode[] = {
        { 0,   N_("Unknown")                 },
        { 1,   N_("Average")                 },
        { 2,   N_("Center weighted average") },
        { 3,   N_("Spot")                    },
        { 4,   N_("Multi-spot")              },
        { 5,   N_("Multi-segment")           },
        { 6,   N_("Partial")                 },
        { 255, N_("Other")                   }
    };

    //! LightSource, tag 0x9208
    extern const TagDetails exifLightSource[] = {
        {   0, N_("Unknown")                                 },
        {   1, N_("Daylight")                                },
        {   2, N_("Fluorescent")                             },
        {   3, N_("Tungsten (incandescent light)")           },
        {   4, N_("Flash")                                   },
        {   9, N_("Fine weather")                            },
        {  10, N_("Cloudy weather")                          },
        {  11, N_("Shade")                                   },
        {  12, N_("Daylight fluorescent (D 5700 - 7100K)")   },
        {  13, N_("Day white fluorescent (N 4600 - 5400K)")  },
        {  14, N_("Cool white fluorescent (W 3900 - 4500K)") },
        {  15, N_("White fluorescent (WW 3200 - 3700K)")     },
        {  17, N_("Standard light A")                        },
        {  18, N_("Standard light B")                        },
        {  19, N_("Standard light C")                        },
        {  20, N_("D55")                                     },
        {  21, N_("D65")                                     },
        {  22, N_("D75")                                     },
        {  23, N_("D50")                                     },
        {  24, N_("ISO studio tungsten")                     },
        { 255, N_("Other light source")                      }
    };

    //! Flash, tag 0x9209
    extern const TagDetails exifFlash[] = {
        { 0x00, N_("No flash")                                                      },
        { 0x01, N_("Fired")                                                         },
        { 0x05, N_("Fired, strobe return light not detected")                       },
        { 0x07, N_("Fired, strobe return light detected")                           },
        { 0x09, N_("Yes, compulsory")                                               },
        { 0x0d, N_("Yes, compulsory, return light not detected")                    },
        { 0x0f, N_("Yes, compulsory, return light detected")                        },
        { 0x10, N_("No, compulsory")                                                },
        { 0x18, N_("No, auto")                                                      },
        { 0x19, N_("Yes, auto")                                                     },
        { 0x1d, N_("Yes, auto, return light not detected")                          },
        { 0x1f, N_("Yes, auto, return light detected")                              },
        { 0x20, N_("No flash function")                                             },
        { 0x41, N_("Yes, red-eye reduction")                                        },
        { 0x45, N_("Yes, red-eye reduction, return light not detected")             },
        { 0x47, N_("Yes, red-eye reduction, return light detected")                 },
        { 0x49, N_("Yes, compulsory, red-eye reduction")                            },
        { 0x4d, N_("Yes, compulsory, red-eye reduction, return light not detected") },
        { 0x4f, N_("Yes, compulsory, red-eye reduction, return light detected")     },
        { 0x59, N_("Yes, auto, red-eye reduction")                                  },
        { 0x5d, N_("Yes, auto, red-eye reduction, return light not detected")       },
        { 0x5f, N_("Yes, auto, red-eye reduction, return light detected")           }
    };

    //! ColorSpace, tag 0xa001
    extern const TagDetails exifColorSpace[] = {
        {      1, N_("sRGB")         },
        {      2, N_("Adobe RGB")    },    // Not defined to Exif 2.2 spec. But used by a lot of cameras.
        { 0xffff, N_("Uncalibrated") }
    };

    //! SensingMethod, tag 0xa217
    extern const TagDetails exifSensingMethod[] = {
        { 1, N_("Not defined")             },
        { 2, N_("One-chip color area")     },
        { 3, N_("Two-chip color area")     },
        { 4, N_("Three-chip color area")   },
        { 5, N_("Color sequential area")   },
        { 7, N_("Trilinear sensor")        },
        { 8, N_("Color sequential linear") }
    };

    //! FileSource, tag 0xa300
    extern const TagDetails exifFileSource[] = {
        { 1, N_("Film scanner")            },	// Not defined to Exif 2.2 spec.
        { 2, N_("Reflexion print scanner") },	// but used by some scanner device softwares.
        { 3, N_("Digital still camera")    }
    };

    //! SceneType, tag 0xa301
    extern const TagDetails exifSceneType[] = {
        { 1, N_("Directly photographed") }
    };

    //! exifCustomRendered, tag 0xa401
    extern const TagDetails exifCustomRendered[] = {
        { 0, N_("Normal process") },
        { 1, N_("Custom process") }
    };

    //! ExposureMode, tag 0xa402
    extern const TagDetails exifExposureMode[] = {
        { 0, N_("Auto")         },
        { 1, N_("Manual")       },
        { 2, N_("Auto bracket") }
    };

    //! WhiteBalance, tag 0xa403
    extern const TagDetails exifWhiteBalance[] = {
        { 0, N_("Auto")   },
        { 1, N_("Manual") }
    };

    //! SceneCaptureType, tag 0xa406
    extern const TagDetails exifSceneCaptureType[] = {
        { 0, N_("Standard")    },
        { 1, N_("Landscape")   },
        { 2, N_("Portrait")    },
        { 3, N_("Night scene") }
    };

    //! GainControl, tag 0xa407
    extern const TagDetails exifGainControl[] = {
        { 0, N_("None")           },
        { 1, N_("Low gain up")    },
        { 2, N_("High gain up")   },
        { 3, N_("Low gain down")  },
        { 4, N_("High gain down") }
    };

    //! Contrast, tag 0xa408
    extern const TagDetails exifContrast[] = {
        { 0, N_("Normal") },
        { 1, N_("Soft")   },
        { 2, N_("Hard")   }
    };

    //! Saturation, tag 0xa409
    extern const TagDetails exifSaturation[] = {
        { 0, N_("Normal") },
        { 1, N_("Low")    },
        { 2, N_("High")   }
    };

    //! Sharpness, tag 0xa40a
    extern const TagDetails exifSharpness[] = {
        { 0, N_("Normal") },
        { 1, N_("Soft")   },
        { 2, N_("Hard")   }
    };

    //! SubjectDistanceRange, tag 0xa40c
    extern const TagDetails exifSubjectDistanceRange[] = {
        { 0, N_("Unknown")      },
        { 1, N_("Macro")        },
        { 2, N_("Close view")   },
        { 3, N_("Distant view") }
    };

    // Exif IFD Tags
    static const TagInfo exifTagInfo[] = {
        TagInfo(0x829a, "ExposureTime", N_("Exposure Time"),
                N_("Exposure time, given in seconds (sec)."),
                exifIfdId, captureCond, unsignedRational, print0x829a),
        TagInfo(0x829d, "FNumber", N_("FNumber"),
                N_("The F number."),
                exifIfdId, captureCond, unsignedRational, print0x829d),
        TagInfo(0x8822, "ExposureProgram", N_("Exposure Program"),
                N_("The class of the program used by the camera to set exposure "
                "when the picture is taken."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifExposureProgram)),
        TagInfo(0x8824, "SpectralSensitivity", N_("Spectral Sensitivity"),
                N_("Indicates the spectral sensitivity of each channel of the "
                "camera used. The tag value is an ASCII string compatible "
                "with the standard developed by the ASTM Technical Committee."),
                exifIfdId, captureCond, asciiString, printValue),
        TagInfo(0x8827, "ISOSpeedRatings", N_("ISO Speed Ratings"),
                N_("Indicates the ISO Speed and ISO Latitude of the camera or "
                "input device as specified in ISO 12232."),
                exifIfdId, captureCond, unsignedShort, print0x8827),
        TagInfo(0x8828, "OECF", N_("Opto-Electoric Conversion Function"),
                N_("Indicates the Opto-Electoric Conversion Function (OECF) "
                "specified in ISO 14524. <OECF> is the relationship between "
                "the camera optical input and the image values."),
                exifIfdId, captureCond, undefined, printValue),
        TagInfo(0x9000, "ExifVersion", N_("Exif Version"),
                N_("The version of this standard supported. Nonexistence of this "
                "field is taken to mean nonconformance to the standard."),
                exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0x9003, "DateTimeOriginal", N_("Date and Time (original)"),
                N_("The date and time when the original image data was generated. "
                "For a digital still camera the date and time the picture was taken are recorded."),
                exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9004, "DateTimeDigitized", N_("Date and Time (digitized)"),
                N_("The date and time when the image was stored as digital data."),
                exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9101, "ComponentsConfiguration", N_("Components Configuration"),
                N_("Information specific to compressed data. The channels of "
                "each component are arranged in order from the 1st "
                "component to the 4th. For uncompressed data the data "
                "arrangement is given in the <PhotometricInterpretation> tag. "
                "However, since <PhotometricInterpretation> can only "
                "express the order of Y, Cb and Cr, this tag is provided "
                "for cases when compressed data uses components other than "
                "Y, Cb, and Cr and to enable support of other sequences."),
                exifIfdId, imgConfig, undefined, print0x9101),
        TagInfo(0x9102, "CompressedBitsPerPixel", N_("Compressed Bits per Pixel"),
                N_("Information specific to compressed data. The compression mode "
                "used for a compressed image is indicated in unit bits per pixel."),
                exifIfdId, imgConfig, unsignedRational, printFloat),
        TagInfo(0x9201, "ShutterSpeedValue", N_("Shutter speed"),
                N_("Shutter speed. The unit is the APEX (Additive System of "
                "Photographic Exposure) setting."),
                exifIfdId, captureCond, signedRational, print0x9201),
        TagInfo(0x9202, "ApertureValue", N_("Aperture"),
                N_("The lens aperture. The unit is the APEX value."),
                exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9203, "BrightnessValue", N_("Brightness"),
                N_("The value of brightness. The unit is the APEX value. "
                "Ordinarily it is given in the range of -99.99 to 99.99."),
                exifIfdId, captureCond, signedRational, printFloat),
        TagInfo(0x9204, "ExposureBiasValue", N_("Exposure Bias"),
                N_("The exposure bias. The units is the APEX value. Ordinarily "
                "it is given in the range of -99.99 to 99.99."),
                exifIfdId, captureCond, signedRational, print0x9204),
        TagInfo(0x9205, "MaxApertureValue", N_("Max Aperture Value"),
                N_("The smallest F number of the lens. The unit is the APEX value. "
                "Ordinarily it is given in the range of 00.00 to 99.99, "
                "but it is not limited to this range."),
                exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9206, "SubjectDistance", N_("Subject Distance"),
                N_("The distance to the subject, given in meters."),
                exifIfdId, captureCond, unsignedRational, print0x9206),
        TagInfo(0x9207, "MeteringMode", N_("Metering Mode"),
                N_("The metering mode."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifMeteringMode)),
        TagInfo(0x9208, "LightSource", N_("Light Source"),
                N_("The kind of light source."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifLightSource)),
        TagInfo(0x9209, "Flash", N_("Flash"),
                N_("This tag is recorded when an image is taken using a strobe light (flash)."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifFlash)),
        TagInfo(0x920a, "FocalLength", N_("Focal Length"),
                N_("The actual focal length of the lens, in mm. Conversion is not "
                "made to the focal length of a 35 mm film camera."),
                exifIfdId, captureCond, unsignedRational, print0x920a),
        TagInfo(0x9214, "SubjectArea", N_("Subject Area"),
                N_("This tag indicates the location and area of the main subject "
                "in the overall scene."),
                exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0x927c, "MakerNote", N_("Maker Note"),
                N_("A tag for manufacturers of Exif writers to record any desired "
                "information. The contents are up to the manufacturer."),
                exifIfdId, userInfo, undefined, printValue),
        TagInfo(0x9286, "UserComment", N_("User Comment"),
                N_("A tag for Exif users to write keywords or comments on the image "
                "besides those in <ImageDescription>, and without the "
                "character code limitations of the <ImageDescription> tag."),
                exifIfdId, userInfo, comment, print0x9286),
        TagInfo(0x9290, "SubSecTime", N_("Sub-seconds Time"),
                N_("A tag used to record fractions of seconds for the <DateTime> tag."),
                exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9291, "SubSecTimeOriginal", N_("Sub-seconds Time Original"),
                N_("A tag used to record fractions of seconds for the <DateTimeOriginal> tag."),
                exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9292, "SubSecTimeDigitized", N_("Sub-seconds Time Digitized"),
                N_("A tag used to record fractions of seconds for the <DateTimeDigitized> tag."),
                exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0xa000, "FlashpixVersion", N_("FlashPix Version"),
                N_("The FlashPix format version supported by a FPXR file."),
                exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0xa001, "ColorSpace", N_("Color Space"),
                N_("The color space information tag is always "
                "recorded as the color space specifier. Normally sRGB "
                "is used to define the color space based on the PC monitor "
                "conditions and environment. If a color space other than "
                "sRGB is used, Uncalibrated is set. Image data "
                "recorded as Uncalibrated can be treated as sRGB when it is "
                "converted to FlashPix."),
                exifIfdId, imgCharacter, unsignedShort, EXV_PRINT_TAG(exifColorSpace)),
        TagInfo(0xa002, "PixelXDimension", N_("Pixel X Dimension"),
                N_("Information specific to compressed data. When a "
                "compressed file is recorded, the valid width of the "
                "meaningful image must be recorded in this tag, whether or "
                "not there is padding data or a restart marker. This tag "
                "should not exist in an uncompressed file."),
                exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa003, "PixelYDimension", N_("Pixel Y Dimension"),
                N_("Information specific to compressed data. When a compressed "
                "file is recorded, the valid height of the meaningful image "
                "must be recorded in this tag, whether or not there is padding "
                "data or a restart marker. This tag should not exist in an "
                "uncompressed file. Since data padding is unnecessary in the vertical "
                "direction, the number of lines recorded in this valid image height tag "
	        "will in fact be the same as that recorded in the SOF."),
                exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa004, "RelatedSoundFile", N_("Related Sound File"),
                N_("This tag is used to record the name of an audio file related "
                "to the image data. The only relational information "
                "recorded here is the Exif audio file name and extension (an "
                "ASCII string consisting of 8 characters + '.' + 3 "
                "characters). The path is not recorded."),
                exifIfdId, relatedFile, asciiString, printValue),
        TagInfo(0xa005, "InteroperabilityTag", N_("Interoperability IFD Pointer"),
                N_("Interoperability IFD is composed of tags which stores the "
                "information to ensure the Interoperability and pointed "
                "by the following tag located in Exif IFD. "
                "The Interoperability structure of Interoperability IFD is "
                "the same as TIFF defined IFD structure but does not contain the "
                "image data characteristically compared with normal TIFF IFD."),
                exifIfdId, exifFormat, unsignedLong, printValue),
        TagInfo(0xa20b, "FlashEnergy", N_("Flash Energy"),
                N_("Indicates the strobe energy at the time the image is "
                "captured, as measured in Beam Candle Power Seconds (BCPS)."),
                exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa20c, "SpatialFrequencyResponse", N_("Spatial Frequency Response"),
                N_("This tag records the camera or input device spatial frequency "
                "table and SFR values in the direction of image width, "
                "image height, and diagonal direction, as specified in ISO 12233."),
                exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa20e, "FocalPlaneXResolution", N_("Focal Plane X-Resolution"),
                N_("Indicates the number of pixels in the image width (X) direction "
                "per <FocalPlaneResolutionUnit> on the camera focal plane."),
                exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa20f, "FocalPlaneYResolution", N_("Focal Plane Y-Resolution"),
                N_("Indicates the number of pixels in the image height (V) direction "
                "per <FocalPlaneResolutionUnit> on the camera focal plane."),
                exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", N_("Focal Plane Resolution Unit"),
                N_("Indicates the unit for measuring <FocalPlaneXResolution> and "
                "<FocalPlaneYResolution>. This value is the same as the <ResolutionUnit>."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifUnit)),
        TagInfo(0xa214, "SubjectLocation", N_("Subject Location"),
                N_("Indicates the location of the main subject in the scene. The "
                "value of this tag represents the pixel at the center of the "
                "main subject relative to the left edge, prior to rotation "
                "processing as per the <Rotation> tag. The first value "
                "indicates the X column number and second indicates the Y row number."),
                exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0xa215, "ExposureIndex", N_("Exposure index"),
                N_("Indicates the exposure index selected on the camera or "
                "input device at the time the image is captured."),
                exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa217, "SensingMethod", N_("Sensing Method"),
                N_("Indicates the image sensor type on the camera or input device."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSensingMethod)),
        TagInfo(0xa300, "FileSource", N_("File Source"),
                N_("Indicates the image source. If a DSC recorded the image, "
                "this tag value of this tag always be set to 3, indicating "
                "that the image was recorded on a DSC."),
                exifIfdId, captureCond, undefined, EXV_PRINT_TAG(exifFileSource)),
        TagInfo(0xa301, "SceneType", N_("Scene Type"),
                N_("Indicates the type of scene. If a DSC recorded the image, "
                "this tag value must always be set to 1, indicating that the "
                "image was directly photographed."),
                exifIfdId, captureCond, undefined, EXV_PRINT_TAG(exifSceneType)),
        TagInfo(0xa302, "CFAPattern", N_("Color Filter Array Pattern"),
                N_("Indicates the color filter array (CFA) geometric pattern of the "
                "image sensor when a one-chip color area sensor is used. "
                "It does not apply to all sensing methods."),
                exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa401, "CustomRendered", N_("Custom Rendered"),
                N_("This tag indicates the use of special processing on image "
                "data, such as rendering geared to output. When special "
                "processing is performed, the reader is expected to disable "
                "or minimize any further processing."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifCustomRendered)),
        TagInfo(0xa402, "ExposureMode", N_("Exposure Mode"),
                N_("This tag indicates the exposure mode set when the image was "
                "shot. In auto-bracketing mode, the camera shoots a series of "
                "frames of the same scene at different exposure settings."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifExposureMode)),
        TagInfo(0xa403, "WhiteBalance", N_("White Balance"),
                N_("This tag indicates the white balance mode set when the image was shot."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifWhiteBalance)),
        TagInfo(0xa404, "DigitalZoomRatio", N_("Digital Zoom Ratio"),
                N_("This tag indicates the digital zoom ratio when the image was "
                "shot. If the numerator of the recorded value is 0, this "
                "indicates that digital zoom was not used."),
                exifIfdId, captureCond, unsignedRational, print0xa404),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", N_("Focal Length In 35mm Film"),
                N_("This tag indicates the equivalent focal length assuming a "
                "35mm film camera, in mm. A value of 0 means the focal "
                "length is unknown. Note that this tag differs from the "
                "<FocalLength> tag."),
                exifIfdId, captureCond, unsignedShort, print0xa405),
        TagInfo(0xa406, "SceneCaptureType", N_("Scene Capture Type"),
                N_("This tag indicates the type of scene that was shot. It can "
                "also be used to record the mode in which the image was "
                "shot. Note that this differs from the <SceneType> tag."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSceneCaptureType)),
        TagInfo(0xa407, "GainControl", N_("Gain Control"),
                N_("This tag indicates the degree of overall image gain adjustment."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifGainControl)),
        TagInfo(0xa408, "Contrast", N_("Contrast"),
                N_("This tag indicates the direction of contrast processing "
                "applied by the camera when the image was shot."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifContrast)),
        TagInfo(0xa409, "Saturation", N_("Saturation"),
                N_("This tag indicates the direction of saturation processing "
                "applied by the camera when the image was shot."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSaturation)),
        TagInfo(0xa40a, "Sharpness", N_("Sharpness"),
                N_("This tag indicates the direction of sharpness processing "
                "applied by the camera when the image was shot."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSharpness)),
        TagInfo(0xa40b, "DeviceSettingDescription", N_("Device Setting Description"),
                N_("This tag indicates information on the picture-taking "
                "conditions of a particular camera model. The tag is used "
                "only to indicate the picture-taking conditions in the reader."),
                exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa40c, "SubjectDistanceRange", N_("Subject Distance Range"),
                N_("This tag indicates the distance to the subject."),
                exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSubjectDistanceRange)),
        TagInfo(0xa420, "ImageUniqueID", N_("Image Unique ID"),
                N_("This tag indicates an identifier assigned uniquely to "
                "each image. It is recorded as an ASCII string equivalent "
                "to hexadecimal notation and 128-bit fixed length."),
                exifIfdId, otherTags, asciiString, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownExifTag)", N_("Unknown Exif tag"),
                N_("Unknown Exif tag"),
                ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    const TagInfo* ExifTags::exifTagList()
    {
        return exifTagInfo;
    }

    //! GPS latitude reference, tag 0x0001; also GPSDestLatitudeRef, tag 0x0013
    extern const TagDetails exifGPSLatitudeRef[] = {
        { 78, N_("North") },
        { 83, N_("South") }
    };

    //! GPS longitude reference, tag 0x0003; also GPSDestLongitudeRef, tag 0x0015
    extern const TagDetails exifGPSLongitudeRef[] = {
        { 69, N_("East") },
        { 87, N_("West") }
    };

    //! GPS altitude reference, tag 0x0005
    extern const TagDetails exifGPSAltitudeRef[] = {
        { 0, N_("Above sea level") },
        { 1, N_("Below sea level") }
    };

    //! GPS speed reference, tag 0x000c
    extern const TagDetails exifGPSSpeedRef[] = {
        { 75, N_("km/h")  },
        { 77, N_("mph")   },
        { 78, N_("knots") }
    };

    // GPS Info Tags
    static const TagInfo gpsTagInfo[] = {
        TagInfo(0x0000, "GPSVersionID", N_("GPS Version ID"),
                N_("Indicates the version of <GPSInfoIFD>. The version is given "
                "as 2.0.0.0. This tag is mandatory when <GPSInfo> tag is "
                "present. (Note: The <GPSVersionID> tag is given in bytes, "
                "unlike the <ExifVersion> tag. When the version is "
                "2.0.0.0, the tag value is 02000000.H)."),
                gpsIfdId, gpsTags, unsignedByte, printValue),
        TagInfo(0x0001, "GPSLatitudeRef", N_("GPS Latitude Reference"),
                N_("Indicates whether the latitude is north or south latitude. The "
                "ASCII value 'N' indicates north latitude, and 'S' is south latitude."),
                gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLatitudeRef)),
        TagInfo(0x0002, "GPSLatitude", N_("GPS Latitude"),
                N_("Indicates the latitude. The latitude is expressed as three "
                "RATIONAL values giving the degrees, minutes, and seconds, "
                "respectively. When degrees, minutes and seconds are expressed, "
                "the format is dd/1,mm/1,ss/1. When degrees and minutes are used "
                "and, for example, fractions of minutes are given up to two "
                "decimal places, the format is dd/1,mmmm/100,0/1."),
                gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0003, "GPSLongitudeRef", N_("GPS Longitude Reference"),
                N_("Indicates whether the longitude is east or west longitude. "
                "ASCII 'E' indicates east longitude, and 'W' is west longitude."),
                gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLongitudeRef)),
        TagInfo(0x0004, "GPSLongitude", N_("GPS Longitude"),
                N_("Indicates the longitude. The longitude is expressed as three "
                "RATIONAL values giving the degrees, minutes, and seconds, "
                "respectively. When degrees, minutes and seconds are expressed, "
                "the format is ddd/1,mm/1,ss/1. When degrees and minutes are "
                "used and, for example, fractions of minutes are given up to "
                "two decimal places, the format is ddd/1,mmmm/100,0/1."),
                gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0005, "GPSAltitudeRef", N_("GPS Altitude Reference"),
                N_("Indicates the altitude used as the reference altitude. If the "
                "reference is sea level and the altitude is above sea level, 0 "
                "is given. If the altitude is below sea level, a value of 1 is given "
                "and the altitude is indicated as an absolute value in the "
                "GSPAltitude tag. The reference unit is meters. Note that this tag "
                "is BYTE type, unlike other reference tags."),
                gpsIfdId, gpsTags, unsignedByte, EXV_PRINT_TAG(exifGPSAltitudeRef)),
        TagInfo(0x0006, "GPSAltitude", N_("GPS Altitude"),
                N_("Indicates the altitude based on the reference in GPSAltitudeRef. "
                "Altitude is expressed as one RATIONAL value. The reference unit is meters."),
                gpsIfdId, gpsTags, unsignedRational, print0x0006),
        TagInfo(0x0007, "GPSTimeStamp", N_("GPS Time Stamp"),
                N_("Indicates the time as UTC (Coordinated Universal Time). "
                "<TimeStamp> is expressed as three RATIONAL values "
                "giving the hour, minute, and second (atomic clock)."),
                gpsIfdId, gpsTags, unsignedRational, print0x0007),
        TagInfo(0x0008, "GPSSatellites", N_("GPS Satellites"),
                N_("Indicates the GPS satellites used for measurements. This tag can be used "
                "to describe the number of satellites, their ID number, angle of elevation, "
                "azimuth, SNR and other information in ASCII notation. The format is not "
                "specified. If the GPS receiver is incapable of taking measurements, value "
                "of the tag is set to NULL."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0009, "GPSStatus", N_("GPS Status"),
                N_("Indicates the status of the GPS receiver when the image is recorded. "
                "\"A\" means measurement is in progress, and \"V\" means the measurement "
                "is Interoperability."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000a, "GPSMeasureMode", N_("GPS Measure Mode"),
                N_("Indicates the GPS measurement mode. \"2\" means two-dimensional measurement and \"3\" "
                "means three-dimensional measurement is in progress."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000b, "GPSDOP", N_("GPS Data Degree of Precision"),
                N_("Indicates the GPS DOP (data degree of precision). An HDOP value is written "
                "during two-dimensional measurement, and PDOP during three-dimensional measurement."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000c, "GPSSpeedRef", N_("GPS Speed Reference"),
                N_("Indicates the unit used to express the GPS receiver speed of movement. "
                "\"K\" \"M\" and \"N\" represents kilometers per hour, miles per hour, and knots."),
                gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSSpeedRef)),
        TagInfo(0x000d, "GPSSpeed", N_("GPS Speed"),
                N_("Indicates the speed of GPS receiver movement."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000e, "GPSTrackRef", N_("GPS Track Ref"),
                N_("Indicates the reference for giving the direction of GPS receiver movement. "
                "\"T\" denotes true direction and \"M\" is magnetic direction."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000f, "GPSTrack", N_("GPS Track"),
                N_("Indicates the direction of GPS receiver movement. The range of values is "
                "from 0.00 to 359.99."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0010, "GPSImgDirectionRef", N_("GPS Image Direction Reference"),
                N_("Indicates the reference for giving the direction of the image when it is captured. "
                "\"T\" denotes true direction and \"M\" is magnetic direction."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0011, "GPSImgDirection", N_("GPS Image Direction"),
                N_("Indicates the direction of the image when it was captured. The range of values "
                "is from 0.00 to 359.99."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0012, "GPSMapDatum", N_("GPS Map Datum"),
                N_("Indicates the geodetic survey data used by the GPS receiver. If the survey data "
                "is restricted to Japan, the value of this tag is \"TOKYO\" or \"WGS-84\"."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0013, "GPSDestLatitudeRef", N_("GPS Destination Latitude Refeference"),
                N_("Indicates whether the latitude of the destination point is north or south latitude. "
                "The ASCII value \"N\" indicates north latitude, and \"S\" is south latitude."),
                gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLatitudeRef)),
        TagInfo(0x0014, "GPSDestLatitude", N_("GPS Destination Latitude"),
                N_("Indicates the latitude of the destination point. The latitude is expressed as "
                "three RATIONAL values giving the degrees, minutes, and seconds, respectively. "
                "If latitude is expressed as degrees, minutes and seconds, a typical format would "
                "be dd/1,mm/1,ss/1. When degrees and minutes are used and, for example, "
                "fractions of minutes are given up to two decimal places, the format would be "
                "dd/1,mmmm/100,0/1."),
                gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0015, "GPSDestLongitudeRef", N_("GPS Destination Longitude Reference"),
                N_("Indicates whether the longitude of the destination point is east or west longitude. "
                "ASCII \"E\" indicates east longitude, and \"W\" is west longitude."),
                gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLongitudeRef)),
        TagInfo(0x0016, "GPSDestLongitude", N_("GPS Destination Longitude"),
                N_("Indicates the longitude of the destination point. The longitude is expressed "
                "as three RATIONAL values giving the degrees, minutes, and seconds, respectively. "
                "If longitude is expressed as degrees, minutes and seconds, a typical format would be "
                "ddd/1,mm/1,ss/1. When degrees and minutes are used and, for example, fractions of "
                "minutes are given up to two decimal places, the format would be ddd/1,mmmm/100,0/1."),
                gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0017, "GPSDestBearingRef", N_("GPS Destination Bearing Reference"),
                N_("Indicates the reference used for giving the bearing to the destination point. "
                "\"T\" denotes true direction and \"M\" is magnetic direction."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0018, "GPSDestBearing", N_("GPS Destination Bearing"),
                N_("Indicates the bearing to the destination point. The range of values is from "
                "0.00 to 359.99."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0019, "GPSDestDistanceRef", N_("GPS Destination Distance Reference"),
                N_("Indicates the unit used to express the distance to the destination point. "
                "\"K\", \"M\" and \"N\" represent kilometers, miles and knots."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001a, "GPSDestDistance", N_("GPS Destination Distance"),
                N_("Indicates the distance to the destination point."),
                gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x001b, "GPSProcessingMethod", N_("GPS Processing Method"),
                N_("A character string recording the name of the method used for location finding. "
                "The first byte indicates the character code used, and this is followed by the name "
                "of the method."),
                gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001c, "GPSAreaInformation", N_("GPS Area Information"),
                N_("A character string recording the name of the GPS area. The first byte indicates "
                "the character code used, and this is followed by the name of the GPS area."),
                gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001d, "GPSDateStamp", N_("GPS Date Stamp"),
                N_("A character string recording date and time information relative to UTC "
                "(Coordinated Universal Time). The format is \"YYYY:MM:DD.\"."),
                gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001e, "GPSDifferential", N_("GPS Differential"),
                N_("Indicates whether differential correction is applied to the GPS receiver."),
                gpsIfdId, gpsTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownGpsTag)", N_("Unknown GPSInfo tag"),
                N_("Unknown GPSInfo tag"),
                ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    const TagInfo* ExifTags::gpsTagList()
    {
        return gpsTagInfo;
    }

    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", N_("Interoperability Index"),
                N_("Indicates the identification of the Interoperability rule. "
                "Use \"R98\" for stating ExifR98 Rules. Four bytes used "
                "including the termination code (NULL). see the separate "
                "volume of Recommended Exif Interoperability Rules (ExifR98) "
                "for other tags used for ExifR98."),
                iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x0002, "InteroperabilityVersion", N_("Interoperability Version"),
                N_("Interoperability version"),
                iopIfdId, iopTags, undefined, printValue),
        TagInfo(0x1000, "RelatedImageFileFormat", N_("Related Image File Format"),
                N_("File format of image file"),
                iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x1001, "RelatedImageWidth", N_("Related Image Width"),
                N_("Image width"),
                iopIfdId, iopTags, unsignedLong, printValue),
        TagInfo(0x1002, "RelatedImageLength", N_("Related Image Length"),
                N_("Image height"),
                iopIfdId, iopTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", N_("Unknown Exif Interoperability tag"),
                N_("Unknown Exif Interoperability tag"),
                ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    const TagInfo* ExifTags::iopTagList()
    {
        return iopTagInfo;
    }

    // Unknown Tag
    static const TagInfo unknownTag(0xffff, "Unknown tag", N_("Unknown tag"),
                                    N_("Unknown tag"),
                                    ifdIdNotSet, sectionIdNotSet, asciiString, printValue);

    // Tag lookup lists with tag names, desc and where they (preferably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0,
        ifdTagInfo, exifTagInfo, gpsTagInfo, iopTagInfo, ifdTagInfo,
        0
    };

    // Lookup list for registered makernote tag info tables
    const TagInfo* ExifTags::makerTagInfos_[];

    // All makernote ifd ids, in the same order as the tag infos in makerTagInfos_
    IfdId ExifTags::makerIfdIds_[];

    void ExifTags::registerBaseTagInfo(IfdId ifdId)
    {
        registerMakerTagInfo(ifdId, ifdTagInfo);
    }

    void ExifTags::registerMakerTagInfo(IfdId ifdId, const TagInfo* tagInfo)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS; ++i) {
            if (makerIfdIds_[i] == 0) {
                makerIfdIds_[i] = ifdId;
                makerTagInfos_[i] = tagInfo;
                break;
            }
        }
        if (i == MAX_MAKER_TAG_INFOS) throw Error(16);
    } // ExifTags::registerMakerTagInfo

    int ExifTags::tagInfoIdx(uint16_t tag, IfdId ifdId)
    {
        const TagInfo* tagInfo = tagInfos_[ifdId];
        if (tagInfo == 0) return -1;
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].tag_ == tag) return idx;
        }
        return -1;
    } // ExifTags::tagInfoIdx

    const TagInfo* ExifTags::makerTagInfo(uint16_t tag, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].tag_ == tag) return &makerTagInfos_[i][k];
        }

        return 0;
    } // ExifTags::makerTagInfo

    const TagInfo* ExifTags::makerTagInfo(const std::string& tagName,
                                          IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].name_ == tagName) {
                return &makerTagInfos_[i][k];
            }
        }

        return 0;
    } // ExifTags::makerTagInfo

    bool ExifTags::isMakerIfd(IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        return i != MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != IfdId(0);
    }

    std::string ExifTags::tagName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->name_;
        }
        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << tag;
        return os.str();
    } // ExifTags::tagName

    const char* ExifTags::tagTitle(uint16_t tag, IfdId ifdId)
    {
        return tagLabel(tag, ifdId);
    } // ExifTags::tagTitle

    const char* ExifTags::tagLabel(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return _(unknownTag.title_);
            return _(tagInfos_[ifdId][idx].title_);
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return _(tagInfo->title_);
        }
        return "";
    } // ExifTags::tagLabel

    const char* ExifTags::tagDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return _(unknownTag.desc_);
            return _(tagInfos_[ifdId][idx].desc_);
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return _(tagInfo->desc_);
        }
        return "";
    } // ExifTags::tagDesc

    const char* ExifTags::sectionName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return sectionInfo_[unknownTag.sectionId_].name_;
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return sectionInfo_[tagInfo[idx].sectionId_].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return sectionInfo_[tagInfo->sectionId_].name_;
        }
        return "";
    } // ExifTags::sectionName

    const char* ExifTags::sectionDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return _(sectionInfo_[unknownTag.sectionId_].desc_);
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return _(sectionInfo_[tagInfo[idx].sectionId_].desc_);
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return _(sectionInfo_[tagInfo->sectionId_].desc_);
        }
        return "";
    } // ExifTags::sectionDesc

    uint16_t ExifTags::tag(const std::string& tagName, IfdId ifdId)
    {
        uint16_t tag = 0xffff;
        if (isExifIfd(ifdId)) {
            const TagInfo* tagInfo = tagInfos_[ifdId];
            if (tagInfo) {
                int idx;
                for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
                    if (tagInfo[idx].name_ == tagName) break;
                }
                tag = tagInfo[idx].tag_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tagName, ifdId);
            if (tagInfo != 0) tag = tagInfo->tag_;
        }
        if (tag == 0xffff) {
            if (!isHex(tagName, 4, "0x")) throw Error(7, tagName, ifdId);
            std::istringstream is(tagName);
            is >> std::hex >> tag;
        }
        return tag;
    } // ExifTags::tag

    IfdId ExifTags::ifdIdByIfdItem(const std::string& ifdItem)
    {
        int i;
        for (i = int(lastIfdId) - 1; i > 0; --i) {
            if (ifdInfo_[i].item_ == ifdItem) break;
        }
        return IfdId(i);
    }

    const char* ExifTags::ifdName(IfdId ifdId)
    {
        return ifdInfo_[ifdId].name_;
    }

    const char* ExifTags::ifdItem(IfdId ifdId)
    {
        return ifdInfo_[ifdId].item_;
    }

    const char* ExifTags::sectionName(SectionId sectionId)
    {
        return sectionInfo_[sectionId].name_;
    }

    SectionId ExifTags::sectionId(const std::string& sectionName)
    {
        int i;
        for (i = int(lastSectionId) - 1; i > 0; --i) {
            if (sectionInfo_[i].name_ == sectionName) break;
        }
        return SectionId(i);
    }

    TypeId ExifTags::tagType(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].typeId_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->typeId_;
        }
        return unknownTag.typeId_;
    }

    std::ostream& ExifTags::printTag(std::ostream& os,
                                     uint16_t tag,
                                     IfdId ifdId,
                                     const Value& value)
    {
        if (value.count() == 0) return os;
        PrintFct fct = printValue;
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) {
                fct = tagInfos_[ifdId][idx].printFct_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) fct = tagInfo->printFct_;
        }
        return fct(os, value);
    } // ExifTags::printTag

    void ExifTags::taglist(std::ostream& os)
    {
        for (int i=0; ifdTagInfo[i].tag_ != 0xffff; ++i) {
            os << ifdTagInfo[i] << "\n";
        }
        for (int i=0; exifTagInfo[i].tag_ != 0xffff; ++i) {
            os << exifTagInfo[i] << "\n";
        }
        for (int i=0; iopTagInfo[i].tag_ != 0xffff; ++i) {
            os << iopTagInfo[i] << "\n";
        }
        for (int i=0; gpsTagInfo[i].tag_ != 0xffff; ++i) {
            os << gpsTagInfo[i] << "\n";
        }
    } // ExifTags::taglist

    void ExifTags::makerTaglist(std::ostream& os, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i != MAX_MAKER_TAG_INFOS) {
            const TagInfo* mnTagInfo = makerTagInfos_[i];
            for (int k=0; mnTagInfo[k].tag_ != 0xffff; ++k) {
                os << mnTagInfo[k] << "\n";
            }
        }
    } // ExifTags::makerTaglist

    const char* ExifKey::familyName_ = "Exif";

    ExifKey::ExifKey(const std::string& key)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_(key)
    {
        decomposeKey();
    }

    ExifKey::ExifKey(uint16_t tag, const std::string& ifdItem)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_("")
    {
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(23, ifdId);
        }
        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        makeKey();
    }

    ExifKey::ExifKey(const Entry& e)
        : tag_(e.tag()), ifdId_(e.ifdId()),
          ifdItem_(ExifTags::ifdItem(e.ifdId())),
          idx_(e.idx()), key_("")
    {
        makeKey();
    }

    ExifKey::ExifKey(const ExifKey& rhs)
        : Key(rhs), tag_(rhs.tag_), ifdId_(rhs.ifdId_), ifdItem_(rhs.ifdItem_),
          idx_(rhs.idx_), key_(rhs.key_)
    {
    }

    ExifKey::~ExifKey()
    {
    }

    ExifKey& ExifKey::operator=(const ExifKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        tag_ = rhs.tag_;
        ifdId_ = rhs.ifdId_;
        ifdItem_ = rhs.ifdItem_;
        idx_ = rhs.idx_;
        key_ = rhs.key_;
        return *this;
    }

    std::string ExifKey::tagName() const
    {
        return ExifTags::tagName(tag_, ifdId_);
    }

    std::string ExifKey::tagLabel() const 	
    {
        return ExifTags::tagLabel(tag_, ifdId_);
    }

    ExifKey::AutoPtr ExifKey::clone() const
    {
        return AutoPtr(clone_());
    }

    ExifKey* ExifKey::clone_() const
    {
        return new ExifKey(*this);
    }

    std::string ExifKey::sectionName() const
    {
        return ExifTags::sectionName(tag(), ifdId());
    }

    void ExifKey::decomposeKey()
    {
        // Get the family name, IFD name and tag name parts of the key
        std::string::size_type pos1 = key_.find('.');
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string familyName = key_.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key_);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key_.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string ifdItem = key_.substr(pos0, pos1 - pos0);
        if (ifdItem == "") throw Error(6, key_);
        std::string tagName = key_.substr(pos1 + 1);
        if (tagName == "") throw Error(6, key_);

        // Find IfdId
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ifdId == ifdIdNotSet) throw Error(6, key_);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(6, key_);
        }
        // Convert tag
        uint16_t tag = ExifTags::tag(tagName, ifdId);

        // Translate hex tag name (0xabcd) to a real tag name if there is one
        tagName = ExifTags::tagName(tag, ifdId);

        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        key_ = familyName + "." + ifdItem + "." + tagName;
    }

    void ExifKey::makeKey()
    {
        key_ =   std::string(familyName_)
               + "." + ifdItem_
               + "." + ExifTags::tagName(tag_, ifdId_);
    }

    // *************************************************************************
    // free functions

    bool isExifIfd(IfdId ifdId)
    {
        bool rc;
        switch (ifdId) {
        case ifd0Id:    rc = true; break;
        case exifIfdId: rc = true; break;
        case gpsIfdId:  rc = true; break;
        case iopIfdId:  rc = true; break;
        case ifd1Id:    rc = true; break;
        default:        rc = false; break;
        }
        return rc;
    } // isExifIfd

    std::ostream& operator<<(std::ostream& os, const TagInfo& ti)
    {
        ExifKey exifKey(ti.tag_, ExifTags::ifdItem(ti.ifdId_));
        return os << ExifTags::tagName(ti.tag_, ti.ifdId_) << ",\t"
                  << std::dec << ti.tag_ << ",\t"
                  << "0x" << std::setw(4) << std::setfill('0')
                  << std::right << std::hex << ti.tag_ << ",\t"
                  << ExifTags::ifdName(ti.ifdId_) << ",\t"
                  << exifKey.key() << ",\t"
                  << TypeInfo::typeName(
                      ExifTags::tagType(ti.tag_, ti.ifdId_)) << ",\t"
                  << ExifTags::tagDesc(ti.tag_, ti.ifdId_);
    }

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
    {
        int32_t nominator;
        int32_t denominator;
        char c;
        is >> nominator >> c >> denominator;
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& operator<<(std::ostream& os, const URational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, URational& r)
    {
        uint32_t nominator;
        uint32_t denominator;
        char c;
        is >> nominator >> c >> denominator;
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& printValue(std::ostream& os, const Value& value)
    {
        return os << value;
    }

    std::ostream& printLong(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<long>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printLong

    std::ostream& printFloat(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<float>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printFloat

    std::ostream& printDegrees(std::ostream& os, const Value& value)
    {
        if (value.count() == 3) {
            std::ostringstream oss;
            oss.copyfmt(os);
            static const char* unit[] = { "deg", "'", "\"" };
            static const int prec[] = { 7, 5, 3 };
            int n;
            for (n = 2; n > 0; --n) {
                if (value.toRational(n).first != 0) break;
            }
            for (int i = 0; i < n + 1; ++i) {
                const int32_t z = value.toRational(i).first;
                const int32_t d = value.toRational(i).second;
                // Hack: Need Value::toDouble
                double b = static_cast<double>(z)/d;
                const int p = z % d == 0 ? 0 : prec[i];
                os << std::fixed << std::setprecision(p) << b
                   << unit[i] << " ";
            }
            os.copyfmt(oss);
        }
        else {
            os << value;
        }

        return os;
    } // printDegrees

    std::ostream& printUcs2(std::ostream& os, const Value& value)
    {
#ifdef EXV_HAVE_ICONV
        bool go = true;
        iconv_t cd = (iconv_t)(-1);
        if (value.typeId() != unsignedByte) {
            go = false;
        }
        if (go) {
            cd = iconv_open("UTF-8", "UCS-2LE");
            if (cd == (iconv_t)(-1)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: iconv_open: " << strError() << "\n";
#endif
                go = false;
            }
        }
        if (go) {
            DataBuf ib(value.size());
            value.copy(ib.pData_, invalidByteOrder);
            DataBuf ob(value.size());
            char* outptr = reinterpret_cast<char*>(ob.pData_);
            const char* outbuf = outptr;
            size_t outbytesleft = ob.size_;
            EXV_ICONV_CONST char* inbuf
                = reinterpret_cast<EXV_ICONV_CONST char*>(ib.pData_);
            size_t inbytesleft = ib.size_;
            size_t rc = iconv(cd,
                              &inbuf,
                              &inbytesleft,
                              &outptr,
                              &outbytesleft);
            if (rc == size_t(-1)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: iconv: "
                          << strError()
                          << " inbytesleft = " << inbytesleft << "\n";
#endif
                go = false;
            }
            if (go) {
                // Todo: What if outbytesleft == 0
                os << std::string(outbuf, outptr-outbuf);
            }
        }
        if (cd != (iconv_t)(-1)) {
            iconv_close(cd);
        }
        if (!go) {
            os << value;
        }
#else // !EXV_HAVE_ICONV
        os << value;
#endif // EXV_HAVE_ICONV
        return os;

    } // printUcs2

    std::ostream& print0x0006(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        const int32_t d = value.toRational().second;
        const int p = d > 1 ? 1 : 0;
        os << std::fixed << std::setprecision(p) << value.toFloat() << " m";
        os.copyfmt(oss);

        return os;
    }

    std::ostream& print0x0007(std::ostream& os, const Value& value)
    {
        if (value.count() == 3) {
            std::ostringstream oss;
            oss.copyfmt(os);
            const float sec = 3600 * value.toFloat(0)
                              + 60 * value.toFloat(1)
                              + value.toFloat(2);
            int p = 0;
            if (sec != static_cast<int>(sec)) p = 1;

            const int hh = static_cast<int>(sec / 3600);
            const int mm = static_cast<int>((sec - 3600 * hh) / 60);
            const float ss = sec - 3600 * hh - 60 * mm;

            os << std::setw(2) << std::setfill('0') << std::right << hh << ":"
               << std::setw(2) << std::setfill('0') << std::right << mm << ":"
               << std::setw(2 + p * 2) << std::setfill('0') << std::right
               << std::fixed << std::setprecision(p) << ss;

            os.copyfmt(oss);
        }
        else {
            os << value;
        }

        return os;
    }

    std::ostream& print0x8298(std::ostream& os, const Value& value)
    {
        // Print the copyright information in the format Photographer, Editor
        std::string val = value.toString();
        std::string::size_type pos = val.find('\0');
        if (pos != std::string::npos) {
            std::string photographer(val, 0, pos);
            if (photographer != " ") os << photographer;
            std::string editor(val, pos + 1);
            if (editor != "") {
                if (photographer != " ") os << ", ";
                os << editor;
            }
        }
        else {
            os << val;
        }
        return os;
    }

    std::ostream& print0x829a(std::ostream& os, const Value& value)
    {
        Rational t = value.toRational();
        if (t.first > 1 && t.second > 1 && t.second >= t.first) {
            t.second = static_cast<uint32_t>(
                static_cast<float>(t.second) / t.first + 0.5);
            t.first = 1;
        }
        if (t.second > 1 && t.second < t.first) {
            t.first = static_cast<uint32_t>(
                static_cast<float>(t.first) / t.second + 0.5);
            t.second = 1;
        }
        if (t.second == 1) {
            os << t.first << " s";
        }
        else {
            os << t.first << "/" << t.second << " s";
        }
        return os;
    }

    std::ostream& print0x829d(std::ostream& os, const Value& value)
    {
        Rational fnumber = value.toRational();
        if (fnumber.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fnumber.first) / fnumber.second;
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& print0x8827(std::ostream& os, const Value& value)
    {
        return os << value.toLong();
    }

    std::ostream& print0x9101(std::ostream& os, const Value& value)
    {
        for (long i = 0; i < value.count(); ++i) {
            long l = value.toLong(i);
            switch (l) {
            case 0:  break;
            case 1:  os << "Y"; break;
            case 2:  os << "Cb"; break;
            case 3:  os << "Cr"; break;
            case 4:  os << "R"; break;
            case 5:  os << "G"; break;
            case 6:  os << "B"; break;
            default: os << "(" << l << ")"; break;
            }
        }
        return os;
    }

    std::ostream& print0x9201(std::ostream& os, const Value& value)
    {
        URational ur = exposureTime(value.toFloat());
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

    std::ostream& print0x9202(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << "F" << std::setprecision(2) << fnumber(value.toFloat());
        os.copyfmt(oss);

        return os;
    }

    std::ostream& print0x9204(std::ostream& os, const Value& value)
    {
        Rational bias = value.toRational();
        if (bias.second <= 0) {
            os << "(" << bias.first << "/" << bias.second << ")";
        }
        else if (bias.first == 0) {
            os << "0";
        }
        else {
            int32_t d = gcd(bias.first, bias.second);
            int32_t num = std::abs(bias.first) / d;
            int32_t den = bias.second / d;
            os << (bias.first < 0 ? "-" : "+") << num;
            if (den != 1) {
                os << "/" << den;
            }
        }
        return os;
    }

    std::ostream& print0x9206(std::ostream& os, const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
        }
        else if (static_cast<uint32_t>(distance.first) == 0xffffffff) {
            os << _("Infinity");
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& print0x920a(std::ostream& os, const Value& value)
    {
        Rational length = value.toRational();
        if (length.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)length.first / length.second
               << " mm";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    // Todo: Implement this properly
    std::ostream& print0x9286(std::ostream& os, const Value& value)
    {
        if (value.size() > 8) {
            DataBuf buf(value.size());
            value.copy(buf.pData_, bigEndian);
            // Hack: Skip the leading 8-Byte character code, truncate
            // trailing '\0's and let the stream take care of the remainder
            std::string userComment(reinterpret_cast<char*>(buf.pData_) + 8, buf.size_ - 8);
            std::string::size_type pos = userComment.find_last_not_of('\0');
            os << userComment.substr(0, pos + 1);
        }
        return os;
    }

    std::ostream& print0xa404(std::ostream& os, const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.second == 0) {
            os << _("Digital zoom not used");
        }
        else {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& print0xa405(std::ostream& os, const Value& value)
    {
        long length = value.toLong();
        if (length == 0) {
            os << _("Unknown");
        }
        else {
            os << length << ".0 mm";
        }
        return os;
    }

    float fnumber(float apertureValue)
    {
        return static_cast<float>(std::exp(std::log(2.0) * apertureValue / 2));
    }

    URational exposureTime(float shutterSpeedValue)
    {
        URational ur(1, 1);
        double tmp = std::exp(std::log(2.0) * shutterSpeedValue);
        if (tmp > 1) {
            ur.second = static_cast<long>(tmp + 0.5);
        }
        else {
            ur.first = static_cast<long>(1/tmp + 0.5);
        }
        return ur;
    }

}                                       // namespace Exiv2
