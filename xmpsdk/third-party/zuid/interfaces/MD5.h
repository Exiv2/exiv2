#ifndef __MD5_h__
#define __MD5_h__

/******************************************************************************/

/*
	MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm

	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights 
	reserved.

	License to copy and use this software is granted provided that it is 
	identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm" in 
	all material mentioning or referencing this software or this function. 

	License is also granted to make and use derivative works provided that such 
	works are identified as "derived from the RSA Data Security, Inc. MD5 
	Message-Digest Algorithm" in all material mentioning or referencing the 
	derived work. 
	
	RSA Data Security, Inc. makes no representations concerning either the 
	merchantability of this software or the suitability of this software for 
	any particular purpose. It is provided "as is" without express or implied 
	warranty of any kind. 
	
	These notices must be retained in any copies of any part of this 
	documentation and/or software. 
*/

/******************************************************************************/

#include "public/include/XMP_Const.h"	// For safe fixed integer sizes.

/* MD5 context. */
struct MD5_CTX
	{
	XMP_Uns32 state[4];                                   /* state (ABCD) */
	XMP_Uns32 count[2];        /* number of bits, modulo 2^64 (lsb first) */
	XMP_Uns8 buffer[64];                         /* input buffer */
	};

extern void MD5Init (MD5_CTX *);
extern void MD5Update (MD5_CTX *, XMP_Uns8 *, XMP_Uns32);
extern void MD5Final(XMP_Uns8 [16], MD5_CTX *);

/******************************************************************************/

#endif
