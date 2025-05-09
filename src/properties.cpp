// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "properties.hpp"

#include "error.hpp"
#include "i18n.h"  // NLS support.
#include "image_int.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "value.hpp"
#include "xmp_exiv2.hpp"

#include <iostream>

namespace {
//! Struct used in the lookup table for pretty print functions
struct XmpPrintInfo {
  //! Comparison operator for key
  bool operator==(const std::string& key) const {
    return key == key_;
  }

  std::string_view key_;      //!< XMP key
  Exiv2::PrintFct printFct_;  //!< Print function
};

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

//! @cond IGNORE
extern const XmpPropertyInfo xmpDcInfo[];
extern const XmpPropertyInfo xmpDigikamInfo[];
extern const XmpPropertyInfo xmpKipiInfo[];
extern const XmpPropertyInfo xmpXmpInfo[];
extern const XmpPropertyInfo xmpXmpRightsInfo[];
extern const XmpPropertyInfo xmpXmpMMInfo[];
extern const XmpPropertyInfo xmpXmpBJInfo[];
extern const XmpPropertyInfo xmpXmpTPgInfo[];
extern const XmpPropertyInfo xmpXmpDMInfo[];
extern const XmpPropertyInfo xmpMicrosoftInfo[];
extern const XmpPropertyInfo xmpPdfInfo[];
extern const XmpPropertyInfo xmpPhotoshopInfo[];
extern const XmpPropertyInfo xmpCrsInfo[];
extern const XmpPropertyInfo xmpCrssInfo[];
extern const XmpPropertyInfo xmpTiffInfo[];
extern const XmpPropertyInfo xmpExifInfo[];
extern const XmpPropertyInfo xmpExifEXInfo[];
extern const XmpPropertyInfo xmpAuxInfo[];
extern const XmpPropertyInfo xmpIptcInfo[];
extern const XmpPropertyInfo xmpIptcExtInfo[];
extern const XmpPropertyInfo xmpPlusInfo[];
extern const XmpPropertyInfo xmpMediaProInfo[];
extern const XmpPropertyInfo xmpExpressionMediaInfo[];
extern const XmpPropertyInfo xmpMicrosoftPhotoInfo[];
extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfoInfo[];
extern const XmpPropertyInfo xmpMicrosoftPhotoRegionInfo[];
extern const XmpPropertyInfo xmpMWGRegionsInfo[];
extern const XmpPropertyInfo xmpMWGKeywordInfo[];
extern const XmpPropertyInfo xmpVideoInfo[];
extern const XmpPropertyInfo xmpAudioInfo[];
extern const XmpPropertyInfo xmpDwCInfo[];
extern const XmpPropertyInfo xmpDctermsInfo[];
extern const XmpPropertyInfo xmpLrInfo[];
extern const XmpPropertyInfo xmpAcdseeInfo[];
extern const XmpPropertyInfo xmpGPanoInfo[];

constexpr XmpNsInfo xmpNsInfo[] = {
    // Schemas   -   NOTE: Schemas which the XMP-SDK doesn't know must be registered in XmpParser::initialize -
    // Todo: Automate this
    {"http://purl.org/dc/elements/1.1/", "dc", xmpDcInfo, N_("Dublin Core schema")},
    {"http://www.digikam.org/ns/1.0/", "digiKam", xmpDigikamInfo, N_("digiKam Photo Management schema")},
    {"http://www.digikam.org/ns/kipi/1.0/", "kipi", xmpKipiInfo, N_("KDE Image Program Interface schema")},
    {"http://ns.adobe.com/xap/1.0/", "xmp", xmpXmpInfo, N_("XMP Basic schema")},
    {"http://ns.adobe.com/xap/1.0/rights/", "xmpRights", xmpXmpRightsInfo, N_("XMP Rights Management schema")},
    {"http://ns.adobe.com/xap/1.0/mm/", "xmpMM", xmpXmpMMInfo, N_("XMP Media Management schema")},
    {"http://ns.adobe.com/xap/1.0/bj/", "xmpBJ", xmpXmpBJInfo, N_("XMP Basic Job Ticket schema")},
    {"http://ns.adobe.com/xap/1.0/t/pg/", "xmpTPg", xmpXmpTPgInfo, N_("XMP Paged-Text schema")},
    {"http://ns.adobe.com/xmp/1.0/DynamicMedia/", "xmpDM", xmpXmpDMInfo, N_("XMP Dynamic Media schema")},
    {"http://ns.microsoft.com/photo/1.0/", "MicrosoftPhoto", xmpMicrosoftInfo, N_("Microsoft Photo schema")},
    {"http://ns.adobe.com/lightroom/1.0/", "lr", xmpLrInfo, N_("Adobe Lightroom schema")},
    {"http://ns.adobe.com/pdf/1.3/", "pdf", xmpPdfInfo, N_("Adobe PDF schema")},
    {"http://ns.adobe.com/photoshop/1.0/", "photoshop", xmpPhotoshopInfo, N_("Adobe photoshop schema")},
    {"http://ns.adobe.com/camera-raw-settings/1.0/", "crs", xmpCrsInfo, N_("Camera Raw schema")},
    {"http://ns.adobe.com/camera-raw-saved-settings/1.0/", "crss", xmpCrssInfo, N_("Camera Raw Saved Settings")},
    {"http://ns.adobe.com/tiff/1.0/", "tiff", xmpTiffInfo, N_("Exif Schema for TIFF Properties")},
    {"http://ns.adobe.com/exif/1.0/", "exif", xmpExifInfo, N_("Exif schema for Exif-specific Properties")},
    {"http://cipa.jp/exif/1.0/", "exifEX", xmpExifEXInfo, N_("Exif 2.3 metadata for XMP")},
    {"http://ns.adobe.com/exif/1.0/aux/", "aux", xmpAuxInfo, N_("Exif schema for Additional Exif Properties")},
    {"http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/", "iptc", xmpIptcInfo,
     N_("IPTC Core schema")},  // NOTE: 'Iptc4xmpCore' is just too long, so make 'iptc'
    {"http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/", "Iptc4xmpCore", xmpIptcInfo,
     N_("IPTC Core schema")},  // the default prefix. But provide the official one too.
    {"http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "iptcExt", xmpIptcExtInfo,
     N_("IPTC Extension schema")},  // NOTE: It really should be 'Iptc4xmpExt' but following
    {"http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "Iptc4xmpExt", xmpIptcExtInfo,
     N_("IPTC Extension schema")},  // example above, 'iptcExt' is the default, Iptc4xmpExt works too.
    {"http://ns.useplus.org/ldf/xmp/1.0/", "plus", xmpPlusInfo, N_("PLUS License Data Format schema")},
    {"http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro", xmpMediaProInfo, N_("iView Media Pro schema")},
    {"http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia", xmpExpressionMediaInfo,
     N_("Expression Media schema")},
    {"http://ns.microsoft.com/photo/1.2/", "MP", xmpMicrosoftPhotoInfo, N_("Microsoft Photo 1.2 schema")},
    {"http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI", xmpMicrosoftPhotoRegionInfoInfo,
     N_("Microsoft Photo RegionInfo schema")},
    {"http://ns.microsoft.com/photo/1.2/t/Region#", "MPReg", xmpMicrosoftPhotoRegionInfo,
     N_("Microsoft Photo Region schema")},
    {"http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs", xmpMWGRegionsInfo,
     N_("Metadata Working Group Regions schema")},
    {"http://www.metadataworkinggroup.com/schemas/keywords/", "mwg-kw", xmpMWGKeywordInfo,
     N_("Metadata Working Group Keywords schema")},
    {"http://www.video/", "video", xmpVideoInfo, N_("XMP Extended Video schema")},
    {"http://www.audio/", "audio", xmpAudioInfo, N_("XMP Extended Audio schema")},
    {"http://rs.tdwg.org/dwc/index.htm", "dwc", xmpDwCInfo, N_("XMP Darwin Core schema")},
    {"http://purl.org/dc/terms/", "dcterms", xmpDctermsInfo,
     N_("Qualified Dublin Core schema")},  // Note: used as properties under dwc:record
    {"http://ns.acdsee.com/iptc/1.0/", "acdsee", xmpAcdseeInfo, N_("ACDSee XMP schema")},
    {"http://ns.google.com/photos/1.0/panorama/", "GPano", xmpGPanoInfo, N_("Google Photo Sphere XMP schema")},

    // Structures
    {"http://ns.adobe.com/xap/1.0/g/", "xmpG", nullptr, N_("Colorant structure")},
    {"http://ns.adobe.com/xap/1.0/g/img/", "xmpGImg", nullptr, N_("Thumbnail structure")},
    {"http://ns.adobe.com/xap/1.0/sType/Dimensions#", "stDim", nullptr, N_("Dimensions structure")},
    {"http://ns.adobe.com/xap/1.0/sType/Font#", "stFnt", nullptr, N_("Font structure")},
    {"http://ns.adobe.com/xap/1.0/sType/ResourceEvent#", "stEvt", nullptr, N_("Resource Event structure")},
    {"http://ns.adobe.com/xap/1.0/sType/ResourceRef#", "stRef", nullptr, N_("ResourceRef structure")},
    {"http://ns.adobe.com/xap/1.0/sType/Version#", "stVer", nullptr, N_("Version structure")},
    {"http://ns.adobe.com/xap/1.0/sType/Job#", "stJob", nullptr, N_("Basic Job/Workflow structure")},
    {"http://ns.adobe.com/xmp/sType/Area#", "stArea", nullptr, N_("Area structure")},

    // Qualifiers
    {"http://ns.adobe.com/xmp/Identifier/qual/1.0/", "xmpidq", nullptr, N_("Qualifier for xmp:Identifier")},
};

const XmpPropertyInfo xmpDcInfo[] = {
    {"contributor", N_("Contributor"), "bag ProperName", xmpBag, xmpExternal,
     N_("Contributors to the resource (other than the authors).")},
    {"coverage", N_("Coverage"), "Text", xmpText, xmpExternal,
     N_("The spatial or temporal topic of the resource, the spatial applicability of the "
        "resource, or the jurisdiction under which the resource is relevant.")},
    {"creator", N_("Creator"), "seq ProperName", xmpSeq, xmpExternal,
     N_("The authors of the resource (listed in order of precedence, if significant).")},
    {"date", N_("Date"), "seq Date", xmpSeq, xmpExternal,
     N_("Date(s) that something interesting happened to the resource.")},
    {"description", N_("Description"), "Lang Alt", langAlt, xmpExternal,
     N_("A textual description of the content of the resource. Multiple values may be "
        "present for different languages.")},
    {"format", N_("Format"), "MIMEType", xmpText, xmpInternal,
     N_("The file format used when saving the resource. Tools and applications should set "
        "this property to the save format of the data. It may include appropriate qualifiers.")},
    {"identifier", N_("Identifier"), "Text", xmpText, xmpExternal,
     N_("Unique identifier of the resource. Recommended best practice is to identify the "
        "resource by means of a string conforming to a formal identification system.")},
    {"language", N_("Language"), "bag Locale", xmpBag, xmpInternal,
     N_("An unordered array specifying the languages used in the resource.")},
    {"publisher", N_("Publisher"), "bag ProperName", xmpBag, xmpExternal,
     N_("An entity responsible for making the resource available. Examples of a Publisher "
        "include a person, an organization, or a service. Typically, the name of a Publisher "
        "should be used to indicate the entity.")},
    {"relation", N_("Relation"), "bag Text", xmpBag, xmpInternal,
     N_("Relationships to other documents. Recommended best practice is to identify the "
        "related resource by means of a string conforming to a formal identification system.")},
    {"rights", N_("Rights"), "Lang Alt", langAlt, xmpExternal,
     N_("Informal rights statement, selected by language. Typically, rights information "
        "includes a statement about various property rights associated with the resource, "
        "including intellectual property rights.")},
    {"source", N_("Source"), "Text", xmpText, xmpExternal,
     N_("Unique identifier of the work from which this resource was derived.")},
    {"subject", N_("Subject"), "bag Text", xmpBag, xmpExternal,
     N_("An unordered array of descriptive phrases or keywords that specify the topic of the "
        "content of the resource.")},
    {"title", N_("Title"), "Lang Alt", langAlt, xmpExternal,
     N_("The title of the document, or the name given to the resource. Typically, it will be "
        "a name by which the resource is formally known.")},
    {"type", N_("Type"), "bag open Choice", xmpBag, xmpExternal,
     N_("A document type; for example, novel, poem, or working paper.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpDigikamInfo[] = {
    {"TagsList", N_("Tags List"), "seq Text", xmpSeq, xmpExternal,
     N_("The list of complete tags path as string. The path hierarchy is separated by '/' character (ex.: "
        "\"City/Paris/Monument/Eiffel Tower\".")},
    {"CaptionsAuthorNames", N_("Captions Author Names"), "Lang Alt", langAlt, xmpExternal,
     N_("The list of all captions author names for each language alternative captions set in standard XMP tags.")},
    {"CaptionsDateTimeStamps", N_("Captions Date Time Stamps"), "Lang Alt", langAlt, xmpExternal,
     N_("The list of all captions date time stamps for each language alternative captions set in standard XMP "
        "tags.")},
    {"ImageHistory", N_("Image History"), "Text", xmpText, xmpExternal,
     N_("An XML based content to list all action processed on this image with image editor (as crop, rotate, color "
        "corrections, adjustments, etc.).")},
    {"LensCorrectionSettings", N_("Lens Correction Settings"), "Text", xmpText, xmpExternal,
     N_("The list of Lens Correction tools settings used to fix lens distortion. This include Batch Queue Manager "
        "and Image editor tools based on LensFun library.")},
    {"ColorLabel", N_("Color Label"), "Text", xmpText, xmpExternal,
     N_("The color label assigned to this item. Possible values are \"0\": no label; \"1\": Red; \"2\": Orange; "
        "\"3\": Yellow; \"4\": Green; \"5\": Blue; \"6\": Magenta; \"7\": Gray; \"8\": Black; \"9\": White.")},
    {"PickLabel", N_("Pick Label"), "Text", xmpText, xmpExternal,
     N_("The pick label assigned to this item. Possible values are \"0\": no label; \"1\": item rejected; \"2\": "
        "item in pending validation; \"3\": item accepted.")},
    {"Preview", N_("JPEG preview"), "Text", xmpText, xmpExternal,
     N_("Reduced size JPEG preview image encoded as base64 for a fast screen rendering.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpKipiInfo[] = {
    {"PanoramaInputFiles", N_("Panorama Input Files"), "Text", xmpText, xmpExternal,
     N_("The list of files processed with Hugin program through Panorama tool.")},
    {"EnfuseInputFiles", N_("Enfuse Input Files"), "Text", xmpText, xmpExternal,
     N_("The list of files processed with Enfuse program through ExpoBlending tool.")},
    {"EnfuseSettings", N_("Enfuse Settings"), "Text", xmpText, xmpExternal,
     N_("The list of Enfuse settings used to blend image stack with ExpoBlending tool.")},
    {"picasawebGPhotoId", N_("PicasaWeb Item ID"), "Text", xmpText, xmpExternal,
     N_("Item ID from PicasaWeb web service.")},
    {"yandexGPhotoId", N_("Yandex Fotki Item ID"), "Text", xmpText, xmpExternal,
     N_("Item ID from Yandex Fotki web service.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpInfo[] = {
    {"Advisory", N_("Advisory"), "bag XPath", xmpBag, xmpExternal,
     N_("An unordered array specifying properties that were edited outside the authoring "
        "application. Each item should contain a single namespace and XPath separated by "
        "one ASCII space (U+0020).")},
    {"BaseURL", N_("Base URL"), "URL", xmpText, xmpInternal,
     N_("The base URL for relative URLs in the document content. If this document contains "
        "Internet links, and those links are relative, they are relative to this base URL. "
        "This property provides a standard way for embedded relative URLs to be interpreted "
        "by tools. Web authoring tools should set the value based on their notion of where "
        "URLs will be interpreted.")},
    {"CreateDate", N_("Create Date"), "Date", xmpText, xmpExternal,
     N_("The date and time the resource was originally created.")},
    {"CreatorTool", N_("Creator Tool"), "AgentName", xmpText, xmpInternal,
     N_("The name of the first known tool used to create the resource. If history is "
        "present in the metadata, this value should be equivalent to that of "
        "xmpMM:History's softwareAgent property.")},
    {"Identifier", N_("Identifier"), "bag Text", xmpBag, xmpExternal,
     N_("An unordered array of text strings that unambiguously identify the resource within "
        "a given context. An array item may be qualified with xmpidq:Scheme to denote the "
        "formal identification system to which that identifier conforms. Note: The "
        "dc:identifier property is not used because it lacks a defined scheme qualifier and "
        "has been defined in the XMP Specification as a simple (single-valued) property.")},
    {"Label", N_("Label"), "Text", xmpText, xmpExternal,
     N_("A word or short phrase that identifies a document as a member of a user-defined "
        "collection. Used to organize documents in a file browser.")},
    {"MetadataDate", N_("Metadata Date"), "Date", xmpText, xmpInternal,
     N_("The date and time that any metadata for this resource was last changed. It should "
        "be the same as or more recent than xmp:ModifyDate.")},
    {"ModifyDate", N_("Modify Date"), "Date", xmpText, xmpInternal,
     N_("The date and time the resource was last modified. Note: The value of this property "
        "is not necessarily the same as the file's system modification date because it is "
        "set before the file is saved.")},
    {"Nickname", N_("Nickname"), "Text", xmpText, xmpExternal, N_("A short informal name for the resource.")},
    {"Rating", N_("Rating"), "Closed Choice of Real", xmpText, xmpExternal,
     N_("A number that indicates a document's status relative to other documents, "
        "used to organize documents in a file browser. Values are user-defined within an "
        "application-defined range.")},
    {"Thumbnails", N_("Thumbnails"), "alt Thumbnail", xmpText, xmpInternal,
     N_("An alternative array of thumbnail images for a file, which can differ in "
        "characteristics such as size or image encoding.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpRightsInfo[] = {
    {"Certificate", N_("Certificate"), "URL", xmpText, xmpExternal, N_("Online rights management certificate.")},
    {"Marked", N_("Marked"), "Boolean", xmpText, xmpExternal, N_("Indicates that this is a rights-managed resource.")},
    {"Owner", N_("Owner"), "bag ProperName", xmpBag, xmpExternal,
     N_("An unordered array specifying the legal owner(s) of a resource.")},
    {"UsageTerms", N_("Usage Terms"), "Lang Alt", langAlt, xmpExternal,
     N_("Text instructions on how a resource can be legally used.")},
    {"WebStatement", N_("Web Statement"), "URL", xmpText, xmpExternal,
     N_("The location of a web page describing the owner and/or rights statement for this resource.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpMMInfo[] = {
    {"DerivedFrom", N_("Derived From"), "ResourceRef", xmpText, xmpInternal,
     N_("A reference to the original document from which this one is derived. It is a "
        "minimal reference; missing components can be assumed to be unchanged. For example, "
        "a new version might only need to specify the instance ID and version number of the "
        "previous version, or a rendition might only need to specify the instance ID and "
        "rendition class of the original.")},
    {"DocumentID", N_("Document ID"), "URI", xmpText, xmpInternal,
     N_("The common identifier for all versions and renditions of a document. It should be "
        "based on a UUID; see Document and Instance IDs below.")},
    {"History", N_("History"), "seq ResourceEvent", xmpSeq, xmpInternal,
     N_("An ordered array of high-level user actions that resulted in this resource. It is "
        "intended to give human readers a general indication of the steps taken to make the "
        "changes from the previous version to this one. The list should be at an abstract "
        "level; it is not intended to be an exhaustive keystroke or other detailed history.")},
    {"Ingredients", N_("Ingredients"), "bag ResourceRef", xmpBag, xmpInternal,
     N_("References to resources that were incorporated, by inclusion or reference, into this resource.")},
    {"InstanceID", N_("Instance ID"), "URI", xmpText, xmpInternal,
     N_("An identifier for a specific incarnation of a document, updated each time a file "
        "is saved. It should be based on a UUID; see Document and Instance IDs below.")},
    {"ManagedFrom", N_("Managed From"), "ResourceRef", xmpText, xmpInternal,
     N_("A reference to the document as it was prior to becoming managed. It is set when a "
        "managed document is introduced to an asset management system that does not "
        "currently own it. It may or may not include references to different management systems.")},
    {"Manager", N_("Manager"), "AgentName", xmpText, xmpInternal,
     N_("The name of the asset management system that manages this resource. Along with "
        "xmpMM: ManagerVariant, it tells applications which asset management system to "
        "contact concerning this document.")},
    {"ManageTo", N_("Manage To"), "URI", xmpText, xmpInternal,
     N_("A URI identifying the managed resource to the asset management system; the presence "
        "of this property is the formal indication that this resource is managed. The form "
        "and content of this URI is private to the asset management system.")},
    {"ManageUI", N_("Manage UI"), "URI", xmpText, xmpInternal,
     N_("A URI that can be used to access information about the managed resource through a "
        "web browser. It might require a custom browser plug-in.")},
    {"ManagerVariant", N_("Manager Variant"), "Text", xmpText, xmpInternal,
     N_("Specifies a particular variant of the asset management system. The format of this "
        "property is private to the specific asset management system.")},
    {"OriginalDocumentID", N_("Original Document ID"), "URI", xmpText, xmpInternal,
     N_("Refer to Part 1, Data Model, Serialization, and Core "
        "Properties, for definition.")},
    {"Pantry", N_("Pantry"), "bag struct", xmpText, xmpInternal,
     N_("Each array item has a structure value with a potentially "
        "unique set of fields, containing extracted XMP from a "
        "component. Each field is a property from the XMP of a "
        "contained resource component, with all substructure "
        "preserved. "
        "Each pantry entry shall contain an xmpMM:InstanceID. "
        "Only one copy of the pantry entry for any given "
        "xmpMM:InstanceID shall be retained in the pantry. "
        "Nested pantry items shall be removed from the individual "
        "pantry item and promoted to the top level of the pantry.")},
    {"RenditionClass", N_("Rendition Class"), "RenditionClass", xmpText, xmpInternal,
     N_("The rendition class name for this resource. This property should be absent or set "
        "to default for a document version that is not a derived rendition.")},
    {"RenditionParams", N_("Rendition Params"), "Text", xmpText, xmpInternal,
     N_("Can be used to provide additional rendition parameters that are too complex or "
        "verbose to encode in xmpMM: RenditionClass.")},
    {"VersionID", N_("Version ID"), "Text", xmpText, xmpInternal,
     N_("The document version identifier for this resource. Each version of a document gets "
        "a new identifier, usually simply by incrementing integers 1, 2, 3 . . . and so on. "
        "Media management systems can have other conventions or support branching which "
        "requires a more complex scheme.")},
    {"Versions", N_("Versions"), "seq Version", xmpText, xmpInternal,
     N_("The version history associated with this resource. Entry [1] is the oldest known "
        "version for this document, entry [last()] is the most recent version. Typically, a "
        "media management system would fill in the version information in the metadata on "
        "check-in. It is not guaranteed that a complete history  versions from the first to "
        "this one will be present in the xmpMM:Versions property. Interior version information "
        "can be compressed or eliminated and the version history can be truncated at some point.")},
    {"LastURL", N_("Last URL"), "URL", xmpText, xmpInternal, N_("Deprecated for privacy protection.")},
    {"RenditionOf", N_("Rendition Of"), "ResourceRef", xmpText, xmpInternal,
     N_("Deprecated in favor of xmpMM:DerivedFrom. A reference to the document of which this is "
        "a rendition.")},
    {"SaveID", N_("Save ID"), "Integer", xmpText, xmpInternal,
     N_("Deprecated. Previously used only to support the xmpMM:LastURL property.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpBJInfo[] = {
    {"JobRef", N_("Job Reference"), "bag Job", xmpText, xmpExternal,
     N_("References an external job management file for a job process in which the document is being used. Use of "
        "job "
        "names is under user control. Typical use would be to identify all documents that are part of a particular "
        "job or contract. "
        "There are multiple values because there can be more than one job using a particular document at any time, "
        "and it can "
        "also be useful to keep historical information about what jobs a document was part of previously.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpTPgInfo[] = {
    {"MaxPageSize", N_("Maximum Page Size"), "Dimensions", xmpText, xmpInternal,
     N_("The size of the largest page in the document (including any in contained documents).")},
    {"NPages", N_("Number of Pages"), "Integer", xmpText, xmpInternal,
     N_("The number of pages in the document (including any in contained documents).")},
    {"Fonts", N_("Fonts"), "bag Font", xmpText, xmpInternal,
     N_("An unordered array of fonts that are used in the document (including any in contained documents).")},
    {"Colorants", N_("Colorants"), "seq Colorant", xmpText, xmpInternal,
     N_("An ordered array of colorants (swatches) that are used in the document (including any in contained "
        "documents).")},
    {"PlateNames", N_("Plate Names"), "seq Text", xmpSeq, xmpInternal,
     N_("An ordered array of plate names that are needed to print the document (including any in contained "
        "documents).")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpXmpDMInfo[] = {
    {"absPeakAudioFilePath", N_("Absolute Peak Audio File Path"), "URI", xmpText, xmpInternal,
     N_("The absolute path to the file's peak audio file. If empty, no peak file exists.")},
    {"album", N_("Album"), "Text", xmpText, xmpExternal, N_("The name of the album.")},
    {"altTapeName", N_("Alternative Tape Name"), "Text", xmpText, xmpExternal,
     N_("An alternative tape name, set via the project window or timecode dialog in Premiere. "
        "If an alternative name has been set and has not been reverted, that name is displayed.")},
    {"altTimecode", N_("Alternative Time code"), "Timecode", xmpText, xmpExternal,
     N_("A timecode set by the user. When specified, it is used instead of the startTimecode.")},
    {"artist", N_("Artist"), "Text", xmpText, xmpExternal, N_("The name of the artist or artists.")},
    {"audioModDate", N_("Audio Modified Date"), "Date", xmpText, xmpInternal,
     N_("(deprecated) The date and time when the audio was last modified.")},
    {"audioChannelType", N_("Audio Channel Type"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The audio channel type. One of: Mono, Stereo, 5.1, 7.1, 16 Channel, Other.")},
    {"audioCompressor", N_("Audio Compressor"), "Text", xmpText, xmpInternal,
     N_("The audio compression used. For example, MP3.")},
    {"audioSampleRate", N_("Audio Sample Rate"), "Integer", xmpText, xmpInternal,
     N_("The audio sample rate. Can be any value, but commonly 32000, 44100, or 48000.")},
    {"audioSampleType", N_("Audio Sample Type"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The audio sample type. One of: 8Int, 16Int, 24Int, 32Int, 32Float, Compressed, Packed, Other.")},
    {"beatSpliceParams", N_("Beat Splice Parameters"), "beatSpliceStretch", xmpText, xmpInternal,
     N_("Additional parameters for Beat Splice stretch mode.")},
    {"cameraAngle", N_("Camera Angle"), "open Choice of Text", xmpText, xmpExternal,
     N_("The orientation of the camera to the subject in a static shot, from a fixed set of industry standard "
        "terminology. Predefined values include:Low Angle, Eye Level, High Angle, Overhead Shot, Birds Eye Shot, "
        "Dutch Angle, POV, Over the Shoulder, Reaction Shot.")},
    {"cameraLabel", N_("Camera Label"), "Text", xmpText, xmpExternal,
     N_("A description of the camera used for a shoot. Can be any string, but is usually simply a number, for "
        "example \"1\", \"2\", or more explicitly \"Camera 1\".")},
    {"cameraModel", N_("Camera Model"), "Text", xmpText, xmpExternal,
     N_("The make and model of the camera used for a shoot.")},
    {"cameraMove", N_("Camera Move"), "open Choice of Text", xmpText, xmpExternal,
     N_("The movement of the camera during the shot, from a fixed set of industry standard terminology. Predefined "
        "values include: Aerial, Boom Up, Boom Down, Crane Up, Crane Down, Dolly In, Dolly Out, Pan Left, Pan "
        "Right, Pedestal Up, Pedestal Down, Tilt Up, Tilt Down, Tracking, Truck Left, Truck Right, Zoom In, Zoom "
        "Out.")},
    {"client", N_("Client"), "Text", xmpText, xmpExternal,
     N_("The client for the job of which this shot or take is a part.")},
    {"comment", N_("Comment"), "Text", xmpText, xmpExternal, N_("A user's comments.")},
    {"composer", N_("Composer"), "Text", xmpText, xmpExternal, N_("The composer's name.")},
    {"contributedMedia", N_("Contributed Media"), "bag Media", xmpBag, xmpInternal,
     N_("An unordered list of all media used to create this media.")},
    {"copyright", N_("Copyright"), "Text", xmpText, xmpExternal,
     N_("(Deprecated in favour of dc:rights.) The copyright information.")},
    {"director", N_("Director"), "Text", xmpText, xmpExternal, N_("The director of the scene.")},
    {"directorPhotography", N_("Director Photography"), "Text", xmpText, xmpExternal,
     N_("The director of photography for the scene.")},
    {"duration", N_("Duration"), "Time", xmpText, xmpInternal, N_("The duration of the media file.")},
    {"engineer", N_("Engineer"), "Text", xmpText, xmpExternal, N_("The engineer's name.")},
    {"fileDataRate", N_("File Data Rate"), "Rational", xmpText, xmpInternal,
     N_("The file data rate in megabytes per second. For example: \"36/10\" = 3.6 MB/sec")},
    {"genre", N_("Genre"), "Text", xmpText, xmpExternal, N_("The name of the genre.")},
    {"good", N_("Good"), "Boolean", xmpText, xmpExternal, N_("A checkbox for tracking whether a shot is a keeper.")},
    {"instrument", N_("Instrument"), "Text", xmpText, xmpExternal, N_("The musical instrument.")},
    {"introTime", N_("Intro Time"), "Time", xmpText, xmpInternal, N_("The duration of lead time for queuing music.")},
    {"key", N_("Key"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The audio's musical key. One of: C, C#, D, D#, E, F, F#, G, G#, A, A#, B.")},
    {"logComment", N_("Log Comment"), "Text", xmpText, xmpExternal, N_("User's log comments.")},
    {"loop", N_("Loop"), "Boolean", xmpText, xmpInternal, N_("When true, the clip can be looped seamlessly.")},
    {"numberOfBeats", N_("Number Of Beats"), "Real", xmpText, xmpInternal, N_("The number of beats.")},
    {"markers", N_("Markers"), "seq Marker", xmpSeq, xmpInternal, N_("An ordered list of markers")},
    {"metadataModDate", N_("Metadata Modified Date"), "Date", xmpText, xmpInternal,
     N_("(deprecated) The date and time when the metadata was last modified.")},
    {"outCue", N_("Out Cue"), "Time", xmpText, xmpInternal, N_("The time at which to fade out.")},
    {"projectName", N_("Project Name"), "Text", xmpText, xmpExternal,
     N_("The name of the project of which this file is a part.")},
    {"projectRef", N_("Project Reference"), "ProjectLink", xmpText, xmpInternal,
     N_("A reference to the project that created this file.")},
    {"pullDown", N_("Pull Down"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The sampling phase of film to be converted to video (pull-down). One of: "
        "WSSWW, SSWWW, SWWWS, WWWSS, WWSSW, WSSWW_24p, SSWWW_24p, SWWWS_24p, WWWSS_24p, WWSSW_24p.")},
    {"relativePeakAudioFilePath", N_("Relative Peak Audio File Path"), "URI", xmpText, xmpInternal,
     N_("The relative path to the file's peak audio file. If empty, no peak file exists.")},
    {"relativeTimestamp", N_("Relative Timestamp"), "Time", xmpText, xmpInternal,
     N_("The start time of the media inside the audio project.")},
    {"releaseDate", N_("Release Date"), "Date", xmpText, xmpExternal, N_("The date the title was released.")},
    {"resampleParams", N_("Resample Parameters"), "resampleStretch", xmpText, xmpInternal,
     N_("Additional parameters for Resample stretch mode.")},
    {"scaleType", N_("Scale Type"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The musical scale used in the music. One of: Major, Minor, Both, Neither.")},
    {"scene", N_("Scene"), "Text", xmpText, xmpExternal, N_("The name of the scene.")},
    {"shotDate", N_("Shot Date"), "Date", xmpText, xmpExternal, N_("The date and time when the video was shot.")},
    {"shotDay", N_("Shot Day"), "Text", xmpText, xmpExternal,
     N_("The day in a multiday shoot. For example: \"Day 2\", \"Friday\".")},
    {"shotLocation", N_("Shot Location"), "Text", xmpText, xmpExternal,
     N_("The name of the location where the video was shot. For example: \"Oktoberfest, Munich Germany\" "
        "For more accurate positioning, use the EXIF GPS values.")},
    {"shotName", N_("Shot Name"), "Text", xmpText, xmpExternal, N_("The name of the shot or take.")},
    {"shotNumber", N_("Shot Number"), "Text", xmpText, xmpExternal,
     N_("The position of the shot in a script or production, relative to other shots. For example: 1, 2, 1a, 1b, "
        "1.1, 1.2.")},
    {"shotSize", N_("Shot Size"), "open Choice of Text", xmpText, xmpExternal,
     N_("The size or scale of the shot framing, from a fixed set of industry standard terminology. Predefined "
        "values include: "
        "ECU --extreme close-up, MCU -- medium close-up. CU -- close-up, MS -- medium shot, "
        "WS -- wide shot, MWS -- medium wide shot, EWS -- extreme wide shot.")},
    {"speakerPlacement", N_("Speaker Placement"), "Text", xmpText, xmpExternal,
     N_("A description of the speaker angles from center front in degrees. For example: "
        "\"Left = -30, Right = 30, Center = 0, LFE = 45, Left Surround = -110, Right Surround = 110\"")},
    {"startTimecode", N_("Start Time Code"), "Timecode", xmpText, xmpInternal,
     N_("The timecode of the first frame of video in the file, as obtained from the device control.")},
    {"stretchMode", N_("Stretch Mode"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The audio stretch mode. One of: Fixed length, Time-Scale, Resample, Beat Splice, Hybrid.")},
    {"takeNumber", N_("Take Number"), "Integer", xmpText, xmpExternal,
     N_("A numeric value indicating the absolute number of a take.")},
    {"tapeName", N_("Tape Name"), "Text", xmpText, xmpExternal,
     N_("The name of the tape from which the clip was captured, as set during the capture process.")},
    {"tempo", N_("Tempo"), "Real", xmpText, xmpInternal, N_("The audio's tempo.")},
    {"timeScaleParams", N_("Time Scale Parameters"), "timeScaleStretch", xmpText, xmpInternal,
     N_("Additional parameters for Time-Scale stretch mode.")},
    {"timeSignature", N_("Time Signature"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The time signature of the music. One of: 2/4, 3/4, 4/4, 5/4, 7/4, 6/8, 9/8, 12/8, other.")},
    {"trackNumber", N_("Track Number"), "Integer", xmpText, xmpExternal,
     N_("A numeric value indicating the order of the audio file within its original recording.")},
    {"tracks", N_("Tracks"), "bag Track", xmpBag, xmpInternal,
     N_("An unordered list of tracks. A track is a named set of markers, which can specify a frame rate for all "
        "markers in the set. See also xmpDM:markers.")},
    {"videoAlphaMode", N_("Video Alpha Mode"), "closed Choice of Text", xmpText, xmpExternal,
     N_("The alpha mode. One of: straight, pre-multiplied.")},
    {"videoAlphaPremultipleColor", N_("Video Alpha Premultiple Color"), "Colorant", xmpText, xmpExternal,
     N_("A color in CMYK or RGB to be used as the pre-multiple color when "
        "alpha mode is pre-multiplied.")},
    {"videoAlphaUnityIsTransparent", N_("Video Alpha Unity Is Transparent"), "Boolean", xmpText, xmpInternal,
     N_("When true, unity is clear, when false, it is opaque.")},
    {"videoColorSpace", N_("Video Color Space"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The color space. One of: sRGB (used by Photoshop), CCIR-601 (used for NTSC), "
        "CCIR-709 (used for HD).")},
    {"videoCompressor", N_("Video Compressor"), "Text", xmpText, xmpInternal,
     N_("Video compression used. For example, jpeg.")},
    {"videoFieldOrder", N_("Video Field Order"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The field order for video. One of: Upper, Lower, Progressive.")},
    {"videoFrameRate", N_("Video Frame Rate"), "open Choice of Text", xmpText, xmpInternal,
     N_("The video frame rate. One of: 24, NTSC, PAL.")},
    {"videoFrameSize", N_("Video Frame Size"), "Dimensions", xmpText, xmpInternal,
     N_("The frame size. For example: w:720, h: 480, unit:pixels")},
    {"videoModDate", N_("Video Modified Date"), "Date", xmpText, xmpInternal,
     N_("(deprecated) The date and time when the video was last modified.")},
    {"videoPixelDepth", N_("Video Pixel Depth"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The size in bits of each color component of a pixel. Standard Windows 32-bit "
        "pixels have 8 bits per component. One of: 8Int, 16Int, 32Int, 32Float.")},
    {"videoPixelAspectRatio", N_("Video Pixel Aspect Ratio"), "Rational", xmpText, xmpInternal,
     N_("The aspect ratio, expressed as ht/wd. For example: \"648/720\" = 0.9")},
    {"partOfCompilation", N_("Part Of Compilation"), "Boolean", xmpText, xmpExternal, N_("Part of compilation.")},
    {"lyrics", N_("Lyrics"), "Text", xmpText, xmpExternal, N_("Lyrics text. No association with timecode.")},
    {"discNumber", N_("Disc Number"), "Text", xmpText, xmpExternal,
     N_("If in a multi-disc set, might contain total number of discs. For example: 2/3.")},

    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMicrosoftInfo[] = {
    {"CameraSerialNumber", N_("Camera Serial Number"), "Text", xmpText, xmpExternal, N_("Camera Serial Number.")},
    {"DateAcquired", N_("Date Acquired"), "Date", xmpText, xmpExternal, N_("Date Acquired.")},
    {"FlashManufacturer", N_("Flash Manufacturer"), "Text", xmpText, xmpExternal, N_("Flash Manufacturer.")},
    {"FlashModel", N_("Flash Model"), "Text", xmpText, xmpExternal, N_("Flash Model.")},
    {"LastKeywordIPTC", N_("Last Keyword IPTC"), "bag Text", xmpBag, xmpExternal, N_("Last Keyword IPTC.")},
    {"LastKeywordXMP", N_("Last Keyword XMP"), "bag Text", xmpBag, xmpExternal, N_("Last Keyword XMP.")},
    {"LensManufacturer", N_("Lens Manufacturer"), "Text", xmpText, xmpExternal, N_("Lens Manufacturer.")},
    {"LensModel", N_("Lens Model"), "Text", xmpText, xmpExternal, N_("Lens Model.")},
    {"Rating", N_("Rating Percent"), "Text", xmpText, xmpExternal, N_("Rating Percent.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpLrInfo[] = {
    {"hierarchicalSubject", N_("Hierarchical Subject"), "bag Text", xmpBag, xmpExternal,
     N_("Adobe Lightroom hierarchical keywords.")},
    {"privateRTKInfo", N_("Private RTK Info"), "Text", xmpText, xmpExternal, N_("Adobe Lightroom private RTK info.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpPdfInfo[] = {
    {"Keywords", N_("Keywords"), "Text", xmpText, xmpExternal, N_("Keywords.")},
    {"PDFVersion", N_("PDF Version"), "Text", xmpText, xmpInternal,
     N_("The PDF file version (for example: 1.0, 1.3, and so on).")},
    {"Producer", N_("Producer"), "AgentName", xmpText, xmpInternal,
     N_("The name of the tool that created the PDF document.")},
    {"Trapped", N_("Trapped"), "Boolean", xmpText, xmpExternal, N_("True when the document has been trapped.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpPhotoshopInfo[] = {
    {"DateCreated", N_("Date Created"), "Date", xmpText, xmpExternal,
     N_("The date the intellectual content of the document was created (rather than the creation "
        "date of the physical representation), following IIM conventions. For example, a photo "
        "taken during the American Civil War would have a creation date during that epoch "
        "(1861-1865) rather than the date the photo was digitized for archiving.")},
    {"Headline", N_("Headline"), "Text", xmpText, xmpExternal, N_("Headline.")},
    {"Country", N_("Country"), "Text", xmpText, xmpExternal, N_("Country/primary location.")},
    {"State", N_("State"), "Text", xmpText, xmpExternal, N_("Province/state.")},
    {"City", N_("City"), "Text", xmpText, xmpExternal, N_("City.")},
    {"Credit", N_("Credit"), "Text", xmpText, xmpExternal, N_("Credit.")},
    {"AuthorsPosition", N_("Authors Position"), "Text", xmpText, xmpExternal, N_("By-line title.")},
    {"CaptionWriter", N_("Caption Writer"), "Text", xmpText, xmpExternal, N_("Writer/editor.")},
    {"Category", N_("Category"), "Text", xmpText, xmpExternal, N_("Category. Limited to 3 7-bit ASCII characters.")},
    {"Instructions", N_("Instructions"), "Text", xmpText, xmpExternal, N_("Special instructions.")},
    {"Source", N_("Source"), "Text", xmpText, xmpExternal, N_("Source.")},
    {"SupplementalCategories", N_("Supplemental Categories"), "bag Text", xmpBag, xmpExternal,
     N_("Supplemental category.")},
    {"TransmissionReference", N_("Transmission Reference"), "Text", xmpText, xmpExternal,
     N_("Original transmission reference.")},
    {"Urgency", N_("Urgency"), "Integer", xmpText, xmpExternal, N_("Urgency. Valid range is 1-8.")},
    {"ICCProfile", N_("ICC Profile"), "Text", xmpText, xmpInternal,
     N_("The colour profile, such as AppleRGB, AdobeRGB1998.")},
    {"ColorMode", N_("Color Mode"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("The colour mode. One of: 0 = Bitmap, 1 = Grayscale, 2 = Indexed, 3 = RGB, 4 = CMYK, 7 = Multichannel, 8 = "
        "Duotone, 9 = Lab.")},
    {"AncestorID", N_("Ancestor ID"), "URI", xmpText, xmpExternal, N_("The unique identifier of a document.")},
    {"DocumentAncestors", N_("Document Ancestors"), "bag Ancestor", xmpBag, xmpExternal,
     N_("If the source document for a copy-and-paste or place operation has a document ID, that ID is added to "
        "this list in the destination document's XMP.")},
    {"History", N_("History"), "Text", xmpText, xmpExternal,
     N_("The history that appears in the FileInfo panel, if activated in the application preferences.")},
    {"TextLayers", N_("Text Layers"), "seq Layer", xmpSeq, xmpExternal,
     N_("If a document has text layers, this property caches the text for each layer.")},
    {"LayerName", N_("Layer Name"), "Text", xmpText, xmpExternal, N_("The identifying name of the text layer.")},
    {"LayerText", N_("Layer Text"), "Text", xmpText, xmpExternal, N_("The text content of the text layer.")},
    {"EmbeddedXMPDigest", N_("Embedded XMP Digest"), "Text", xmpText, xmpExternal, N_("Embedded XMP Digest.")},
    {"LegacyIPTCDigest", N_("Legacy IPTC Digest"), "Text", xmpText, xmpExternal, N_("Legacy IPTC Digest.")},
    {"SidecarForExtension", N_("Sidecar F or Extension"), "Text", xmpText, xmpExternal,
     N_("Filename extension of associated image file.")},

    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

//! XMP crs:CropUnits
constexpr TagDetails crsCropUnits[] = {
    {0, N_("pixels")},
    {1, N_("inches")},
    {2, N_("cm")},
};

const XmpPropertyInfo xmpCrssInfo[] = {
    {"SavedSettings", N_("Saved Settings"), "SavedSettings", xmpText, xmpInternal,
     N_("*Main structure* Camera Raw Saved Settings.")},
    {"Name", N_("Name"), "Text", xmpText, xmpExternal, N_("Camera Raw Saved Settings Name.")},
    {"Type", N_("Type"), "Text", xmpText, xmpExternal, N_("Camera Raw Saved Settings Type.")},
    {"Parameters", N_("Parameters"), "Parameters", xmpText, xmpInternal,
     N_("*Main structure* Camera Raw Saved Settings Parameters.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpCrsInfo[] = {
    {"AutoBrightness", N_("Auto Brightness"), "Boolean", xmpText, xmpInternal,
     N_("When true, \"Brightness\" is automatically adjusted.")},
    {"AutoContrast", N_("Auto Contrast"), "Boolean", xmpText, xmpInternal,
     N_("When true, \"Contrast\" is automatically adjusted.")},
    {"AutoExposure", N_("Auto Exposure"), "Boolean", xmpText, xmpInternal,
     N_("When true, \"Exposure\" is automatically adjusted.")},
    {"AutoShadows", N_("Auto Shadows"), "Boolean", xmpText, xmpInternal,
     N_("When true,\"Shadows\" is automatically adjusted.")},
    {"BlueHue", N_("Blue Hue"), "Integer", xmpText, xmpInternal, N_("\"Blue Hue\" setting. Range -100 to 100.")},
    {"BlueSaturation", N_("Blue Saturation"), "Integer", xmpText, xmpInternal,
     N_("\"Blue Saturation\" setting. Range -100 to +100.")},
    {"Brightness", N_("Brightness"), "Integer", xmpText, xmpInternal, N_("\"Brightness\" setting. Range 0 to +150.")},
    {"CameraProfile", N_("Camera Profile"), "Text", xmpText, xmpInternal, N_("\"Camera Profile\" setting.")},
    {"ChromaticAberrationB", N_("Chromatic Aberration Blue"), "Integer", xmpText, xmpInternal,
     N_("\"Chromatic Aberration, Fix Blue/Yellow Fringe\" setting. Range -100 to +100.")},
    {"ChromaticAberrationR", N_("Chromatic Aberration Red"), "Integer", xmpText, xmpInternal,
     N_("\"Chromatic Aberration, Fix Red/Cyan Fringe\" setting. Range -100 to +100.")},
    {"ColorNoiseReduction", N_("Color Noise Reduction"), "Integer", xmpText, xmpInternal,
     N_("\"Color Noise Reduction\" setting. Range 0 to +100.")},
    {"Contrast", N_("Contrast"), "Integer", xmpText, xmpInternal, N_("\"Contrast\" setting. Range -50 to +100.")},
    {"CropTop", N_("Crop Top"), "Real", xmpText, xmpInternal, N_("When \"Has Crop\" is true, top of crop rectangle")},
    {"CropLeft", N_("Crop Left"), "Real", xmpText, xmpInternal,
     N_("When \"Has Crop\" is true, left of crop rectangle.")},
    {"CropBottom", N_("Crop Bottom"), "Real", xmpText, xmpInternal,
     N_("When \"Has Crop\" is true, bottom of crop rectangle.")},
    {"CropRight", N_("Crop Right"), "Real", xmpText, xmpInternal,
     N_("When \"Has Crop\" is true, right of crop rectangle.")},
    {"CropAngle", N_("Crop Angle"), "Real", xmpText, xmpInternal,
     N_("When \"Has Crop\" is true, angle of crop rectangle.")},
    {"CropWidth", N_("Crop Width"), "Real", xmpText, xmpInternal,
     N_("Width of resulting cropped image in CropUnits units.")},
    {"CropHeight", N_("Crop Height"), "Real", xmpText, xmpInternal,
     N_("Height of resulting cropped image in CropUnits units.")},
    {"CropUnits", N_("Crop Units"), "Integer", xmpText, xmpInternal,
     N_("Units for CropWidth and CropHeight. 0=pixels, 1=inches, 2=cm")},
    {"Exposure", N_("Exposure"), "Real", xmpText, xmpInternal, N_("\"Exposure\" setting. Range -4.0 to +4.0.")},
    {"GreenHue", N_("Green Hue"), "Integer", xmpText, xmpInternal, N_("\"Green Hue\" setting. Range -100 to +100.")},
    {"GreenSaturation", N_("Green Saturation"), "Integer", xmpText, xmpInternal,
     N_("\"Green Saturation\" setting. Range -100 to +100.")},
    {"HasCrop", N_("Has Crop"), "Boolean", xmpText, xmpInternal, N_("When true, image has a cropping rectangle.")},
    {"HasSettings", N_("Has Settings"), "Boolean", xmpText, xmpInternal,
     N_("When true, non-default camera raw settings.")},
    {"LuminanceSmoothing", N_("Luminance Smoothing"), "Integer", xmpText, xmpInternal,
     N_("\"Luminance Smoothing\" setting. Range 0 to +100.")},
    {"RawFileName", N_("Raw File Name"), "Text", xmpText, xmpInternal,
     N_("File name of raw file (not a complete path).")},
    {"RedHue", N_("Red Hue"), "Integer", xmpText, xmpInternal, N_("\"Red Hue\" setting. Range -100 to +100.")},
    {"RedSaturation", N_("Red Saturation"), "Integer", xmpText, xmpInternal,
     N_("\"Red Saturation\" setting. Range -100 to +100.")},
    {"Saturation", N_("Saturation"), "Integer", xmpText, xmpInternal,
     N_("\"Saturation\" setting. Range -100 to +100.")},
    {"Shadows", N_("Shadows"), "Integer", xmpText, xmpInternal, N_("\"Shadows\" setting. Range 0 to +100.")},
    {"ShadowTint", N_("Shadow Tint"), "Integer", xmpText, xmpInternal,
     N_("\"Shadow Tint\" setting. Range -100 to +100.")},
    {"Sharpness", N_("Sharpness"), "Integer", xmpText, xmpInternal, N_("\"Sharpness\" setting. Range 0 to +100.")},
    {"Temperature", N_("Temperature"), "Integer", xmpText, xmpInternal,
     N_("\"Temperature\" setting. Range 2000 to 50000.")},
    {"Tint", N_("Tint"), "Integer", xmpText, xmpInternal, N_("\"Tint\" setting. Range -150 to +150.")},
    {"ToneCurve", N_("Tone Curve"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurveName", N_("Tone Curve Name"), "Choice Text", xmpText, xmpInternal,
     N_("The name of the Tone Curve described by ToneCurve. One of: Linear, Medium Contrast, "
        "Strong Contrast, Custom or a user-defined preset name.")},
    {"Version", N_("Version"), "Text", xmpText, xmpInternal, N_("Version of Camera Raw plugin.")},
    {"VignetteAmount", N_("Vignette Amount"), "Integer", xmpText, xmpInternal,
     N_("\"Vignetting Amount\" setting. Range -100 to +100.")},
    {"VignetteMidpoint", N_("Vignette Midpoint"), "Integer", xmpText, xmpInternal,
     N_("\"Vignetting Midpoint\" setting. Range 0 to +100.")},
    {"WhiteBalance", N_("White Balance"), "Closed Choice Text", xmpText, xmpInternal,
     N_("\"White Balance\" setting. One of: As Shot, Auto, Daylight, Cloudy, Shade, Tungsten, "
        "Fluorescent, Flash, Custom")},
    // The following properties are not in the XMP specification. They are found in sample files from Adobe
    // applications and in exiftool.
    {"AlreadyApplied", N_("Already Applied"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Converter", N_("Converter"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"MoireFilter", N_("Moire Filter"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Smoothness", N_("Smoothness"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"CameraProfileDigest", N_("Camera Profile Digest"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Clarity", N_("Clarity"), "Integer", xmpText, xmpInternal, N_("Not in XMP Specification. Found in sample files.")},
    {"ConvertToGrayscale", N_("Convert To Grayscale"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Defringe", N_("Defringe"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"FillLight", N_("Fill Light"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HighlightRecovery", N_("Highlight Recovery"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentAqua", N_("Hue Adjustment Aqua"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentBlue", N_("Hue Adjustment Blue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentGreen", N_("Hue Adjustment Green"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentMagenta", N_("Hue Adjustment Magenta"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentOrange", N_("Hue Adjustment Orange"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentPurple", N_("Hue Adjustment Purple"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentRed", N_("Hue Adjustment Red"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"HueAdjustmentYellow", N_("Hue Adjustment Yellow"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"IncrementalTemperature", N_("Incremental Temperature"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"IncrementalTint", N_("Incremental Tint"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentAqua", N_("Luminance Adjustment Aqua"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentBlue", N_("Luminance Adjustment Blue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentGreen", N_("Luminance Adjustment Green"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentMagenta", N_("Luminance Adjustment Magenta"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentOrange", N_("Luminance Adjustment Orange"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentPurple", N_("Luminance Adjustment Purple"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentRed", N_("Luminance Adjustment Red"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceAdjustmentYellow", N_("Luminance Adjustment Yellow"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricDarks", N_("Parametric Darks"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricHighlights", N_("Parametric Highlights"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricHighlightSplit", N_("Parametric Highlight Split"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricLights", N_("Parametric Lights"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricMidtoneSplit", N_("Parametric Midtone Split"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricShadows", N_("Parametric Shadows"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ParametricShadowSplit", N_("Parametric Shadow Split"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentAqua", N_("Saturation Adjustment Aqua"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentBlue", N_("Saturation Adjustment Blue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentGreen", N_("Saturation Adjustment Green"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentMagenta", N_("Saturation Adjustment Magenta"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentOrange", N_("Saturation Adjustment Orange"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentPurple", N_("Saturation Adjustment Purple"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentRed", N_("Saturation Adjustment Red"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SaturationAdjustmentYellow", N_("Saturation Adjustment Yellow"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SharpenDetail", N_("Sharpen Detail"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SharpenEdgeMasking", N_("Sharpen Edge Masking"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SharpenRadius", N_("Sharpen Radius"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SplitToningBalance", N_("Split Toning Balance"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SplitToningHighlightHue", N_("Split Toning Highlight Hue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SplitToningHighlightSaturation", N_("Split Toning Highlight Saturation"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SplitToningShadowHue", N_("Split Toning Shadow Hue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"SplitToningShadowSaturation", N_("Split Toning Shadow Saturation"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Vibrance", N_("Vibrance"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerRed", N_("Gray Mixer Red"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerOrange", N_("Gray Mixer Orange"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerYellow", N_("Gray Mixer Yellow"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerGreen", N_("Gray Mixer Green"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerAqua", N_("Gray Mixer Aqua"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerBlue", N_("Gray Mixer Blue"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerPurple", N_("Gray Mixer Purple"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrayMixerMagenta", N_("Gray Mixer Magenta"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"RetouchInfo", N_("Retouch Info"), "bag Text", xmpBag, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"RedEyeInfo", N_("Red Eye Info"), "bag Text", xmpBag, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"CropUnit", N_("Crop Unit"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteAmount", N_("Post Crop Vignette Amount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteMidpoint", N_("Post Crop Vignette Midpoint"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteFeather", N_("Post Crop Vignette Feather"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteRoundness", N_("Post Crop Vignette Roundness"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteStyle", N_("Post Crop Vignette Style"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ProcessVersion", N_("Process Version"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileEnable", N_("Lens Profile Enable"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileSetup", N_("Lens Profile Setup"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileName", N_("Lens Profile Name"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileFilename", N_("Lens Profile Filename"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileDigest", N_("Lens Profile Digest"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileDistortionScale", N_("Lens Profile Distortion Scale"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileChromaticAberrationScale", N_("Lens Profile Chromatic Aberration Scale"), "Integer", xmpText,
     xmpInternal, N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileVignettingScale", N_("Lens Profile Vignetting Scale"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensManualDistortionAmount", N_("Lens Manual Distortion Amount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveVertical", N_("Perspective Vertical"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveHorizontal", N_("Perspective Horizontal"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveRotate", N_("Perspective Rotate"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveScale", N_("Perspective Scale"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"CropConstrainToWarp", N_("Crop Constrain To Warp"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceNoiseReductionDetail", N_("Luminance Noise Reduction Detail"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LuminanceNoiseReductionContrast", N_("Luminance Noise Reduction Contrast"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ColorNoiseReductionDetail", N_("Color Noise Reduction Detail"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrainAmount", N_("Grain Amount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrainSize", N_("Grain Size"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"GrainFrequency", N_("GrainFrequency"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"AutoLateralCA", N_("Auto Lateral CA"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Exposure2012", N_("Exposure 2012"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Contrast2012", N_("Contrast 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Highlights2012", N_("Highlights 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Shadows2012", N_("Shadows 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Whites2012", N_("Whites 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Blacks2012", N_("Blacks 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Clarity2012", N_("Clarity 2012"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PostCropVignetteHighlightContrast", N_("Post Crop Vignette Highlight Contrast"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ToneCurveName2012", N_("Tone Curve Name 2012"), "Text", xmpText, xmpInternal,
     N_("Values: Linear, Medium Contrast, Strong Contrast, Custom. Not in XMP Specification. Found in sample "
        "files.")},
    {"ToneCurveRed", N_("Tone Curve Red"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurveGreen", N_("Tone Curve Green"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurveBlue", N_("Tone Curve Blue"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurvePV2012", N_("Tone Curve PV 2012"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurvePV2012Red", N_("Tone Curve PV 2012 Red"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurvePV2012Green", N_("Tone Curve PV 2012 Green"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"ToneCurvePV2012Blue", N_("Tone Curve PV 2012 Blue"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer) defining a \"Tone Curve\".")},
    {"DefringePurpleAmount", N_("Defringe Purple Amount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefringePurpleHueLo", N_("Defringe Purple Hue Lo"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefringePurpleHueHi", N_("Defringe Purple Hue Hi"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefringeGreenAmount", N_("Defringe Green Amount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefringeGreenHueLo", N_("Defringe Green Hue Lo"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefringeGreenHueHi", N_("Defringe Green Hue Hi"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"AutoWhiteVersion", N_("Defringe Green Hue Hi"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"ColorNoiseReductionSmoothness", N_("Color Noise Reduction Smoothness"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveAspect", N_("Perspective Aspect"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerspectiveUpright", N_("Perspective Upright"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightVersion", N_("Upright Version"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightCenterMode", N_("Upright Center Mode"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightCenterNormX", N_("Upright Center Norm X"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightCenterNormY", N_("Upright Center Norm Y"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightFocalMode", N_("Upright Focal Mode"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightFocalLength35mm", N_("Upright Focal Length 35mm"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightPreview", N_("Upright Preview"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransformCount", N_("Upright TransformCount"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightDependentDigest", N_("Upright DependentDigest"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransform_0", N_("Upright Transform_0"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransform_1", N_("Upright Transform_1"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransform_2", N_("Upright Transform_2"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransform_3", N_("Upright Transform_3"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"UprightTransform_4", N_("Upright Transform_4"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyExifMake", N_("Lens Profile Match Key Exif Make"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyExifModel", N_("Lens Profile Match Key Exif Model"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyCameraModelName", N_("Lens Profile Match Key Camera Model Name"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyLensInfo", N_("Lens Profile Match Key Lens Info"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyLensID", N_("Lens Profile Match Key Lens ID"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyLensName", N_("Lens Profile Match Key Lens Name"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeyIsRaw", N_("Lens Profile Match Key Is Raw"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"LensProfileMatchKeySensorFormatFactor", N_("Lens Profile Match Key Sensor Format Factor"), "Real", xmpText,
     xmpInternal, N_("Not in XMP Specification. Found in sample files.")},
    {"DefaultAutoTone", N_("Default Auto Tone"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefaultAutoGray", N_("DefaultAuto Gray"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefaultsSpecificToSerial", N_("Defaults Specific To Serial"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DefaultsSpecificToISO", N_("Defaults Specific To ISO"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"DNGIgnoreSidecars", N_("DNG IgnoreSidecars"), "Boolean", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"NegativeCachePath", N_("Negative Cache Path"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"NegativeCacheMaximumSize", N_("Negative Cache Maximum Size"), "Real", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"NegativeCacheLargePreviewSize", N_("Negative Cache Large Preview Size"), "Integer", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"JPEGHandling", N_("JPEG Handling"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"TIFFHandling", N_("TIFF Handling"), "Text", xmpText, xmpInternal,
     N_("Not in XMP Specification. Found in sample files.")},
    // Corrections root structure properties
    {"CircularGradientBasedCorrections", N_("CircularGradientBasedCorrections"), "CircularGradientBasedCorrections",
     xmpText, xmpInternal, N_("*Root structure* ")},
    {"PaintBasedCorrections", N_("PaintBasedCorrections"), "PaintBasedCorrections", xmpText, xmpInternal,
     N_("*Root structure* ")},
    {"CorrectionMasks", N_("CorrectionMasks"), "CorrectionMasks", xmpText, xmpInternal, N_("*sub Root structure* ")},
    // Corrections child properties
    {"What", N_("What"), "Text", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"MaskValue", N_("Mask Value"), "Real", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Radius", N_("Radius"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Flow", N_("Flow"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"CenterWeight", N_("Center Weight"), "Real", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Dabs", N_("Dabs"), "Seq of points (Integer, Integer)", xmpText, xmpInternal,
     N_("Array of points (Integer, Integer).")},
    {"ZeroX", N_("Zero X"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"ZeroY", N_("Zero Y"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"FullX", N_("Full X"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"FullY", N_("Full Y"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Top", N_("Top"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Left", N_("Left"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Bottom", N_("Bottom"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Right", N_("Right"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Angle", N_("Angle"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Midpoint", N_("Midpoint"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Roundness", N_("Roundness"), "Real", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Feather", N_("Feather"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Flipped", N_("Flipped"), "Boolean", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Version", N_("Version"), "Integer", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"SizeX", N_("Size X"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"SizeY", N_("Size Y"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"X", N_("X"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Y", N_("Y"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Alpha", N_("Alpha"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"CenterValue", N_("Center Value"), "Real", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"PerimeterValue", N_("Perimeter Value"), "Real", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    // Retouch root structure properties
    {"RetouchAreas", N_("RetouchAreas"), "RetouchAreas", xmpText, xmpInternal, N_("*Root structure* ")},
    // Retouch child properties
    {"SpotType", N_("Spot Type"), "Text", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"SourceState", N_("Source State"), "Text", xmpText, xmpExternal,
     N_("Not in XMP Specification. Found in sample files.")},
    {"Method", N_("Method"), "Text", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"SourceX", N_("Source X"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"OffsetY", N_("Offset Y"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Opacity", N_("Opacity"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Feather", N_("Feather"), "Real", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    {"Seed", N_("Seed"), "Integer", xmpText, xmpExternal, N_("Not in XMP Specification. Found in sample files.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpTiffInfo[] = {
    {"ImageWidth", N_("Image Width"), "Integer", xmpText, xmpInternal,
     N_("TIFF tag 256, 0x100. Image width in pixels.")},
    {"ImageLength", N_("Image Length"), "Integer", xmpText, xmpInternal,
     N_("TIFF tag 257, 0x101. Image height in pixels.")},
    {"BitsPerSample", N_("Bits Per Sample"), "seq Integer", xmpSeq, xmpInternal,
     N_("TIFF tag 258, 0x102. Number of bits per component in each channel.")},
    {"Compression", N_("Compression"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 259, 0x103. Compression scheme: 1 = uncompressed; 6 = JPEG.")},
    {"PhotometricInterpretation", N_("Photometric Interpretation"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 262, 0x106. Pixel Composition: 2 = RGB; 6 = YCbCr.")},
    {"Orientation", N_("Orientation"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 274, 0x112. Orientation:"
        "1 = 0th row at top, 0th column at left "
        "2 = 0th row at top, 0th column at right "
        "3 = 0th row at bottom, 0th column at right "
        "4 = 0th row at bottom, 0th column at left "
        "5 = 0th row at left, 0th column at top "
        "6 = 0th row at right, 0th column at top "
        "7 = 0th row at right, 0th column at bottom "
        "8 = 0th row at left, 0th column at bottom")},
    {"SamplesPerPixel", N_("Samples Per Pixel"), "Integer", xmpText, xmpInternal,
     N_("TIFF tag 277, 0x115. Number of components per pixel.")},
    {"PlanarConfiguration", N_("Planar Configuration"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 284, 0x11C. Data layout:1 = chunky; 2 = planar.")},
    {"YCbCrSubSampling", N_("YCbCr Sub Sampling"), "Closed Choice of seq Integer", xmpSeq, xmpInternal,
     N_("TIFF tag 530, 0x212. Sampling ratio of chrominance "
        "components: [2, 1] = YCbCr4:2:2; [2, 2] = YCbCr4:2:0")},
    {"YCbCrPositioning", N_("YCbCr Positioning"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 531, 0x213. Position of chrominance vs. "
        "luminance components: 1 = centered; 2 = co-sited.")},
    {"XResolution", N_("X Resolution"), "Rational", xmpText, xmpInternal,
     N_("TIFF tag 282, 0x11A. Horizontal resolution in pixels per unit.")},
    {"YResolution", N_("Y Resolution"), "Rational", xmpText, xmpInternal,
     N_("TIFF tag 283, 0x11B. Vertical resolution in pixels per unit.")},
    {"ResolutionUnit", N_("Resolution Unit"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("TIFF tag 296, 0x128. Unit used for XResolution and "
        "YResolution. Value is one of: 2 = inches; 3 = centimeters.")},
    {"TransferFunction", N_("Transfer Function"), "seq Integer", xmpSeq, xmpInternal,
     N_("TIFF tag 301, 0x12D. Transfer function for image "
        "described in tabular style with 3 * 256 entries.")},
    {"WhitePoint", N_("White Point"), "seq Rational", xmpSeq, xmpInternal,
     N_("TIFF tag 318, 0x13E. Chromaticity of white point.")},
    {"PrimaryChromaticities", N_("Primary Chromaticities"), "seq Rational", xmpSeq, xmpInternal,
     N_("TIFF tag 319, 0x13F. Chromaticity of the three primary colors.")},
    {"YCbCrCoefficients", N_("YCbCr Coefficients"), "seq Rational", xmpSeq, xmpInternal,
     N_("TIFF tag 529, 0x211. Matrix coefficients for RGB to YCbCr transformation.")},
    {"ReferenceBlackWhite", N_("Reference Black White"), "seq Rational", xmpSeq, xmpInternal,
     N_("TIFF tag 532, 0x214. Reference black and white point values.")},
    {"DateTime", N_("Date and Time"), "Date", xmpText, xmpInternal,
     N_("TIFF tag 306, 0x132 (primary) and EXIF tag 37520, "
        "0x9290 (subseconds). Date and time of image creation "
        "(no time zone in EXIF), stored in ISO 8601 format, not "
        "the original EXIF format. This property includes the "
        "value for the EXIF SubSecTime attribute. "
        "NOTE: This property is stored in XMP as xmp:ModifyDate.")},
    {"ImageDescription", N_("Image Description"), "Lang Alt", langAlt, xmpExternal,
     N_("TIFF tag 270, 0x10E. Description of the image. Note: This property is stored in XMP as dc:description.")},
    {"Make", N_("Make"), "ProperName", xmpText, xmpInternal,
     N_("TIFF tag 271, 0x10F. Manufacturer of recording equipment.")},
    {"Model", N_("Model"), "ProperName", xmpText, xmpInternal,
     N_("TIFF tag 272, 0x110. Model name or number of equipment.")},
    {"Software", N_("Software"), "AgentName", xmpText, xmpInternal,
     N_("TIFF tag 305, 0x131. Software or firmware used to generate image. "
        "Note: This property is stored in XMP as xmp:CreatorTool.")},
    {"Artist", N_("Artist"), "ProperName", xmpText, xmpExternal,
     N_("TIFF tag 315, 0x13B. Camera owner, photographer or image creator. "
        "Note: This property is stored in XMP as the first item in the dc:creator array.")},
    {"Copyright", N_("Copyright"), "Lang Alt", langAlt, xmpExternal,
     N_("TIFF tag 33432, 0x8298. Copyright information. "
        "Note: This property is stored in XMP as dc:rights.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpExifInfo[] = {
    {"ExifVersion", N_("Exif Version"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("EXIF tag 36864, 0x9000. EXIF version number.")},
    {"FlashpixVersion", N_("Flashpix Version"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("EXIF tag 40960, 0xA000. Version of FlashPix.")},
    {"ColorSpace", N_("Color Space"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 40961, 0xA001. Color space information")},
    {"ComponentsConfiguration", N_("Components Configuration"), "Closed Choice of seq Integer", xmpSeq, xmpInternal,
     N_("EXIF tag 37121, 0x9101. Configuration of components in data: 4 5 6 0 (if RGB compressed data), "
        "1 2 3 0 (other cases).")},
    {"CompressedBitsPerPixel", N_("Compressed Bits Per Pixel"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37122, 0x9102. Compression mode used for a compressed image is indicated "
        "in unit bits per pixel.")},
    {"PixelXDimension", N_("Pixel X Dimension"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 40962, 0xA002. Valid image width, in pixels.")},
    {"PixelYDimension", N_("Pixel Y Dimension"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 40963, 0xA003. Valid image height, in pixels.")},
    {"UserComment", N_("User Comment"), "Lang Alt", langAlt, xmpExternal,
     N_("EXIF tag 37510, 0x9286. Comments from user.")},
    {"RelatedSoundFile", N_("Related Sound File"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 40964, 0xA004. An \"8.3\" file name for the related sound file.")},
    {"DateTimeOriginal", N_("Date and Time Original"), "Date", xmpText, xmpInternal,
     N_("EXIF tags 36867, 0x9003 (primary) and 37521, 0x9291 (subseconds). "
        "Date and time when original image was generated, in ISO 8601 format. "
        "Includes the EXIF SubSecTimeOriginal data.")},
    {"DateTimeDigitized", N_("Date and Time Digitized"), "Date", xmpText, xmpInternal,
     N_("EXIF tag 36868, 0x9004 (primary) and 37522, 0x9292 (subseconds). Date and time when "
        "image was stored as digital data, can be the same as DateTimeOriginal if originally "
        "stored in digital form. Stored in ISO 8601 format. Includes the EXIF "
        "SubSecTimeDigitized data.")},
    {"ExposureTime", N_("Exposure Time"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 33434, 0x829A. Exposure time in seconds.")},
    {"FNumber", N_("F Number"), "Rational", xmpText, xmpInternal, N_("EXIF tag 33437, 0x829D. F number.")},
    {"ExposureProgram", N_("Exposure Program"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 34850, 0x8822. Class of program used for exposure.")},
    {"SpectralSensitivity", N_("Spectral Sensitivity"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 34852, 0x8824. Spectral sensitivity of each channel.")},
    {"ISOSpeedRatings", N_("ISOSpeedRatings"), "seq Integer", xmpSeq, xmpInternal,
     N_("EXIF tag 34855, 0x8827. ISO Speed and ISO Latitude of the input device as "
        "specified in ISO 12232.")},
    {"OECF", N_("OECF"), "OECF/SFR", xmpText, xmpInternal,
     N_("EXIF tag 34856, 0x8828. Opto-Electoric Conversion Function as specified in ISO 14524.")},
    {"ShutterSpeedValue", N_("Shutter Speed Value"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37377, 0x9201. Shutter speed, unit is APEX. See Annex C of the EXIF specification.")},
    {"ApertureValue", N_("Aperture Value"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37378, 0x9202. Lens aperture, unit is APEX.")},
    {"BrightnessValue", N_("Brightness Value"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37379, 0x9203. Brightness, unit is APEX.")},
    {"ExposureBiasValue", N_("Exposure Bias Value"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37380, 0x9204. Exposure bias, unit is APEX.")},
    {"MaxApertureValue", N_("Maximum Aperture Value"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37381, 0x9205. Smallest F number of lens, in APEX.")},
    {"SubjectDistance", N_("Subject Distance"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37382, 0x9206. Distance to subject, in meters.")},
    {"MeteringMode", N_("Metering Mode"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 37383, 0x9207. Metering mode.")},
    {"LightSource", N_("Light Source"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 37384, 0x9208. Light source.")},
    {"Flash", N_("Flash"), "Flash", xmpText, xmpInternal,
     N_("EXIF tag 37385, 0x9209. Strobe light (flash) source data.")},
    {"FocalLength", N_("Focal Length"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 37386, 0x920A. Focal length of the lens, in millimeters.")},
    {"SubjectArea", N_("Subject Area"), "seq Integer", xmpSeq, xmpInternal,
     N_("EXIF tag 37396, 0x9214. The location and area of the main subject in the overall scene.")},
    {"FlashEnergy", N_("Flash Energy"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 41483, 0xA20B. Strobe energy during image capture.")},
    {"SpatialFrequencyResponse", N_("Spatial Frequency Response"), "OECF/SFR", xmpText, xmpInternal,
     N_("EXIF tag 41484, 0xA20C. Input device spatial frequency table and SFR values as "
        "specified in ISO 12233.")},
    {"FocalPlaneXResolution", N_("Focal Plane X Resolution"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 41486, 0xA20E. Horizontal focal resolution, measured pixels per unit.")},
    {"FocalPlaneYResolution", N_("Focal Plane Y Resolution"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 41487, 0xA20F. Vertical focal resolution, measured in pixels per unit.")},
    {"FocalPlaneResolutionUnit", N_("Focal Plane Resolution Unit"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41488, 0xA210. Unit used for FocalPlaneXResolution and FocalPlaneYResolution.")},
    {"SubjectLocation", N_("Subject Location"), "seq Integer", xmpSeq, xmpInternal,
     N_("EXIF tag 41492, 0xA214. Location of the main subject of the scene. The first value is the "
        "horizontal pixel and the second value is the vertical pixel at which the "
        "main subject appears.")},
    {"ExposureIndex", N_("Exposure Index"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 41493, 0xA215. Exposure index of input device.")},
    {"SensingMethod", N_("Sensing Method"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41495, 0xA217. Image sensor type on input device.")},
    {"FileSource", N_("File Source"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41728, 0xA300. Indicates image source.")},
    {"SceneType", N_("Scene Type"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41729, 0xA301. Indicates the type of scene.")},
    {"CFAPattern", N_("CFA Pattern"), "CFAPattern", xmpText, xmpInternal,
     N_("EXIF tag 41730, 0xA302. Color filter array geometric pattern of the image sense.")},
    {"CustomRendered", N_("Custom Rendered"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41985, 0xA401. Indicates the use of special processing on image data.")},
    {"ExposureMode", N_("Exposure Mode"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41986, 0xA402. Indicates the exposure mode set when the image was shot.")},
    {"WhiteBalance", N_("White Balance"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41987, 0xA403. Indicates the white balance mode set when the image was shot.")},
    {"DigitalZoomRatio", N_("Digital Zoom Ratio"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 41988, 0xA404. Indicates the digital zoom ratio when the image was shot.")},
    {"FocalLengthIn35mmFilm", N_("Focal Length In 35mm Film"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 41989, 0xA405. Indicates the equivalent focal length assuming a 35mm film "
        "camera, in mm. A value of 0 means the focal length is unknown. Note that this tag "
        "differs from the FocalLength tag.")},
    {"SceneCaptureType", N_("Scene Capture Type"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41990, 0xA406. Indicates the type of scene that was shot.")},
    {"GainControl", N_("Gain Control"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41991, 0xA407. Indicates the degree of overall image gain adjustment.")},
    {"Contrast", N_("Contrast"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41992, 0xA408. Indicates the direction of contrast processing applied by the camera.")},
    {"Saturation", N_("Saturation"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41993, 0xA409. Indicates the direction of saturation processing applied by the camera.")},
    {"Sharpness", N_("Sharpness"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41994, 0xA40A. Indicates the direction of sharpness processing applied by the camera.")},
    {"DeviceSettingDescription", N_("Device Setting Description"), "DeviceSettings", xmpText, xmpInternal,
     N_("EXIF tag 41995, 0xA40B. Indicates information on the picture-taking conditions of a particular camera "
        "model.")},
    {"SubjectDistanceRange", N_("Subject Distance Range"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 41996, 0xA40C. Indicates the distance to the subject.")},
    {"ImageUniqueID", N_("Image Unique ID"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 42016, 0xA420. An identifier assigned uniquely to each image. It is recorded as a 32 "
        "character ASCII string, equivalent to hexadecimal notation and 128-bit fixed length.")},
    {"GPSVersionID", N_("GPS Version ID"), "Text", xmpText, xmpInternal,
     N_("GPS tag 0, 0x00. A decimal encoding of each of the four EXIF bytes with period separators. "
        "The current value is \"2.0.0.0\".")},
    {"GPSLatitude", N_("GPS Latitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("GPS tag 2, 0x02 (position) and 1, 0x01 (North/South). Indicates latitude.")},
    {"GPSLongitude", N_("GPS Longitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("GPS tag 4, 0x04 (position) and 3, 0x03 (East/West). Indicates longitude.")},
    {"GPSAltitudeRef", N_("GPS Altitude Reference"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("GPS tag 5, 0x05. Indicates whether the altitude is above or below sea level.")},
    {"GPSAltitude", N_("GPS Altitude"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 6, 0x06. Indicates altitude in meters.")},
    {"GPSTimeStamp", N_("GPS Time Stamp"), "Date", xmpText, xmpInternal,
     N_("GPS tag 29 (date), 0x1D, and, and GPS tag 7 (time), 0x07. Time stamp of GPS data, "
        "in Coordinated Universal Time. Note: The GPSDateStamp tag is new in EXIF 2.2. "
        "The GPS timestamp in EXIF 2.1 does not include a date. If not present, "
        "the date component for the XMP should be taken from exif:DateTimeOriginal, or if that is "
        "also lacking from exif:DateTimeDigitized. If no date is available, do not write "
        "exif:GPSTimeStamp to XMP.")},
    {"GPSSatellites", N_("GPS Satellites"), "Text", xmpText, xmpInternal,
     N_("GPS tag 8, 0x08. Satellite information, format is unspecified.")},
    {"GPSStatus", N_("GPS Status"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("GPS tag 9, 0x09. Status of GPS receiver at image creation time.")},
    {"GPSMeasureMode", N_("GPS Measure Mode"), "Text", xmpText, xmpInternal,
     N_("GPS tag 10, 0x0A. GPS measurement mode, Text type.")},
    {"GPSDOP", N_("GPS DOP"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 11, 0x0B. Degree of precision for GPS data.")},
    {"GPSSpeedRef", N_("GPS Speed Reference"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("GPS tag 12, 0x0C. Units used to speed measurement.")},
    {"GPSSpeed", N_("GPS Speed"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 13, 0x0D. Speed of GPS receiver movement.")},
    {"GPSTrackRef", N_("GPS Track Reference"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("GPS tag 14, 0x0E. Reference for movement direction.")},
    {"GPSTrack", N_("GPS Track"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 15, 0x0F. Direction of GPS movement, values range from 0 to 359.99.")},
    {"GPSImgDirectionRef", N_("GPS Image Direction Reference"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("GPS tag 16, 0x10. Reference for image direction.")},
    {"GPSImgDirection", N_("GPS Image Direction"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 17, 0x11. Direction of image when captured, values range from 0 to 359.99.")},
    {"GPSMapDatum", N_("GPS Map Datum"), "Text", xmpText, xmpInternal, N_("GPS tag 18, 0x12. Geodetic survey data.")},
    {"GPSDestLatitude", N_("GPS Destination Latitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("GPS tag 20, 0x14 (position) and 19, 0x13 (North/South). Indicates destination latitude.")},
    {"GPSDestLongitude", N_("GPS Destination Longitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("GPS tag 22, 0x16 (position) and 21, 0x15 (East/West). Indicates destination longitude.")},
    {"GPSDestBearingRef", N_("GPS Destination Bearing Reference"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("GPS tag 23, 0x17. Reference for movement direction.")},
    {"GPSDestBearing", N_("GPS Destination Bearing"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 24, 0x18. Destination bearing, values from 0 to 359.99.")},
    {"GPSDestDistanceRef", N_("GPS Destination Distance Reference"), "Closed Choice  of Text", xmpText, xmpInternal,
     N_("GPS tag 25, 0x19. Units used for speed measurement.")},
    {"GPSDestDistance", N_("GPS Destination Distance"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 26, 0x1A. Distance to destination.")},
    {"GPSProcessingMethod", N_("GPS Processing Method"), "Text", xmpText, xmpInternal,
     N_("GPS tag 27, 0x1B. A character string recording the name of the method used for location finding.")},
    {"GPSAreaInformation", N_("GPS Area Information"), "Text", xmpText, xmpInternal,
     N_("GPS tag 28, 0x1C. A character string recording the name of the GPS area.")},
    {"GPSDifferential", N_("GPS Differential"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("GPS tag 30, 0x1E. Indicates whether differential correction is applied to the GPS receiver.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpExifEXInfo[] = {
    {"Gamma", N_("Gamma"), "Rational", xmpText, xmpInternal,
     N_("EXIF tag 42240, 0xA500. Indicates the value of coefficient gamma.")},
    {"PhotographicSensitivity", N_("Photographic Sensitivity"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34855, 0x8827. Indicates the sensitivity of the camera or input device when the image was shot "
        "up to the value of 65535 with one of the following parameters that are defined in ISO 12232: standard "
        "output sensitivity (SOS), recommended exposure index (REI), or ISO speed.")},
    {"SensitivityType", N_("Sensitivity Type"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("EXIF tag 34864, 0x8830. Indicates which one of the parameters of ISO12232 is used for "
        "PhotographicSensitivity:0 = Unknown "
        "1 = Standard output sensitivity (SOS) "
        "2 = Recommended exposure index (REI) "
        "3 = ISO speed "
        "4 = Standard output sensitivity (SOS) and recommended exposure index (REI) "
        "5 = Standard output sensitivity (SOS) and ISO speed "
        "6 = Recommended exposure index (REI) and ISO speed "
        "7 = Standard output sensitivity (SOS) and recommended exposure index (REI) and ISO speed")},
    {"StandardOutput-Sensitivity", N_("Standard Output Sensitivity"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34865, 0x8831. Indicates the standard output sensitivity value of a camera or input device "
        "defined in ISO 12232.")},
    {"RecommendedExposureIndex", N_("Recommended Exposure Index"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34866, 0x8832. Indicates the recommended exposure index value of a camera or input device "
        "defined in ISO 12232.")},
    {"ISOSpeed", N_("ISO Speed"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34867, 0x8833. Indicates the ISO speed value of a camera or input device defined in ISO 12232.")},
    {"ISOSpeedLatitudeyyy", N_("ISO Speed Latitude yyy"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34868, 0x8834. Indicates the ISO speed latitude yyy value of a camera or input device defined in "
        "ISO 12232.")},
    {"ISOSpeedLatitudezzz", N_("ISO Speed Latitude zzz"), "Integer", xmpText, xmpInternal,
     N_("EXIF tag 34869, 0x8835. Indicates the ISO speed latitude zzz value of a camera or input device defined in "
        "ISO 12232.")},
    {"CameraOwnerName", N_("Camera Owner Name"), "Proper-Name", xmpText, xmpInternal,
     N_("EXIF tag 42032, 0xA430. This tag records the owner of a camera used in photography as an ASCII string.")},
    {"BodySerialNumber", N_("Body Serial Number"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 42033, 0xA431. The serial number of the camera or camera body used to take the photograph.")},
    {"LensSpecification", N_("Lens Specification"), "Ordered array of Rational", xmpText, xmpInternal,
     N_("EXIF tag 42034, 0xA432. notes minimum focal length, maximum focal length, minimum F number in the minimum "
        "focal length, and minimum F number in the maximum focal length, which are specification information for "
        "the lens that was used in photography.")},
    {"LensMake", N_("Lens Make"), "Proper-Name", xmpText, xmpInternal,
     N_("EXIF tag 42035, 0xA433. Records the lens manufacturer as an ASCII string.")},
    {"LensModel", N_("Lens Model"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 42036, 0xA434. Records the lens's model name and model number as an ASCII string.")},
    {"LensSerialNumber", N_("Lens Serial Number"), "Text", xmpText, xmpInternal,
     N_("EXIF tag 42037, 0xA435. This tag records the serial number of the interchangeable lens that was used in "
        "photography as an ASCII string.")},

    {"InteroperabilityIndex", N_("Interoperability Index"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("EXIF tag 1, 0x0001. Indicates the identification of the Interoperability rule.  "
        "R98 = Indicates a file conforming to R98 file specification of Recommended Exif Interoperability Rules "
        "(Exif R 98) or to DCF basic file stipulated by Design Rule for Camera File System (DCF). "
        "THM = Indicates a file conforming to DCF thumbnail file stipulated by Design rule for Camera File System. "
        "R03 = Indicates a file conforming to DCF Option File stipulated by Design rule for Camera File System.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpAuxInfo[] = {
    {"Lens", N_("Lens"), "Text", xmpText, xmpInternal,
     N_("A description of the lens used to take the photograph. For example, \"70-200 mm f/2.8-4.0\".")},
    {"SerialNumber", N_("Serial Number"), "Text", xmpText, xmpInternal,
     N_("The serial number of the camera or camera body used to take the photograph.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpIptcInfo[] = {
    {"AltTextAccessibility", N_("Alternative Text (Accessibility)"), "Lang Alt", langAlt, xmpExternal,
     N_("A brief textual description of the purpose and meaning of an image that can be accessed by assistive "
        "technology or displayed when the image is disabled in the browser. It should not exceed 250 characters.")},
    {"CreatorContactInfo", N_("Creator's Contact Info"), "ContactInfo", xmpText, xmpExternal,
     N_("The creator's contact information provides all necessary information to get in contact "
        "with the creator of this image and comprises a set of sub-properties for proper addressing.")},
    {"CiAdrExtadr", N_("Contact Info: Address (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiAdrExtadr tag in ContactInfo struct instead. sub-key Creator Contact Info: address. "
        "Comprises an optional company name and all required "
        "information to locate the building or postbox to which mail should be sent.")},
    {"CiAdrCity", N_("Contact Info-City (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiAdrCity tag in ContactInfo struct instead. sub-key Creator Contact Info: The "
        "contact information city part.")},
    {"CiAdrRegion", N_("Contact Info-State/Province (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiAdrRegion tag in ContactInfo struct instead. sub-key Creator Contact Info: state or "
        "province.")},
    {"CiAdrPcode", N_("Contact Info-Postal Code (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiAdrPcode tag in ContactInfo struct instead. sub-key Creator Contact Info: local "
        "postal code.")},
    {"CiAdrCtry", N_("Contact Info-Country (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiAdrCtry tag in ContactInfo struct instead. sub-key Creator Contact Info: country.")},
    {"CiEmailWork", N_("Contact Info-Email (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiEmailWork tag in ContactInfo struct instead. sub-key Creator Contact Info: email "
        "address.")},
    {"CiTelWork", N_("Contact Info-Phone (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiTelWork tag in ContactInfo struct instead. sub-key Creator Contact Info: phone "
        "number.")},
    {"CiUrlWork", N_("Contact Info-Web URL (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CiUrlWork tag in ContactInfo struct instead. sub-key Creator Contact Info: web "
        "address.")},
    {"ExtDescrAccessibility", N_("Extended Description (Accessibility)"), "Lang Alt", langAlt, xmpExternal,
     N_("A more detailed textual description of the purpose and meaning of an image that elaborates on the "
        "information provided by the Alt Text (Accessibility) property. This property does not have a "
        "character limitation and is not required if the Alt Text (Accessibility) field sufficiently describes the "
        "image.")},
    {"IntellectualGenre", N_("Intellectual Genre"), "Text", xmpText, xmpExternal,
     N_("Describes the nature, intellectual, artistic or journalistic characteristic of an image.")},
    {"Scene", N_("IPTC Scene Code"), "bag closed Choice of Text", xmpBag, xmpExternal,
     N_("Describes the scene of a photo content. Specifies one or more terms from the IPTC "
        "\"Scene-NewsCodes\". Each Scene is represented as a string of 6 digits in an unordered list.")},
    {"SubjectCode", N_("IPTC Subject Code"), "bag closed Choice of Text", xmpBag, xmpExternal,
     N_("Specifies one or more Subjects from the IPTC \"Subject-NewsCodes\" taxonomy to "
        "categorize the image. Each Subject is represented as a string of 8 digits in an unordered list.")},
    {"Location", N_("Sublocation (legacy)"), "Text", xmpText, xmpExternal,
     N_("(legacy) Exact name of the sublocation shown in the image. This sublocation name could either be the name "
        "of a sublocation to a city or the name of a well known location or (natural) monument "
        "outside a city. In the sense of a sublocation to a city this element is at the fourth "
        "level of a top-down geographical hierarchy.")},
    {"CountryCode", N_("Country Code (legacy)"), "closed Choice of Text", xmpText, xmpExternal,
     N_("(legacy) Code of the country of the location shown in the image. This element is at the top/first level "
        "of "
        "a top-down geographical hierarchy. The code should be taken from ISO 3166 two or three "
        "letter code. The full name of a country should go in the \"Country\" element.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpIptcExtInfo[] = {
    {"AddlModelInfo", N_("Additional model information"), "Text", xmpText, xmpExternal,
     N_("Information about the ethnicity and other facets of the model(s) in a model-released image.")},
    {"OrganisationInImageCode", N_("Code of featured Organisation"), "bag Text", xmpBag, xmpExternal,
     N_("Code from a controlled vocabulary for identifying the organisation or company which is featured in the "
        "image.")},
    {"AboutCvTerm", N_("CV-Term About Image"), "bag CVTerm", xmpBag, xmpExternal,
     N_("One or more topics, themes or entities the content is about, each one expressed by a term from a "
        "Controlled Vocabulary.")},
    {"CVterm", N_("Controlled Vocabulary Term"), "bag URI", xmpBag, xmpExternal,
     N_("A term to describe the content of the image by a value from a Controlled Vocabulary.")},
    {"ModelAge", N_("Model age"), "bag Integer", xmpBag, xmpExternal,
     N_("Age of the human model(s) at the time this image was taken in a model released image.")},
    {"OrganisationInImageName", N_("Name of featured Organisation"), "bag Text", xmpBag, xmpExternal,
     N_("Name of the organisation or company which is featured in the image.")},
    {"PersonInImage", N_("Person shown"), "bag Text", xmpBag, xmpExternal, N_("Name of a person shown in the image.")},
    {"PersonInImageWDetails", N_("Person Shown (Details)"), "bag Person", xmpBag, xmpExternal,
     N_("Details about a person the content is about.")},
    {"ProductInImage", N_("Product Shown"), "bag Product", xmpBag, xmpExternal, N_("A product the content is about.")},
    {"PropertyReleaseID", N_("Property Release Id"), "bag Text", xmpBag, xmpExternal,
     N_("Optional identifier associated with each Property Release.")},
    {"DigImageGUID", N_("Digital Image Identifier"), "Text", xmpText, xmpInternal,
     N_("Globally unique identifier for this digital image. It is created and applied by the creator of the "
        "digital image at the time of its creation. This value shall not be changed after that time.")},
    {"DigitalSourcefileType", N_("Physical type of original photo (deprecated)"), "URI", xmpText, xmpExternal,
     N_("Deprecated, use the DigitalSourceType tag instead. The type of the source digital file.")},
    {"DigitalSourceType", N_("Digital Source Type"), "URI", xmpText, xmpExternal,
     N_("The type of the source of this digital image.")},
    {"Event", N_("Event Name"), "Lang Alt", langAlt, xmpExternal,
     N_("Names or describes the specific event at which the photo was taken.")},
    {"EventId", N_("Event Identifier"), "bag Text", xmpBag, xmpExternal,
     N_("Identifier(s) of the specific event at which the photo was taken.")},
    {"EmbdEncRightsExpr", N_("Embedded Encoded Rights Expression"), "bag EERE", xmpBag, xmpInternal,
     N_("An embedded rights expression using any rights expression language.")},
    {"Genre", N_("Genre"), "bag CVTerm", xmpBag, xmpExternal,
     N_("Artistic, style, journalistic, product or other genre(s) of the image (expressed by a term "
        "from any Controlled Vocabulary).")},
    {"ImageRegion", N_("Image Region(s)"), "bag ImageRegion", xmpBag, xmpExternal,
     N_("Sets a region inside an image by defining its boundaries. All pixels of the boundary are "
        "also part of the region. It may include metadata related to this region.")},
    {"LinkedEncRightsExpr", N_("Linked Encoded Rights Expression"), "bag LERE", xmpBag, xmpExternal,
     N_("A linked rights expression using any rights expression language.")},
    {"MaxAvailHeight", N_("Maximum available height"), "Integer", xmpText, xmpExternal,
     N_("The maximum available height in pixels of the original photo from which this photo has been derived by "
        "downsizing.")},
    {"MaxAvailWidth", N_("Maximum available width"), "Integer", xmpText, xmpExternal,
     N_("The maximum available width in pixels of the original photo from which this photo has been derived by "
        "downsizing.")},
    {"RegistryId", N_("Registry Entry"), "bag RegistryEntryDetails", xmpBag, xmpExternal,
     N_("Both a Registry Item Id and a Registry Organisation Id to record any registration of this digital image "
        "with a registry.")},
    {"RegItemId", N_("Registry Entry-Item Identifier (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the RegItemId tag in Product struct instead. A unique identifier created by a registry "
        "and applied by the creator of the digital image. This value shall not be changed after being applied. "
        "This identifier is linked to a corresponding Registry Organisation Identifier.")},
    {"RegOrgId", N_("Registry Entry-Organisation Identifier (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the RegOrgId tag in RegistryEntryDetails struct instead. An identifier for the registry "
        "which issued the corresponding Registry Image Id.")},
    {"IptcLastEdited", N_("IPTC Fields Last Edited"), "Date", xmpText, xmpExternal,
     N_("The date and optionally time when any of the IPTC photo metadata fields has been last edited.")},
    {"LocationShown", N_("Location Shown"), "bag LocationDetails", xmpBag, xmpExternal,
     N_("A location shown in the image.")},
    {"LocationCreated", N_("Location Created"), "bag LocationDetails", xmpBag, xmpExternal,
     N_("The location the photo was taken.")},
    {"City", N_("Location-City (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the City tag in LocationDetails struct instead. Name of the city of a location.")},
    {"CountryCode", N_("Location-Country ISO-Code (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CountryCode tag in LocationDetails struct instead. The ISO code of a country of a "
        "location.")},
    {"CountryName", N_("Location-Country Name (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the CountryName tag in LocationDetails struct instead. The name of a country of a "
        "location.")},
    {"ProvinceState", N_("Location-Province/State (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the ProvinceState tag in LocationDetails struct instead. The name of a subregion of a "
        "country - a province or state - of a location.")},
    {"Sublocation", N_("Location-Sublocation (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the City tag in LocationDetails struct instead. Name of a sublocation. This sublocation "
        "name could either be the name of a sublocation to a city or the name of a well known location or "
        "(natural) monument outside a city.")},
    {"WorldRegion", N_("Location-World Region (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the WorldRegion tag in LocationDetails struct instead. The name of a world region of a "
        "location.")},
    {"ArtworkOrObject", N_("Artwork or object in the image"), "bag ArtworkOrObjectDetails", xmpBag, xmpExternal,
     N_("A set of metadata about artwork or an object in the image.")},
    {"AOCopyrightNotice", N_("Artwork or object-Copyright notice (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the AOCopyrightNotice tag in ArtworkOrObjectDetails struct instead. Contains any "
        "necessary copyright notice for claiming the intellectual property for artwork or an object in the image "
        "and should identify the current owner of the copyright of this work with associated intellectual property "
        "rights.")},
    {"AOCreator", N_("Artwork or object-Creator (deprecated)"), "seq ProperName", xmpSeq, xmpExternal,
     N_("Deprecated, use the AOCreator tag in ArtworkOrObjectDetails struct instead. Contains the name of the "
        "artist who has created artwork or an object in the image. In cases where the artist could or should not "
        "be identified the name of a company or organisation may be appropriate.")},
    {"AODateCreated", N_("Artwork or object-Date Created (deprecated)"), "Date", xmpText, xmpExternal,
     N_("Deprecated, use the AODateCreated tag in ArtworkOrObjectDetails struct instead. Designates the date and "
        "optionally the time the artwork or object in the image was created. This relates to artwork or objects "
        "with associated intellectual property rights.")},
    {"AOSource", N_("Artwork or object-Source (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the AOSource tag in ArtworkOrObjectDetails struct instead. The organisation or body "
        "holding and registering the artwork or object in the image for inventory purposes.")},
    {"AOSourceInvNo", N_("Artwork or object-Source inventory number (deprecated)"), "Text", xmpText, xmpExternal,
     N_("Deprecated, use the AOSourceInvNo tag in ArtworkOrObjectDetails struct instead. The inventory number "
        "issued by the organisation or body holding and registering the artwork or object in the image.")},
    {"AOTitle", N_("Artwork or object-Title (deprecated)"), "Lang Alt", langAlt, xmpExternal,
     N_("Deprecated, use the AOTitle tag in ArtworkOrObjectDetails struct instead. A reference for the artwork or "
        "object in the image.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

//! XMP Iptc4xmpCore:Scene
//  Source: http://cv.iptc.org/newscodes/scene/
constexpr TagVocabulary iptcScene[] = {
    {"010100", N_("Headshot")},       {"010200", N_("Half-length")},
    {"010300", N_("Full-length")},    {"010400", N_("Profile")},
    {"010500", N_("rear view")},      {"010600", N_("Single")},
    {"010700", N_("Couple")},         {"010800", N_("Two")},
    {"010900", N_("Group")},          {"011000", N_("General view")},
    {"011100", N_("Panoramic view")}, {"011200", N_("Aerial view")},
    {"011300", N_("Under-water")},    {"011400", N_("Night scene")},
    {"011500", N_("Satellite")},      {"011600", N_("Exterior view")},
    {"011700", N_("Interior view")},  {"011800", N_("Close-up")},
    {"011900", N_("Action")},         {"012000", N_("Performing")},
    {"012100", N_("Posing")},         {"012200", N_("Symbolic")},
    {"012300", N_("Off-beat")},       {"012400", N_("Movie scene")},
};

//! XMP Iptc4xmpCore:SubjectCode
//  Source: https://cv.iptc.org/newscodes/subjectcode/
constexpr TagVocabulary iptcSubjectCode[] = {
    {"01000000", "arts, culture and entertainment"},
    {"01001000", "archaeology"},
    {"01002000", "architecture"},
    {"01003000", "bullfighting"},
    {"01004000", "festive event (including carnival)"},
    {"01005000", "cinema"},
    {"01005001", "film festival"},
    {"01006000", "dance"},
    {"01007000", "fashion"},
    {"01007001", "jewelry"},
    {"01008000", "language"},
    {"01009000", "library and museum"},
    {"01010000", "literature"},
    {"01010001", "fiction"},
    {"01010002", "poetry"},
    {"01011000", "music"},
    {"01011001", "classical music"},
    {"01011002", "folk music"},
    {"01011003", "jazz music"},
    {"01011004", "popular music"},
    {"01011005", "country music"},
    {"01011006", "rock and roll music"},
    {"01011007", "hip-hop"},
    {"01012000", "painting"},
    {"01013000", "photography"},
    {"01014000", "radio"},
    {"01015000", "sculpture"},
    {"01015001", "plastic art"},
    {"01016000", "television"},
    {"01016001", "soap opera"},
    {"01017000", "theatre"},
    {"01017001", "music theatre"},
    {"01018000", "monument and heritage site"},
    {"01018001", "institution-DEPRECATED"},
    {"01019000", "customs and tradition"},
    {"01020000", "arts (general)"},
    {"01021000", "entertainment (general)"},
    {"01021001", "entertainment award"},
    {"01022000", "culture (general)"},
    {"01022001", "cultural development"},
    {"01023000", "nightclub"},
    {"01024000", "cartoon"},
    {"01025000", "animation"},
    {"01026000", "mass media"},
    {"01026001", "periodicals"},
    {"01026002", "news media"},
    {"01026003", "newspapers"},
    {"01026004", "reviews"},
    {"01027000", "internet"},
    {"01028000", "history"},
    {"02000000", "crime, law and justice"},
    {"02001000", "crime"},
    {"02001001", "homicide"},
    {"02001002", "computer crime"},
    {"02001003", "theft"},
    {"02001004", "drug trafficking"},
    {"02001005", "sexual assault"},
    {"02001006", "assault (general)"},
    {"02001007", "kidnapping"},
    {"02001008", "arson"},
    {"02001009", "gang activity"},
    {"02001010", "terrorism"},
    {"02002000", "judiciary (system of justice)"},
    {"02002001", "lawyer"},
    {"02002002", "judge"},
    {"02002003", "court administration"},
    {"02003000", "police"},
    {"02003001", "law enforcement"},
    {"02003002", "investigation"},
    {"02003003", "arrest"},
    {"02004000", "punishment"},
    {"02004001", "fine"},
    {"02004002", "execution"},
    {"02005000", "prison"},
    {"02006000", "laws"},
    {"02006001", "criminal"},
    {"02006002", "civil"},
    {"02007000", "justice and rights"},
    {"02007001", "civil rights"},
    {"02008000", "trials"},
    {"02008001", "litigation"},
    {"02008002", "arbitration"},
    {"02008003", "court preliminary"},
    {"02009000", "prosecution"},
    {"02009001", "defendant"},
    {"02009002", "witness"},
    {"02010000", "organized crime"},
    {"02011000", "international law"},
    {"02011001", "international court or tribunal"},
    {"02011002", "extradition"},
    {"02012000", "corporate crime"},
    {"02012001", "fraud"},
    {"02012002", "embezzlement"},
    {"02012003", "restraint of trade"},
    {"02012004", "breach of contract"},
    {"02012005", "anti-trust crime"},
    {"02012006", "corruption"},
    {"02012007", "bribery"},
    {"02013000", "war crime"},
    {"02014000", "inquest"},
    {"02015000", "inquiry"},
    {"02016000", "tribunal"},
    {"03000000", "disaster and accident"},
    {"03001000", "drought"},
    {"03002000", "earthquake"},
    {"03003000", "famine"},
    {"03004000", "fire"},
    {"03005000", "flood"},
    {"03006000", "industrial accident"},
    {"03006001", "structural failures"},
    {"03007000", "meteorological disaster"},
    {"03007001", "windstorms"},
    {"03008000", "nuclear accident"},
    {"03009000", "pollution"},
    {"03010000", "transport accident"},
    {"03010001", "road accident"},
    {"03010002", "railway accident"},
    {"03010003", "air and space accident"},
    {"03010004", "maritime accident"},
    {"03011000", "volcanic eruption"},
    {"03012000", "relief and aid organisation"},
    {"03013000", "accident (general)"},
    {"03014000", "emergency incident"},
    {"03014001", "explosion"},
    {"03015000", "disaster (general)"},
    {"03015001", "natural disasters"},
    {"03015002", "avalanche/landslide"},
    {"03016000", "emergency planning"},
    {"03017000", "rescue"},
    {"04000000", "economy, business and finance"},
    {"04001000", "agriculture"},
    {"04001001", "arable farming"},
    {"04001002", "fishing industry"},
    {"04001003", "forestry and timber"},
    {"04001004", "livestock farming"},
    {"04001005", "viniculture"},
    {"04001006", "aquaculture"},
    {"04002000", "chemicals"},
    {"04002001", "biotechnology"},
    {"04002002", "fertiliser"},
    {"04002003", "health and beauty product"},
    {"04002004", "inorganic chemical"},
    {"04002005", "organic chemical"},
    {"04002006", "pharmaceutical"},
    {"04002007", "synthetic and plastic"},
    {"04003000", "computing and information technology"},
    {"04003001", "hardware"},
    {"04003002", "networking"},
    {"04003003", "satellite technology"},
    {"04003004", "semiconductors and active components"},
    {"04003005", "software"},
    {"04003006", "telecommunication equipment"},
    {"04003007", "telecommunication service"},
    {"04003008", "security"},
    {"04003009", "wireless technology"},
    {"04004000", "construction and property"},
    {"04004001", "heavy construction"},
    {"04004002", "house building"},
    {"04004003", "real estate"},
    {"04004004", "farms"},
    {"04004005", "land price"},
    {"04004006", "renovation"},
    {"04004007", "design and engineering"},
    {"04005000", "energy and resource"},
    {"04005001", "alternative energy"},
    {"04005002", "coal"},
    {"04005003", "oil and gas - downstream activities"},
    {"04005004", "oil and gas - upstream activities"},
    {"04005005", "nuclear power"},
    {"04005006", "electricity production and distribution"},
    {"04005007", "waste management and pollution control"},
    {"04005008", "water supply"},
    {"04005009", "natural resources (general)"},
    {"04005010", "energy (general)"},
    {"04005011", "natural gas"},
    {"04005012", "petrol"},
    {"04005013", "diesel fuel"},
    {"04005014", "kerosene/paraffin"},
    {"04006000", "financial and business service"},
    {"04006001", "accountancy and auditing"},
    {"04006002", "banking"},
    {"04006003", "consultancy service"},
    {"04006004", "employment agency"},
    {"04006005", "healthcare provider"},
    {"04006006", "insurance"},
    {"04006007", "legal service"},
    {"04006008", "market research"},
    {"04006009", "stock broking"},
    {"04006010", "personal investing"},
    {"04006011", "market trend"},
    {"04006012", "shipping service"},
    {"04006013", "personal service"},
    {"04006014", "janitorial service"},
    {"04006015", "funeral parlour and crematorium"},
    {"04006016", "rental service"},
    {"04006017", "wedding service"},
    {"04006018", "personal finance"},
    {"04006019", "personal income"},
    {"04006020", "auction service"},
    {"04006021", "printing/promotional service"},
    {"04006022", "investment service"},
    {"04007000", "consumer goods"},
    {"04007001", "clothing"},
    {"04007002", "department store"},
    {"04007003", "food"},
    {"04007004", "mail order"},
    {"04007005", "retail"},
    {"04007006", "speciality store"},
    {"04007007", "wholesale"},
    {"04007008", "beverage"},
    {"04007009", "electronic commerce"},
    {"04007010", "luxury good"},
    {"04007011", "non-durable good"},
    {"04007012", "toy"},
    {"04008000", "macro economics"},
    {"04008001", "central bank"},
    {"04008002", "consumer issue"},
    {"04008003", "debt market"},
    {"04008004", "economic indicator"},
    {"04008005", "emerging market"},
    {"04008006", "foreign exchange market"},
    {"04008007", "government aid"},
    {"04008008", "government debt"},
    {"04008009", "interest rate"},
    {"04008010", "international economic institution"},
    {"04008011", "international (foreign) trade"},
    {"04008012", "loan market"},
    {"04008013", "economic organization"},
    {"04008014", "consumer confidence"},
    {"04008015", "trade dispute"},
    {"04008016", "inflation and deflation"},
    {"04008017", "prices"},
    {"04008018", "currency values"},
    {"04008019", "budgets and budgeting"},
    {"04008020", "credit and debt"},
    {"04008021", "loans"},
    {"04008022", "mortgages"},
    {"04008023", "financial markets"},
    {"04008024", "commodity markets"},
    {"04008025", "investments"},
    {"04008026", "stocks"},
    {"04008027", "bonds"},
    {"04008028", "mutual funds"},
    {"04008029", "derivative securities"},
    {"04008030", "imports"},
    {"04008031", "exports"},
    {"04008032", "trade agreements"},
    {"04008033", "trade policy"},
    {"04008034", "business enterprises"},
    {"04008035", "tariff"},
    {"04008036", "trade balance"},
    {"04009000", "market and exchange"},
    {"04009001", "energy"},
    {"04009002", "metal"},
    {"04009003", "securities"},
    {"04009004", "soft commodity"},
    {"04010000", "media"},
    {"04010001", "advertising"},
    {"04010002", "book"},
    {"04010003", "cinema industry"},
    {"04010004", "news agency"},
    {"04010005", "newspaper and magazine"},
    {"04010006", "online"},
    {"04010007", "public relation"},
    {"04010008", "radio industry"},
    {"04010009", "satellite and cable service"},
    {"04010010", "television industry"},
    {"04010011", "music industry"},
    {"04011000", "manufacturing and engineering"},
    {"04011001", "aerospace"},
    {"04011002", "automotive equipment"},
    {"04011003", "defence equipment"},
    {"04011004", "electrical appliance"},
    {"04011005", "heavy engineering"},
    {"04011006", "industrial component"},
    {"04011007", "instrument engineering"},
    {"04011008", "shipbuilding"},
    {"04011009", "machine manufacturing"},
    {"04012000", "metal and mineral"},
    {"04012001", "building material"},
    {"04012002", "gold and precious material"},
    {"04012003", "iron and steel"},
    {"04012004", "non ferrous metal"},
    {"04012005", "mining"},
    {"04013000", "process industry"},
    {"04013001", "distiller and brewer"},
    {"04013002", "food"},
    {"04013003", "furnishings and furniture"},
    {"04013004", "paper and packaging product"},
    {"04013005", "rubber product"},
    {"04013006", "soft drinks"},
    {"04013007", "textile and clothing"},
    {"04013008", "tobacco"},
    {"04014000", "tourism and leisure"},
    {"04014001", "casino and gambling"},
    {"04014002", "hotel and accommodation"},
    {"04014003", "recreational and sporting goods"},
    {"04014004", "restaurant and catering"},
    {"04014005", "tour operator"},
    {"04015000", "transport"},
    {"04015001", "air transport"},
    {"04015002", "railway"},
    {"04015003", "road transport"},
    {"04015004", "waterway and maritime transport"},
    {"04016000", "company information"},
    {"04016001", "accounting and audit"},
    {"04016002", "annual and special corporate meeting"},
    {"04016003", "annual report"},
    {"04016004", "antitrust issue"},
    {"04016005", "merger, acquisition and takeover"},
    {"04016006", "analysts' comment"},
    {"04016007", "bankruptcy"},
    {"04016008", "board of directors (appointment and change)"},
    {"04016009", "buyback"},
    {"04016010", "C.E.O. interview"},
    {"04016011", "corporate officer"},
    {"04016012", "corporate profile"},
    {"04016013", "contract"},
    {"04016014", "defence contract"},
    {"04016015", "dividend announcement"},
    {"04016016", "earnings forecast"},
    {"04016017", "financially distressed company"},
    {"04016018", "earnings"},
    {"04016019", "financing and stock offering"},
    {"04016020", "government contract"},
    {"04016021", "global expansion"},
    {"04016022", "insider trading"},
    {"04016023", "joint venture"},
    {"04016024", "leveraged buyout"},
    {"04016025", "layoffs and downsizing"},
    {"04016026", "licensing agreement"},
    {"04016027", "litigation and regulation"},
    {"04016028", "management change"},
    {"04016029", "marketing"},
    {"04016030", "new product"},
    {"04016031", "patent, copyright and trademark"},
    {"04016032", "plant closing"},
    {"04016033", "plant opening"},
    {"04016034", "privatisation"},
    {"04016035", "proxy filing"},
    {"04016036", "rating"},
    {"04016037", "research and development"},
    {"04016038", "quarterly or semiannual financial statement"},
    {"04016039", "restructuring and recapitalisation"},
    {"04016040", "spin-off"},
    {"04016041", "stock activity"},
    {"04016042", "industrial production"},
    {"04016043", "productivity"},
    {"04016044", "inventories"},
    {"04016045", "sales"},
    {"04016046", "corporations"},
    {"04016047", "shareholders"},
    {"04016048", "corporate performance"},
    {"04016049", "losses"},
    {"04016050", "credit ratings"},
    {"04016051", "stock splits"},
    {"04016052", "stock options"},
    {"04016053", "recalls (products)"},
    {"04016054", "globalization"},
    {"04016055", "consumers"},
    {"04016056", "purchase"},
    {"04016057", "new service"},
    {"04017000", "economy (general)"},
    {"04017001", "economic policy"},
    {"04018000", "business (general)"},
    {"04018001", "institution"},
    {"04019000", "finance (general)"},
    {"04019001", "money and monetary policy"},
    {"05000000", "education"},
    {"05001000", "adult education"},
    {"05002000", "further education"},
    {"05003000", "parent organisation"},
    {"05004000", "preschool"},
    {"05005000", "school"},
    {"05005001", "elementary schools"},
    {"05005002", "middle schools"},
    {"05005003", "high schools"},
    {"05006000", "teachers union"},
    {"05007000", "university"},
    {"05008000", "upbringing"},
    {"05009000", "entrance examination"},
    {"05010000", "teaching and learning"},
    {"05010001", "students"},
    {"05010002", "teachers"},
    {"05010003", "curriculum"},
    {"05010004", "test/examination"},
    {"05011000", "religious education"},
    {"05011001", "parochial school"},
    {"05011002", "seminary"},
    {"05011003", "yeshiva"},
    {"05011004", "madrasa"},
    {"06000000", "environmental issue"},
    {"06001000", "renewable energy"},
    {"06002000", "conservation"},
    {"06002001", "endangered species"},
    {"06002002", "ecosystem"},
    {"06003000", "energy saving"},
    {"06004000", "environmental politics"},
    {"06005000", "environmental pollution"},
    {"06005001", "air pollution"},
    {"06005002", "water pollution"},
    {"06006000", "natural resources"},
    {"06006001", "land resources"},
    {"06006002", "parks"},
    {"06006003", "forests"},
    {"06006004", "wetlands"},
    {"06006005", "mountains"},
    {"06006006", "rivers"},
    {"06006007", "oceans"},
    {"06006008", "wildlife"},
    {"06006009", "energy resources"},
    {"06007000", "nature"},
    {"06007001", "invasive species"},
    {"06008000", "population"},
    {"06009000", "waste"},
    {"06010000", "water"},
    {"06011000", "global warming"},
    {"06012000", "hazardous materials"},
    {"06013000", "environmental cleanup"},
    {"07000000", "health"},
    {"07001000", "disease"},
    {"07001001", "communicable diseases"},
    {"07001002", "virus diseases"},
    {"07001003", "AIDS"},
    {"07001004", "cancer"},
    {"07001005", "heart disease"},
    {"07001006", "alzheimer's disease"},
    {"07001007", "animal diseases"},
    {"07001008", "plant diseases"},
    {"07001009", "retrovirus"},
    {"07002000", "epidemic and plague"},
    {"07003000", "health treatment"},
    {"07003001", "prescription drugs"},
    {"07003002", "dietary supplements"},
    {"07003003", "diet"},
    {"07003004", "medical procedure/test"},
    {"07003005", "therapy"},
    {"07004000", "health organisations"},
    {"07005000", "medical research"},
    {"07006000", "medical staff"},
    {"07006001", "primary care physician"},
    {"07006002", "health-workers union"},
    {"07007000", "medicine"},
    {"07007001", "herbal"},
    {"07007002", "holistic"},
    {"07007003", "western"},
    {"07007004", "traditional Chinese"},
    {"07008000", "preventative medicine"},
    {"07008001", "vaccines"},
    {"07009000", "injury"},
    {"07010000", "hospital and clinic"},
    {"07011000", "government health care"},
    {"07011001", "medicare"},
    {"07011002", "medicaid"},
    {"07012000", "private health care"},
    {"07013000", "healthcare policy"},
    {"07013001", "food safety"},
    {"07014000", "medical specialisation"},
    {"07014001", "geriatric"},
    {"07014002", "pediatrics"},
    {"07014003", "reproduction"},
    {"07014004", "genetics"},
    {"07014005", "obstetrics/gynecology"},
    {"07015000", "medical service"},
    {"07016000", "physical fitness"},
    {"07017000", "illness"},
    {"07017001", "mental illness"},
    {"07017002", "eating disorder"},
    {"07017003", "obesity"},
    {"07018000", "medical conditions"},
    {"07019000", "patient"},
    {"08000000", "human interest"},
    {"08001000", "animal"},
    {"08002000", "curiosity"},
    {"08003000", "people"},
    {"08003001", "advice"},
    {"08003002", "celebrity"},
    {"08003003", "accomplishment"},
    {"08003004", "human mishap"},
    {"08003005", "fortune-telling"},
    {"08004000", "mystery"},
    {"08005000", "society"},
    {"08005001", "ceremony"},
    {"08005002", "death"},
    {"08005003", "funeral"},
    {"08005004", "estate bestowal"},
    {"08005005", "memorial"},
    {"08006000", "award and prize"},
    {"08006001", "record"},
    {"08007000", "imperial and royal matters"},
    {"08008000", "plant"},
    {"09000000", "labour"},
    {"09001000", "apprentices"},
    {"09002000", "collective contract"},
    {"09002001", "contract issue-wages"},
    {"09002002", "contract issue-healthcare"},
    {"09002003", "contract issue-work rules"},
    {"09003000", "employment"},
    {"09003001", "labor market"},
    {"09003002", "job layoffs"},
    {"09003003", "child labor"},
    {"09003004", "occupations"},
    {"09004000", "labour dispute"},
    {"09005000", "labour legislation"},
    {"09006000", "retirement"},
    {"09007000", "retraining"},
    {"09008000", "strike"},
    {"09009000", "unemployment"},
    {"09010000", "unions"},
    {"09011000", "wage and pension"},
    {"09011001", "employee benefits"},
    {"09011002", "social security"},
    {"09012000", "work relations"},
    {"09013000", "health and safety at work"},
    {"09014000", "advanced training"},
    {"09015000", "employer"},
    {"09016000", "employee"},
    {"10000000", "lifestyle and leisure"},
    {"10001000", "game"},
    {"10001001", "Go"},
    {"10001002", "chess"},
    {"10001003", "bridge"},
    {"10001004", "shogi"},
    {"10002000", "gaming and lottery"},
    {"10003000", "gastronomy"},
    {"10003001", "organic foods"},
    {"10004000", "hobby"},
    {"10004001", "DIY"},
    {"10004002", "shopping"},
    {"10004003", "gardening"},
    {"10005000", "holiday or vacation"},
    {"10006000", "tourism"},
    {"10007000", "travel and commuting"},
    {"10007001", "traffic"},
    {"10008000", "club and association"},
    {"10009000", "lifestyle (house and home)"},
    {"10010000", "leisure (general)"},
    {"10011000", "public holiday"},
    {"10012000", "hunting"},
    {"10013000", "fishing"},
    {"10014000", "auto trends"},
    {"10015000", "adventure"},
    {"10016000", "beauty"},
    {"10017000", "consumer issue"},
    {"10018000", "wedding"},
    {"11000000", "politics"},
    {"11001000", "defence"},
    {"11001001", "veterans affairs"},
    {"11001002", "national security"},
    {"11001003", "security measures"},
    {"11001004", "armed Forces"},
    {"11001005", "military equipment"},
    {"11001006", "firearms"},
    {"11001007", "biological and chemical weapons"},
    {"11001008", "missile systems"},
    {"11001009", "nuclear weapons"},
    {"11002000", "diplomacy"},
    {"11002001", "summit"},
    {"11002002", "international relations"},
    {"11002003", "peace negotiations"},
    {"11002004", "alliances"},
    {"11003000", "election"},
    {"11003001", "political candidates"},
    {"11003002", "political campaigns"},
    {"11003003", "campaign finance"},
    {"11003004", "national elections"},
    {"11003005", "regional elections"},
    {"11003006", "local elections"},
    {"11003007", "voting"},
    {"11003008", "poll"},
    {"11003009", "european elections"},
    {"11003010", "primary"},
    {"11004000", "espionage and intelligence"},
    {"11005000", "foreign aid"},
    {"11005001", "economic sanction"},
    {"11006000", "government"},
    {"11006001", "civil and public service"},
    {"11006002", "safety of citizens"},
    {"11006003", "think tank"},
    {"11006004", "national government"},
    {"11006005", "executive (government)"},
    {"11006006", "heads of state"},
    {"11006007", "government departments"},
    {"11006008", "public officials"},
    {"11006009", "ministers (government)"},
    {"11006010", "public employees"},
    {"11006011", "privatisation"},
    {"11006012", "nationalisation"},
    {"11006013", "impeachment"},
    {"11007000", "human rights"},
    {"11008000", "local authority"},
    {"11009000", "parliament"},
    {"11009001", "upper house"},
    {"11009002", "lower house"},
    {"11010000", "parties and movements"},
    {"11010001", "non government organizations (NGO)"},
    {"11011000", "refugee"},
    {"11012000", "regional authority"},
    {"11013000", "state budget and tax"},
    {"11013001", "public finance"},
    {"11014000", "treaty and international organisation-DEPRECATED"},
    {"11014001", "international relations-DEPRECATED"},
    {"11014002", "peace negotiations-DEPRECATED"},
    {"11014003", "alliances-DEPRECATED"},
    {"11015000", "constitution"},
    {"11016000", "interior policy"},
    {"11016001", "data protection"},
    {"11016002", "housing and urban planning"},
    {"11016003", "pension and welfare"},
    {"11016004", "personal weapon control"},
    {"11016005", "indigenous people"},
    {"11016006", "personal data collection"},
    {"11016007", "planning inquiries"},
    {"11017000", "migration"},
    {"11018000", "citizens initiative and recall"},
    {"11019000", "referenda"},
    {"11020000", "nuclear policy"},
    {"11021000", "lobbying"},
    {"11022000", "regulatory policy and organisation"},
    {"11023000", "censorship"},
    {"11024000", "politics (general)"},
    {"11024001", "political systems"},
    {"11024002", "democracy"},
    {"11024003", "political development"},
    {"11025000", "freedom of the press"},
    {"11026000", "freedom of religion"},
    {"11027000", "treaty"},
    {"11028000", "international organisation"},
    {"12000000", "religion and belief"},
    {"12001000", "cult and sect"},
    {"12002000", "belief (faith)"},
    {"12002001", "unificationism"},
    {"12002002", "scientology"},
    {"12003000", "freemasonry"},
    {"12004000", "religion-DEPRECATED"},
    {"12004001", "christianity-DEPRECATED"},
    {"12004002", "islam-DEPRECATED"},
    {"12004003", "judaism-DEPRECATED"},
    {"12004004", "buddhism-DEPRECATED"},
    {"12004005", "hinduism-DEPRECATED"},
    {"12005000", "church (organisation)-DEPRECATED"},
    {"12005001", "religious facilities-DEPRECATED"},
    {"12006000", "values"},
    {"12006001", "ethics"},
    {"12006002", "corrupt practices"},
    {"12007000", "church and state relations"},
    {"12008000", "philosophy"},
    {"12009000", "christianity"},
    {"12009001", "protestant"},
    {"12009002", "lutheran"},
    {"12009003", "reformed"},
    {"12009004", "anglican"},
    {"12009005", "methodist"},
    {"12009006", "baptist"},
    {"12009007", "mennonite"},
    {"12009009", "mormon"},
    {"12009010", "roman catholic"},
    {"12009011", "old catholic"},
    {"12009012", "orthodoxy"},
    {"12009013", "salvation army"},
    {"12010000", "islam"},
    {"12011000", "judaism"},
    {"12012000", "buddhism"},
    {"12013000", "hinduism"},
    {"12014000", "religious festival or holiday"},
    {"12014001", "christmas"},
    {"12014002", "easter"},
    {"12014003", "pentecost"},
    {"12014004", "ramadan"},
    {"12014005", "yom kippur"},
    {"12015000", "religious leader"},
    {"12015001", "pope"},
    {"12016000", "nature religion"},
    {"12017000", "taoism"},
    {"12018000", "shintoism"},
    {"12019000", "sikhism"},
    {"12020000", "jainism"},
    {"12021000", "parsasm"},
    {"12022000", "confucianism"},
    {"12023000", "religious text"},
    {"12023001", "bible"},
    {"12023002", "qur'an"},
    {"12023003", "torah"},
    {"12024000", "interreligious dialogue"},
    {"12025000", "religious event"},
    {"12025001", "catholic convention"},
    {"12025002", "protestant convention"},
    {"12025004", "ritual"},
    {"12026000", "concordat"},
    {"12027000", "ecumenism"},
    {"13000000", "science and technology"},
    {"13001000", "applied science"},
    {"13001001", "physics"},
    {"13001002", "chemistry"},
    {"13001003", "cosmology"},
    {"13001004", "particle physics"},
    {"13002000", "engineering"},
    {"13002001", "material science"},
    {"13003000", "human science"},
    {"13003001", "social sciences"},
    {"13003002", "history"},
    {"13003003", "psychology"},
    {"13003004", "sociology"},
    {"13003005", "anthropology"},
    {"13004000", "natural science"},
    {"13004001", "geology"},
    {"13004002", "paleontology"},
    {"13004003", "geography"},
    {"13004004", "botany"},
    {"13004005", "zoology"},
    {"13004006", "physiology"},
    {"13004007", "astronomy"},
    {"13004008", "biology"},
    {"13005000", "philosophical science"},
    {"13006000", "research"},
    {"13006001", "survey"},
    {"13007000", "scientific exploration"},
    {"13008000", "space programme"},
    {"13009000", "science (general)"},
    {"13010000", "technology (general)"},
    {"13010001", "rocketry"},
    {"13010002", "laser"},
    {"13011000", "standards"},
    {"13012000", "animal science"},
    {"13013000", "micro science"},
    {"13014000", "marine science"},
    {"13015000", "weather science"},
    {"13016000", "electronics"},
    {"13017000", "identification technology"},
    {"13018000", "mathematics"},
    {"13019000", "biotechnology"},
    {"13020000", "agricultural research and technology"},
    {"13021000", "nanotechnology"},
    {"13022000", "IT/computer sciences"},
    {"13023000", "scientific institutions"},
    {"14000000", "social issue"},
    {"14001000", "addiction"},
    {"14002000", "charity"},
    {"14003000", "demographics"},
    {"14003001", "population and census"},
    {"14003002", "immigration"},
    {"14003003", "illegal immigrants"},
    {"14003004", "emigrants"},
    {"14004000", "disabled"},
    {"14005000", "euthanasia (also includes assisted suicide)"},
    {"14005001", "suicide"},
    {"14006000", "family"},
    {"14006001", "parent and child"},
    {"14006002", "adoption"},
    {"14006003", "marriage"},
    {"14006004", "divorce"},
    {"14006005", "sex"},
    {"14006006", "courtship"},
    {"14007000", "family planning"},
    {"14008000", "health insurance"},
    {"14009000", "homelessness"},
    {"14010000", "minority group"},
    {"14010001", "gays and lesbians"},
    {"14010002", "national or ethnic minority"},
    {"14011000", "pornography"},
    {"14012000", "poverty"},
    {"14013000", "prostitution"},
    {"14014000", "racism"},
    {"14015000", "welfare"},
    {"14016000", "abortion"},
    {"14017000", "missing person"},
    {"14017001", "missing due to hostilities"},
    {"14018000", "long term care"},
    {"14019000", "juvenile delinquency"},
    {"14020000", "nuclear radiation victims"},
    {"14021000", "slavery"},
    {"14022000", "abusive behaviour"},
    {"14023000", "death and dying"},
    {"14024000", "people"},
    {"14024001", "children"},
    {"14024002", "infants"},
    {"14024003", "teen-agers"},
    {"14024004", "adults"},
    {"14024005", "senior citizens"},
    {"14025000", "social issues (general)"},
    {"14025001", "social conditions"},
    {"14025002", "social problems"},
    {"14025003", "discrimination"},
    {"14025004", "social services"},
    {"14025005", "death penalty policies"},
    {"14026000", "ordnance clearance"},
    {"14027000", "reconstruction"},
    {"15000000", "sport"},
    {"15001000", "aero and aviation sport"},
    {"15001001", "parachuting"},
    {"15001002", "sky diving"},
    {"15002000", "alpine skiing"},
    {"15002001", "downhill"},
    {"15002002", "giant slalom"},
    {"15002003", "super G"},
    {"15002004", "slalom"},
    {"15002005", "combined"},
    {"15003000", "American football"},
    {"15003001", "(US) National Football League (NFL) (North American)"},
    {"15003002", "CFL"},
    {"15003003", "AFL-DEPRECATED"},
    {"15004000", "archery"},
    {"15004001", "FITA / Outdoor target archery"},
    {"15004002", "crossbow shooting"},
    {"15005000", "athletics, track and field"},
    {"15005001", "100 m"},
    {"15005002", "200 m"},
    {"15005003", "400 m"},
    {"15005004", "800 m"},
    {"15005005", "1000 m"},
    {"15005006", "1500 m"},
    {"15005007", "mile"},
    {"15005008", "2000 m"},
    {"15005009", "3000 m"},
    {"15005010", "5000 m"},
    {"15005011", "10,000 m"},
    {"15005012", "20 km"},
    {"15005013", "one hour"},
    {"15005014", "25000"},
    {"15005015", "30000"},
    {"15005016", "110 m hurdles"},
    {"15005017", "400 m hurdles"},
    {"15005018", "3000 m steeplechase"},
    {"15005019", "high jump"},
    {"15005020", "pole vault"},
    {"15005021", "long jump"},
    {"15005022", "triple jump"},
    {"15005023", "shot put"},
    {"15005024", "discus throw"},
    {"15005025", "hammer throw"},
    {"15005026", "javelin throw"},
    {"15005027", "decathlon"},
    {"15005028", "4x100 m"},
    {"15005029", "4x200 m"},
    {"15005030", "4x400 m"},
    {"15005031", "4x800 m"},
    {"15005032", "4x1500 m"},
    {"15005033", "walk 1 h"},
    {"15005034", "walk 2 h"},
    {"15005035", "10 km walk"},
    {"15005036", "15 km walk"},
    {"15005037", "20 km walk"},
    {"15005038", "30 km walk"},
    {"15005039", "50 km walk"},
    {"15005040", "100 m hurdles"},
    {"15005041", "5 km walk"},
    {"15005042", "heptathlon"},
    {"15005043", "1500 m walk"},
    {"15005044", "2000 m walk"},
    {"15005045", "3000 m walk"},
    {"15005046", "50 m"},
    {"15005047", "50 m hurdles"},
    {"15005048", "50 yards"},
    {"15005049", "50 yard hurdles"},
    {"15005050", "60 m"},
    {"15005051", "60 m hurdles"},
    {"15005052", "60 yards"},
    {"15005053", "60 yard hurdles"},
    {"15005054", "100 yards"},
    {"15005055", "100 yard hurdles"},
    {"15005056", "300 m"},
    {"15005057", "300 yards"},
    {"15005058", "440 yards"},
    {"15005059", "500 m"},
    {"15005060", "500 yards"},
    {"15005061", "600 m"},
    {"15005062", "600 yards"},
    {"15005063", "880 yards"},
    {"15005064", "1000 yards"},
    {"15005065", "2 miles"},
    {"15005066", "3 miles"},
    {"15005067", "6 miles"},
    {"15005068", "4x1 mile"},
    {"15005069", "pentathlon"},
    {"15006000", "badminton"},
    {"15007000", "baseball"},
    {"15007001", "Major League Baseball (North American Professional) - American League"},
    {"15007002", "Major League Baseball (North American Professional) - National League"},
    {"15007003", "Major League Baseball (North American Professional) - Special (e.g. All-Star, World Series)"},
    {"15007004", "rubberball baseball"},
    {"15007005", "Major League Baseball Playoffs"},
    {"15007006", "World Series"},
    {"15008000", "basketball"},
    {"15008001", "National Basketball Association (North American Professional)"},
    {"15008002", "professional - Women general"},
    {"15008003", "Swiss netball"},
    {"15008004", "German netball"},
    {"15008005", "Dutch netball"},
    {"15009000", "biathlon"},
    {"15009001", "7.5 km"},
    {"15009002", "10 km"},
    {"15009003", "15 km"},
    {"15009004", "20 km"},
    {"15009005", "4x7.5 km relay"},
    {"15009006", "12.5 km pursuit"},
    {"15010000", "billiards, snooker and pool"},
    {"15010001", "8 ball"},
    {"15010002", "9 ball"},
    {"15010003", "14.1"},
    {"15010004", "continuous-DEPRECATED"},
    {"15010005", "other-DEPRECATED"},
    {"15010006", "snooker"},
    {"15011000", "bobsleigh"},
    {"15011001", "two-man sled"},
    {"15011002", "four-man sled"},
    {"15012000", "bowling"},
    {"15013000", "bowls and petanque"},
    {"15014000", "boxing"},
    {"15014001", "super-heavyweight"},
    {"15014002", "heavyweight"},
    {"15014003", "cruiserweight"},
    {"15014004", "light-heavyweight"},
    {"15014005", "super-middleweight"},
    {"15014006", "middleweight"},
    {"15014007", "light-middleweight"},
    {"15014008", "welterweight"},
    {"15014009", "light-welterweight"},
    {"15014010", "lightweight"},
    {"15014011", "super-featherweight"},
    {"15014012", "featherweight"},
    {"15014013", "super-bantamweight"},
    {"15014014", "bantamweight"},
    {"15014015", "super-flyweight"},
    {"15014016", "flyweight"},
    {"15014017", "light flyweight"},
    {"15014018", "straw"},
    {"15014019", "IBF"},
    {"15014020", "WBA"},
    {"15014021", "WBC"},
    {"15014022", "WBO"},
    {"15014023", "French boxing"},
    {"15014024", "Thai boxing"},
    {"15015000", "canoeing and kayaking"},
    {"15015001", "Slalom"},
    {"15015002", "200 m"},
    {"15015003", "500 m"},
    {"15015004", "1000 m"},
    {"15015005", "K1"},
    {"15015006", "K2"},
    {"15015007", "K4"},
    {"15015008", "C1"},
    {"15015009", "C2"},
    {"15015010", "C4"},
    {"15015011", "canoe sailing"},
    {"15015012", "pontoniering"},
    {"15016000", "climbing"},
    {"15016001", "mountaineering"},
    {"15016002", "sport climbing"},
    {"15017000", "cricket"},
    {"15018000", "curling"},
    {"15018001", "icestock sport"},
    {"15019000", "cycling"},
    {"15019001", "track"},
    {"15019002", "pursuit"},
    {"15019003", "Olympic sprint"},
    {"15019004", "sprint"},
    {"15019005", "Keirin"},
    {"15019006", "points race"},
    {"15019007", "Madison race"},
    {"15019008", "500 m time trial"},
    {"15019009", "1 km time trial"},
    {"15019010", "one hour"},
    {"15019011", "road race"},
    {"15019012", "road time trial"},
    {"15019013", "staging race"},
    {"15019014", "cyclo-cross"},
    {"15019015", "Vtt"},
    {"15019016", "Vtt-cross"},
    {"15019017", "Vtt-downhill"},
    {"15019018", "bi-crossing"},
    {"15019019", "trial"},
    {"15019020", "artistic cycling"},
    {"15019021", "cycle ball"},
    {"15020000", "dancing"},
    {"15021000", "diving"},
    {"15021001", "10 m platform"},
    {"15021002", "10 m platform synchronised"},
    {"15021003", "3 m springboard"},
    {"15021004", "3 m springboard synchronised"},
    {"15021005", "subaquatics"},
    {"15021006", "scuba diving"},
    {"15022000", "equestrian"},
    {"15022001", "three-day event"},
    {"15022002", "dressage"},
    {"15022003", "jumping"},
    {"15022004", "cross country"},
    {"15023000", "fencing"},
    {"15023001", "epee"},
    {"15023002", "foil"},
    {"15023003", "sabre"},
    {"15024000", "field Hockey"},
    {"15024001", "roll hockey"},
    {"15025000", "figure Skating"},
    {"15025001", "singles"},
    {"15025002", "pairs"},
    {"15025003", "ice dance"},
    {"15026000", "freestyle Skiing"},
    {"15026001", "moguls"},
    {"15026002", "aerials"},
    {"15026003", "artistic skiing"},
    {"15027000", "golf"},
    {"15028000", "gymnastics"},
    {"15028001", "floor exercise"},
    {"15028002", "vault"},
    {"15028003", "pommel horse"},
    {"15028004", "uneven bars"},
    {"15028005", "parallel bars"},
    {"15028006", "horizontal bar"},
    {"15028007", "rings"},
    {"15028008", "beam"},
    {"15028009", "rhythmic"},
    {"15028010", "clubs"},
    {"15028011", "hoop"},
    {"15028012", "ribbon"},
    {"15028013", "rope"},
    {"15028014", "ball"},
    {"15028015", "trampoline"},
    {"15029000", "handball (team)"},
    {"15030000", "horse racing, harness racing"},
    {"15030001", "flat racing"},
    {"15030002", "steeple chase"},
    {"15030003", "trotting"},
    {"15030004", "cross country"},
    {"15031000", "ice hockey"},
    {"15031001", "National Hockey League (North American)"},
    {"15031002", "sledge hockey"},
    {"15032000", "Jai Alai (Pelota)"},
    {"15032001", "fronton"},
    {"15032002", "jai-alai"},
    {"15032003", "left wall"},
    {"15032004", "trinquet"},
    {"15032005", "rebot"},
    {"15032006", "chistera ancha"},
    {"15032007", "chistera corta"},
    {"15032008", "bare hand"},
    {"15032009", "pala-ancha"},
    {"15032010", "pala-corta"},
    {"15032011", "pasaka"},
    {"15032012", "xare"},
    {"15033000", "judo"},
    {"15033001", "heavyweight"},
    {"15033002", "half-heavyweight"},
    {"15033003", "middleweight"},
    {"15033004", "half-middleweight"},
    {"15033005", "half-lightweight"},
    {"15033006", "lightweight"},
    {"15033007", "extra lightweight"},
    {"15034000", "karate"},
    {"15034001", "sparring"},
    {"15034002", "formal exercise-DEPRECATED"},
    {"15035000", "lacrosse"},
    {"15036000", "luge"},
    {"15036001", "singles"},
    {"15036002", "doubles"},
    {"15037000", "marathon"},
    {"15038000", "modern pentathlon"},
    {"15038001", "running"},
    {"15038002", "shooting"},
    {"15038003", "swimming"},
    {"15038004", "fencing"},
    {"15038005", "showjumping"},
    {"15039000", "motor racing"},
    {"15039001", "Formula One"},
    {"15039002", "F3000"},
    {"15039003", "endurance"},
    {"15039004", "Indy"},
    {"15039005", "CART"},
    {"15039006", "NHRA"},
    {"15039007", "NASCAR"},
    {"15039008", "TRUCKI"},
    {"15040000", "motor rallying"},
    {"15040001", "rallying"},
    {"15040002", "pursuit"},
    {"15040003", "rallycross"},
    {"15041000", "motorcycling"},
    {"15041001", "speed-Grand-Prix"},
    {"15041002", "enduro"},
    {"15041003", "grass-track"},
    {"15041004", "moto-ball"},
    {"15041005", "moto-cross"},
    {"15041006", "rallying"},
    {"15041007", "trial"},
    {"15041008", "endurance"},
    {"15041009", "superbike"},
    {"15041010", "125 cm3"},
    {"15041011", "250 cm3"},
    {"15041012", "500 cm3"},
    {"15041013", "side-cars"},
    {"15041014", "motoGP"},
    {"15042000", "netball"},
    {"15043000", "nordic skiing"},
    {"15043001", "cross-country"},
    {"15043002", "5 km classical time"},
    {"15043003", "10 km classical style"},
    {"15043004", "10 km pursuit free style"},
    {"15043005", "15 km classical style"},
    {"15043006", "15 km pursuit free style"},
    {"15043007", "10 km + 15 km combined"},
    {"15043008", "30 km classical style"},
    {"15043009", "30km free style"},
    {"15043010", "50 km free style"},
    {"15043011", "4x5 km relay"},
    {"15043012", "4x10 km relay"},
    {"15043013", "nordic combined"},
    {"15043014", "raid"},
    {"15043015", "5 km pursuit free style"},
    {"15043016", "1.5 km sprint free"},
    {"15043017", "50 km classic style"},
    {"15044000", "orienteering"},
    {"15044001", "ski orienteering"},
    {"15045000", "polo"},
    {"15046000", "power boating"},
    {"15046001", "F1"},
    {"15046002", "F2"},
    {"15047000", "rowing"},
    {"15047001", "single sculls"},
    {"15047002", "double sculls"},
    {"15047003", "quadruple sculls"},
    {"15047004", "coxless pair"},
    {"15047005", "coxless four"},
    {"15047006", "eight"},
    {"15047007", "lightweight"},
    {"15048000", "rugby league"},
    {"15049000", "rugby union"},
    {"15049001", "rugby 7"},
    {"15050000", "sailing"},
    {"15050001", "Tornado"},
    {"15050002", "soling"},
    {"15050003", "49er"},
    {"15050004", "Europe"},
    {"15050005", "Laser"},
    {"15050006", "470"},
    {"15050007", "Finn"},
    {"15050008", "Star"},
    {"15050009", "flying dutchmann"},
    {"15050010", "505"},
    {"15050011", "staging race"},
    {"15050012", "around the world"},
    {"15050013", "monohull"},
    {"15050014", "multihulls"},
    {"15050015", "yngling"},
    {"15050016", "mistral"},
    {"15051000", "shooting"},
    {"15051001", "10 m air rifle"},
    {"15051002", "10 m air pistol"},
    {"15051003", "10 m running target"},
    {"15051004", "25 m rapid fire pistol"},
    {"15051005", "25 m sport pistol"},
    {"15051006", "50 m free pistol"},
    {"15051007", "50 m free rifle prone"},
    {"15051008", "50 m free rifle 3x40"},
    {"15051009", "50 m sport rifle 3x20"},
    {"15051010", "trap"},
    {"15051011", "double trap"},
    {"15051012", "skeet"},
    {"15052000", "ski jumping"},
    {"15052001", "K90 jump"},
    {"15052002", "K120 jump"},
    {"15052003", "K180 (flying jump)"},
    {"15053000", "snow boarding"},
    {"15053001", "giant slalom"},
    {"15053002", "half-pipe"},
    {"15054000", "soccer"},
    {"15055000", "softball"},
    {"15056000", "speed skating"},
    {"15056001", "500 m"},
    {"15056002", "1000 m"},
    {"15056003", "1500 m"},
    {"15056004", "3000 m"},
    {"15056005", "5000 m"},
    {"15056006", "10000 m"},
    {"15056007", "Short-track"},
    {"15056008", "st 500 m"},
    {"15056009", "st 1000m"},
    {"15056010", "st 1500m"},
    {"15056011", "st 3000m"},
    {"15056012", "st 3000m relay"},
    {"15056013", "st 5000m"},
    {"15056014", "st 5000m relay"},
    {"15057000", "speedway"},
    {"15058000", "sports organisations"},
    {"15058001", "IOC"},
    {"15058002", "international federation"},
    {"15058003", "continental federation"},
    {"15058004", "national federation"},
    {"15058005", "GAISF"},
    {"15059000", "squash"},
    {"15060000", "sumo wrestling"},
    {"15061000", "surfing"},
    {"15062000", "swimming"},
    {"15062001", "50 m freestyle"},
    {"15062002", "100 m freestyle"},
    {"15062003", "200 m freestyle"},
    {"15062004", "400 m freestyle"},
    {"15062005", "800 m freestyle"},
    {"15062006", "1500 m freestyle"},
    {"15062007", "relay 4x50 m freestyle"},
    {"15062008", "relay 4x100 m freestyle"},
    {"15062009", "relay 4x200 m freestyle"},
    {"15062010", "50 m backstroke"},
    {"15062011", "100 m backstroke"},
    {"15062012", "200 m backstroke"},
    {"15062013", "50 m breaststroke"},
    {"15062014", "100 m breaststroke"},
    {"15062015", "200 m breaststroke"},
    {"15062016", "50 m butterfly"},
    {"15062017", "100 m butterfly"},
    {"15062018", "200 m butterfly"},
    {"15062019", "100 m medley"},
    {"15062020", "200 m medley"},
    {"15062021", "400 m medley"},
    {"15062022", "relay 4x50 m medlay"},
    {"15062023", "relay4x100 m medley"},
    {"15062024", "short course"},
    {"15062025", "synchronised technical routine"},
    {"15062026", "synchronised free routine"},
    {"15063000", "table tennis"},
    {"15064000", "Taekwon-Do"},
    {"15064001", "under 49 kg"},
    {"15064002", "under 58 kg"},
    {"15064003", "49-57 kg"},
    {"15064004", "58-68 kg"},
    {"15064005", "57-67 kg"},
    {"15064006", "68-80 kg"},
    {"15064007", "over 67 kg"},
    {"15064008", "over 80 kg"},
    {"15065000", "tennis"},
    {"15065001", "soft tennis"},
    {"15066000", "triathlon"},
    {"15066001", "triathlon swimming"},
    {"15066002", "triathlon cycling"},
    {"15066003", "triathlon run"},
    {"15067000", "volleyball"},
    {"15067001", "beach volleyball"},
    {"15068000", "water polo"},
    {"15069000", "water skiing"},
    {"15069001", "slalom"},
    {"15069002", "trick"},
    {"15069003", "jump"},
    {"15069004", "combined"},
    {"15070000", "weightlifting"},
    {"15070001", "snatch"},
    {"15070002", "clean and jerk"},
    {"15070003", "48 kg"},
    {"15070004", "53 kg"},
    {"15070005", "63 kg"},
    {"15070006", "75 kg"},
    {"15070007", "over 75 kg"},
    {"15070008", "56 kg"},
    {"15070009", "62 kg"},
    {"15070010", "69 kg"},
    {"15070011", "77 kg"},
    {"15070012", "85 kg"},
    {"15070013", "94 kg"},
    {"15070014", "105 kg"},
    {"15070015", "over 105 kg"},
    {"15070016", "powerlifting"},
    {"15071000", "windsurfing"},
    {"15071001", "ocean"},
    {"15071002", "lake"},
    {"15071003", "river"},
    {"15071004", "land"},
    {"15072000", "wrestling"},
    {"15072001", "freestyle"},
    {"15072002", "greco-roman"},
    {"15072003", "over 130 kg"},
    {"15072004", "130 kg"},
    {"15072005", "97 kg"},
    {"15072006", "85 kg"},
    {"15072007", "76 kg"},
    {"15072008", "69 kg"},
    {"15072009", "63 kg"},
    {"15072010", "58 kg"},
    {"15072011", "54 kg"},
    {"15072012", "Swiss wrestling"},
    {"15073000", "sports event"},
    {"15073001", "Summer Olympics"},
    {"15073002", "Winter Olympics"},
    {"15073003", "Summer universiade"},
    {"15073004", "Winter Universiade"},
    {"15073005", "Commonwealth Games"},
    {"15073006", "Winter Goodwill Games"},
    {"15073007", "Summer Asian Games"},
    {"15073008", "Winter Asian Games"},
    {"15073009", "Panamerican Games"},
    {"15073010", "African Games"},
    {"15073011", "Mediterranean Games"},
    {"15073012", "SouthEast Asiatic Games"},
    {"15073013", "PanPacific Games"},
    {"15073014", "SouthPacific Games"},
    {"15073015", "PanArabic Games"},
    {"15073016", "Summer Goodwill Games"},
    {"15073017", "World games"},
    {"15073018", "World Cup"},
    {"15073019", "intercontinental cup"},
    {"15073020", "continental cup"},
    {"15073021", "international cup"},
    {"15073022", "National Cup"},
    {"15073023", "interregional cup"},
    {"15073024", "regional cup"},
    {"15073025", "league cup"},
    {"15073026", "world championship"},
    {"15073027", "intercontinental championship"},
    {"15073028", "continental championship 1st level"},
    {"15073029", "continental championship 2nd level"},
    {"15073030", "continental championship 3rd level"},
    {"15073031", "national championship 1st level"},
    {"15073032", "national championship 2nd level"},
    {"15073033", "national championship3rdlevel"},
    {"15073034", "national championship 4th level"},
    {"15073035", "regional championship"},
    {"15073036", "Grand Prix"},
    {"15073037", "intercontinental tournament"},
    {"15073038", "continental tournament"},
    {"15073039", "international tournament"},
    {"15073040", "national tournament"},
    {"15073041", "inter-nations competition"},
    {"15073042", "inter-clubs competition"},
    {"15073043", "friendly competition"},
    {"15073044", "all-stars competition"},
    {"15073045", "exhibition"},
    {"15073046", "Super Bowl"},
    {"15073047", "paralympic games"},
    {"15074000", "rodeo"},
    {"15074001", "barrel racing"},
    {"15074002", "calf roping"},
    {"15074003", "bull riding"},
    {"15074004", "bulldogging"},
    {"15074005", "saddle bronc"},
    {"15074006", "bareback"},
    {"15074007", "goat roping"},
    {"15075000", "mini golf sport"},
    {"15076000", "bandy"},
    {"15077000", "flying disc"},
    {"15077001", "ultimate"},
    {"15077002", "guts"},
    {"15077003", "overall"},
    {"15077004", "distance"},
    {"15077005", "discathon"},
    {"15077006", "DDC"},
    {"15077007", "SCF"},
    {"15077008", "freestyle"},
    {"15077009", "accuracy"},
    {"15077010", "disc golf"},
    {"15078000", "floorball"},
    {"15079000", "casting"},
    {"15080000", "tug-of-war"},
    {"15081000", "croquette"},
    {"15082000", "dog racing"},
    {"15082001", "sled"},
    {"15082002", "oval track"},
    {"15083000", "skeleton"},
    {"15084000", "Australian rules football"},
    {"15085000", "Canadian football"},
    {"15086000", "duathlon"},
    {"15087000", "hornuss"},
    {"15088000", "fist ball"},
    {"15089000", "inline skating"},
    {"15090000", "grass ski"},
    {"15091000", "snowbiking"},
    {"15092000", "twirling"},
    {"15093000", "kendo"},
    {"15094000", "jukendo"},
    {"15095000", "naginata"},
    {"15096000", "kyudo"},
    {"15097000", "kabaddi"},
    {"15098000", "sepak takraw"},
    {"15099000", "wushu"},
    {"15100000", "darts"},
    {"15101000", "bodybuilding"},
    {"15102000", "sports disciplinary action"},
    {"15103000", "sports awards"},
    {"16000000", "unrest, conflicts and war"},
    {"16001000", "act of terror"},
    {"16002000", "armed conflict"},
    {"16003000", "civil unrest"},
    {"16003001", "revolutions"},
    {"16003002", "rebellions"},
    {"16003003", "political dissent"},
    {"16003004", "religious conflict"},
    {"16003005", "social conflict"},
    {"16004000", "coup d'etat"},
    {"16005000", "guerrilla activity"},
    {"16005001", "bioterrorism"},
    {"16005002", "bombings"},
    {"16006000", "massacre"},
    {"16006001", "genocide"},
    {"16007000", "riots"},
    {"16008000", "demonstration"},
    {"16009000", "war"},
    {"16009001", "civil war"},
    {"16009002", "international military intervention"},
    {"16009003", "prisoners and detainees"},
    {"16010000", "conflict (general)"},
    {"16010001", "peacekeeping force"},
    {"16011000", "crisis"},
    {"16012000", "weaponry"},
    {"17000000", "weather"},
    {"17001000", "forecast"},
    {"17002000", "global change"},
    {"17003000", "report"},
    {"17003001", "weather news"},
    {"17004000", "statistic"},
    {"17005000", "warning"},
};

//! XMP iptcExt:DigitalSourcefileType
constexpr TagVocabulary iptcExtDigitalSourcefileType[] = {
    {"scanfilm", N_("Scan from film")},   {"scantransparency", N_("Scan from transparency (including slide)")},
    {"scanprint", N_("Scan from print")}, {"cameraraw", N_("Camera RAW")},
    {"cameratiff", N_("Camera TIFF")},    {"camerajpeg", N_("Camera JPEG")},
};

//! XMP iptcExt:DigitalSourceType
//  Source: http://cv.iptc.org/newscodes/digitalsourcetype/
constexpr TagVocabulary iptcExtDigitalSourceType[] = {
    {"digitalCapture", N_("Digital capture")},
    {"negativeFilm", N_("Digitised from a negative on film")},
    {"positiveFilm", N_("Digitised from a positive on film")},
    {"print", N_("Digitised from a print on non-transparent medium")},
    {"softwareImage", N_("The digital image was created by computer software")},
};

const XmpPropertyInfo xmpPlusInfo[] = {
    // PLUS Version 1.2.0
    {"Version", N_("PLUS Version"), "Text", xmpText, xmpExternal,
     N_("The version number of the PLUS standards in place at the time of the transaction.")},
    {"Licensee", N_("Licensee"), "seq LicenseeDetail", xmpSeq, xmpExternal,
     N_("Party or parties to whom the license is granted by the Licensor/s under the license transaction.")},
    {"LicenseeID", N_("Licensee ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Licensee.")},
    {"LicenseeName", N_("Licensee Name"), "ProperName", xmpText, xmpExternal, N_("Name of each Licensee.")},
    {"EndUser", N_("End User"), "seq EndUserDetail", xmpSeq, xmpExternal,
     N_("Party or parties ultimately making use of the image under the license.")},
    {"EndUserID", N_("End User ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each End User.")},
    {"EndUserName", N_("End User Name"), "ProperName", xmpText, xmpExternal, N_("Name of each End User.")},
    {"Licensor", N_("Licensor"), "seq LicensorDetail", xmpSeq, xmpExternal,
     N_("Party or parties granting the license to the Licensee.")},
    {"LicensorID", N_("Licensor ID"), "Text", xmpText, xmpExternal, N_("Optional PLUS-ID identifying each Licensor.")},
    {"LicensorName", N_("Licensor Name"), "ProperName", xmpText, xmpExternal, N_("Name of each Licensor.")},
    {"LicensorStreetAddress", N_("Licensor Address"), "Text", xmpText, xmpExternal, N_("Licensor street address.")},
    {"LicensorExtendedAddress", N_("Licensor Address Detail"), "Text", xmpText, xmpExternal,
     N_("Additional Licensor mailing address details.")},
    {"LicensorCity", N_("Licensor City"), "Text", xmpText, xmpExternal, N_("Licensor City name.")},
    {"LicensorRegion", N_("Licensor State or Province"), "Text", xmpText, xmpExternal,
     N_("Licensor State or Province name.")},
    {"LicensorPostalCode", N_("Licensor Postal Code"), "Text", xmpText, xmpExternal,
     N_("Licensor Postal Code or Zip Code.")},
    {"LicensorCountry", N_("Licensor Country"), "Text", xmpText, xmpExternal, N_("Licensor Country name.")},
    {"LicensorTelephoneType1", N_("Licensor Telephone Type 1"), "URL", xmpText, xmpExternal,
     N_("Licensor Telephone Type 1.")},
    {"LicensorTelephone1", N_("Licensor Telephone 1"), "Text", xmpText, xmpExternal,
     N_("Licensor Telephone number 1.")},
    {"LicensorTelephoneType2", N_("Licensor Telephone Type 2"), "URL", xmpText, xmpExternal,
     N_("Licensor Telephone Type 2.")},
    {"LicensorTelephone2", N_("Licensor Telephone 2"), "Text", xmpText, xmpExternal,
     N_("Licensor Telephone number 2.")},
    {"LicensorEmail", N_("Licensor Email"), "Text", xmpText, xmpExternal, N_("Licensor Email address.")},
    {"LicensorURL", N_("Licensor URL"), "URL", xmpText, xmpExternal, N_("Licensor world wide web address.")},
    {"LicensorNotes", N_("Licensor Notes"), "Lang Alt", langAlt, xmpExternal,
     N_("Supplemental information for use in identifying and contacting the Licensor/s.")},
    {"MediaSummaryCode", N_("PLUS Media Summary Code"), "Text", xmpText, xmpExternal,
     N_("A PLUS-standardized alphanumeric code string summarizing the media usages included in the license.")},
    {"LicenseStartDate", N_("License Start Date"), "Date", xmpText, xmpExternal,
     N_("The date on which the license takes effect.")},
    {"LicenseEndDate", N_("License End Date"), "Date", xmpText, xmpExternal,
     N_("The date on which the license expires.")},
    {"MediaConstraints", N_("Media Constraints"), "Lang Alt", langAlt, xmpExternal,
     N_("Constraints limiting the scope of PLUS Media Usage/s included in the license to particular named media or "
        "to media not yet specifically defined in the PLUS Media Matrix.")},
    {"RegionConstraints", N_("Region Constraints"), "Lang Alt", langAlt, xmpExternal,
     N_("Constraints limiting the scope of geographic distribution to specific cities, states, provinces or other "
        "areas to be included in or excluded from the PLUS Regions specified in the Media Usages specified in the "
        "license.")},
    {"ProductOrServiceConstraints", N_("Product or Service Constraints"), "Lang Alt", langAlt, xmpExternal,
     N_("Constraints limiting usage of the image to promotion of/association with a named product or service.")},
    {"ImageFileConstraints", N_("Image File Constraints"), "bag URL", xmpBag, xmpExternal,
     N_("Constraints on the changing of the image file name, metadata or file type.")},
    {"ImageAlterationConstraints", N_("Image Alteration Constraints"), "bag URL", xmpBag, xmpExternal,
     N_("Constraints on alteration of the image by cropping, flipping, retouching, colorization, de-colorization "
        "or merging.")},
    {"ImageDuplicationConstraints", N_("Image Duplication Constraints"), "URL", xmpText, xmpExternal,
     N_("Constraints on the creation of duplicates of the image.")},
    {"ModelReleaseStatus", N_("Model Release Status"), "URL", xmpText, xmpExternal,
     N_("Summarizes the availability and scope of model releases authorizing usage of the likenesses of persons "
        "appearing in the photograph.")},
    {"ModelReleaseID", N_("Model Release ID"), "bag Text", xmpBag, xmpExternal,
     N_("Optional identifier associated with each Model Release.")},
    {"MinorModelAgeDisclosure", N_("Minor Model Age Disclosure"), "URL", xmpText, xmpExternal,
     N_("Age of the youngest model pictured in the image, at the time that the image was made.")},
    {"PropertyReleaseStatus", N_("Property Release Status"), "URL", xmpText, xmpExternal,
     N_("Summarizes the availability and scope of property releases authorizing usage of the properties appearing "
        "in the photograph.")},
    {"PropertyReleaseID", N_("Property Release ID"), "bag Text", xmpBag, xmpExternal,
     N_("Optional identifier associated with each Property Release.")},
    {"OtherConstraints", N_("Other Constraints"), "Lang Alt", langAlt, xmpExternal,
     N_("Additional constraints on the license.")},
    {"CreditLineRequired", N_("Credit Line Required"), "URL", xmpText, xmpExternal,
     N_("Attribution requirements, if any.")},
    {"AdultContentWarning", N_("Adult Content Warning"), "URL", xmpText, xmpExternal,
     N_("Warning indicating the presence of content not suitable for minors.")},
    {"OtherLicenseRequirements", N_("Other License Requirements"), "Lang Alt", langAlt, xmpExternal,
     N_("Additional license requirements.")},
    {"TermsAndConditionsText", N_("Terms and Conditions Text"), "Lang Alt", langAlt, xmpExternal,
     N_("Terms and Conditions applying to the license.")},
    {"TermsAndConditionsURL", N_("Terms and Conditions URL"), "URL", xmpText, xmpExternal,
     N_("URL for Terms and Conditions applying to the license.")},
    {"OtherConditions", N_("Other License Conditions"), "Lang Alt", langAlt, xmpExternal,
     N_("Additional license conditions.")},
    {"ImageType", N_("Image Type"), "URL", xmpText, xmpExternal, N_("Identifies the type of image delivered.")},
    {"LicensorImageID", N_("Licensor Image ID"), "Text", xmpText, xmpExternal,
     N_("Optional identifier assigned by the Licensor to the image.")},
    {"FileNameAsDelivered", N_("Image File Name As Delivered"), "Text", xmpText, xmpExternal,
     N_("Name of the image file delivered to the Licensee for use under the license.")},
    {"ImageFileFormatAsDelivered", N_("Image File Format As Delivered"), "URL", xmpText, xmpExternal,
     N_("File format of the image file delivered to the Licensee for use under the license.")},
    {"ImageFileSizeAsDelivered", N_("Image File Size As Delivered"), "URL", xmpText, xmpExternal,
     N_("Size of the image file delivered to the Licensee.")},
    {"CopyrightStatus", N_("Copyright Status"), "URL", xmpText, xmpExternal, N_("Copyright status of the image.")},
    {"CopyrightRegistrationNumber", N_("Copyright Registration Number"), "Text", xmpText, xmpExternal,
     N_("Copyright Registration Number, if any, applying to the licensed image.")},
    {"FirstPublicationDate", N_("First Publication Date"), "Date", xmpText, xmpExternal,
     N_("The date on which the image was first published.")},
    {"CopyrightOwner", N_("Copyright Owner"), "seq CopyrightOwnerDetail", xmpSeq, xmpExternal,
     N_("Owner or owners of the copyright in the licensed image.")},
    {"CopyrightOwnerID", N_("Copyright Owner ID"), "Text", xmpText, xmpExternal,
     N_("Optional PLUS-ID identifying each Copyright Owner.")},
    {"CopyrightOwnerName", N_("Copyright Owner Name"), "ProperName", xmpText, xmpExternal,
     N_("Name of Copyright Owner.")},
    {"CopyrightOwnerImageID", N_("Copyright Owner Image ID"), "Text", xmpText, xmpExternal,
     N_("Optional identifier assigned by the Copyright Owner to the image.")},
    {"ImageCreator", N_("Image Creator"), "seq ImageCreatorDetail", xmpSeq, xmpExternal, N_("Creator/s of the image.")},
    {"ImageCreatorID", N_("Image Creator ID"), "Text", xmpText, xmpExternal,
     N_("Optional PLUS-ID identifying each Image Creator.")},
    {"ImageCreatorName", N_("Image Creator Name"), "ProperName", xmpText, xmpExternal, N_("Name of Image Creator.")},
    {"ImageCreatorImageID", N_("Image Creator Image ID"), "Text", xmpText, xmpExternal,
     N_("Optional identifier assigned by the Image Creator to the image.")},
    {"ImageSupplierID", N_("Image Supplier ID"), "Text", xmpText, xmpExternal,
     N_("Optional PLUS-ID identifying the Image Supplier.")},
    {"ImageSupplierName", N_("Image Supplier Name"), "ProperName", xmpText, xmpExternal, N_("Name of Image Supplier.")},
    {"ImageSupplierImageID", N_("Image Supplier Image ID"), "Text", xmpText, xmpExternal,
     N_("Optional identifier assigned by the Image Supplier to the image.")},
    {"LicenseeImageID", N_("Licensee Image ID"), "Text", xmpText, xmpExternal,
     N_("Optional identifier assigned by the Licensee to the image.")},
    {"LicenseeImageNotes", N_("Licensee Image Notes"), "Lang Alt", langAlt, xmpExternal,
     N_("Notes added by Licensee.")},
    {"OtherImageInfo", N_("Other Image Info"), "Lang Alt", langAlt, xmpExternal, N_("Additional image information.")},
    {"LicenseID", N_("License ID"), "Text", xmpText, xmpExternal,
     N_("Optional PLUS-ID assigned by the Licensor to the License.")},
    {"LicensorTransactionID", N_("Licensor Transaction ID"), "bag Text", xmpBag, xmpExternal,
     N_("Identifier assigned by Licensor for Licensor's reference and internal use.")},
    {"LicenseeTransactionID", N_("Licensee Transaction ID"), "bag Text", xmpBag, xmpExternal,
     N_("Identifier assigned by Licensee for Licensee's reference and internal use.")},
    {"LicenseeProjectReference", N_("Licensee Project Reference"), "bag Text", xmpBag, xmpExternal,
     N_("Project reference name or description assigned by Licensee.")},
    {"LicenseTransactionDate", N_("License Transaction Date"), "Date", xmpText, xmpExternal,
     N_("The date of the License Transaction.")},
    {"Reuse", N_("Reuse"), "URL", xmpText, xmpExternal,
     N_("Indicates whether a license is a repeat or an initial license.  Reuse may require that licenses stored in "
        "files previously delivered to the customer be updated.")},
    {"OtherLicenseDocuments", N_("Other License Documents"), "bag Text", xmpBag, xmpExternal,
     N_("Reference information for additional documents associated with the license.")},
    {"OtherLicenseInfo", N_("Other License Info"), "Lang Alt", langAlt, xmpExternal,
     N_("Additional license information.")},
    {"Custom1", N_("Custom 1"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensor's discretion.")},
    {"Custom2", N_("Custom 2"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensor's discretion.")},
    {"Custom3", N_("Custom 3"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensor's discretion.")},
    {"Custom4", N_("Custom 4"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensor's discretion.")},
    {"Custom5", N_("Custom 5"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensor's discretion.")},
    {"Custom6", N_("Custom 6"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensee's discretion.")},
    {"Custom7", N_("Custom 7"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensee's discretion.")},
    {"Custom8", N_("Custom 8"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensee's discretion.")},
    {"Custom9", N_("Custom 9"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensee's discretion.")},
    {"Custom10", N_("Custom 10"), "bag Lang Alt", xmpBag, xmpExternal,
     N_("Optional field for use at Licensee's discretion.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

//! XMP plus:AdultContentWarning
constexpr TagVocabulary plusAdultContentWarning[] = {
    {"CW-AWR", N_("Adult Content Warning Required")},
    {"CW-NRQ", N_("Not Required")},
    {"CW-UNK", N_("Unknown")},
};

//! XMP plus:CopyrightStatus
constexpr TagVocabulary plusCopyrightStatus[] = {
    {"CS-PRO", N_("Protected")},
    {"CS-PUB", N_("Public Domain")},
    {"CS-UNK", N_("Unknown")},
};

//! XMP plus:CreditLineRequired
constexpr TagVocabulary plusCreditLineRequired[] = {
    {"CR-CAI", N_("Credit Adjacent To Image")},
    {"CR-CCA", N_("Credit in Credits Area")},
    {"CR-COI", N_("Credit on Image")},
    {"CR-NRQ", N_("Not Require")},
};

//! XMP plus:ImageAlterationConstraints
constexpr TagVocabulary plusImageAlterationConstraints[] = {
    {"AL-CLR", N_("No Colorization")}, {"AL-CRP", N_("No Cropping")}, {"AL-DCL", N_("No De-Colorization")},
    {"AL-FLP", N_("No Flipping")},     {"AL-MRG", N_("No Merging")},  {"AL-RET", N_("No Retouching")},
};

//! XMP plus:ImageDuplicationConstraints
constexpr TagVocabulary plusImageDuplicationConstraints[] = {
    {"DP-LIC", N_("Duplication Only as Necessary Under License")},
    {"DP-NDC", N_("No Duplication Constraints")},
    {"DP-NOD", N_("No Duplication")},
};

//! XMP plus:ImageFileConstraints
constexpr TagVocabulary plusImageFileConstraints[] = {
    {"IF-MFN", N_("Maintain File Name")},
    {"IF-MFT", N_("Maintain File Type")},
    {"IF-MID", N_("Maintain ID in File Name")},
    {"IF-MMD", N_("Maintain Metadata")},
};

//! XMP plus:ImageFileFormatAsDelivered
constexpr TagVocabulary plusImageFileFormatAsDelivered[] = {
    {"FF-BMP", N_("Windows Bitmap (BMP)")},
    {"FF-DNG", N_("Digital Negative (DNG)")},
    {"FF-EPS", N_("Encapsulated PostScript (EPS)")},
    {"FF-GIF", N_("Graphics Interchange Format (GIF)")},
    {"FF-JPG", N_("JPEG Interchange Formats (JPG, JIF, JFIF)")},
    {"FF-OTR", N_("Other")},
    {"FF-PIC", N_("Macintosh Picture (PICT)")},
    {"FF-PNG", N_("Portable Network Graphics (PNG)")},
    {"FF-PSD", N_("Photoshop Document (PSD)")},
    {"FF-RAW", N_("Proprietary RAW Image Format")},
    {"FF-TIF", N_("Tagged Image File Format (TIFF)")},
    {"FF-WMP", N_("Windows Media Photo (HD Photo)")},
};

//! XMP plus:ImageFileSizeAsDelivered
constexpr TagVocabulary plusImageFileSizeAsDelivered[] = {
    {"SZ-G50", N_("Greater than 50 MB")}, {"SZ-U01", N_("Up to 1 MB")},  {"SZ-U10", N_("Up to 10 MB")},
    {"SZ-U30", N_("Up to 30 MB")},        {"SZ-U50", N_("Up to 50 MB")},
};

//! XMP plus:ImageType
constexpr TagVocabulary plusImageType[] = {
    {"TY-ILL", N_("Illustrated Image")},
    {"TY-MCI", N_("Multimedia or Composited Image")},
    {"TY-OTR", N_("Other")},
    {"TY-PHO", N_("Photographic Image")},
    {"TY-VID", N_("Video")},
};

//! XMP plus:LicensorTelephoneType
constexpr TagVocabulary plusLicensorTelephoneType[] = {
    {"cell", N_("Cell")}, {"fax", N_("FAX")}, {"home", N_("Home")}, {"pager", N_("Pager")}, {"work", N_("Work")},
};

//! XMP plus:MinorModelAgeDisclosure
constexpr TagVocabulary plusMinorModelAgeDisclosure[] = {
    {"AG-UNK", N_("Age Unknown")},     {"AG-A25", N_("Age 25 or Over")}, {"AG-A24", N_("Age 24")},
    {"AG-A23", N_("Age 23")},          {"AG-A22", N_("Age 22")},         {"AG-A21", N_("Age 21")},
    {"AG-A20", N_("Age 20")},          {"AG-A19", N_("Age 19")},         {"AG-A18", N_("Age 18")},
    {"AG-A17", N_("Age 17")},          {"AG-A16", N_("Age 16")},         {"AG-A15", N_("Age 15")},
    {"AG-U14", N_("Age 14 or Under")},
};

//! XMP plus:ModelReleaseStatus
constexpr TagVocabulary plusModelReleaseStatus[] = {
    {"MR-NON", N_("None")},
    {"MR-NAP", N_("Not Applicable")},
    {"MR-UMR", N_("Unlimited Model Releases")},
    {"MR-LMR", N_("Limited or Incomplete Model Releases")},
};

//! XMP plus:PropertyReleaseStatus
constexpr TagVocabulary plusPropertyReleaseStatus[] = {
    {"PR-NON", N_("None")},
    {"PR-NAP", N_("Not Applicable")},
    {"PR-UPR", N_("Unlimited Property Releases")},
    {"PR-LPR", N_("Limited or Incomplete Property Releases")},
};

//! XMP plus:Reuse
constexpr TagVocabulary plusReuse[] = {
    {"RE-NAP", N_("Not Applicable")},
    {"RE-REU", N_("Repeat Use")},
};

const XmpPropertyInfo xmpMediaProInfo[] = {
    {"Event", N_("Event"), "Text", xmpText, xmpExternal, N_("Fixture Identification")},
    {"Status", N_("Status"), "Text", xmpText, xmpExternal, N_("A notation making the image unique")},
    {"People", N_("People"), "bag Text", xmpBag, xmpExternal, N_("Contact")},
    {"CatalogSets", N_("Catalog Sets"), "bag Text", xmpBag, xmpExternal,
     N_("Descriptive markers of catalog items by content")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpExpressionMediaInfo[] = {
    {"Event", N_("Event"), "Text", xmpText, xmpExternal, N_("Fixture Identification")},
    {"Status", N_("Status"), "Text", xmpText, xmpExternal, N_("A notation making the image unique")},
    {"People", N_("People"), "bag Text", xmpBag, xmpExternal, N_("Contact")},
    {"CatalogSets", N_("Catalog Sets"), "bag Text", xmpBag, xmpExternal,
     N_("Descriptive markers of catalog items by content")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMicrosoftPhotoInfo[] = {
    {"RegionInfo", N_("RegionInfo"), "RegionInfo", xmpText, xmpInternal,
     N_("Microsoft Photo people-tagging metadata root")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMicrosoftPhotoRegionInfoInfo[] = {
    {"Regions", N_("Regions"), "bag Region", xmpBag, xmpExternal, N_("Contains Regions/person tags")},
    {"DateRegionsValid", N_("Date Regions Valid"), "Date", xmpText, xmpExternal,
     N_("Date the last region was created")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMicrosoftPhotoRegionInfo[] = {
    {"PersonDisplayName", N_("Person Display Name"), "Text", xmpText, xmpExternal,
     N_("Name of the person (in the given rectangle)")},
    {"Rectangle", N_("Rectangle"), "Text", xmpText, xmpExternal,
     N_("Rectangle that identifies the person within the photo")},
    {
        "PersonEmailDigest",
        N_("Person Email Digest"),
        "Text",
        xmpText,
        xmpExternal,
        N_("SHA-1 encrypted message hash of the person's Windows Live e-mail address"),
    },
    {"PersonLiveIdCID", N_("Person LiveId CID"), "Text", xmpText, xmpExternal,
     N_("Signed decimal representation of the person's Windows Live CID")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMWGRegionsInfo[] = {
    {"Regions", N_("Regions"), "RegionInfo", xmpText, xmpInternal,
     N_("Main structure containing region based information")},
    {"AppliedToDimensions", N_("Applied To Dimensions"), "Dimensions", xmpText, xmpExternal,
     N_("Width and height of image when storing region data")},
    {"RegionList", N_("Region List"), "bag RegionStruct", xmpBag, xmpExternal, N_("List of Region structures")},
    {"Area", N_("Area"), "Area", xmpText, xmpExternal, N_("Descriptive markers of catalog items by content")},
    {"Type", N_("Type"), "closed Choice of Text", xmpText, xmpExternal,
     N_("Type purpose of region (Face|Pet|Focus|BarCode)")},
    {"Name", N_("Name"), "Text", xmpText, xmpExternal, N_("Name/ short description of content in image region")},
    {"Description", N_("Description"), "Text", xmpText, xmpExternal,
     N_("Usage scenario for a given focus area (EvaluatedUsed|EvaluatedNotUsed|NotEvaluatedNotUsed)")},
    {"FocusUsage", N_("Focus Usage"), "closed Choice of Text", xmpText, xmpExternal,
     N_("Descriptive markers of catalog items by content")},
    {"BarCodeValue", N_("Bar Code Value"), "Text", xmpText, xmpExternal, N_("Decoded BarCode value string")},
    {"Extensions", N_("Extensions"), "Text", xmpText, xmpInternal,
     N_("Any top level XMP property to describe the region content")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpMWGKeywordInfo[] = {
    {"Keywords", N_("Keywords"), "KeywordInfo", xmpText, xmpInternal,
     N_("Main structure containing keyword based information")},
    {"Hierarchy", N_("Hierarchy"), "bag KeywordStruct", xmpBag, xmpExternal, N_("List of root keyword structures")},
    {"Keyword", N_("Keyword"), "Text", xmpText, xmpExternal, N_("Name of keyword (-node)")},
    {"Applied", N_("Applied"), "Boolean", xmpText, xmpExternal,
     N_("True if this keyword has been applied, False otherwise. If missing, mwg-kw:Applied is presumed True for "
        "leaf nodes and False for ancestor nodes")},
    {"Children", N_("Children"), "bag KeywordStruct", xmpBag, xmpExternal, N_("List of children keyword structures")},

    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpGPanoInfo[] = {
    {"UsePanoramaViewer", N_("Use Panorama Viewer"), "Boolean", xmpText, xmpExternal,
     N_("Whether to show this image in a panorama viewer rather than as a normal flat image. This may be specified "
        "based on user preferences or by the stitching software. The application displaying or ingesting the image "
        "may choose to ignore this.")},
    {"CaptureSoftware", N_("Capture Software"), "Text", xmpText, xmpExternal,
     N_("If capture was done using an application on a mobile device, such as an Android phone, the name of the "
        "application that was used (such as \"Photo Sphere\"). This should be left blank if source images were "
        "captured manually, such as by using a DSLR on a tripod.")},
    {"StitchingSoftware", N_("Stitching Software"), "Text", xmpText, xmpExternal,
     N_("The software that was used to create the final panorama. This may sometimes be the same value as that of  "
        "GPano:CaptureSoftware.")},
    {"ProjectionType", N_("Projection Type"), "Open Choice of Text", xmpText, xmpExternal,
     N_("Projection type used in the image file. Google products currently support the value equirectangular.")},
    {"PoseHeadingDegrees", N_("Pose Heading Degrees"), "Real", xmpText, xmpExternal,
     N_("Compass heading, measured in degrees, for the center the image. Value must be >= 0 and < 360.")},
    {"PosePitchDegrees", N_("Pose Pitch Degrees"), "Real", xmpText, xmpExternal,
     N_("Pitch, measured in degrees, for the center in the image. Value must be >= -90 and <= 90.")},
    {"PoseRollDegrees", N_("Pose Roll Degrees"), "Real", xmpText, xmpExternal,
     N_("Roll, measured in degrees, of the image where level with the horizon is 0. Value must be > -180 and <= "
        "180.")},
    {"InitialViewHeadingDegrees", N_("Initial View Heading Degrees"), "Integer", xmpText, xmpExternal,
     N_("The heading angle of the initial view in degrees.")},
    {"InitialViewPitchDegrees", N_("Initial View Pitch Degrees"), "Integer", xmpText, xmpExternal,
     N_("The pitch angle of the initial view in degrees.")},
    {"InitialViewRollDegrees", N_("Initial View Roll Degrees"), "Integer", xmpText, xmpExternal,
     N_("The roll angle of the initial view in degrees.")},
    {"InitialHorizontalFOVDegrees", N_("Initial Horizontal FOV Degrees"), "Real", xmpText, xmpExternal,
     N_("The initial horizontal field of view that the viewer should display (in degrees). This is similar to a "
        "zoom level.")},
    {"FirstPhotoDate", N_("First Photo Date"), "Date", xmpText, xmpExternal,
     N_("Date and time for the first image created in the panorama.")},
    {"LastPhotoDate", N_("Last Photo Date"), "Date", xmpText, xmpExternal,
     N_("Date and time for the last image created in the panorama.")},
    {"SourcePhotosCount", N_("Source Photos Count"), "Integer", xmpText, xmpExternal,
     N_("Number of source images used to create the panorama")},
    {"ExposureLockUsed", N_("Exposure Lock Used"), "Boolean", xmpText, xmpExternal,
     N_("When individual source photographs were captured, whether or not the camera's exposure setting was "
        "locked.")},
    {"CroppedAreaImageWidthPixels", N_("Cropped Area Image Width Pixels"), "Integer", xmpText, xmpExternal,
     N_("Original width in pixels of the image (equal to the actual image's width for unedited images).")},
    {"CroppedAreaImageHeightPixels", N_("Cropped Area Image Height Pixels"), "Integer", xmpText, xmpExternal,
     N_("Original height in pixels of the image (equal to the actual image's height for unedited images).")},
    {"FullPanoWidthPixels", N_("Full Pano Width Pixels"), "Integer", xmpText, xmpExternal,
     N_("Original full panorama width from which the image was cropped. Or, if only a partial panorama was "
        "captured, this specifies the width of what the full panorama would have been.")},
    {"FullPanoHeightPixels", N_("Full Pano Height Pixels"), "Integer", xmpText, xmpExternal,
     N_("Original full panorama height from which the image was cropped. Or, if only a partial panorama was "
        "captured, this specifies the height of what the full panorama would have been.")},
    {"CroppedAreaLeftPixels", N_("Cropped Area Left Pixels"), "Integer", xmpText, xmpExternal,
     N_("Column where the left edge of the image was cropped from the full sized panorama.")},
    {"CroppedAreaTopPixels", N_("Cropped Area Top Pixels"), "Integer", xmpText, xmpExternal,
     N_("Row where the top edge of the image was cropped from the full sized panorama.")},
    {"InitialCameraDolly", N_("Initial Camera Dolly"), "Real", xmpText, xmpExternal,
     N_("This optional parameter moves the virtual camera position along the line of sight, away from the center "
        "of the photo sphere. A rear surface position is represented by the value -1.0, while a front surface "
        "position is represented by 1.0. For normal viewing, this parameter should be set to 0.")},

    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpVideoInfo[] = {
    {"Album", N_("Album"), "Text", xmpText, xmpExternal, N_("The name of the album.")},
    {"ArchivalLocation", N_("Archival Location"), "Text", xmpText, xmpExternal,
     N_("Information about the Archival Location.")},
    {"Arranger", N_("Arranger"), "Text", xmpText, xmpExternal, N_("Information about the Arranger.")},
    {"ArrangerKeywords", N_("Arranger Keywords"), "Text", xmpText, xmpExternal,
     N_("Information about the Arranger Keywords.")},
    {"Artist", N_("Artist"), "Text", xmpText, xmpExternal, N_("The name of the artist or artists.")},
    {"AspectRatio", N_("Video Aspect Ratio"), "Rational", xmpText, xmpExternal,
     N_("Ratio of Width:Height, helps to determine how a video would be displayed on a screen")},
    {"AspectRatioType", N_("Video Aspect Ratio Type"), "Text", xmpText, xmpExternal,
     N_("Aspect Ratio Type. E.g.: Free-Resizing or Fixed")},
    {"AttachFileData", N_("Attached File Data"), "Text", xmpText, xmpExternal, N_("Attached File Data")},
    {"AttachFileDesc", N_("Attached File Description"), "Text", xmpText, xmpExternal, N_("Attached File Description")},
    {"AttachFileMIME", N_("Attached File MIME Type"), "Text", xmpText, xmpExternal, N_("Attached File MIME Type")},
    {"AttachFileName", N_("Attached File Name"), "Text", xmpText, xmpExternal, N_("Attached File Name")},
    {"AttachFileUID", N_("Attached File UID"), "Integer", xmpText, xmpExternal, N_("Attached File Universal ID")},
    {"BaseURL", N_("Base URL"), "Text", xmpText, xmpExternal, N_("A C string that specifies a Base URL.")},
    {"BitDepth", N_("Bit Depth"), "Integer", xmpText, xmpExternal,
     N_("A 16-bit integer that indicates the pixel depth of the compressed image. Values of 1, 2, 4, 8 , 16, 24, "
        "and 32 indicate the depth of color images")},
    {"Brightness", N_("Brightness"), "Integer", xmpText, xmpExternal, N_("Brightness setting.")},
    {"CameraByteOrder", N_("Camera Byte Order"), "Text", xmpText, xmpExternal,
     N_("Byte Order used by the Video Capturing device.")},
    {"Cinematographer", N_("Video Cinematographer"), "Text", xmpText, xmpExternal,
     N_("The video Cinematographer information.")},
    {"CleanApertureWidth", N_("Clean Aperture Width"), "Integer", xmpText, xmpExternal,
     N_("Clean aperture width in pixels")},
    {"CleanApertureHeight", N_("Clean Aperture Height"), "Integer", xmpText, xmpExternal,
     N_("Clean aperture height in pixels")},
    {"Codec", N_("Video Codec"), "Text", xmpText, xmpExternal,
     N_("The video codec information. Informs about the encoding algorithm of video. Codec Info is required for "
        "video playback.")},
    {"CodecDecodeAll", N_("Video Codec Decode Info"), "Text", xmpText, xmpExternal,
     N_("Contains information the video Codec Decode All, i.e. Enabled/Disabled")},
    {"CodecDescription", N_("Video Codec Description"), "Text", xmpText, xmpExternal,
     N_("Contains description the codec.")},
    {"CodecInfo", N_("Video Codec Information"), "Text", xmpText, xmpExternal,
     N_("Contains information the codec needs before decoding can be started.")},
    {"CodecDownloadUrl", N_("Video Codec Download URL"), "Text", xmpText, xmpExternal, N_("Video Codec Download URL.")},
    {"CodecSettings", N_("Video Codec Settings"), "Text", xmpText, xmpExternal,
     N_("Contains settings the codec needs before decoding can be started.")},
    {"ColorMode", N_("Color Mode"), "Text", xmpText, xmpExternal, N_("Color Mode")},
    {"ColorNoiseReduction", N_("Color Noise Reduction"), "Integer", xmpText, xmpExternal,
     N_("\"Color Noise Reduction\" setting. Range 0 to +100.")},
    {"ColorSpace", N_("Video Color Space"), "closed Choice of Text", xmpText, xmpInternal,
     N_("The color space. One of: sRGB (used by Photoshop), CCIR-601 (used for NTSC), "
        "CCIR-709 (used for HD).")},
    {"Comment", N_("Comment"), "Text", xmpText, xmpExternal, N_("Information about the Comment.")},
    {"Commissioned", N_("Commissioned"), "Text", xmpText, xmpExternal, N_("Commissioned.")},
    {"CompatibleBrands", N_("QTime Compatible FileType Brand"), "Text", xmpText, xmpExternal,
     N_("Other QuickTime Compatible FileType Brand")},
    {"Composer", N_("Composer"), "Text", xmpText, xmpExternal, N_("Information about the Composer.")},
    {"ComposerKeywords", N_("Composer Keywords"), "Text", xmpText, xmpExternal,
     N_("Information about the Composer Keywords.")},
    {"Compressor", N_("Compressor"), "Text", xmpText, xmpExternal, N_("Video Compression Library Used")},
    {"CompressorID", N_("Video Compressor ID"), "Text", xmpText, xmpExternal,
     N_("Video Compression ID of Technology/Codec Used")},
    {"CompressorVersion", N_("Compressor Version"), "Text", xmpText, xmpExternal,
     N_("Information about the Compressor Version.")},
    {"Container", N_("Container Type"), "Text", xmpText, xmpExternal, N_("Primary Metadata Container")},
    {"ContentCompressAlgo", N_("Content Compression Algorithm"), "Text", xmpText, xmpExternal,
     N_("Content Compression Algorithm. E.g.: zlib")},
    {"ContentEncodingType", N_("Content Encoding Type"), "Text", xmpText, xmpExternal,
     N_("Content Encoding Type. E.g.: Encryption or Compression")},
    {"ContentEncryptAlgo", N_("Content Encryption Algorithm"), "Text", xmpText, xmpExternal,
     N_("Content Encryption Algorithm. E.g.: Blowfish")},
    {"ContentSignAlgo", N_("Content Signature Algorithm"), "Text", xmpText, xmpExternal,
     N_("Content Signature Algorithm. E.g.: RSA")},
    {"ContentSignHashAlgo", N_("Content Sign Hash Algorithm"), "Text", xmpText, xmpExternal,
     N_("Content Signature Hash Algorithm. E.g.: SHA1-160 or MD5")},
    {"Contrast", N_("Contrast"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("Indicates the direction of contrast processing applied by the camera.")},
    {"Copyright", N_("Copyright"), "Text", xmpText, xmpExternal,
     N_("Copyright, can be name of an organization or an individual.")},
    {"CostumeDesigner", N_("Costume Designer"), "Text", xmpText, xmpExternal,
     N_("Costume Designer associated with the video.")},
    {"Country", N_("Country"), "Text", xmpText, xmpExternal, N_("Name of the country where the video was created.")},
    {"CreationDate", N_("Creation Date"), "Integer", xmpText, xmpExternal,
     N_("Specifies the date and time of the initial creation of the file. The value is given as the "
        "number of 100-nanosecond intervals since January 1, 1601, according to Coordinated Universal Time "
        "(Greenwich Mean Time).")},
    {"CropBottom", N_("Pixel Crop Bottom"), "Integer", xmpText, xmpExternal,
     N_("Number of Pixels to be cropped from the bottom.")},
    {"CropLeft", N_("Pixel Crop Left"), "Integer", xmpText, xmpExternal,
     N_("Number of Pixels to be cropped from the left.")},
    {"CropRight", N_("Pixel Crop Right"), "Integer", xmpText, xmpExternal,
     N_("Number of Pixels to be cropped from the right.")},
    {"Cropped", N_("Cropped"), "Integer", xmpText, xmpExternal, N_("Field that indicates if a video is cropped.")},
    {"CropTop", N_("Pixel Crop Top"), "Integer", xmpText, xmpExternal,
     N_("Number of Pixels to be cropped from the top.")},
    {"CurrentTime", N_("Current Time"), "Integer", xmpText, xmpExternal,
     N_("The time value for current time position within the movie.")},
    {"DataPackets", N_("Data Packets"), "Integer", xmpText, xmpExternal,
     N_("Specifies the number of Data Packet entries that exist within the Data Object.")},
    {"DateTimeOriginal", N_("Date and Time Original"), "Date", xmpText, xmpInternal,
     N_("Date and time when original video was generated, in ISO 8601 format.")},
    {"DateTimeDigitized", N_("Date and Time Digitized"), "Date", xmpText, xmpInternal,
     N_("Date and time when video was stored as digital data, can be the same "
        "as DateTimeOriginal if originally stored in digital form. Stored in ISO 8601 format.")},
    {"DateUTC", N_("Date-Time Original"), "Text", xmpText, xmpExternal, N_("Contains the production date")},
    {"DefaultOn", N_("Video Track Default On"), "Text", xmpText, xmpExternal,
     N_("Video Track Default On , i.e. Enabled/Disabled")},
    {"DigitalZoomRatio", N_("Digital Zoom Ratio"), "Rational", xmpText, xmpInternal,
     N_("Indicates the digital zoom ratio when the video was shot.")},
    {"Dimensions", N_("Dimensions"), "Text", xmpText, xmpExternal,
     N_("Information about the Dimensions of the video frame.")},
    {"Director", N_("Director"), "Text", xmpText, xmpExternal, N_("Information about the Director.")},
    {"DisplayUnit", N_("Video Display Unit"), "Text", xmpText, xmpExternal,
     N_("Video display unit. E.g.: cm, pixels, inch")},
    {"DistributedBy", N_("Distributed By"), "Text", xmpText, xmpExternal,
     N_("Distributed By, i.e. name of person or organization.")},
    {"DocType", N_("Doc Type"), "Text", xmpText, xmpExternal,
     N_("Describes the contents of the file. In the case of a MATROSKA file, its value is 'matroska'")},
    {"DocTypeReadVersion", N_("Doc Type Read Version"), "Integer", xmpText, xmpExternal,
     N_("A Matroska video specific property, helps in determining the compatibility of file with a particular "
        "version of a video player")},
    {"DocTypeVersion", N_("Doc Type Version"), "Integer", xmpText, xmpExternal,
     N_("A Matroska video specific property, indicated the version of filetype, helps in determining the "
        "compatibility")},
    {"DotsPerInch", N_("Dots Per Inch"), "Integer", xmpText, xmpExternal, N_("Dots Per Inch")},
    {"duration", N_("Duration"), "Integer", xmpText, xmpExternal,
     N_("The duration of the media file. Measured in milli-seconds.")},
    {"EBMLReadVersion", N_("EBML Read Version"), "Integer", xmpText, xmpExternal,
     N_("Extensible Binary Meta Language Read Version")},
    {"EBMLVersion", N_("EBML Version"), "Integer", xmpText, xmpExternal, N_("Extensible Binary Meta Language Version")},
    {"Edit1", N_("Edit Block 1 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit2", N_("Edit Block 2 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit3", N_("Edit Block 3 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit4", N_("Edit Block 4 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit5", N_("Edit Block 5 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit6", N_("Edit Block 6 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit7", N_("Edit Block 7 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit8", N_("Edit Block 8 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"Edit9", N_("Edit Block 9 / Language"), "Text", xmpText, xmpExternal,
     N_("Information about the Edit / Language.")},
    {"EditedBy", N_("Edited By"), "Text", xmpText, xmpExternal, N_("Edited By, i.e. name of person or organization.")},
    {"Enabled", N_("Video Track Enabled"), "Text", xmpText, xmpExternal,
     N_("Status of Video Track, i.e. Enabled/Disabled")},
    {"EncodedBy", N_("Encoded By"), "Text", xmpText, xmpExternal,
     N_("Encoded By, i.e. name of person or organization.")},
    {"EncodedPixelsWidth", N_("Encoded Pixels Width"), "Integer", xmpText, xmpExternal,
     N_("Encoded Pixels width in pixels")},
    {"EncodedPixelsHeight", N_("Encoded Pixels Height"), "Integer", xmpText, xmpExternal,
     N_("Encoded Pixels height in pixels")},
    {"Encoder", N_("Encoder"), "Text", xmpText, xmpExternal, N_("Information about the Encoder.")},
    {"EndTimecode", N_("End Timecode"), "Integer", xmpText, xmpExternal, N_("End Timecode")},
    {"Engineer", N_("Engineer"), "Text", xmpText, xmpExternal, N_("Engineer, in most cases name of person.")},
    {"Equipment", N_("Equipment"), "Text", xmpText, xmpExternal,
     N_("Information about the Equipment used for recording Video.")},
    {"ExposureCompensation", N_("Exposure Compensation"), "Text", xmpText, xmpExternal,
     N_("Exposure Compensation Information.")},
    {"ExposureProgram", N_("Exposure Program"), "Text", xmpText, xmpExternal, N_("Exposure Program Information.")},
    {"ExposureTime", N_("Exposure Time"), "Rational", xmpText, xmpInternal, N_("Exposure time in seconds.")},
    {"ExtendedContentDescription", N_("Extended Content Description"), "Text", xmpSeq, xmpExternal,
     N_("Extended Content Description, usually found in ASF type files.")},
    {"FileDataRate", N_("File Data Rate"), "Rational", xmpText, xmpExternal,
     N_("The file data rate in megabytes per second. For example: \"36/10\" = 3.6 MB/sec")},
    {"FileID", N_("File ID"), "Text", xmpText, xmpExternal, N_("File ID.")},
    {"FileLength", N_("File Length"), "Integer", xmpText, xmpInternal, N_("File length.")},
    {"FileName", N_("File Name"), "Text", xmpText, xmpExternal, N_("File Name or Absolute File Path")},
    {"FileSize", N_("File Size"), "Integer", xmpText, xmpExternal, N_("File Size, in MB")},
    {"FileType", N_("File Type"), "Text", xmpText, xmpExternal, N_("Extension of File or Type of File")},
    {"FilterEffect", N_("Filter Effect"), "Text", xmpText, xmpExternal, N_("Filter Effect Settings Applied.")},
    {"FirmwareVersion", N_("Firmware Version"), "Text", xmpText, xmpExternal,
     N_("Firmware Version of the Camera/Video device.")},
    {"FNumber", N_("F Number"), "Rational", xmpText, xmpInternal, N_("F number. Camera Lens specific data.")},
    {"FocalLength", N_("Focal Length"), "Rational", xmpText, xmpInternal,
     N_("Focal length of the lens, in millimeters.")},
    {"FocusMode", N_("Focus Mode"), "Text", xmpText, xmpExternal,
     N_("Focus Mode of the Lens. E.g.: AF for Auto Focus")},
    {"Format", N_("Format"), "Text", xmpText, xmpExternal,
     N_("Indication of movie format (computer-generated, digitized, and so on).")},
    {"FrameCount", N_("Frame Count"), "Integer", xmpText, xmpExternal, N_("Total number of frames in a video")},
    {"FrameHeight", N_("Frame Height"), "Integer", xmpText, xmpExternal, N_("Height of frames in a video")},
    {"FrameRate", N_("Video Frame Rate"), "Frames per Second", xmpText, xmpExternal,
     N_("Rate at which frames are presented in a video (Expressed in fps(Frames per Second))")},
    {"FrameSize", N_("Video Frame Size"), "Dimensions", xmpText, xmpExternal,
     N_("The frame size. For example: w:720, h: 480, unit:pixels")},
    {"FrameWidth", N_("Frame Width"), "Integer", xmpText, xmpExternal, N_("Width of frames in a video")},
    {"Genre", N_("Genre"), "Text", xmpText, xmpExternal, N_("The name of the genre.")},
    {"GPSAltitude", N_("GPS Altitude"), "Rational", xmpText, xmpInternal,
     N_("GPS tag 6, 0x06. Indicates altitude in meters.")},
    {"GPSAltitudeRef", N_("GPS Altitude Reference"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("GPS tag 5, 0x05. Indicates whether the altitude is above or below sea level.")},
    {"GPSCoordinates", N_("GPS Coordinates"), "Text", xmpText, xmpExternal,
     N_("Information about the GPS Coordinates.")},
    {"GPSDateStamp", N_("GPS Time Stamp"), "Date", xmpText, xmpInternal, N_("Date stamp of GPS data.")},
    {"GPSImgDirection", N_("GPS Image Direction"), "Rational", xmpText, xmpInternal,
     N_("Direction of image when captured, values range from 0 to 359.99.")},
    {"GPSImgDirectionRef", N_("GPS Image Direction Reference"), "Closed Choice of Text", xmpText, xmpInternal,
     N_("Reference for image direction.")},
    {"GPSLatitude", N_("GPS Latitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("(North/South). Indicates latitude.")},
    {"GPSLongitude", N_("GPS Longitude"), "GPSCoordinate", xmpText, xmpInternal,
     N_("(East/West). Indicates longitude.")},
    {"GPSMapDatum", N_("GPS Map Datum"), "Text", xmpText, xmpInternal, N_("Geodetic survey data.")},
    {"GPSSatellites", N_("GPS Satellites"), "Text", xmpText, xmpInternal,
     N_("Satellite information, format is unspecified.")},
    {"GPSTimeStamp", N_("GPS Time Stamp"), "Date", xmpText, xmpInternal, N_("Time stamp of GPS data.")},
    {"GPSVersionID", N_("GPS Version ID"), "Text", xmpText, xmpInternal,
     N_("A decimal encoding with period separators.")},
    {"GraphicsMode", N_("Graphics Mode"), "Text", xmpText, xmpExternal,
     N_("A 16-bit integer that specifies the transfer mode. The transfer mode specifies which Boolean"
        "operation QuickDraw should perform when drawing or transferring an image from one location to another.")},
    {"Grouping", N_("Grouping"), "Text", xmpText, xmpExternal, N_("Information about the Grouping.")},
    {"HandlerClass", N_("Handler Class"), "Text", xmpText, xmpExternal,
     N_("A four-character code that identifies the type of the handler. Only two values are valid for this field: "
        "'mhlr' for media handlers and 'dhlr' for data handlers.")},
    {"HandlerDescription", N_("Handler Description"), "Text", xmpText, xmpExternal,
     N_("A (counted) string that specifies the name of the component-that is, the media handler used when this "
        "media was created..")},
    {"HandlerType", N_("Handler Type"), "Text", xmpText, xmpExternal,
     N_("A four-character code that identifies the type of the media handler or data handler.")},
    {"HandlerVendorID", N_("Handler Vendor ID"), "Text", xmpText, xmpExternal, N_("Component manufacturer.")},
    {"Height", N_("Video Height"), "Integer", xmpText, xmpExternal, N_("Video height in pixels")},
    {"HueAdjustment", N_("Hue Adjustment"), "Integer", xmpText, xmpExternal,
     N_("Hue Adjustment Settings Information.")},
    {"ImageLength", N_("Image Length"), "Integer", xmpText, xmpExternal,
     N_("Image Length, a property inherited from BitMap format")},
    {"InfoBannerImage", N_("Info Banner Image"), "Text", xmpText, xmpExternal, N_("Information Banner Image.")},
    {"InfoBannerURL", N_("Info Banner URL"), "Text", xmpText, xmpExternal, N_("Information Banner URL.")},
    {"Information", N_("Information"), "Text", xmpText, xmpExternal, N_("Additional Movie Information.")},
    {"InfoText", N_("Info Text"), "Text", xmpText, xmpExternal, N_("Information Text.")},
    {"InfoURL", N_("Info URL"), "Text", xmpText, xmpExternal, N_("Information URL.")},
    {"ISO", N_("ISO Setting"), "Integer", xmpText, xmpExternal, N_("Information about the ISO Setting.")},
    {"ISRCCode", N_("ISRC Code"), "Text", xmpText, xmpExternal, N_("Information about the ISRC Code.")},
    {"Junk", N_("Junk Data"), "Text", xmpText, xmpExternal, N_("Video Junk data")},
    {"Language", N_("Language"), "Text", xmpText, xmpExternal, N_("Language.")},
    {"Length", N_("Length"), "Integer", xmpText, xmpExternal, N_("The length of the media file.")},
    {"LensModel", N_("Lens Model"), "Text", xmpText, xmpExternal, N_("Lens Model.")},
    {"LensType", N_("Lens Type"), "Text", xmpText, xmpExternal, N_("Lens Type.")},
    {"Lightness", N_("Lightness"), "Text", xmpText, xmpExternal, N_("Lightness.")},
    {"LocationInfo", N_("Location Information"), "Text", xmpText, xmpExternal, N_("Location Information.")},
    {"LogoIconURL", N_("Logo Icon URL"), "Text", xmpText, xmpExternal, N_("A C string that specifies Logo Icon URL.")},
    {"LogoURL", N_("Logo URL"), "Text", xmpText, xmpExternal, N_("A C string that specifies a Logo URL.")},
    {"Lyrics", N_("Lyrics"), "Text", xmpText, xmpExternal, N_("Lyrics of a Song/Video.")},
    {"MajorBrand", N_("QTime Major FileType Brand"), "Text", xmpText, xmpExternal,
     N_("QuickTime Major File Type Brand")},
    {"Make", N_("Equipment Make"), "Text", xmpText, xmpExternal, N_("Manufacturer of recording equipment")},
    {"MakerNoteType", N_("Camera Maker Note Type"), "Text", xmpText, xmpExternal, N_("Maker Note Type of the camera.")},
    {"MakerNoteVersion", N_("Camera Maker Note Version"), "Text", xmpText, xmpExternal,
     N_("Maker Note Version of the camera.")},
    {"MakerURL", N_("Maker URL"), "Text", xmpText, xmpExternal, N_("Camera Manufacturer's URL.")},
    {"MaxApertureValue", N_("Maximum Aperture Value"), "Rational", xmpText, xmpInternal,
     N_("Smallest F number of lens, in APEX.")},
    {"MaxBitRate", N_("Maximum Bit Rate"), "Integer", xmpText, xmpExternal,
     N_("Specifies the maximum instantaneous bit rate in bits per second for the entire file. This shall equal the "
        "sum of the bit rates of the individual digital media streams.")},
    {"MaxDataRate", N_("Maximum Data Rate"), "kiloBytes per Second", xmpText, xmpExternal,
     N_("Peak rate at which data is presented in a video (Expressed in kB/s(kiloBytes per Second))")},
    {"MediaCreateDate", N_("Media Track Create Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was "
        "created.")},
    {"MediaDuration", N_("Media Track Duration"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the duration of this media (in the movie's time coordinate system).")},
    {"MediaHeaderVersion", N_("Media Header Version"), "Text", xmpText, xmpExternal,
     N_("A 1-byte specification of the version of this media header")},
    {"MediaLangCode", N_("Media Language Code"), "Integer", xmpText, xmpExternal,
     N_("A 16-bit integer that specifies the language code for this media.")},
    {"MediaModifyDate", N_("Media Track Modify Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was "
        "last modified.")},
    {"MediaTimeScale", N_("Media Time Scale"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the time scale for this media-that is, the number of time units that pass per "
        "second in its time coordinate system.")},
    {"Medium", N_("Medium"), "Text", xmpSeq, xmpExternal, N_("Medium.")},
    {"Metadata", N_("Metadata"), "Text", xmpSeq, xmpExternal,
     N_("An array of Unknown / Unregistered Metadata Tags and their values.")},
    {"MetadataLibrary", N_("Metadata Library"), "Text", xmpSeq, xmpExternal,
     N_("An array of Unregistered Metadata Library Tags and their values.")},
    {"MeteringMode", N_("Metering Mode"), "Closed Choice of Integer", xmpText, xmpInternal, N_("Metering mode.")},
    {"MicroSecPerFrame", N_("Micro Seconds Per Frame"), "Integer", xmpText, xmpExternal,
     N_("Number of micro seconds per frame, or frame rate")},
    {"MimeType", N_("Mime Type"), "Text", xmpText, xmpExternal, N_("Tells about the video format")},
    {"MinorVersion", N_("QTime Minor FileType Version"), "Text", xmpText, xmpExternal,
     N_("QuickTime Minor File Type Version")},
    {"Model", N_("Equipment Model"), "Text", xmpText, xmpExternal, N_("Model name or number of equipment.")},
    {"ModificationDate", N_("Modification Date-Time"), "Text", xmpText, xmpExternal,
     N_("Contains the modification date of the video")},
    {"MovieHeaderVersion", N_("Movie Header Version"), "Integer", xmpText, xmpExternal, N_("Movie Header Version")},
    {"MusicBy", N_("Music By"), "Text", xmpText, xmpExternal, N_("Music By, i.e. name of person or organization.")},
    {"MuxingApp", N_("Muxing App"), "Text", xmpText, xmpExternal,
     N_("Contains the name of the library that has been used to create the file (like \"libmatroska 0.7.0\")")},
    {"Name", N_("Name"), "Text", xmpText, xmpExternal, N_("Name of song or the event.")},
    {"NextTrackID", N_("Next Track ID"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates a value to use for the track ID number of the next track added to this "
        "movie. Note that 0 is not a valid track ID value.")},
    {"NumOfColours", N_("Number Of Colours"), "Integer/Text", xmpText, xmpExternal, N_("Total number of colours used")},
    {"NumOfImpColours", N_("Number Of Important Colours"), "Integer/Text", xmpText, xmpExternal,
     N_("Number Of Important Colours, a property inherited from BitMap format")},
    {"NumOfParts", N_("Number Of Parts"), "Integer", xmpText, xmpExternal, N_("Total number of parts in the video.")},
    {"OpColor", N_("Operation Colours"), "Integer/Text", xmpText, xmpExternal,
     N_("Three 16-bit values that specify the red, green, and blue colors for the transfer mode operation "
        "indicated in the graphics mode field.")},
    {"Organization", N_("Organization"), "Text", xmpText, xmpExternal,
     N_("Name of organization associated with the video.")},
    {"Orientation", N_("Orientation"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("Video Orientation:"
        "1 = Horizontal (normal) "
        "2 = Mirror horizontal "
        "3 = Rotate 180 "
        "4 = Mirror vertical "
        "5 = Mirror horizontal and rotate 270 CW "
        "6 = Rotate 90 CW "
        "7 = Mirror horizontal and rotate 90 CW "
        "8 = Rotate 270 CW")},
    {"Part", N_("Part"), "Text", xmpText, xmpExternal, N_("Part.")},
    {"Performers", N_("Performers"), "Text", xmpText, xmpExternal, N_("Performers involved in the video.")},
    {"PerformerKeywords", N_("Performer Keywords"), "Text", xmpText, xmpExternal, N_("Performer Keywords.")},
    {"PerformerURL", N_("Performer URL"), "Text", xmpText, xmpExternal, N_("Performer's dedicated URL.")},
    {"PictureControlData", N_("Picture Control Data"), "Text", xmpText, xmpExternal, N_("Picture Control Data.")},
    {"PictureControlVersion", N_("Picture Control Version"), "Text", xmpText, xmpExternal,
     N_("Picture Control Data Version.")},
    {"PictureControlName", N_("Picture Control Name"), "Text", xmpText, xmpExternal, N_("Picture Control Name.")},
    {"PictureControlBase", N_("Picture Control Base"), "Text", xmpText, xmpExternal, N_("Picture Control Data Base.")},
    {"PictureControlAdjust", N_("Picture Control Adjust"), "Text", xmpText, xmpExternal,
     N_("Picture Control Adjust Information.")},
    {"PictureControlQuickAdjust", N_("Picture Control Quick Adjust"), "Text", xmpText, xmpExternal,
     N_("Picture Control Quick Adjustment Settings.")},
    {"PlaySelection", N_("Play Selection"), "Text", xmpText, xmpExternal, N_("Play Selection.")},
    {"PlayMode", N_("Play Mode"), "Text", xmpText, xmpExternal, N_("Information about the Play Mode.")},
    {"PhysicalEquivalent", N_("Chapter Physical Equivalent"), "Text", xmpText, xmpExternal,
     N_("Contains the information of External media.")},
    {"PixelDepth", N_("Video Pixel Depth"), "closed Choice of Text", xmpText, xmpExternal,
     N_("The size in bits of each color component of a pixel. Standard Windows 32-bit "
        "pixels have 8 bits per component. One of: 8Int, 16Int, 32Int, 32Float.")},
    {"PixelPerMeterX", N_("Pixels Per Meter X"), "Integer", xmpText, xmpExternal,
     N_("Pixels Per Meter X, a property inherited from BitMap format")},
    {"PixelPerMeterY", N_("Pixels Per Meter Y"), "Integer", xmpText, xmpExternal,
     N_("Pixels Per Meter Y, a property inherited from BitMap format")},
    {"Planes", N_("Planes"), "Integer", xmpText, xmpExternal, N_("The number of Image Planes in the video")},
    {"PosterTime", N_("Poster Time"), "Integer", xmpText, xmpExternal,
     N_("The time value of the time of the movie poster.")},
    {"PreferredRate", N_("Preferred Rate"), "Rational", xmpText, xmpExternal,
     N_("A 32-bit fixed-point number that specifies the rate at which to play this movie. A value of 1.0 indicates "
        "normal rate.")},
    {"PreferredVolume", N_("Preferred Volume"), "Rational", xmpText, xmpExternal,
     N_("A 16-bit fixed-point number that specifies how loud to play this movie's sound. A value of 1.0 indicates "
        "full volume.")},
    {"Preroll", N_("Preroll"), "Integer", xmpText, xmpExternal,
     N_("Specifies the amount of time to buffer data before starting to play the file, in millisecond units. If "
        "this value is nonzero,"
        "the Play Duration field and all of the payload Presentation Time fields have been offset by this amount. "
        "Therefore, player software "
        "must subtract the value in the preroll field from the play duration and presentation times to calculate "
        "their actual values.")},
    {"PreviewAtomType", N_("Preview Atom Type"), "Text", xmpText, xmpExternal,
     N_("Indicates the type of atom that contains the preview data")},
    {"PreviewDate", N_("Preview Date"), "Integer", xmpText, xmpExternal,
     N_("The date of the movie preview in local time converted from UTC")},
    {"PreviewDuration", N_("Preview Duration"), "Integer", xmpText, xmpExternal,
     N_("The duration of the movie preview in movie time scale units")},
    {"PreviewTime", N_("Preview Time"), "Integer", xmpText, xmpExternal,
     N_("The time value in the movie at which the preview begins.")},
    {"PreviewVersion", N_("Preview Version"), "Integer", xmpText, xmpExternal, N_("The version of the movie preview")},
    {"ProducedBy", N_("Produced By"), "Text", xmpText, xmpExternal,
     N_("Produced By, i.e. name of person or organization.")},
    {"Producer", N_("Producer"), "Text", xmpText, xmpExternal, N_("Producer involved with the video.")},
    {"ProducerKeywords", N_("Producer Keywords"), "Text", xmpText, xmpExternal,
     N_("Information about the Producer Keywords.")},
    {"ProductionApertureWidth", N_("Production Aperture Width"), "Integer", xmpText, xmpExternal,
     N_("Production aperture width in pixels")},
    {"ProductionApertureHeight", N_("Production Aperture Height"), "Integer", xmpText, xmpExternal,
     N_("Production aperture height in pixels")},
    {"ProductionDesigner", N_("Production Designer"), "Text", xmpText, xmpExternal,
     N_("Information about the Production Designer.")},
    {"ProductionStudio", N_("Production Studio"), "Text", xmpText, xmpExternal,
     N_("Information about the Production Studio.")},
    {"Product", N_("Product"), "Text", xmpText, xmpExternal, N_("Product.")},
    {"ProjectRef", N_("Project Reference"), "ProjectLink", xmpText, xmpExternal,
     N_("A reference to the project that created this file.")},
    {"Rate", N_("Rate"), "Integer", xmpText, xmpExternal, N_("Rate.")},
    {"Rated", N_("Rated"), "Text", xmpText, xmpExternal, N_("The age circle required for viewing the video.")},
    {"Rating", N_("Rating"), "Text", xmpText, xmpExternal, N_("Rating, e.g. 7  or 8 (generally out of 10).")},
    {"RecordLabelName", N_("Record Label Name"), "Text", xmpText, xmpExternal,
     N_("Record Label Name, or the name of the organization recording the video.")},
    {"RecordLabelURL", N_("Record Label URL"), "Text", xmpText, xmpExternal, N_("Record Label URL.")},
    {"RecordingCopyright", N_("Recording Copyright"), "Text", xmpText, xmpExternal, N_("Recording Copyright.")},
    {"Requirements", N_("Requirements"), "Text", xmpText, xmpExternal, N_("Information about the Requirements.")},
    {"ResolutionUnit", N_("Resolution Unit"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("Unit used for XResolution and YResolution. Value is one of: 2 = inches; 3 = centimeters.")},
    {"RippedBy", N_("Ripped By"), "Text", xmpText, xmpExternal, N_("Ripped By, i.e. name of person or organization.")},
    {"Saturation", N_("Saturation"), "Closed Choice of Integer", xmpText, xmpInternal,
     N_("Indicates the direction of saturation processing applied by the camera.")},
    {"SecondaryGenre", N_("Secondary Genre"), "Text", xmpText, xmpExternal, N_("The name of the secondary genre.")},
    {"SelectionTime", N_("Selection Time"), "Integer", xmpText, xmpExternal,
     N_("The time value for the start time of the current selection.")},
    {"SelectionDuration", N_("Selection Duration"), "Integer", xmpText, xmpExternal,
     N_("The duration of the current selection in movie time scale units.")},
    {"SendDuration", N_("Send Duration"), "Integer", xmpText, xmpExternal,
     N_("Specifies the time needed to send the file in 100-nanosecond units. This value should "
        "include the duration of the last packet in the content.")},
    {"Sharpness", N_("Sharpness"), "Integer", xmpText, xmpExternal, N_("\"Sharpness\" setting. Range 0 to +100.")},
    {"Software", N_("Software"), "Text", xmpText, xmpExternal, N_("Software used to generate / create Video data.")},
    {"SoftwareVersion", N_("Software Version"), "Text", xmpText, xmpExternal, N_("The Version of the software used.")},
    {"SongWriter", N_("Song Writer"), "Text", xmpText, xmpExternal, N_("The name of the song writer.")},
    {"SongWriterKeywords", N_("Song Writer Keywords"), "Text", xmpText, xmpExternal, N_("Song Writer Keywords.")},
    {"Source", N_("Source"), "Text", xmpText, xmpExternal, N_("Source.")},
    {"SourceCredits", N_("Source Credits"), "Text", xmpText, xmpExternal, N_("Source Credits.")},
    {"SourceForm", N_("Source Form"), "Text", xmpText, xmpExternal, N_("Source Form.")},
    {"SourceImageHeight", N_("Source Image Height"), "Integer", xmpText, xmpExternal, N_("Video height in pixels")},
    {"SourceImageWidth", N_("Source Image Width"), "Integer", xmpText, xmpExternal, N_("Video width in pixels")},
    {"Starring", N_("Starring"), "Text", xmpText, xmpExternal,
     N_("Starring, name of famous people appearing in the video.")},
    {"StartTimecode", N_("Start Timecode"), "Integer", xmpText, xmpExternal, N_("Start Timecode")},
    {"Statistics", N_("Statistics"), "Text", xmpText, xmpExternal, N_("Statistics.")},
    {"StreamCount", N_("Stream Count"), "Integer", xmpText, xmpExternal, N_("Total Number Of Streams")},
    {"StreamName", N_("Stream Name"), "Text", xmpText, xmpExternal,
     N_("Describes the Stream Name. E.g.: FUJIFILM AVI STREAM 0100")},
    {"StreamQuality", N_("Stream Quality"), "Integer", xmpText, xmpExternal, N_("General Stream Quality")},
    {"StreamSampleRate", N_("Stream Sample Rate"), "Rational", xmpText, xmpExternal, N_("Stream Sample Rate")},
    {"StreamSampleCount", N_("Stream Sample Count"), "Integer", xmpText, xmpExternal, N_("Stream Sample Count")},
    {"StreamSampleSize", N_("Stream Sample Size"), "Integer", xmpText, xmpExternal, N_("General Stream Sample Size")},
    {"StreamType", N_("Stream Type"), "Text", xmpText, xmpExternal,
     N_("Describes the Stream Type. E.g.: Video, Audio or Subtitles")},
    {"SubTCodec", N_("Subtitles Codec"), "Text", xmpText, xmpExternal,
     N_("Subtitles stream codec, for general purpose")},
    {"SubTCodecDecodeAll", N_("Subtitle Codec Decode Info"), "Text", xmpText, xmpExternal,
     N_("Contains information the Subtitles codec decode all, i.e. Enabled/Disabled")},
    {"SubTCodecInfo", N_("Subtitles Codec Information"), "Text", xmpText, xmpExternal,
     N_("Contains additional information about subtitles.")},
    {"SubTCodecDownloadUrl", N_("Subtitle Codec Download URL"), "Text", xmpText, xmpExternal,
     N_("Video Subtitle Codec Download URL.")},
    {"SubTCodecSettings", N_("Subtitle Codec Settings"), "Text", xmpText, xmpExternal,
     N_("Contains settings the codec needs before decoding can be started.")},
    {"SubTDefaultOn", N_("Subtitle Track Default On"), "Text", xmpText, xmpExternal,
     N_("Subtitles Track Default On , i.e. Enabled/Disabled")},
    {"SubTEnabled", N_("Subtitle Track Enabled"), "Text", xmpText, xmpExternal,
     N_("Status of Subtitles Track, i.e. Enabled/Disabled")},
    {"Subtitle", N_("Subtitle"), "Text", xmpText, xmpExternal, N_("Subtitle of the video.")},
    {"SubtitleKeywords", N_("Subtitle Keywords"), "Text", xmpText, xmpExternal, N_("Subtitle Keywords.")},
    {"SubTLang", N_("Subtitles Language"), "Text", xmpText, xmpExternal,
     N_("The Language in which the subtitles is recorded in.")},
    {"SubTTrackForced", N_("Subtitle Track Forced"), "Text", xmpText, xmpExternal,
     N_("Subtitles Track Forced , i.e. Enabled/Disabled")},
    {"SubTTrackLacing", N_("Subtitle Track Lacing"), "Text", xmpText, xmpExternal,
     N_("Subtitles Track Lacing , i.e. Enabled/Disabled")},
    {"Subject", N_("Subject"), "Text", xmpText, xmpExternal, N_("Subject.")},
    {"TapeName", N_("Tape Name"), "Text", xmpText, xmpExternal, N_("TapeName.")},
    {"TagDefault", N_("Tag Default Setting"), "Text", xmpText, xmpExternal,
     N_("If Tag is Default enabled, this value is Yes, else No")},
    {"TagLanguage", N_("Tag Language"), "Text", xmpText, xmpExternal, N_("Language that has been used to define tags")},
    {"TagName", N_("Tag Name"), "Text", xmpText, xmpExternal,
     N_("Tags could be used to define several titles for a segment.")},
    {"TagString", N_("Tag String"), "Text", xmpText, xmpExternal, N_("Information contained in a Tags")},
    {"TargetType", N_("Target Type"), "Text", xmpText, xmpExternal,
     N_("A string describing the logical level of the object the Tag is referring to.")},
    {"Technician", N_("Technician"), "Text", xmpText, xmpExternal, N_("Technician, in most cases name of person.")},
    {"ThumbnailHeight", N_("Thumbnail Height"), "Integer", xmpText, xmpExternal, N_("Preview Image Thumbnail Height.")},
    {"ThumbnailLength", N_("Thumbnail Length"), "Integer", xmpText, xmpExternal, N_("Preview Image Thumbnail Length.")},
    {"ThumbnailWidth", N_("Thumbnail Width"), "Integer", xmpText, xmpExternal, N_("Preview Image Thumbnail Width.")},
    {"TimecodeScale", N_("Timecode Scale"), "Rational", xmpText, xmpExternal,
     N_("Multiplying factor which is helpful in calculation of a particular timecode")},
    {"TimeOffset", N_("Time Offset"), "Integer", xmpText, xmpExternal,
     N_("Specifies the presentation time offset of the stream in 100-nanosecond units. This value shall be equal "
        "to the send time of the first interleaved packet in the data section.")},
    {"TimeScale", N_("Time Scale"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the time scale for this movie-that is, the number of time units that"
        "pass per second in its time coordinate system. A time coordinate system that measures time"
        "in sixtieths of a second, for example, has a time scale of 60.")},
    {"Title", N_("Title"), "Text", xmpText, xmpExternal,
     N_("Contains a general name of the SEGMENT, like 'Lord of the Rings - The Two Towers', however, Tags could be "
        "used to define several titles for a segment.")},
    {"ToningEffect", N_("Toning Effect"), "Text", xmpText, xmpExternal, N_("Toning Effect Settings Applied.")},
    {"TotalFrameCount", N_("Total Frame Count"), "Integer", xmpText, xmpExternal,
     N_("Total number of frames in a video")},
    {"TotalStream", N_("Number Of Streams"), "Integer", xmpText, xmpExternal,
     N_("Total number of streams present in a video. E.g.: Video, Audio or Subtitles")},
    {"Track", N_("Track"), "Text", xmpText, xmpExternal, N_("Information about the Track.")},
    {"TrackCreateDate", N_("Video Track Create Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was "
        "created.")},
    {"TrackDuration", N_("Video Track Duration"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the duration of this track (in the movie's time coordinate system).")},
    {"TrackForced", N_("Video Track Forced"), "Text", xmpText, xmpExternal,
     N_("Video Track Forced , i.e. Enabled/Disabled")},
    {"TrackID", N_("Track ID"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that uniquely identifies the track. The value 0 cannot be used.")},
    {"TrackHeaderVersion", N_("Track Header Version"), "Text", xmpText, xmpExternal,
     N_("A 1-byte specification of the version of this track header")},
    {"TrackLacing", N_("Video Track Lacing"), "Text", xmpText, xmpExternal,
     N_("Video Track Lacing , i.e. Enabled/Disabled")},
    {"TrackLang", N_("Track Language"), "Text", xmpText, xmpExternal,
     N_("The Language in which a particular stream is recorded in.")},
    {"TrackLayer", N_("Video Track Layer"), "Integer", xmpText, xmpExternal,
     N_("A 16-bit integer that indicates this track's spatial priority in its movie. The QuickTime Movie"
        "Toolbox uses this value to determine how tracks overlay one another. Tracks with lower layer"
        "values are displayed in front of tracks with higher layer values.")},
    {"TrackModifyDate", N_("Video Track Modify Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was "
        "last modified.")},
    {"TrackName", N_("Track Name"), "Text", xmpText, xmpExternal,
     N_("Track Name could be used to define titles for a segment.")},
    {"TrackNumber", N_("Track Number"), "Integer", xmpText, xmpExternal, N_("Track Number.")},
    {"TrackVolume", N_("Track Volume"), "Rational", xmpText, xmpExternal,
     N_("A 16-bit fixed-point number that specifies how loud to play this track's sound. A value of 1.0 indicates "
        "full volume.")},
    {"TranslateCodec", N_("Chapter Translate Codec"), "Text", xmpText, xmpExternal,
     N_("Chapter Translate Codec information. Usually used in Matroska file type.")},
    {"UnknownInfo", N_("Unknown Information"), "Text", xmpText, xmpExternal,
     N_("Unknown / Unregistered Metadata Tags and their values.")},
    {"UnknownInfo2", N_("Unknown Information"), "Text", xmpText, xmpExternal,
     N_("Unknown / Unregistered Metadata Tags and their values.")},
    {"URL", N_("Video URL"), "Text", xmpText, xmpExternal,
     N_("A C string that specifies a URL. There may be additional data after the C string.")},
    {"URN", N_("Video URN"), "Text", xmpText, xmpExternal,
     N_("A C string that specifies a URN. There may be additional data after the C string.")},
    {"VariProgram", N_("Vari Program"), "Text", xmpText, xmpExternal,
     N_("Software settings used to generate / create Video data.")},
    {"VegasVersionMajor", N_("Vegas Version Major"), "Text", xmpText, xmpExternal, N_("Vegas Version Major.")},
    {"VegasVersionMinor", N_("Vegas Version Minor"), "Text", xmpText, xmpExternal, N_("Vegas Version Minor.")},
    {"Vendor", N_("Vendor"), "Text", xmpText, xmpExternal,
     N_("The developer of the compressor that generated the compressed data.")},
    {"VendorID", N_("Vendor ID"), "Text", xmpText, xmpExternal,
     N_("A 32-bit integer that specifies the developer of the compressor that generated the compressed data. Often "
        "this field contains 'appl' to indicate Apple Computer, Inc.")},
    {"VideoQuality", N_("Video Quality"), "Integer", xmpText, xmpExternal, N_("Video Stream Quality")},
    {"VideoSampleSize", N_("Video Sample Size"), "Integer", xmpText, xmpExternal, N_("Video Stream Sample Size")},
    {"VideoScanType", N_("Video Scan Type"), "Text", xmpText, xmpExternal,
     N_("Video Scan Type, it can be Progressive or Interlaced")},
    {"WatermarkURL", N_("Watermark URL"), "Text", xmpText, xmpExternal,
     N_("A C string that specifies a Watermark URL.")},
    {"WhiteBalance", N_("White Balance"), "Closed Choice Text", xmpText, xmpExternal,
     N_("\"White Balance\" setting. One of: As Shot, Auto, Daylight, Cloudy, Shade, Tungsten, "
        "Fluorescent, Flash, Custom")},
    {"WhiteBalanceFineTune", N_("White Balance Fine Tune"), "Integer", xmpText, xmpExternal,
     N_("White Balance Fine Tune.")},
    {"Width", N_("Video Width"), "Integer", xmpText, xmpExternal, N_("Video width in pixels")},
    {"WindowLocation", N_("Window Location"), "Text", xmpText, xmpExternal,
     N_("Information about the Window Location.")},
    {"WorldTime", N_("World Time"), "Integer", xmpText, xmpExternal, N_("World Time")},
    {"WrittenBy", N_("Written By"), "Text", xmpText, xmpExternal,
     N_("Written By, i.e. name of person or organization.")},
    {"WritingApp", N_("Writing App"), "Text", xmpText, xmpExternal,
     N_("Contains the name of the application used to create the file (like \"mkvmerge 0.8.1\")")},
    {"XResolution", N_("X Resolution"), "Rational", xmpText, xmpInternal,
     N_("Horizontal resolution in pixels per unit.")},
    {"Year", N_("Year"), "Integer", xmpText, xmpExternal, N_("Year in which the video was made.")},
    {"YResolution", N_("Y Resolution"), "Rational", xmpText, xmpInternal,
     N_("Vertical resolution in pixels per unit.")},
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpAudioInfo[] = {
    {"AvgBytePerSec", N_("Average Bytes Per Second"), "Integer", xmpText, xmpExternal,
     N_("Average Bytes Per Second found in audio stream")},
    {"Balance", N_("Balance"), "Integer", xmpText, xmpExternal, N_("Indicates the left-right balance of the audio")},
    {"BitsPerSample", N_("Bits Per Sample/ Bit Rate"), "Integer", xmpText, xmpExternal, N_("Bits per test sample")},
    {"ChannelType", N_("Audio Channel Type"), "Integers", xmpText, xmpExternal,
     N_("The audio channel type. One of: Mono, Stereo, 5.1, 7.1.")},
    {"Codec", N_("Audio Codec"), "Text", xmpText, xmpExternal, N_("Codec used for Audio Encoding/Decoding")},
    {"CodecDecodeAll", N_("Audio Codec Decode Info"), "Text", xmpText, xmpExternal,
     N_("Contains information the audio codec decode all, i.e. Enabled/Disabled")},
    {"CodecDescription", N_("Audio Codec Description"), "Text", xmpText, xmpExternal,
     N_("Contains description the codec.")},
    {"CodecDownloadUrl", N_("Audio Codec Download URL"), "Text", xmpText, xmpExternal, N_("Audio Codec Download URL.")},
    {"CodecInfo", N_("Audio Codec Information"), "Text", xmpText, xmpExternal,
     N_("Contains information the codec needs before decoding can be started. An example is the Vorbis "
        "initialization packets for Vorbis audio.")},
    {"CodecSettings", N_("Audio Codec Settings"), "Text", xmpText, xmpExternal,
     N_("Contains settings the codec needs before decoding can be started.")},
    {"Compressor", N_("Audio Compressor"), "Text", xmpText, xmpExternal,
     N_("The audio compression used. For example, MP3.")},
    {"DefaultDuration", N_("Audio Default Duration"), "Text", xmpText, xmpExternal,
     N_("The number of micro seconds an audio chunk plays.")},
    {"DefaultStream", N_("Audio Default Stream"), "Text", xmpText, xmpExternal,
     N_("Audio Stream that would be played by default.")},
    {"DefaultOn", N_("Audio Track Default On"), "Text", xmpText, xmpExternal,
     N_("Audio Track Default On , i.e. Enabled/Disabled")},
    {"Enabled", N_("Audio Track Enabled"), "Text", xmpText, xmpExternal,
     N_("Status of Audio Track, i.e. Enabled/Disabled")},
    {"Format", N_("Audio Format"), "Text", xmpText, xmpExternal,
     N_("A four-character code that identifies the format of the audio.")},
    {"HandlerClass", N_("Handler Class"), "Text", xmpText, xmpExternal,
     N_("A four-character code that identifies the type of the handler. Only two values are valid for this field: "
        "'mhlr' for media handlers and 'dhlr' for data handlers.")},
    {"HandlerDescription", N_("Handler Description"), "Text", xmpText, xmpExternal,
     N_("A (counted) string that specifies the name of the component-that is, the media handler used when this "
        "media was created..")},
    {"HandlerType", N_("Handler Type"), "Text", xmpText, xmpExternal,
     N_("A four-character code that identifies the type of the media handler or data handler.")},
    {"HandlerVendorID", N_("Handler Vendor ID"), "Text", xmpText, xmpExternal, N_("Component manufacturer.")},
    {"MediaCreateDate", N_("Media Track Create Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was "
        "created.")},
    {"MediaDuration", N_("Media Track Duration"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the duration of this media (in the movie's time coordinate system).")},
    {"MediaHeaderVersion", N_("Media Header Version"), "Text", xmpText, xmpExternal,
     N_("A 1-byte specification of the version of this media header")},
    {"MediaLangCode", N_("Media Language Code"), "Integer", xmpText, xmpExternal,
     N_("A 16-bit integer that specifies the language code for this media.")},
    {"MediaModifyDate", N_("Media Track Modify Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the media header was "
        "last modified.")},
    {"MediaTimeScale", N_("Media Time Scale"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the time scale for this media-that is, the number of time units that pass per "
        "second in its time coordinate system.")},
    {"OutputSampleRate", N_("Output Audio Sample Rate"), "Integer", xmpText, xmpExternal,
     N_("The output audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.")},
    {"SampleCount", N_("Audio Sample Count"), "Integer", xmpText, xmpExternal,
     N_("Sample taken for Analyzing Audio Stream")},
    {"SampleRate", N_("Audio Sample Rate"), "Integer", xmpText, xmpExternal,
     N_("The audio sample rate. Can be any value, but commonly 32000, 41100, or 48000.")},
    {"SampleType", N_("Audio Sample Type"), "closed Choice of Text", xmpText, xmpExternal,
     N_("The audio sample type. One of: 8Int, 16Int, 32Int, 32Float.")},
    {"SchemeTitle", N_("Sound Scheme Title"), "Text", xmpText, xmpExternal, N_("Sound Scheme Title.")},
    {"TimeOffset", N_("Time Offset"), "Integer", xmpText, xmpExternal,
     N_("Specifies the presentation time offset of the stream in 100-nanosecond units. This value shall be equal "
        "to the send time of the first interleaved packet in the data section.")},
    {"TrackCreateDate", N_("Audio Track Create Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was "
        "created.")},
    {"TrackDuration", N_("Audio Track Duration"), "Integer", xmpText, xmpExternal,
     N_("A time value that indicates the duration of this track (in the movie's time coordinate system).")},
    {"TrackForced", N_("Audio Track Forced"), "Text", xmpText, xmpExternal,
     N_("Audio Track Forced , i.e. Enabled/Disabled")},
    {"TrackID", N_("Track ID"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that uniquely identifies the track. The value 0 cannot be used.")},
    {"TrackHeaderVersion", N_("Track Header Version"), "Text", xmpText, xmpExternal,
     N_("A 1-byte specification of the version of this track header")},
    {"TrackLacing", N_("Audio Track Lacing"), "Text", xmpText, xmpExternal,
     N_("Audio Track Lacing , i.e. Enabled/Disabled")},
    {"TrackLang", N_("Track Language"), "Text", xmpText, xmpExternal,
     N_("The Language in which a particular stream is recorded in.")},
    {"TrackLayer", N_("Audio Track Layer"), "Integer", xmpText, xmpExternal,
     N_("A 16-bit integer that indicates this track's spatial priority in its movie. The QuickTime Movie"
        "Toolbox uses this value to determine how tracks overlay one another. Tracks with lower layer"
        "values are displayed in front of tracks with higher layer values.")},
    {"TrackModifyDate", N_("Audio Track Modify Date"), "Integer", xmpText, xmpExternal,
     N_("A 32-bit integer that indicates (in seconds since midnight, January 1, 1904) when the track header was "
        "last modified.")},
    {"TrackVolume", N_("Track Volume"), "Rational", xmpText, xmpExternal,
     N_("A 16-bit fixed-point number that specifies how loud to play this track's sound. A value of 1.0 indicates "
        "full volume.")},
    {"URL", N_("Audio URL"), "Text", xmpText, xmpExternal,
     N_("A C string that specifies a URL. There may be additional data after the C string.")},
    {"URN", N_("Audio URN"), "Text", xmpText, xmpExternal,
     N_("A C string that specifies a URN. There may be additional data after the C string.")},
    {"VendorID", N_("Vendor ID"), "Text", xmpText, xmpExternal,
     N_("A 32-bit integer that specifies the developer of the compressor that generated the compressed data. Often "
        "this field contains 'appl' to indicate Apple Computer, Inc.")},
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpDctermsInfo[] = {
    // dcterms properties used by DwC under the Record Class.
    {"type", N_("Type"), "bag open Choice", xmpBag, xmpExternal, N_("The nature or genre of the resource.")},
    {"modified", N_("Date Modified"), "seq Date", xmpSeq, xmpExternal, N_("Date on which the resource was changed.")},
    {"language", N_("Language"), "bag Locale", xmpBag, xmpExternal, N_("A language of the resource.")},
    {"license", N_("License"), "Lang Alt", langAlt, xmpExternal,
     N_("A legal document giving official permission to do something with the resource.")},
    {"rightsHolder", N_("Rights Holder"), "seq ProperName", xmpSeq, xmpExternal,
     N_("A person or organization owning or managing rights over the resource.")},
    {"accessRights", N_("Access Rights"), "Lang Alt", langAlt, xmpExternal,
     N_("Information about who can access the resource or an indication of its security status.")},
    {"bibliographicCitation", N_("Bibliographic Citation"), "Text", xmpText, xmpExternal,
     N_("A bibliographic reference for the resource.")},
    {"references", N_("References"), "bag Text", xmpBag, xmpExternal,
     N_("A related resource that is referenced, cited, or otherwise pointed to by the described resource.")},
    // Location Level Class
    {
        "Location",
        N_("Location"),
        "Location",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing Darwin Core location based information."),
    },
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpDwCInfo[] = {

    // Record Level Class
    {
        "Record",
        N_("Record"),
        "Record",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing record based information."),
    },
    // Record Level Terms
    {"institutionID", N_("Institution ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the institution having custody of the object(s) or information referred to in the "
        "record.")},
    {"collectionID", N_("Collection ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the collection or dataset from which the record was derived. For physical specimens, "
        "the recommended best practice is to use the identifier in a collections registry such as the Biodiversity "
        "Collections Index (http://www.biodiversitycollectionsindex.org/).")},
    {"institutionCode", N_("Institution Code"), "Text", xmpText, xmpExternal,
     N_("The name (or acronym) in use by the institution having custody of the object(s) or information referred "
        "to in the record.")},
    {"datasetID", N_("Dataset ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the set of data. May be a global unique identifier or an identifier specific to a "
        "collection or institution.")},
    {"collectionCode", N_("Collection Code"), "Text", xmpText, xmpExternal,
     N_("The name, acronym, coden, or initialism identifying the collection or data set from which the record was "
        "derived.")},
    {"datasetName", N_("Dataset Name"), "Text", xmpText, xmpExternal,
     N_("The name identifying the data set from which the record was derived.")},
    {"ownerInstitutionCode", N_("Owner Institution Code"), "Text", xmpText, xmpExternal,
     N_("The name (or acronym) in use by the institution having ownership of the object(s) or information referred "
        "to in the record.")},
    {"basisOfRecord", N_("Basis Of Record"), "Text", xmpText, xmpExternal,
     N_("The specific nature of the data record - a subtype of the type. Recommended best practice is to use a "
        "controlled vocabulary such as the Darwin Core Type Vocabulary "
        "(http://rs.tdwg.org/dwc/terms/type-vocabulary/index.htm).")},
    {"informationWithheld", N_("Information Withheld"), "Text", xmpText, xmpExternal,
     N_("Additional information that exists, but that has not been shared in the given record.")},
    {"dataGeneralizations", N_("Data Generalizations"), "Text", xmpText, xmpExternal,
     N_("Actions taken to make the shared data less specific or complete than in its original form. Suggests that "
        "alternative data of higher quality may be available on request.")},
    {"dynamicProperties", N_("Dynamic Properties"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of additional measurements, facts, characteristics, or assertions "
        "about the record. Meant to provide a mechanism for structured content such as key-value pairs.")},

    // Occurrence Level Class
    {
        "Occurrence",
        N_("Occurrence"),
        "Occurrence",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing occurrence based information."),
    },
    // Occurrence Level Terms
    {"occurrenceID", N_("Occurrence ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the Occurrence (as opposed to a particular digital record of the occurrence). In the "
        "absence of a persistent global unique identifier, construct one from a combination of identifiers in the "
        "record that will most closely make the occurrenceID globally unique.")},
    {"catalogNumber", N_("Catalog Number"), "Text", xmpText, xmpExternal,
     N_("An identifier (preferably unique) for the record within the data set or collection.")},
    {"occurrenceDetails", N_("Occurrence Details"), "Text", xmpText, xmpExternal,
     N_("Deprecated. Details about the Occurrence.")},
    {"occurrenceRemarks", N_("Occurrence Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the Occurrence.")},
    {"recordNumber", N_("Record Number"), "Text", xmpText, xmpExternal,
     N_("An identifier given to the Occurrence at the time it was recorded. Often serves as a link between field "
        "notes and an Occurrence record, such as a specimen collector's number.")},
    {"recordedBy", N_("Recorded By"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of names of people, groups, or organizations responsible for "
        "recording the original Occurrence. The primary collector or observer, especially one who applies a "
        "personal identifier (recordNumber), should be listed first.")},
    {"individualID", N_("Individual ID"), "Text", xmpText, xmpExternal,
     N_("Deprecated. An identifier for an individual or named group of individual organisms represented in the "
        "Occurrence. Meant to accommodate resampling of the same individual or group for monitoring purposes. May "
        "be a global unique identifier or an identifier specific to a data set.")},
    {"individualCount", N_("Individual Count"), "Integer", xmpText, xmpExternal,
     N_("The number of individuals represented present at the time of the Occurrence.")},
    {"organismQuantity", N_("Organism Quantity"), "Text", xmpText, xmpExternal,
     N_("A number or enumeration value for the quantity of organisms.")},
    {"organismQuantityType", N_("Organism Quantity Type"), "Text", xmpText, xmpExternal,
     N_("The type of quantification system used for the quantity of organisms.")},
    {"sex", N_("Sex"), "Text", xmpText, xmpExternal,
     N_("The sex of the biological individual(s) represented in the Occurrence. Recommended best practice is to "
        "use a controlled vocabulary.")},
    {"lifeStage", N_("Life Stage"), "Text", xmpText, xmpExternal,
     N_("The age class or life stage of the biological individual(s) at the time the Occurrence was recorded. "
        "Recommended best practice is to use a controlled vocabulary.")},
    {"reproductiveCondition", N_("Reproductive Condition"), "Text", xmpText, xmpExternal,
     N_("The reproductive condition of the biological individual(s) represented in the Occurrence. Recommended "
        "best practice is to use a controlled vocabulary.")},
    {"behavior", N_("Behavior"), "Text", xmpText, xmpExternal,
     N_("A description of the behavior shown by the subject at the time the Occurrence was recorded. Recommended "
        "best practice is to use a controlled vocabulary.")},
    {"establishmentMeans", N_("Establishment Means"), "Text", xmpText, xmpExternal,
     N_("The process by which the biological individual(s) represented in the Occurrence became established at the "
        "location. Recommended best practice is to use a controlled vocabulary.")},
    {"occurrenceStatus", N_("Occurrence Status"), "Text", xmpText, xmpExternal,
     N_("A statement about the presence or absence of a Taxon at a Location. Recommended best practice is to use a "
        "controlled vocabulary.")},
    {"preparations", N_("Preparations"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of preparations and preservation methods for a specimen.")},
    {"disposition", N_("Disposition"), "Text", xmpText, xmpExternal,
     N_("The current state of a specimen with respect to the collection identified in collectionCode or "
        "collectionID. Recommended best practice is to use a controlled vocabulary.")},
    {"otherCatalogNumbers", N_("Other Catalog Numbers"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of previous or alternate fully qualified catalog numbers or other "
        "human-used identifiers for the same Occurrence, whether in the current or any other data set or "
        "collection.")},
    {"previousIdentifications", N_("Previous Identifications"), "bag Text", xmpBag, xmpExternal,
     N_("Deprecated. A list (concatenated and separated) of previous assignments of names to the Occurrence.")},
    {"associatedMedia", N_("Associated Media"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of identifiers (publication, global unique identifier, URI) of media "
        "associated with the Occurrence.")},
    {"associatedReferences", N_("Associated References"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of identifiers (publication, bibliographic reference, global unique "
        "identifier, URI) of literature associated with the Occurrence.")},
    {"associatedOccurrences", N_("Associated Occurrences"), "bag Text", xmpBag, xmpExternal,
     N_("Deprecated. A list (concatenated and separated) of identifiers of other Occurrence records and their "
        "associations to this Occurrence.")},
    {"associatedSequences", N_("Associated Sequences"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of identifiers (publication, global unique identifier, URI) of "
        "genetic sequence information associated with the Occurrence.")},
    {"associatedTaxa", N_("Associated Taxa"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of identifiers or names of taxa and their associations with the "
        "Occurrence.")},

    // Organism Level Class
    {
        "Organism",
        N_("Organism"),
        "Organism",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing organism based information."),
    },
    // Organism Level Terms
    {"organismID", N_("Organism ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the Organism instance (as opposed to a particular digital record of the Organism). May "
        "be a globally unique identifier or an identifier specific to the data set.")},
    {"organismName", N_("Organism Name"), "Text", xmpText, xmpExternal,
     N_("A textual name or label assigned to an Organism instance.")},
    {"organismScope", N_("Organism Scope"), "Text", xmpText, xmpExternal,
     N_("A description of the kind of Organism instance. Can be used to indicate whether the Organism instance "
        "represents a discrete organism or if it represents a particular type of aggregation. Recommended best "
        "practice is to use a controlled vocabulary.")},
    {"associatedOccurrences", N_("Organism Associated Occurrences"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated with a vertical bar ' | ') of identifiers of other Occurrence records "
        "and their associations to this Occurrence.")},
    {"associatedOrganisms", N_("Associated Organisms"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated with a vertical bar ' | ' ) of identifiers of other Organisms and "
        "their associations to this Organism.")},
    {"previousIdentifications", N_("Previous Identifications"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated with a vertical bar ' | ' ) of previous assignments of names to the "
        "Organism.")},
    {"organismRemarks", N_("Organism Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the Organism instance.")},

    // Material Sample Level Class
    {
        "MaterialSample",
        N_("Material Sample"),
        "MaterialSample",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing material sample based information."),
    },
    {
        "LivingSpecimen",
        N_("Living Specimen"),
        "LivingSpecimen",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing living specimen based information. A specimen that is alive."),
    },
    {
        "PreservedSpecimen",
        N_("Preserved Specimen"),
        "PreservedSpecimen",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing preserved specimen based information. A specimen that has been preserved."),
    },
    {
        "FossilSpecimen",
        N_("Fossil Specimen"),
        "FossilSpecimen",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing fossil specimen based information. A preserved specimen that is a fossil."),
    },
    // Material Sample Level Terms
    {"materialSampleID", N_("Material Sample ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the MaterialSample (as opposed to a particular digital record of the material sample). "
        "In the absence of a persistent global unique identifier, construct one from a combination of identifiers "
        "in the record that will most closely make the materialSampleID globally unique.")},

    // Event Level Class
    {
        "Event",
        N_("Event"),
        "Event",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing event based information."),
    },
    {
        "HumanObservation",
        N_("Human Observation"),
        "HumanObservation",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing human observation based information."),
    },
    {
        "MachineObservation",
        N_("Machine Observation"),
        "MachineObservation",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing machine observation based information."),
    },
    // Event Level Terms
    {"eventID", N_("Event ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the set of information associated with an Event (something that occurs at a place and "
        "time). May be a global unique identifier or an identifier specific to the data set.")},
    {"parentEventID", N_("Parent Event ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the broader Event that groups this and potentially other Events.")},
    {"eventDate", N_("Event Date"), "Date", xmpText, xmpExternal,
     N_("The date-time or interval during which an Event occurred. For occurrences, this is the date-time when the "
        "event was recorded. Not suitable for a time in a geological context. Recommended best practice is to use "
        "an encoding scheme, such as ISO 8601:2004(E).")},
    {"earliestDate", N_("Event Earliest Date"), "Date", xmpText, xmpExternal,
     N_("Deprecated. (Child of Xmp.dwc.Event) The date-time or interval during which an Event started. For "
        "occurrences, this is the date-time when the event was recorded. Not suitable for a time in a geological "
        "context. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")},
    {"latestDate", N_("Event Latest Date"), "Date", xmpText, xmpExternal,
     N_("Deprecated. (Child of Xmp.dwc.Event) The date-time or interval during which an Event ended. For "
        "occurrences, this is the date-time when the event was recorded. Not suitable for a time in a geological "
        "context. Recommended best practice is to use an encoding scheme, such as ISO 8601:2004(E).")},
    {"eventTime", N_("Event Time"), "Date", xmpText, xmpExternal,
     N_("The time or interval during which an Event occurred. Recommended best practice is to use an encoding "
        "scheme, such as ISO 8601:2004(E).")},
    {"startDayOfYear", N_("Start Day Of Year"), "Integer", xmpText, xmpExternal,
     N_("The earliest ordinal day of the year on which the Event occurred (1 for January 1, 365 for December 31, "
        "except in a leap year, in which case it is 366).")},
    {"endDayOfYear", N_("End Day Of Year"), "Integer", xmpText, xmpExternal,
     N_("The latest ordinal day of the year on which the Event occurred (1 for January 1, 365 for December 31, "
        "except in a leap year, in which case it is 366).")},
    {"year", N_("Year"), "Integer", xmpText, xmpExternal,
     N_("The four-digit year in which the Event occurred, according to the Common Era Calendar.")},
    {"month", N_("Month"), "Integer", xmpText, xmpExternal, N_("The ordinal month in which the Event occurred.")},
    {"day", N_("Day"), "Integer", xmpText, xmpExternal,
     N_("The integer day of the month on which the Event occurred.")},
    {"verbatimEventDate", N_("Verbatim Event Date"), "Text", xmpText, xmpExternal,
     N_("The verbatim original representation of the date and time information for an Event.")},
    {"habitat", N_("Habitat"), "Text", xmpText, xmpExternal,
     N_("A category or description of the habitat in which the Event occurred.")},
    {"samplingProtocol", N_("Sampling Protocol"), "Text", xmpText, xmpExternal,
     N_("The name of, reference to, or description of the method or protocol used during an Event.")},
    {"samplingEffort", N_("Sampling Effort"), "Text", xmpText, xmpExternal,
     N_("The amount of effort expended during an Event.")},
    {"sampleSizeValue", N_("Sampling Size Value"), "Text", xmpText, xmpExternal,
     N_("A numeric value for a measurement of the size (time duration, length, area, or volume) of a sample in a "
        "sampling event.")},
    {"sampleSizeUnit", N_("Sampling Size Unit"), "Text", xmpText, xmpExternal,
     N_("The unit of measurement of the size (time duration, length, area, or volume) of a sample in a sampling "
        "event.")},
    {"fieldNumber", N_("Field Number"), "Text", xmpText, xmpExternal,
     N_("An identifier given to the event in the field. Often serves as a link between field notes and the "
        "Event.")},
    {"fieldNotes", N_("Field Notes"), "Text", xmpText, xmpExternal,
     N_("One of (a) an indicator of the existence of, (b) a reference to (publication, URI), or (c) the text of "
        "notes taken in the field about the Event.")},
    {"eventRemarks", N_("Event Remarks"), "Text", xmpText, xmpExternal, N_("Comments or notes about the Event.")},

    // Location Level Class
    {
        "dctermsLocation",
        N_("Location Class"),
        "Location",
        xmpText,
        xmpInternal,
        N_("Deprecated. Use Xmp.dcterms.Location instead. *Main structure* containing location based information."),
    },
    // Location Level Terms
    {"locationID", N_("Location ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the set of location information (data associated with Location). May be a global unique "
        "identifier or an identifier specific to the data set.")},
    {"higherGeographyID", N_("Higher Geography ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the geographic region within which the Location occurred. Recommended best practice is "
        "to use an persistent identifier from a controlled vocabulary such as the Getty Thesaurus of Geographic "
        "Names.")},
    {"higherGeography", N_("Higher Geography"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of geographic names less specific than the information captured in "
        "the locality term.")},
    {"continent", N_("Continent"), "Text", xmpText, xmpExternal,
     N_("The name of the continent in which the Location occurs. Recommended best practice is to use a controlled "
        "vocabulary such as the Getty Thesaurus of Geographic Names or the ISO 3166 Continent code.")},
    {"waterBody", N_("Water Body"), "Text", xmpText, xmpExternal,
     N_("The name of the water body in which the Location occurs. Recommended best practice is to use a controlled "
        "vocabulary such as the Getty Thesaurus of Geographic Names.")},
    {"islandGroup", N_("Island Group"), "Text", xmpText, xmpExternal,
     N_("The name of the island group in which the Location occurs. Recommended best practice is to use a "
        "controlled vocabulary such as the Getty Thesaurus of Geographic Names.")},
    {"island", N_("Island"), "Text", xmpText, xmpExternal,
     N_("The name of the island on or near which the Location occurs. Recommended best practice is to use a "
        "controlled vocabulary such as the Getty Thesaurus of Geographic Names.")},
    {"country", N_("Country"), "Text", xmpText, xmpExternal,
     N_("The name of the country or major administrative unit in which the Location occurs. Recommended best "
        "practice is to use a controlled vocabulary such as the Getty Thesaurus of Geographic Names.")},
    {"countryCode", N_("Country Code"), "Text", xmpText, xmpExternal,
     N_("The standard code for the country in which the Location occurs. Recommended best practice is to use ISO "
        "3166-1-alpha-2 country codes.")},
    {"stateProvince", N_("State Province"), "Text", xmpText, xmpExternal,
     N_("The name of the next smaller administrative region than country (state, province, canton, department, "
        "region, etc.) in which the Location occurs.")},
    {"county", N_("County"), "Text", xmpText, xmpExternal,
     N_("The full, unabbreviated name of the next smaller administrative region than stateProvince (county, shire, "
        "department, etc.) in which the Location occurs.")},
    {"municipality", N_("Municipality"), "Text", xmpText, xmpExternal,
     N_("The full, unabbreviated name of the next smaller administrative region than county (city, municipality, "
        "etc.) in which the Location occurs. Do not use this term for a nearby named place that does not contain "
        "the actual location.")},
    {"locality", N_("Locality"), "Text", xmpText, xmpExternal,
     N_("The specific description of the place. Less specific geographic information can be provided in other "
        "geographic terms (higherGeography, continent, country, stateProvince, county, municipality, waterBody, "
        "island, islandGroup). This term may contain information modified from the original to correct perceived "
        "errors or standardize the description.")},
    {"verbatimLocality", N_("Verbatim Locality"), "Text", xmpText, xmpExternal,
     N_("The original textual description of the place.")},
    {"verbatimElevation", N_("Verbatim Elevation"), "Text", xmpText, xmpExternal,
     N_("The original description of the elevation (altitude, usually above sea level) of the Location.")},
    {"minimumElevationInMeters", N_("Minimum Elevation In Meters"), "Real", xmpText, xmpExternal,
     N_("The lower limit of the range of elevation (altitude, usually above sea level), in meters.")},
    {"maximumElevationInMeters", N_("Maximum Elevation In Meters"), "Real", xmpText, xmpExternal,
     N_("The upper limit of the range of elevation (altitude, usually above sea level), in meters.")},
    {"verbatimDepth", N_("Verbatim Depth"), "Text", xmpText, xmpExternal,
     N_("The original description of the depth below the local surface.")},
    {"minimumDepthInMeters", N_("Minimum Depth In Meters"), "Real", xmpText, xmpExternal,
     N_("The lesser depth of a range of depth below the local surface, in meters.")},
    {"maximumDepthInMeters", N_("Maximum Depth In Meters"), "Real", xmpText, xmpExternal,
     N_("The greater depth of a range of depth below the local surface, in meters.")},
    {"minimumDistanceAboveSurfaceInMeters", N_("Minimum Distance Above Surface In Meters"), "Real", xmpText,
     xmpExternal,
     N_("The lesser distance in a range of distance from a reference surface in the vertical direction, in meters. "
        "Use positive values for locations above the surface, negative values for locations below. If depth "
        "measures are given, the reference surface is the location given by the depth, otherwise the reference "
        "surface is the location given by the elevation.")},
    {"maximumDistanceAboveSurfaceInMeters", N_("Maximum Distance Above Surface In Meters"), "Real", xmpText,
     xmpExternal,
     N_("The greater distance in a range of distance from a reference surface in the vertical direction, in "
        "meters. Use positive values for locations above the surface, negative values for locations below. If "
        "depth measures are given, the reference surface is the location given by the depth, otherwise the "
        "reference surface is the location given by the elevation.")},
    {"locationAccordingTo", N_("Location According To"), "Text", xmpText, xmpExternal,
     N_("Information about the source of this Location information. Could be a publication (gazetteer), "
        "institution, or team of individuals.")},
    {"locationRemarks", N_("Location Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the Location.")},
    {"verbatimCoordinates", N_("Verbatim Coordinates"), "Text", xmpText, xmpExternal,
     N_("The verbatim original spatial coordinates of the Location. The coordinate ellipsoid, geodeticDatum, or "
        "full Spatial Reference System (SRS) for these coordinates should be stored in verbatimSRS and the "
        "coordinate system should be stored in verbatimCoordinateSystem.")},
    {"verbatimLatitude", N_("Verbatim Latitude"), "Text", xmpText, xmpExternal,
     N_("The verbatim original latitude of the Location. The coordinate ellipsoid, geodeticDatum, or full Spatial "
        "Reference System (SRS) for these coordinates should be stored in verbatimSRS and the coordinate system "
        "should be stored in verbatimCoordinateSystem.")},
    {"verbatimLongitude", N_("Verbatim Longitude"), "Text", xmpText, xmpExternal,
     N_("The verbatim original longitude of the Location. The coordinate ellipsoid, geodeticDatum, or full Spatial "
        "Reference System (SRS) for these coordinates should be stored in verbatimSRS and the coordinate system "
        "should be stored in verbatimCoordinateSystem.")},
    {"verbatimCoordinateSystem", N_("Verbatim Coordinate System"), "Text", xmpText, xmpExternal,
     N_("The spatial coordinate system for the verbatimLatitude and verbatimLongitude or the verbatimCoordinates "
        "of the Location. Recommended best practice is to use a controlled vocabulary.")},
    {"verbatimSRS", N_("Verbatim SRS"), "Text", xmpText, xmpExternal,
     N_("The ellipsoid, geodetic datum, or spatial reference system (SRS) upon which coordinates given in "
        "verbatimLatitude and verbatimLongitude, or verbatimCoordinates are based. Recommended best practice is "
        "use the EPSG code as a controlled vocabulary to provide an SRS, if known. Otherwise use a controlled "
        "vocabulary for the name or code of the geodetic datum, if known. Otherwise use a controlled vocabulary "
        "for the name or code of the ellipsoid, if known. If none of these is known, use the value \"unknown\".")},
    {"decimalLatitude", N_("Decimal Latitude"), "Real", xmpText, xmpExternal,
     N_("The geographic latitude (in decimal degrees, using the spatial reference system given in geodeticDatum) "
        "of the geographic center of a Location. Positive values are north of the Equator, negative values are "
        "south of it. Legal values lie between -90 and 90, inclusive.")},
    {"decimalLongitude", N_("Decimal Longitude"), "Real", xmpText, xmpExternal,
     N_("The geographic longitude (in decimal degrees, using the spatial reference system given in geodeticDatum) "
        "of the geographic center of a Location. Positive values are east of the Greenwich Meridian, negative "
        "values are west of it. Legal values lie between -180 and 180, inclusive.")},
    {"geodeticDatum", N_("Geodetic Datum"), "Text", xmpText, xmpExternal,
     N_("The ellipsoid, geodetic datum, or spatial reference system (SRS) upon which the geographic coordinates "
        "given in decimalLatitude and decimalLongitude as based. Recommended best practice is use the EPSG code as "
        "a controlled vocabulary to provide an SRS, if known. Otherwise use a controlled vocabulary for the name "
        "or code of the geodetic datum, if known. Otherwise use a controlled vocabulary for the name or code of "
        "the ellipsoid, if known. If none of these is known, use the value \"unknown\".")},
    {"coordinateUncertaintyInMeters", N_("Coordinate Uncertainty In Meters"), "Real", xmpText, xmpExternal,
     N_("The horizontal distance (in meters) from the given decimalLatitude and decimalLongitude describing the "
        "smallest circle containing the whole of the Location. Leave the value empty if the uncertainty is "
        "unknown, cannot be estimated, or is not applicable (because there are no coordinates). Zero is not a "
        "valid value for this term.")},
    {"coordinatePrecision", N_("Coordinate Precision"), "Text", xmpText, xmpExternal,
     N_("A decimal representation of the precision of the coordinates given in the decimalLatitude and "
        "decimalLongitude.")},
    {"pointRadiusSpatialFit", N_("Point Radius Spatial Fit"), "Real", xmpText, xmpExternal,
     /* xgettext:no-c-format */
     N_("The ratio of the area of the point-radius (decimalLatitude, decimalLongitude, "
        "coordinateUncertaintyInMeters) to the area of the true (original, or most specific) spatial "
        "representation of the Location. Legal values are 0, greater than or equal to 1, or undefined. A value of "
        "1 is an exact match or 100% overlap. A value of 0 should be used if the given point-radius does not "
        "completely contain the original representation. The pointRadiusSpatialFit is undefined (and should be "
        "left blank) if the original representation is a point without uncertainty and the given georeference is "
        "not that same point (without uncertainty). If both the original and the given georeference are the same "
        "point, the pointRadiusSpatialFit is 1.")},
    {"footprintWKT", N_("Footprint WKT"), "Text", xmpText, xmpExternal,
     N_("A Well-Known Text (WKT) representation of the shape (footprint, geometry) that defines the Location. A "
        "Location may have both a point-radius representation (see decimalLatitude) and a footprint "
        "representation, and they may differ from each other.")},
    {"footprintSRS", N_("Footprint SRS"), "Text", xmpText, xmpExternal,
     N_("A Well-Known Text (WKT) representation of the Spatial Reference System (SRS) for the footprintWKT of the "
        "Location. Do not use this term to describe the SRS of the decimalLatitude and decimalLongitude, even if "
        "it is the same as for the footprintWKT - use the geodeticDatum instead.")},
    {"footprintSpatialFit", N_("Footprint Spatial Fit"), "Real", xmpText, xmpExternal,
     /* xgettext:no-c-format */
     N_("The ratio of the area of the footprint (footprintWKT) to the area of the true (original, or most "
        "specific) spatial representation of the Location. Legal values are 0, greater than or equal to 1, or "
        "undefined. A value of 1 is an exact match or 100% overlap. A value of 0 should be used if the given "
        "footprint does not completely contain the original representation. The footprintSpatialFit is undefined "
        "(and should be left blank) if the original representation is a point and the given georeference is not "
        "that same point. If both the original and the given georeference are the same point, the "
        "footprintSpatialFit is 1.")},
    {"georeferencedBy", N_("Georeferenced By"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of names of people, groups, or organizations who determined the "
        "georeference (spatial representation) for the Location.")},
    {"georeferencedDate", N_("Georeferenced Date"), "Date", xmpText, xmpExternal,
     N_("The date on which the Location was georeferenced. Recommended best practice is to use an encoding scheme, "
        "such as ISO 8601:2004(E).")},
    {"georeferenceProtocol", N_("Georeference Protocol"), "Text", xmpText, xmpExternal,
     N_("A description or reference to the methods used to determine the spatial footprint, coordinates, and "
        "uncertainties.")},
    {"georeferenceSources", N_("Georeference Sources"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of maps, gazetteers, or other resources used to georeference the "
        "Location, described specifically enough to allow anyone in the future to use the same resources.")},
    {"georeferenceVerificationStatus", N_("Georeference Verification Status"), "Text", xmpText, xmpExternal,
     N_("A categorical description of the extent to which the georeference has been verified to represent the best "
        "possible spatial description. Recommended best practice is to use a controlled vocabulary.")},
    {"georeferenceRemarks", N_("Georeference Remarks"), "Text", xmpText, xmpExternal,
     N_("Notes or comments about the spatial description determination, explaining assumptions made in addition or "
        "opposition to the those formalized in the method referred to in georeferenceProtocol.")},

    // Geological Context Level Class
    {
        "GeologicalContext",
        N_("Geological Context"),
        "GeologicalContext",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing geological context based information."),
    },
    // Geological Context Level Terms
    {"geologicalContextID", N_("Geological Context ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the set of information associated with a GeologicalContext (the location within a "
        "geological context, such as stratigraphy). May be a global unique identifier or an identifier specific to "
        "the data set.")},
    {"earliestEonOrLowestEonothem", N_("Earliest Eon Or Lowest Eonothem"), "Text", xmpText, xmpExternal,
     N_("The full name of the earliest possible geochronologic eon or lowest chrono-stratigraphic eonothem or the "
        "informal name (\"Precambrian\") attributable to the stratigraphic horizon from which the cataloged item "
        "was collected.")},
    {"latestEonOrHighestEonothem", N_("Latest Eon Or Highest Eonothem"), "Text", xmpText, xmpExternal,
     N_("The full name of the latest possible geochronologic eon or highest chrono-stratigraphic eonothem or the "
        "informal name (\"Precambrian\") attributable to the stratigraphic horizon from which the cataloged item "
        "was collected.")},
    {"earliestEraOrLowestErathem", N_("Earliest Era Or Lowest Erathem"), "Text", xmpText, xmpExternal,
     N_("The full name of the earliest possible geochronologic era or lowest chronostratigraphic erathem "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"latestEraOrHighestErathem", N_("Latest Era Or Highest Erathem"), "Text", xmpText, xmpExternal,
     N_("The full name of the latest possible geochronologic era or highest chronostratigraphic erathem "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"earliestPeriodOrLowestSystem", N_("Earliest Period Or Lowest System"), "Text", xmpText, xmpExternal,
     N_("The full name of the earliest possible geochronologic period or lowest chronostratigraphic system "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"latestPeriodOrHighestSystem", N_("Latest Period Or Highest System"), "Text", xmpText, xmpExternal,
     N_("The full name of the latest possible geochronologic period or highest chronostratigraphic system "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"earliestEpochOrLowestSeries", N_("Earliest Epoch Or Lowest Series"), "Text", xmpText, xmpExternal,
     N_("The full name of the earliest possible geochronologic epoch or lowest chronostratigraphic series "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"latestEpochOrHighestSeries", N_("Latest Epoch Or Highest Series"), "Text", xmpText, xmpExternal,
     N_("The full name of the latest possible geochronologic epoch or highest chronostratigraphic series "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"earliestAgeOrLowestStage", N_("Earliest Age Or Lowest Stage"), "Text", xmpText, xmpExternal,
     N_("The full name of the earliest possible geochronologic age or lowest chronostratigraphic stage "
        "attributable to the stratigraphic horizon from which the cataloged item was collected.")},
    {"latestAgeOrHighestStage", N_("Latest Age Or Highest Stage"), "Text", xmpText, xmpExternal,
     N_("The full name of the latest possible geochronologic age or highest chronostratigraphic stage attributable "
        "to the stratigraphic horizon from which the cataloged item was collected.")},
    {"lowestBiostratigraphicZone", N_("Lowest Biostratigraphic Zone"), "Text", xmpText, xmpExternal,
     N_("The full name of the lowest possible geological biostratigraphic zone of the stratigraphic horizon from "
        "which the cataloged item was collected.")},
    {"highestBiostratigraphicZone", N_("Highest Biostratigraphic Zone"), "Text", xmpText, xmpExternal,
     N_("The full name of the highest possible geological biostratigraphic zone of the stratigraphic horizon from "
        "which the cataloged item was collected.")},
    {"lithostratigraphicTerms", N_("Lithostratigraphic Terms"), "Text", xmpText, xmpExternal,
     N_("The combination of all litho-stratigraphic names for the rock from which the cataloged item was "
        "collected.")},
    {"group", N_("Group"), "Text", xmpText, xmpExternal,
     N_("The full name of the lithostratigraphic group from which the cataloged item was collected.")},
    {"formation", N_("Formation"), "Text", xmpText, xmpExternal,
     N_("The full name of the lithostratigraphic formation from which the cataloged item was collected.")},
    {"member", N_("Member"), "Text", xmpText, xmpExternal,
     N_("The full name of the lithostratigraphic member from which the cataloged item was collected.")},
    {"bed", N_("Bed"), "Text", xmpText, xmpExternal,
     N_("The full name of the lithostratigraphic bed from which the cataloged item was collected.")},

    // Identification Level Class
    {
        "Identification",
        N_("Identification"),
        "Identification",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing identification based information."),
    },
    // Identification Level Terms
    {"identificationID", N_("Identification ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the Identification (the body of information associated with the assignment of a "
        "scientific name). May be a global unique identifier or an identifier specific to the data set.")},
    {"identifiedBy", N_("Identified By"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of names of people, groups, or organizations who assigned the Taxon "
        "to the subject.")},
    {"dateIdentified", N_("Date Identified"), "Date", xmpText, xmpExternal,
     N_("The date on which the subject was identified as representing the Taxon. Recommended best practice is to "
        "use an encoding scheme, such as ISO 8601:2004(E).")},
    {"identificationReferences", N_("Identification References"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of references (publication, global unique identifier, URI) used in "
        "the Identification.")},
    {"identificationVerificationStatus", N_("Identification Verification Status"), "Text", xmpText, xmpExternal,
     N_("A categorical indicator of the extent to which the taxonomic identification has been verified to be "
        "correct. Recommended best practice is to use a controlled vocabulary such as that used in HISPID/ABCD.")},
    {"identificationRemarks", N_("Identification Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the Identification.")},
    {"identificationQualifier", N_("Identification Qualifier"), "Text", xmpText, xmpExternal,
     N_("A brief phrase or a standard term (\"cf.\" \"aff.\") to express the determiner's doubts about the "
        "Identification.")},
    {"typeStatus", N_("Type Status"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of nomenclatural types (type status, typified scientific name, "
        "publication) applied to the subject.")},

    // Taxon Level Class
    {
        "Taxon",
        N_("Taxon"),
        "Taxon",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing taxonomic based information."),
    },
    // Taxon Level Terms
    {"taxonID", N_("Taxon ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the set of taxon information (data associated with the Taxon class). May be a global "
        "unique identifier or an identifier specific to the data set.")},
    {"scientificNameID", N_("Scientific Name ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the nomenclatural (not taxonomic) details of a scientific name.")},
    {"acceptedNameUsageID", N_("Accepted Name Usage ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the name usage (documented meaning of the name according to a source) of the currently "
        "valid (zoological) or accepted (botanical) taxon.")},
    {"parentNameUsageID", N_("Parent Name Usage ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the name usage (documented meaning of the name according to a source) of the direct, "
        "most proximate higher-rank parent taxon (in a classification) of the most specific element of the "
        "scientificName.")},
    {"originalNameUsageID", N_("Original Name Usage ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the name usage (documented meaning of the name according to a source) in which the "
        "terminal element of the scientificName was originally established under the rules of the associated "
        "nomenclaturalCode.")},
    {"nameAccordingToID", N_("Name According To ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the source in which the specific taxon concept circumscription is defined or implied. "
        "See nameAccordingTo.")},
    {"namePublishedInID", N_("Name Published In ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the publication in which the scientificName was originally established under the rules "
        "of the associated nomenclaturalCode.")},
    {"taxonConceptID", N_("Taxon Concept ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the taxonomic concept to which the record refers - not for the nomenclatural details of "
        "a taxon.")},
    {"scientificName", N_("Scientific Name"), "Text", xmpText, xmpExternal,
     N_("The full scientific name, with authorship and date information if known. When forming part of an "
        "Identification, this should be the name in lowest level taxonomic rank that can be determined. This term "
        "should not contain identification qualifications, which should instead be supplied in the "
        "IdentificationQualifier term.")},
    {"acceptedNameUsage", N_("Accepted Name Usage"), "Text", xmpText, xmpExternal,
     N_("The full name, with authorship and date information if known, of the currently valid (zoological) or "
        "accepted (botanical) taxon.")},
    {"parentNameUsage", N_("Parent Name Usage"), "Text", xmpText, xmpExternal,
     N_("The full name, with authorship and date information if known, of the direct, most proximate higher-rank "
        "parent taxon (in a classification) of the most specific element of the scientificName.")},
    {"originalNameUsage", N_("Original Name Usage"), "Text", xmpText, xmpExternal,
     N_("The taxon name, with authorship and date information if known, as it originally appeared when first "
        "established under the rules of the associated nomenclaturalCode. The basionym (botany) or basonym "
        "(bacteriology) of the scientificName or the senior/earlier homonym for replaced names.")},
    {"nameAccordingTo", N_("Name According To"), "Text", xmpText, xmpExternal,
     N_("The reference to the source in which the specific taxon concept circumscription is defined or implied - "
        "traditionally signified by the Latin \"sensu\" or \"sec.\" (from secundum, meaning \"according to\"). For "
        "taxa that result from identifications, a reference to the keys, monographs, experts and other sources "
        "should be given.")},
    {"namePublishedIn", N_("Name Published In"), "Text", xmpText, xmpExternal,
     N_("A reference for the publication in which the scientificName was originally established under the rules of "
        "the associated nomenclaturalCode.")},
    {"namePublishedInYear", N_("Name Published In Year"), "Text", xmpText, xmpExternal,
     N_("The four-digit year in which the scientificName was published.")},
    {"higherClassification", N_("Higher Classification"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of taxa names terminating at the rank immediately superior to the "
        "taxon referenced in the taxon record. Recommended best practice is to order the list starting with the "
        "highest rank and separating the names for each rank with a semi-colon ;")},
    {"kingdom", N_("Kingdom"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the kingdom in which the taxon is classified.")},
    {"phylum", N_("Phylum"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the phylum or division in which the taxon is classified.")},
    {"class", N_("Class"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the class in which the taxon is classified.")},
    {"order", N_("Order"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the order in which the taxon is classified.")},
    {"family", N_("Family"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the family in which the taxon is classified.")},
    {"genus", N_("Genus"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the genus in which the taxon is classified.")},
    {"subgenus", N_("Subgenus"), "Text", xmpText, xmpExternal,
     N_("The full scientific name of the subgenus in which the taxon is classified. Values should include the "
        "genus to avoid homonym confusion.")},
    {"specificEpithet", N_("Specific Epithet"), "Text", xmpText, xmpExternal,
     N_("The name of the first or species epithet of the scientificName.")},
    {"infraspecificEpithet", N_("Infraspecific Epithet"), "Text", xmpText, xmpExternal,
     N_("The name of the lowest or terminal infraspecific epithet of the scientificName, excluding any rank "
        "designation.")},
    {"taxonRank", N_("Taxon Rank"), "Text", xmpText, xmpExternal,
     N_("The taxonomic rank of the most specific name in the scientificName. Recommended best practice is to use a "
        "controlled vocabulary.")},
    {"verbatimTaxonRank", N_("Verbatim Taxon Rank"), "Text", xmpText, xmpExternal,
     N_("The taxonomic rank of the most specific name in the scientificName as it appears in the original "
        "record.")},
    {"scientificNameAuthorship", N_("Scientific Name Authorship"), "Text", xmpText, xmpExternal,
     N_("The authorship information for the scientificName formatted according to the conventions of the "
        "applicable nomenclaturalCode.")},
    {"vernacularName", N_("Vernacular Name"), "Lang Alt", langAlt, xmpExternal, N_("A common or vernacular name.")},
    {"nomenclaturalCode", N_("Nomenclatural Code"), "Text", xmpText, xmpExternal,
     N_("The nomenclatural code (or codes in the case of an ambiregnal name) under which the scientificName is "
        "constructed. Recommended best practice is to use a controlled vocabulary.")},
    {"taxonomicStatus", N_("Taxonomic Status"), "Text", xmpText, xmpExternal,
     N_("The status of the use of the scientificName as a label for a taxon. Requires taxonomic opinion to define "
        "the scope of a taxon. Rules of priority then are used to define the taxonomic status of the nomenclature "
        "contained in that scope, combined with the experts opinion. It must be linked to a specific taxonomic "
        "reference that defines the concept. Recommended best practice is to use a controlled vocabulary.")},
    {"nomenclaturalStatus", N_("Nomenclatural Status"), "Text", xmpText, xmpExternal,
     N_("The status related to the original publication of the name and its conformance to the relevant rules of "
        "nomenclature. It is based essentially on an algorithm according to the business rules of the code. It "
        "requires no taxonomic opinion.")},
    {"taxonRemarks", N_("Taxon Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the taxon or name.")},

    // Resource Relationship Level Class
    {
        "ResourceRelationship",
        N_("Resource Relationship"),
        "ResourceRelationship",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing relationships between resources based information."),
    },
    // Resource Relationship Level Terms
    {"resourceRelationshipID", N_("Resource Relationship ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for an instance of relationship between one resource (the subject) and another "
        "(relatedResource, the object).")},
    {"resourceID", N_("Resource ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the resource that is the subject of the relationship.")},
    {"relatedResourceID", N_("Related Resource ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for a related resource (the object, rather than the subject of the relationship).")},
    {"relationshipOfResource", N_("Relationship Of Resource"), "Text", xmpText, xmpExternal,
     N_("The relationship of the resource identified by relatedResourceID to the subject (optionally identified by "
        "the resourceID). Recommended best practice is to use a controlled vocabulary.")},
    {"relationshipAccordingTo", N_("Relationship According To"), "Text", xmpText, xmpExternal,
     N_("The source (person, organization, publication, reference) establishing the relationship between the two "
        "resources.")},
    {"relationshipEstablishedDate", N_("Relationship Established Date"), "Date", xmpText, xmpExternal,
     N_("The date-time on which the relationship between the two resources was established. Recommended best "
        "practice is to use an encoding scheme, such as ISO 8601:2004(E).")},
    {"relationshipRemarks", N_("Relationship Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes about the relationship between the two resources.")},

    // Measurement Or Fact Level Class
    {
        "MeasurementOrFact",
        N_("Measurement Or Fact"),
        "MeasurementOrFact",
        xmpText,
        xmpInternal,
        N_("*Main structure* containing measurement based information."),
    },
    // Measurement Or Fact Level Terms
    {"measurementID", N_("Measurement ID"), "Text", xmpText, xmpExternal,
     N_("An identifier for the MeasurementOrFact (information pertaining to measurements, facts, characteristics, "
        "or assertions). May be a global unique identifier or an identifier specific to the data set.")},
    {"measurementType", N_("Measurement Type"), "Text", xmpText, xmpExternal,
     N_("The nature of the measurement, fact, characteristic, or assertion. Recommended best practice is to use a "
        "controlled vocabulary.")},
    {"measurementValue", N_("Measurement Value"), "Text", xmpText, xmpExternal,
     N_("The value of the measurement, fact, characteristic, or assertion.")},
    {"measurementAccuracy", N_("Measurement Accuracy"), "Text", xmpText, xmpExternal,
     N_("The description of the potential error associated with the measurementValue.")},
    {"measurementUnit", N_("Measurement Unit"), "Text", xmpText, xmpExternal,
     N_("The units associated with the measurementValue. Recommended best practice is to use the International "
        "System of Units (SI).")},
    {"measurementDeterminedDate", N_("Measurement Determined Date"), "Date", xmpText, xmpExternal,
     N_("The date on which the MeasurementOrFact was made. Recommended best practice is to use an encoding scheme, "
        "such as ISO 8601:2004(E).")},
    {"measurementDeterminedBy", N_("Measurement Determined By"), "bag Text", xmpBag, xmpExternal,
     N_("A list (concatenated and separated) of names of people, groups, or organizations who determined the value "
        "of the MeasurementOrFact.")},
    {"measurementMethod", N_("Measurement Method"), "Text", xmpText, xmpExternal,
     N_("A description of or reference to (publication, URI) the method or protocol used to determine the "
        "measurement, fact, characteristic, or assertion.")},
    {"measurementRemarks", N_("Measurement Remarks"), "Text", xmpText, xmpExternal,
     N_("Comments or notes accompanying the MeasurementOrFact.")},
    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

const XmpPropertyInfo xmpAcdseeInfo[] = {
    {"caption", N_("Caption"), "Text", xmpText, xmpExternal, N_("A brief description of the file")},
    {"datetime", N_("Date Time"), "Text", xmpText, xmpExternal, N_("Date and Time")},
    {"author", N_("Author"), "Text", xmpText, xmpExternal, N_("The name of the author or photographer")},
    {"rating", N_("Rating"), "Text", xmpText, xmpExternal, N_("Numerical rating from 1 to 5")},
    {"notes", N_("Notes"), "Text", xmpText, xmpExternal,
     N_("Any descriptive or additional free-form text up to 4,095 characters")},
    {"tagged", N_("Tagged"), "Text", xmpText, xmpExternal, N_("True or False")},
    {"categories", N_("Categories"), "Text", xmpText, xmpExternal, N_("Catalog of hierarchical keywords and groups")},

    // End of list marker
    {nullptr, nullptr, nullptr, invalidTypeId, xmpInternal, nullptr},
};

constexpr XmpPrintInfo xmpPrintInfo[] = {
    {"Xmp.crs.CropUnits", EXV_PRINT_TAG(crsCropUnits)},
    {"Xmp.exif.ApertureValue", print0x9202},
    {"Xmp.exif.BrightnessValue", printFloat},
    {"Xmp.exif.ColorSpace", print0xa001},
    {"Xmp.exif.ComponentsConfiguration", print0x9101},
    {"Xmp.exif.Contrast", printNormalSoftHard},
    {"Xmp.exif.CreateDate", printXmpDate},
    {"Xmp.exif.CustomRendered", print0xa401},
    {"Xmp.exif.DateTimeOriginal", printXmpDate},
    {"Xmp.exif.ExifVersion", printXmpVersion},
    {"Xmp.exif.ExposureBiasValue", print0x9204},
    {"Xmp.exif.ExposureMode", print0xa402},
    {"Xmp.exif.ExposureProgram", print0x8822},
    {"Xmp.exif.FileSource", print0xa300},
    {"Xmp.exif.FlashpixVersion", printXmpVersion},
    {"Xmp.exif.FNumber", print0x829d},
    {"Xmp.exif.FocalLength", print0x920a},
    {"Xmp.exif.FocalPlaneResolutionUnit", printExifUnit},
    {"Xmp.exif.FocalPlaneXResolution", printFloat},
    {"Xmp.exif.FocalPlaneYResolution", printFloat},
    {"Xmp.exif.GainControl", print0xa407},
    {"Xmp.exif.GPSAltitudeRef", print0x0005},
    {"Xmp.exif.GPSDestBearingRef", printGPSDirRef},
    {"Xmp.exif.GPSDestDistanceRef", print0x0019},
    {"Xmp.exif.GPSDifferential", print0x001e},
    {"Xmp.exif.GPSImgDirectionRef", printGPSDirRef},
    {"Xmp.exif.GPSMeasureMode", print0x000a},
    {"Xmp.exif.GPSSpeedRef", print0x000c},
    {"Xmp.exif.GPSStatus", print0x0009},
    {"Xmp.exif.GPSTimeStamp", printXmpDate},
    {"Xmp.exif.GPSTrackRef", printGPSDirRef},
    {"Xmp.exif.LightSource", print0x9208},
    {"Xmp.exif.MeteringMode", print0x9207},
    {"Xmp.exif.ModifyDate", printXmpDate},
    {"Xmp.exif.Saturation", print0xa409},
    {"Xmp.exif.SceneCaptureType", print0xa406},
    {"Xmp.exif.SceneType", print0xa301},
    {"Xmp.exif.SensingMethod", print0xa217},
    {"Xmp.exif.Sharpness", printNormalSoftHard},
    {"Xmp.exif.ShutterSpeedValue", print0x9201},
    {"Xmp.exif.SubjectDistanceRange", print0xa40c},
    {"Xmp.exif.WhiteBalance", print0xa403},
    {"Xmp.tiff.Orientation", print0x0112},
    {"Xmp.tiff.ResolutionUnit", printExifUnit},
    {"Xmp.tiff.XResolution", printInt64},
    {"Xmp.tiff.YCbCrPositioning", print0x0213},
    {"Xmp.tiff.YResolution", printInt64},
    {"Xmp.iptc.Scene", EXV_PRINT_VOCABULARY_MULTI(iptcScene)},
    {"Xmp.iptc.SubjectCode", EXV_PRINT_VOCABULARY_MULTI(iptcSubjectCode)},
    {"Xmp.iptcExt.DigitalSourcefileType", EXV_PRINT_VOCABULARY(iptcExtDigitalSourcefileType)},
    {"Xmp.iptcExt.DigitalSourceType", EXV_PRINT_VOCABULARY(iptcExtDigitalSourceType)},
    {"Xmp.plus.AdultContentWarning", EXV_PRINT_VOCABULARY(plusAdultContentWarning)},
    {"Xmp.plus.CopyrightStatus", EXV_PRINT_VOCABULARY(plusCopyrightStatus)},
    {"Xmp.plus.CreditLineRequired", EXV_PRINT_VOCABULARY(plusCreditLineRequired)},
    {"Xmp.plus.ImageAlterationConstraints", EXV_PRINT_VOCABULARY(plusImageAlterationConstraints)},
    {"Xmp.plus.ImageDuplicationConstraints", EXV_PRINT_VOCABULARY(plusImageDuplicationConstraints)},
    {"Xmp.plus.ImageFileConstraints", EXV_PRINT_VOCABULARY(plusImageFileConstraints)},
    {"Xmp.plus.ImageFileFormatAsDelivered", EXV_PRINT_VOCABULARY(plusImageFileFormatAsDelivered)},
    {"Xmp.plus.ImageFileSizeAsDelivered", EXV_PRINT_VOCABULARY(plusImageFileSizeAsDelivered)},
    {"Xmp.plus.ImageType", EXV_PRINT_VOCABULARY(plusImageType)},
    {"Xmp.plus.LicensorTelephoneType1", EXV_PRINT_VOCABULARY(plusLicensorTelephoneType)},
    {"Xmp.plus.LicensorTelephoneType2", EXV_PRINT_VOCABULARY(plusLicensorTelephoneType)},
    {"Xmp.plus.MinorModelAgeDisclosure", EXV_PRINT_VOCABULARY(plusMinorModelAgeDisclosure)},
    {"Xmp.plus.ModelReleaseStatus", EXV_PRINT_VOCABULARY(plusModelReleaseStatus)},
    {"Xmp.plus.PropertyReleaseStatus", EXV_PRINT_VOCABULARY(plusPropertyReleaseStatus)},
    {"Xmp.plus.Reuse", EXV_PRINT_VOCABULARY(plusReuse)},
};

bool XmpNsInfo::operator==(const XmpNsInfo::Ns& ns) const {
  return ns_ == ns.ns_;
}

bool XmpNsInfo::operator==(const XmpNsInfo::Prefix& prefix) const {
  return prefix_ == prefix.prefix_;
}

bool XmpPropertyInfo::operator==(const std::string& name) const {
  return name_ == name;
}

XmpProperties::NsRegistry XmpProperties::nsRegistry_;
std::mutex XmpProperties::mutex_;

/// \todo not used internally. At least we should test it
const XmpNsInfo* XmpProperties::lookupNsRegistry(const XmpNsInfo::Prefix& prefix) {
  auto scopedReadLock = std::scoped_lock(mutex_);
  return lookupNsRegistryUnsafe(prefix);
}

const XmpNsInfo* XmpProperties::lookupNsRegistryUnsafe(const XmpNsInfo::Prefix& prefix) {
  for (const auto& [_, p] : nsRegistry_) {
    if (p == prefix)
      return &p;
  }
  return nullptr;
}

void XmpProperties::registerNs(const std::string& ns, const std::string& prefix) {
  auto scopedWriteLock = std::scoped_lock(mutex_);
  std::string ns2 = ns;
  if (ns2.back() != '/' && ns2.back() != '#')
    ns2 += '/';
  // Check if there is already a registered namespace with this prefix
  if (auto xnp = lookupNsRegistryUnsafe(XmpNsInfo::Prefix{prefix})) {
#ifndef SUPPRESS_WARNINGS
    if (ns2 != xnp->ns_)
      EXV_WARNING << "Updating namespace URI for " << prefix << " from " << xnp->ns_ << " to " << ns2 << "\n";
#endif
    unregisterNsUnsafe(xnp->ns_);
  }
  // Allocated memory is freed when the namespace is unregistered.
  // Using malloc/free for better system compatibility in case
  // users don't unregister their namespaces explicitly.
  XmpNsInfo xn;
  auto c = new char[ns2.size() + 1];
  std::strcpy(c, ns2.c_str());
  xn.ns_ = c;
  c = new char[prefix.size() + 1];
  std::strcpy(c, prefix.c_str());
  xn.prefix_ = c;
  xn.xmpPropertyInfo_ = nullptr;
  xn.desc_ = "";
  nsRegistry_[ns2] = xn;
}

void XmpProperties::unregisterNs(const std::string& ns) {
  auto scoped_write_lock = std::scoped_lock(mutex_);
  unregisterNsUnsafe(ns);
}

void XmpProperties::unregisterNsUnsafe(const std::string& ns) {
  auto i = nsRegistry_.find(ns);
  if (i != nsRegistry_.end()) {
    delete[] i->second.prefix_;
    delete[] i->second.ns_;
    nsRegistry_.erase(i);
  }
}

void XmpProperties::unregisterNs() {
  auto scoped_write_lock = std::scoped_lock(mutex_);
  /// \todo check if we are not unregistering the first NS
  auto i = nsRegistry_.begin();
  while (i != nsRegistry_.end()) {
    auto kill = i++;
    unregisterNsUnsafe(kill->first);
  }
}

std::string XmpProperties::prefix(const std::string& ns) {
  auto scoped_read_lock = std::scoped_lock(mutex_);
  std::string ns2 = ns;
  if (ns2.back() != '/' && ns2.back() != '#')
    ns2 += '/';

  auto i = nsRegistry_.find(ns2);
  std::string p;
  if (i != nsRegistry_.end())
    p = i->second.prefix_;
  else if (auto xn = Exiv2::find(xmpNsInfo, XmpNsInfo::Ns{std::move(ns2)}))
    p = std::string(xn->prefix_);
  return p;
}

std::string XmpProperties::ns(const std::string& prefix) {
  auto scoped_read_lock = std::scoped_lock(mutex_);
  if (auto xn = lookupNsRegistryUnsafe(XmpNsInfo::Prefix{prefix}))
    return xn->ns_;
  return nsInfoUnsafe(prefix)->ns_;
}

const char* XmpProperties::propertyTitle(const XmpKey& key) {
  const XmpPropertyInfo* pi = propertyInfo(key);
  return pi ? pi->title_ : nullptr;
}

const char* XmpProperties::propertyDesc(const XmpKey& key) {
  const XmpPropertyInfo* pi = propertyInfo(key);
  return pi ? pi->desc_ : nullptr;
}

TypeId XmpProperties::propertyType(const XmpKey& key) {
  const XmpPropertyInfo* pi = propertyInfo(key);
  return pi ? pi->typeId_ : xmpText;
}

const XmpPropertyInfo* XmpProperties::propertyInfo(const XmpKey& key) {
  std::string prefix = key.groupName();
  std::string property = key.tagName();
  // If property is a path for a nested property, determines the innermost element
  if (auto i = property.find_last_of('/'); i != std::string::npos) {
    i = std::distance(property.begin(), std::find_if(property.begin() + i, property.end(), isalpha));
    property = property.substr(i);
    i = property.find_first_of(':');
    if (i != std::string::npos) {
      prefix = property.substr(0, i);
      property = property.substr(i + 1);
    }
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Nested key: " << key.key() << ", prefix: " << prefix << ", property: " << property << "\n";
#endif
  }
  if (auto pl = propertyList(prefix)) {
    for (size_t j = 0; pl[j].name_; ++j) {
      if (property == pl[j].name_) {
        return pl + j;
      }
    }
  }
  return nullptr;
}

/// \todo not used internally. At least we should test it
const char* XmpProperties::nsDesc(const std::string& prefix) {
  return nsInfo(prefix)->desc_;
}

const XmpPropertyInfo* XmpProperties::propertyList(const std::string& prefix) {
  return nsInfo(prefix)->xmpPropertyInfo_;
}

const XmpNsInfo* XmpProperties::nsInfo(const std::string& prefix) {
  auto scoped_read_lock = std::scoped_lock(mutex_);
  return nsInfoUnsafe(prefix);
}

const XmpNsInfo* XmpProperties::nsInfoUnsafe(const std::string& prefix) {
  const auto pf = XmpNsInfo::Prefix{prefix};
  const XmpNsInfo* xn = lookupNsRegistryUnsafe(pf);
  if (!xn)
    xn = Exiv2::find(xmpNsInfo, pf);
  if (!xn)
    throw Error(ErrorCode::kerNoNamespaceInfoForXmpPrefix, prefix);
  return xn;
}

void XmpProperties::registeredNamespaces(Exiv2::Dictionary& nsDict) {
  for (auto&& i : xmpNsInfo) {
    Exiv2::XmpParser::registerNs(i.ns_, i.prefix_);
  }
  Exiv2::XmpParser::registeredNamespaces(nsDict);
}

void XmpProperties::printProperties(std::ostream& os, const std::string& prefix) {
  if (auto pl = propertyList(prefix)) {
    for (int i = 0; pl[i].name_; ++i) {
      os << pl[i];
    }
  }

}  // XmpProperties::printProperties

std::ostream& XmpProperties::printProperty(std::ostream& os, const std::string& key, const Value& value) {
  PrintFct fct = printValue;
  if (value.count() != 0) {
    if (auto info = Exiv2::find(xmpPrintInfo, key))
      fct = info->printFct_;
  }
  return fct(os, value, nullptr);
}

//! @brief Internal Pimpl structure with private members and data of class XmpKey.
struct XmpKey::Impl {
  Impl() = default;                                              //!< Default constructor
  Impl(const std::string& prefix, const std::string& property);  //!< Constructor

  /*!
    @brief Parse and convert the \em key string into property and prefix.
           Updates data members if the string can be decomposed, or throws
           \em Error.

    @throw Error if the key cannot be decomposed.
  */
  void decomposeKey(const std::string& key);  //!< Mysterious magic

  // DATA
  static constexpr auto familyName_ = "Xmp";  //!< "Xmp"

  std::string prefix_;    //!< Prefix
  std::string property_;  //!< Property name
};

//! @brief Constructor for Internal Pimpl structure XmpKey::Impl::Impl
XmpKey::Impl::Impl(const std::string& prefix, const std::string& property) {
  // Validate prefix
  if (XmpProperties::ns(prefix).empty())
    throw Error(ErrorCode::kerNoNamespaceForPrefix, prefix);

  property_ = property;
  prefix_ = prefix;
}

XmpKey::XmpKey(const std::string& key) : p_(std::make_unique<Impl>()) {
  p_->decomposeKey(key);
}

XmpKey::XmpKey(const std::string& prefix, const std::string& property) : p_(std::make_unique<Impl>(prefix, property)) {
}

XmpKey::~XmpKey() = default;

XmpKey::XmpKey(const XmpKey& rhs) : p_(std::make_unique<Impl>(*rhs.p_)) {
}

XmpKey& XmpKey::operator=(const XmpKey& rhs) {
  if (this == &rhs)
    return *this;
  *p_ = *rhs.p_;
  return *this;
}

XmpKey::UniquePtr XmpKey::clone() const {
  return UniquePtr(clone_());
}

XmpKey* XmpKey::clone_() const {
  return new XmpKey(*this);
}

std::string XmpKey::key() const {
  return std::string(Exiv2::XmpKey::Impl::familyName_) + "." + p_->prefix_ + "." + p_->property_;
}

const char* XmpKey::familyName() const {
  return Exiv2::XmpKey::Impl::familyName_;
}

std::string XmpKey::groupName() const {
  return p_->prefix_;
}

std::string XmpKey::tagName() const {
  return p_->property_;
}

std::string XmpKey::tagLabel() const {
  const char* pt = XmpProperties::propertyTitle(*this);
  if (!pt)
    return tagName();
  return pt;
}

std::string XmpKey::tagDesc() const {
  const char* pt = XmpProperties::propertyDesc(*this);
  if (!pt)
    return "";
  return pt;
}

uint16_t XmpKey::tag() const {
  return 0;
}

std::string XmpKey::ns() const {
  return XmpProperties::ns(p_->prefix_);
}

//! @cond IGNORE
void XmpKey::Impl::decomposeKey(const std::string& key) {
  // Get the family name, prefix and property name parts of the key
  if (!key.starts_with(familyName_))
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string::size_type pos1 = key.find('.');
  std::string::size_type pos0 = pos1 + 1;
  pos1 = key.find('.', pos0);
  if (pos1 == std::string::npos)
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string prefix = key.substr(pos0, pos1 - pos0);
  if (prefix.empty())
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string property = key.substr(pos1 + 1);
  if (property.empty())
    throw Error(ErrorCode::kerInvalidKey, key);

  // Validate prefix
  if (XmpProperties::ns(prefix).empty())
    throw Error(ErrorCode::kerNoNamespaceForPrefix, prefix);

  property_ = std::move(property);
  prefix_ = std::move(prefix);
}  // XmpKey::Impl::decomposeKey

// *************************************************************************
// free functions
// *************************************************************************
// free functions
std::ostream& operator<<(std::ostream& os, const XmpPropertyInfo& propertyInfo) {
  // CSV encoded I am \"dead\" beat" => "I am ""dead"" beat"
  std::string escapedDesc;
  escapedDesc.push_back('"');
  for (char c : std::string_view(propertyInfo.desc_)) {
    if (c == '"')
      escapedDesc += "\"\"";
    else
      escapedDesc.push_back(c);
  }
  escapedDesc.push_back('"');

  return os << stringFormat("{},{},{},{},{},{}\n", propertyInfo.name_, propertyInfo.title_, propertyInfo.xmpValueType_,
                            TypeInfo::typeName(propertyInfo.typeId_),
                            (propertyInfo.xmpCategory_ == xmpExternal ? "External" : "Internal"), escapedDesc);
}
//! @endcond

}  // namespace Exiv2
