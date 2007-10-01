#ifndef __XMLParserAdapter_hpp__
#define __XMLParserAdapter_hpp__

// =================================================================================================
// Copyright 2005-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMPCore_Impl.hpp"

// =================================================================================================
// Abstract base class for XML parser adapters used by the XMP toolkit.
// =================================================================================================

enum { kXMLPendingInputMax = 16 };

class XMLParserAdapter {
public:

	XMLParserAdapter()
		: tree(0,"",kRootNode), rootNode(0), rootCount(0), charEncoding(XMP_OptionBits(-1)), pendingCount(0)
	{
		#if XMP_DebugBuild
			parseLog = 0;
		#endif
	};

	virtual ~XMLParserAdapter() {};
	
	virtual void ParseBuffer ( const void * buffer, size_t length, bool last ) = 0;

	XML_Node		tree;
	XML_NodeVector	parseStack;
	XML_Node *		rootNode;
	size_t			rootCount;

	XMP_OptionBits	charEncoding;
	size_t          pendingCount;
	unsigned char	pendingInput[kXMLPendingInputMax];	// Buffered input for character encoding checks.
	
	#if XMP_DebugBuild
		FILE * parseLog;
	#endif

};

// =================================================================================================

#endif	// __XMLParserAdapter_hpp__
