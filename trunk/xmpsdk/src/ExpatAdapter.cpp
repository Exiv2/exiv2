// =================================================================================================
// Copyright 2005-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMPCore_Impl.hpp"

#include "ExpatAdapter.hpp"
#include "XMPMeta.hpp"

#include <string.h>

using namespace std;

#if XMP_WinBuild
	#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif

// *** Set memory handlers.

#ifndef DumpXMLParseEvents
	#define DumpXMLParseEvents 0
#endif

#define FullNameSeparator	'@'

// =================================================================================================

static void StartNamespaceDeclHandler    ( void * userData, XMP_StringPtr prefix, XMP_StringPtr uri );
static void EndNamespaceDeclHandler      ( void * userData, XMP_StringPtr prefix );

static void StartElementHandler          ( void * userData, XMP_StringPtr name, XMP_StringPtr* attrs );
static void EndElementHandler            ( void * userData, XMP_StringPtr name );

static void CharacterDataHandler         ( void * userData, XMP_StringPtr cData, int len );
static void StartCdataSectionHandler     ( void * userData );
static void EndCdataSectionHandler       ( void * userData );

static void ProcessingInstructionHandler ( void * userData, XMP_StringPtr target, XMP_StringPtr data );
static void CommentHandler               ( void * userData, XMP_StringPtr comment );

// =================================================================================================
// =================================================================================================

ExpatAdapter::ExpatAdapter() : parser(0), nesting(0)
{

	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( this->parseLog == 0 ) this->parseLog = stdout;
	#endif

	this->parser = XML_ParserCreateNS ( 0, FullNameSeparator );
	if ( this->parser == 0 ) XMP_Throw ( "Failure creating Expat parser", kXMPErr_ExternalFailure );
	
	XML_SetUserData ( this->parser, this );
	
	XML_SetNamespaceDeclHandler ( this->parser, StartNamespaceDeclHandler, EndNamespaceDeclHandler );
	XML_SetElementHandler ( this->parser, StartElementHandler, EndElementHandler );

	XML_SetCharacterDataHandler ( this->parser, CharacterDataHandler );
	XML_SetCdataSectionHandler ( this->parser, StartCdataSectionHandler, EndCdataSectionHandler );

	XML_SetProcessingInstructionHandler ( this->parser, ProcessingInstructionHandler );
	XML_SetCommentHandler ( this->parser, CommentHandler );

	// ??? XML_SetDefaultHandlerExpand ( this->parser, DefaultHandler );
	
	this->parseStack.push_back ( &this->tree );	// Push the XML root node.

}	// ExpatAdapter::ExpatAdapter

// =================================================================================================

ExpatAdapter::~ExpatAdapter()
{

	if ( this->parser != 0 ) XML_ParserFree ( this->parser );
	this->parser = 0;

}	// ExpatAdapter::~ExpatAdapter

// =================================================================================================

#if XMP_DebugBuild
	static XMP_VarString sExpatMessage;
#endif

static const char * kOneSpace = " ";

void ExpatAdapter::ParseBuffer ( const void * buffer, size_t length, bool last )
{
	enum XML_Status status;
	
	if ( length == 0 ) {	// Expat does not like empty buffers.
		if ( ! last ) return;
		buffer = kOneSpace;
		length = 1;
	}
	
	status = XML_Parse ( this->parser, (const char *)buffer, length, last );

	if ( status != XML_STATUS_OK ) {
	
		XMP_StringPtr errMsg = "XML parsing failure";

		#if 0	// XMP_DebugBuild	// Disable for now to make test output uniform. Restore later with thread safety.
		
			// *** This is a good candidate for a callback error notification mechanism.
			// *** This code is not thread safe, the sExpatMessage isn't locked. But that's OK for debug usage.

			enum XML_Error expatErr = XML_GetErrorCode ( this->parser );
			const char *   expatMsg = XML_ErrorString ( expatErr );
			int errLine = XML_GetCurrentLineNumber ( this->parser );
		
			char msgBuffer[1000];
			// AUDIT: Use of sizeof(msgBuffer) for snprintf length is safe.
			snprintf ( msgBuffer, sizeof(msgBuffer), "# Expat error %d at line %d, \"%s\"", expatErr, errLine, expatMsg );
			sExpatMessage = msgBuffer;
			errMsg = sExpatMessage.c_str();

			#if  DumpXMLParseEvents
				if ( this->parseLog != 0 ) fprintf ( this->parseLog, "%s\n", errMsg, expatErr, errLine, expatMsg );
			#endif

		#endif

		XMP_Throw ( errMsg, kXMPErr_BadXML );

	}
	
}	// ExpatAdapter::ParseBuffer

// =================================================================================================
// =================================================================================================

#if XMP_DebugBuild & DumpXMLParseEvents

	static inline void PrintIndent ( FILE * file, size_t nesting )
	{
		for ( ; nesting > 0; --nesting ) fprintf ( file, "  " );
	}

#endif

// =================================================================================================

static void SetQualName ( XMP_StringPtr fullName, XML_Node * node )
{
	// Expat delivers the full name as a catenation of namespace URI, separator, and local name.

	// As a compatibility hack, an "about" or "ID" attribute of an rdf:Description element is
	// changed to "rdf:about" or rdf:ID. Easier done here than in the RDF recognizer.
	
	// As a bug fix hack, change a URI of "http://purl.org/dc/1.1/" to ""http://purl.org/dc/elements/1.1/.
	// Early versions of Flash that put XMP in SWF used a bad URI for the dc: namespace.

	// ! This code presumes the RDF namespace prefix is "rdf".

	size_t sepPos = strlen(fullName);
	for ( --sepPos; sepPos > 0; --sepPos ) {
		if ( fullName[sepPos] == FullNameSeparator ) break;
	}

	if ( fullName[sepPos] == FullNameSeparator ) {

		XMP_StringPtr prefix;
		XMP_StringPtr localPart = fullName + sepPos + 1;

		node->ns.assign ( fullName, sepPos );
		if ( node->ns == "http://purl.org/dc/1.1/" ) node->ns = "http://purl.org/dc/elements/1.1/";
		bool found = XMPMeta::GetNamespacePrefix ( node->ns.c_str(), &prefix, &voidStringLen );
		if ( ! found ) XMP_Throw ( "Unknown URI in Expat full name", kXMPErr_ExternalFailure );
		
		node->name = prefix;
		node->name += localPart;

	} else {

		node->name = fullName;	// The name is not in a namespace.
	
		if ( node->parent->name == "rdf:Description" ) {
			if ( node->name == "about" ) {
				node->ns   = kXMP_NS_RDF;
				node->name = "rdf:about";
			} else if ( node->name == "ID" ) {
				node->ns   = kXMP_NS_RDF;
				node->name = "rdf:ID";
			}
		}
		
	}

}	// SetQualName

// =================================================================================================

static void StartNamespaceDeclHandler ( void * userData, XMP_StringPtr prefix, XMP_StringPtr uri )
{
	IgnoreParam(userData);
	
	// As a bug fix hack, change a URI of "http://purl.org/dc/1.1/" to ""http://purl.org/dc/elements/1.1/.
	// Early versions of Flash that put XMP in SWF used a bad URI for the dc: namespace.
	
	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( prefix == 0 ) prefix = "_dflt_";	// Have default namespace.
	if ( uri == 0 ) return;	// Ignore, have xmlns:pre="", no URI to register.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "StartNamespace: %s - \"%s\"\n", prefix, uri );
		}
	#endif
	
	if ( XMP_LitMatch ( uri, "http://purl.org/dc/1.1/" ) ) uri = "http://purl.org/dc/elements/1.1/";
	(void) XMPMeta::RegisterNamespace ( uri, prefix, &voidStringPtr, &voidStringLen );

}	// StartNamespaceDeclHandler

// =================================================================================================

static void EndNamespaceDeclHandler ( void * userData, XMP_StringPtr prefix )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( prefix == 0 ) prefix = "_dflt_";	// Have default namespace.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "EndNamespace: %s\n", prefix );
		}
	#endif
	
	// ! Nothing to do, Expat has done all of the XML processing.

}	// EndNamespaceDeclHandler

// =================================================================================================

static void StartElementHandler ( void * userData, XMP_StringPtr name, XMP_StringPtr* attrs )
{
	XMP_Assert ( attrs != 0 );
	ExpatAdapter * thiz = (ExpatAdapter*)userData;
	
	size_t attrCount = 0;
	for ( XMP_StringPtr* a = attrs; *a != 0; ++a ) ++attrCount;
	if ( (attrCount & 1) != 0 )	XMP_Throw ( "Expat attribute info has odd length", kXMPErr_ExternalFailure );
	attrCount = attrCount/2;	// They are name/value pairs.
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "StartElement: %s, %d attrs", name, attrCount );
			for ( XMP_StringPtr* attr = attrs; *attr != 0; attr += 2 ) {
				XMP_StringPtr attrName = *attr;
				XMP_StringPtr attrValue = *(attr+1);
				fprintf ( thiz->parseLog, ", %s = \"%s\"", attrName, attrValue );
			}
			fprintf ( thiz->parseLog, "\n" );
		}
	#endif

	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * elemNode   = new XML_Node ( parentNode, "", kElemNode );
	
	SetQualName ( name, elemNode );
	
	for ( XMP_StringPtr* attr = attrs; *attr != 0; attr += 2 ) {

		XMP_StringPtr attrName = *attr;
		XMP_StringPtr attrValue = *(attr+1);
		XML_Node * attrNode = new XML_Node ( elemNode, "", kAttrNode );

		SetQualName ( attrName, attrNode );
		attrNode->value = attrValue;
		if ( attrNode->name == "xml:lang" ) NormalizeLangValue ( &attrNode->value );
		elemNode->attrs.push_back ( attrNode );

	}
	
	parentNode->content.push_back ( elemNode );
	thiz->parseStack.push_back ( elemNode );
	
	if ( (elemNode->name == "rdf:RDF") || (elemNode->name == "pxmp:XMP_Packet") ) {
		thiz->rootNode = elemNode;
		++thiz->rootCount;
	}

	++thiz->nesting;

}	// StartElementHandler

// =================================================================================================

static void EndElementHandler ( void * userData, XMP_StringPtr name )
{
	IgnoreParam(name);
	
	ExpatAdapter * thiz = (ExpatAdapter*)userData;

	--thiz->nesting;
	(void) thiz->parseStack.pop_back();
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "EndElement: %s\n", name );
		}
	#endif

}	// EndElementHandler

// =================================================================================================

static void CharacterDataHandler ( void * userData, XMP_StringPtr cData, int len )
{
	ExpatAdapter * thiz = (ExpatAdapter*)userData;
	
	if ( (cData == 0) || (len == 0) ) { cData = ""; len = 0; }
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "CharContent: \"" );
			for ( ; len > 0; --len, ++cData ) fprintf ( thiz->parseLog, "%c", *cData );
			fprintf ( thiz->parseLog, "\"\n" );
		}
	#endif
	
	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * cDataNode  = new XML_Node ( parentNode, "", kCDataNode );
	
	cDataNode->value.assign ( cData, len );
	parentNode->content.push_back ( cDataNode );
	
}	// CharacterDataHandler

// =================================================================================================

static void StartCdataSectionHandler ( void * userData )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "StartCDATA\n" );
		}
	#endif
	
	// *** Since markup isn't recognized inside CDATA, this affects XMP's double escaping.
	
}	// StartCdataSectionHandler

// =================================================================================================

static void EndCdataSectionHandler ( void * userData )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "EndCDATA\n" );
		}
	#endif	

}	// EndCdataSectionHandler

// =================================================================================================

static void ProcessingInstructionHandler ( void * userData, XMP_StringPtr target, XMP_StringPtr data )
{
	XMP_Assert ( target != 0 );
	ExpatAdapter * thiz = (ExpatAdapter*)userData;

	if ( ! XMP_LitMatch ( target, "xpacket" ) ) return;	// Ignore all PIs except the XMP packet wrapper.
	if ( data == 0 ) data = "";
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "PI: %s - \"%s\"\n", target, data );
		}
	#endif
	
	XML_Node * parentNode = thiz->parseStack.back();
	XML_Node * piNode  = new XML_Node ( parentNode, target, kPINode );
	
	piNode->value.assign ( data );
	parentNode->content.push_back ( piNode );
	
}	// ProcessingInstructionHandler

// =================================================================================================

static void CommentHandler ( void * userData, XMP_StringPtr comment )
{
	IgnoreParam(userData);

	#if XMP_DebugBuild & DumpXMLParseEvents		// Avoid unused variable warning.
		ExpatAdapter * thiz = (ExpatAdapter*)userData;
	#endif

	if ( comment == 0 ) comment = "";
	
	#if XMP_DebugBuild & DumpXMLParseEvents
		if ( thiz->parseLog != 0 ) {
			PrintIndent ( thiz->parseLog, thiz->nesting );
			fprintf ( thiz->parseLog, "Comment: \"%s\"\n", comment );
		}
	#endif
	
	// ! Comments are ignored.
	
}	// CommentHandler

// =================================================================================================
