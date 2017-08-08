#ifndef __XMP_Environment_h__
#define __XMP_Environment_h__ 1

// =================================================================================================
// XMP_Environment.h - Build environment flags for the XMP toolkit.
// ================================================================
//
// This header is just C preprocessor macro definitions to set up the XMP toolkit build environment.
// It must be the first #include in any chain since it might affect things in other #includes.
//
// =================================================================================================

// =================================================================================================
// Copyright 2002 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

// =================================================================================================
// Determine the Platform
// ======================

// One of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV must be defined by the client. Since some other code
// requires these to be defined without values, they are only used here to define XMP-specific
// macros with 0 or 1 values.

// ! Tempting though it might be to have a standard macro for big or little endian, there seems to
// ! be no decent way to do that on our own in UNIX. Forcing it on the client isn't acceptable.

#if defined ( MAC_ENV )

	#if 0	// ! maybe someday - ! MAC_ENV
		#error "MAC_ENV must be defined so that \"#if MAC_ENV\" is true"
	#endif
	
    #if defined ( WIN_ENV ) || defined ( UNIX_ENV ) || defined ( IOS_ENV )
        #error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV"
    #endif

    #define XMP_MacBuild  1
    #define XMP_WinBuild  0
    #define XMP_UNIXBuild 0
	#define XMP_iOSBuild  0

#elif defined ( WIN_ENV )

	#if 0	// ! maybe someday - ! WIN_ENV
		#error "WIN_ENV must be defined so that \"#if WIN_ENV\" is true"
	#endif
	
    #if defined ( MAC_ENV ) || defined ( UNIX_ENV ) || defined ( IOS_ENV )
		#error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV"
    #endif

    #define XMP_MacBuild  0
    #define XMP_WinBuild  1
    #define XMP_UNIXBuild 0
	#define XMP_iOSBuild  0

#elif defined ( UNIX_ENV )

	#if 0	// ! maybe someday - ! UNIX_ENV
		#error "UNIX_ENV must be defined so that \"#if UNIX_ENV\" is true"
	#endif
	
	#if defined ( MAC_ENV ) || defined ( WIN_ENV ) || defined ( IOS_ENV )
		#error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV"
	#endif

    #define XMP_MacBuild  0
    #define XMP_WinBuild  0
    #define XMP_UNIXBuild 1
	#define XMP_iOSBuild  0

#elif defined ( IOS_ENV )

	#if 0	// ! maybe someday - ! IOS_ENV
		#error "IOS_ENV must be defined so that \"#if IOS_ENV\" is true"
	#endif

	#if defined ( MAC_ENV ) || defined ( WIN_ENV ) || defined ( UNIX_ENV )
		#error "XMP environment error - must define only one of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV"
	#endif

	#define XMP_MacBuild  0
	#define XMP_WinBuild  0
	#define XMP_UNIXBuild 0
	#define XMP_iOSBuild  1

#else

    #error "XMP environment error - must define one of MAC_ENV, WIN_ENV, UNIX_ENV or IOS_ENV"

#endif

// =================================================================================================
// Common Macros
// =============

#if defined ( DEBUG )
    #if defined ( NDEBUG )
        #error "XMP environment error - both DEBUG and NDEBUG are defined"
    #endif
    #define XMP_DebugBuild 1
#endif

#if defined ( NDEBUG )
    #define XMP_DebugBuild 0
#endif

#ifndef XMP_DebugBuild
    #define XMP_DebugBuild 0
#endif

#if XMP_DebugBuild
    #include <stdio.h>  // The assert macro needs printf.
#endif

#ifndef DISABLE_SERIALIZED_IMPORT_EXPORT 
	#define DISABLE_SERIALIZED_IMPORT_EXPORT 0
#endif

#ifndef XMP_64
	#if _WIN64 || defined(_LP64)
		#define XMP_64 1
	#else
		#define XMP_64 0
	#endif
#endif

// =================================================================================================
// Macintosh Specific Settings
// ===========================
#if (XMP_MacBuild)
	#define XMP_HELPER_DLL_IMPORT __attribute__((visibility("default")))
	#define XMP_HELPER_DLL_EXPORT __attribute__((visibility("default")))
	#define XMP_HELPER_DLL_PRIVATE __attribute__((visibility("hidden")))
	#define APICALL 
#endif

// =================================================================================================
// Windows Specific Settings
// =========================
#if (XMP_WinBuild)
	#define XMP_HELPER_DLL_IMPORT
	#define XMP_HELPER_DLL_EXPORT
	#define XMP_HELPER_DLL_PRIVATE
	#define APICALL __stdcall
#endif

// =================================================================================================
// UNIX Specific Settings
// ======================
#if (XMP_UNIXBuild)
	#define XMP_HELPER_DLL_IMPORT
	#define XMP_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
	#define XMP_HELPER_DLL_PRIVATE __attribute__ ((visibility ("hidden")))
	#define APICALL 
#endif

// =================================================================================================
// IOS Specific Settings
// ===========================
#if (XMP_iOSBuild)
	#include <TargetConditionals.h>
	#if (TARGET_CPU_ARM)
		#define XMP_IOS_ARM 1
	#else
		#define XMP_IOS_ARM 0
	#endif
	#define XMP_HELPER_DLL_IMPORT __attribute__((visibility("default")))
	#define XMP_HELPER_DLL_EXPORT __attribute__((visibility("default")))
	#define XMP_HELPER_DLL_PRIVATE __attribute__((visibility("hidden")))
	#define APICALL 
#endif

// =================================================================================================

#if (XMP_DynamicBuild)
	#define XMP_PUBLIC XMP_HELPER_DLL_EXPORT
	#define XMP_PRIVATE XMP_HELPER_DLL_PRIVATE
#elif (XMP_StaticBuild)
	#define XMP_PUBLIC
	#define XMP_PRIVATE
#else
	#define XMP_PUBLIC XMP_HELPER_DLL_IMPORT
	#define XMP_PRIVATE XMP_HELPER_DLL_PRIVATE
#endif

#endif  // __XMP_Environment_h__
