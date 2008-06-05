#ifndef __TXMPUtils_hpp__
#define __TXMPUtils_hpp__ 1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMPSDK.hpp"
#endif

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

//  ================================================================================================
/// \file TXMPUtils.hpp
/// \brief Template class for the XMP Toolkit utility services.
///
/// \c TXMPUtils is the template class providing utility services for the XMP Toolkit. It should be
/// instantiated with a string class such as <tt>std::string</tt>. Please read the general toolkit
/// usage notes for information about the overall architecture of the XMP API.
//  ================================================================================================

//  ================================================================================================
/// \class TXMPUtils TXMPUtils.hpp
/// \brief Template class for the XMP Toolkit utility services.
///
/// \c TXMPUtils is the template class providing utility services for the XMP Toolkit. It should be
/// instantiated with a string class such as <tt>std::string</tt>. Please read the general toolkit
/// usage notes for information about the overall architecture of the XMP API.
///
/// This is a class for C++ scoping purposes only. It has only static functions, you cannot create
/// an object. These are all functions that layer cleanly on top of the core XMP toolkit. The
/// template wraps a string class around the raw XMP API, so that output strings are automatically
/// copied and access is fully thread safe.  String objects are only necessary for output strings.
/// Input strings are literals and passed as typical C <tt>const char *</tt>.
///
/// The template parameter, class \c TtStringObj, is described in the XMPSDK.hpp umbrella header.
//  ================================================================================================

template <class tStringObj>
class TXMPUtils {

public:

    // =============================================================================================
    // No constructors or destructor declared or needed
    // ================================================

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name Path composition functions
    /// @{
    /// These functions provide support for composing path expressions to deeply nested properties.
    /// The functions in \c TXMPMeta such as \c GetProperty, \c GetArrayItem, and \c GetStructField
    /// provide easy access to top level simple properties, items in top level arrays, and fields
    /// of top level structs. They do not provide convenient access to more complex things like
    /// fields several levels deep in a complex struct, or fields within an array of structs, or
    /// items of an array that is a field of a struct. These functions can also be used to compose
    /// paths to top level array items or struct fields so that you can use the binary accessors
    /// like \c GetProperty_Int.
    ///
    /// You can use these functions is to compose a complete path expression, or all but the last
    /// component. Suppose you have a property that is an array of integers within a struct. You can
    /// access one of the array items like this:
    ///
    /// \verbatim
    ///  SXMPUtils::ComposeStructFieldPath ( schemaNS, "Struct", fieldNS, "Array", &path );
    ///  SXMPUtils::ComposeArrayItemPath ( schemaNS, path, index, &path );
    ///  exists = xmpObj.GetProperty_Int ( schemaNS, path, &value, &options );
    /// \endverbatim
    ///
    /// You could also use this code if you want the string form of the integer:
    ///
    /// \verbatim
    ///  SXMPUtils::ComposeStructFieldPath ( schemaNS, "Struct", fieldNS, "Array", &path );
    ///  xmpObj.GetArrayItem ( schemaNS, path, index, &value, &options );
    /// \endverbatim
    ///
    /// \note It might look confusing that the \c schemaNS is passed in all of the calls above. This
    /// is because the XMP toolkit keeps the top level "schema" namespace separate from the rest
    /// of the path expression.

    //  --------------------------------------------------------------------------------------------
    /// \brief Compose the path expression for an item in an array.
    ///
    /// \param schemaNS The namespace URI for the array. Must not be null or the empty string.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// \param itemIndex The index of the desired item. Arrays in XMP are indexed from 1. The
    /// constant \c kXMP_ArrayLastItem always refers to the last existing array item.
    ///
    /// \param fullPath A pointer to the string that will be assigned the composed path. This will
    /// be of the form <tt>ns:arrayName[i]</tt>, where "ns" is the prefix for \c schemaNS and "i"
    /// is the decimal representation of \c itemIndex. If the value of \c itemIndex is
    /// \c kXMP_ArrayLastItem, the path is <tt>ns:arrayName[last()]</tt>.

    static void
    ComposeArrayItemPath ( XMP_StringPtr schemaNS,
                           XMP_StringPtr arrayName,
                           XMP_Index     itemIndex,
                           tStringObj *  fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief Compose the path expression for a field in a struct.
    ///
    /// \param schemaNS The namespace URI for the struct. Must not be null or the empty string.
    ///
    /// \param structName The name of the struct. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// \param fieldNS The namespace URI for the field. Must not be null or the empty string.
    ///
    /// \param fieldName The name of the field. Must be a simple XML name, must not be null or the
    /// empty string.
    ///
    /// \param fullPath A pointer to the string that will be assigned the composed path. This will
    /// be of the form <tt>ns:structName/fNS:fieldName</tt>, where "ns" is the prefix for
    /// \c schemaNS and "fNS" is the prefix for \c fieldNS.

    static void
    ComposeStructFieldPath ( XMP_StringPtr schemaNS,
                             XMP_StringPtr structName,
                             XMP_StringPtr fieldNS,
                             XMP_StringPtr fieldName,
                             tStringObj *  fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief Compose the path expression for a qualifier.
    ///
    /// \param schemaNS The namespace URI for the property to which the qualifier is attached. Must
    /// not be null or the empty string.
    ///
    /// \param propName The name of the property to which the qualifier is attached. May be a general
    /// path expression, must not be null or the empty string.
    ///
    /// \param qualNS The namespace URI for the qualifier. May be null or the empty string if the
    /// qualifier is in the XML empty namespace.
    ///
    /// \param qualName The name of the qualifier. Must be a simple XML name, must not be null or the
    /// empty string.
    ///
    /// \param fullPath A pointer to the string that will be assigned the composed path. This will
    /// be of the form <tt>ns:propName/?qNS:qualName</tt>, where "ns" is the prefix for \c schemaNS
    /// and "qNS" is the prefix for \c qualNS.

    static void
    ComposeQualifierPath ( XMP_StringPtr schemaNS,
                           XMP_StringPtr propName,
                           XMP_StringPtr qualNS,
                           XMP_StringPtr qualName,
                           tStringObj *  fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief Compose the path expression to select an alternate item by language.
    ///
    /// The path syntax allows two forms of "content addressing" that may be used to select an item
    /// in an array of alternatives. The form used in \c ComposeLangSelector lets you select an
    /// item in an alt-text array based on the value of its <tt>xml:lang</tt> qualifier. The other
    /// form of content addressing is shown in \c ComposeFieldSelector.
    ///
    /// \note \c ComposeLangSelector does not supplant \c SetLocalizedText or \c GetLocalizedText.
    /// They should generally be used, as they provide extra logic to choose the appropriate
    /// language and maintain consistency with the 'x-default' value. \c ComposeLangSelector gives
    /// you an path expression that is explicitly and only for the language given in the
    /// \c langName parameter.
    ///
    /// \param schemaNS The namespace URI for the array. Must not be null or the empty string.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// \param langName The RFC 3066 code for the desired language.
    ///
    /// \param fullPath A pointer to the string that will be assigned the composed path. This will
    /// be of the form <tt>ns:arrayName[\@xml:lang='langName']</tt>,
    /// where "ns" is the prefix for \c schemaNS.

    static void
    ComposeLangSelector ( XMP_StringPtr schemaNS,
                          XMP_StringPtr arrayName,
                          XMP_StringPtr langName,
                          tStringObj *  fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c ComposeLangSelector is a simple overload in the template that calls
    /// the above form passing <tt>langName.c_str()</tt>.

    static void
    ComposeLangSelector ( XMP_StringPtr      schemaNS,
                          XMP_StringPtr      arrayName,
                          const tStringObj & langName,
                          tStringObj *       fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief Compose the path expression to select an alternate item by a field's value.
    ///
    /// The path syntax allows two forms of "content addressing" that may be used to select an item
    /// in an array of alternatives. The form used in \c ComposeFieldSelector lets you select an
    /// item in an array of structs based on the value of one of the fields in the structs. The
    /// other form of content addressing is shown in \c ComposeLangSelector.
    ///
    /// For example, consider a simple struct that has two fields, the name of a city and the URI
    /// of an FTP site in that city. Use this to create an array of download alternatives. You can
    /// show the user a popup built from the values of the city fields. You can then get the
    /// corresponding URI as follows:
    ///
    /// \verbatim
    ///  ComposeFieldSelector ( schemaNS, "Downloads", fieldNS, "City", chosenCity, &path );
    ///  exists = GetStructField ( schemaNS, path, fieldNS, "URI", &uri );
    /// \endverbatim
    ///
    /// \param schemaNS The namespace URI for the array. Must not be null or the empty string.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string.
    ///
    /// \param fieldNS The namespace URI for the field used as the selector. Must not be null or the
    /// empty string.
    ///
    /// \param fieldName The name of the field used as the selector. Must be a simple XML name, must
    /// not be null or the empty string. It must be the name of a field that is itself simple.
    ///
    /// \param fieldValue The desired value of the field.
    ///
    /// \param fullPath A pointer to the string that will be assigned the composed path. This will
    /// be of the form <tt>ns:arrayName[fNS:fieldName='fieldValue']</tt>, where "ns" is the prefix
    /// for \c schemaNS and "fNS" is the prefix for \c fieldNS.

    static void
    ComposeFieldSelector ( XMP_StringPtr schemaNS,
                           XMP_StringPtr arrayName,
                           XMP_StringPtr fieldNS,
                           XMP_StringPtr fieldName,
                           XMP_StringPtr fieldValue,
                           tStringObj *  fullPath );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of ComposeFieldSelector is a simple overload in the template that calls the
    /// above form passing <tt>fieldValue.c_str()</tt>.

    static void
    ComposeFieldSelector ( XMP_StringPtr      schemaNS,
                           XMP_StringPtr      arrayName,
                           XMP_StringPtr      fieldNS,
                           XMP_StringPtr      fieldName,
                           const tStringObj & fieldValue,
                           tStringObj *       fullPath );

    /// @}

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name Binary-String conversion functions
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from Boolean to string.
    ///
    /// \param binValue The Boolean value to be converted.
    ///
    /// \param strValue The string representation of the Boolean. The values used are given by the
    /// macros \c kXMP_TrueStr and \c kXMP_FalseStr found in \c XMP_Const.h.

    static void
    ConvertFromBool ( bool         binValue,
                      tStringObj * strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from integer to string.
    ///
    /// \param binValue The integer value to be converted.
    ///
    /// \param format Optional C sprintf format for the conversion. Defaults to "%d".
    ///
    /// \param strValue The string representation of the integer.

    static void
    ConvertFromInt ( long          binValue,
                     XMP_StringPtr format,
                     tStringObj *  strValue );

    static void
    ConvertFromInt64 ( long long     binValue,
                       XMP_StringPtr format,
                       tStringObj *  strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from floating point to string.
    ///
    /// \param binValue The floating point value to be converted.
    ///
    /// \param format Optional C sprintf format for the conversion. Defaults to "%f".
    ///
    /// \param strValue The string representation of the floating point value.

    static void
    ConvertFromFloat ( double        binValue,
                       XMP_StringPtr format,
                       tStringObj *  strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from date/time to string.
    ///
    /// Format a date according to the ISO 8601 profile in http://www.w3.org/TR/NOTE-datetime:
    ///	YYYY
    ///	YYYY-MM
    ///	YYYY-MM-DD
    ///	YYYY-MM-DDThh:mmTZD
    ///	YYYY-MM-DDThh:mm:ssTZD
    ///	YYYY-MM-DDThh:mm:ss.sTZD
    ///
    ///	YYYY = four-digit year
    ///	MM	 = two-digit month (01=January, etc.)
    ///	DD	 = two-digit day of month (01 through 31)
    ///	hh	 = two digits of hour (00 through 23)
    ///	mm	 = two digits of minute (00 through 59)
    ///	ss	 = two digits of second (00 through 59)
    ///	s	 = one or more digits representing a decimal fraction of a second
    ///	TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
    ///
    /// \note ISO 8601 does not seem to allow years less than 1000 or greater than 9999. We allow
    /// any year, even negative ones. The year is formatted as "%.4d".
    ///
    /// \note As a compatibility "tactic" (OK, a hack), so-called time-only input is allowed where
    /// the year, month, and day are all zero. This is output as "0000-00-00...".
    ///
    /// \param binValue The \c XMP_DateTime value to be converted.
    ///
    /// \param strValue The ISO 8601 string representation of the date/time.

    static void
    ConvertFromDate ( const XMP_DateTime & binValue,
                      tStringObj *         strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from string to Boolean.
    ///
    /// \param strValue The string representation of the Boolean.
    ///
    /// \result The appropriate C++ bool value for the string. The preferred strings are
    /// \c kXMP_TrueStr and \c kXMP_FalseStr. If these do not match, a case insensitive comparison is
    /// tried, then simply 't' or 'f', and finally non-zero and zero integer representations.

    static bool
    ConvertToBool ( XMP_StringPtr strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c ConvertToBool is a simple overload in the template that calls the
    /// above form passing <tt>strValue.c_str()</tt>.

    static bool
    ConvertToBool ( const tStringObj & strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from string to integer.
    ///
    /// \param strValue The string representation of the integer.
    ///
    /// \result The integer value as a C long.

    static long
    ConvertToInt ( XMP_StringPtr strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c ConvertToInt is a simple overload in the template that calls the above
    /// form passing <tt>strValue.c_str()</tt>.

    static long
    ConvertToInt ( const tStringObj & strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from string to 64 bit integer.
    ///
    /// \param strValue The string representation of the integer.
    ///
    /// \result The integer value as a C long long.

    static long long
    ConvertToInt64 ( XMP_StringPtr strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of ConvertToInt64 is a simple overload in the template that calls the above
    /// form passing <tt>strValue.c_str()</tt>.

    static long long
    ConvertToInt64 ( const tStringObj & strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from string to floating point.
    ///
    /// \param strValue The string representation of the floating point value.
    ///
    /// \result The floating point value.

    static double
    ConvertToFloat ( XMP_StringPtr strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c ConvertToFloat is a simple overload in the template that calls the
    /// above form passing <tt>strValue.c_str()</tt>.

    static double
    ConvertToFloat ( const tStringObj & strValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from string to date/time.
    ///
    /// Parse a date according to the ISO 8601 profile in http://www.w3.org/TR/NOTE-datetime:
    ///	YYYY
    ///	YYYY-MM
    ///	YYYY-MM-DD
    ///	YYYY-MM-DDThh:mmTZD
    ///	YYYY-MM-DDThh:mm:ssTZD
    ///	YYYY-MM-DDThh:mm:ss.sTZD
    ///
    ///	YYYY = four-digit year
    ///	MM	 = two-digit month (01=January, etc.)
    ///	DD	 = two-digit day of month (01 through 31)
    ///	hh	 = two digits of hour (00 through 23)
    ///	mm	 = two digits of minute (00 through 59)
    ///	ss	 = two digits of second (00 through 59)
    ///	s	 = one or more digits representing a decimal fraction of a second
    ///	TZD	 = time zone designator (Z or +hh:mm or -hh:mm)
    ///
    /// \note ISO 8601 does not seem to allow years less than 1000 or greater than 9999. We allow
    /// any year, even negative ones. The year is assumed to be formatted as "%.4d".
    ///
    /// \note As compatibility "tactics" (OK, hacks), a missing date portion or missing TZD are
    /// tolerated. A missing date value may begin with "Thh:" or "hh:"; the year, month, and day are
    /// all set to zero in the XMP_DateTime value. If TZD is missing, assume the time is in local time.
    ///
    /// \param strValue The ISO 8601 string representation of the date/time.
    ///
    /// \param binValue A pointer to the \c XMP_DateTime variable to be assigned the date/time components.

    static void
    ConvertToDate ( XMP_StringPtr  strValue,
                    XMP_DateTime * binValue );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c ConvertToDate is a simple overload in the template that calls the above
    /// form passing s<tt>strValue.c_str()</tt>.

    static void
    ConvertToDate ( const tStringObj & strValue,
                    XMP_DateTime *     binValue );

    /// @}

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name Date/Time functions
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain the current date and time.
    ///
    /// \param time	A pointer to the \c XMP_DateTime variable to be assigned the current date
    /// and time. The returned time is UTC, properly adjusted for the local time zone. The
    /// resolution of the time is not guaranteed to be finer than seconds.

    static void
    CurrentDateTime ( XMP_DateTime * time );

    //  --------------------------------------------------------------------------------------------
    /// \brief Set the local time zone.
    ///
    /// \param time	A pointer to the \c XMP_DateTime variable containing the value to be modified. Any
    /// existing time zone value is replaced, the other date/time fields are not adjusted in any way.

    static void
    SetTimeZone ( XMP_DateTime * time );

    //  --------------------------------------------------------------------------------------------
    /// \brief Make sure a time is UTC.
    ///
    /// \param time	A pointer to the \c XMP_DateTime variable containing the time to be modified. If
    /// the time zone is not UTC, the time is adjusted and the time zone set to be UTC.

    static void
    ConvertToUTCTime ( XMP_DateTime * time );

    //  --------------------------------------------------------------------------------------------
    /// \brief Make sure a time is local.
    ///
    /// \param time	A pointer to the \c XMP_DateTime variable containing the time to be modified. If
    /// the time zone is not the local zone, the time is adjusted and the time zone set to be local.

    static void
    ConvertToLocalTime ( XMP_DateTime * time );

    //  --------------------------------------------------------------------------------------------
    /// \brief Compare the order of two date/time values.
    ///
    /// \param left The "lefthand" date/time.
    ///
    /// \param right The "righthand" date/time.
    ///
    /// \result
    /// \li -1 if left is before right
    /// \li 0 if left matches right
    /// \li +1 if left is after right

    static int
    CompareDateTime ( const XMP_DateTime & left,
                      const XMP_DateTime & right );

    /// @}

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name Base 64 Encoding and Decoding
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief Convert from raw data to Base64 encoded string.
    ///
    /// \param rawStr The pointer to raw data to be converted.
    ///
    /// \param rawLen The length of raw data to be converted.
    ///
    /// \param encodedStr The XMP object to contain the encoded string.

    static void
    EncodeToBase64 ( XMP_StringPtr rawStr,
                     XMP_StringLen rawLen,
                     tStringObj *  encodedStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c EncodeToBase64 is a simple overload in the template that calls the
    /// above form passing <tt>rawStr.c_str()</tt>, and <tt>rawStr.size()</tt>.

    static void
    EncodeToBase64 ( const tStringObj & rawStr,
                     tStringObj *       encodedStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief Decode from Base64 encoded string to raw data.
    ///
    /// \param encodedStr The pointer to encoded data to be converted.
    ///
    /// \param encodedLen The length of encoded datavto be converted.
    ///
    /// \param rawStr The XMP object to contain the decoded string.

    static void
    DecodeFromBase64 ( XMP_StringPtr encodedStr,
                       XMP_StringLen encodedLen,
                       tStringObj *  rawStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c DecodeFromBase64 is a simple overload in the template that calls the
    /// above form passing <tt>encodedStr.c_str()</tt>, and <tt>encodedStr.size()</tt>.

    static void
    DecodeFromBase64 ( const tStringObj & encodedStr,
                       tStringObj *       rawStr );

    /// @}

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name JPEG file handling
    /// @{
    /// These functions support the partitioning of XMP in JPEG files into standard and extended 
    /// portions in order to work around the 64KB size limit of JPEG marker segments.

    //  --------------------------------------------------------------------------------------------
    /// \brief Create XMP serializations appropriate for a JPEG file. The standard XMP in a JPEG
    /// file is limited to about 65500 bytes. \c PackageForJPEG attempts to fit the serialization
    /// within that limit. If necessary it will partition the XMP into 2 serializations.
    ///
    /// \param xmpObj The XMP for the JPEG file.
    ///
    /// \param standardXMP The full standard XMP packet.
    ///
    /// \param extendedXMP The serialized extended XMP, empty if not needed.
    ///
    /// \param extendedDigest An MD5 digest of the serialized extended XMP, empty if not needed.

    static void
    PackageForJPEG ( const TXMPMeta<tStringObj> & xmpObj,
                     tStringObj * standardXMP,
                     tStringObj * extendedXMP,
                     tStringObj * extendedDigest );

    //  --------------------------------------------------------------------------------------------
    /// \brief Put the extended XMP properties back into the full XMP.
    ///
    /// \param fullXMP The full XMP, presumed to be initialized from the standard XMP packet.
    ///
    /// \param extendedXMP The properties that were partitioned into the extended XMP.

    static void
    MergeFromJPEG ( TXMPMeta<tStringObj> * fullXMP,
                    const TXMPMeta<tStringObj> & extendedXMP );

    /// @}

    // =============================================================================================
    // =============================================================================================

    //  ============================================================================================
    /// \name UI helper functions
    /// @{
    /// These functions are mainly of interest in implementing a user interface for editing XMP.

    //  --------------------------------------------------------------------------------------------
    /// \brief Create a single edit string from an array of strings.
    ///
    /// TBD - needs more description
    ///
    /// \param xmpObj The XMP object containing the array to be catenated.
    ///
    /// \param schemaNS The schema namespace URI for the array. Must not be null or the empty string.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Each item in the array must be a simple string value.
    ///
    /// \param separator The string to be used to separate the items in the catenated string.
    /// Defaults to "; ", ASCII semicolon and space (U+003B, U+0020).
    ///
    /// \param quotes The characters to be used as quotes around array items that contain a separator.
    /// Defaults to '"', ASCII quote (U+0022).
    ///
    /// \param options Option flags to control the catenation.
    ///
    /// \param catedStr A pointer to the string to be assigned the catenated array items.

    static void
    CatenateArrayItems ( const TXMPMeta<tStringObj> & xmpObj,
                         XMP_StringPtr  schemaNS,
                         XMP_StringPtr  arrayName,
                         XMP_StringPtr  separator,
                         XMP_StringPtr  quotes,
                         XMP_OptionBits options,
                         tStringObj *   catedStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief Separate a single edit string into an array of strings.
    ///
    /// TBD - needs more description
    ///
    /// \param xmpObj The XMP object containing the array to be updated.
    ///
    /// \param schemaNS The schema namespace URI for the array. Must not be null or the empty string.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Each item in the array must be a simple string value.
    ///
    /// \param options Option flags to control the separation.
    ///
    /// \param catedStr The string to be separated into the array items.

    static void
    SeparateArrayItems ( TXMPMeta<tStringObj> * xmpObj,
                         XMP_StringPtr  schemaNS,
                         XMP_StringPtr  arrayName,
                         XMP_OptionBits options,
                         XMP_StringPtr  catedStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SeparateArrayItems is a simple overload in the template that calls
    /// the aboveform passing <tt>catedStr.c_str()</tt>.

    static void
    SeparateArrayItems ( TXMPMeta<tStringObj> * xmpObj,
                         XMP_StringPtr      schemaNS,
                         XMP_StringPtr      arrayName,
                         XMP_OptionBits     options,
                         const tStringObj & catedStr );

    //  --------------------------------------------------------------------------------------------
    /// \brief Remove multiple properties from an XMP object.
    ///
    /// \c RemoveProperties was created to support the File Info dialog's Delete button, and has
    /// been been generalized somewhat from those specific needs. It operates in one of three main
    /// modes depending on the schemaNS and propName parameters:
    ///
    /// \li Non-empty \c schemaNS and \c propName - The named property is removed if it is an
    /// external property, or if the \c kXMPUtil_DoAllProperties option is passed. It does not
    /// matter whether the named property is an actual property or an alias.
    ///
    /// \li Non-empty \c schemaNS and empty \c propName - The all external properties in the named
    /// schema are removed. Internal properties are also removed if the \c kXMPUtil_DoAllProperties
    /// option is passed. In addition, aliases from the named schema will be removed if the \c
    /// kXMPUtil_IncludeAliases option is passed.
    ///
    /// \li Empty \c schemaNS and empty \c propName - All external properties in all schema are
    /// removed. Internal properties are also removed if the \c kXMPUtil_DoAllProperties option is
    /// passed. Aliases are implicitly handled because the associated actuals are.
    ///
    /// It is an error to pass and empty schemaNS and non-empty propName.
    ///
    /// \param xmpObj The XMP object containing the properties to be removed.
    ///
    /// \param schemaNS Optional schema namespace URI for the properties to be removed.
    ///
    /// \param propName Optional path expression for the property to be removed.
    ///
    /// \param options Option flags to control the deletion. The defined flags are:
    /// \li \c kXMPUtil_DoAllProperties - Do internal properties in addition to external properties.
    /// \li \c kXMPUtil_IncludeAliases - Include aliases in the "named schema" case above.

    static void
    RemoveProperties ( TXMPMeta<tStringObj> * xmpObj,
                       XMP_StringPtr  schemaNS = 0,
                       XMP_StringPtr  propName = 0,
                       XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief Append properties from one XMP object to another.
    ///
    /// \c AppendProperties was created to support the File Info dialog's Append button, and has
    /// been been generalized somewhat from those specific needs. It appends information from one
    /// XMP object (\c source) to another (\c dest). The default operation is to append only external
    /// properties that do not already exist in the destination. The kXMPUtil_DoAllProperties option
    /// can be used to operate on all properties, external and internal. The kXMPUtil_ReplaceOldValues
    /// option can be used to replace the values of existing properties. The notion of external
    /// versus internal applies only to top level properties. The keep-or-replace-old notion applies
    /// within structs and arrays as described below.
    ///
    /// If kXMPUtil_ReplaceOldValues is passed then the processing is restricted to the top level
    /// properties. The processed properties from the source (according to kXMPUtil_DoAllProperties)
    /// are propagated to the destination, replacing any existing values. Properties in the destination
    /// that are not in the source are left alone.
    ///
    /// If kXMPUtil_ReplaceOldValues is not passed then the processing is more complicated. Top level
    /// properties are added to the destination if they do not already exist. If they do exist but
    /// differ in form (simple/struct/array) then the destination is left alone. If the forms match,
    /// simple properties are left unchanged while structs and arrays are merged.
    ///
    /// If kXMPUtil_DeleteEmptyValues is passed then an empty value in the source XMP causes the
    /// corresponding Dest XMP property to be deleted. The default is to treat empty values the same
    /// as non-empty values. An empty value is any of a simple empty string, an array with no items,
    /// or a struct with no fields. Qualifiers are ignored.
    ///
    /// The detailed behavior is defined by the following pseudo-code:
    /// \verbatim
    ///   AppendProperties ( sourceXMP, destXMP, options ):
    ///      doAll = options & kXMPUtil_DoAllProperties
    ///      replaceOld = options & kXMPUtil_ReplaceOldValues
    ///      deleteEmpty = options & kXMPUtil_DeleteEmptyValues
    ///      for all source schema (top level namespaces):
    ///         for all top level properties in sourceSchema:
    ///            if doAll or prop is external:
    ///               AppendSubtree ( sourceNode, destSchema, replaceOld, deleteEmpty )
    ///
    ///   AppendSubtree ( sourceNode, destParent, replaceOld, deleteEmpty ):
    ///      if deleteEmpty and source value is empty:
    ///         delete the corresponding child from destParent
    ///      else if sourceNode not in destParent (by name):
    ///         copy sourceNode's subtree to destParent
    ///      else if replaceOld:
    ///         delete subtree from destParent
    ///         copy sourceNode's subtree to destParent
    ///      else:
    ///         // Already exists in dest and not replacing, merge structs and arrays
    ///         if sourceNode and destNode forms differ:
    ///            return, leave the destNode alone
    ///         else if form is a struct:
    ///            for each field in sourceNode:
    ///               AppendSubtree ( sourceNode.field, destNode, replaceOld )
    ///         else if form is an alt-text array:
    ///            copy new items by xml:lang value into the destination
    ///         else if form is an array:
    ///            copy new items by value into the destination, ignoring order and duplicates
    /// \endverbatim
    ///
    /// \note \c AppendProperties can be expensive if replaceOld is not passed and the XMP contains
    /// large arrays. The array item checking described above is n-squared. Each source item is
    /// checked to see if it already exists in the destination, without regard to order or duplicates.
    /// Simple items are compared by value and xml:lang qualifier, other qualifiers are ignored.
    /// Structs are recursively compared by field names, without regard to field order. Arrays are
    /// compared by recursively comparing all items.
    ///
    /// \param source The source XMP object.
    ///
    /// \param dest The destination XMP object.
    ///
    /// \param options Option flags to control the copying.
    /// \li \c kXMPUtil_DoAllProperties - Do internal properties in addition to external properties.
    /// \li \c kXMPUtil_ReplaceOldValues - Replace the values of existing properties.
    /// \li \c kXMPUtil_DeleteEmptyValues - Delete properties if the new value is empty.

    static void
    AppendProperties ( const TXMPMeta<tStringObj> & source,
                       TXMPMeta<tStringObj> *       dest,
                       XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief Replicate a subtree from one XMP object into another, possibly at a different location.
    ///
    /// TBD - needs more description
    ///
    /// \param source The source XMP object.
    ///
    /// \param dest The destination XMP object.
    ///
    /// \param sourceNS The schema namespace URI for the source subtree.
    ///
    /// \param sourceRoot The root location for the source subtree. May be a general path expression,
    /// must not be null or the empty string.
    ///
    /// \param destNS The schema namespace URI for the destination. Defaults to the source namespace.
    ///
    /// \param destRoot The root location for the destination. May be a general path expression.
    /// Defaults to the source location.
    ///
    /// \param options Option flags to control the separation.

    static void
    DuplicateSubtree ( const TXMPMeta<tStringObj> & source,
                       TXMPMeta<tStringObj> *       dest,
                       XMP_StringPtr  sourceNS,
                       XMP_StringPtr  sourceRoot,
                       XMP_StringPtr  destNS = 0,
                       XMP_StringPtr  destRoot = 0,
                       XMP_OptionBits options = 0 );


    /// @}

    // =============================================================================================

};  // class TXMPUtils

// =================================================================================================

#endif // __TXMPUtils_hpp__
