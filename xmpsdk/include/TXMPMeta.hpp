#ifndef __TXMPMeta_hpp__
#define __TXMPMeta_hpp__    1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMPSDK.hpp"
#endif

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

//  ================================================================================================
/// \file TXMPMeta.hpp
/// \brief Template class for the XMP Toolkit core services.
///
/// TXMPMeta is the template class providing the core services of the XMP Toolkit. It should be
/// instantiated with a string class such as <tt>std::string</tt>. Please read the general toolkit
/// usage notes for information about the overall architecture of the XMP API.
//  ================================================================================================

//  ================================================================================================
/// \class TXMPMeta TXMPMeta.hpp
/// \brief Template class for the XMP Toolkit core services.
///
/// \c TXMPMeta is the template class providing the core services of the XMP Toolkit. It should be
/// instantiated with a string class such as std::string. Please read the general toolkit usage notes
/// for information about the overall architecture of the XMP API.
///
/// This template wraps a string object class around the raw XMP API. This provides two significant
/// benefits, output strings are automatically copied and access is fully thread safe. The
/// umbrella header, \c XMPSDK.hpp, provides an \c SXMPMeta typedef for the instantiated template. String
/// objects are only necessary for output strings. Input string are literals and passed as typical
/// C <tt>const char *</tt>.
///
/// The template parameter, class \c TtStringObj, is described in the XMPSDK.hpp umbrella header.
///
/// <b>Be aware that the \c TXMPMeta class is a normal C++ template, it is instantiated and local to
/// each client executable. As are the other TXMP* classes. Different clients might not even use the
/// same string type to instantiate \c TXMPMeta.</b>
///
/// Because of this you should not pass \c SXMPMeta objects, or pointers to \c SXMPMeta objects,
/// across DLL boundaries. There is a safe internal reference that you can pass, then construct a
/// local object on the callee side. This construction does not create a cloned XMP tree, it is the
/// same underlying XMP object safely wrapped in each client's \c SXMPMeta object.
///
/// Use GetInternalRef and the associated constructor like this:
/// \code
///  --- The callee's header contains:
///  CalleeMethod ( XMPMetaRef xmpRef );
///
///  --- The caller's code contains:
///  SXMPMeta callerXMP;
///  CalleeMethod ( callerXMP.GetInternalRef() );
///
///  --- The callee's code contains:
///  SXMPMeta calleeXMP ( xmpRef );
/// \endcode
//  ================================================================================================

template <class tStringObj> class TXMPIterator;
template <class tStringObj> class TXMPUtils;

// -------------------------------------------------------------------------------------------------

template <class tStringObj>
class TXMPMeta {

public:

    // =============================================================================================
    // Initialization and termination
    // ==============================

    //  --------------------------------------------------------------------------------------------
    /// \name Initialization and termination
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain version information.

    static void
    GetVersionInfo ( XMP_VersionInfo * info );

    //  --------------------------------------------------------------------------------------------
    /// \brief Initialize the XMP Toolkit.
    ///
    /// The XMP Toolkit may be explicitly initialized before use. The allocate/delete parameters must
    /// be either both null (0), or both non-null.

    static bool
    Initialize();
    //  --------------------------------------------------------------------------------------------
    /// \brief Terminate the XMP Toolkit.

    static void
    Terminate();

    /// @}

    // =============================================================================================
    // Constuctors and destructor
    // =========================

    //  --------------------------------------------------------------------------------------------
    /// \name Constructors and destructor
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief Default constructor, creates an empty object.
    ///
    /// The default constructor creates a new empty \c TXMPMeta object.

    TXMPMeta();

    //  --------------------------------------------------------------------------------------------
    /// \brief Copy constructor, creates a client object refering to the same internal object.
    ///
    /// The copy constructor creates a new \c TXMPMeta object that refers to the same internal XMP object.

    TXMPMeta ( const TXMPMeta<tStringObj> & original );

    //  --------------------------------------------------------------------------------------------
    /// \brief Assignment operator, assigns the internal ref and increments the ref count.
    ///
    /// The assignment operator assigns the internal ref from the rhs object and increments the
    /// reference count on the underlying internal XMP object.

    void operator= ( const TXMPMeta<tStringObj> & rhs );

    //  --------------------------------------------------------------------------------------------
    /// \brief Reconstruct an XMP object from an internal ref.
    ///
    /// This constructor creates a new \c TXMPMeta object that refers to the underlying \c xmpRef,
    /// which was obtained from some other XMP object by the \c GetInternalRef method. This is used
    /// to safely pass XMP objects across DLL boundaries.

    TXMPMeta ( XMPMetaRef xmpRef );

    //  --------------------------------------------------------------------------------------------
    /// \brief Construct an object and parse one buffer of RDF into it.
    ///
    /// This constructor creates a new \c TXMPMeta object and populates it with metadata from a
    /// buffer containing serialized RDF. This buffer must be a complete RDF parse stream. Pass
    /// (0,0) to construct an empty \c TXMPMeta object. The result of an actual parse is identical
    /// to creating an empty object then calling <tt>TXMPMeta::ParseFromBuffer</tt>. The RDF must be
    /// complete. If you need to parse with multiple buffers, create an empty object and use
    /// \c TXMPMeta::ParseFromBuffer.
    ///
    /// \param buffer   A pointer to the buffer of RDF to be parsed. May be null if the length is 0.
    ///
    /// \param xmpSize  The length in bytes of the buffer.

    TXMPMeta ( XMP_StringPtr buffer,
               XMP_StringLen xmpSize );

    //  --------------------------------------------------------------------------------------------
    /// \brief Destructor, typical virtual destructor.

    virtual ~TXMPMeta() throw();

    /// @}

    // =============================================================================================
    // Global state functions
    // ======================

    //  --------------------------------------------------------------------------------------------
    /// \name Global option flags
    /// @{
    /// The global option flags affect the overall behavior of the XMP Toolkit. The available options
    /// are declared in <tt>XMP_Const.h</tt>. <b>(There are none at present.)</b>

    /// \brief GetGlobalOptions returns the set of global option flags.

    static XMP_OptionBits
    GetGlobalOptions();

    /// \brief \c SetGlobalOptions updates the set of global option flags. The entire set is
    /// replaced with the new values. If only one flag is to be modified, use \c GetGlobalOptions
    /// to obtain the current set, modify the desired flag, then use \c SetGlobalOptions.
    ///
    /// \note There are no options to set yet.

    static void
    SetGlobalOptions ( XMP_OptionBits options );

    /// @}

    //  --------------------------------------------------------------------------------------------
    /// \name Internal data structure dump utilities
    /// @{
    /// These are debugging utilities that dump internal data structures. The output callback is
    /// described in <tt>XMP_Const.h</tt>.

    /// \brief \c DumpNamespaces dumps the list of registered namespace URIs and prefixes.

    static XMP_Status
    DumpNamespaces ( XMP_TextOutputProc outProc,
                     void *             refCon );

    /// \brief \c DumpAliases dumps the list of registered aliases and corresponding actuals.

    static XMP_Status
    DumpAliases ( XMP_TextOutputProc outProc,
                  void *             refCon );

    /// @}

    //  --------------------------------------------------------------------------------------------
    /// \name Namespace Functions
    /// @{
    /// Namespaces must be registered before use in namespace URI parameters or path expressions.
    /// Within the XMP Toolkit the registered namespace URIs and prefixes must be unique. Additional
    /// namespaces encountered when parsing RDF are automatically registered.
    ///
    /// The namespace URI should always end in an XML name separator such as '/' or '#'. This is
    /// because some forms of RDF shorthand catenate a namespace URI with an element name to form a
    /// new URI.

    //  --------------------------------------------------------------------------------------------
    /// \brief Register a namespace URI with a suggested prefix.
    ///
    /// It is not an error if the URI is already registered, no matter what the prefix is. If the
    /// URI is not registered but the suggested prefix is in use, a unique prefix is created from
    /// the suggested one. The actual registeed prefix is always returned. The function result
    /// tells if the registered prefix is the suggested one.
    ///
    /// \param namespaceURI The URI for the namespace. Must be a valid XML URI.
    ///
    /// \param suggestedPrefix The suggested prefix to be used if the URI is not yet registered.
    /// Must be a valid XML name.
    ///
    /// \param registeredPrefix Returns the prefix actually registered for this URI.
    ///
    /// \result Returns true if the registered prefix matches the suggested prefix.
    ///
    /// \note No checking is presently done on either the URI or the prefix.

    static bool
    RegisterNamespace ( XMP_StringPtr namespaceURI,
                        XMP_StringPtr suggestedPrefix,
                        tStringObj *  registeredPrefix );

    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain the prefix for a registered namespace URI.
    ///
    /// It is not an error if the namespace URI is not registered. The output \c namespacePrefix
    /// string is not modified if the namespace URI is not registered.
    ///
    /// \param namespaceURI The URI for the namespace. Must not be null or the empty string.
    ///
    /// \param namespacePrefix Returns the prefix registered for this URI, with a terminating ':'.
    ///
    /// \result Returns true if the namespace URI is registered.

    static bool
    GetNamespacePrefix ( XMP_StringPtr namespaceURI,
                         tStringObj *  namespacePrefix );

    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain the URI for a registered namespace prefix.
    ///
    /// It is not an error if the namespace prefix is not registered. The output \c namespaceURI
    /// string is not modified if the namespace prefix is not registered.
    ///
    /// \param namespacePrefix The prefix for the namespace. Must not be null or the empty string.
    ///
    /// \param namespaceURI Returns the URI registered for this prefix.
    ///
    /// \result Returns true if the namespace prefix is registered.

    static bool
    GetNamespaceURI ( XMP_StringPtr namespacePrefix,
                      tStringObj *  namespaceURI );

    //  --------------------------------------------------------------------------------------------
    /// \brief Delete a namespace from the registry.
    ///
    /// Does nothing if the URI is not registered, or if the \c namespaceURI parameter is null or the
    /// empty string.
    ///
    /// \param namespaceURI The URI for the namespace.
    ///
    /// \note <b>Not yet implemented.</b>

    static void
    DeleteNamespace ( XMP_StringPtr namespaceURI );

    /// @}

    //  --------------------------------------------------------------------------------------------
    /// \name Alias Functions
    /// @{
    /// Aliases in XMP serve the same purpose as Windows file shortcuts, Macintosh file aliases, or
    /// UNIX file symbolic links. The aliases are simply multiple names for the same property. One
    /// distinction of XMP aliases is that they are ordered, there is an alias name pointing to an
    /// actual name. The primary significance of the actual name is that it is the preferred name
    /// for output, generally the most widely recognized name.
    ///
    /// The names that can be aliased in XMP are restricted. The alias must be a top level property
    /// name, not a field within a structure or an element within an array. The actual may be a top
    /// level property name, the first element within a top level array, or the default element in
    /// an alt-text array. This does not mean the alias can only be a simple property. It is OK to
    /// alias a top level structure or array to an identical top level structure or array, or to the
    /// first item of an array of structures.

    //  --------------------------------------------------------------------------------------------
    /// \brief Associates an alias name with an actual name.
    ///
    /// Define a alias mapping from one namespace/property to another. Both property names must be
    /// simple names. An alias can be a direct mapping, where the alias and actual have the same
    /// data type. It is also possible to map a simple alias to an item in an array. This can either
    /// be to the first item in the array, or to the 'x-default' item in an alt-text array. Multiple
    /// alias names may map to the same actual, as long as the forms match. It is a no-op to
    /// reregister an alias in an identical fashion.
    ///
    /// \param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// \param aliasProp The name of the alias. Must be a simple name, not null or the empty string
    /// and not a general path expression.
    ///
    /// \param actualNS The namespace URI for the actual. Must not be null or the empty string.
    ///
    /// \param actualProp The name of the actual. Must be a simple name, not null or the empty string
    /// and not a general path expression.
    ///
    /// \param arrayForm Provides the array form for simple aliases to an array item. This is needed
    /// to know what kind of array to create if set for the first time via the simple alias. Pass
    /// \c kXMP_NoOptions, the default value, for all direct aliases regardless of whether the actual
    /// data type is an array or not.
    ///
    /// Constants for the arrayForm parameter:
    ///
    /// \li \c kXMP_NoOptions - This is a direct mapping. The actual data type does not matter.
    /// \li \c kXMP_PropValueIsArray - The actual is an unordered array, the alias is to the first
    /// element of the array.
    /// \li \c kXMP_PropArrayIsOrdered - The actual is an ordered array, the alias is to the first
    /// element of the array.
    /// \li \c kXMP_PropArrayIsAlternate - The actual is an alternate array, the alias is to the first
    /// element of the array.
    /// \li \c kXMP_PropArrayIsAltText - The actual is an alternate text array, the alias is to the
    /// 'x-default' element of the array.

    static void
    RegisterAlias ( XMP_StringPtr  aliasNS,
                    XMP_StringPtr  aliasProp,
                    XMP_StringPtr  actualNS,
                    XMP_StringPtr  actualProp,
                    XMP_OptionBits arrayForm = kXMP_NoOptions );

    //  --------------------------------------------------------------------------------------------
    /// \brief Determines if a name is an alias, and what it is aliased to.
    ///
    /// \param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// \param aliasProp The name of the alias. May be an arbitrary path expression path, must not
    /// null or the empty string.
    ///
    /// \param actualNS Untouched if <tt>aliasNS:aliasProp</tt> is not an alias. Otherwise returns
    /// the namespace URI for the actual. May be null if the namespace URI is not wanted.
    ///
    /// \param actualProp Untouched if <tt>aliasNS:aliasProp</tt> is not an alias. Otherwise
    /// returns the path of the actual. May be null if the actual's path is not wanted.
    ///
    /// \param arrayForm Untouched if <tt>aliasNS:aliasProp</tt> is not an alias. Otherwise returns
    /// the form of the actual. This is 0 (\c kXMP_NoOptions) if the alias and actual forms match,
    /// otherwise it is the options passed to <tt>TXMPMeta::RegisterAlias</tt>. May be null if the
    /// actual's form is not wanted.
    ///
    /// \result Returns true if the input is an alias.
    ///
    /// \note The client output strings are not written until return, so a call like the following
    /// may be used to "reduce" a path to the base form:
    /// \code
    ///  isAlias = SXMPMeta::ResolveAlias ( ns.c_str(), path.c_str(), &ns, &path, 0 );
    /// \endcode

    static bool
    ResolveAlias ( XMP_StringPtr    aliasNS,
                   XMP_StringPtr    aliasProp,
                   tStringObj *     actualNS,
                   tStringObj *     actualProp,
                   XMP_OptionBits * arrayForm );

    //  --------------------------------------------------------------------------------------------
    /// \brief Delete an alias.
    ///
    /// This only deletes the registration of the alias, it does not delete the actual property. It
    /// does delete any view of the property through the alias name. It is OK to attempt to delete
    /// an alias that does not exist, that is if the alias name is not registered as an alias.
    ///
    /// \param aliasNS The namespace URI for the alias. Must not be null or the empty string.
    ///
    /// \param aliasProp The name of the alias. Must be a simple name, not null or the empty string
    /// and not a general path expression.

    static void
    DeleteAlias ( XMP_StringPtr aliasNS,
                  XMP_StringPtr aliasProp );

    //  --------------------------------------------------------------------------------------------
    /// \brief Registers all of the built-in aliases for a standard namespace.
    ///
    /// The built-in aliases are documented in the XMP Specification. This registers the aliases in
    /// the given namespace, that is the aliases from this namespace to actuals in other namespaces.
    ///
    /// \param schemaNS The namespace URI for the aliases. Must not be null or the empty string.

    static void
    RegisterStandardAliases ( XMP_StringPtr schemaNS );

    /// @}

    // =============================================================================================
    // Basic property manipulation functions
    // =====================================

    // *** Should add discussion of schemaNS and propName prefix usage.

    //  --------------------------------------------------------------------------------------------
    /// \name Functions for getting property values
    /// @{
    /// The property value "getters" all take a property specification, The first two parameters
    /// are always the top level namespace URI (the "schema" namespace) and the basic name of the
    /// property being referenced. See the introductory discussion of path expression usage for
    /// more information.
    ///
    /// All of the functions return a Boolean result telling if the property exists, and if it does
    /// they also return option flags describing the property. If the property exists and has a
    /// value, the string value is also returned. The string is Unicode in UTF-8 encoding. Arrays
    /// and the non-leaf levels of structs do not have values. The possible option flags that
    /// describe properties are:
    ///
    /// \li \c kXMP_PropValueIsURI - The property value is a URI. It is serialized to RDF using the
    /// <tt>rdf:resource</tt> attribute. Not mandatory for URIs, but considered RDF-savvy.
    ///
    /// \li \c kXMP_PropHasQualifiers - The property has qualifiers. These could be an
    /// <tt>xml:lang</tt> attribute, an <tt>rdf:type</tt> property, or a general qualifier. See the
    /// introductory discussion of qualified properties for more information.
    ///
    /// \li \c kXMP_PropIsQualifier - This property is a qualifier for some other property. Note
    /// that if the qualifier itself has a structured value, this flag is only set for the top node
    /// of the qualifier's subtree. Qualifiers may have arbitrary structure, and may even have
    /// qualifiers.
    ///
    /// \li \c kXMP_PropHasLang - This property has an <tt>xml:lang</tt> qualifier.
    ///
    /// \li \c kXMP_PropHasType - This property has an <tt>rdf:type</tt> qualifier.
    ///
    /// \li \c kXMP_PropValueIsStruct - This property contains nested fields (models a C struct).
    ///
    /// \li \c kXMP_PropValueIsArray - This property is an array. By itself (no ...ArrayIs... flags),
    /// this indicates a general unordered array. It is serialized using an <tt>rdf:Bag</tt> container.
    ///
    /// \li \c kXMP_PropArrayIsOrdered - This property is an ordered array. Appears in conjunction
    /// with \c kXMP_PropValueIsArray. It is serialized using an <tt>rdf:Seq</tt> container.
    ///
    /// \li \c kXMP_PropArrayIsAlternate - This property is an alternative array. Appears in
    /// conjunction with \c kXMP_PropValueIsArray. It is serialized using an <tt>rdf:Alt</tt> container.
    ///
    /// \li \c kXMP_PropArrayIsAltText - This property is an alt-text array. Appears in conjunction
    /// with \c kXMP_PropArrayIsAlternate. It is serialized using an <tt>rdf:Alt</tt> container. Each
    /// array element is a simple property with an <tt>xml:lang</tt> attribute.
    ///
    /// \li \c kXMP_PropIsAlias - The given property name is an alias. This is only returned by
    /// \c GetProperty and then only if the property name is simple, not an path expression.
    ///
    /// \li \c kXMP_PropHasAliases - The given property name has aliases. This is only returned by
    /// \c GetProperty and then only if the property name is simple, not an path expression.
    ///
    /// \li \c kXMP_PropIsStable - The value of this property is not related to the document
    /// content.
    ///
    /// \li \c kXMP_PropIsDerived -  The value of this property is derived from the document
    /// content.
    ///
    /// \li \c kXMP_PropIsInternal - The value of this property is "owned" by the application, it
    /// should not generally be editable in a UI.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty is the simplest property getter, mainly for top level simple
    /// properties or after using the path composition functions in \c TXMPUtils.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. May be null or the empty string if the
    /// first component of the propName path contains a namespace prefix. The URI must be for a
    /// registered namespace.
    ///
    /// \param propName The name of the property. May be a general path expression, must not be
    /// null or the empty string. Using a namespace prefix on the first component is optional. If
    /// present without a \c schemaNS value then the prefix specifies the namespace. The prefix
    /// must be for a registered namespace. If both a \c schemaNS URI and \c propName prefix are
    /// present, they must be corresponding parts of a registered namespace.
    ///
    /// \param propValue A pointer to the string that is assigned the value of the property, if
    /// the property has a value. Arrays and non-leaf levels of structs do not have values. May be
    /// null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty ( XMP_StringPtr    schemaNS,
                  XMP_StringPtr    propName,
                  tStringObj *     propValue,
                  XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetArrayItem provides access to items within an array. The index is passed as an
    /// integer, you need not worry about the path string syntax for array items, convert a loop
    /// index to a string, etc.
    ///
    /// \result Returns true if the array item exists.
    ///
    /// \param schemaNS The namespace URI for the array. Has the same usage as in \c GetProperty.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GeProperty.
    ///
    /// \param itemIndex The index of the desired item. Arrays in XMP are indexed from 1. The constant
    /// \c kXMP_ArrayLastItem always refers to the last existing array item.
    ///
    /// \param itemValue A pointer to the string that is assigned the value of the array item, if
    /// the array item has a value. Arrays and non-leaf levels of structs do not have values. May be
    /// null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the array item. May be null if the flags are not wanted.

    bool
    GetArrayItem ( XMP_StringPtr    schemaNS,
                   XMP_StringPtr    arrayName,
                   XMP_Index        itemIndex,
                   tStringObj *     itemValue,
                   XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetStructField provides access to fields within a nested structure. The namespace
    /// for the field is passed as a URI, you need not worry about the path string syntax.
    ///
    /// The names of fields should be XML qualified names, that is within an XML namespace. The path
    /// syntax for a qualified name uses the namespace prefix. This is unreliable since the prefix
    /// is never guaranteed. The URI is the formal name, the prefix is just a local shorthand in a
    /// given sequence of XML text.
    ///
    /// \result Returns true if the field exists.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param structName The name of the struct. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param fieldNS The namespace URI for the field. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c structName parameter.
    ///
    /// \param fieldValue A pointer to the string that is assigned the value of the field, if
    /// the field has a value. Arrays and non-leaf levels of structs do not have values. May be
    /// null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the field. May be null if the flags are not wanted.

    bool
    GetStructField ( XMP_StringPtr    schemaNS,
                     XMP_StringPtr    structName,
                     XMP_StringPtr    fieldNS,
                     XMP_StringPtr    fieldName,
                     tStringObj *     fieldValue,
                     XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetQualifier provides access to a qualifier attached to a property. The namespace
    /// for the qualifier is passed as a URI, you need not worry about the path string syntax. In
    /// many regards qualifiers are like struct fields. See the introductory discussion of
    /// qualified properties for more information.
    ///
    /// The names of qualifiers should be XML qualified names, that is within an XML namespace. The
    /// path syntax for a qualified name uses the namespace prefix. This is unreliable since the
    /// prefix is never guaranteed. The URI is the formal name, the prefix is just a local shorthand
    /// in a given sequence of XML text.
    ///
    /// \note Qualifiers are only supported for simple leaf properties at this time.
    ///
    /// \result Returns true if the qualifier exists.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property to which the qualifier is attached. May be a general
    /// path expression, must not be null or the empty string. Has the same namespace prefix usage
    /// as in \c GetProperty.
    ///
    /// \param qualNS The namespace URI for the qualifier. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param qualName The name of the qualifier. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c propName parameter.
    ///
    /// \param qualValue A pointer to the string that is assigned the value of the qualifier, if
    /// the qualifier has a value. Arrays and non-leaf levels of structs do not have values. May be
    /// null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the qualifier. May be null if the flags are not wanted.

    bool
    GetQualifier ( XMP_StringPtr    schemaNS,
                   XMP_StringPtr    propName,
                   XMP_StringPtr    qualNS,
                   XMP_StringPtr    qualName,
                   tStringObj *     qualValue,
                   XMP_OptionBits * options ) const;

    /// @}

    // =============================================================================================

    //  --------------------------------------------------------------------------------------------
    /// \name Functions for setting property values
 	/// @{
    /// The property value "setters" all take a property specification, their differences are in
    /// the form of this. The first two parameters are always the top level namespace URI (the
    /// "schema" namespace) and the basic name of the property being referenced. See the
    /// introductory discussion of path expression usage for more information.
    ///
    /// All of the functions take a string value for the property and option flags describing the
    /// property. The value must be Unicode in UTF-8 encoding. Arrays and non-leaf levels of
    /// structs do not have values. Empty arrays and structs may be created using appropriate
    /// option flags. All levels of structs that is assigned implicitly are created if necessary.
    /// \c AppendArayItem implicitly creates the named array if necessary.
    ///
    /// The canonical form of these functions take the value as an \c XMP_StringPtr, a pointer to a
    /// null terminated string. (\c XMP_StringPtr is a typedef for <tt>const char *</tt>.) They
    /// also have overloaded forms that take a string object. These are implemented in the template
    /// instantiation as a call to the canonical form, using <tt>value.c_str()</tt> to obtain the
    /// \c XMP_StringPtr.
    ///
    /// The possible option flags are:
    ///
    /// \li \c kXMP_PropValueIsURI - The property value is a URI. It is serialized to RDF using the
    /// <tt>rdf:resource</tt> attribute. Not mandatory for URIs, but considered RDF-savvy.
    ///
    /// \li \c kXMP_PropValueIsStruct - This property contains nested fields (models a C struct).
    /// Not necessary, may be used to create an empty struct. A struct is implicitly created when
    /// first field is set.
    ///
    /// \li \c kXMP_PropValueIsArray - This property is an array. By itself (no ...ArrayIs...
    /// flags), this indicates a general unordered array. It is serialized using an
    /// <tt>rdf:Bag</tt> container.
    ///
    /// \li \c kXMP_PropArrayIsOrdered - This property is an ordered array. Implies \c
    /// kXMP_PropValueIsArray, may be used together. It is serialized using an <tt>rdf:Seq</tt>
    /// container.
    ///
    /// \li \c kXMP_PropArrayIsAlternate - This property is an alternative array. Implies \c
    /// kXMP_PropArrayIsOrdered, may be used together. It is serialized using an <tt>rdf:Alt</tt>
    /// container.
    ///
    /// \li \c kXMP_PropArrayIsAltText - This property is an alt-text array. Implies \c
    /// kXMP_PropArrayIsAlternate, may be used together. It is serialized using an <tt>rdf:Alt</tt>
    /// container. Each array element must be a simple property with an <tt>xml:lang</tt> attribute.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty is the simplest property setter, mainly for top level simple
    /// properties or after using the path composition functions in \c TXMPUtils.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the null terminated UTF-8 string that is the value of the
    /// property, if the property has a value. Arrays and non-leaf levels of structs do not have
    /// values. Must be null if the value is not relevant.
    ///
    /// \param options Option flags describing the property. See the earlier description.

    void
    SetProperty ( XMP_StringPtr  schemaNS,
                  XMP_StringPtr  propName,
                  XMP_StringPtr  propValue,
                  XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SetProperty is a simple overload in the template that calls the above
    /// form passing <tt>propValue.c_str()</tt>.

    void
    SetProperty ( XMP_StringPtr      schemaNS,
                  XMP_StringPtr      propName,
                  const tStringObj & propValue,
                  XMP_OptionBits     options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetArrayItem provides access to items within an array. The index is passed as an
    /// integer, you need not worry about the path string syntax for array items, convert a loop
    /// index to a string, etc. The array passed to \c SetArrayItem must already exist. See also
    /// \c AppendArrayItem.
    ///
    /// In normal usage the selected array item is modified. A new item is automatically
    /// appended if the index is the array size plus 1. A new item may be inserted before or after
    /// any item by using one of the following option flags:
    ///
    /// \li \c kXMP_InsertBeforeItem - Insert a new array item before the selected one.
    /// \li \c kXMP_InsertAfterItem -  Insert a new array item after the selected one.
    ///
    /// \param schemaNS The namespace URI for the array. Has the same usage as in \c GetProperty.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param itemIndex The index of the desired item. Arrays in XMP are indexed from 1. The
    /// constant \c kXMP_ArrayLastItem always refers to the last existing array item.
    ///
    /// \param itemValue A pointer to the null terminated UTF-8 string that is the value of the array
    /// item, if the array item has a value. Has the same usage as \c propValue in \c GetProperty.
    ///
    /// \param options Option flags describing the item. See the earlier description.

    void
    SetArrayItem ( XMP_StringPtr  schemaNS,
                   XMP_StringPtr  arrayName,
                   XMP_Index      itemIndex,
                   XMP_StringPtr  itemValue,
                   XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SetArrayItem is a simple overload in the template that calls the above
    /// form passing <tt>itemValue.c_str()</tt>.

    void
    SetArrayItem ( XMP_StringPtr      schemaNS,
                   XMP_StringPtr      arrayName,
                   XMP_Index          itemIndex,
                   const tStringObj & itemValue,
                   XMP_OptionBits     options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c AppendArrayItem simplifies construction of an array by not requiring that you
    /// pre-create an empty array. The array that is assigned is created automatically if it does
    /// not yet exist. Each call to \c AppendArrayItem appends an item to the array. The
    /// corresponding parameters have the same use as \c SetArrayItem. The \c arrayOptions
    /// parameter is used to specify what kind of array. If the array exists, it must have the
    /// specified form.
    ///
    /// \param schemaNS The namespace URI for the array. Has the same usage as in \c GetProperty.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propPath in \c GetProperty.
    ///
    /// \param arrayOptions Option flags describing the array form. The only valid bits are those
    /// that are part of \c kXMP_PropArrayFormMask: \c kXMP_PropValueIsArray, \c
    /// kXMP_PropArrayIsOrdered, \c kXMP_PropArrayIsAlternate, or \c kXMP_PropArrayIsAltText.
    ///
    /// \param itemValue A pointer to the null terminated UTF-8 string that is the value of the
    /// array item, if the array item has a value. Has the same usage as \c propValue in \c
    /// GetProperty.
    ///
    /// \param itemOptions Option flags describing the item. See the earlier description.

    void
    AppendArrayItem ( XMP_StringPtr  schemaNS,
                      XMP_StringPtr  arrayName,
                      XMP_OptionBits arrayOptions,
                      XMP_StringPtr  itemValue,
                      XMP_OptionBits itemOptions = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c AppendArrayItem is a simple overload in the template that calls the
    /// above form passing <tt>itemValue.c_str()</tt>.

    void
    AppendArrayItem ( XMP_StringPtr      schemaNS,
                      XMP_StringPtr      arrayName,
                      XMP_OptionBits     arrayOptions,
                      const tStringObj & itemValue,
                      XMP_OptionBits     itemOptions = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetStructField provides access to fields within a nested structure. The namespace
    /// for the field is passed as a URI, you need not worry about the path string syntax.
    ///
    /// The names of fields should be XML qualified names, that is within an XML namespace. The
    /// path syntax for a qualified name uses the namespace prefix, which is unreliable because
    /// the prefix is never guaranteed. The URI is the formal name, the prefix is just a local
    /// shorthand in a given sequence of XML text.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param structName The name of the struct. May be a general path expression, must not be
    /// null or the empty string. Has the same namespace prefix usage as \c propName in \c
    /// GetProperty.
    ///
    /// \param fieldNS The namespace URI for the field. Has the same URI and prefix usage as the \c
    /// schemaNS parameter.
    ///
    /// \param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c structName parameter.
    ///
    /// \param fieldValue A pointer to the null terminated UTF-8 string that is the value of the
    /// field, if the field has a value. Has the same usage as \c propValue in \c GetProperty.
    ///
    /// \param options Option flags describing the field. See the earlier description.

    void
    SetStructField ( XMP_StringPtr   schemaNS,
                     XMP_StringPtr   structName,
                     XMP_StringPtr   fieldNS,
                     XMP_StringPtr   fieldName,
                     XMP_StringPtr   fieldValue,
                     XMP_OptionBits  options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SetStructField is a simple overload in the template that calls the
    /// above form passing <tt>fieldValue.c_str()</tt>.

    void
    SetStructField ( XMP_StringPtr      schemaNS,
                     XMP_StringPtr      structName,
                     XMP_StringPtr      fieldNS,
                     XMP_StringPtr      fieldName,
                     const tStringObj & fieldValue,
                     XMP_OptionBits     options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetQualifier provides access to a qualifier attached to a property. The namespace
    /// for the qualifier is passed as a URI, you need not worry about the path string syntax. In
    /// many regards qualifiers are like struct fields. See the introductory discussion of
    /// qualified properties for more information.
    ///
    /// The names of qualifiers should be XML qualified names, that is within an XML namespace. The
    /// path syntax for a qualified name uses the namespace prefix, which is unreliable because
    /// the prefix is never guaranteed. The URI is the formal name, the prefix is just a local
    /// shorthand in a given sequence of XML text.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property to which the qualifier is attached. Has the same
    /// usage as in \c GetProperty.
    ///
    /// \param qualNS The namespace URI for the qualifier. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param qualName The name of the qualifier. Must be a single XML name, must not be null or
    /// the empty string. Has the same namespace prefix usage as the \c propName parameter.
    ///
    /// \param qualValue A pointer to the null terminated UTF-8 string that is the value of the
    /// qualifier, if the qualifier has a value. Has the same usage as \c propValue in \c
    /// GetProperty.
    ///
    /// \param options Option flags describing the qualifier. See the earlier description.

    void
    SetQualifier ( XMP_StringPtr  schemaNS,
                   XMP_StringPtr  propName,
                   XMP_StringPtr  qualNS,
                   XMP_StringPtr  qualName,
                   XMP_StringPtr  qualValue,
                   XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SetQualifier is a simple overload in the template that calls the above
    /// form passing <tt>qualValue.c_str()</tt>.

    void
    SetQualifier ( XMP_StringPtr      schemaNS,
                   XMP_StringPtr      propName,
                   XMP_StringPtr      qualNS,
                   XMP_StringPtr      qualName,
                   const tStringObj & qualValue,
                   XMP_OptionBits     options = 0 );

    /// @}

    // =============================================================================================

    //  --------------------------------------------------------------------------------------------
    /// \name Functions for deleting and detecting properties.
    /// @{
    /// These should be obvious from the descriptions of the getters and setters.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DeleteProperty deletes the given XMP subtree rooted at the given property. It is
    /// not an error if the property does not exist.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.

    void
    DeleteProperty ( XMP_StringPtr schemaNS,
                     XMP_StringPtr propName );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DeleteArrayItem deletes the given XMP subtree rooted at the given array item. It
    /// is not an error if the array item does not exist.
    ///
    /// \param schemaNS The namespace URI for the array. Has the same usage as in \c GetProperty.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param itemIndex The index of the desired item. Arrays in XMP are indexed from 1. The
    /// constant \c kXMP_ArrayLastItem always refers to the last existing array item.

    void
    DeleteArrayItem ( XMP_StringPtr schemaNS,
                      XMP_StringPtr arrayName,
                      XMP_Index     itemIndex );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DeleteStructField deletes the given XMP subtree rooted at the given struct field.
    /// It is not an error if the field does not exist.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param structName The name of the struct. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param fieldNS The namespace URI for the field. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c structName parameter.

    void
    DeleteStructField ( XMP_StringPtr schemaNS,
                        XMP_StringPtr structName,
                        XMP_StringPtr fieldNS,
                        XMP_StringPtr fieldName );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DeleteQualifier deletes the given XMP subtree rooted at the given qualifier. It
    /// is not an error if the qualifier does not exist.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property to which the qualifier is attached. Has the same
    /// usage as in \c GetProperty.
    ///
    /// \param qualNS The namespace URI for the qualifier. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param qualName The name of the qualifier. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c propName parameter.

    void
    DeleteQualifier ( XMP_StringPtr schemaNS,
                      XMP_StringPtr propName,
                      XMP_StringPtr qualNS,
                      XMP_StringPtr qualName );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DoesPropertyExist tells if the property exists.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.

    bool
    DoesPropertyExist ( XMP_StringPtr schemaNS,
                        XMP_StringPtr propName ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DoesArrayItemExist tells if the array item exists.
    ///
    /// \result Returns true if the array item exists.
    ///
    /// \param schemaNS The namespace URI for the array. Has the same usage as in \c GetProperty.
    ///
    /// \param arrayName The name of the array. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param itemIndex The index of the desired item. Arrays in XMP are indexed from 1. The
    /// constant \c kXMP_ArrayLastItem always refers to the last existing array item.

    bool
    DoesArrayItemExist ( XMP_StringPtr schemaNS,
                         XMP_StringPtr arrayName,
                         XMP_Index     itemIndex ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DoesStructFieldExist tells if the struct field exists.
    ///
    /// \result Returns true if the field exists.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param structName The name of the struct. May be a general path expression, must not be null
    /// or the empty string. Has the same namespace prefix usage as \c propName in \c GetProperty.
    ///
    /// \param fieldNS The namespace URI for the field. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param fieldName The name of the field. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c structName parameter.

    bool
    DoesStructFieldExist ( XMP_StringPtr schemaNS,
                           XMP_StringPtr structName,
                           XMP_StringPtr fieldNS,
                           XMP_StringPtr fieldName ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief  \c DoesQualifierExist tells if the qualifier exists.
    ///
    /// \result Returns true if the qualifier exists.
    ///
    /// \param schemaNS The namespace URI for the struct. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property to which the qualifier is attached. Has the same
    /// usage as in \c GetProperty.
    ///
    /// \param qualNS The namespace URI for the qualifier. Has the same URI and prefix usage as the
    /// \c schemaNS parameter.
    ///
    /// \param qualName The name of the qualifier. Must be a single XML name, must not be null or the
    /// empty string. Has the same namespace prefix usage as the \c propName parameter.

    bool
    DoesQualifierExist ( XMP_StringPtr schemaNS,
                         XMP_StringPtr propName,
                         XMP_StringPtr qualNS,
                         XMP_StringPtr qualName ) const;

    /// @}

    // =============================================================================================
    // Specialized Get and Set functions
    // =================================

    //  --------------------------------------------------------------------------------------------
    /// \name Functions for accessing localized text (alt-text) properties.
    /// @{
    /// These functions provide convenient support for localized text properties, including a number
    /// of special and obscure aspects. Localized text properties are stored in alt-text arrays.
    /// They allow multiple concurrent localizations of a property value, for example a document
    /// title or copyright in several languages.
    ///
    /// The most important aspect of these functions is that they select an appropriate array item
    /// based on one or two RFC 3066 language tags. One of these languages, the "specific" language,
    /// is preferred and selected if there is an exact match. For many languages it is also possible
    /// to define a "generic" language that may be used if there is no specific language match. The
    /// generic language must be a valid RFC 3066 primary subtag, or the empty string.
    ///
    /// For example, a specific language of "en-US" should be used in the US, and a specific language
    /// of "en-UK" should be used in England. It is also appropriate to use "en" as the generic
    /// language in each case. If a US document goes to England, the "en-US" title is selected
    /// by using the "en" generic language and the "en-UK" specific language.
    ///
    ///	It is considered poor practice, but allowed, to pass a specific language that is just an
    /// RFC 3066 primary tag. For example "en" is not a good specific language, it should only be
    /// used as a generic language. Passing "i" or "x" as the generic language is also considered
    /// poor practice but allowed.
    ///
    /// Advice from the W3C about the use of RFC 3066 language tags can be found at:
    /// \li http://www.w3.org/International/articles/language-tags/
    ///
    /// \note RFC 3066 language tags must be treated in a case insensitive manner. The XMP toolkit
    /// does this by normalizing their capitalization:
    ///
	/// \li The primary subtag is lower case, the suggested practice of ISO 639.
	/// \li All 2 letter secondary subtags are upper case, the suggested practice of ISO 3166.
	/// \li All other subtags are lower case.
    ///
    /// The XMP specification defines an artificial language, "x-default", that is used to
    /// explicitly denote a default item in an alt-text array. The XMP toolkit normalizes alt-text
    /// arrays such that the x-default item is the first item. The \c SetLocalizedText function has
    /// several special features related to the x-default item, see its description for details.
    ///
    /// The selection of the array item is the same for \c GetLocalizedText and \c SetLocalizedText:
    ///
    /// \li Look for an exact match with the specific language.
    /// \li If a generic language is given, look for a partial match.
    /// \li Look for an x-default item.
    /// \li Choose the first item.
    ///
    /// A partial match with the generic language is where the start of the item's language matches
    /// the generic string and the next character is '-'. An exact match is also recognized as a
    /// degenerate case.
    ///
    /// It is fine to pass x-default as the specific language. In this case, selection of an x-default
    /// item is an exact match by the first rule, not a selection by the 3rd rule. The last 2 rules
    /// are fallbacks used when the specific and generic languages fail to produce a match.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetLocalizedText returns information about a selected item in an alt-text array.
    /// The array item is selected according to the rules given above.
    ///
    /// \result Returns true if an appropriate array item exists.
    ///
    /// \param schemaNS The namespace URI for the alt-text array. Has the same usage as in \c
    /// GetProperty.
    ///
    /// \param altTextName The name of the alt-text array. May be a general path expression, must
    /// not be null or the empty string. Has the same namespace prefix usage as \c propName in \c
    /// GetProperty.
    ///
    /// \param genericLang The name of the generic language as an RFC 3066 primary subtag. May be
    /// null or the empty string if no generic language is wanted.
    ///
    /// \param specificLang The name of the specific language as an RFC 3066 tag. Must not be null
    /// or the empty string.
    ///
    /// \param actualLang A pointer to the string that is assigned the language of the selected
    /// array item, if an appropriate array item is found. May be null if the language is not
    /// wanted.
    ///
    /// \param itemValue A pointer to the string that is assigned the value of the array item, if
    /// an appropriate array item is found. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the array item. May be null if the flags are not wanted.

    bool
    GetLocalizedText ( XMP_StringPtr    schemaNS,
                       XMP_StringPtr    altTextName,
                       XMP_StringPtr    genericLang,
                       XMP_StringPtr    specificLang,
                       tStringObj *     actualLang,
                       tStringObj *     itemValue,
                       XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetLocalizedText modifies the value of a selected item in an alt-text array.
    /// Creates an appropriate array item if necessary, and handles special cases for the x-default
    /// item.
    ///
    /// If the selected item is from a match with the specific language, the value of that item is
    /// modified. If the existing value of that item matches the existing value of the x-default
    /// item, the x-default item is also modified. If the array only has 1 existing item (which is
    /// not x-default), an x-default item is added with the given value.
    ///
    /// If the selected item is from a match with the generic language and there are no other
    /// generic matches, the value of that item is modified. If the existing value of that item
    /// matches the existing value of the x-default item, the x-default item is also modified. If
    /// the array only has 1 existing item (which is not x-default), an x-default item is added
    /// with the given value.
    ///
    /// If the selected item is from a partial match with the generic language and there are other
    /// partial matches, a new item is created for the specific language. The x-default item is not
    /// modified.
    ///
    /// If the selected item is from the last 2 rules then a new item is created for the specific
    /// language. If the array only had an x-default item, the x-default item is also modified. If
    /// the array was empty, items are created for the specific language and x-default.
    ///
    /// \param schemaNS The namespace URI for the alt-text array. Has the same usage as in \c
    /// GetProperty.
    ///
    /// \param altTextName The name of the alt-text array. May be a general path expression, must
    /// not be null or the empty string. Has the same namespace prefix usage as \c propName in \c
    /// GetProperty.
    ///
    /// \param genericLang The name of the generic language as an RFC 3066 primary subtag. May be
    /// null or the empty string if no generic language is wanted.
    ///
    /// \param specificLang The name of the specific language as an RFC 3066 tag. Must not be null
    /// or the empty string.
    ///
    /// \param itemValue A pointer to the null terminated UTF-8 string that is the new value for
    /// the appropriate array item.
    ///
    /// \param options Option flags, none are defined at present.

    void
    SetLocalizedText ( XMP_StringPtr  schemaNS,
                       XMP_StringPtr  altTextName,
                       XMP_StringPtr  genericLang,
                       XMP_StringPtr  specificLang,
                       XMP_StringPtr  itemValue,
                       XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SetLocalizedText is a simple overload in the template that calls the
    /// above form passing <tt>itemValue.c_str()</tt>.

    void
    SetLocalizedText ( XMP_StringPtr      schemaNS,
                       XMP_StringPtr      altTextName,
                       XMP_StringPtr      genericLang,
                       XMP_StringPtr      specificLang,
                       const tStringObj & itemValue,
                       XMP_OptionBits     options = 0 );

    /// @}

    // =============================================================================================

    //  --------------------------------------------------------------------------------------------
    /// \name Functions accessing properties as binary values.
    /// @{
	/// These are very similar to \c GetProperty and \c SetProperty above, but the value is
	/// returned or
    /// provided in binary form instead of as a UTF-8 string. The path composition functions in
    /// \c TXMPUtils may be used to compose an path expression for fields in nested structures, items
    /// in arrays, or qualifiers.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty_Bool returns the value of a Boolean property as a C++ bool.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the bool variable that is assigned the value of the property.
    /// May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty_Bool ( XMP_StringPtr    schemaNS,
                       XMP_StringPtr    propName,
                       bool *           propValue,
                       XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty_Int returns the value of an integer property as a C long integer.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the long integer variable that is assigned the value of
    /// the property. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty_Int ( XMP_StringPtr    schemaNS,
                      XMP_StringPtr    propName,
                      long *           propValue,
                      XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty_Int64 returns the value of an integer property as a C long long integer.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the long long integer variable that is assigned the value of
    /// the property. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty_Int64 ( XMP_StringPtr    schemaNS,
                        XMP_StringPtr    propName,
                        long long *      propValue,
                        XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty_Float returns the value of a flaoting point property as a C double float.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the double float variable that is assigned the value of
    /// the property. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty_Float ( XMP_StringPtr    schemaNS,
                        XMP_StringPtr    propName,
                        double *         propValue,
                        XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c GetProperty_Date returns the value of a date/time property as an \c XMP_DateTime struct.
    ///
    /// \result Returns true if the property exists.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue A pointer to the \c XMP_DateTime variable that is assigned the value of
    /// the property. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    bool
    GetProperty_Date ( XMP_StringPtr    schemaNS,
                       XMP_StringPtr    propName,
                       XMP_DateTime *   propValue,
                       XMP_OptionBits * options ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty_Bool sets the value of a Boolean property from a C++ bool.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue The bool value to be assigned to the property.
    ///
    /// \param options A pointer to the \c XMP_OptionBits variable that is assigned option flags
    /// describing the property. May be null if the flags are not wanted.

    void
    SetProperty_Bool ( XMP_StringPtr  schemaNS,
                       XMP_StringPtr  propName,
                       bool           propValue,
                       XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty_Int sets the value of an integer property from a C long integer.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue The long integer value to be assigned to the property.
    ///
    /// \param options Option flags describing the property.

    void
    SetProperty_Int ( XMP_StringPtr  schemaNS,
                      XMP_StringPtr  propName,
                      long           propValue,
                      XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty_Int64 sets the value of an integer property from a C long long integer.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue The long long integer value to be assigned to the property.
    ///
    /// \param options Option flags describing the property.

    void
    SetProperty_Int64 ( XMP_StringPtr  schemaNS,
                        XMP_StringPtr  propName,
                        long long      propValue,
                        XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty_Float sets the value of a floating point property from a C double float.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue The double float value to be assigned to the property.
    ///
    /// \param options Option flags describing the property.

    void
    SetProperty_Float ( XMP_StringPtr  schemaNS,
                        XMP_StringPtr  propName,
                        double         propValue,
                        XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SetProperty_Date sets the value of a date/time property from an \c XMP_DateTime struct.
    ///
    /// \param schemaNS The namespace URI for the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propName The name of the property. Has the same usage as in \c GetProperty.
    ///
    /// \param propValue The \c XMP_DateTime value to be assigned to the property.
    ///
    /// \param options Option flags describing the property.

    void
    SetProperty_Date ( XMP_StringPtr         schemaNS,
                       XMP_StringPtr         propName,
                       const XMP_DateTime &  propValue,
                       XMP_OptionBits        options = 0 );

    /// @}

    // =============================================================================================
    // Miscellaneous Member Functions
    // ==============================

    //  --------------------------------------------------------------------------------------------
    /// \name Misceallaneous functions.
    /// @{

    //  --------------------------------------------------------------------------------------------
    /// \brief GetInternalRef Returns an internal reference that may be safely passed across DLL
    /// boundaries and reconstructed.

    XMPMetaRef
    GetInternalRef() const;

    //  --------------------------------------------------------------------------------------------
    /// \brief GetObjectName --TBD--

    void
    GetObjectName ( tStringObj * name ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief SetObjectName --TBD--

    void
    SetObjectName ( XMP_StringPtr name );

    //  --------------------------------------------------------------------------------------------
    /// \brief SetObjectName --TBD--

    void
    SetObjectName ( tStringObj name );

    //  --------------------------------------------------------------------------------------------
    /// \brief GetObjectOptions --TBD--

    XMP_OptionBits
    GetObjectOptions() const;

    //  --------------------------------------------------------------------------------------------
    /// \brief SetObjectOptions --TBD--
    ///
    /// \note <b>Not yet implemented.</b> File a bug if you need this.

    void
    SetObjectOptions ( XMP_OptionBits options );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c Clone creates a deep clone of the XMP object.
    ///
    /// This function creates a deep clone of the XMP object. Assignment and copy constructors do
    /// not, they just increment a reference count. Note that \c Clone returns an object, not a
    /// pointer. This is easy to misuse:
    ///
    /// \code
    ///		SXMPMeta * clone1 = &sourceXMP.Clone();                  // ! This does not work!
    ///		SXMPMeta * clone2 = new SXMPMeta ( sourceXMP.Clone() );  // This works.
    ///		SXMPMeta   clone3 ( sourceXMP.Clone );  // This works also. (Not a pointer.)
    /// \endcode
    ///
    /// In the code above, the assignment to \c clone1 creates a temporary object, initializes it
    /// with the clone, assigns the address of the temporary to \c clone1, then deletes the
    /// temporary. The \c clone3 example also works, you do not have to use an explicit pointer.
    /// This is good for local usage, you don't have to worry about memory leaks.
    ///
    /// \param options Option flags, not are defined at present.
    ///
    /// \result An XMP object cloned from the original.

    TXMPMeta
    Clone ( XMP_OptionBits options = 0 ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief CountArrayItems --TBD--

    XMP_Index
    CountArrayItems ( XMP_StringPtr schemaNS,
                      XMP_StringPtr arrayName ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief \c DumpObject dumps the content of an XMP object.

    XMP_Status
    DumpObject ( XMP_TextOutputProc outProc,
                 void *             refCon ) const;

    /// @}

    // =============================================================================================

    //  --------------------------------------------------------------------------------------------
    /// \name Functions for parsing and serializing.
    /// @{
	/// These functions support parsing serialized RDF into an XMP object, and serailizing an XMP
    /// object into RDF. The input for parsing may be any valid Unicode encoding. ISO Latin-1 is
    /// also recognized, but its use is strongly discouraged. Serialization is always as UTF-8.

    //  --------------------------------------------------------------------------------------------
    /// \brief \c ParseFromBuffer parses RDF from a series of input buffers. The buffers may be any
    /// length. The buffer boundaries need not respect XML tokens or even Unicode characters.
    ///
    /// \param buffer A pointer to a buffer of input. May be null if \c bufferSize is 0.
    ///
    /// \param bufferSize The length of this buffer in bytes. Zero is a valid value. Termination of
    /// an input loop is convenient by passing \c kXMP_ParseMoreBuffers for all real input, then
    /// having a final call with a zero length and \c kXMP_NoOptions.
    ///
    /// \param options Options controlling the parsing.
    ///
    /// The available options are:
    ///
    /// \li \c kXMP_ParseMoreBuffers - This is not the last buffer of input, more calls follow.
    /// \li \c kXMP_RequireXMPMeta - The x:xmpmeta XML element is required around <tt>rdf:RDF</tt>.
    /// \li \c kXMP_StrictAliasing - Do not reconcile alias differences, throw an exception.
    ///
    /// \note The \c kXMP_StrictAliasing option is not yet implemented.

    void
    ParseFromBuffer ( XMP_StringPtr  buffer,
                      XMP_StringLen  bufferSize,
                      XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief \c SerializeToBuffer serializes an XMP object into a string as RDF.
    ///
    /// \param rdfString A pointer to the string to receive the serialized RDF. Must not be null.
    ///
    /// \param options Option flags to control the serialization.
    ///
    /// \param padding The amount of padding to be added if a writeable XML packet is created. If
    /// zero is passed (the default) an appropriate amount of padding is computed.
    ///
    /// \param newline The string to be used as a line terminator. If empty it defaults to
    /// linefeed, U+000A, the standard XML newline.
    ///
    /// \param indent The string to be used for each level of indentation in the serialized RDF. If
    /// empty it defaults to two ASCII spaces, U+0020.
    ///
    /// \param baseIndent The number of levels of indentation to be used for the outermost XML
    /// element in the serialized RDF. This is convenient when embedding the RDF in other text.
    ///
    /// The available option flags are:
    ///
    /// \li \c kXMP_OmitPacketWrapper - Do not include an XML packet wrapper.
    /// \li \c kXMP_ReadOnlyPacket - Create a read-only XML packet wapper.
    /// \li \c kXMP_UseCompactFormat - Use a highly compact RDF syntax and layout.
    /// \li \c kXMP_WriteAliasComments - Include XML comments for aliases.
    /// \li \c kXMP_IncludeThumbnailPad - Include typical space for a JPEG thumbnail in the padding
    /// if no <tt>xmp:Thumbnails</tt> property is present.
    /// \li \c kXMP_ExactPacketLength - The padding parameter provides the overall packet length.
    /// The actual amount of padding is computed. An exception is thrown if the packet exceeds this
    /// length with no padding.
    ///
    /// The specified options must be logically consistent, an exception is thrown if not. You
    /// cannot specify both \c kXMP_OmitPacketWrapper along with \c kXMP_ReadOnlyPacket, \c
    /// kXMP_IncludeThumbnailPad, or \c kXMP_ExactPacketLength.
    ///
    /// In addition, one of the following encoding options may be included:
    ///
    /// \li \c kXMP_EncodeUTF8 - Encode as UTF-8, the default.
    /// \li \c kXMP_EncodeUTF16Big - Encode as big-endian UTF-16.
    /// \li \c kXMP_EncodeUTF16Little - Encode as little-endian UTF-16.
    /// \li \c kXMP_EncodeUTF32Big - Encode as big-endian UTF-32.
    /// \li \c kXMP_EncodeUTF32Little - Encode as little-endian UTF-32.

    void
    SerializeToBuffer ( tStringObj *   rdfString,
                        XMP_OptionBits options,
                        XMP_StringLen  padding,
                        XMP_StringPtr  newline,
                        XMP_StringPtr  indent = "",
                        XMP_Index      baseIndent = 0 ) const;

    //  --------------------------------------------------------------------------------------------
    /// \brief This form of \c SerializeToBuffer is a simple overload in the template that calls the
    /// above form passing default values for the \c newline, \c indent, and \c baseIndent
    /// parameters.

    void
    SerializeToBuffer ( tStringObj *   rdfString,
                        XMP_OptionBits options = 0,
                        XMP_StringLen  padding = 0 ) const;

    /// @}

    // =============================================================================================

    XMPMetaRef xmpRef;  // *** Should be private, see below.

private:

#if 0	// *** VS.Net and gcc seem to not handle the friend declarations properly.
    friend class TXMPIterator <class tStringObj>;
    friend class TXMPUtils <class tStringObj>;
#endif

};  // class TXMPMeta

#endif  // __TXMPMeta_hpp__
