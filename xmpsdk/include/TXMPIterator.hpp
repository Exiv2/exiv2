#ifndef __TXMPIterator_hpp__
#define __TXMPIterator_hpp__ 1

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
/// \file TXMPIterator.hpp
/// \brief Template class for the XMP Toolkit iteration services.
///
/// This template class provides iteration services for the XMP Toolkit. It should be instantiated
/// with a string class such as <tt>std::string</tt>. Please read the general usage notes for
/// information on the overall architecture of the XMP API.
//  ================================================================================================

//  ================================================================================================
/// \class TXMPIterator TXMPIterator.hpp
/// \brief Template class for the XMP Toolkit iteration services.
///
/// This template class provides iteration services for the XMP Toolkit. It should be instantiated
/// with a string class such as <tt>std::string</tt>. Please read the general usage notes for
/// information on the overall architecture of the XMP API.
///
/// \c TXMPIterator provides a uniform means to iterate over several XMP data structures, including
/// the schema and properties within an XMP object plus global tables such as registered
/// namespaces. The template wraps a string class around the raw XMP API, so that output strings
/// are automatically copied and access is fully thread safe.  String objects are only necessary
/// for output strings. Input string are literals and passed as typical C <tt>const char *</tt>.
///
/// The template parameter, class \c TtStringObj, is described in the XMPSDK.hpp umbrella header.
///
/// \note Only XMP object iteration is implemented at this time. There are no table iterators yet.
///
/// Iteration over the schema and properties within an XMP object is the most important and complex
/// use of \c TTXMPIterator. It is helpful to have a thorough understanding of the XMP data tree.
/// One way to learn this is to create some complex XMP and examine the output of
/// <tt>TXMPMeta::DumpObject</tt>. This is also described in the XMP Specification, in the XMP Data
/// Model chapter.
///
/// The top of the XMP data tree is a single root node. This does not explicitly appear in the dump
/// and is never visited by an iterator (that is, it is never returned from
/// <tt>TXMPIterator::Next</tt>). Beneath the root are schema nodes. These are just collectors for
/// top level properties in the same namespace. They are created and destroyed implicitly. Beneath
/// the schema nodes are the property nodes. The nodes below a property node depend on its type
/// (simple, struct, or array) and whether it has qualifiers.
///
/// A \c TXMPIterator constructor defines a starting point for the iteration and options that control
/// how it proceeds. By default the iteration starts at the root and visits all nodes beneath it in
/// a depth first manner. The root node is not visited, the first visited node is a schema node. You
/// can provide a schema name or property path to select a different starting node. By default this
/// visits the named root node first then all nodes beneath it in a depth first manner.
///
/// The <tt>TXMPIterator::Next</tt> method delivers the schema URI, path, and option flags for the
/// node being visited. If the node is simple it also delivers the value. Qualifiers for this node
/// are visited next. The fields of a struct or items of an array are visited after the qualifiers
/// of the parent.
///
/// The options to control the iteration are:
///
/// \li \c kXMP_IterJustChildren - Visit just the immediate children of the root. Skip the root
/// itself and all nodes below the immediate children. This omits the qualifiers of the immediate
/// children, the qualifier nodes being below what they qualify.
///
/// \li \c kXMP_IterJustLeafNodes - Visit just the leaf property nodes and their qualifiers.
///
/// \li \c kXMP_IterJustLeafName - Return just the leaf component of the node names. The default is
/// to return the full path name.
///
/// \li \c kXMP_IterIncludeAliases - Include aliases as part of the iteration. Since aliases are not
/// actual nodes the default iteration does not visit them.
///
/// \li \c kXMP_IterOmitQualifiers - Do not visit the qualifiers of a node.
///
//  ================================================================================================

#include "client-glue/WXMPIterator.hpp"

template <class tStringObj>
class TXMPIterator {

public:

    //  --------------------------------------------------------------------------------------------
    /// \brief Assignment operator, assigns the internal ref and increments the ref count.
    ///
    /// The assignment operator assigns the internal ref from the rhs object and increments the
    /// reference count on the underlying internal XMP iterator.

    void operator= ( const TXMPIterator<tStringObj> & rhs );

    //  --------------------------------------------------------------------------------------------
    /// \brief Copy constructor, creates a client object refering to the same internal object.
    ///
    /// The copy constructor creates a new client iterator that refers to the same underlying iterator.

    TXMPIterator ( const TXMPIterator<tStringObj> & original );

    //  --------------------------------------------------------------------------------------------
    /// \brief Construct an iterator for the properties within an XMP object.
    ///
    /// Construct an iterator for the properties within an XMP object. The general operation of an
    /// XMP object iterator was described above. Overloaded forms are provided to iterate the entire
    /// data tree, properties within a specific schema, or a subtree rooted at a specific node.
    ///
    /// \param xmpObj The XMP object over which to iterate.
    ///
    /// \param schemaNS Optional schema namespace URI to restrict the iteration. Omitted (visit all
    /// schema) by passing 0 or "".
    ///
    /// \param propName Optional property name to restrict the iteration. May be an arbitrary path
    /// expression. Omitted (visit all properties) by passing 0 or "". If not null/empty a schema
    /// URI must also be provided.
    ///
    /// \param options Option flags to control the iteration.
    ///
    /// The available option flags are:
    ///
    /// \li \c kXMP_IterJustChildren - Just visit the immediate children of the root, default is subtree.
    /// \li \c kXMP_IterJustLeafNodes - Just visit the leaf nodes, default visits all nodes.
    /// \li \c kXMP_IterJustLeafName - Return just the leaf part of the path, default is the full path.
    /// \li \c kXMP_IterOmitQualifiers - Omit all qualifiers.

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_StringPtr  schemaNS,
                   XMP_StringPtr  propName,
                   XMP_OptionBits options = 0 );

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_StringPtr  schemaNS,
                   XMP_OptionBits options = 0 );

    TXMPIterator ( const TXMPMeta<tStringObj> & xmpObj,
                   XMP_OptionBits options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief Construct an iterator for the global tables of the XMP toolkit.
    ///
    /// \note <b>Not yet implemented.</b> File a bug if you need this.

    TXMPIterator ( XMP_StringPtr  schemaNS,
                   XMP_StringPtr  propName,
                   XMP_OptionBits options );

    //  --------------------------------------------------------------------------------------------
    /// \brief Destructor, typical virtual destructor.

    virtual ~TXMPIterator() throw();

    //  --------------------------------------------------------------------------------------------
    /// \brief Visit the next node in the iteration.
    ///
    /// \result Returns true if there was another node to visit, false if the iteration is done.
    ///
    /// \param schemaNS A pointer to the string that is assigned the schema namespace URI of
    /// the current property. May be null if the value is not wanted.
    ///
    /// \param propPath A pointer to the string that is assigned the XPath name of the current
    /// property. May be null if the value is not wanted.
    ///
    /// \param propValue A pointer to the string that is assigned the value of the current
    /// property. May be null if the value is not wanted.
    ///
    /// \param options A pointer to the XMP_OptionBits variable that is assigned the flags
    /// describing the current property.

    bool
    Next ( tStringObj *     schemaNS = 0,
           tStringObj *     propPath = 0,
           tStringObj *     propValue = 0,
           XMP_OptionBits * options = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief Skip some portion of the remaining iterations.
    ///
    /// \param options Option flags to control the iteration.
    ///
    /// The available option flags are:
    ///
    /// \li \c kXMP_IterSkipSubtree -  Skip the subtree below the current node.
    /// \li \c kXMP_IterSkipSiblings - Skip the subtree below and remaining siblings of the current node.

    void
    Skip ( XMP_OptionBits options );

private:

    XMPIteratorRef  iterRef;

    TXMPIterator();	// ! Hidden, must choose property or table iteration.

};  // class TXMPIterator

// =================================================================================================

#endif // __TXMPIterator_hpp__
