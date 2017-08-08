#ifndef __XMP_incl_cpp__
#define __XMP_incl_cpp__ 1

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2002 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

//  ================================================================================================
/// \file XMP.incl_cpp
/// \brief Overall client glue file for the XMP toolkit.
///
/// This is an overall client source file of XMP toolkit glue, the only XMP-specific one that
/// clients should build in projects. This ensures that all of the client-side glue code for the
/// XMP toolkit gets compiled.
///
/// You cannot compile this file directly, because the template's string type must be declared and 
/// only the client can do that. Instead, include this in some other source file. For example, 
/// to use <tt>std::string</tt> you only need these two lines:
///
/// \code
///  #include <string>
///  #include "XMP.incl_cpp"
/// \endcode


#include "XMP.hpp" // ! This must be the first include!

#define XMP_ClientBuild 1

#if XMP_WinBuild
    #if XMP_DebugBuild
        #pragma warning ( push, 4 )
    #else
        #pragma warning ( push, 3 )
    #endif

   	#pragma warning ( disable : 4127 ) // conditional expression is constant
    #pragma warning ( disable : 4189 ) // local variable is initialized but not referenced
    #pragma warning ( disable : 4702 ) // unreachable code
    #pragma warning ( disable : 4800 ) // forcing value to bool 'true' or 'false' (performance warning)
#endif

#if defined ( TXMP_STRING_TYPE ) && (! TXMP_EXPAND_INLINE)

    // We're using a single out of line instantiation. Do it here.

    #include "client-glue/TXMPMeta.incl_cpp"
    #include "client-glue/TXMPIterator.incl_cpp"
    #include "client-glue/TXMPUtils.incl_cpp"
    template class TXMPMeta <TXMP_STRING_TYPE>;
    template class TXMPIterator <TXMP_STRING_TYPE>;
    template class TXMPUtils <TXMP_STRING_TYPE>;
	#if XMP_INCLUDE_XMPFILES
	    #include "client-glue/TXMPFiles.incl_cpp"
	    template class TXMPFiles <TXMP_STRING_TYPE>;
   #endif

#endif

#if XMP_WinBuild
    #pragma warning ( pop )
#endif

#endif // __XMP_incl_cpp__
