// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      properties.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "properties.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"
#include "i18n.h"                // NLS support.
#include "xmp.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    extern const XmpPropertyInfo xmpDcInfo[];
    extern const XmpPropertyInfo xmpXmpInfo[];
    extern const XmpPropertyInfo xmpXmpRightsInfo[];
    extern const XmpPropertyInfo xmpXmpMMInfo[];
    extern const XmpPropertyInfo xmpXmpBJInfo[];
    extern const XmpPropertyInfo xmpXmpTPgInfo[];
    extern const XmpPropertyInfo xmpXmpDMInfo[];
    extern const XmpPropertyInfo xmpPdfInfo[];
    extern const XmpPropertyInfo xmpPhotoshopInfo[];
    extern const XmpPropertyInfo xmpCrsInfo[];
    extern const XmpPropertyInfo xmpTiffInfo[];
    extern const XmpPropertyInfo xmpExifInfo[];
    extern const XmpPropertyInfo xmpAuxInfo[];
    extern const XmpPropertyInfo xmpIptcInfo[];

    extern const XmpNsInfo xmpNsInfo[] = {
        // Schemas
        { "http://purl.org/dc/elements/1.1/",             "dc",           xmpDcInfo,        N_("Dublin Core schema")                        },
        { "http://ns.adobe.com/xap/1.0/",                 "xmp",          xmpXmpInfo,       N_("XMP Basic schema")                          },
        { "http://ns.adobe.com/xap/1.0/rights/",          "xmpRights",    xmpXmpRightsInfo, N_("XMP Rights Management schema")              },
        { "http://ns.adobe.com/xap/1.0/mm/",              "xmpMM",        xmpXmpMMInfo,     N_("XMP Media Management schema")               },
        { "http://ns.adobe.com/xap/1.0/bj/",              "xmpBJ",        xmpXmpBJInfo,     N_("XMP Basic Job Ticket schema")               },
        { "http://ns.adobe.com/xap/1.0/t/pg/",            "xmpTPg",       xmpXmpTPgInfo,    N_("XMP Paged-Text schema")                     },
        { "http://ns.adobe.com/xmp/1.0/DynamicMedia/",    "xmpDM",        xmpXmpDMInfo,     N_("XMP Dynamic Media schema")                  },
        { "http://ns.adobe.com/pdf/1.3/",                 "pdf",          xmpPdfInfo,       N_("Adobe PDF schema")                          },
        { "http://ns.adobe.com/photoshop/1.0/",           "photoshop",    xmpPhotoshopInfo, N_("Adobe photoshop schema")                    },
        { "http://ns.adobe.com/camera-raw-settings/1.0/", "crs",          xmpCrsInfo,       N_("Camera Raw schema")                         },
        { "http://ns.adobe.com/tiff/1.0/",                "tiff",         xmpTiffInfo,      N_("Exif Schema for TIFF Properties")           },
        { "http://ns.adobe.com/exif/1.0/",                "exif",         xmpExifInfo,      N_("Exif schema for Exif-specific Properties")  },
        { "http://ns.adobe.com/exif/1.0/aux/",            "aux",          xmpAuxInfo,       N_("Exif schema for Additional Exif Properties")},
        { "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/",  "iptc",         xmpIptcInfo,      N_("IPTC Core schema")                          }, 
                                                                                             // NOTE: 'Iptc4xmpCore' is just too long

        // Structures
        { "http://ns.adobe.com/xap/1.0/g/",                   "xapG",    0, N_("Colorant structure")           },
        { "http://ns.adobe.com/xap/1.0/sType/Dimensions#",    "stDim",   0, N_("Dimensions structure")         },
        { "http://ns.adobe.com/xap/1.0/sType/Font#",          "stFnt",   0, N_("Font structure")               },
        { "http://ns.adobe.com/xap/1.0/g/img/",               "xapGImg", 0, N_("Thumbnail structure")          },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#", "stEvt",   0, N_("Resource Event structure")     },
        { "http://ns.adobe.com/xap/1.0/sType/ResourceRef#",   "stRef",   0, N_("ResourceRef structure")        },
        { "http://ns.adobe.com/xap/1.0/sType/Version#",       "stVer",   0, N_("Version structure")            },
        { "http://ns.adobe.com/xap/1.0/sType/Job#",           "stJob",   0, N_("Basic Job/Workflow structure") },

        // Qualifiers
        { "http://ns.adobe.com/xmp/Identifier/qual/1.0/", "xmpidq", 0, N_("Qualifier for xmp:Identifier") }
    };

    extern const XmpPropertyInfo xmpDcInfo[] = {
        { "contributor",      N_("Contributor"),      "bag ProperName",  xmpBag,       xmpExternal, N_("Contributors to the resource (other than the authors).")                               },
        { "coverage",         N_("Coverage"),         "Text",            xmpText,      xmpExternal, N_("The extent or scope of the resource.")                                                 },
        { "creator",          N_("Creator"),          "seq ProperName",  xmpSeq,       xmpExternal, N_("The authors of the resource (listed in order of precedence, if significant).")         },
        { "date",             N_("Date"),             "seq Date",        xmpSeq,       xmpExternal, N_("Date(s) that something interesting happened to the resource.")                         },
        { "description",      N_("Description"),      "Lang Alt",        langAlt,      xmpExternal, N_("A textual description of the content of the resource. Multiple values may be "
                                                                                                       "present for different languages.")                                                     },
        { "format",           N_("Format"),           "MIMEType",        xmpText,      xmpInternal, N_("The file format used when saving the resource. Tools and applications should set "
                                                                                                       "this property to the save format of the data. It may include appropriate qualifiers.") },
        { "identifier",       N_("Identifier"),       "Text",            xmpText,      xmpExternal, N_("Unique identifier of the resource.")                                                   },
        { "language",         N_("Language"),         "bag Locale",      xmpBag,       xmpInternal, N_("An unordered array specifying the languages used in the resource.")                    },
        { "publisher",        N_("Publisher"),        "bag ProperName",  xmpBag,       xmpExternal, N_("Publishers.")                                                                          },
        { "relation",         N_("Relation"),         "bag Text",        xmpBag,       xmpInternal, N_("Relationships to other documents.")                                                    },
        { "rights",           N_("Rights"),           "Lang Alt",        langAlt,      xmpExternal, N_("Informal rights statement, selected by language.")                                     },
        { "source",           N_("Source"),           "Text",            xmpText,      xmpExternal, N_("Unique identifier of the work from which this resource was derived.")                  },
        { "subject",          N_("Subject"),          "bag Text",        xmpBag,       xmpExternal, N_("An unordered array of descriptive phrases or keywords that specify the topic of the "
                                                                                                       "content of the resource.")                                                             },
        { "title",            N_("Title"),            "Lang Alt",        langAlt,      xmpExternal, N_("The title of the document, or the name given to the resource. Typically, it will be "
                                                                                                       "a name by which the resource is formally known.")                                      },
        { "type",             N_("Type"),             "bag open Choice", xmpBag,       xmpExternal, N_("A document type; for example, novel, poem, or working paper.")                         },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpInfo[] = {
        { "Advisory",         N_("Advisory"),         "bag XPath",                xmpBag,    xmpExternal, N_("An unordered array specifying properties that were edited outside the authoring "
                                                                                                             "application. Each item should contain a single namespace and XPath separated by "
                                                                                                             "one ASCII space (U+0020).") },
        { "BaseURL",          N_("Base URL"),         "URL",                      xmpText,   xmpInternal, N_("The base URL for relative URLs in the document content. If this document contains "
                                                                                                             "Internet links, and those links are relative, they are relative to this base URL. "
                                                                                                             "This property provides a standard way for embedded relative URLs to be interpreted "
                                                                                                             "by tools. Web authoring tools should set the value based on their notion of where "
                                                                                                             "URLs will be interpreted.") },
        { "CreateDate",       N_("Create Date"),      "Date",                     xmpText,   xmpInternal, N_("The date and time the resource was originally created.") },
        { "CreatorTool",      N_("Creator Tool"),     "AgentName",                xmpText,   xmpInternal, N_("The name of the first known tool used to create the resource. If history is "
                                                                                                             "present in the metadata, this value should be equivalent to that of "
                                                                                                             "xmpMM:History's softwareAgent property.") },
        { "Identifier",       N_("Identifier"),       "bag Text",                 xmpBag,    xmpExternal, N_("An unordered array of text strings that unambiguously identify the resource within "
                                                                                                             "a given context. An array item may be qualified with xmpidq:Scheme to denote the "
                                                                                                             "formal identification system to which that identifier conforms. Note: The "
                                                                                                             "dc:identifier property is not used because it lacks a defined scheme qualifier and "
                                                                                                             "has been defined in the XMP Specification as a simple (single-valued) property.") },
        { "Label",            N_("Label"),            "Text",                     xmpText,   xmpExternal, N_("A word or short phrase that identifies a document as a member of a user-defined "
                                                                                                             "collection. Used to organize documents in a file browser.") },
        { "MetadataDate",     N_("Metadata Date"),    "Date",                     xmpText,   xmpInternal, N_("The date and time that any metadata for this resource was last changed. It should "
                                                                                                             "be the same as or more recent than xmp:ModifyDate.") },
        { "ModifyDate",       N_("Modify Date"),      "Date",                     xmpText,   xmpInternal, N_("The date and time the resource was last modified. Note: The value of this property "
                                                                                                             "is not necessarily the same as the file's system modification date because it is "
                                                                                                             "set before the file is saved.") },
        { "Nickname",         N_("Nickname"),         "Text",                     xmpText,   xmpExternal, N_("A short informal name for the resource.") },
        { "Rating",           N_("Rating"),           "Closed Choice of Integer", xmpText,   xmpExternal, N_("A number that indicates a document's status relative to other documents, "
                                                                                                             "used to organize documents in a file browser. Values are user-defined within an "
                                                                                                             "application-defined range.") },
        { "Thumbnails",       N_("Thumbnails"),       "alt Thumbnail",            undefined, xmpInternal, N_("An alternative array of thumbnail images for a file, which can differ in "
                                                                                                             "characteristics such as size or image encoding.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpRightsInfo[] = {
        { "Certificate",      N_("Certificate"),   "URL",            xmpText,       xmpExternal, N_("Online rights management certificate.") },
        { "Marked",           N_("Marked"),        "Boolean",        xmpText,       xmpExternal, N_("Indicates that this is a rights-managed resource.") },
        { "Owner",            N_("Owner"),         "bag ProperName", xmpBag,        xmpExternal, N_("An unordered array specifying the legal owner(s) of a resource.") },
        { "UsageTerms",       N_("Usage Terms"),   "Lang Alt",       langAlt,       xmpExternal, N_("Text instructions on how a resource can be legally used.") },
        { "WebStatement",     N_("Web Statement"), "URL",            xmpText,       xmpExternal, N_("The location of a web page describing the owner and/or rights statement for this resource.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpMMInfo[] = {
        { "DerivedFrom",      N_("Derived From"),      "ResourceRef",       xmpText,    xmpInternal, N_("A reference to the original document from which this one is derived. It is a "
                                                                                                        "minimal reference; missing components can be assumed to be unchanged. For example, "
                                                                                                        "a new version might only need to specify the instance ID and version number of the "
                                                                                                        "previous version, or a rendition might only need to specify the instance ID and "
                                                                                                        "rendition class of the original.") },
        { "DocumentID",       N_("Document ID"),       "URI",               xmpText,    xmpInternal, N_("The common identifier for all versions and renditions of a document. It should be "
                                                                                                        "based on a UUID; see Document and Instance IDs below.") },
        { "History",          N_("History"),           "seq ResourceEvent", xmpText,    xmpInternal, N_("An ordered array of high-level user actions that resulted in this resource. It is "
                                                                                                        "intended to give human readers a general indication of the steps taken to make the "
                                                                                                        "changes from the previous version to this one. The list should be at an abstract "
                                                                                                        "level; it is not intended to be an exhaustive keystroke or other detailed history.") },
        { "InstanceID",       N_("Instance ID"),       "URI",               xmpText,    xmpInternal, N_("An identifier for a specific incarnation of a document, updated each time a file "
                                                                                                        "is saved. It should be based on a UUID; see Document and Instance IDs below.") },
        { "ManagedFrom",      N_("Managed From"),      "ResourceRef",       xmpText,    xmpInternal, N_("A reference to the document as it was prior to becoming managed. It is set when a "
                                                                                                        "managed document is introduced to an asset management system that does not "
                                                                                                        "currently own it. It may or may not include references to different management systems.") },
        { "Manager",          N_("Manager"),           "AgentName",         xmpText,    xmpInternal, N_("The name of the asset management system that manages this resource. Along with "
                                                                                                        "xmpMM: ManagerVariant, it tells applications which asset management system to "
                                                                                                        "contact concerning this document.") },
        { "ManageTo",         N_("Manage To"),         "URI",               xmpText,    xmpInternal, N_("A URI identifying the managed resource to the asset management system; the presence "
                                                                                                        "of this property is the formal indication that this resource is managed. The form "
                                                                                                        "and content of this URI is private to the asset management system.") },
        { "ManageUI",         N_("Manage UI"),         "URI",               xmpText,    xmpInternal, N_("A URI that can be used to access information about the managed resource through a "
                                                                                                        "web browser. It might require a custom browser plug- in.") },
        { "ManagerVariant",   N_("Manager Variant"),   "Text",              xmpText,    xmpInternal, N_("Specifies a particular variant of the asset management system. The format of this "
                                                                                                        "property is private to the specific asset management system.") },
        { "RenditionClass",   N_("Rendition Class"),   "RenditionClass",    xmpText,    xmpInternal, N_("The rendition class name for this resource. This property should be absent or set "
                                                                                                        "to default for a document version that is not a derived rendition.") },
        { "RenditionParams",  N_("Rendition Params"),  "Text",              xmpText,    xmpInternal, N_("Can be used to provide additional rendition parameters that are too complex or "
                                                                                                        "verbose to encode in xmpMM: RenditionClass.") },
        { "VersionID",        N_("Version ID"),        "Text",              xmpText,    xmpInternal, N_("The document version identifier for this resource. Each version of a document gets "
                                                                                                        "a new identifier, usually simply by incrementing integers 1, 2, 3 . . . and so on. "
                                                                                                        "Media management systems can have other conventions or support branching which "
                                                                                                        "requires a more complex scheme.") },
        { "Versions",         N_("Versions"),          "seq Version",       xmpText,    xmpInternal, N_("The version history associated with this resource. Entry [1] is the oldest known "
                                                                                                        "version for this document, entry [last()] is the most recent version. Typically, a "
                                                                                                        "media management system would fill in the version information in the metadata on "
                                                                                                        "check-in. It is not guaranteed that a complete history  versions from the first to "
                                                                                                        "this one will be present in the xmpMM:Versions property. Interior version information "
                                                                                                        "can be compressed or eliminated and the version history can be truncated at some point.") },
        { "LastURL",          N_("Last URL"),          "URL",               xmpText,    xmpInternal, N_("Deprecated for privacy protection.") },
        { "RenditionOf",      N_("Rendition Of"),      "ResourceRef",       xmpText,    xmpInternal, N_("Deprecated in favor of xmpMM:DerivedFrom. A reference to the document of which this is "
                                                                                                        "a rendition.") },
        { "SaveID",           N_("Save ID"),           "Integer",           xmpText,    xmpInternal, N_("Deprecated. Previously used only to support the xmpMM:LastURL property.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpBJInfo[] = {
        { "JobRef",           N_("Job Reference"),     "bag Job",   xmpText,    xmpExternal, N_("References an external job management file for a job process in which the document is being used. Use of job "
                                                                                                "names is under user control. Typical use would be to identify all documents that are part of a particular job or contract. "
                                                                                                "There are multiple values because there can be more than one job using a particular document at any time, and it can "
                                                                                                "also be useful to keep historical information about what jobs a document was part of previously.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpTPgInfo[] = {
        { "MaxPageSize",      N_("MaxPageSize"),  "Dimensions",   xmpText,    xmpInternal, N_("The size of the largest page in the document (including any in contained documents).") },
        { "NPages",           N_("NPages"),       "Integer",      xmpText,    xmpInternal, N_("The number of pages in the document (including any in contained documents).") },
        { "Fonts",            N_("Fonts"),        "bag Font",     xmpText,    xmpInternal, N_("An unordered array of fonts that are used in the document (including any in contained documents).") },
        { "Colorants",        N_("Colorants"),    "seq Colorant", xmpText,    xmpInternal, N_("An ordered array of colorants (swatches) that are used in the document (including any in contained documents).") },
        { "PlateNames",       N_("PlateNames"),   "seq Text",     xmpSeq,     xmpInternal, N_("An ordered array of plate names that are needed to print the document (including any in contained documents).") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpXmpDMInfo[] = {
        { "projectRef",                   N_("Project Ref"),                      "ProjectLink",           xmpText, xmpInternal, N_("A reference to the project that created this file.") },
        { "videoFrameRate",               N_("Video Frame Rate"),                 "open Choice of Text",   xmpText, xmpInternal, N_("The video frame rate. One of: 24, NTSC, PAL.") },
        { "videoFrameSize",               N_("Video Frame Size"),                 "Dimensions",            xmpText, xmpInternal, N_("The frame size. For example: w:720, h: 480, unit:pixels") },
        { "videoPixelAspectRatio",        N_("Video Pixel Aspect Ratio"),         "Rational",              xmpText, xmpInternal, N_("The aspect ratio, expressed as ht/wd. For example: \"648/720\" = 0.9") },
        { "videoPixelDepth",              N_("Video Pixel Depth"),                "closed Choice of Text", xmpText, xmpInternal, N_("The size in bits of each color component of a pixel. Standard Windows 32-bit "
                                                                                                                                    "pixels have 8 bits per component. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "videoColorSpace",              N_("Video Color Space"),                "closed Choice of Text", xmpText, xmpInternal, N_("The color space. One of: sRGB (used by Photoshop), CCIR-601 (used for NTSC), "
                                                                                                                                    "CCIR-709 (used for HD).") },
        { "videoAlphaMode",               N_("Video Alpha Mode"),                 "closed Choice of Text", xmpText, xmpExternal, N_("The alpha mode. One of: straight, pre-multiplied.") },
        { "videoAlphaPremultipleColor",   N_("Video Alpha Premultiple Color"),    "Colorant",              xmpText, xmpExternal, N_("A color in CMYK or RGB to be used as the pre-multiple color when "
                                                                                                                                    "alpha mode is pre-multiplied.") },
        { "videoAlphaUnityIsTransparent", N_("Video Alpha Unity Is Transparent"), "Boolean",               xmpText, xmpInternal, N_("When true, unity is clear, when false, it is opaque.") },
        { "videoCompressor",              N_("Video Compressor"),                 "Text",                  xmpText, xmpInternal, N_("Video compression used. For example, jpeg.") },
        { "videoFieldOrder",              N_("Video Field Order"),                "closed Choice of Text", xmpText, xmpInternal, N_("The field order for video. One of: Upper, Lower, Progressive.") },
        { "pullDown",                     N_("Pull Down"),                        "closed Choice of Text", xmpText, xmpInternal, N_("The sampling phase of film to be converted to video (pull-down). One of: "
                                                                                                                                    "WSSWW, SSWWW, SWWWS, WWWSS, WWSSW, WSSWW_24p, SSWWW_24p, SWWWS_24p, WWWSS_24p, WWSSW_24p.") },
        { "audioSampleRate",              N_("Audio Sample Rate"),                "Integer",               xmpText, xmpInternal, N_("The audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.") },
        { "audioSampleType",              N_("Audio Sample Type"),                "closed Choice of Text", xmpText, xmpInternal, N_("The audio sample type. One of: 8Int, 16Int, 32Int, 32Float.") },
        { "audioChannelType",             N_("Audio Channel Type"),               "closed Choice of Text", xmpText, xmpInternal, N_("The audio channel type. One of: Mono, Stereo, 5.1, 7.1.") },
        { "audioCompressor",              N_("Audio Compressor"),                 "Text",                  xmpText, xmpInternal, N_("The audio compression used. For example, MP3.") },
        { "speakerPlacement",             N_("Speaker Placement"),                "Text",                  xmpText, xmpExternal, N_("A description of the speaker angles from center front in degrees. For example: "
                                                                                                                                    "\"Left = -30, Right = 30, Center = 0, LFE = 45, Left Surround = -110, Right Surround = 110\"") },
        { "fileDataRate",                 N_("File Data Rate"),                   "Rational",              xmpText, xmpInternal, N_("The file data rate in megabytes per second. For example: \"36/10\" = 3.6 MB/sec") },
        { "tapeName",                     N_("Tape Name"),                        "Text",                  xmpText, xmpExternal, N_("The name of the tape from which the clip was captured, as set during the capture process.") },
        { "altTapeName",                  N_("Alt Tape Name"),                    "Text",                  xmpText, xmpExternal, N_("An alternative tape name, set via the project window or timecode dialog in Premiere. "
                                                                                                                                    "If an alternative name has been set and has not been reverted, that name is displayed.") },
        { "startTimecode",                N_("Start Time Code"),                  "Timecode",              xmpText, xmpInternal, N_("The timecode of the first frame of video in the file, as obtained from the device control.") },
        { "altTimecode",                  N_("Alt Time code"),                    "Timecode",              xmpText, xmpExternal, N_("A timecode set by the user. When specified, it is used instead of the startTimecode.") },
        { "duration",                     N_("Duration"),                         "Time",                  xmpText, xmpInternal, N_("The duration of the media file.") },
        { "scene",                        N_("scene"),                            "Text",                  xmpText, xmpExternal, N_("The name of the scene.") },
        { "shotName",                     N_("shotName"),                         "Text",                  xmpText, xmpExternal, N_("The name of the shot or take.") },
        { "shotDate",                     N_("shotDate"),                         "Date",                  xmpText, xmpExternal, N_("The date and time when the video was shot.") },
        { "shotLocation",                 N_("shotLocation"),                     "Text",                  xmpText, xmpExternal, N_("The name of the location where the video was shot. For example: \"Oktoberfest, Munich Germany\" "
                                                                                                                                    "For more accurate positioning, use the EXIF GPS values.") },
        { "logComment",                   N_("logComment"),                       "Text",                  xmpText, xmpExternal, N_("User's log comments.") },
        { "markers",                      N_("markers"),                          "seq Marker",            xmpText, xmpInternal, N_("An ordered list of markers") },
        { "contributedMedia",             N_("contributedMedia"),                 "bag Media",             xmpText, xmpInternal, N_("An unordered list of all media used to create this media.") },
        { "absPeakAudioFilePath",         N_("absPeakAudioFilePath"),             "URI",                   xmpText, xmpInternal, N_("The absolute path to the file's peak audio file. If empty, no peak file exists.") },
        { "relativePeakAudioFilePath",    N_("relativePeakAudioFilePath"),        "URI",                   xmpText, xmpInternal, N_("The relative path to the file's peak audio file. If empty, no peak file exists.") },
        { "videoModDate",                 N_("videoModDate"),                     "Date",                  xmpText, xmpInternal, N_("The date and time when the video was last modified.") },
        { "audioModDate",                 N_("audioModDate"),                     "Date",                  xmpText, xmpInternal, N_("The date and time when the audio was last modified.") },
        { "metadataModDate",              N_("metadataModDate"),                  "Date",                  xmpText, xmpInternal, N_("The date and time when the metadata was last modified.") },
        { "artist",                       N_("artist"),                           "Text",                  xmpText, xmpExternal, N_("The name of the artist or artists.") },
        { "album",                        N_("album"),                            "Text",                  xmpText, xmpExternal, N_("The name of the album.") },
        { "trackNumber",                  N_("trackNumber"),                      "Integer",               xmpText, xmpExternal, N_("A numeric value indicating the order of the audio file within its original recording.") },
        { "genre",                        N_("genre"),                            "Text",                  xmpText, xmpExternal, N_("The name of the genre.") },
        { "copyright",                    N_("copyright"),                        "Text",                  xmpText, xmpExternal, N_("The copyright information.") },
        { "releaseDate",                  N_("releaseDate"),                      "Date",                  xmpText, xmpExternal, N_("The date the title was released.") },
        { "composer",                     N_("composer"),                         "Text",                  xmpText, xmpExternal, N_("The composer's name.") },
        { "engineer",                     N_("engineer"),                         "Text",                  xmpText, xmpExternal, N_("The engineer's name.") },
        { "tempo",                        N_("tempo"),                            "Real",                  xmpText, xmpInternal, N_("The audio's tempo.") },
        { "instrument",                   N_("instrument"),                       "Text",                  xmpText, xmpExternal, N_("The musical instrument.") },
        { "introTime",                    N_("introTime"),                        "Time",                  xmpText, xmpInternal, N_("The duration of lead time for queuing music.") },
        { "outCue",                       N_("outCue"),                           "Time",                  xmpText, xmpInternal, N_("The time at which to fade out.") },
        { "relativeTimestamp",            N_("relativeTimestamp"),                "Time",                  xmpText, xmpInternal, N_("The start time of the media inside the audio project.") },
        { "loop",                         N_("loop"),                             "Boolean",               xmpText, xmpInternal, N_("When true, the clip can be looped seemlessly.") },
        { "numberOfBeats",                N_("numberOfBeats"),                    "Real",                  xmpText, xmpInternal, N_("The number of beats.") },
        { "key",                          N_("key"),                              "closed Choice of Text", xmpText, xmpInternal, N_("The audio's musical key. One of: C, C#, D, D#, E, F, F#, G, G#, A, A#, B.") },
        { "stretchMode",                  N_("stretchMode"),                      "closed Choice of Text", xmpText, xmpInternal, N_("The audio stretch mode. One of: Fixed length, Time-Scale, Resample, Beat Splice, Hybrid.") },
        { "timeScaleParams",              N_("timeScaleParams"),                  "timeScaleStretch",      xmpText, xmpInternal, N_("Additional parameters for Time-Scale stretch mode.") },
        { "resampleParams",               N_("resampleParams"),                   "resampleStretch",       xmpText, xmpInternal, N_("Additional parameters for Resample stretch mode.") },
        { "beatSpliceParams",             N_("beatSpliceParams"),                 "beatSpliceStretch",     xmpText, xmpInternal, N_("Additional parameters for Beat Splice stretch mode.") },
        { "timeSignature",                N_("timeSignature"),                    "closed Choice of Text", xmpText, xmpInternal, N_("The time signature of the music. One of: 2/4, 3/4, 4/4, 5/4, 7/4, 6/8, 9/8, 12/8, other.") },
        { "scaleType",                    N_("scaleType"),                        "closed Choice of Text", xmpText, xmpInternal, N_("The musical scale used in the music. One of: Major, Minor, Both, Neither. "
                                                                                                                                    "Neither is most often used for instruments with no associated scale, such as drums.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpPdfInfo[] = {
        { "Keywords",   N_("Keywords"),    "Text",      xmpText, xmpExternal, N_("Keywords.") },
        { "PDFVersion", N_("PDF Version"), "Text",      xmpText, xmpInternal, N_("The PDF file version (for example: 1.0, 1.3, and so on).") },
        { "Producer",   N_("Producer"),    "AgentName", xmpText, xmpInternal, N_("The name of the tool that created the PDF document.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpPhotoshopInfo[] = {
        { "AuthorsPosition",        N_("Authors Position"),        "Text",       xmpText, xmpExternal, N_("By-line title.") },
        { "CaptionWriter",          N_("Caption Writer"),          "ProperName", xmpText, xmpExternal, N_("Writer/editor.") },
        { "Category",               N_("Category"),                "Text",       xmpText, xmpExternal, N_("Category. Limited to 3 7-bit ASCII characters.") },
        { "City",                   N_("City"),                    "Text",       xmpText, xmpExternal, N_("City.") },
        { "Country",                N_("Country"),                 "Text",       xmpText, xmpExternal, N_("Country/primary location.") },
        { "Credit",                 N_("Credit"),                  "Text",       xmpText, xmpExternal, N_("Credit.") },
        { "DateCreated",            N_("Date Created"),            "Date",       xmpText, xmpExternal, N_("The date the intellectual content of the document was created (rather than the creation "
                                                                                                          "date of the physical representation), following IIM conventions. For example, a photo "
                                                                                                           "taken during the American Civil War would have a creation date during that epoch "
                                                                                                          "(1861-1865) rather than the date the photo was digitized for archiving.") },
        { "Headline",               N_("Headline"),                "Text",       xmpText, xmpExternal, N_("Headline.") },
        { "Instructions",           N_("Instructions"),            "Text",       xmpText, xmpExternal, N_("Special instructions.") },
        { "Source",                 N_("Source"),                  "Text",       xmpText, xmpExternal, N_("Source.") },
        { "State",                  N_("State"),                   "Text",       xmpText, xmpExternal, N_("Province/state.") },
        { "SupplementalCategories", N_("Supplemental Categories"), "bag Text",   xmpBag,  xmpExternal, N_("Supplemental category.") },
        { "TransmissionReference",  N_("Transmission Reference"),  "Text",       xmpText, xmpExternal, N_("Original transmission reference.") },
        { "Urgency",                N_("Urgency"),                 "Integer",    xmpText, xmpExternal, N_("Urgency. Valid range is 1-8.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! crs:CropUnits
    extern const TagDetails xmpCrsCropUnits[] = {
        { 0, N_("pixels") },
        { 1, N_("inches") },
        { 2, N_("cm")     }
    };

    extern const XmpPropertyInfo xmpCrsInfo[] = {
        { "AutoBrightness",       N_("Auto Brightness"),           "Boolean",                          xmpText, xmpExternal, N_("When true, \"Brightness\" is automatically adjusted.") },
        { "AutoContrast",         N_("Auto Contrast"),             "Boolean",                          xmpText, xmpExternal, N_("When true, \"Contrast\" is automatically adjusted.") },
        { "AutoExposure",         N_("Auto Exposure"),             "Boolean",                          xmpText, xmpExternal, N_("When true, \"Exposure\" is automatically adjusted.") },
        { "AutoShadows",          N_("Auto Shadows"),              "Boolean",                          xmpText, xmpExternal, N_("When true,\"Shadows\" is automatically adjusted.") },
        { "BlueHue",              N_("Blue Hue"),                  "Integer",                          xmpText, xmpExternal, N_("\"Blue Hue\" setting. Range -100 to 100.") },
        { "BlueSaturation",       N_("Blue Saturation"),           "Integer",                          xmpText, xmpExternal, N_("\"Blue Saturation\" setting. Range -100 to +100.") },
        { "Brightness",           N_("Brightness"),                "Integer",                          xmpText, xmpExternal, N_("\"Brightness\" setting. Range 0 to +150.") },
        { "CameraProfile",        N_("Camera Profile"),            "Text",                             xmpText, xmpExternal, N_("\"Camera Profile\" setting.") },
        { "ChromaticAberrationB", N_("Chromatic Aberration Blue"), "Integer",                          xmpText, xmpExternal, N_("\"Chomatic Aberration, Fix Blue/Yellow Fringe\" setting. Range -100 to +100.") },
        { "ChromaticAberrationR", N_("Chromatic Aberration Red"),  "Integer",                          xmpText, xmpExternal, N_("\"Chomatic Aberration, Fix Red/Cyan Fringe\" setting. Range -100 to +100.") },
        { "ColorNoiseReduction",  N_("Color Noise Reduction"),     "Integer",                          xmpText, xmpExternal, N_("\"Color Noise Reducton\" setting. Range 0 to +100.") },
        { "Contrast",             N_("Contrast"),                  "Integer",                          xmpText, xmpExternal, N_("\"Contrast\" setting. Range -50 to +100.") },
        { "CropTop",              N_("Crop Top"),                  "Real",                             xmpText, xmpExternal, N_("When HasCrop is true, top of crop rectangle") },
        { "CropLeft",             N_("Crop Left"),                 "Real",                             xmpText, xmpExternal, N_("When HasCrop is true, left of crop rectangle.") },
        { "CropBottom",           N_("Crop Bottom"),               "Real",                             xmpText, xmpExternal, N_("When HasCrop is true, bottom of crop rectangle.") },
        { "CropRight",            N_("Crop Right"),                "Real",                             xmpText, xmpExternal, N_("When HasCrop is true, right of crop rectangle.") },
        { "CropAngle",            N_("Crop Angle"),                "Real",                             xmpText, xmpExternal, N_("When HasCrop is true, angle of crop rectangle.") },
        { "CropWidth",            N_("Crop Width"),                "Real",                             xmpText, xmpExternal, N_("Width of resulting cropped image in CropUnits units.") },
        { "CropHeight",           N_("Crop Height"),               "Real",                             xmpText, xmpExternal, N_("Height of resulting cropped image in CropUnits units.") },
        { "CropUnits",            N_("Crop Units"),                "Integer",                          xmpText, xmpExternal, N_("Units for CropWidth and CropHeight. 0=pixels, 1=inches, 2=cm") },
        { "Exposure",             N_("Exposure"),                  "Real",                             xmpText, xmpExternal, N_("\"Exposure\" setting. Range -4.0 to +4.0.") },
        { "GreenHue",             N_("GreenHue"),                  "Integer",                          xmpText, xmpExternal, N_("\"Green Hue\" setting. Range -100 to +100.") },
        { "GreenSaturation",      N_("Green Saturation"),          "Integer",                          xmpText, xmpExternal, N_("\"Green Saturation\" setting. Range -100 to +100.") },
        { "HasCrop",              N_("Has Crop"),                  "Boolean",                          xmpText, xmpExternal, N_("When true, image has a cropping rectangle.") },
        { "HasSettings",          N_("Has Settings"),              "Boolean",                          xmpText, xmpExternal, N_("When true, non-default camera raw settings.") },
        { "LuminanceSmoothing",   N_("Luminance Smoothing"),       "Integer",                          xmpText, xmpExternal, N_("\"Luminance Smoothing\" setting. Range 0 to +100.") },
        { "RawFileName",          N_("Raw File Name"),             "Text",                             xmpText, xmpInternal, N_("File name fo raw file (not a complete path).") },
        { "RedHue",               N_("Red Hue"),                   "Integer",                          xmpText, xmpExternal, N_("\"Red Hue\" setting. Range -100 to +100.") },
        { "RedSaturation",        N_("Red Saturation"),            "Integer",                          xmpText, xmpExternal, N_("\"Red Saturation\" setting. Range -100 to +100.") },
        { "Saturation",           N_("Saturation"),                "Integer",                          xmpText, xmpExternal, N_("\"Saturation\" setting. Range -100 to +100.") },
        { "Shadows",              N_("Shadows"),                   "Integer",                          xmpText, xmpExternal, N_("\"Shadows\" setting. Range 0 to +100.") },
        { "ShadowTint",           N_("Shadow Tint"),               "Integer",                          xmpText, xmpExternal, N_("\"Shadow Tint\" setting. Range -100 to +100.") },
        { "Sharpness",            N_("Sharpness"),                 "Integer",                          xmpText, xmpExternal, N_("\"Sharpness\" setting. Range 0 to +100.") },
        { "Temperature",          N_("Temperature"),               "Integer",                          xmpText, xmpExternal, N_("\"Temperature\" setting. Range 2000 to 50000.") },
        { "Tint",                 N_("Tint"),                      "Integer",                          xmpText, xmpExternal, N_("\"Tint\" setting. Range -150 to +150.") },
        { "ToneCurve",            N_("Tone Curve"),                "Seq of points (Integer, Integer)", xmpText, xmpExternal, N_("Array of points (Integer, Integer) defining a \"Tone Curve\".") },
        { "ToneCurveName",        N_("Tone Curve Name"),           "Choice Text",                      xmpText, xmpInternal, N_("The name of the Tone Curve described by ToneCurve. One of: Linear, Medium Contrast, "
                                                                                                                                "Strong Contrast, Custom or a user-defined preset name.") },
        { "Version",              N_("Version"),                   "Text",                             xmpText, xmpInternal, N_("Version of Camera Raw plugin.") },
        { "VignetteAmount",       N_("Vignette Amount"),           "Integer",                          xmpText, xmpExternal, N_("\"Vignetting Amount\" setting. Range -100 to +100.") },
        { "VignetteMidpoint",     N_("Vignette Midpoint"),         "Integer",                          xmpText, xmpExternal, N_("\"Vignetting Midpoint\" setting. Range 0 to +100.") },
        { "WhiteBalance",         N_("White Balance"),             "Closed Choice Text",               xmpText, xmpExternal, N_("\"White Balance\" setting. One of: As Shot, Auto, Daylight, Cloudy, Shade, Tungsten, "
                                                                                                                                "Fluorescent, Flash, Custom") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpTiffInfo[] = {
        { "ImageWidth",                N_("Image Width"),                "Integer",                      xmpText, xmpInternal, N_("TIFF tag 256, 0x100. Image width in pixels.") },
        { "ImageLength",               N_("Image Length"),               "Integer",                      xmpText, xmpInternal, N_("TIFF tag 257, 0x101. Image height in pixels.") },
        { "BitsPerSample",             N_("Bits Per Sample"),            "seq Integer",                  xmpSeq,  xmpInternal, N_("TIFF tag 258, 0x102. Number of bits per component in each channel.") },
        { "Compression",               N_("Compression"),                "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 259, 0x103. Compression scheme: 1 = uncompressed; 6 = JPEG.") },
        { "PhotometricInterpretation", N_("Photometric Interpretation"), "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 262, 0x106. Pixel Composition: 2 = RGB; 6 = YCbCr.") },
        { "Orientation",               N_("Orientation"),                "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 274, 0x112. Orientation:"
                                                                                                                                  "1 = 0th row at top, 0th column at left "
                                                                                                                                  "2 = 0th row at top, 0th column at right "
                                                                                                                                  "3 = 0th row at bottom, 0th column at right "
                                                                                                                                  "4 = 0th row at bottom, 0th column at left "
                                                                                                                                  "5 = 0th row at left, 0th column at top "
                                                                                                                                  "6 = 0th row at right, 0th column at top "
                                                                                                                                  "7 = 0th row at right, 0th column at bottom "
                                                                                                                                  "8 = 0th row at left, 0th column at bottom") },
        { "SamplesPerPixel",           N_("Samples Per Pixel"),          "Integer",                      xmpText, xmpInternal, N_("TIFF tag 277, 0x115. Number of components per pixel.") },
        { "PlanarConfiguration",       N_("Planar Configuration"),       "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 284, 0x11C. Data layout:1 = chunky; 2 = planar.") },
        { "YCbCrSubSampling",          N_("YCbCr Sub Sampling"),         "Closed Choice of seq Integer", xmpSeq,  xmpInternal, N_("TIFF tag 530, 0x212. Sampling ratio of chrominance "
                                                                                                                                  "components: [2, 1] = YCbCr4:2:2; [2, 2] = YCbCr4:2:0") },
        { "YCbCrPositioning",          N_("YCbCr Positioning"),          "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 531, 0x213. Position of chrominance vs. "
                                                                                                                                  "luminance components: 1 = centered; 2 = co-sited.") },
        { "XResolution",               N_("X Resolution"),               "Rational",                     xmpText, xmpInternal, N_("TIFF tag 282, 0x11A. Horizontal resolution in pixels per unit.") },
        { "YResolution",               N_("Y Resolution"),               "Rational",                     xmpText, xmpInternal, N_("TIFF tag 283, 0x11B. Vertical resolution in pixels per unit.") },
        { "ResolutionUnit",            N_("Resolution Unit"),            "Closed Choice of Integer",     xmpText, xmpInternal, N_("TIFF tag 296, 0x128. Unit used for XResolution and "
                                                                                                                                  "YResolution. Value is one of: 2 = inches; 3 = centimeters.") },
        { "TransferFunction",          N_("Transfer Function"),          "seq Integer",                  xmpSeq,  xmpInternal, N_("TIFF tag 301, 0x12D. Transfer function for image "
                                                                                                                                  "described in tabular style with 3 * 256 entries.") },
        { "WhitePoint",                N_("White Point"),                "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 318, 0x13E. Chromaticity of white point.") },
        { "PrimaryChromaticities",     N_("Primary Chromaticities"),     "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 319, 0x13F. Chromaticity of the three primary colors.") },
        { "YCbCrCoefficients",         N_("YCbCr Coefficients"),         "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 529, 0x211. Matrix coefficients for RGB to YCbCr transformation.") },
        { "ReferenceBlackWhite",       N_("Reference Black White"),      "seq Rational",                 xmpSeq,  xmpInternal, N_("TIFF tag 532, 0x214. Reference black and white point values.") },
        { "DateTime",                  N_("Date Time"),                  "Date",                         date,    xmpInternal, N_("TIFF tag 306, 0x132 (primary) and EXIF tag 37520, "
                                                                                                                                  "0x9290 (subseconds). Date and time of image creation "
                                                                                                                                  "(no time zone in EXIF), stored in ISO 8601 format, not "
                                                                                                                                  "the original EXIF format. This property includes the "
                                                                                                                                  "value for the EXIF SubSecTime attribute. "
                                                                                                                                  "NOTE: This property is stored in XMP as xmp:ModifyDate.") },
        { "ImageDescription",          N_("Image Description"),          "Lang Alt",                     langAlt, xmpExternal, N_("TIFF tag 270, 0x10E. Description of the image. Note: This property is stored in XMP as dc:description.") },
        { "Make",                      N_("Make"),                       "ProperName",                   xmpText, xmpInternal, N_("TIFF tag 271, 0x10F. Manufacturer of recording equipment.") },
        { "Model",                     N_("Model"),                      "ProperName",                   xmpText, xmpInternal, N_("TIFF tag 272, 0x110. Model name or number of equipment.") },
        { "Software",                  N_("Software"),                   "AgentName",                    xmpText, xmpInternal, N_("TIFF tag 305, 0x131. Software or firmware used to generate image. "
                                                                                                                                  "Note: This property is stored in XMP as xmp:CreatorTool. ") },
        { "Artist",                    N_("Artist"),                     "ProperName",                   xmpText, xmpExternal, N_("TIFF tag 315, 0x13B. Camera owner, photographer or image creator. "
                                                                                                                                  "Note: This property is stored in XMP as the first item in the dc:creator array.") },
        { "Copyright",                 N_("Copyright"),                  "Lang Alt",                     langAlt, xmpExternal, N_("TIFF tag 33432, 0x8298. Copyright information. "
                                                                                                                                  "Note: This property is stored in XMP as dc:rights.") },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    //! exif:ColorSpace
    extern const TagDetails xmpExifColorSpace[] = {
        { 1,     N_("sRGB")         },
        { 2,     N_("Adobe RGB")    },    // Not defined to Exif 2.2 spec. But used by a lot of cameras.
        { 65535, N_("uncalibrated") }
    };

    //! exif:ComponentsConfiguration
    extern const TagDetails xmpExifComponentsConfiguration[] = {
        { 0, "does not exist" },
        { 1, "Y"              },
        { 2, "Cb"             },
        { 3, "Cr"             },
        { 4, "R"              },
        { 5, "G"              },
        { 6, "B"              }
    };

    //! exif:ExposureProgram
    extern const TagDetails xmpExifExposureProgram[] = {
        { 0, N_("not defined")       },
        { 1, N_("Manual")            },
        { 2, N_("Normal program")    },
        { 3, N_("Aperture priority") },
        { 4, N_("Shutter priority")  },
        { 5, N_("Creative program")  },
        { 6, N_("Action program")    },
        { 7, N_("Portrait mode")     },
        { 8, N_("Landscape mode")    }
    };

    //! exif:MeteringMode
    extern const TagDetails xmpExifMeteringMode[] = {
        { 0, N_("unknown")              },
        { 1, N_("Average")               },
        { 2, N_("CenterWeightedAverage") },
        { 3, N_("Spot")                  },
        { 4, N_("MultiSpot")             },
        { 5, N_("Pattern")               },
        { 6, N_("Partial")               },
        { 255, N_("other")               }
    };

    //! exif:LightSource
    extern const TagDetails xmpExifLightSource[] = {
        {   0, N_("unknown")                                 },
        {   1, N_("Daylight")                                },
        {   2, N_("Fluorescent")                             },
        {   3, N_("Tungsten")                                },
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
        { 255, N_("other")                                   }
    };

    //! exif:FocalPlaneResolutionUnit
    extern const TagDetails xmpExifFocalPlaneResolutionUnit[] = {
        { 2, N_("inches")      },
        { 3, N_("centimeters") }
    };

    //! exif:SensingMethod
    extern const TagDetails xmpExifSensingMethod[] = {
        { 1, N_("Not defined")                    },
        { 2, N_("One-chip color area sensor")     },
        { 3, N_("Two-chip color area sensor")     },
        { 4, N_("Three-chip color area sensor")   },
        { 5, N_("Color sequential area sensor")   },
        { 7, N_("Trilinear sensor")               },
        { 8, N_("Color sequential linear sensor") }
    };

    //! exif:FileSource
    extern const TagDetails xmpExifFileSource[] = {
        { 3, N_("DSC") }
    };

    //! exif:SceneType
    extern const TagDetails xmpExifSceneType[] = {
        { 1, N_("directly photographed image") }
    };

    //! exif:CustomRendered
    extern const TagDetails xmpExifCustomRendered[] = {
        { 0, N_("Normal process") },
        { 1, N_("Custom process") }
    };

    //! exif:ExposureMode
    extern const TagDetails xmpExifExposureMode[] = {
        { 0, N_("Auto exposure")   },
        { 1, N_("Manual exposure") },
        { 2, N_("Auto bracket")    }
    };

    //! exif:WhiteBalance
    extern const TagDetails xmpExifWhiteBalance[] = {
        { 0, N_("Auto white balance")   },
        { 1, N_("Manual white balance") }
    };

    //! exif:SceneCaptureType
    extern const TagDetails xmpExifSceneCaptureType[] = {
        { 0, N_("Standard")    },
        { 1, N_("Landscape")   },
        { 2, N_("Portrait")    },
        { 3, N_("Night scene") }
    };

    //! exif:GainControl
    extern const TagDetails xmpExifGainControl[] = {
        { 0, N_("None")           },
        { 1, N_("Low gain up")    },
        { 2, N_("High gain up")   },
        { 3, N_("Low gain down")  },
        { 4, N_("High gain down") }
    };

    //! exif:Contrast, exif:Sharpness
    extern const TagDetails xmpExifNormalSoftHard[] = {
        { 0, N_("Normal") },
        { 1, N_("Soft")   },
        { 2, N_("Hard")   }
    };

    //! exif:Saturation
    extern const TagDetails xmpExifSaturation[] = {
        { 0, N_("Normal")          },
        { 1, N_("Low saturation")  },
        { 2, N_("High saturation") }
    };

    //! exif:SubjectDistanceRange
    extern const TagDetails xmpExifSubjectDistanceRange[] = {
        { 0, N_("Unknown")      },
        { 1, N_("Macro")        },
        { 2, N_("Close view")   },
        { 3, N_("Distant view") }
    };

    //! exif:GPSAltitudeRef
    extern const TagDetails xmpExifGPSAltitudeRef[] = {
        { 0, N_("Above sea level") },
        { 1, N_("Below sea level") }
    };

    //! exif:GPSStatus
    extern const TagDetails xmpExifGPSStatus[] = {
        { 'A', N_("measurement in progress")         },
        { 'V', N_("measurement is interoperability") }
    };

    //! exif:GPSMeasureMode
    extern const TagDetails xmpExifGPSMeasureMode[] = {
        { 2, N_("two-dimensional measurement")   },
        { 3, N_("three-dimensional measurement") }
    };

    //! exif:GPSSpeedRef
    extern const TagDetails xmpExifGPSSpeedRef[] = {
        { 'K', N_("kilometers per hour") },
        { 'M', N_("miles per hour")      },
        { 'N', N_("knots")               }
    };

    //! exif:GPSTrackRef, exif:GPSImgDirectionRef, exif:GPSDestBearingRef
    extern const TagDetails xmpExifGPSDirection[] = {
        { 'T', N_("true direction")     },
        { 'M', N_("magnetic direction") }
    };

    //! exif:GPSDestDistanceRef
    extern const TagDetails xmpExifGPSDestDistanceRef[] = {
        { 'K', N_("kilometers") },
        { 'M', N_("miles")      },
        { 'N', N_("knots")      }
    };

    //! exif:GPSDifferential
    extern const TagDetails xmpExifGPSDifferential[] = {
        { 0, N_("Without correction") },
        { 1, N_("Correction applied") }
    };

    extern const XmpPropertyInfo xmpExifInfo[] = {
        { "ExifVersion",      "ExifVersion",      "Closed Choice of Text", xmpText, xmpInternal, "EXIF tag 36864, 0x9000. EXIF version number." },
        { "FlashpixVersion",  "FlashpixVersion",  "Closed Choice of Text", xmpText, xmpInternal, "EXIF tag 40960, 0xA000. Version of FlashPix." },
        { "ColorSpace",       "ColorSpace",       "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 40961, 0xA001. Color space information" },
        { "ComponentsConfiguration", "ComponentsConfiguration", "Closed Choice of seq Integer", xmpSeq, xmpInternal, "EXIF tag 37121, 0x9101. Configuration of components in data: 4 5 6 0 (if RGB compressed data), 1 2 3 0 (other cases)." },
        { "CompressedBitsPerPixel", "CompressedBitsPerPixel", "Rational",  xmpText, xmpInternal, "EXIF tag 37122, 0x9102. Compression mode used for a compressed image is indicated in unit bits per pixel." },
        { "PixelXDimension",  "PixelXDimension",  "Integer",     xmpText,          xmpInternal, "EXIF tag 40962, 0xA002. Valid image width, in pixels." },
        { "PixelYDimension",  "PixelYDimension",  "Integer",     xmpText,          xmpInternal, "EXIF tag 40963, 0xA003. Valid image height, in pixels." },
        { "UserComment",      "UserComment",      "Lang Alt",    langAlt,          xmpExternal, "EXIF tag 37510, 0x9286. Comments from user." },
        { "RelatedSoundFile", "RelatedSoundFile", "Text",        xmpText,          xmpInternal, "EXIF tag 40964, 0xA004. An \"8.3\" file name for the related sound file." },
        { "DateTimeOriginal", "DateTimeOriginal", "Date",        xmpText,          xmpInternal, "EXIF tags 36867, 0x9003 (primary) and 37521, 0x9291 (subseconds). Date and time when original image was generated, in ISO 8601 format. Includes the EXIF SubSecTimeOriginal data." },
        { "DateTimeDigitized", "DateTimeDigitized", "Date",      xmpText,          xmpInternal, "EXIF tag 36868, 0x9004 (primary) and 37522, 0x9292 (subseconds). Date and time when "
                                                                                                "image was stored as digital data, can be the same as DateTimeOriginal if originally "
                                                                                                "stored in digital form. Stored in ISO 8601 format. Includes the EXIF SubSecTimeDigitized data." },
        { "ExposureTime",     "ExposureTime",     "Rational",    xmpText,          xmpInternal, "EXIF tag 33434, 0x829A. Exposure time in seconds." },
        { "FNumber",          "FNumber",          "Rational",    xmpText,          xmpInternal, "EXIF tag 33437, 0x829D. F number." },
        { "ExposureProgram",  "ExposureProgram",  "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 34850, 0x8822. Class of program used for exposure." },
        { "SpectralSensitivity", "SpectralSensitivity", "Text",  xmpText,          xmpInternal, "EXIF tag 34852, 0x8824. Spectral sensitivity of each channel." },
        { "ISOSpeedRatings",  "ISOSpeedRatings",  "seq Integer", xmpSeq,           xmpInternal, "EXIF tag 34855, 0x8827. ISO Speed and ISO Latitude of the input device as specified in ISO 12232." },
        { "OECF",             "OECF",             "OECF/SFR",    xmpText,          xmpInternal, "EXIF tag 34856, 0x8828. Opto-Electoric Conversion Function as specified in ISO 14524." },
        { "ShutterSpeedValue", "ShutterSpeedValue", "Rational",  xmpText,          xmpInternal, "EXIF tag 37377, 0x9201. Shutter speed, unit is APEX. See Annex C of the EXIF specification." },
        { "ApertureValue",    "ApertureValue",    "Rational",    xmpText,          xmpInternal, "EXIF tag 37378, 0x9202. Lens aperture, unit is APEX." },
        { "BrightnessValue",  "BrightnessValue",  "Rational",    xmpText,          xmpInternal, "EXIF tag 37379, 0x9203. Brightness, unit is APEX." },
        { "ExposureBiasValue", "ExposureBiasValue", "Rational",  xmpText,          xmpInternal, "EXIF tag 37380, 0x9204. Exposure bias, unit is APEX." },
        { "MaxApertureValue", "MaxApertureValue", "Rational",    xmpText,          xmpInternal, "EXIF tag 37381, 0x9205. Smallest F number of lens, in APEX." },
        { "SubjectDistance",  "SubjectDistance",  "Rational",    xmpText,          xmpInternal, "EXIF tag 37382, 0x9206. Distance to subject, in meters." },
        { "MeteringMode",     "MeteringMode",     "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 37383, 0x9207. Metering mode." },
        { "LightSource",      "LightSource",      "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 37384, 0x9208. Light source." },
        { "Flash",            "Flash",            "Flash",       xmpText,          xmpInternal, "EXIF tag 37385, 0x9209. Strobe light (flash) source data." },
        { "FocalLength",      "FocalLength",      "Rational",    xmpText,          xmpInternal, "EXIF tag 37386, 0x920A. Focal length of the lens, in millimeters." },
        { "SubjectArea",      "SubjectArea",      "seq Integer", xmpSeq,           xmpInternal, "EXIF tag 37396, 0x9214. The location and area of the main subject in the overall scene." },
        { "FlashEnergy",      "FlashEnergy",      "Rational",    xmpText,          xmpInternal, "EXIF tag 41483, 0xA20B. Strobe energy during image capture." },
        { "SpatialFrequencyResponse", "SpatialFrequencyResponse", "OECF/SFR", xmpText, xmpInternal, "EXIF tag 41484, 0xA20C. Input device spatial frequency table and SFR values as specified in ISO 12233." },
        { "FocalPlaneXResolution", "FocalPlaneXResolution", "Rational", xmpText,   xmpInternal, "EXIF tag 41486, 0xA20E. Horizontal focal resolution, measured pixels per unit." },
        { "FocalPlaneYResolution", "FocalPlaneYResolution", "Rational", xmpText,   xmpInternal, "EXIF tag 41487, 0xA20F. Vertical focal resolution, measured in pixels per unit." },
        { "FocalPlaneResolutionUnit", "FocalPlaneResolutionUnit", "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41488, 0xA210. Unit used for FocalPlaneXResolution and FocalPlaneYResolution." },
        { "SubjectLocation",  "SubjectLocation",  "seq Integer", xmpSeq,           xmpInternal, "EXIF tag 41492, 0xA214. Location of the main subject of the scene. The first value is the "
                                                                                                "horizontal pixel and the second value is the vertical pixel at which the main subject appears." },
        { "ExposureIndex",    "ExposureIndex",    "Rational",    xmpText,          xmpInternal, "EXIF tag 41493, 0xA215. Exposure index of input device." },
        { "SensingMethod",    "SensingMethod",    "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41495, 0xA217. Image sensor type on input device." },
        { "FileSource",       "FileSource",       "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41728, 0xA300. Indicates image source." },
        { "SceneType",        "SceneType",        "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41729, 0xA301. Indicates the type of scene." },
        { "CFAPattern",       "CFAPattern",       "CFAPattern",  xmpText,          xmpInternal, "EXIF tag 41730, 0xA302. Color filter array geometric pattern of the image sense." },
        { "CustomRendered",   "CustomRendered",   "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41985, 0xA401. Indicates the use of special processing on image data." },
        { "ExposureMode",     "ExposureMode",     "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41986, 0xA402. Indicates the exposure mode set when the image was shot." },
        { "WhiteBalance",     "WhiteBalance",     "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41987, 0xA403. Indicates the white balance mode set when the image was shot." },
        { "DigitalZoomRatio", "DigitalZoomRatio", "Rational",    xmpText,          xmpInternal, "EXIF tag 41988, 0xA404. Indicates the digital zoom ratio when the image was shot." },
        { "FocalLengthIn35mmFilm", "FocalLengthIn35mmFilm", "Integer", xmpText,    xmpInternal, "EXIF tag 41989, 0xA405. Indicates the equivalent focal length assuming a 35mm film "
                                                                                                "camera, in mm. A value of 0 means the focal length is unknown. Note that this tag differs from the FocalLength tag." },
        { "SceneCaptureType", "SceneCaptureType", "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41990, 0xA406. Indicates the type of scene that was shot." },
        { "GainControl",      "GainControl", "Closed Choice of Integer", xmpText,  xmpInternal, "EXIF tag 41991, 0xA407. Indicates the degree of overall image gain adjustment." },
        { "Contrast",         "Contrast", "Closed Choice of Integer", xmpText,     xmpInternal, "EXIF tag 41992, 0xA408. Indicates the direction of contrast processing applied by the camera." },
        { "Saturation",       "Saturation", "Closed Choice of Integer", xmpText,   xmpInternal, "EXIF tag 41993, 0xA409. Indicates the direction of saturation processing applied by the camera." },
        { "Sharpness",        "Sharpness", "Closed Choice of Integer", xmpText,    xmpInternal, "EXIF tag 41994, 0xA40A. Indicates the direction of sharpness processing applied by the camera." },
        { "DeviceSettingDescription", "DeviceSettingDescription", "DeviceSettings", xmpText, xmpInternal, "EXIF tag 41995, 0xA40B. Indicates information on the picture-taking conditions of a particular camera model." },
        { "SubjectDistanceRange", "SubjectDistanceRange", "Closed Choice of Integer", xmpText, xmpInternal, "EXIF tag 41996, 0xA40C. Indicates the distance to the subject." },
        { "ImageUniqueID",    "ImageUniqueID",    "Text",        xmpText,          xmpInternal, "EXIF tag 42016, 0xA420. An identifier assigned uniquely to each image. It is recorded as a 32 "
                                                                                                "character ASCII string, equivalent to hexadecimal notation and 128-bit fixed length." },
        { "GPSVersionID",     "GPSVersionID",     "Text",        xmpText,          xmpInternal, "GPS tag 0, 0x00. A decimal encoding of each of the four EXIF bytes with period separators. The current value is \"2.0.0.0\"." },
        { "GPSLatitude",      "GPSLatitude",      "GPSCoordinate", xmpText,        xmpInternal, "GPS tag 2, 0x02 (position) and 1, 0x01 (North/South). Indicates latitude." },
        { "GPSLongitude",     "GPSLongitude",     "GPSCoordinate", xmpText,        xmpInternal, "GPS tag 4, 0x04 (position) and 3, 0x03 (East/West). Indicates longitude." },
        { "GPSAltitudeRef",   "GPSAltitudeRef",   "Closed Choice of Integer", xmpText, xmpInternal, "GPS tag 5, 0x5. Indicates whether the altitude is above or below sea level." },
        { "GPSAltitude",      "GPSAltitude",      "Rational",    xmpText,          xmpInternal, "GPS tag 6, 0x06. Indicates altitude in meters." },
        { "GPSTimeStamp",     "GPSTimeStamp",     "Date",        xmpText,          xmpInternal, "GPS tag 29 (date), 0x1D, and, and GPS tag 7 (time), 0x07. Time stamp of GPS data, in Coordinated Universal Time. "
                                                                                                "Note: The GPSDateStamp tag is new in EXIF 2.2. The GPS timestamp in EXIF 2.1 does not include a date. If not present, "
                                                                                                "the date component for the XMP should be taken from exif:DateTimeOriginal, or if that is "
                                                                                                "also lacking from exif:DateTimeDigitized. If no date is available, do not write exif:GPSTimeStamp to XMP." },
        { "GPSSatellites",    "GPSSatellites",    "Text",        xmpText,          xmpInternal, "GPS tag 8, 0x08. Satellite information, format is unspecified." },
        { "GPSStatus",        "GPSStatus",        "Closed Choice of Text", xmpText, xmpInternal, "GPS tag 9, 0x09. Status of GPS receiver at image creation time." },
        { "GPSMeasureMode",   "GPSMeasureMode",   "Text",        xmpText,          xmpInternal, "GPS tag 10, 0x0A. GPS measurement mode, Text type." },
        { "GPSDOP",           "GPSDOP",           "Rational",    xmpText,          xmpInternal, "GPS tag 11, 0x0B. Degree of precision for GPS data." },
        { "GPSSpeedRef",      "GPSSpeedRef",      "Closed Choice of Text", xmpText, xmpInternal, "GPS tag 12, 0x0C. Units used to speed measurement." },
        { "GPSSpeed",         "GPSSpeed",         "Rational",    xmpText,          xmpInternal, "GPS tag 13, 0x0D. Speed of GPS receiver movement." },
        { "GPSTrackRef",      "GPSTrackRef",      "Closed Choice of Text", xmpText, xmpInternal, "GPS tag 14, 0x0E. Reference for movement direction." },
        { "GPSTrack",         "GPSTrack",         "Rational",    xmpText,          xmpInternal, "GPS tag 15, 0x0F. Direction of GPS movement, values range from 0 to 359.99." },
        { "GPSImgDirectionRef", "GPSImgDirectionRef", "Closed Choice of Text", xmpText, xmpInternal, "GPS tag 16, 0x10. Reference for movement direction." },
        { "GPSImgDirection",  "GPSImgDirection",  "Rational",    xmpText,          xmpInternal, "GPS tag 17, 0x11. Direction of image when captured, values range from 0 to 359.99." },
        { "GPSMapDatum",      "GPSMapDatum",      "Text",        xmpText,          xmpInternal, "GPS tag 18, 0x12. Geodetic survey data." },
        { "GPSDestLatitude",  "GPSDestLatitude",  "GPSCoordinate", xmpText,        xmpInternal, "GPS tag 20, 0x14 (position) and 19, 0x13 (North/South). Indicates destination latitude." },
        { "GPSDestLongitude", "GPSDestLongitude", "GPSCoordinate", xmpText,        xmpInternal, "GPS tag 22, 0x16 (position) and 21, 0x15 (East/West). Indicates destination longitude." },
        { "GPSDestBearingRef", "GPSDestBearingRef", "Closed Choice of Text", xmpText, xmpInternal, "GPS tag 23, 0x17. Reference for movement direction." },
        { "GPSDestBearing",   "GPSDestBearing",   "Rational",    xmpText,          xmpInternal, "GPS tag 24, 0x18. Destination bearing, values from 0 to 359.99." },
        { "GPSDestDistanceRef", "GPSDestDistanceRef", "Closed Choice  of Text", xmpText, xmpInternal, "GPS tag 25, 0x19. Units used for speed measurement." },
        { "GPSDestDistance",  "GPSDestDistance",  "Rational",    xmpText,          xmpInternal, "GPS tag 26, 0x1A. Distance to destination." },
        { "GPSProcessingMethod", "GPSProcessingMethod", "Text",  xmpText,          xmpInternal, "GPS tag 27, 0x1B. A character string recording the name of the method used for location finding." },
        { "GPSAreaInformation", "GPSAreaInformation", "Text",    xmpText,          xmpInternal, "GPS tag 28, 0x1C. A character string recording the name of the GPS area." },
        { "GPSDifferential",  "GPSDifferential",  "Closed Choice of Integer", xmpText, xmpInternal, "GPS tag 30, 0x1E. Indicates whether differential correction is applied to the GPS receiver." },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpAuxInfo[] = {
        { "Lens",             "Lens",             "Text",        xmpText,          xmpInternal, "A description of the lens used to take the photograph. For example, \"70-200 mm f/2.8-4.0\"." },
        { "SerialNumber",     "SerialNumber",     "Text",        xmpText,          xmpInternal, "The serial number of the camera or camera body used to take the photograph." },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    extern const XmpPropertyInfo xmpIptcInfo[] = {
        { "CiAdrCity",        "Contact Info-City", "Text",       xmpText,          xmpExternal, "The contact information city part." },
        { "CiAdrCtry",        "Contact Info-Country", "Text",    xmpText,          xmpExternal, "The contact information country part." },
        { "CiAdrExtadr",      "Contact Info-Address", "Text",    xmpText,          xmpExternal, "The contact information address part. Comprises an optional company name and all required "
                                                                                                "information to locate the building or postbox to which mail should be sent." },
        { "CiAdrPcode",       "Contact Info-Postal Code", "Text", xmpText,         xmpExternal, "The contact information part denoting the local postal code." },
        { "CiAdrRegion",      "Contact Info-State/Province", "Text", xmpText,      xmpExternal, "The contact information part denoting regional information like state or province." },
        { "CiEmailWork",      "Contact Info-Email", "Text",      xmpText,          xmpExternal, "The contact information email address part." },
        { "CiTelWork",        "Contact Info-Phone", "Text",      xmpText,          xmpExternal, "The contact information phone number part." },
        { "CiUrlWork",        "Contact Info-Web URL", "Text",    xmpText,          xmpExternal, "The contact information web address part." },
        { "CountryCode",      "Country Code",     "closed Choice of Text", xmpText, xmpExternal, "Code of the country the content is focussing on -- either the country shown in visual "
                                                                                                "media or referenced in text or audio media. This element is at the top/first level of "
                                                                                                "a top-down geographical hierarchy. The code should be taken from ISO 3166 two or three "
                                                                                                "letter code. The full name of a country should go to the \"Country\" element." },
        { "CreatorContactInfo", "Creator's Contact Info", "ContactInfo", xmpText,  xmpExternal, "The creator's contact information provides all necessary information to get in contact "
                                                                                                "with the creator of this news object and comprises a set of sub-properties for proper addressing." },
        { "IntellectualGenre", "Intellectual Genre", "Text",     xmpText,          xmpExternal, "Describes the nature, intellectual or journalistic characteristic of a news object, not "
                                                                                                "specifically its content." },
        { "Location",         "Location",         "Text",        xmpText,          xmpExternal, "Name of a location the content is focussing on -- either the location shown in visual "
                                                                                                "media or referenced by text or audio media. This location name could either be the name "
                                                                                                "of a sublocation to a city or the name of a well known location or (natural) monument "
                                                                                                "outside a city. In the sense of a sublocation to a city this element is at the fourth "
                                                                                                "level of a top-down geographical hierarchy." },
        { "Scene",            "IPTC Scene",       "bag closed Choice of Text", xmpBag, xmpExternal, "Describes the scene of a photo content. Specifies one or more terms from the IPTC "
                                                                                                "\"Scene-NewsCodes\". Each Scene is represented as a string of 6 digits in an unordered list." },
        { "SubjectCode",      "IPTC Subject Code", "bag closed Choice of Text", xmpBag, xmpExternal, "Specifies one or more Subjects from the IPTC \"Subject-NewsCodes\" taxonomy to "
                                                                                                "categorize the content. Each Subject is represented as a string of 8 digits in an unordered list." },
        // End of list marker
        { 0, 0, 0, invalidTypeId, xmpInternal, 0 }
    };

    XmpNsInfo::Ns::Ns(const std::string& ns)
        : ns_(ns)
    {
    }

    XmpNsInfo::Prefix::Prefix(const std::string& prefix)
        : prefix_(prefix)
    {
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Ns& ns) const
    {
        std::string n(ns_);
        return n == ns.ns_;
    }

    bool XmpNsInfo::operator==(const XmpNsInfo::Prefix& prefix) const
    {
        std::string p(prefix_);
        return p == prefix.prefix_;
    }

    bool XmpPropertyInfo::operator==(const std::string& name) const
    {
        std::string n(name_);
        return n == name;
    }

    XmpProperties::NsRegistry XmpProperties::nsRegistry_;

    void XmpProperties::registerNs(const std::string& ns,
                                   const std::string& prefix)
    {
        std::string ns2 = ns;
        if (   ns2.substr(ns2.size() - 1, 1) != "/"
            && ns2.substr(ns2.size() - 1, 1) != "#") ns2 += "/";
        nsRegistry_[ns2] = prefix;
        XmpParser::registerNs(ns2, prefix);
    }

    std::string XmpProperties::prefix(const std::string& ns)
    {
        std::string ns2 = ns;
        if (ns2.substr(ns2.size() - 1, 1) != "/") ns2 += "/";
        NsRegistry::const_iterator i = nsRegistry_.find(ns2);
        std::string p;
        if (i != nsRegistry_.end()) {
            p = i->second;
        }
        else {
            const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Ns(ns2));
            if (xn) p = std::string(xn->prefix_);
        }
        return p;
    }

    std::string XmpProperties::ns(const std::string& prefix)
    {
        std::string n;
        for (NsRegistry::const_iterator i = nsRegistry_.begin();
             i != nsRegistry_.end(); ++i) {
            if (i->second == prefix) {
                return i->first;
            }
        }
        return nsInfo(prefix)->ns_;
    }

    const char* XmpProperties::propertyTitle(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->title_ : 0;
    }

    const char* XmpProperties::propertyDesc(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->desc_ : 0;
    }

    TypeId XmpProperties::propertyType(const XmpKey& key)
    {
        const XmpPropertyInfo* pi = propertyInfo(key);
        return pi ? pi->typeId_ : xmpText;
    }

    const XmpPropertyInfo* XmpProperties::propertyInfo(const XmpKey& key)
    {
        const XmpPropertyInfo* pl = propertyList(key.groupName());
        if (!pl) return 0;
        const XmpPropertyInfo* pi = 0;
        for (int i = 0; pl[i].name_ != 0; ++i) {
            if (std::string(pl[i].name_) == key.tagName()) {
                pi = pl + i;
                break;
            }
        }
        return pi;
    }

    const char* XmpProperties::nsDesc(const std::string& prefix)
    {
        return nsInfo(prefix)->desc_;
    }

    const XmpPropertyInfo* XmpProperties::propertyList(const std::string& prefix)
    {
        return nsInfo(prefix)->xmpPropertyInfo_;
    }

    const XmpNsInfo* XmpProperties::nsInfo(const std::string& prefix)
    {
        const XmpNsInfo* xn = find(xmpNsInfo, XmpNsInfo::Prefix(prefix));
        if (!xn) throw Error(35, prefix);
        return xn;
    }

    void XmpProperties::printProperties(std::ostream& os, const std::string& prefix)
    {
        const XmpPropertyInfo* pl = propertyList(prefix);
        if (pl) {
            const int ck = sizeof(pl) / sizeof(pl[0]);                
            for (int k = 0; k < ck; ++k) {
                os << pl[k];
            }
        }

    } // XmpProperties::printProperties

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class XmpKey.
    struct XmpKey::Impl {
        Impl() {}                       //!< Default constructor
        Impl(const std::string& prefix, const std::string& property); //!< Constructor

        /*!
          @brief Parse and convert the \em key string into property and prefix.
                 Updates data members if the string can be decomposed, or throws
                 \em Error.

          @throw Error if the key cannot be decomposed.
        */
        void decomposeKey(const std::string& key);

        // DATA
        static const char* familyName_; //!< "Xmp"

        std::string prefix_;            //!< Prefix
        std::string property_;          //!< Property name
    };
    //! @endcond

    XmpKey::Impl::Impl(const std::string& prefix, const std::string& property)
    {
        // Validate prefix
        if (XmpProperties::ns(prefix).empty()) throw Error(46, prefix);

        property_ = property;
        prefix_ = prefix;
    }

    const char* XmpKey::Impl::familyName_ = "Xmp";

    XmpKey::XmpKey(const std::string& key)
        : p_(new Impl)
    {
        p_->decomposeKey(key);
    }

    XmpKey::XmpKey(const std::string& prefix, const std::string& property)
        : p_(new Impl(prefix, property))
    {
    }

    XmpKey::~XmpKey()
    {
        delete p_;
    }

    XmpKey::XmpKey(const XmpKey& rhs)
        : Key(rhs), p_(new Impl(*rhs.p_))
    {
    }

    XmpKey& XmpKey::operator=(const XmpKey& rhs)
    {
        if (this == &rhs) return *this;
        *p_ = *rhs.p_;
        return *this;
    }

    XmpKey::AutoPtr XmpKey::clone() const
    {
        return AutoPtr(clone_());
    }

    XmpKey* XmpKey::clone_() const
    {
        return new XmpKey(*this);
    }

    std::string XmpKey::key() const
    {
        return std::string(p_->familyName_) + "." + p_->prefix_ + "." + p_->property_;
    }

    const char* XmpKey::familyName() const
    {
        return p_->familyName_;
    }

    std::string XmpKey::groupName() const
    {
        return p_->prefix_;
    }

    std::string XmpKey::tagName() const
    { 
        return p_->property_;
    }

    std::string XmpKey::tagLabel() const
    {
        return XmpProperties::propertyTitle(*this);
    }

    std::string XmpKey::ns() const
    {
        return XmpProperties::ns(p_->prefix_);
    }

    void XmpKey::Impl::decomposeKey(const std::string& key)
    {
        // Get the family name, prefix and property name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string familyName = key.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key);
        std::string prefix = key.substr(pos0, pos1 - pos0);
        if (prefix == "") throw Error(6, key);
        std::string property = key.substr(pos1 + 1);
        if (property == "") throw Error(6, key);

        // Validate prefix
        if (XmpProperties::ns(prefix).empty()) throw Error(46, prefix);

        property_ = property;
        prefix_ = prefix;
    } // XmpKey::Impl::decomposeKey

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const XmpPropertyInfo& property)
    {
        return os << property.name_                       << ",\t"
                  << property.title_                      << ",\t"
                  << property.xmpValueType_               << ",\t"
                  << TypeInfo::typeName(property.typeId_) << ",\t"
                  << ( property.xmpCategory_ == xmpExternal ? "External" : "Internal" ) << ",\t"
                  << property.desc_                       << "\n";
    }

}                                       // namespace Exiv2
