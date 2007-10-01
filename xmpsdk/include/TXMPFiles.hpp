#ifndef __TXMPFiles_hpp__
#define __TXMPFiles_hpp__	1

#if ( ! __XMP_hpp__ )
    #error "Do not directly include, use XMP.hpp"
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
/// \file TXMPFiles.hpp
/// \brief API for access to the "main" metadata in a file.
///
/// \c TXMPFiles provides the API for the Adobe XMP Toolkit's File Handler component. This provides
/// convenient access to the main, or document level, XMP for a file. The File Handler supports file
/// I/O, the XMP Toolkit Core supports manipulation of the XMP properties. The File Handler is
/// intended to eventually have smart, efficient support for all file formats for which the means to
/// embed XMP is defined in the XMP Specification. Where possible this support will allow injection
/// of XMP where none currently exists, expansion of XMP without regard to existing padding, and
/// reconciliation of the XMP and other legacy forms of metadata.
///
///\c  TXMPFiles is designed for use by clients interested in the metadata and not in the primary file
/// content. The Adobe Bridge application is a typical example. \c TXMPFiles is not intended to be
/// particulary appropriate for files authored by an application. I.e. those files for which the
/// application has explicit knowledge of the file format.
//  ================================================================================================

//  ================================================================================================
/// \class TXMPFiles TXMPFiles.hpp
/// \brief API for access to the "main" metadata in a file.
///
/// \c TXMPFiles provides the API for the Adobe XMP Toolkit's File Handler component. This provides
/// convenient access to the main, or document level, XMP for a file. The general model is to open
/// a file, read and write the metadata, then close the file. While open, portions of the file
/// might be maintained in RAM data structures. Memory usage can vary considerably depending on
/// file format and access options. The file may be opened for read-only or read-write access, with
/// typical exclusion for both modes.
///
/// Errors result in the throw of an \c XMPError exception.
///
/// The template is instantiated with a string object class. This allows a clean implementation
/// that provides two major benefits: output string storage is fully owned by the client and access
/// is fully thread safe. The template parameter, class \c tStringObj, is described in the XMP.hpp
/// umbrella header.
///
/// To use TXMPFiles define TXMP_STRING_TYPE and XMP_INCLUDE_XMPFILES, then include the XMP.hpp 
/// umbrella header:
/// \code
///  #define TXMP_STRING_TYPE std::string
///  #define XMP_INCLUDE_XMPFILES 1
///  #include "XMP.hpp"
/// \endcode
//  ================================================================================================

template <class tStringObj>
class TXMPFiles {

public:
	
	//  ============================================================================================
    /// \name Initialization and termination
    /// @{
    /// \c SXMPFiles must be initialized before use and may be terminated when done.

    static void GetVersionInfo ( XMP_VersionInfo * versionInfo );

	/// \brief \c Initialize must be called before using \c SXMPFiles. It returns a Boolean 
	/// success/failure value.
	
	static bool Initialize();
	static bool Initialize ( XMP_OptionBits options );

	/// \brief \c Terminate may be called when done using \c SXMPFiles. It deallocates global data
	/// structures created by \c Initialize.

	static void Terminate();

    /// @}

	//  ============================================================================================
    /// \name Constructors and destructor
    /// @{
    /// The default constructor initializes an object that is associated with no file. The alternate
    /// constructors call OpenFile. The destructor automatically calls CloseFile if necessary.
    
    /// \brief The default constructor initializes an object that is associated with no file.
    
    TXMPFiles();
	virtual ~TXMPFiles() throw();

    /// \brief These alternate constructors call \c OpenFile. The second form is a trivial overload
    /// that calls the first form passing \c filePath.c_str().
    
    TXMPFiles ( XMP_StringPtr  filePath,
				XMP_FileFormat format = kXMP_UnknownFile,
				XMP_OptionBits openFlags = 0 );

    TXMPFiles ( const tStringObj & filePath,
				XMP_FileFormat     format = kXMP_UnknownFile,
				XMP_OptionBits     openFlags = 0 );

	/// \brief The copy constructor and assignment operator increment an internal reference count,
	/// they do not perform a deep copy.
	
    TXMPFiles ( const TXMPFiles<tStringObj> & original );

	void operator= ( const TXMPFiles<tStringObj> & rhs );

	/// \brief The "ref" constructor and \c GetInternalRef serve the same purpose as their analogs
	/// in SXMPMeta, safely passing \c SXMPFiles references across DLL boundaries where the clients
	/// might have used different string types when instantiating \c TXMPFiles.
	
    TXMPFiles ( XMPFilesRef xmpFilesObj );
    
    XMPFilesRef GetInternalRef();

    /// @}
	
	//  ============================================================================================
    /// \name Static Functions
    /// @{
    
    //  --------------------------------------------------------------------------------------------
    /// \brief Determine the supported features for a given file format.
    ///
    /// The supported features can vary quite a bit among file formats, depending on both the
    /// general capabilities of the format and the implementation of the handler for that format.
    ///
    /// \param format The format whose support flags are desired.
    ///
    /// \param handlerFlags A set of option bits showing the support for this format:
    ///
    /// \li kXMPFiles_CanInjectXMP - Can inject first-time XMP into an existing file.
    /// \li kXMPFiles_CanExpand - Can expand XMP or other metadata in an existing file.
    /// \li kXMPFiles_CanRewrite - Can copy one file to another, writing new metadata.
    /// \li kXMPFiles_CanReconcile - Supports reconciliation between XMP and other forms.
    /// \li kXMPFiles_AllowsOnlyXMP - Allows access to just the XMP, ignoring other forms.
    /// \li kXMPFiles_ReturnsRawPacket - File handler returns raw XMP packet information and string.
    /// \li kXMPFiles_ReturnsTNail - File handler returns native thumbnail information.
    ///
    /// The kXMPFiles_AllowsOnlyXMP flag is only meaningful if kXMPFiles_CanReconcile is set.
    ///
    /// If kXMPFiles_ReturnsRawPacket is set, the returned packet information might have an offset
    /// of -1 to indicate an unknown offset. While all file handlers should be able to return the
    /// raw packet, some might not know the offset of the packet within the file. This is typical
    /// in cases where external libraries are used. These cases might not even allow return of the
    /// raw packet.
    ///
    /// \result Returns true if the format has explicit "smart" support. Returns false if the format
    /// is handled by the default packet scanning plus heuristics.
    
    static bool GetFormatInfo ( XMP_FileFormat   format,
    							XMP_OptionBits * handlerFlags = 0 );

    /// @}
	
	//  ============================================================================================
    /// \name OpenFile, CloseFile, and related file-oriented operations
    /// @{
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Open a file for metadata access.
    ///
    /// Opens a file for the requested forms of metadata access. Opening the file at a minimum
    /// causes the raw XMP packet to be read from the file. If the file handler supports legacy
    /// metadata reconciliation then legacy metadata will also be read, unless kXMPFiles_OpenOnlyXMP
    /// is passed. If the file handler supports native thumbnails and kXMPFiles_OpenCacheTNail is
    /// passed then the native thumbnail will also be cached.
    ///
    /// If the file is opened for read-only access (passing kXMPFiles_OpenForRead), then the disk
    /// file itself will be closed after reading the data from it. The XMPFiles object will not be
    /// "closed" though, it is still necessary to call CloseFile when finished using it. Other
    /// methods (GetXMP, etc.) can only be used between the OpenFile and CloseFile calls. The XMPFiles
    /// destructor will not call CloseFile, any pending updates will be lost.
    ///
    /// If the file is opened for update (passing kXMPFiles_OpenForUpdate), then the disk file remains
    /// open until CloseFile is called. The disk file is only updated once, when Close file is called,
    /// no matter how many calls are made to PutXMP.
    ///
    /// Ideally the XMP is not parsed and legacy reconciliation is not performed until GetXMP is
    /// called. This is not guaranteed though, specific file handlers might do earlier parsing of
    /// the XMP. This delayed parsing and the early disk file close for read-only access are
    /// optimizations to help clients implementing file browsers. They can access the file briefly
    /// and possibly display a thumbnail, then postpone more expensive XMP processing until later.
    ///
    /// \param filePath The UTF-8 path for the file, appropriate for the local OS. Overloads are
    /// declared to pass the path as either a "const char *" or a string object.
    ///
    /// \param format The format of the file. If the format is unknown pass \c kXMP_UnknownFile and
    /// the format will be determined from the file content. The first handler to check will be
    /// guessed from the file's extension. Passing any other format value is generally just a hint
    /// about what file handler to try first (instead of the one based on the extension). If the 
    /// kXMPFiles_OpenStrictly is set, then any format other than kXMP_UnknownFile requires that the
    /// file actually be that format, an exception is thrown if not.
    ///
    /// \param openFlags A set of option bits describing the desired access. By default (zero) the
    /// file is opened for read-only access and the format handler decides on the level of
    /// reconciliation that will be performed. By default a best effort will be made to locate the
    /// correct XMP and to reconcile XMP with other forms (if reconciliation is done). The option
    /// \c kXMPFiles_OpenStrictly may be used to force more strict rules, resulting is exceptions for
    /// errors. The definition of strictness is specific to each handler, there may be no difference.
    ///
    /// The defined openFlag bits are:
    ///
    /// \li kXMPFiles_OpenForRead - Open for read-only access.
    /// \li kXMPFiles_OpenForUpdate - Open for reading and writing.
    /// \li kXMPFiles_OpenOnlyXMP - Only the XMP is wanted, no reconciliation.
    /// \li kXMPFiles_OpenCacheTNail - Cache thumbnail if possible, GetThumbnail will be called.
    /// \li kXMPFiles_OpenStrictly - Be strict about locating XMP and reconciling with other forms.
    /// \li kXMPFiles_OpenUseSmartHandler - Require the use of a smart handler.
    /// \li kXMPFiles_OpenUsePacketScanning - Force packet scanning, don't use a smart handler.
    ///
    /// \result Returns true if the file is succesfully opened and attached to a file handler.
    /// Returns false for "anticipated" problems, e.g. passing kXMPFiles_OpenUseSmartHandler but not
    /// having an appropriate smart handler. Throws an exception for serious problems.

	bool OpenFile ( XMP_StringPtr  filePath,
				    XMP_FileFormat format = kXMP_UnknownFile,
				    XMP_OptionBits openFlags = 0 );
	
	bool OpenFile ( const tStringObj & filePath,
				    XMP_FileFormat     format = kXMP_UnknownFile,
				    XMP_OptionBits     openFlags = 0 );

    //  --------------------------------------------------------------------------------------------
    /// \brief Close an opened file.
    ///
    /// Performs any necessary output to the file and closes it. Files that are opened for update
    /// are written to only when closing.
    ///
    /// \param closeFlags A set of bit flags for optional closing actions.
    ///
    /// The defined closeFlags bits are:
    ///
    /// \li kXMPFiles_UpdateSafely - Write into a temporary file then swap for crash safety.
    
    void CloseFile ( XMP_OptionBits closeFlags = 0 );
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Get basic information about an opened file.
    ///
    /// \param filePath If not null, returns the path passed to OpenFile.
    ///
    /// \param openFlags If not null, returns the flags passed to OpenFile.
    ///
    /// \param format If not null, returns the format of the file.
    ///
    /// \param handlerFlags If not null, returns the handler's capability flags.
    ///
    /// \result Returns true if a file is opened, false otherwise. This notion of "open" really means
    /// that OpenFile has been called but CloseFile has not. The actual disk file might be closed in
    /// the host file system sense, as explained for OpenFile.
	
	bool GetFileInfo ( tStringObj *     filePath = 0,
					   XMP_OptionBits * openFlags = 0,
					   XMP_FileFormat * format = 0,
    				   XMP_OptionBits * handlerFlags = 0 );
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Set the callback function used to check for a user signaled abort.
    ///
    /// \param abortProc The callback function used to check for a user signaled abort. It will be
    /// called periodically to allow an abort of time consuming operations. The abort results in an
    /// exception being thrown. The callback function should return true to signal an abort.
    ///
    /// \param abortArg An argument passed to the callback function.

    void SetAbortProc ( XMP_AbortProc abortProc,
    					void *        abortArg );

    /// @}
	
	//  ============================================================================================
    /// \name Metadata Access Functions
    /// @{
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain the XMP.
    ///
    /// \c GetXMP is used to obtain the parsed XMP, and/or the raw XMP packet, and/or information
    /// about the raw XMP packet. If all parameters are null it simply tells if XMP is present or
    /// not. The options provided when the file was opened determine if reconciliation is done with
    /// other forms of metadata.
    ///
    /// \param xmpObj If not null, returns the parsed XMP.
    ///
    /// \param xmpPacket If not null, returns the raw XMP packet as stored in the file. The encoding
    /// of the packet is given in the packetInfo. The string will be empty if the low level file
    /// handler does not provide the raw packet.
    ///
    /// \param packetInfo If not null, returns the location and form of the raw XMP in the file. The
    /// charForm and writeable flag reflect the raw XMP in the file. The parsed XMP property values
    /// are always UTF-8. The writeable flag is taken from the packet trailer, it is only relevant
    /// for "format ignorant" writing.
    ///
    /// \note The packetInfo struct always reflects the state of the XMP in the file. The offset,
    /// length, and character form will not change as a result of calling \c PutXMP unless the file
    /// is also written.
    ///
    /// \note Some file handlers might not return location or contents of the raw packet string.
    /// Check the \c kXMPFiles_ReturnsRawPacket bit returned by GetFormatInfo if you depend on this.
    /// If the low level file handler does not provide the raw packet location then the offset and
    /// length will both be 0, the charForm will be UTF-8, and the writeable flag will be false.
    ///
    /// \result Returns true if the file has XMP, false otherwise.
    
    bool GetXMP ( SXMPMeta *       xmpObj = 0,
    			  tStringObj *     xmpPacket = 0,
    			  XMP_PacketInfo * packetInfo = 0 );
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Obtain the native thumbnail.
    ///
    /// \c GetThumbnail is used to obtain native thumbnail information, if the associated file
    /// handler supports that and the thumbnail was cached by OpenFile. This requires that
    /// kXMPFiles_OpenCacheTNail be passed to OpenFile. The tnailInfo output pointer can be null,
    /// in which case GetThumbnail will simply tell if a recognized native thumbnail is present.
    ///
    /// \param tnailInfo If not null, returns information about a recognized native thumbnail, and
    /// some related information about the primary image if appropriate.
    ///
    /// \note The returned thumbnail information can be incomplete. What gets returned can depend on
    /// the file format, the file handler's capabilities, and the specific file content.
    ///
    /// \li The fullHeight, fullWIdth, and fullOrientation fields are only meaningful for image files.
    /// They are not meaningful for multi-page files such as PDF or InDesign, for dynamic audio or
    /// video files, etc. The fields will be zero if not meaningful or not determined.
    ///
    /// \li The tnailImage and tnailSize fields might be zero even if a "recognized" thumbnail is
    /// present. Being recognized means only that the handler has determined that the file does
    /// contain a native thumbnail. The thumbnail data might be of a format that the file handler
    /// cannot (or does not) return a single contiguous block of thumbnail data. A possible case of
    /// this is a TIFF uncompressed thumbnail, the handler might not have logic to gather the various
    /// disjoint pieces of the thumbnail from the overall TIFF stream.
    ///
    /// \result Returns true if a recognized native thumbnail is presentand the thumbnail was
    /// cached by OpenFile. This requires that kXMPFiles_OpenCacheTNail be passed to OpenFile. Note
    /// that GetThumbnail can return true but still not return an actual thumbnail image, see the
    /// above note.
    
    bool GetThumbnail ( XMP_ThumbnailInfo * tnailInfo );
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Update the XMP.
    ///
    /// \c PutXMP supplies new XMP for the file. However, the file is not actully written until
    /// closed. The options provided when the file was opened determine if reconciliation is done
    /// with other forms of metadata. Overloads are provided to pass the XMP as an XMP
    /// object, a string object, or a "const char *" plus length.
    
    void PutXMP ( const SXMPMeta & xmpObj );

    void PutXMP ( const tStringObj & xmpPacket );
    
    void PutXMP ( XMP_StringPtr xmpPacket,
                  XMP_StringLen xmpLength = kXMP_UseNullTermination );
	
    //  --------------------------------------------------------------------------------------------
    /// \brief Determine if the XMP can be updated.
    ///
    /// \c CanPutXMP determines if the XMP can (probably) be updated. The provided XMP is only used
    /// to obtain the length of the serialized packet. The new XMP is not kept, calling this will
    /// not cause the file to be written when closed. Overloads are provided to pass the XMP as an
    /// XMP object, a string object, or a "const char *" plus length. This is implemented roughly
    /// as:
    ///
    /// \code
    ///  bool CanPutXMP ( XMP_StringPtr xmpPacket )
    ///  {
    /// 	XMP_FileFormat format;
	/// 	this->GetFileInfo ( 0, &format, 0 );
	///
	/// 	XMP_OptionBits formatFlags;
	/// 	GetFormatInfo ( format, &formatFlags );
    /// 
    /// 	if ( (formatFlags & kXMPFiles_CanInjectXMP) && (formatFlags & kXMPFiles_CanExpand) ) return true;
    ///
    /// 	XMP_PacketInfo packetInfo;
    /// 	bool hasXMP = this->GetXMP ( 0, 0, &packetInfo );
    ///
    /// 	if ( ! hasXMP ) {
    /// 		if ( formatFlags & kXMPFiles_CanInjectXMP ) return true;
    /// 	} else {
    /// 		if ( (formatFlags & kXMPFiles_CanExpand) ||
    /// 			 (packetInfo.length >= strlen(xmpPacket)) ) return true;
    /// 	}
    /// 
    /// 	return false;
    /// 
    ///  }
    /// \endcode
    
    bool CanPutXMP ( const SXMPMeta & xmpObj );

    bool CanPutXMP ( const tStringObj & xmpPacket );
    
    bool CanPutXMP ( XMP_StringPtr xmpPacket,
                     XMP_StringLen xmpLength = kXMP_UseNullTermination );

    /// @}

	// =============================================================================================

private:
	XMPFilesRef xmpFilesRef;

};	// class TXMPFiles

// =================================================================================================

#endif // __TXMPFiles_hpp__
