// =================================================================================================
// Copyright 2002-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
//
// Adobe patent application tracking #P435, entitled 'Unique markers to simplify embedding data of
// one format in a file with a different format', inventors: Sean Parent, Greg Gilley.
// =================================================================================================

#include "XMP_Environment.h"	// ! This must be the first include!
#include "XMPCore_Impl.hpp"

#include "XMPMeta.hpp"
#include "XMPIterator.hpp"
#include "XMPUtils.hpp"

#include "XMP_Version.h"
#include "UnicodeInlines.incl_cpp"
#include "UnicodeConversions.hpp"

#include <cstdio>

#if XMP_DebugBuild
	#include <iostream>
#endif

using namespace std;

#if XMP_WinBuild
	#pragma warning ( disable : 4533 )	// initialization of '...' is skipped by 'goto ...'
	#pragma warning ( disable : 4702 )	// unreachable code
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
	#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif


// *** Use the XMP_PropIsXyz (Schema, Simple, Struct, Array, ...) macros
// *** Add debug codegen checks, e.g. that typical masking operations really work
// *** Change all uses of strcmp and strncmp to XMP_LitMatch and XMP_LitNMatch


// =================================================================================================
// Local Types and Constants
// =========================


// =================================================================================================
// Static Variables
// ================

XMP_VarString * xdefaultName = 0;

// These are embedded version strings.

const char * kXMPCore_EmbeddedVersion   = kXMPCore_VersionMessage;
const char * kXMPCore_EmbeddedCopyright = kXMPCoreName " " kXMP_CopyrightStr;

// =================================================================================================
// Local Utilities
// ===============

#define IsHexDigit(ch)		( (('0' <= (ch)) && ((ch) <= '9')) || (('A' <= (ch)) && ((ch) <= 'F')) )
#define HexDigitValue(ch)	( (((ch) - '0') < 10) ? ((ch) - '0') : ((ch) - 'A' + 10) )

static const char * kTenSpaces = "          ";
#define OutProcPadding(pad)	{ size_t padLen = (pad); 												\
							  for ( ; padLen >= 10; padLen -= 10 ) OutProcNChars ( kTenSpaces, 10 );	\
							  for ( ; padLen > 0; padLen -= 1 ) OutProcNChars ( " ", 1 ); }


#define OutProcNewline()	{ status = (*outProc) ( refCon, "\n", 1 );  if ( status != 0 ) goto EXIT; }

#define OutProcNChars(p,n)	{ status = (*outProc) ( refCon, (p), (n) );  if ( status != 0 ) goto EXIT; }

#define OutProcLiteral(lit)	{ status = (*outProc) ( refCon, (lit), strlen(lit) );  if ( status != 0 ) goto EXIT; }

#define OutProcString(str)	{ status = (*outProc) ( refCon, (str).c_str(), (str).size() );  if ( status != 0 ) goto EXIT; }

#define OutProcULong(num)	{ snprintf ( buffer, sizeof(buffer), "%lu", (num) ); /* AUDIT: Using sizeof for snprintf length is safe */ \
							  status = (*outProc) ( refCon, buffer, strlen(buffer) );  if ( status != 0 ) goto EXIT; }

#define OutProcHexInt(num)	{ snprintf ( buffer, sizeof(buffer), "%lX", (num) ); /* AUDIT: Using sizeof for snprintf length is safe */	\
							  status = (*outProc) ( refCon, buffer, strlen(buffer) );  if ( status != 0 ) goto EXIT; }

static const char * kIndent = "   ";
#define OutProcIndent(lev)	{ for ( size_t i = 0; i < (lev); ++i ) OutProcNChars ( kIndent, 3 ); }


// -------------------------------------------------------------------------------------------------
// DumpStringMap
// -------------

static XMP_Status
DumpStringMap ( const XMP_StringMap & map, XMP_StringPtr label, XMP_TextOutputProc outProc, void * refCon )
{
	XMP_Status			status;
	XMP_cStringMapPos	currPos;
	XMP_cStringMapPos	endPos = map.end();
	
	size_t maxLen = 0;
	for ( currPos = map.begin(); currPos != endPos; ++currPos ) {
		size_t currLen = currPos->first.size();
		if ( currLen > maxLen ) maxLen = currLen;
	}
	
	OutProcNewline();
	OutProcLiteral ( label );
	OutProcNewline();
	
	for ( currPos = map.begin(); currPos != endPos; ++currPos ) {
		OutProcNChars ( "  ", 2 );
		OutProcString ( currPos->first );
		OutProcPadding ( maxLen - currPos->first.size() );
		OutProcNChars ( " => ", 4 );
		OutProcString ( currPos->second );
		OutProcNewline();
	}
	
EXIT:
	return status;
	
}	// DumpStringMap


// -------------------------------------------------------------------------------------------------
// DumpNodeOptions
// ---------------

static XMP_Status
DumpNodeOptions	( XMP_OptionBits	 options,
				  XMP_TextOutputProc outProc,
				  void *			 refCon )
{
	XMP_Status	status;
	char		buffer [32];	// Decimal of a 64 bit int is at most about 20 digits.

	static const char * optNames[] = { " schema",		// 0x8000_0000
									   " ?30",
									   " ?29",
									   " -COMMAS-",
									   " ?27",			// 0x0800_0000
									   " ?26",
									   " ?25",
									   " ?24",
									   " ?23",			// 0x0080_0000
									   " isStale",
									   " isDerived",
									   " isStable",
									   " ?19",			// 0x0008_0000
									   " isInternal",
									   " hasAliases",
									   " isAlias",
									   " -AFTER-",		// 0x0000_8000
									   " -BEFORE-",
									   " isCompact",
									   " isLangAlt",
									   " isAlt",		// 0x0000_0800
									   " isOrdered",
									   " isArray",
									   " isStruct",
									   " hasType",		// 0x0000_0080
									   " hasLang",
									   " isQual",
									   " hasQual",
									   " ?3",			// 0x0000_0008
									   " ?2",
									   " URI",
									   " ?0" };

	if ( options == 0 ) {
	
		OutProcNChars ( "(0x0)", 5 );
	
	} else {
	
		OutProcNChars ( "(0x", 3 );
		OutProcHexInt ( options );
		OutProcNChars ( " :", 2 );

		XMP_OptionBits mask = 0x80000000;
		for ( int b = 0; b < 32; ++b ) {
			if ( options & mask ) OutProcLiteral ( optNames[b] );
			mask = mask >> 1;
		}
		OutProcNChars ( ")", 1 );
	
	}
	
EXIT:
	return status;

}	// DumpNodeOptions


// -------------------------------------------------------------------------------------------------
// DumpPropertyTree
// ----------------

// *** Extract the validation code into a separate routine to call on exit in debug builds.

static XMP_Status
DumpPropertyTree ( const XMP_Node *	  currNode,
				   int				  indent,
				   size_t			  itemIndex,
				   XMP_TextOutputProc outProc,
				   void *			  refCon )
{
	XMP_Status	status;
	char		buffer [32];	// Decimal of a 64 bit int is at most about 20 digits.

	OutProcIndent ( (size_t)indent );
	if ( itemIndex == 0 ) {
		if ( currNode->options & kXMP_PropIsQualifier ) OutProcNChars ( "? ", 2 );
		OutProcString ( currNode->name );
	} else {
		OutProcNChars ( "[", 1 );
		OutProcULong ( static_cast<unsigned long>(itemIndex) );
		OutProcNChars ( "]", 1 );
	}

	if ( ! (currNode->options & kXMP_PropCompositeMask) ) {
		OutProcNChars ( " = \"", 4 );
		OutProcString ( currNode->value );
		OutProcNChars ( "\"", 1 );
	}

	if ( currNode->options != 0 ) {
		OutProcNChars ( "  ", 2 );
		status = DumpNodeOptions ( currNode->options, outProc, refCon );
		if ( status != 0 ) goto EXIT;
	}
	
	if ( currNode->options & kXMP_PropHasLang ) {
		if ( currNode->qualifiers.empty() || (currNode->qualifiers[0]->name != "xml:lang") ) {
			OutProcLiteral ( "  ** bad lang flag **" );
		}
	}
	// *** Check rdf:type also.

	if ( ! (currNode->options & kXMP_PropCompositeMask) ) {
		if ( ! currNode->children.empty() ) OutProcLiteral ( "  ** bad children **" );
	} else if ( currNode->options & kXMP_PropValueIsArray ) {
		if ( currNode->options & kXMP_PropValueIsStruct ) OutProcLiteral ( "  ** bad comp flags **" );
	} else if ( (currNode->options & kXMP_PropCompositeMask) != kXMP_PropValueIsStruct ) {
		OutProcLiteral ( "  ** bad comp flags **" );
	}

	#if 0	// *** XMP_DebugBuild
		if ( (currNode->_namePtr != currNode->name.c_str()) ||
			 (currNode->_valuePtr != currNode->value.c_str()) ) OutProcLiteral ( "  ** bad debug string **" );
	#endif
	
	OutProcNewline();
	
	for ( size_t qualNum = 0, qualLim = currNode->qualifiers.size(); qualNum < qualLim; ++qualNum ) {

		const XMP_Node * currQual = currNode->qualifiers[qualNum];

		if ( currQual->parent != currNode ) OutProcLiteral ( "** bad parent link => " );
		if ( currQual->name == kXMP_ArrayItemName ) OutProcLiteral ( "** bad qual name => " );
		if ( ! (currQual->options & kXMP_PropIsQualifier) ) OutProcLiteral ( "** bad qual flag => " );
		if ( currQual->name == "xml:lang" ) {
			if ( (qualNum != 0) || (! (currNode->options & kXMP_PropHasLang)) ) OutProcLiteral ( "** bad lang qual => " );
		}

		status = DumpPropertyTree ( currQual, indent+2, 0, outProc, refCon );
		if ( status != 0 ) goto EXIT;

	}

	for ( size_t childNum = 0, childLim = currNode->children.size(); childNum < childLim; ++childNum ) {

		const XMP_Node * currChild = currNode->children[childNum];

		if ( currChild->parent != currNode ) OutProcLiteral ( "** bad parent link => " );
		if ( currChild->options & kXMP_PropIsQualifier ) OutProcLiteral ( "** bad qual flag => " );
	
		if ( currNode->options & kXMP_PropValueIsArray ) {
			itemIndex = childNum+1;
			if ( currChild->name != kXMP_ArrayItemName ) OutProcLiteral ( "** bad item name => " );
		} else {
			itemIndex = 0;
			if ( currChild->name == kXMP_ArrayItemName ) OutProcLiteral ( "** bad field name => " );
		}

		status = DumpPropertyTree ( currChild, indent+1, itemIndex, outProc, refCon );
		if ( status != 0 ) goto EXIT;

	}
	
EXIT:
	return status;

}	// DumpPropertyTree


// -------------------------------------------------------------------------------------------------
// DumpXMLTree
// -----------

#if DumpXMLParseTree

static inline void PutHexByte ( FILE * log, unsigned char ch )
{

	fprintf ( log, "\\x" );
	if ( ch < 0x10 ) {
		fprintf ( log, "%c", kHexDigits[ch] );
	} else {
		fprintf ( log, "%c%c", kHexDigits[ch>>4], kHexDigits[ch&0xF] );
	}
	
}	// PutHexByte

// -------------------------------------------------------------------------------------------------

static void PutClearString ( FILE * log, const std::string & str )
{

	for ( size_t i = 0; i != str.size(); ++i ) {
		unsigned char ch = str[i];
		if ( (0x20 <= ch) && (ch <= 0x7F) ) {
			fprintf ( log, "%c", ch );
		} else {
			PutHexByte ( log, ch );
		}
	}

}	// PutClearString

// -------------------------------------------------------------------------------------------------

static void DumpXMLTree ( FILE * log, const XML_Node & node, int indent )
{
	size_t i;
	
	#if 0	// *** XMP_DebugBuild
		if ( (node._namePtr != node.name.c_str()) ||
			 (node._valuePtr != node.value.c_str()) ) fprintf ( log, "*** bad debug string ***\n" );
	#endif
	
	for ( i = 0; i != (size_t)indent; ++i ) fprintf ( log, "  " );
	
	switch ( node.kind ) {
	
		case kRootNode :
			fprintf ( log, "\nStart of XML tree dump\n\n" );
			if ( (indent != 0) || (! node.attrs.empty()) ||
				 (! node.ns.empty()) || (! node.name.empty()) || (!node.value.empty()) ) fprintf ( log, " ** invalid root ** \n" );
			for ( i = 0; i < node.children.size(); ++i ) {
				XMP_Uns8 kind = node.children[i]->kind;
				if ( (kind == kRootNode) || (kind == kAttrNode) ) fprintf ( log, " ** invalid child ** \n" );
				DumpXMLTree ( log, *node.children[i], indent+1 );
			}
			fprintf ( log, "\nEnd of XML tree dump\n" );
			break;
	
		case kElemNode :
			fprintf ( log, "Elem %s", node.name.c_str() );
			if ( indent == 0 ) fprintf ( log, " ** invalid elem ** " );
			if ( ! node.ns.empty() ) fprintf ( log, " @ %s", node.ns.c_str() );
			fprintf ( log, "\n" );
			for ( i = 0; i < node.attrs.size(); ++i ) {
				XMP_Uns8 kind = node.attrs[i]->kind;
				if ( kind != kAttrNode ) fprintf ( log, " ** invalid attr ** \n" );
				DumpXMLTree ( log, *node.attrs[i], indent+2 );
			}
			for ( i = 0; i < node.children.size(); ++i ) {
				XMP_Uns8 kind = node.children[i]->kind;
				if ( (kind == kRootNode) || (kind == kAttrNode) ) fprintf ( log, " ** invalid child ** \n" );
				DumpXMLTree ( log, *node.children[i], indent+1 );
			}
			break;
	
		case kAttrNode :
			fprintf ( log, "Attr %s", node.name.c_str() );
			if ( (indent == 0) || node.name.empty() || (! node.attrs.empty()) || (! node.children.empty()) ) fprintf ( log, " ** invalid attr ** " );
			fprintf ( log, " = \"" );
			PutClearString ( log, node.value );
			fprintf ( log, "\"" );
			if ( ! node.ns.empty() ) fprintf ( log, " @ %s", node.ns.c_str() );
			fprintf ( log, "\n" );
			break;
	
		case kCDataNode :
			if ( (indent == 0) || (! node.ns.empty()) || (! node.name.empty()) ||
				 (! node.attrs.empty()) || (! node.children.empty()) ) fprintf ( log, " ** invalid cdata ** \n" );
			fprintf ( log, "\"" );
			PutClearString ( log, node.value );
			fprintf ( log, "\"\n" );
			break;
	
		case kPINode :
			fprintf ( log, "PI %s", node.name.c_str() );
			if ( (indent == 0) || node.name.empty() || (! node.children.empty()) ) fprintf ( log, " ** invalid pi ** \n" );
			if ( ! node.value.empty() ) {
				fprintf ( log, " <? " );
				PutClearString ( log, node.value );
				fprintf ( log, " ?>" );
			}
			fprintf ( log, "\n" );
			break;
	
	}

}	// DumpXMLTree

#endif	// DumpXMLParseTree


// =================================================================================================
// Constructors
// ============


XMPMeta::XMPMeta() : clientRefs(0), prevTkVer(0), tree(XMP_Node(0,"",0)), xmlParser(0)
{
	// Nothing more to do, clientRefs is incremented in wrapper.
	#if XMP_TraceCTorDTor
		printf ( "Default construct XMPMeta @ %.8X\n", this );
	#endif
}	// XMPMeta

// -------------------------------------------------------------------------------------------------

XMPMeta::~XMPMeta() RELEASE_NO_THROW
{
	#if XMP_TraceCTorDTor
		printf ( "Destruct XMPMeta @ %.8X\n", this );
	#endif

	XMP_Assert ( this->clientRefs <= 0 );
	if ( xmlParser != 0 ) delete ( xmlParser );
	xmlParser = 0;

}	// ~XMPMeta


// =================================================================================================
// Class Static Functions
// ======================
//
//
// =================================================================================================

// -------------------------------------------------------------------------------------------------
// GetVersionInfo
// --------------

/* class-static */ void
XMPMeta::GetVersionInfo ( XMP_VersionInfo * info )
{

	memset ( info, 0, sizeof(XMP_VersionInfo) );
	
	info->major   = XMP_API_VERSION_MAJOR;
	info->minor   = XMP_API_VERSION_MINOR;
	info->micro   = XMP_API_VERSION_MICRO;
	info->isDebug = kXMPCore_DebugFlag;
	info->flags   = 0;	// ! None defined yet.
	info->message = kXMPCore_VersionMessage;
	
}	// GetVersionInfo

// -------------------------------------------------------------------------------------------------
// Initialize
// ----------

/* class-static */ bool
XMPMeta::Initialize()
{
	// Allocate and initialize static objects.
	
	++sXMP_InitCount;
	if ( sXMP_InitCount > 1 ) return true;
	
	#if TraceXMPCalls
		// xmpOut = fopen ( "xmp.out", "w" );	// Coordinate with client glue in WXMP_Common.hpp
		fprintf ( xmpOut, "XMP initializing\n" ); fflush ( xmpOut );
	#endif
	
	sExceptionMessage = new XMP_VarString();
	XMP_InitMutex ( &sXMPCoreLock );
    sOutputNS  = new XMP_VarString;
    sOutputStr = new XMP_VarString;

	xdefaultName = new XMP_VarString ( "x-default" );
	
	sNamespaceURIToPrefixMap	= new XMP_StringMap;
	sNamespacePrefixToURIMap	= new XMP_StringMap;
	sRegisteredAliasMap			= new XMP_AliasMap;
	
	InitializeUnicodeConversions();
	
	// Register standard namespaces and aliases.
	
	XMP_StringPtr	voidPtr;
	XMP_StringLen	voidLen;
	
	(void) RegisterNamespace ( kXMP_NS_XML, "xml", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_RDF, "rdf", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_DC, "dc", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_XMP, "xap", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDF, "pdf", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_Photoshop, "photoshop", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PSAlbum, "album", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_EXIF, "exif", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_EXIF_Aux, "aux", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_TIFF, "tiff", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PNG, "png", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_JPEG, "jpeg", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_JP2K, "jp2k", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_CameraRaw, "crs", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_ASF, "asf", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_WAV, "wav", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_AdobeStockPhoto, "bmsp", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_XMP_Rights, "xapRights", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_MM, "xapMM", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_BJ, "xapBJ", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_Note, "xmpNote", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_DM, "xmpDM", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_Text, "xapT", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_PagedFile, "xapTPg", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_Graphics, "xapG", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_Image, "xapGImg", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_XMP_Font, "stFnt", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_Dimensions, "stDim", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_ResourceEvent, "stEvt", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_ResourceRef, "stRef", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_ST_Version, "stVer", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_ST_Job, "stJob", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_XMP_ManifestItem, "stMfs", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_XMP_IdentifierQual, "xmpidq", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_IPTCCore, "Iptc4xmpCore", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_PDFA_Schema, "pdfaSchema", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFA_Property, "pdfaProperty", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFA_Type, "pdfaType", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFA_Field, "pdfaField", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFA_ID, "pdfaid", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFA_Extension, "pdfaExtension", &voidPtr, &voidLen );

	(void) RegisterNamespace ( kXMP_NS_PDFX, "pdfx", &voidPtr, &voidLen );
	(void) RegisterNamespace ( kXMP_NS_PDFX_ID, "pdfxid", &voidPtr, &voidLen );
	
	(void) RegisterNamespace ( "adobe:ns:meta/", "x", &voidPtr, &voidLen );
	(void) RegisterNamespace ( "http://ns.adobe.com/iX/1.0/", "iX", &voidPtr, &voidLen );

// 06-Oct-07, ahu: Do not use aliases. They result in unexpected behaviour.
//	XMPMeta::RegisterStandardAliases ( "" );
	
	// Initialize the other core classes.
	
	if ( ! XMPIterator::Initialize() ) XMP_Throw ( "Failure from XMPIterator::Initialize", kXMPErr_InternalFailure );
	if ( ! XMPUtils::Initialize() ) XMP_Throw ( "Failure from XMPUtils::Initialize", kXMPErr_InternalFailure );

	// Do miscelaneous semantic checks of types and arithmetic.

	XMP_Assert ( sizeof(XMP_Int8) == 1 );
	XMP_Assert ( sizeof(XMP_Int16) == 2 );
	XMP_Assert ( sizeof(XMP_Int32) == 4 );
	XMP_Assert ( sizeof(XMP_Int64) == 8 );
	XMP_Assert ( sizeof(XMP_Uns8) == 1 );
	XMP_Assert ( sizeof(XMP_Uns16) == 2 );
	XMP_Assert ( sizeof(XMP_Uns32) == 4 );
	XMP_Assert ( sizeof(XMP_Uns64) == 8 );
	
	XMP_Assert ( sizeof(XMP_OptionBits) == 4 );	// Check that option masking work on all 32 bits.
	XMP_OptionBits flag = ~0UL;
	XMP_Assert ( flag == (XMP_OptionBits)(-1L) );
	XMP_Assert ( (flag ^ kXMP_PropHasLang) == 0xFFFFFFBFUL );
	XMP_Assert ( (flag & ~kXMP_PropHasLang) == 0xFFFFFFBFUL );
	
	XMP_OptionBits opt1 = 0;	// Check the general option bit macros.
	XMP_OptionBits opt2 = ~0UL;
	XMP_SetOption ( opt1, kXMP_PropValueIsArray );
	XMP_ClearOption ( opt2, kXMP_PropValueIsArray );
	XMP_Assert ( opt1 == ~opt2 );
	XMP_Assert ( XMP_TestOption ( opt1, kXMP_PropValueIsArray ) );
	XMP_Assert ( ! XMP_TestOption ( opt2, kXMP_PropValueIsArray ) );
	
	XMP_Assert ( XMP_PropIsSimple ( ~kXMP_PropCompositeMask ) );	// Check the special option bit macros.
	XMP_Assert ( ! XMP_PropIsSimple ( kXMP_PropValueIsStruct ) );
	XMP_Assert ( ! XMP_PropIsSimple ( kXMP_PropValueIsArray ) );

	XMP_Assert ( XMP_PropIsStruct ( kXMP_PropValueIsStruct ) );
	XMP_Assert ( XMP_PropIsArray  ( kXMP_PropValueIsArray ) );
	XMP_Assert ( ! XMP_PropIsStruct ( ~kXMP_PropValueIsStruct ) );
	XMP_Assert ( ! XMP_PropIsArray  ( ~kXMP_PropValueIsArray ) );
	
	XMP_Assert ( XMP_ArrayIsUnordered ( ~kXMP_PropArrayIsOrdered ) );
	XMP_Assert ( XMP_ArrayIsOrdered   ( kXMP_PropArrayIsOrdered ) );
	XMP_Assert ( XMP_ArrayIsAlternate ( kXMP_PropArrayIsAlternate ) );
	XMP_Assert ( XMP_ArrayIsAltText   ( kXMP_PropArrayIsAltText ) );
	XMP_Assert ( ! XMP_ArrayIsUnordered ( kXMP_PropArrayIsOrdered ) );
	XMP_Assert ( ! XMP_ArrayIsOrdered   ( ~kXMP_PropArrayIsOrdered ) );
	XMP_Assert ( ! XMP_ArrayIsAlternate ( ~kXMP_PropArrayIsAlternate ) );
	XMP_Assert ( ! XMP_ArrayIsAltText   ( ~kXMP_PropArrayIsAltText ) );
	
	XMP_Assert ( XMP_PropHasQualifiers ( kXMP_PropHasQualifiers ) );
	XMP_Assert ( XMP_PropIsQualifier   ( kXMP_PropIsQualifier ) );
	XMP_Assert ( XMP_PropHasLang       ( kXMP_PropHasLang ) );
	XMP_Assert ( ! XMP_PropHasQualifiers ( ~kXMP_PropHasQualifiers ) );
	XMP_Assert ( ! XMP_PropIsQualifier   ( ~kXMP_PropIsQualifier ) );
	XMP_Assert ( ! XMP_PropHasLang       ( ~kXMP_PropHasLang ) );
	
	XMP_Assert ( XMP_NodeIsSchema ( kXMP_SchemaNode ) );
	XMP_Assert ( XMP_PropIsAlias  ( kXMP_PropIsAlias ) );
	XMP_Assert ( ! XMP_NodeIsSchema ( ~kXMP_SchemaNode ) );
	XMP_Assert ( ! XMP_PropIsAlias  ( ~kXMP_PropIsAlias ) );
	
	#if 0	// Generally off, enable to hand check generated code.
		extern XMP_OptionBits opt3, opt4;
		if ( XMP_TestOption ( opt3, kXMP_PropValueIsArray ) ) opt4 = opt3;
		if ( ! XMP_TestOption ( opt3, kXMP_PropValueIsStruct ) ) opt4 = opt3;
		static bool ok1 = XMP_TestOption ( opt4, kXMP_PropValueIsArray );
		static bool ok2 = ! XMP_TestOption ( opt4, kXMP_PropValueIsStruct );
	#endif
	
	// Make sure the embedded info strings are referenced and kept.
	if ( (kXMPCore_EmbeddedVersion[0] == 0) || (kXMPCore_EmbeddedCopyright[0] == 0) ) return false;
	return true;

}	// Initialize


// -------------------------------------------------------------------------------------------------
// Terminate
// ---------

#define EliminateGlobal(g) delete ( g ); g = 0

/* class-static */ void
XMPMeta::Terminate() RELEASE_NO_THROW
{
	--sXMP_InitCount;
	if ( sXMP_InitCount > 0 ) return;

	#if TraceXMPCalls
		fprintf ( xmpOut, "XMP terminating\n" ); fflush ( xmpOut );
		// fclose ( xmpOut );	// Coordinate with fopen in XMPMeta::Initialize.
	#endif
	
	XMPIterator::Terminate();
	XMPUtils::Terminate();

	EliminateGlobal ( sNamespaceURIToPrefixMap );
	EliminateGlobal ( sNamespacePrefixToURIMap );
	EliminateGlobal ( sRegisteredAliasMap );
    
    EliminateGlobal ( xdefaultName );
    EliminateGlobal ( sOutputNS );
    EliminateGlobal ( sOutputStr );
	EliminateGlobal ( sExceptionMessage );

	XMP_TermMutex ( sXMPCoreLock );

}	// Terminate


// -------------------------------------------------------------------------------------------------
// Unlock
// ------

/* class-static */ void
XMPMeta::Unlock ( XMP_OptionBits options )
{
	options = options;	// Avoid unused parameter warning.	// *** Need IgnoreParam macro.

    #if TraceXMPLocking
    	fprintf ( xmpOut, "  Unlocking XMP toolkit, count = %d\n", sLockCount ); fflush ( xmpOut );
	#endif
    --sLockCount;
    XMP_Assert ( sLockCount == 0 );
	XMP_ExitCriticalRegion ( sXMPCoreLock );

}	// Unlock


// -------------------------------------------------------------------------------------------------
// UnlockObject
// ------------

void
XMPMeta::UnlockObject ( XMP_OptionBits options ) const
{
	options = options;	// Avoid unused parameter warning.

	XMPMeta::Unlock ( 0 );

}	// UnlockObject


// -------------------------------------------------------------------------------------------------
// DumpNamespaces
// --------------
//
// Dump the prefix to URI map (easier to read) and verify that both are consistent and legit.

// *** Should put checks in a separate routine for regular calling in debug builds.

/* class-static */ XMP_Status
XMPMeta::DumpNamespaces ( XMP_TextOutputProc outProc,
                          void *             refCon )
{
	XMP_Assert ( outProc != 0 );	// ! Enforced by wrapper.
	XMP_Status status;
	
	XMP_StringMapPos p2uEnd = sNamespacePrefixToURIMap->end();	// ! Move up to avoid gcc complaints.
	XMP_StringMapPos u2pEnd = sNamespaceURIToPrefixMap->end();
	
	status = DumpStringMap ( *sNamespacePrefixToURIMap, "Dumping namespace prefix to URI map", outProc, refCon );
	if ( status != 0 ) goto EXIT;
	
	if ( sNamespacePrefixToURIMap->size() != sNamespaceURIToPrefixMap->size() ) {
		OutProcLiteral ( "** bad namespace map sizes **" );
		XMP_Throw ( "Fatal namespace map problem", kXMPErr_InternalFailure );
	}
	
	for ( XMP_StringMapPos nsLeft = sNamespacePrefixToURIMap->begin(); nsLeft != p2uEnd; ++nsLeft ) {

		XMP_StringMapPos nsOther = sNamespaceURIToPrefixMap->find ( nsLeft->second );
		if ( (nsOther == u2pEnd) || (nsLeft != sNamespacePrefixToURIMap->find ( nsOther->second )) ) {
			OutProcLiteral ( "  ** bad namespace URI **  " );
			OutProcString ( nsLeft->second );
			goto FAILURE;
		}
		
		for ( XMP_StringMapPos nsRight = nsLeft; nsRight != p2uEnd; ++nsRight ) {
			if ( nsRight == nsLeft ) continue;	// ! Can't start at nsLeft+1, no operator+!
			if ( nsLeft->second == nsRight->second ) {
				OutProcLiteral ( "  ** duplicate namespace URI **  " );
				OutProcString ( nsLeft->second );
				goto FAILURE;
			}
		}

	}
	
	for ( XMP_StringMapPos nsLeft = sNamespaceURIToPrefixMap->begin(); nsLeft != u2pEnd; ++nsLeft ) {

		XMP_StringMapPos nsOther = sNamespacePrefixToURIMap->find ( nsLeft->second );
		if ( (nsOther == p2uEnd) || (nsLeft != sNamespaceURIToPrefixMap->find ( nsOther->second )) ) {
			OutProcLiteral ( "  ** bad namespace prefix **  " );
			OutProcString ( nsLeft->second );
			goto FAILURE;
		}

		for ( XMP_StringMapPos nsRight = nsLeft; nsRight != u2pEnd; ++nsRight ) {
			if ( nsRight == nsLeft ) continue;	// ! Can't start at nsLeft+1, no operator+!
			if ( nsLeft->second == nsRight->second ) {
				OutProcLiteral ( "  ** duplicate namespace prefix **  " );
				OutProcString ( nsLeft->second );
				goto FAILURE;
			}
		}

	}

EXIT:
	return status;

FAILURE:
	OutProcNewline();
	(void) DumpStringMap ( *sNamespaceURIToPrefixMap, "Dumping namespace URI to prefix map", outProc, refCon );
	XMP_Throw ( "Fatal namespace map problem", kXMPErr_InternalFailure );
	return 0;
	
}	// DumpNamespaces


// -------------------------------------------------------------------------------------------------
// DumpAliases
// -----------

/* class-static */ XMP_Status
XMPMeta::DumpAliases ( XMP_TextOutputProc outProc,
                       void *             refCon )
{
	XMP_Assert ( outProc != 0 );	// ! Enforced by wrapper.
	XMP_Status status;

	XMP_Assert ( sRegisteredAliasMap != 0 );

	XMP_cAliasMapPos aliasPos;
	XMP_cAliasMapPos aliasEnd = sRegisteredAliasMap->end();
	
	size_t maxLen = 0;
	for ( aliasPos = sRegisteredAliasMap->begin(); aliasPos != aliasEnd; ++aliasPos ) {
		size_t currLen = aliasPos->first.size();
		if ( currLen > maxLen ) maxLen = currLen;
	}
	
	OutProcLiteral ( "Dumping alias name to actual path map" );
	OutProcNewline();
		
	for ( aliasPos = sRegisteredAliasMap->begin(); aliasPos != aliasEnd; ++aliasPos ) {

		OutProcNChars ( "   ", 3 );
		OutProcString ( aliasPos->first );
		OutProcPadding ( maxLen - aliasPos->first.size() );
		OutProcNChars ( " => ", 4 );

		size_t actualPathSize = aliasPos->second.size();
		for ( size_t stepNum = 1; stepNum < actualPathSize; ++stepNum ) OutProcString ( aliasPos->second[stepNum].step );

		XMP_OptionBits arrayForm = aliasPos->second[1].options & kXMP_PropArrayFormMask;

		if ( arrayForm == 0 ) {
			if ( actualPathSize != 2 ) OutProcLiteral ( "  ** bad actual path **" );
		} else {
			OutProcNChars ( "  ", 2 );
			DumpNodeOptions ( arrayForm, outProc, refCon );
			if ( ! (arrayForm & kXMP_PropValueIsArray) ) OutProcLiteral ( "  ** bad array form **" );
			if ( actualPathSize != 3 ) OutProcLiteral ( "  ** bad actual path **" );
		}

		if ( aliasPos->second[0].options != kXMP_SchemaNode ) OutProcLiteral ( "  ** bad schema form **" );

		OutProcNewline();

	}
	
EXIT:
	return status;
	
}	// DumpAliases


// -------------------------------------------------------------------------------------------------
// GetGlobalOptions
// ----------------

/* class-static */ XMP_OptionBits
XMPMeta::GetGlobalOptions()
{
	XMP_OptionBits	options = 0;
	
	return options;
	
}	// GetGlobalOptions


// -------------------------------------------------------------------------------------------------
// SetGlobalOptions
// ----------------

/* class-static */ void
XMPMeta::SetGlobalOptions ( XMP_OptionBits options )
{

	XMP_Throw ( "Unimplemented method XMPMeta::SetGlobalOptions", kXMPErr_Unimplemented );
		void * p; p = &options;	// Avoid unused param warnings.

}	// SetGlobalOptions


// -------------------------------------------------------------------------------------------------
// RegisterNamespace
// -----------------

/* class-static */ bool
XMPMeta::RegisterNamespace ( XMP_StringPtr	 namespaceURI,
							 XMP_StringPtr	 suggestedPrefix,
							 XMP_StringPtr * registeredPrefix,
							 XMP_StringLen * prefixSize )
{
	bool	prefixMatches	= false;
	
	XMP_Assert ( (registeredPrefix != 0) && (prefixSize != 0) );		// ! Enforced by wrapper.
	if ( (*namespaceURI == 0) || (*suggestedPrefix == 0) ) {
		XMP_Throw ( "Empty namespace URI or prefix", kXMPErr_BadParam );
	}
	
	XMP_VarString	nsURI ( namespaceURI );
	XMP_VarString	suggPrefix ( suggestedPrefix );
	if ( suggPrefix[suggPrefix.size()-1] != ':' ) suggPrefix += ':';
	VerifySimpleXMLName ( suggestedPrefix, suggestedPrefix+suggPrefix.size()-1 );	// Exclude the colon.
	
	XMP_StringMapPos	uriPos	= sNamespaceURIToPrefixMap->find ( nsURI );
	
	if ( uriPos == sNamespaceURIToPrefixMap->end() ) {
		
		// The URI is not yet registered, make sure we use a unique prefix.
		
		XMP_VarString	uniqPrefix ( suggPrefix );
		int		suffix = 0;
		char	buffer [32];

		while ( true ) {
			if ( sNamespacePrefixToURIMap->find ( uniqPrefix ) == sNamespacePrefixToURIMap->end() ) break;
			++suffix;
			snprintf ( buffer, sizeof(buffer), "_%d_:", suffix );	// AUDIT: Using sizeof for snprintf length is safe.
			uniqPrefix = suggPrefix;
			uniqPrefix.erase ( uniqPrefix.size()-1 );	// ! Remove the trailing ':'.
			uniqPrefix += buffer;
		}
		
		// Add the new namespace to both maps.
		
		XMP_StringPair	newNS ( nsURI, uniqPrefix );
		uriPos = sNamespaceURIToPrefixMap->insert ( sNamespaceURIToPrefixMap->end(), newNS );
		
		newNS.first.swap ( newNS.second );
		(void) sNamespacePrefixToURIMap->insert ( sNamespacePrefixToURIMap->end(), newNS );

	}
	
	// Return the actual prefix and see if it matches the suggested prefix.
	
	*registeredPrefix = uriPos->second.c_str();
	*prefixSize = uriPos->second.size();
	
	prefixMatches = ( uriPos->second == suggPrefix );
	return prefixMatches;
	
}	// RegisterNamespace


// -------------------------------------------------------------------------------------------------
// GetNamespacePrefix
// ------------------

/* class-static */ bool
XMPMeta::GetNamespacePrefix	( XMP_StringPtr   namespaceURI,
							  XMP_StringPtr * namespacePrefix,
							  XMP_StringLen * prefixSize )
{
	bool	found	= false;
	
	XMP_Assert ( *namespaceURI != 0 ); 	// ! Enforced by wrapper.
	XMP_Assert ( (namespacePrefix != 0) && (prefixSize != 0) );	// ! Enforced by wrapper.

	XMP_VarString    nsURI ( namespaceURI );
	XMP_StringMapPos uriPos	= sNamespaceURIToPrefixMap->find ( nsURI );
	
	if ( uriPos != sNamespaceURIToPrefixMap->end() ) {
		*namespacePrefix = uriPos->second.c_str();
		*prefixSize = uriPos->second.size();
		found = true;
	}
	
	return found;
	
}	// GetNamespacePrefix


// -------------------------------------------------------------------------------------------------
// GetNamespaceURI
// ---------------

/* class-static */ bool
XMPMeta::GetNamespaceURI ( XMP_StringPtr   namespacePrefix,
						   XMP_StringPtr * namespaceURI,
						   XMP_StringLen * uriSize )
{
	bool	found	= false;
	
	XMP_Assert ( *namespacePrefix != 0 ); 	// ! Enforced by wrapper.
	XMP_Assert ( (namespacePrefix != 0) && (namespaceURI != 0) );	// ! Enforced by wrapper.

	XMP_VarString nsPrefix ( namespacePrefix );
	if ( nsPrefix[nsPrefix.size()-1] != ':' ) nsPrefix += ':';
	
	XMP_StringMapPos prefixPos = sNamespacePrefixToURIMap->find ( nsPrefix );
	
	if ( prefixPos != sNamespacePrefixToURIMap->end() ) {
		*namespaceURI = prefixPos->second.c_str();
		*uriSize = prefixPos->second.size();
		found = true;
	}
	
	return found;
	
}	// GetNamespaceURI


// -------------------------------------------------------------------------------------------------
// DeleteNamespace
// ---------------

// *** Don't allow standard namespaces to be deleted.

/* class-static */ void
XMPMeta::DeleteNamespace ( XMP_StringPtr namespaceURI )
{
	namespaceURI = namespaceURI;	// Avoid unused parameter warning.
	XMP_Assert ( *namespaceURI != 0 ); 	// ! Enforced by wrapper.
	XMP_Throw ( "Unimplemented method XMPMeta::DeleteNamespace", kXMPErr_Unimplemented );   // *** #error "write me"

}	// DeleteNamespace


// -------------------------------------------------------------------------------------------------
// RegisterAlias
// -------------
//
// Allow 3 kinds of alias: 
//	TopProp => TopProp
//	TopProp => TopArray[1]
//	TopProp => TopArray[@xml:lang='x-default']
//
// A new alias can be made to something that is already aliased, as long as the net result is one of
// the legitimate forms. The new alias can already have aliases to it, also as long as result of
// adjusting all of the exiting aliases leaves them legal.
//
// ! The caller assumes all risk that new aliases do not invalidate existing XMPMeta objects. Any
// ! conflicts will result in later references throwing bad XPath exceptions.

/* class-static */ void
XMPMeta::RegisterAlias ( XMP_StringPtr 	aliasNS,
						 XMP_StringPtr	aliasProp,
						 XMP_StringPtr	actualNS,
						 XMP_StringPtr	actualProp,
						 XMP_OptionBits arrayForm )
{
	XMP_ExpandedXPath	expAlias, expActual;
	XMP_AliasMapPos		mapPos;
	XMP_ExpandedXPath *	regActual = 0;

	XMP_Assert ( (aliasNS != 0) && (aliasProp != 0) && (actualNS != 0) && (actualProp != 0) );	// Enforced by wrapper.
		
	// Expand the alias and actual names, make sure they are one of the basic 3 forms. When counting
	// the expanded XPath size remember that the schema URI is the first component. We don't have to
	// compare the schema URIs though, the (unique) prefix is part of the top property name.
	
	ExpandXPath ( aliasNS, aliasProp, &expAlias );
	ExpandXPath ( actualNS, actualProp, &expActual );
	if ( (expAlias.size() != 2) || (expActual.size() != 2) ) {
		XMP_Throw ( "Alias and actual property names must be simple", kXMPErr_BadXPath );
	}
	
	arrayForm = VerifySetOptions ( arrayForm, 0 );
	if ( arrayForm != 0 ) {
		if ( (arrayForm & ~kXMP_PropArrayFormMask) != 0 ) XMP_Throw ( "Only array form flags are allowed", kXMPErr_BadOptions );
		expActual[1].options |= arrayForm;	// Set the array form for the top level step.
		if ( ! (arrayForm & kXMP_PropArrayIsAltText) ) {
			expActual.push_back ( XPathStepInfo ( "[1]", kXMP_ArrayIndexStep ) );
		} else {
			expActual.push_back ( XPathStepInfo ( "[?xml:lang=\"x-default\"]", kXMP_QualSelectorStep ) );
		}
	}

	// See if there are any conflicts with existing aliases. A couple of the checks are easy. If the
	// alias is already aliased it is only OK to reregister an identical alias. If the actual is
	// already aliased to something else and the new chain is legal, just swap in the old base.

	mapPos = sRegisteredAliasMap->find ( expAlias[kRootPropStep].step );
	if ( mapPos != sRegisteredAliasMap->end() ) {

		// This alias is already registered to something, make sure it is the same something.

		regActual = &mapPos->second;
		if ( arrayForm != (mapPos->second[1].options & kXMP_PropArrayFormMask) ) {
			XMP_Throw ( "Mismatch with existing alias array form", kXMPErr_BadParam );
		}
		if ( expActual.size() != regActual->size() ) {
			XMP_Throw ( "Mismatch with existing actual path", kXMPErr_BadParam );
		}
		if ( expActual[kRootPropStep].step != (*regActual)[kRootPropStep].step ) {
			XMP_Throw ( "Mismatch with existing actual name", kXMPErr_BadParam );
		}
		if ( (expActual.size() == 3) && (expActual[kAliasIndexStep].step != (*regActual)[kAliasIndexStep].step) ) {
			XMP_Throw ( "Mismatch with existing actual array item", kXMPErr_BadParam );
		}
		return;

	}

	mapPos = sRegisteredAliasMap->find ( expActual[kRootPropStep].step );
	if ( mapPos != sRegisteredAliasMap->end() ) {
	
		// The actual is already aliased to something else.
		
		regActual = &mapPos->second;
		if ( expActual.size() == 2 ) {
			expActual = *regActual;	// TopProp => TopProp => anything : substitute the entire old base.
		} else if ( regActual->size() != 2 ) {
			XMP_Throw ( "Can't alias an array item to an array item", kXMPErr_BadParam );	// TopProp => TopArray[] => TopArray[] : nope.
		} else {
			expActual[kSchemaStep].step = (*regActual)[kSchemaStep].step;		// TopProp => TopArray[] => TopProp :
			expActual[kRootPropStep].step   = (*regActual)[kRootPropStep].step;	//	substitute the old base name.
		}
	
	}
	
	// Checking for existing aliases to this one is touchier. This involves updating the alias map,
	// which must not be done unless all of the changes are legal. So we need 2 loops, one to verify
	// that everything is OK, and one to make the changes. The bad case is:
	//	TopProp => TopArray[] => TopArray[]
	// In the valid cases we back substitute the new base.
	
	for ( mapPos = sRegisteredAliasMap->begin(); mapPos != sRegisteredAliasMap->end(); ++mapPos ) {
		regActual = &mapPos->second;
		if ( expAlias[kRootPropStep].step == (*regActual)[kRootPropStep].step ) {
			if ( (regActual->size() == 2) && (expAlias.size() == 2) ) {
				XMP_Throw ( "Can't alias an array item to an array item", kXMPErr_BadParam );
			}
		}
	}
	
	for ( mapPos = sRegisteredAliasMap->begin(); mapPos != sRegisteredAliasMap->end(); ++mapPos ) {
		regActual = &mapPos->second;
		if ( expAlias[kRootPropStep].step == (*regActual)[kRootPropStep].step ) {

			if ( regActual->size() == 1 ) {
				*regActual = expActual;	// TopProp => TopProp => anything : substitute the entire new base.
			} else {
				(*regActual)[kSchemaStep].step = expActual[kSchemaStep].step;	// TopProp => TopArray[] => TopProp :
				(*regActual)[kRootPropStep].step   = expActual[kRootPropStep].step;		//	substitute the new base name.
			}

		}
	}
	
	// Finally, all is OK to register the new alias.
	
	(void) sRegisteredAliasMap->insert ( XMP_AliasMap::value_type ( expAlias[kRootPropStep].step, expActual ) );

}	// RegisterAlias


// -------------------------------------------------------------------------------------------------
// ResolveAlias
// ------------

/* class-static */ bool
XMPMeta::ResolveAlias ( XMP_StringPtr	 aliasNS,
						XMP_StringPtr	 aliasProp,
						XMP_StringPtr *	 actualNS,
						XMP_StringLen *	 nsSize,
						XMP_StringPtr *	 actualProp,
						XMP_StringLen *	 propSize,
						XMP_OptionBits * arrayForm )
{
	XMP_Assert ( (aliasNS != 0) && (aliasProp != 0) );	// Enforced by wrapper.
	XMP_Assert ( (actualNS != 0) && (nsSize != 0) && (actualProp != 0) && (propSize != 0) && (arrayForm != 0) );	// Enforced by wrapper.
	
	// Expand the input path and look up the first component in the alias table. Return if not an alias.
	
	XMP_ExpandedXPath fullPath, minPath;
	ExpandXPath ( aliasNS, aliasProp, &fullPath );
	XMP_Assert ( fullPath.size() >= 2 );

	minPath.push_back ( fullPath[kSchemaStep] );
	minPath.push_back ( fullPath[kRootPropStep] );
	XMP_AliasMapPos mapPos = sRegisteredAliasMap->find ( minPath[kRootPropStep].step );
	if ( mapPos == sRegisteredAliasMap->end() ) return false;
	
	// Replace the alias portion of the full expanded path. Compose the output path string.
	
	const XMP_ExpandedXPath & actualPath = mapPos->second;
	
	fullPath[kSchemaStep] = actualPath[kSchemaStep];
	fullPath[kRootPropStep] = actualPath[kRootPropStep];
	if ( actualPath.size() > 2 ) {	// This is an alias to an array item.
		XMP_ExpandedXPathPos insertPos = fullPath.begin() + kAliasIndexStep;
		fullPath.insert ( insertPos, actualPath[kAliasIndexStep] );
	}	
	
	*sOutputNS = fullPath[kSchemaStep].step;
	*actualNS  = sOutputNS->c_str();
	*nsSize    = sOutputNS->size();
	
	ComposeXPath ( fullPath, sOutputStr );
	*actualProp = sOutputStr->c_str();
	*propSize   = sOutputStr->size();

	*arrayForm  = actualPath[kRootPropStep].options & kXMP_PropArrayFormMask;
	
	#if XMP_DebugBuild	// Test that the output string is valid and unchanged by round trip expand/compose.
		XMP_ExpandedXPath rtPath;
		ExpandXPath ( *actualNS, *actualProp, &rtPath );
		std::string rtString;
		ComposeXPath ( rtPath, &rtString );
		XMP_Assert ( rtString == *sOutputStr );
	#endif
	
	return true;
	
}	// ResolveAlias


// -------------------------------------------------------------------------------------------------
// DeleteAlias
// -----------

/* class-static */ void
XMPMeta::DeleteAlias ( XMP_StringPtr aliasNS,
					   XMP_StringPtr aliasProp )
{

	XMP_Assert ( (aliasNS != 0) && (aliasProp != 0) );	// Enforced by wrapper.
	XMP_Throw ( "Unimplemented method XMPMeta::DeleteAlias", kXMPErr_Unimplemented );   // *** #error "write me"
		void * p; p = &aliasNS; p = &aliasProp;	// Avoid unused param warnings.

}	// DeleteAlias


// -------------------------------------------------------------------------------------------------
// RegisterStandardAliases
// -----------------------

/* class-static */ void
XMPMeta::RegisterStandardAliases ( XMP_StringPtr schemaNS )
{
	XMP_Assert ( schemaNS != 0 );	// Enforced by wrapper.

	const bool doAll = (*schemaNS == 0);
	
	if ( doAll || XMP_LitMatch ( schemaNS, kXMP_NS_XMP ) ) {
		// Aliases from XMP to DC.
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Author",       kXMP_NS_DC, "creator", kXMP_PropArrayIsOrdered );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Authors",      kXMP_NS_DC, "creator", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Description",  kXMP_NS_DC, "description", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Format",       kXMP_NS_DC, "format", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Keywords",     kXMP_NS_DC, "subject", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Locale",       kXMP_NS_DC, "language", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP, "Title",        kXMP_NS_DC, "title", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_XMP_Rights, "Copyright", kXMP_NS_DC, "rights", 0 );
	}

	if ( doAll || XMP_LitMatch ( schemaNS, kXMP_NS_PDF ) ) {
		// Aliases from PDF to DC and XMP.
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "Author",       kXMP_NS_DC, "creator", kXMP_PropArrayIsOrdered );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "BaseURL",      kXMP_NS_XMP, "BaseURL", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "CreationDate", kXMP_NS_XMP, "CreateDate", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "Creator",      kXMP_NS_XMP, "CreatorTool", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "ModDate",      kXMP_NS_XMP, "ModifyDate", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "Subject",      kXMP_NS_DC,  "description", kXMP_PropArrayIsAltText );
	    XMPMeta::RegisterAlias ( kXMP_NS_PDF, "Title",        kXMP_NS_DC,  "title", kXMP_PropArrayIsAltText );
    }
    
	if ( doAll || XMP_LitMatch ( schemaNS, kXMP_NS_Photoshop ) ) {
	    // Aliases from PHOTOSHOP to DC and XMP.
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Author",       kXMP_NS_DC, "creator", kXMP_PropArrayIsOrdered );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Caption",      kXMP_NS_DC, "description", kXMP_PropArrayIsAltText );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Copyright",    kXMP_NS_DC, "rights", kXMP_PropArrayIsAltText );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Keywords",     kXMP_NS_DC, "subject", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Marked",       kXMP_NS_XMP_Rights, "Marked", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "Title",        kXMP_NS_DC, "title", kXMP_PropArrayIsAltText );
	    XMPMeta::RegisterAlias ( kXMP_NS_Photoshop, "WebStatement", kXMP_NS_XMP_Rights, "WebStatement", 0 );
    }
	    
	if ( doAll || XMP_LitMatch ( schemaNS, kXMP_NS_TIFF ) || XMP_LitMatch ( schemaNS, kXMP_NS_EXIF ) ) {
	    // Aliases from TIFF and EXIF to DC and XMP.
		XMPMeta::RegisterAlias ( kXMP_NS_TIFF, "Artist",           kXMP_NS_DC,  "creator", kXMP_PropArrayIsOrdered);
	    XMPMeta::RegisterAlias ( kXMP_NS_TIFF, "Copyright",        kXMP_NS_DC,  "rights", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_TIFF, "DateTime",         kXMP_NS_XMP, "ModifyDate", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_TIFF, "ImageDescription", kXMP_NS_DC,  "description", 0 );
	    XMPMeta::RegisterAlias ( kXMP_NS_TIFF, "Software",         kXMP_NS_XMP, "CreatorTool", 0 );
    }
    
	if ( doAll || XMP_LitMatch ( schemaNS, kXMP_NS_PNG ) ) {	// ! From Acrobat ImageCapture:
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "Author",           kXMP_NS_DC,  "creator", kXMP_PropArrayIsOrdered);
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "Copyright",        kXMP_NS_DC,  "rights", kXMP_PropArrayIsAltText);
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "CreationTime",     kXMP_NS_XMP, "CreateDate", 0 );
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "Description",      kXMP_NS_DC,  "description", kXMP_PropArrayIsAltText);
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "ModificationTime", kXMP_NS_XMP, "ModifyDate", 0 );
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "Software",         kXMP_NS_XMP, "CreatorTool", 0 );
		XMPMeta::RegisterAlias ( kXMP_NS_PNG, "Title",            kXMP_NS_DC,  "title", kXMP_PropArrayIsAltText);
	}

}	// RegisterStandardAliases


// =================================================================================================
// Class Methods
// =============
//
//
// =================================================================================================


// -------------------------------------------------------------------------------------------------
// DumpObject
// ----------

XMP_Status
XMPMeta::DumpObject ( XMP_TextOutputProc outProc,
                      void *             refCon ) const
{
	XMP_Assert ( outProc != 0 );	// ! Enforced by wrapper.
	XMP_Status status;
	
	OutProcLiteral ( "Dumping XMPMeta object \"" );
	OutProcString ( tree.name );
	OutProcNChars ( "\"  ", 3 );
	status = DumpNodeOptions ( tree.options, outProc, refCon );
	if ( status != 0 ) goto EXIT;
	#if 0	// *** XMP_DebugBuild
		if ( (tree._namePtr != tree.name.c_str()) ||
			 (tree._valuePtr != tree.value.c_str()) ) OutProcLiteral ( "  ** bad debug string **" );
	#endif
	OutProcNewline();
	
	if ( ! tree.value.empty() ) {
		OutProcLiteral ( "** bad root value **  \"" );
		OutProcString ( tree.value );
		OutProcNChars ( "\"", 1 );
		OutProcNewline();
	}

	if ( ! tree.qualifiers.empty() ) {
		OutProcLiteral ( "** bad root qualifiers **" );
		OutProcNewline();
		for ( size_t qualNum = 0, qualLim = tree.qualifiers.size(); qualNum < qualLim; ++qualNum ) {
			status = DumpPropertyTree ( tree.qualifiers[qualNum], 3, 0, outProc, refCon );
		}
	}

	if ( ! tree.children.empty() ) {

		for ( size_t childNum = 0, childLim = tree.children.size(); childNum < childLim; ++childNum ) {

			const XMP_Node * currSchema = tree.children[childNum];

			OutProcNewline();
			OutProcIndent ( 1 );
			OutProcString ( currSchema->name );
			OutProcNChars ( "  ", 2 );
			OutProcString ( currSchema->value );
			OutProcNChars ( "  ", 2 );
			status = DumpNodeOptions ( currSchema->options, outProc, refCon );
			if ( status != 0 ) goto EXIT;
			#if 0	// *** XMP_DebugBuild
				if ( (currSchema->_namePtr != currSchema->name.c_str()) ||
					 (currSchema->_valuePtr != currSchema->value.c_str()) ) OutProcLiteral ( "  ** bad debug string **" );
			#endif
			OutProcNewline();

			if ( ! (currSchema->options & kXMP_SchemaNode) ) {
				OutProcLiteral ( "** bad schema options **" );
				OutProcNewline();
			}

			if ( ! currSchema->qualifiers.empty() ) {
				OutProcLiteral ( "** bad schema qualifiers **" );
				OutProcNewline();
				for ( size_t qualNum = 0, qualLim = currSchema->qualifiers.size(); qualNum < qualLim; ++qualNum ) {
					DumpPropertyTree ( currSchema->qualifiers[qualNum], 3, 0, outProc, refCon );
				}
			}

			for ( size_t childNum = 0, childLim = currSchema->children.size(); childNum < childLim; ++childNum ) {
				DumpPropertyTree ( currSchema->children[childNum], 2, 0, outProc, refCon );
			}

		}

	}
	
EXIT:
	return status;

}	// DumpObject


// -------------------------------------------------------------------------------------------------
// CountArrayItems
// ---------------

XMP_Index
XMPMeta::CountArrayItems ( XMP_StringPtr schemaNS,
						   XMP_StringPtr arrayName ) const
{
	XMP_Assert ( (schemaNS != 0) && (arrayName != 0) );	// Enforced by wrapper.

	XMP_ExpandedXPath	expPath;
	ExpandXPath ( schemaNS, arrayName, &expPath );
	
	const XMP_Node * arrayNode = FindConstNode ( &tree, expPath );

	if ( arrayNode == 0 ) return 0;
	if ( ! (arrayNode->options & kXMP_PropValueIsArray) ) XMP_Throw ( "The named property is not an array", kXMPErr_BadXPath );
	return arrayNode->children.size();
	
}	// CountArrayItems


// -------------------------------------------------------------------------------------------------
// GetObjectName
// -------------

void
XMPMeta::GetObjectName ( XMP_StringPtr * namePtr,
						 XMP_StringLen * nameLen ) const
{
	
	*namePtr = tree.name.c_str();
	*nameLen = tree.name.size();
	
}	// GetObjectName


// -------------------------------------------------------------------------------------------------
// SetObjectName
// -------------

void
XMPMeta::SetObjectName ( XMP_StringPtr name )
{
	VerifyUTF8 ( name );	// Throws if the string is not legit UTF-8.
	tree.name = name;

}	// SetObjectName


// -------------------------------------------------------------------------------------------------
// GetObjectOptions
// ----------------

XMP_OptionBits
XMPMeta::GetObjectOptions() const
{
	XMP_OptionBits	options	= 0;
	
	return options;
	
}	// GetObjectOptions


// -------------------------------------------------------------------------------------------------
// SetObjectOptions
// ----------------

void
XMPMeta::SetObjectOptions ( XMP_OptionBits options )
{

	XMP_Throw ( "Unimplemented method XMPMeta::SetObjectOptions", kXMPErr_Unimplemented );
		void * p; p = &options;	// Avoid unused param warnings.

}	// SetObjectOptions


// -------------------------------------------------------------------------------------------------
// Clone
// -----

XMPMeta *
XMPMeta::Clone ( XMP_OptionBits options ) const
{
	if ( options != 0 ) XMP_Throw ( "No options are defined yet", kXMPErr_BadOptions );
	
	XMPMeta * clone = new XMPMeta;
	
	clone->tree.options = this->tree.options;
	clone->tree.name    = this->tree.name;
	clone->tree.value   = this->tree.value;
	XMP_Assert ( this->tree.parent == 0 );

	#if 0	// *** XMP_DebugBuild
		clone->tree._namePtr = clone->tree.name.c_str();
		clone->tree._valuePtr = clone->tree.value.c_str();
	#endif
	
	CloneOffspring ( &this->tree, &clone->tree );
	
	XMP_Assert ( clone->clientRefs == 0 );	// Gets incremneted later.
	return clone;
	
}	// Clone

// =================================================================================================
