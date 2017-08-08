#if ! __WXMP_Common_hpp__
#define __WXMP_Common_hpp__ 1

// =================================================================================================
// Copyright 2002 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#ifndef XMP_Inline
	#if TXMP_EXPAND_INLINE
		#define XMP_Inline inline
	#else
		#define XMP_Inline /* not inline */
	#endif
#endif

#define XMP_CTorDTorIntro(Class) template <class tStringObj> XMP_Inline Class<tStringObj>
#define XMP_MethodIntro(Class,ResultType) template <class tStringObj> XMP_Inline ResultType Class<tStringObj>

typedef void (* SetClientStringProc) ( void * clientPtr, XMP_StringPtr valuePtr, XMP_StringLen valueLen );
typedef void (* SetClientStringVectorProc) ( void * clientPtr, XMP_StringPtr * arrayPtr, XMP_Uns32 stringCount );

struct WXMP_Result {
    XMP_StringPtr errMessage;
    void *        ptrResult;
    double        floatResult;
    XMP_Uns64     int64Result;
    XMP_Uns32     int32Result;
	WXMP_Result() : errMessage(0),ptrResult(NULL),floatResult(0),int64Result(0),int32Result(0){};
};

#if __cplusplus
extern "C" {
#endif

#define PropagateException(res)	\
	if ( res.errMessage != 0 ) throw XMP_Error ( res.int32Result, res.errMessage );

#ifndef XMP_TraceClientCalls
	#define XMP_TraceClientCalls		0
	#define XMP_TraceClientCallsToFile	0
#endif

#if ! XMP_TraceClientCalls
	#define InvokeCheck(WCallProto) \
    	WXMP_Result wResult;        \
		WCallProto;                 \
		PropagateException ( wResult )
#else
	extern FILE * xmpClientLog;
	#define InvokeCheck(WCallProto)                                                                          \
    	WXMP_Result wResult;                                                                                 \
    	fprintf ( xmpClientLog, "WXMP calling: %s\n", #WCallProto ); fflush ( xmpClientLog );                \
    	WCallProto;                                                                                          \
    	if ( wResult.errMessage == 0 ) {                                                                     \
			fprintf ( xmpClientLog, "WXMP back, no error\n" ); fflush ( xmpClientLog );                      \
    	} else {                                                                                             \
			fprintf ( xmpClientLog, "WXMP back, error: %s\n", wResult.errMessage ); fflush ( xmpClientLog ); \
    	}                                                                                                    \
		PropagateException ( wResult )
#endif

// =================================================================================================

#define WrapNoCheckVoid(WCallProto) \
	WCallProto;

#define WrapCheckVoid(WCallProto) \
    InvokeCheck(WCallProto);

#define WrapCheckMetaRef(result,WCallProto) \
    InvokeCheck(WCallProto);                \
    XMPMetaRef result = XMPMetaRef(wResult.ptrResult)

#define WrapCheckIterRef(result,WCallProto) \
    InvokeCheck(WCallProto);                \
    XMPIteratorRef result = XMPIteratorRef(wResult.ptrResult)

#define WrapCheckDocOpsRef(result,WCallProto) \
    InvokeCheck(WCallProto);                  \
    XMPDocOpsRef result = XMPDocOpsRef(wResult.ptrResult)

#define  WrapCheckNewMetadata(result,WCallProto) \
    InvokeCheck(WCallProto);                  \
    void * result = wResult.ptrResult

#define WrapCheckBool(result,WCallProto) \
    InvokeCheck(WCallProto);             \
    bool result = bool(wResult.int32Result)

#define WrapCheckTriState(result,WCallProto) \
    InvokeCheck(WCallProto);                 \
    XMP_TriState result = XMP_TriState(wResult.int32Result)

#define WrapCheckOptions(result,WCallProto) \
    InvokeCheck(WCallProto);                \
    XMP_OptionBits result = XMP_OptionBits(wResult.int32Result)

#define WrapCheckStatus(result,WCallProto) \
    InvokeCheck(WCallProto);               \
    XMP_Status result = XMP_Status(wResult.int32Result)

#define WrapCheckIndex(result,WCallProto) \
    InvokeCheck(WCallProto);              \
    XMP_Index result = XMP_Index(wResult.int32Result)

#define WrapCheckInt32(result,WCallProto) \
    InvokeCheck(WCallProto);              \
    XMP_Int32 result = wResult.int32Result

#define WrapCheckInt64(result,WCallProto) \
    InvokeCheck(WCallProto);              \
    XMP_Int64 result = wResult.int64Result

#define WrapCheckFloat(result,WCallProto) \
    InvokeCheck(WCallProto);              \
    double result = wResult.floatResult

#define WrapCheckFormat(result,WCallProto) \
    InvokeCheck(WCallProto);               \
    XMP_FileFormat result = wResult.int32Result

// =================================================================================================

#if __cplusplus
}	// extern "C"
#endif

#endif  // __WXMP_Common_hpp__
