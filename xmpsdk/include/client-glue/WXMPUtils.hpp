#if ! __WXMPUtils_hpp__
#define __WXMPUtils_hpp__ 1

// =================================================================================================
// Copyright 2002 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "client-glue/WXMP_Common.hpp"
#if __cplusplus
extern "C" {
#endif

// =================================================================================================

#define zXMPUtils_ComposeArrayItemPath_1(schemaNS,arrayName,itemIndex,itemPath,SetClientString) \
    WXMPUtils_ComposeArrayItemPath_1 ( schemaNS, arrayName, itemIndex, itemPath, SetClientString, &wResult );

#define zXMPUtils_ComposeStructFieldPath_1(schemaNS,structName,fieldNS,fieldName,fieldPath,SetClientString) \
    WXMPUtils_ComposeStructFieldPath_1 ( schemaNS, structName, fieldNS, fieldName, fieldPath, SetClientString, &wResult );

#define zXMPUtils_ComposeQualifierPath_1(schemaNS,propName,qualNS,qualName,qualPath,SetClientString) \
    WXMPUtils_ComposeQualifierPath_1 ( schemaNS, propName, qualNS, qualName, qualPath, SetClientString, &wResult );

#define zXMPUtils_ComposeLangSelector_1(schemaNS,arrayName,langName,selPath,SetClientString) \
    WXMPUtils_ComposeLangSelector_1 ( schemaNS, arrayName, langName, selPath, SetClientString, &wResult );

#define zXMPUtils_ComposeFieldSelector_1(schemaNS,arrayName,fieldNS,fieldName,fieldValue,selPath,SetClientString) \
    WXMPUtils_ComposeFieldSelector_1 ( schemaNS, arrayName, fieldNS, fieldName, fieldValue, selPath, SetClientString, &wResult );

#define zXMPUtils_ConvertFromBool_1(binValue,strValue,SetClientString) \
    WXMPUtils_ConvertFromBool_1 ( binValue, strValue, SetClientString, &wResult );

#define zXMPUtils_ConvertFromInt_1(binValue,format,strValue,SetClientString) \
    WXMPUtils_ConvertFromInt_1 ( binValue, format, strValue, SetClientString, &wResult );

#define zXMPUtils_ConvertFromInt64_1(binValue,format,strValue,SetClientString) \
    WXMPUtils_ConvertFromInt64_1 ( binValue, format, strValue, SetClientString, &wResult );

#define zXMPUtils_ConvertFromFloat_1(binValue,format,strValue,SetClientString) \
    WXMPUtils_ConvertFromFloat_1 ( binValue, format, strValue, SetClientString, &wResult );

#define zXMPUtils_ConvertFromDate_1(binValue,strValue,SetClientString) \
    WXMPUtils_ConvertFromDate_1 ( binValue, strValue, SetClientString, &wResult );

#define zXMPUtils_ConvertToBool_1(strValue) \
    WXMPUtils_ConvertToBool_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToInt_1(strValue) \
    WXMPUtils_ConvertToInt_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToInt64_1(strValue) \
    WXMPUtils_ConvertToInt64_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToFloat_1(strValue) \
    WXMPUtils_ConvertToFloat_1 ( strValue, &wResult );

#define zXMPUtils_ConvertToDate_1(strValue,binValue) \
    WXMPUtils_ConvertToDate_1 ( strValue, binValue, &wResult );

#define zXMPUtils_CurrentDateTime_1(time) \
    WXMPUtils_CurrentDateTime_1 ( time, &wResult );

#define zXMPUtils_SetTimeZone_1(time) \
    WXMPUtils_SetTimeZone_1 ( time, &wResult );

#define zXMPUtils_ConvertToUTCTime_1(time) \
    WXMPUtils_ConvertToUTCTime_1 ( time, &wResult );

#define zXMPUtils_ConvertToLocalTime_1(time) \
    WXMPUtils_ConvertToLocalTime_1 ( time, &wResult );

#define zXMPUtils_CompareDateTime_1(left,right) \
    WXMPUtils_CompareDateTime_1 ( left, right, &wResult );

#define zXMPUtils_EncodeToBase64_1(rawStr,rawLen,encodedStr,SetClientString) \
    WXMPUtils_EncodeToBase64_1 ( rawStr, rawLen, encodedStr, SetClientString, &wResult );

#define zXMPUtils_DecodeFromBase64_1(encodedStr,encodedLen,rawStr,SetClientString) \
    WXMPUtils_DecodeFromBase64_1 ( encodedStr, encodedLen, rawStr, SetClientString, &wResult );

#define zXMPUtils_PackageForJPEG_1(xmpObj,stdStr,extStr,digestStr,SetClientString) \
    WXMPUtils_PackageForJPEG_1 ( xmpObj, stdStr, extStr, digestStr, SetClientString, &wResult );

#define zXMPUtils_MergeFromJPEG_1(fullXMP,extendedXMP) \
    WXMPUtils_MergeFromJPEG_1 ( fullXMP, extendedXMP, &wResult );

#define zXMPUtils_CatenateArrayItems_1(xmpObj,schemaNS,arrayName,separator,quotes,options,catedStr,SetClientString) \
    WXMPUtils_CatenateArrayItems_1 ( xmpObj, schemaNS, arrayName, separator, quotes, options, catedStr, SetClientString, &wResult );

#define zXMPUtils_SeparateArrayItems_1(xmpObj,schemaNS,arrayName,options,catedStr) \
    WXMPUtils_SeparateArrayItems_1 ( xmpObj, schemaNS, arrayName, options, catedStr, &wResult );

#define zXMPUtils_ApplyTemplate_1(workingXMP,templateXMP,actions) \
    WXMPUtils_ApplyTemplate_1 ( workingXMP, templateXMP, actions, &wResult );

#define zXMPUtils_RemoveProperties_1(xmpObj,schemaNS,propName,options) \
    WXMPUtils_RemoveProperties_1 ( xmpObj, schemaNS, propName, options, &wResult );

#define zXMPUtils_DuplicateSubtree_1(source,dest,sourceNS,sourceRoot,destNS,destRoot,options) \
    WXMPUtils_DuplicateSubtree_1 ( source, dest, sourceNS, sourceRoot, destNS, destRoot, options, &wResult );

// =================================================================================================

extern void
XMP_PUBLIC WXMPUtils_ComposeArrayItemPath_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   arrayName,
                                   XMP_Index       itemIndex,
                                   void *          itemPath,
                                   SetClientStringProc SetClientString,
                                   WXMP_Result *   wResult );

extern void
XMP_PUBLIC WXMPUtils_ComposeStructFieldPath_1 ( XMP_StringPtr   schemaNS,
                                     XMP_StringPtr   structName,
                                     XMP_StringPtr   fieldNS,
                                     XMP_StringPtr   fieldName,
                                     void *          fieldPath,
                                     SetClientStringProc SetClientString,
                                     WXMP_Result *   wResult );

extern void
XMP_PUBLIC WXMPUtils_ComposeQualifierPath_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   propName,
                                   XMP_StringPtr   qualNS,
                                   XMP_StringPtr   qualName,
                                   void *          qualPath,
                                   SetClientStringProc SetClientString,
                                   WXMP_Result *   wResult );

extern void
XMP_PUBLIC WXMPUtils_ComposeLangSelector_1 ( XMP_StringPtr   schemaNS,
                                  XMP_StringPtr   arrayName,
                                  XMP_StringPtr   langName,
                                  void *          selPath,
                                  SetClientStringProc SetClientString,
                                  WXMP_Result *   wResult );

extern void
XMP_PUBLIC WXMPUtils_ComposeFieldSelector_1 ( XMP_StringPtr   schemaNS,
                                   XMP_StringPtr   arrayName,
                                   XMP_StringPtr   fieldNS,
                                   XMP_StringPtr   fieldName,
                                   XMP_StringPtr   fieldValue,
                                   void *          selPath,
                                   SetClientStringProc SetClientString,
                                   WXMP_Result *   wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_ConvertFromBool_1 ( XMP_Bool      binValue,
                              void *        strValue,
                              SetClientStringProc SetClientString,
                              WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertFromInt_1 ( XMP_Int32     binValue,
                             XMP_StringPtr format,
                             void *        strValue,
                             SetClientStringProc SetClientString,
                             WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertFromInt64_1 ( XMP_Int64     binValue,
                               XMP_StringPtr format,
                               void *        strValue,
                               SetClientStringProc SetClientString,
                               WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertFromFloat_1 ( double        binValue,
                               XMP_StringPtr format,
                               void *        strValue,
                               SetClientStringProc SetClientString,
                               WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertFromDate_1 ( const XMP_DateTime & binValue,
                              void *               strValue,
                              SetClientStringProc SetClientString,
                              WXMP_Result *        wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_ConvertToBool_1 ( XMP_StringPtr strValue,
                            WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToInt_1 ( XMP_StringPtr strValue,
                           WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToInt64_1 ( XMP_StringPtr strValue,
                             WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToFloat_1 ( XMP_StringPtr strValue,
                             WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToDate_1 ( XMP_StringPtr  strValue,
                            XMP_DateTime * binValue,
                            WXMP_Result *  wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_CurrentDateTime_1 ( XMP_DateTime * time,
                              WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_SetTimeZone_1 ( XMP_DateTime * time,
                          WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToUTCTime_1 ( XMP_DateTime * time,
                               WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_ConvertToLocalTime_1 ( XMP_DateTime * time,
                                 WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_CompareDateTime_1 ( const XMP_DateTime & left,
                              const XMP_DateTime & right,
                              WXMP_Result *        wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_EncodeToBase64_1 ( XMP_StringPtr rawStr,
                             XMP_StringLen rawLen,
                             void *        encodedStr,
                             SetClientStringProc SetClientString,
                             WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_DecodeFromBase64_1 ( XMP_StringPtr encodedStr,
                               XMP_StringLen encodedLen,
                               void *        rawStr,
                               SetClientStringProc SetClientString,
                               WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_PackageForJPEG_1 ( XMPMetaRef    xmpObj,
                             void *        stdStr,
                             void *        extStr,
                             void *        digestStr,
                             SetClientStringProc SetClientString,
                             WXMP_Result * wResult );

extern void
XMP_PUBLIC WXMPUtils_MergeFromJPEG_1 ( XMPMetaRef    fullXMP,
                            XMPMetaRef    extendedXMP,
                            WXMP_Result * wResult );

// -------------------------------------------------------------------------------------------------

extern void
XMP_PUBLIC WXMPUtils_CatenateArrayItems_1 ( XMPMetaRef     xmpObj,
                                 XMP_StringPtr  schemaNS,
                                 XMP_StringPtr  arrayName,
                                 XMP_StringPtr  separator,
                                 XMP_StringPtr  quotes,
                                 XMP_OptionBits options,
                                 void *         catedStr,
                                 SetClientStringProc SetClientString,
                                 WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_SeparateArrayItems_1 ( XMPMetaRef     xmpObj,
                                 XMP_StringPtr  schemaNS,
                                 XMP_StringPtr  arrayName,
                                 XMP_OptionBits options,
                                 XMP_StringPtr  catedStr,
                                 WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_ApplyTemplate_1 ( XMPMetaRef     workingXMP,
							XMPMetaRef     templateXMP,
							XMP_OptionBits options,
							WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_RemoveProperties_1 ( XMPMetaRef     xmpObj,
                               XMP_StringPtr  schemaNS,
                               XMP_StringPtr  propName,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

extern void
XMP_PUBLIC WXMPUtils_DuplicateSubtree_1 ( XMPMetaRef     source,
                               XMPMetaRef     dest,
                               XMP_StringPtr  sourceNS,
                               XMP_StringPtr  sourceRoot,
                               XMP_StringPtr  destNS,
                               XMP_StringPtr  destRoot,
                               XMP_OptionBits options,
                               WXMP_Result *  wResult );

// =================================================================================================

#if __cplusplus
} /* extern "C" */
#endif

#endif  // __WXMPUtils_hpp__
