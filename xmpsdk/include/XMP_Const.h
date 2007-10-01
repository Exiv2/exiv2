#ifndef __XMP_Const_h__
#define __XMP_Const_h__ 1

/* --------------------------------------------------------------------------------------------- */
/* ** IMPORTANT ** This file must be usable by strict ANSI C compilers. No "//" comments, etc.   */
/* --------------------------------------------------------------------------------------------- */

/*
// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================
*/

#include "XMP_Environment.h"

   #include <stddef.h>

#if XMP_MacBuild	/* ! No stdint.h on Windows and some UNIXes. */
    #include <stdint.h>
#endif

#if __cplusplus
extern "C" {
#endif


/** ================================================================================================
 *  \file XMP_Const.h
 *  \brief Common C/C++ types and constants for the XMP toolkit.
 */


/* ============================================================================================== */
/* Basic types and constants */
/* ========================= */

/* The XMP_... types are used on the off chance that the ..._t types present a problem. In that */
/* case only the declarations of the XMP_... types needs to change, not all of the uses. These  */
/* types are used where fixed sizes are required in order to have a known ABI for a DLL build.  */

#if XMP_MacBuild

    typedef int8_t   XMP_Int8;
    typedef int16_t  XMP_Int16;
    typedef int32_t  XMP_Int32;
    typedef int64_t  XMP_Int64;

    typedef uint8_t  XMP_Uns8;
    typedef uint16_t XMP_Uns16;
    typedef uint32_t XMP_Uns32;
    typedef uint64_t XMP_Uns64;

#else

    typedef signed char XMP_Int8;
    typedef signed short XMP_Int16;
    typedef signed long XMP_Int32;
    typedef signed long long XMP_Int64;

    typedef unsigned char XMP_Uns8;
    typedef unsigned short XMP_Uns16;
    typedef unsigned long XMP_Uns32;
    typedef unsigned long long XMP_Uns64;

#endif

typedef XMP_Uns8 XMP_Bool;

/* Typedefs to preserve old, unfortunate spellings: */

#if 0 /* *** Enable after internal compiles work. */
typedef XMP_Int8  XMPInt8;
typedef XMP_Int16 XMPInt16;
typedef XMP_Int32 XMPInt32;
typedef XMP_Int64 XMPInt64;
typedef XMP_Uns8  XMPUns8;
typedef XMP_Uns16 XMPUns16;
typedef XMP_Uns32 XMPUns32;
typedef XMP_Uns64 XMPUns64;
typedef XMP_Bool  XMPBool;
#endif

/**
 *  \typedef XMPMetaRef
 *  \brief An "ABI safe" pointer to the internal part of an XMP object.
 *
 *  \c XMPMetaRef is an "ABI safe" pointer to the internal part of an XMP object. It should be used
 *  for passing an XMP object across client DLL boundaries. See the discussion in <tt>TXMPMeta.hpp</tt>.
 */

typedef struct __XMPMeta__ *        XMPMetaRef;
typedef struct __XMPIterator__ *    XMPIteratorRef;
typedef struct __XMPFiles__ *       XMPFilesRef;

/* ============================================================================================== */

/**
 *  \name General scalar types and constants
 *  @{
 */

/**
 *  \typedef XMP_StringPtr
 *  \brief The type for input string parameters. A <tt>const char *</tt>, a null-terminated UTF-8 string.
 *
 */

/**
 *  \typedef XMP_StringLen
 *  \brief The type for string length parameters. A 32-bit unsigned integer, as big as will be
 *   practically needed.
 */

/**
 *  \typedef XMP_Index
 *  \brief The type for offsets and indices. A 32-bit signed integer. It is signed because that
 *  often makes loop termination tests safer.
 */

/**
 *  \typedef XMP_OptionBits
 *  \brief The type for a collection of 32 flag bits. Individual flags are defined as enum value
 *  bit masks. A number of macros provide common set or set operations, e.g. \c XMP_PropIsSimple.
 *  For other tests use an expression like "options & kXMP_<theOption>". When passing multiple
 *  option flags use the bitwise-or operator. '|', not the arithmatic plus, '+'.
 */

typedef const char * XMP_StringPtr;  /* Points to a null terminated UTF-8 string. */
typedef XMP_Uns32    XMP_StringLen;
typedef XMP_Int32    XMP_Index;      /* Signed, sometimes -1 is handy. */
typedef XMP_Uns32    XMP_OptionBits; /* Used as 32 individual bits. */

/**
 *  \def kXMP_TrueStr
 *  \brief The canonical true string value for Booleans in serialized XMP. Code that converts
 *  from the string to a bool should be case insensitive, and even allow "1".
 */

/**
 *  \def kXMP_FalseStr
 *  \brief The canonical false string value for Booleans in serialized XMP. Code that converts
 *  from the string to a bool should be case insensitive, and even allow "0".
 */

#define kXMP_TrueStr  "True"  /* Serialized XMP spellings, not for the type bool. */
#define kXMP_FalseStr "False"

/**
 *  @}
 */

/* ============================================================================================== */

/**
 *  \struct XMP_DateTime
 *  \brief The expanded type for a date and time. Dates and time in the serialized XMP are ISO 8601
 *   strings. The \c XMP_DateTime struct allows easy conversion with other formats.
 *
 *  All of the fields are 32 bit, even though most could be 8 bit. This avoids overflow when doing
 *  carries for arithmetic or normalization. All fields have signed values for the same reasons.
 *
 *  The fields of the \c XMP_DateTime struct are:
 *
 *  \li year- The year, can be negative.
 *  \li month - The month in the range 1..12.
 *  \li day - The day of the month in the range 1..31.
 *  \li hour - The hour in the range 0..23.
 *  \li minute - The minute in the range 0..59.
 *  \li second - The second in the range 0..59.
 *  \li tzSign - The "sign" of the time zone, 0 means UTC, -1 is west, +1 is east.
 *  \li tzHour - The time zone hour in the range 0..23.
 *  \li tzMinute - The time zone minute in the range 0..59.
 *  \li nanoSecond - Nanoseconds within a second, often left as zero.
 *
 *  Constants for the \c tzSign are:
 *
 *  \li \c kXMP_TimeIsUTC - The time is UTC.
 *  \li \c kXMP_TimeWestOfUTC - The time zone is west of UTC, behind in time.
 *  \li \c kXMP_TimeEastOfUTC - The time zone is east of UTC, ahead in time.
 *
 *  DateTime values are occasionally used in cases with only a date or only a time component. A date
 *  without a time has zeros in the \c XMP_DateTime struct for all time fields. A time without a date
 *  has zeros for all date fields (year, month, and day).
 */

struct XMP_DateTime {
    XMP_Int32 year;
    XMP_Int32 month;      /* 1..12 */
    XMP_Int32 day;        /* 1..31 */
    XMP_Int32 hour;       /* 0..23 */
    XMP_Int32 minute;     /* 0..59 */
    XMP_Int32 second;     /* 0..59 */
    XMP_Int32 tzSign;     /* -1..+1, 0 means UTC, -1 is west, +1 is east. */
    XMP_Int32 tzHour;     /* 0..23 */
    XMP_Int32 tzMinute;   /* 0..59 */
    XMP_Int32 nanoSecond;
};

enum {  /* Values used for tzSign field. */
    kXMP_TimeWestOfUTC = -1,
    kXMP_TimeIsUTC     =  0,
    kXMP_TimeEastOfUTC = +1
};


/* ============================================================================================== */
/* Standard namespace URI constants */
/* ================================ */

/**
 *  \name XML namespace constants for standard XMP schema.
 *  @{
 */

/**
 *  \def kXMP_NS_XMP
 *  \brief The XML namespace for the XMP "basic" schema.
 */

/**
 *  \def kXMP_NS_XMP_Rights
 *  \brief The XML namespace for the XMP copyright schema.
 */

/**
 *  \def kXMP_NS_XMP_MM
 *  \brief The XML namespace for the XMP digital asset management schema.
 */

/**
 *  \def kXMP_NS_XMP_BJ
 *  \brief The XML namespace for the job management schema.
 */

/**
 *  \def kXMP_NS_XMP_T
 *  \brief The XML namespace for the XMP text document schema.
 */

/**
 *  \def kXMP_NS_XMP_T_PG
 *  \brief The XML namespace for the XMP paged document schema.
 */

/**
 *  \def kXMP_NS_PDF
 *  \brief The XML namespace for the PDF schema.
 */

/**
 *  \def kXMP_NS_Photoshop
 *  \brief The XML namespace for the Photoshop custom schema.
 */

/**
 *  \def kXMP_NS_EXIF
 *  \brief The XML namespace for Adobe's EXIF schema.
 */

/**
 *  \def kXMP_NS_TIFF
 *  \brief The XML namespace for Adobe's TIFF schema.
 */

/**
 *  @}
 */

#define kXMP_NS_XMP        "http://ns.adobe.com/xap/1.0/"

#define kXMP_NS_XMP_Rights "http://ns.adobe.com/xap/1.0/rights/"
#define kXMP_NS_XMP_MM     "http://ns.adobe.com/xap/1.0/mm/"
#define kXMP_NS_XMP_BJ     "http://ns.adobe.com/xap/1.0/bj/"

#define kXMP_NS_PDF        "http://ns.adobe.com/pdf/1.3/"
#define kXMP_NS_Photoshop  "http://ns.adobe.com/photoshop/1.0/"
#define kXMP_NS_PSAlbum    "http://ns.adobe.com/album/1.0/"
#define kXMP_NS_EXIF       "http://ns.adobe.com/exif/1.0/"
#define kXMP_NS_EXIF_Aux   "http://ns.adobe.com/exif/1.0/aux/"
#define kXMP_NS_TIFF       "http://ns.adobe.com/tiff/1.0/"
#define kXMP_NS_PNG        "http://ns.adobe.com/png/1.0/"
#define kXMP_NS_SWF        "http://ns.adobe.com/swf/1.0/"
#define kXMP_NS_JPEG       "http://ns.adobe.com/jpeg/1.0/"
#define kXMP_NS_JP2K       "http://ns.adobe.com/jp2k/1.0/"
#define kXMP_NS_CameraRaw  "http://ns.adobe.com/camera-raw-settings/1.0/"
#define kXMP_NS_DM         "http://ns.adobe.com/xmp/1.0/DynamicMedia/"
#define kXMP_NS_ASF        "http://ns.adobe.com/asf/1.0/"
#define kXMP_NS_WAV        "http://ns.adobe.com/xmp/wav/1.0/"

#define kXMP_NS_XMP_Note   "http://ns.adobe.com/xmp/note/"

#define kXMP_NS_AdobeStockPhoto "http://ns.adobe.com/StockPhoto/1.0/"

/**
 *  \name XML namespace constants for qualifiers and structured property fields.
 *  @{
 */

/**
 *  \def kXMP_NS_XMP_IdentifierQual
 *  \brief The XML namespace for qualifiers of the xmp:Identifier property.
 */

/**
 *  \def kXMP_NS_XMP_Dimensions
 *  \brief The XML namespace for fields of the Dimensions type.
 */

/**
 *  \def kXMP_NS_XMP_Image
 *  \brief The XML namespace for fields of a graphical image. Used for the Thumbnail type.
 */

/**
 *  \def kXMP_NS_XMP_ResourceEvent
 *  \brief The XML namespace for fields of the ResourceEvent type.
 */

/**
 *  \def kXMP_NS_XMP_ResourceRef
 *  \brief The XML namespace for fields of the ResourceRef type.
 */

/**
 *  \def kXMP_NS_XMP_ST_Version
 *  \brief The XML namespace for fields of the Version type.
 */

/**
 *  \def kXMP_NS_XMP_ST_Job
 *  \brief The XML namespace for fields of the JobRef type.
 */

/**
 *  @}
 */

#define kXMP_NS_XMP_IdentifierQual "http://ns.adobe.com/xmp/Identifier/qual/1.0/"
#define kXMP_NS_XMP_Dimensions     "http://ns.adobe.com/xap/1.0/sType/Dimensions#"
#define kXMP_NS_XMP_Text           "http://ns.adobe.com/xap/1.0/t/"
#define kXMP_NS_XMP_PagedFile      "http://ns.adobe.com/xap/1.0/t/pg/"
#define kXMP_NS_XMP_Graphics       "http://ns.adobe.com/xap/1.0/g/"
#define kXMP_NS_XMP_Image          "http://ns.adobe.com/xap/1.0/g/img/"
#define kXMP_NS_XMP_Font           "http://ns.adobe.com/xap/1.0/sType/Font#"
#define kXMP_NS_XMP_ResourceEvent  "http://ns.adobe.com/xap/1.0/sType/ResourceEvent#"
#define kXMP_NS_XMP_ResourceRef    "http://ns.adobe.com/xap/1.0/sType/ResourceRef#"
#define kXMP_NS_XMP_ST_Version     "http://ns.adobe.com/xap/1.0/sType/Version#"
#define kXMP_NS_XMP_ST_Job         "http://ns.adobe.com/xap/1.0/sType/Job#"
#define kXMP_NS_XMP_ManifestItem   "http://ns.adobe.com/xap/1.0/sType/ManifestItem#"

/* Deprecated constant names */
#define kXMP_NS_XMP_T     "http://ns.adobe.com/xap/1.0/t/"
#define kXMP_NS_XMP_T_PG  "http://ns.adobe.com/xap/1.0/t/pg/"
#define kXMP_NS_XMP_G_IMG "http://ns.adobe.com/xap/1.0/g/img/"

/**
 *  \name XML namespace constants from outside Adobe.
 *  @{
 */

/**
 *  \def kXMP_NS_DC
 *  \brief The XML namespace for the Dublin Core schema.
 */

/**
 *  \def kXMP_NS_IPTCCore
 *  \brief The XML namespace for the IPTC Core schema.
 */

/**
 *  \def kXMP_NS_RDF
 *  \brief The XML namespace for RDF.
 */

/**
 *  \def kXMP_NS_XML
 *  \brief The XML namespace for XML.
 */

/**
 *  @}
 */

#define kXMP_NS_DC       "http://purl.org/dc/elements/1.1/"

#define kXMP_NS_IPTCCore "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"

#define kXMP_NS_PDFA_Schema    "http://www.aiim.org/pdfa/ns/schema#"
#define kXMP_NS_PDFA_Property  "http://www.aiim.org/pdfa/ns/property#"
#define kXMP_NS_PDFA_Type      "http://www.aiim.org/pdfa/ns/type#"
#define kXMP_NS_PDFA_Field     "http://www.aiim.org/pdfa/ns/field#"
#define kXMP_NS_PDFA_ID        "http://www.aiim.org/pdfa/ns/id/"
#define kXMP_NS_PDFA_Extension "http://www.aiim.org/pdfa/ns/extension/"

#define kXMP_NS_PDFX           "http://ns.adobe.com/pdfx/1.3/"
#define	kXMP_NS_PDFX_ID        "http://www.npes.org/pdfx/ns/id/"

#define kXMP_NS_RDF      "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define kXMP_NS_XML      "http://www.w3.org/XML/1998/namespace"


/* ============================================================================================== */
/* Enums and macros used for option bits */
/* ===================================== */

#define kXMP_ArrayLastItem      ((XMP_Index)(-1L))
#define kXMP_UseNullTermination ((XMP_StringLen)(~0UL))

#define kXMP_NoOptions          ((XMP_OptionBits)0UL)

#define XMP_SetOption(var,opt)      var |= (opt)
#define XMP_ClearOption(var,opt)    var &= ~(opt)
#define XMP_TestOption(var,opt)     (((var) & (opt)) != 0)
#define XMP_OptionIsSet(var,opt)    (((var) & (opt)) != 0)
#define XMP_OptionIsClear(var,opt)  (((var) & (opt)) == 0)

#define XMP_PropIsSimple(opt)       (((opt) & kXMP_PropCompositeMask) == 0)
#define XMP_PropIsStruct(opt)       (((opt) & kXMP_PropValueIsStruct) != 0)
#define XMP_PropIsArray(opt)        (((opt) & kXMP_PropValueIsArray) != 0)
#define XMP_ArrayIsUnordered(opt)   (((opt) & kXMP_PropArrayIsOrdered) == 0)
#define XMP_ArrayIsOrdered(opt)     (((opt) & kXMP_PropArrayIsOrdered) != 0)
#define XMP_ArrayIsAlternate(opt)   (((opt) & kXMP_PropArrayIsAlternate) != 0)
#define XMP_ArrayIsAltText(opt)     (((opt) & kXMP_PropArrayIsAltText) != 0)

#define XMP_PropHasQualifiers(opt)  (((opt) & kXMP_PropHasQualifiers) != 0)
#define XMP_PropIsQualifier(opt)    (((opt) & kXMP_PropIsQualifier) != 0)
#define XMP_PropHasLang(opt)        (((opt) & kXMP_PropHasLang) != 0)

#define XMP_NodeIsSchema(opt)       (((opt) & kXMP_SchemaNode) != 0)
#define XMP_PropIsAlias(opt)        (((opt) & kXMP_PropIsAlias) != 0)

/* ---------------------------------------------------------------------------------------------- */

enum {  /* Option bits returned from the TXMPMeta::GetXyz functions. */

    /* Options relating to the XML string form of the property value. */
    kXMP_PropValueIsURI       = 0x00000002UL,  /* The value is a URI, use rdf:resource attribute. DISCOURAGED */

    /* Options relating to qualifiers attached to a property. */
    kXMP_PropHasQualifiers    = 0x00000010UL,  /* The property has qualifiers, includes rdf:type and xml:lang. */
    kXMP_PropIsQualifier      = 0x00000020UL,  /* This is a qualifier, includes rdf:type and xml:lang. */
    kXMP_PropHasLang          = 0x00000040UL,  /* Implies kXMP_PropHasQualifiers, property has xml:lang. */
    kXMP_PropHasType          = 0x00000080UL,  /* Implies kXMP_PropHasQualifiers, property has rdf:type. */

    /* Options relating to the data structure form. */
    kXMP_PropValueIsStruct    = 0x00000100UL,  /* The value is a structure with nested fields. */
    kXMP_PropValueIsArray     = 0x00000200UL,  /* The value is an array (RDF alt/bag/seq). */
    kXMP_PropArrayIsUnordered = kXMP_PropValueIsArray,  /* The item order does not matter. */
    kXMP_PropArrayIsOrdered   = 0x00000400UL,  /* Implies kXMP_PropValueIsArray, item order matters. */
    kXMP_PropArrayIsAlternate = 0x00000800UL,  /* Implies kXMP_PropArrayIsOrdered, items are alternates. */

    /* Additional struct and array options. */
    kXMP_PropArrayIsAltText   = 0x00001000UL,  /* Implies kXMP_PropArrayIsAlternate, items are localized text. */
    /* kXMP_InsertBeforeItem  = 0x00004000UL,  ! Used by SetXyz functions. */
    /* kXMP_InsertAfterItem   = 0x00008000UL,  ! Used by SetXyz functions. */

    /* Other miscellaneous options. */
    kXMP_PropIsAlias          = 0x00010000UL,  /* This property is an alias name for another property. */
    kXMP_PropHasAliases       = 0x00020000UL,  /* This property is the base value for a set of aliases. */
    kXMP_PropIsInternal       = 0x00040000UL,  /* This property is an "internal" property, owned by applications. */
    kXMP_PropIsStable         = 0x00100000UL,  /* This property is not derived from the document content. */
    kXMP_PropIsDerived        = 0x00200000UL,  /* This property is derived from the document content. */
    /* kXMPUtil_AllowCommas   = 0x10000000UL,  ! Used by TXMPUtils::CatenateArrayItems and ::SeparateArrayItems. */
    /* kXMP_DeleteExisting    = 0x20000000UL,  ! Used by TXMPMeta::SetXyz functions to delete any pre-existing property. */
    /* kXMP_SchemaNode        = 0x80000000UL,  ! Returned by iterators - #define to avoid warnings */

    /* Masks that are multiple flags. */
    kXMP_PropArrayFormMask    = kXMP_PropValueIsArray | kXMP_PropArrayIsOrdered | kXMP_PropArrayIsAlternate | kXMP_PropArrayIsAltText,
    kXMP_PropCompositeMask    = kXMP_PropValueIsStruct | kXMP_PropArrayFormMask,  /* Is it simple or composite (array or struct)? */
    kXMP_ImplReservedMask     = 0x70000000L   /* Reserved for transient use by the implementation. */
};

#define kXMP_SchemaNode ((XMP_OptionBits)0x80000000UL)

enum {  /* Option bits for the TXMPMeta::SetXyz functions. */

    /* Options shared with GetXyz functions. */
    /*
    kXMP_PropValueIsURI        = 0x00000002UL, DISCOURAGED
    kXMP_PropValueIsStruct     = 0x00000100UL,
    kXMP_PropValueIsArray      = 0x00000200UL,
    kXMP_PropArrayIsOrdered    = 0x00000400UL,
    kXMP_PropArrayIsAlternate  = 0x00000800UL,
    kXMP_PropArrayIsAltText    = 0x00001000UL,
    kXMP_PropValueIsCompact    = 0x00002000UL, RESERVED
    */

    /* Options for array item location. */
    kXMP_InsertBeforeItem      = 0x00004000UL,  /* Insert a new item before the given index. */
    kXMP_InsertAfterItem       = 0x00008000UL,  /* Insert a new item after the given index. */

    /* Miscellaneous options */
    kXMP_DeleteExisting        = 0x20000000UL,  /* Delete any pre-existing property. */

    /* Masks that are multiple flags. */
    kXMP_PropValueOptionsMask  = kXMP_PropValueIsURI,
    kXMP_PropArrayLocationMask = kXMP_InsertBeforeItem | kXMP_InsertAfterItem

};

/* ---------------------------------------------------------------------------------------------- */

enum {  /* Options for TXMPMeta::ParseFromBuffer. */
    kXMP_RequireXMPMeta   = 0x0001UL,  /* Require a surrounding x:xmpmeta element. */
    kXMP_ParseMoreBuffers = 0x0002UL,  /* This is the not last input buffer for this parse stream. */
    kXMP_StrictAliasing   = 0x0004UL   /* Do not reconcile alias differences, throw an exception. */
};

enum {  /* Options for TXMPMeta::SerializeToBuffer. */

    /* *** Option to remove empty struct/array, or leaf with empty value? */

    kXMP_OmitPacketWrapper   = 0x0010UL,  /* Omit the XML packet wrapper. */
    kXMP_ReadOnlyPacket      = 0x0020UL,  /* Default is a writeable packet. */
    kXMP_UseCompactFormat    = 0x0040UL,  /* Use a compact form of RDF. */

    kXMP_IncludeThumbnailPad = 0x0100UL,  /* Include a padding allowance for a thumbnail image. */
    kXMP_ExactPacketLength   = 0x0200UL,  /* The padding parameter is the overall packet length. */
    kXMP_WriteAliasComments  = 0x0400UL,  /* Show aliases as XML comments. */
    kXMP_OmitAllFormatting   = 0x0800UL,  /* Omit all formatting whitespace. */

    _XMP_LittleEndian_Bit    = 0x0001UL,  /* ! Don't use directly, see the combined values below! */
    _XMP_UTF16_Bit           = 0x0002UL,
    _XMP_UTF32_Bit           = 0x0004UL,

    kXMP_EncodingMask        = 0x0007UL,
    kXMP_EncodeUTF8          = 0UL,
    kXMP_EncodeUTF16Big      = _XMP_UTF16_Bit,
    kXMP_EncodeUTF16Little   = _XMP_UTF16_Bit | _XMP_LittleEndian_Bit,
    kXMP_EncodeUTF32Big      = _XMP_UTF32_Bit,
    kXMP_EncodeUTF32Little   = _XMP_UTF32_Bit | _XMP_LittleEndian_Bit

};

/* ---------------------------------------------------------------------------------------------- */

enum {  /* Options for TXMPIterator construction. */

    kXMP_IterClassMask      = 0x00FFUL,  /* The low 8 bits are an enum of what data structure to iterate. */
    kXMP_IterProperties     = 0x0000UL,  /* Iterate the property tree of a TXMPMeta object. */
    kXMP_IterAliases        = 0x0001UL,  /* Iterate the global alias table. */
    kXMP_IterNamespaces     = 0x0002UL,  /* Iterate the global namespace table. */
    kXMP_IterJustChildren   = 0x0100UL,  /* Just do the immediate children of the root, default is subtree. */
    kXMP_IterJustLeafNodes  = 0x0200UL,  /* Just do the leaf nodes, default is all nodes in the subtree. */
    kXMP_IterJustLeafName   = 0x0400UL,  /* Return just the leaf part of the path, default is the full path. */
    kXMP_IterIncludeAliases = 0x0800UL,  /* Include aliases, default is just actual properties. */

    kXMP_IterOmitQualifiers = 0x1000UL   /* Omit all qualifiers. */

};

enum {  /* Options for TXMPIterator::Skip. */
    kXMP_IterSkipSubtree    = 0x0001UL,  /* Skip the subtree below the current node. */
    kXMP_IterSkipSiblings   = 0x0002UL   /* Skip the subtree below and remaining siblings of the current node. */
};

/* ---------------------------------------------------------------------------------------------- */

enum {  /* Options for TXMPUtils::CatenateArrayItems and TXMPUtils::SeparateArrayItems. */

    /* Options shared with GetXyz functions. */
    /*
    kXMP_PropValueIsArray     = 0x00000200UL,
    kXMP_PropArrayIsOrdered   = 0x00000400UL,
    kXMP_PropArrayIsAlternate = 0x00000800UL,
    kXMP_PropArrayIsAltText   = 0x00001000UL,
    */

    kXMPUtil_AllowCommas      = 0x10000000UL   /* Allow commas in item values, default is separator. */

};

enum {  /* Options for TXMPUtils::RemoveProperties and TXMPUtils::AppendProperties. */
    kXMPUtil_DoAllProperties   = 0x0001UL,  /* Do all properties, default is just external properties. */
    kXMPUtil_ReplaceOldValues  = 0x0002UL,  /* Replace existing values, default is to leave them. */
    kXMPUtil_DeleteEmptyValues = 0x0004UL,  /* Delete properties if the new value is empty. */
    kXMPUtil_IncludeAliases    = 0x0800UL   /* == kXMP_IterIncludeAliases */
};

/* ============================================================================================== */
/* Types and Constants for XMP File Handler */
/* ======================================== */

enum {

    /* Public file formats. Hex used to avoid gcc warnings. */
    /* ! Leave them as big endian. There seems to be no decent way on UNIX to determine the target */
    /* ! endianness at compile time. Forcing it on the client isn't acceptable.                    */

    kXMP_PDFFile             = 0x50444620UL,  /* 'PDF ' */
    kXMP_PostScriptFile      = 0x50532020UL,  /* 'PS  ', general PostScript following DSC conventions. */
    kXMP_EPSFile             = 0x45505320UL,  /* 'EPS ', encapsulated PostScript. */

    kXMP_JPEGFile            = 0x4A504547UL,  /* 'JPEG' */
    kXMP_JPEG2KFile          = 0x4A505820UL,  /* 'JPX ', ISO 15444-1 */
    kXMP_TIFFFile            = 0x54494646UL,  /* 'TIFF' */
    kXMP_GIFFile             = 0x47494620UL,  /* 'GIF ' */
    kXMP_PNGFile             = 0x504E4720UL,  /* 'PNG ' */
    
    kXMP_SWFFile             = 0x53574620UL,  /* 'SWF ' */
    kXMP_FLAFile             = 0x464C4120UL,  /* 'FLA ' */
    kXMP_FLVFile             = 0x464C5620UL,  /* 'FLV ' */

    kXMP_MOVFile             = 0x4D4F5620UL,  /* 'MOV ', Quicktime */
    kXMP_AVIFile             = 0x41564920UL,  /* 'AVI ' */
    kXMP_CINFile             = 0x43494E20UL,  /* 'CIN ', Cineon */
    kXMP_WAVFile             = 0x57415620UL,  /* 'WAV ' */
    kXMP_MP3File             = 0x4D503320UL,  /* 'MP3 ' */
    kXMP_SESFile             = 0x53455320UL,  /* 'SES ', Audition session */
    kXMP_CELFile             = 0x43454C20UL,  /* 'CEL ', Audition loop */
    kXMP_MPEGFile            = 0x4D504547UL,  /* 'MPEG' */
    kXMP_MPEG2File           = 0x4D503220UL,  /* 'MP2 ' */
    kXMP_MPEG4File           = 0x4D503420UL,  /* 'MP4 ', ISO 14494-12 and -14 */
    kXMP_WMAVFile            = 0x574D4156UL,  /* 'WMAV', Windows Media Audio and Video */
    kXMP_AIFFFile            = 0x41494646UL,  /* 'AIFF' */

    kXMP_HTMLFile            = 0x48544D4CUL,  /* 'HTML' */
    kXMP_XMLFile             = 0x584D4C20UL,  /* 'XML ' */
    kXMP_TextFile            = 0x74657874UL,  /* 'text' */

    /* Adobe application file formats. */

    kXMP_PhotoshopFile       = 0x50534420UL,  /* 'PSD ' */
    kXMP_IllustratorFile     = 0x41492020UL,  /* 'AI  ' */
    kXMP_InDesignFile        = 0x494E4444UL,  /* 'INDD' */
    kXMP_AEProjectFile       = 0x41455020UL,  /* 'AEP ' */
    kXMP_AEProjTemplateFile  = 0x41455420UL,  /* 'AET ', After Effects Project Template */
    kXMP_AEFilterPresetFile  = 0x46465820UL,  /* 'FFX ' */
    kXMP_EncoreProjectFile   = 0x4E434F52UL,  /* 'NCOR' */
    kXMP_PremiereProjectFile = 0x5052504AUL,  /* 'PRPJ' */
    kXMP_PremiereTitleFile   = 0x5052544CUL,  /* 'PRTL' */

    /* Catch all. */

    kXMP_UnknownFile         = 0x20202020UL   /* '    ' */

};
typedef XMP_Uns32 XMP_FileFormat;

/* ---------------------------------------------------------------------------------------------- */

enum {
    kXMP_CharLittleEndianMask = 1,
    kXMP_Char16BitMask        = 2,  /* Don't use these directly. */
    kXMP_Char32BitMask        = 4
};

enum {  /* The values allow easy testing for 16/32 bit and big/little endian. */
    kXMP_Char8Bit        = 0,
    kXMP_Char16BitBig    = kXMP_Char16BitMask,
    kXMP_Char16BitLittle = kXMP_Char16BitMask | kXMP_CharLittleEndianMask,
    kXMP_Char32BitBig    = kXMP_Char32BitMask,
    kXMP_Char32BitLittle = kXMP_Char32BitMask | kXMP_CharLittleEndianMask,
    kXMP_CharUnknown     = 1 /* ! A bit of a hack, for variable or not-yet-known cases. */
};

#define XMP_CharFormIs16Bit(f)         ( ((int)(f) & kXMP_Char16BitMask) != 0 )
#define XMP_CharFormIs32Bit(f)         ( ((int)(f) & kXMP_Char32BitMask) != 0 )

#define XMP_CharFormIsBigEndian(f)     ( ((int)(f) & kXMP_CharLittleEndianMask) == 0 )
#define XMP_CharFormIsLittleEndian(f)  ( ((int)(f) & kXMP_CharLittleEndianMask) != 0 )

#define XMP_GetCharSize(f)             ( ((int)(f)&6) == 0 ? 1 : (int)(f)&6 )

#define XMP_CharToSerializeForm(cf)    ( (XMP_OptionBits)(cf) )
#define XMP_CharFromSerializeForm(sf)  ( (XMP_Uns8)(sf) )

struct XMP_PacketInfo {
    XMP_Int64 offset;
    XMP_Int32 length;
    XMP_Int32 padSize;   /* Zero if unknown. */
    XMP_Uns8  charForm;
    XMP_Bool  writeable;
    XMP_Uns8  pad1, pad2;
    #if __cplusplus
        XMP_PacketInfo() : offset(0), length(0), padSize(0), charForm(0), writeable(0), pad1(0), pad2(0) {};
    #endif
};
#if ! __cplusplus
    typedef struct XMP_PacketInfo XMP_PacketInfo;
#endif
enum { kXMP_PacketInfoVersion = 3 };

/* ---------------------------------------------------------------------------------------------- */

enum {  /* Values for XMP_ThumbnailInfo.tnailFormat. */
    kXMP_UnknownTNail = 0, /* The thumbnail data has an unknown format. */
    kXMP_JPEGTNail    = 1, /* The thumbnail data is a JPEG stream, presumably compressed. */
    kXMP_TIFFTNail    = 2, /* The thumbnail data is a TIFF stream, presumably uncompressed. */
    kXMP_PShopTNail   = 3  /* The thumbnail data is in the format of Photoshop Image Resource 1036. */
};

struct XMP_ThumbnailInfo {
    XMP_FileFormat   fileFormat;  /* The format of the containing file. */
    XMP_Uns32        fullWidth, fullHeight;    /* Full image size in pixels. */
    XMP_Uns32        tnailWidth, tnailHeight;  /* Thumbnail image size in pixels. */
    XMP_Uns16        fullOrientation, tnailOrientation;  /* Orientation of full image and thumbnail, as defined by Exif for tag 274. */
    const XMP_Uns8 * tnailImage;   /* Raw data from the host file, valid for life of the owning XMPFiles object. Do not modify! */
    XMP_Uns32        tnailSize;    /* The size in bytes of the tnailImage data. */
    XMP_Uns8         tnailFormat;  /* The format of the tnailImage data. */
    XMP_Uns8         pad1, pad2, pad3;
    #if __cplusplus
        XMP_ThumbnailInfo() : fileFormat(kXMP_UnknownFile), fullWidth(0), fullHeight(0),
                              tnailWidth(0), tnailHeight(0), fullOrientation(0), tnailOrientation(0),
                              tnailImage(0), tnailSize(0), tnailFormat(kXMP_UnknownTNail) {};
    #endif
};
#if ! __cplusplus
    typedef struct XMP_ThumbnailInfo XMP_ThumbnailInfo;
#endif
enum { kXMP_ThumbnailInfoVersion = 1 };

/* ---------------------------------------------------------------------------------------------- */

#define kXMPFiles_UnknownOffset	((XMP_Int64)-1)
#define kXMPFiles_UnknownLength	((XMP_Int32)-1)

enum {  /* Options for Initialize. */
    kXMPFiles_NoQuickTimeInit = 0x0001 /* Don't initialize QuickTime, the client will. */
};

enum {	/* Options for GetFormatInfo. */ 
    kXMPFiles_CanInjectXMP        = 0x00000001, /* Can inject first-time XMP into an existing file. */
    kXMPFiles_CanExpand           = 0x00000002, /* Can expand XMP or other metadata in an existing file. */
    kXMPFiles_CanRewrite          = 0x00000004, /* Can copy one file to another, writing new metadata. */
    kXMPFiles_PrefersInPlace      = 0x00000008, /* Can expand, but prefers in-place update. */
    kXMPFiles_CanReconcile        = 0x00000010, /* Supports reconciliation between XMP and other forms. */
    kXMPFiles_AllowsOnlyXMP       = 0x00000020, /* Allows access to just the XMP, ignoring other forms. */
    kXMPFiles_ReturnsRawPacket    = 0x00000040, /* File handler returns raw XMP packet information. */
    kXMPFiles_ReturnsTNail        = 0x00000080, /* File handler returns native thumbnail. */
    kXMPFiles_HandlerOwnsFile     = 0x00000100, /* The file handler does the file open and close. */
    kXMPFiles_AllowsSafeUpdate    = 0x00000200,	/* The file handler allows crash-safe file updates. */
    kXMPFiles_NeedsReadOnlyPacket = 0x00000400,	/* The file format needs the XMP packet to be read-only. */
    kXMPFiles_UsesSidecarXMP      = 0x00000800	/* The file handler uses a "sidecar" file for the XMP. */
};

enum {	/* Options for OpenFile. */
    kXMPFiles_OpenForRead           = 0x00000001, /* Open for read-only access. */
    kXMPFiles_OpenForUpdate         = 0x00000002, /* Open for reading and writing. */
    kXMPFiles_OpenOnlyXMP           = 0x00000004, /* Only the XMP is wanted, allows space/time optimizations. */
    kXMPFiles_OpenCacheTNail        = 0x00000008, /* Cache thumbnail if possible, GetThumbnail will be called. */
    kXMPFiles_OpenStrictly          = 0x00000010, /* Be strict about locating XMP and reconciling with other forms. */
    kXMPFiles_OpenUseSmartHandler   = 0x00000020, /* Require the use of a smart handler. */
    kXMPFiles_OpenUsePacketScanning = 0x00000040, /* Force packet scanning, don't use a smart handler. */
    kXMPFiles_OpenLimitedScanning   = 0x00000080, /* Only packet scan files "known" to need scanning. */
    kXMPFiles_OpenInBackground      = 0x10000000  /* Set if calling from background thread. */
};

/* A note about kXMPFiles_OpenInBackground. The XMPFiles handler for .mov files currently uses      */
/* QuickTime. On Macintosh, calls to Enter/ExitMovies versus Enter/ExitMoviesOnThread must be made. */
/* This option is used to signal background use so that the .mov handler can behave appropriately.  */

enum {	/* Options for CloseFile. */
    kXMPFiles_UpdateSafely = 0x0001	/* Write into a temporary file and swap for crash safety. */
};

/* ============================================================================================== */
/* Exception codes */
/* =============== */

/**
 *  \name Exception codes.
 *  @{
 *  XMP tookit errors result in throwing an \c XMP_Error exception. Any exception thrown within the
 *  XMP toolkit is caught in the toolkit and rethrown as an \c XMP_Error. The \c XMP_Error struct
 *  contains a numeric code and an English explanation. New numeric codes may be added at any time.
 *  There are typically many possible explanations for each numeric code. The explanations try to
 *  be precise about the specific circumstances causing the error.
 *
 *  \note The explanation string is for debugging use only. It must not be shown to users in a
 *  final product. It is written for developers not users, and never localized.
 */

#if ! __cplusplus

    typedef struct XMP_Error {
        XMP_Int32     id;
        XMP_StringPtr errMsg;
    } XMP_Error;

#else

    class XMP_Error {
    public:
        XMP_Error ( XMP_Int32 _id, XMP_StringPtr _errMsg ) : id(_id), errMsg(_errMsg) {};
        inline XMP_Int32     GetID() const     { return id; };
        inline XMP_StringPtr GetErrMsg() const { return errMsg; };
    private:
        XMP_Int32     id;
        XMP_StringPtr errMsg;
    };

#endif

enum {

    /* More or less generic error codes. */
    kXMPErr_Unknown          =   0,
    kXMPErr_TBD              =   1,
    kXMPErr_Unavailable      =   2,
    kXMPErr_BadObject        =   3,
    kXMPErr_BadParam         =   4,
    kXMPErr_BadValue         =   5,
    kXMPErr_AssertFailure    =   6,
    kXMPErr_EnforceFailure   =   7,
    kXMPErr_Unimplemented    =   8,
    kXMPErr_InternalFailure  =   9,
    kXMPErr_Deprecated       =  10,
    kXMPErr_ExternalFailure  =  11,
    kXMPErr_UserAbort        =  12,
    kXMPErr_StdException     =  13,
    kXMPErr_UnknownException =  14,
    kXMPErr_NoMemory         =  15,

    /* More specific parameter error codes.  */
    kXMPErr_BadSchema        = 101,
    kXMPErr_BadXPath         = 102,
    kXMPErr_BadOptions       = 103,
    kXMPErr_BadIndex         = 104,
    kXMPErr_BadIterPosition  = 105,
    kXMPErr_BadParse         = 106,
    kXMPErr_BadSerialize     = 107,
    kXMPErr_BadFileFormat    = 108,
    kXMPErr_NoFileHandler    = 109,
    kXMPErr_TooLargeForJPEG  = 110,

    /* File format and internal structure error codes. */
    kXMPErr_BadXML           = 201,
    kXMPErr_BadRDF           = 202,
    kXMPErr_BadXMP           = 203,
    kXMPErr_EmptyIterator    = 204,
    kXMPErr_BadUnicode       = 205,
    kXMPErr_BadTIFF          = 206,
    kXMPErr_BadJPEG          = 207,
    kXMPErr_BadPSD           = 208,
    kXMPErr_BadPSIR          = 209,
    kXMPErr_BadIPTC          = 210,
    kXMPErr_BadMPEG          = 211

};

/**
 *  @}
 */


/* ============================================================================================== */
/* Client callbacks */
/* ================ */


/**
 *  \name Special purpose callback functions
 *  @{
 */

/**
 *  \typedef XMP_Status
 *  \brief A signed 32 bit integer used as a status result for the output callback routine.
 *  Zero means no error, all other values except -1 are private to the callback. The callback is
 *  wrapped to prevent exceptions being thrown across DLL boundaries. Any exceptions thrown out
 *  of the callback cause a return status of -1.
 */

/**
 *  \typedef XMP_TextOutputProc
 *  \brief Direct text output from the XMP toolkit, such as debugging dumps, is done using client
 *  supplied callbacks. The callback is invoked one or more times for each line of output. The
 *  end of a line is signaled by a '\n' character at the end of the buffer. Formatting newlines
 *  are never present in the middle of a buffer, but values of properties might contain any UTF-8
 *  characters.  A success/fail status is returned by the callback. Any failure result aborts
 *  the output.
 */

typedef XMP_Int32 XMP_Status;

typedef XMP_Status (* XMP_TextOutputProc) ( void *        refCon,
                                            XMP_StringPtr buffer,
                                            XMP_StringLen bufferSize );

/**
 *  @}
 */


/* ============================================================================================== */
/* Stuff with no better place to be */
/* ================================ */

typedef struct XMP_VersionInfo {
    XMP_Uns8      major;    /* The primary release number, the "1" in version "1.2.3". */
    XMP_Uns8      minor;    /* The secondary release number, the "2" in version "1.2.3". */
    XMP_Uns8      micro;    /* The tertiary release number, the "3" in version "1.2.3". */
    XMP_Bool      isDebug;  /* Really a 0/1 bool value. True if this is a debug build. */
    XMP_Uns32     build;    /* A rolling build number, monotonically increasing in a release. */
    XMP_Uns32     flags;    /* Individual feature implementation flags. */
    XMP_StringPtr message;  /* A comprehensive version information string. */
} XMP_VersionInfo;

typedef bool (* XMP_AbortProc) ( void * arg );	/* Used by SXMPFiles::SetAbortProc. */

/* ============================================================================================== */

#if __cplusplus
} /* extern "C" */
#endif

#endif  /* __XMP_Const_h__ */
