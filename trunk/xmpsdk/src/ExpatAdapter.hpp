#ifndef __ExpatAdapter_hpp__
#define __ExpatAdapter_hpp__

// =================================================================================================
// Copyright 2005-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "XMP_Environment.h"	// ! Must be the first #include!
#include "XMLParserAdapter.hpp"

#if UsePublicExpat
	#include "expat.h"
#endif

// =================================================================================================
// Derived XML parser adapter for Expat.
// =================================================================================================

class ExpatAdapter : public XMLParserAdapter {
public:

	XML_Parser parser;
	size_t     nesting;
	
	ExpatAdapter();
	virtual ~ExpatAdapter();
	
	void ParseBuffer ( const void * buffer, size_t length, bool last );

};

// =================================================================================================

#endif	// __ExpatAdapter_hpp__
