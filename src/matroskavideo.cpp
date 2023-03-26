// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"
#include "matroskavideo.hpp"
#include "tags.hpp"
#include "tags_int.hpp"

// + standard includes
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {

/*!
  Tag Look-up list for Matroska Type Video Files
  The Tags have been categorized in 4 categories. Which are
  mentioned as a comment in front of them.
  s   --  Tag to be Skipped
  sd  --  Tag to be Skipped along with its data
  u   --  Tag used directly for storing metadata
  ui  --  Tag used only internally

  see : https://www.matroska.org/technical/elements.html
        https://matroska.sourceforge.net/technical/specs/chapters/index.html
 */

enum matroskaEnum : uint64_t {
  ChapterDisplay = 0x0000,
  TrackType = 0x0003,
  ChapterString = 0x0005,
  Video_Audio_CodecID = 0x0006,
  TrackDefault = 0x0008,
  ChapterTrackNumber = 0x0009,
  Slices = 0x000e,
  ChapterTrack = 0x000f,
  ChapterTimeStart = 0x0011,
  ChapterTimeEnd = 0x0012,
  CueRefTime = 0x0016,
  CueRefCluster = 0x0017,
  ChapterFlagHidden = 0x0018,
  Xmp_video_VideoScanTpye = 0x001a,
  BlockDuration = 0x001b,
  TrackLacing = 0x001c,
  Xmp_audio_ChannelType = 0x001f,
  BlockGroup = 0x0020,
  Block = 0x0021,
  BlockVirtual = 0x0022,
  SimpleBlock = 0x0023,
  CodecState = 0x0024,
  BlockAdditional = 0x0025,
  BlockMore = 0x0026,
  Position = 0x0027,
  CodecDecodeAll = 0x002a,
  PrevSize = 0x002b,
  TrackEntry = 0x002e,
  EncryptedBlock = 0x002f,
  Xmp_video_Width_1 = 0x0030,
  CueTime = 0x0033,
  Xmp_audio_SampleRate = 0x0035,
  ChapterAtom = 0x0036,
  CueTrackPositions = 0x0037,
  TrackUsed = 0x0039,
  Xmp_video_Height_1 = 0x003a,
  CuePoint = 0x003b,
  CRC_32 = 0x003f,
  BlockAdditionalID = 0x004b,
  LaceNumber = 0x004c,
  FrameNumber = 0x004d,
  Delay = 0x004e,
  ClusterDuration = 0x004f,
  TrackNumber = 0x0057,
  CueReference = 0x005b,
  Video = 0x0060,
  Audio = 0x0061,
  Timecode = 0x0067,
  TimeSlice = 0x0068,
  CueCodecState = 0x006a,
  CueRefCodecState = 0x006b,
  Void = 0x006c,
  BlockAddID = 0x006e,
  CueClusterPosition = 0x0071,
  CueTrack = 0x0077,
  ReferencePriority = 0x007a,
  ReferenceBlock = 0x007b,
  ReferenceVirtual = 0x007d,
  Xmp_video_ContentCompressAlgo = 0x0254,
  ContentCompressionSettings = 0x0255,
  Xmp_video_DocType = 0x0282,
  Xmp_video_DocTypeReadVersion = 0x0285,
  Xmp_video_EBMLVersion = 0x0286,
  Xmp_video_DocTypeVersion = 0x0287,
  EBMLMaxIDLength = 0x02f2,
  EBMLMaxSizeLength = 0x02f3,
  Xmp_video_EBMLReadVersion = 0x02f7,
  ChapterLanguage = 0x037c,
  ChapterCountry = 0x037e,
  SegmentFamily = 0x0444,
  Xmp_video_DateUTC = 0x0461,
  Xmp_video_TagLanguage = 0x047a,
  Xmp_video_TagDefault = 0x0484,
  TagBinary = 0x0485,
  Xmp_video_TagString = 0x0487,
  Xmp_video_Duration = 0x0489,
  ChapterProcessPrivate = 0x050d,
  ChapterFlagEnabled = 0x0598,
  Xmp_video_TagName = 0x05a3,
  EditionEntry = 0x05b9,
  EditionUID = 0x05bc,
  EditionFlagHidden = 0x05bd,
  EditionFlagDefault = 0x05db,
  EditionFlagOrdered = 0x05dd,
  Xmp_video_AttachFileData = 0x065c,
  Xmp_video_AttachFileMIME = 0x0660,
  Xmp_video_AttachFileName = 0x066e,
  AttachedFileReferral = 0x0675,
  Xmp_video_AttachFileDesc = 0x067e,
  Xmp_video_AttachFileUID = 0x06ae,
  Xmp_video_ContentEncryptAlgo = 0x07e1,
  ContentEncryptionKeyID = 0x07e2,
  ContentSignature = 0x07e3,
  ContentSignatureKeyID = 0x07e4,
  Xmp_video_ContentSignAlgo_1 = 0x07e5,
  Xmp_video_ContentSignHashAlgo_1 = 0x07e6,
  Xmp_video_MuxingApp = 0x0d80,
  Seek = 0x0dbb,
  ContentEncodingOrder = 0x1031,
  ContentEncodingScope = 0x1032,
  Xmp_video_ContentEncodingType = 0x1033,
  ContentCompression = 0x1034,
  ContentEncryption = 0x1035,
  CueRefNumber = 0x135f,
  Xmp_video_TrackName = 0x136e,
  CueBlockNumber = 0x1378,
  TrackOffset = 0x137f,
  SeekID = 0x13ab,
  SeekPosition = 0x13ac,
  Stereo3DMode = 0x13b8,
  Xmp_video_CropBottom = 0x14aa,
  Xmp_video_Width_2 = 0x14b0,
  Xmp_video_DisplayUnit = 0x14b2,
  Xmp_video_AspectRatioType = 0x14b3,
  Xmp_video_Height_2 = 0x14ba,
  Xmp_video_CropTop = 0x14bb,
  Xmp_video_CropLeft = 0x14cc,
  Xmp_video_CropRight = 0x14dd,
  TrackForced = 0x15aa,
  MaxBlockAdditionID = 0x15ee,
  Xmp_video_WritingApp = 0x1741,
  SilentTracks = 0x1854,
  SilentTrackNumber = 0x18d7,
  AttachedFile = 0x21a7,
  ContentEncoding = 0x2240,
  Xmp_audio_BitsPerSample = 0x2264,
  CodecPrivate = 0x23a2,
  Targets = 0x23c0,
  Xmp_video_PhysicalEquivalent = 0x23c3,
  TagChapterUID = 0x23c4,
  TagTrackUID = 0x23c5,
  TagAttachmentUID = 0x23c6,
  TagEditionUID = 0x23c9,
  Xmp_video_TargetType = 0x23ca,
  SignedElement = 0x2532,
  TrackTranslate = 0x2624,
  TrackTranslateTrackID = 0x26a5,
  TrackTranslateCodec = 0x26bf,
  TrackTranslateEditionUID = 0x26fc,
  SimpleTag = 0x27c8,
  TargetTypeValue = 0x28ca,
  ChapterProcessCommand = 0x2911,
  ChapterProcessTime = 0x2922,
  ChapterTranslate = 0x2924,
  ChapterProcessData = 0x2933,
  ChapterProcess = 0x2944,
  ChapterProcessCodecID = 0x2955,
  ChapterTranslateID = 0x29a5,
  Xmp_video_TranslateCodec = 0x29bf,
  ChapterTranslateEditionUID = 0x29fc,
  ContentEncodings = 0x2d80,
  MinCache = 0x2de7,
  MaxCache = 0x2df8,
  ChapterSegmentUID = 0x2e67,
  ChapterSegmentEditionUID = 0x2ebc,
  TrackOverlay = 0x2fab,
  Tag = 0x3373,
  SegmentFileName = 0x3384,
  SegmentUID = 0x33a4,
  ChapterUID = 0x33c4,
  TrackUID = 0x33c5,
  TrackAttachmentUID = 0x3446,
  BlockAdditions = 0x35a1,
  Xmp_audio_OutputSampleRate = 0x38b5,
  Xmp_video_Title = 0x3ba9,
  ChannelPositions = 0x3d7b,
  SignatureElements = 0x3e5b,
  SignatureElementList = 0x3e7b,
  Xmp_video_ContentSignAlgo_2 = 0x3e8a,
  Xmp_video_ContentSignHashAlgo_2 = 0x3e9a,
  SignaturePublicKey = 0x3ea5,
  Signature = 0x3eb5,
  TrackLanguage = 0x2b59c,
  TrackTimecodeScale = 0x3314f,
  Xmp_video_FrameRate = 0x383e3,
  VideoFrameRate_DefaultDuration = 0x3e383,
  Video_Audio_CodecName = 0x58688,
  CodecDownloadURL = 0x6b240,
  TimecodeScale = 0xad7b1,
  ColorSpace = 0xeb524,
  Xmp_video_OpColor = 0xfb523,
  CodecSettings = 0x1a9697,
  CodecInfoURL = 0x1b4040,
  PrevFileName = 0x1c83ab,
  PrevUID = 0x1cb923,
  NextFileName = 0x1e83bb,
  NextUID = 0x1eb923,
  Chapters = 0x43a770,
  SeekHead = 0x14d9b74,
  Tags = 0x254c367,
  Info = 0x549a966,
  Tracks = 0x654ae6b,
  SegmentHeader = 0x8538067,
  Attachments = 0x941a469,
  EBMLHeader = 0xa45dfa3,
  SignatureSlot = 0xb538667,
  Cues = 0xc53bb6b,
  Cluster = 0xf43b675
};

const MatroskaTag matroskaTags[]{
    {ChapterDisplay, "ChapterDisplay", Master, Composite},
    {TrackType, "TrackType", Boolean, Process},
    {ChapterString, "ChapterString", String, Skip},
    {Video_Audio_CodecID, "Video.Audio.CodecID", InternalField, Skip},  // process
    {TrackDefault, "TrackDefault", Boolean, Process},
    {ChapterTrackNumber, "ChapterTrackNumber", UInteger, Skip},
    {Slices, "Slices", Master, Composite},
    {ChapterTrack, "ChapterTrack", Master, Composite},
    {ChapterTimeStart, "ChapterTimeStart", UInteger, Skip},
    {ChapterTimeEnd, "ChapterTimeEnd", UInteger, Skip},
    {CueRefTime, "CueRefTime", UInteger, Skip},
    {CueRefCluster, "CueRefCluster", UInteger, Skip},
    {ChapterFlagHidden, "ChapterFlagHidden", UInteger, Skip},
    {Xmp_video_VideoScanTpye, "Xmp.video.VideoScanTpye", InternalField, Process},
    {BlockDuration, "BlockDuration", UInteger, Skip},
    {TrackLacing, "TrackLacing", Boolean, Process},
    {Xmp_audio_ChannelType, "Xmp.audio.ChannelType", InternalField, Process},
    {BlockGroup, "BlockGroup", Master, Composite},
    {Block, "Block", Binary, Skip},
    {BlockVirtual, "BlockVirtual", Binary, Skip},
    {SimpleBlock, "SimpleBlock", Binary, Skip},
    {CodecState, "CodecState", Binary, Skip},
    {BlockAdditional, "BlockAdditional", UInteger, Skip},
    {BlockMore, "BlockMore", Master, Composite},
    {Position, "Position", UInteger, Skip},
    {CodecDecodeAll, "CodecDecodeAll", Boolean, Process},
    {PrevSize, "PrevSize", UInteger, Skip},
    {TrackEntry, "TrackEntry", Master, Composite},
    {EncryptedBlock, "EncryptedBlock", Binary, Skip},
    {Xmp_video_Width_1, "Xmp.video.Width", UInteger, Process},
    {CueTime, "CueTime", UInteger, Skip},
    {Xmp_audio_SampleRate, "Xmp.audio.SampleRate", Float, Process},
    {ChapterAtom, "ChapterAtom", Master, Composite},
    {CueTrackPositions, "CueTrackPositions", Master, Composite},
    {TrackUsed, "TrackUsed", Boolean, Process},
    {Xmp_video_Height_1, "Xmp.video.Height", Integer, Process},
    {CuePoint, "CuePoint", Master, Composite},
    {CRC_32, "CRC_32", Binary, Skip},
    {BlockAdditionalID, "BlockAdditionalID", UInteger, Skip},
    {LaceNumber, "LaceNumber", UInteger, Skip},
    {FrameNumber, "FrameNumber", UInteger, Skip},
    {Delay, "Delay", UInteger, Skip},
    {ClusterDuration, "ClusterDuration", Float, Skip},
    {TrackNumber, "Xmp.video.TotalStream", String, Process},
    {CueReference, "CueReference", Master, Composite},
    {Video, "Video", Master, Composite},
    {Audio, "Audio", Master, Composite},
    {Timecode, "Timecode", UInteger, Skip},
    {TimeSlice, "TimeSlice", Master, Composite},
    {CueCodecState, "CueCodecState", UInteger, Skip},
    {CueRefCodecState, "CueRefCodecState", UInteger, Skip},
    {Void, "Void", Binary, Skip},
    {BlockAddID, "BlockAddID", UInteger, Skip},
    {CueClusterPosition, "CueClusterPosition", UInteger, Skip},
    {CueTrack, "CueTrack", UInteger, Skip},
    {ReferencePriority, "ReferencePriority", UInteger, Skip},
    {ReferenceBlock, "ReferenceBlock", Integer, Skip},
    {ReferenceVirtual, "ReferenceVirtual", Integer, Skip},
    {Xmp_video_ContentCompressAlgo, "Xmp.video.ContentCompressAlgo", InternalField, Process},
    {ContentCompressionSettings, "ContentCompressionSettings", Binary, Skip},
    {Xmp_video_DocType, "Xmp.video.DocType", String, Process},
    {Xmp_video_DocTypeReadVersion, "Xmp.video.DocTypeReadVersion", Integer, Process},
    {Xmp_video_EBMLVersion, "Xmp.video.EBMLVersion", Integer, Process},
    {Xmp_video_DocTypeVersion, "Xmp.video.DocTypeVersion", Integer, Process},
    {EBMLMaxIDLength, "EBMLMaxIDLength", UInteger, Skip},
    {EBMLMaxSizeLength, "EBMLMaxSizeLength", UInteger, Skip},
    {Xmp_video_EBMLReadVersion, "Xmp.video.EBMLReadVersion", UInteger, Process},
    {ChapterLanguage, "ChapterLanguage", String, Skip},
    {ChapterCountry, "ChapterCountry", Utf8, Skip},
    {SegmentFamily, "SegmentFamily", Binary, Skip},
    {Xmp_video_DateUTC, "Xmp.video.DateUTC", Date, Process},
    {Xmp_video_TagLanguage, "Xmp.video.TagLanguage", String, Process},
    {Xmp_video_TagDefault, "Xmp.video.TagDefault", Boolean, Process},
    {TagBinary, "TagBinary", Binary, Skip},
    {Xmp_video_TagString, "Xmp.video.TagString", String, Process},
    {Xmp_video_Duration, "Xmp.video.Duration", Date, Process},
    {ChapterProcessPrivate, "ChapterProcessPrivate", Master, Skip},
    {ChapterFlagEnabled, "ChapterFlagEnabled", Boolean, Skip},
    {Xmp_video_TagName, "Xmp.video.TagName", String, Process},
    {EditionEntry, "EditionEntry", Master, Composite},
    {EditionUID, "EditionUID", UInteger, Skip},
    {EditionFlagHidden, "EditionFlagHidden", Boolean, Skip},
    {EditionFlagDefault, "EditionFlagDefault", Boolean, Skip},
    {EditionFlagOrdered, "EditionFlagOrdered", Boolean, Skip},
    {Xmp_video_AttachFileData, "Xmp.video.AttachFileData", String, Process},
    {Xmp_video_AttachFileMIME, "Xmp.video.AttachFileMIME", String, Process},
    {Xmp_video_AttachFileName, "Xmp.video.AttachFileName", String, Process},
    {AttachedFileReferral, "AttachedFileReferral", Binary, Skip},
    {Xmp_video_AttachFileDesc, "Xmp.video.AttachFileDesc", String, Process},
    {Xmp_video_AttachFileUID, "Xmp.video.AttachFileUID", UInteger, Process},
    {Xmp_video_ContentEncryptAlgo, "Xmp.video.ContentEncryptAlgo", InternalField, Process},
    {ContentEncryptionKeyID, "ContentEncryptionKeyID", Binary, Skip},
    {ContentSignature, "ContentSignature", Binary, Skip},
    {ContentSignatureKeyID, "ContentSignatureKeyID", Binary, Skip},
    {Xmp_video_ContentSignAlgo_1, "Xmp.video.ContentSignAlgo", InternalField, Process},
    {Xmp_video_ContentSignHashAlgo_1, "Xmp.video.ContentSignHashAlgo", InternalField, Process},
    {Xmp_video_MuxingApp, "Xmp.video.MuxingApp", String, Process},
    {Seek, "Seek", Master, Composite},
    {ContentEncodingOrder, "ContentEncodingOrder", UInteger, Skip},
    {ContentEncodingScope, "ContentEncodingScope", UInteger, Skip},
    {Xmp_video_ContentEncodingType, "Xmp.video.ContentEncodingType", InternalField, Process},
    {ContentCompression, "ContentCompression", Master, Composite},
    {ContentEncryption, "ContentEncryption", Master, Composite},
    {CueRefNumber, "CueRefNumber", UInteger, Skip},
    {Xmp_video_TrackName, "Xmp.video.TrackName", String, Process},
    {CueBlockNumber, "CueBlockNumber", UInteger, Skip},
    {TrackOffset, "TrackOffset", Integer, Skip},
    {SeekID, "SeekID", Binary, Skip},
    {SeekPosition, "SeekPosition", UInteger, Skip},
    {Stereo3DMode, "Stereo3DMode", UInteger, Skip},
    {Xmp_video_CropBottom, "Xmp.video.CropBottom", Integer, Process},
    {Xmp_video_Width_2, "Xmp.video.Width", Integer, Process},
    {Xmp_video_DisplayUnit, "Xmp.video.DisplayUnit", InternalField, Process},
    {Xmp_video_AspectRatioType, "Xmp.video.AspectRatioType", InternalField, Process},
    {Xmp_video_Height_2, "Xmp.video.Height", Integer, Process},
    {Xmp_video_CropTop, "Xmp.video.CropTop", Integer, Process},
    {Xmp_video_CropLeft, "Xmp.video.CropLeft", Integer, Process},
    {Xmp_video_CropRight, "Xmp.video.CropRight", Integer, Process},
    {TrackForced, "TrackForced", Boolean, Process},
    {MaxBlockAdditionID, "MaxBlockAdditionID", UInteger, Skip},
    {Xmp_video_WritingApp, "Xmp.video.WritingApp", String, Process},
    {SilentTracks, "SilentTracks", Master, Composite},
    {SilentTrackNumber, "SilentTrackNumber", UInteger, Skip},
    {AttachedFile, "AttachedFile", Master, Composite},
    {ContentEncoding, "ContentEncoding", Master, Composite},
    {Xmp_audio_BitsPerSample, "Xmp.audio.BitsPerSample", Integer, Process},
    {CodecPrivate, "CodecPrivate", Binary, Skip},
    {Targets, "Targets", Master, Composite},
    {Xmp_video_PhysicalEquivalent, "Xmp.video.PhysicalEquivalent", InternalField, Process},
    {TagChapterUID, "TagChapterUID", UInteger, Skip},
    {TagTrackUID, "TagTrackUID", UInteger, Skip},
    {TagAttachmentUID, "TagAttachmentUID", UInteger, Skip},
    {TagEditionUID, "TagEditionUID", UInteger, Skip},
    {Xmp_video_TargetType, "Xmp.video.TargetType", String, Process},
    {SignedElement, "SignedElement", Binary, Skip},
    {TrackTranslate, "TrackTranslate", Master, Composite},
    {TrackTranslateTrackID, "TrackTranslateTrackID", Binary, Skip},
    {TrackTranslateCodec, "TrackTranslateCodec", UInteger, Skip},
    {TrackTranslateEditionUID, "TrackTranslateEditionUID", UInteger, Skip},
    {SimpleTag, "SimpleTag", Master, Composite},
    {TargetTypeValue, "TargetTypeValue", UInteger, Skip},
    {ChapterProcessCommand, "ChapterProcessCommand", Master, Composite},
    {ChapterProcessTime, "ChapterProcessTime", UInteger, Skip},
    {ChapterTranslate, "ChapterTranslate", Master, Composite},
    {ChapterProcessData, "ChapterProcessData", Binary, Skip},
    {ChapterProcess, "ChapterProcess", Master, Composite},
    {ChapterProcessCodecID, "ChapterProcessCodecID", UInteger, Skip},
    {ChapterTranslateID, "ChapterTranslateID", Binary, Skip},
    {Xmp_video_TranslateCodec, "Xmp.video.TranslateCodec", InternalField, Process},
    {ChapterTranslateEditionUID, "ChapterTranslateEditionUID", UInteger, Skip},
    {ContentEncodings, "ContentEncodings", Master, Composite},
    {MinCache, "MinCache", UInteger, Skip},
    {MaxCache, "MaxCache", UInteger, Skip},
    {ChapterSegmentUID, "ChapterSegmentUID", Binary, Skip},
    {ChapterSegmentEditionUID, "ChapterSegmentEditionUID", UInteger, Skip},
    {TrackOverlay, "TrackOverlay", UInteger, Skip},
    {Tag, "Tag", Master, Composite},
    {SegmentFileName, "SegmentFileName", Utf8, Skip},
    {SegmentUID, "SegmentUID", Binary, Skip},
    {ChapterUID, "ChapterUID", UInteger, Skip},
    {TrackUID, "TrackUID", UInteger, Skip},
    {TrackAttachmentUID, "TrackAttachmentUID", UInteger, Skip},
    {BlockAdditions, "BlockAdditions", Master, Composite},
    {Xmp_audio_OutputSampleRate, "Xmp.audio.OutputSampleRate", Float, Process},
    {Xmp_video_Title, "Xmp.video.Title", String, Process},
    {ChannelPositions, "ChannelPositions", Binary, Skip},
    {SignatureElements, "SignatureElements", Master, Composite},
    {SignatureElementList, "SignatureElementList", Master, Composite},
    {Xmp_video_ContentSignAlgo_2, "Xmp.video.ContentSignAlgo", InternalField, Process},
    {Xmp_video_ContentSignHashAlgo_2, "Xmp.video.ContentSignHashAlgo", InternalField, Process},
    {SignaturePublicKey, "SignaturePublicKey", Binary, Skip},
    {Signature, "Signature", Binary, Skip},
    {TrackLanguage, "TrackLanguage", String,
     Skip},  // Process :  see values here https://www.loc.gov/standards/iso639-2/php/code_list.php
    {TrackTimecodeScale, "TrackTimecodeScale", Float, Skip},
    {Xmp_video_FrameRate, "Xmp.video.FrameRate", Float, Process},
    {VideoFrameRate_DefaultDuration, "VideoFrameRate.DefaultDuration", Float, Skip},
    {Video_Audio_CodecName, "Video.Audio.CodecName", InternalField, Process},
    {CodecDownloadURL, "CodecDownloadURL", InternalField, Process},
    {TimecodeScale, "Xmp.video.TimecodeScale", Date, Process},
    {ColorSpace, "ColorSpace", String, Process},
    {Xmp_video_OpColor, "Xmp.video.OpColor", Float, Skip},
    {CodecSettings, "CodecSettings", Boolean, Process},
    {CodecInfoURL, "CodecInfoURL", InternalField, Process},
    {PrevFileName, "PrevFileName", Utf8, Skip},
    {PrevUID, "PrevUID", Binary, Skip},
    {NextFileName, "NextFileName", Utf8, Skip},
    {NextUID, "NextUID", Binary, Skip},
    {Chapters, "Chapters", Master, Skip},
    {SeekHead, "SeekHead", Master, Composite},
    {Tags, "Tags", Master, Composite},
    {Info, "Info", Master, Composite},
    {Tracks, "Tracks", Master, Composite},
    {SegmentHeader, "SegmentHeader", Master, Composite},
    {Attachments, "Attachments", Master, Composite},
    {EBMLHeader, "EBMLHeader", Master, Composite},
    {SignatureSlot, "SignatureSlot", Master, Composite},
    {Cues, "Cues", Master, Composite},
    {Cluster, "Cluster", Master, Composite},
};

const MatroskaTag matroskaTrackType[] = {
    {0x1, "Video"},     {0x2, "Audio"},    {0x3, "Complex"},  {0x10, "Logo"},
    {0x11, "Subtitle"}, {0x12, "Buttons"}, {0x20, "Control"},
};

const MatroskaTag compressionAlgorithm[] = {
    {0, "zlib "},
    {1, "bzlib"},
    {2, "lzo1x"},
    {3, "Header Stripping"},
};

const MatroskaTag audioChannels[] = {
    {1, "Mono"},
    {2, "Stereo"},
    {5, "5.1 Surround Sound"},
    {7, "7.1 Surround Sound"},
};

const MatroskaTag displayUnit[] = {
    {0x0, "Pixels"}, {0x1, "cm"}, {0x2, "inches"}, {0x3, "display aspect ratio"}, {0x2, "unknown"},
};

const MatroskaTag encryptionAlgorithm[] = {
    {0, "Not Encrypted"}, {1, "DES"}, {2, "3DES"}, {3, "Twofish"}, {4, "Blowfish"}, {5, "AES"},
};

const MatroskaTag chapterPhysicalEquivalent[] = {
    {10, "Index"}, {20, "Track"}, {30, "Session"}, {40, "Layer"}, {50, "Side"}, {60, "CD / DVD"}, {70, "Set / Package"},
};

const MatroskaTag encodingType[] = {
    {0, "Compression"},
    {1, "Encryption"},
};

const MatroskaTag videoScanType[] = {
    {0, "Progressive"},
    {1, "Interlaced"},
};

const MatroskaTag chapterTranslateCodec[] = {
    {0, "Matroska Script"},
    {1, "DVD Menu"},
};

const MatroskaTag aspectRatioType[] = {
    {0, "Free Resizing"},
    {1, "Keep Aspect Ratio"},
    {2, "Fixed"},
};

const MatroskaTag contentSignatureAlgorithm[] = {
    {0, "Not Signed"},
    {1, "RSA"},
};

const MatroskaTag contentSignatureHashAlgorithm[] = {
    {0, "Not Signed"},
    {1, "SHA1-160"},
    {2, "MD5"},
};

const MatroskaTag trackEnable[] = {
    {0x1, "Xmp.video.Enabled"},
    {0x2, "Xmp.audio.Enabled"},
    {0x11, "Xmp.video.SubTEnabled"},
};

const MatroskaTag defaultOn[] = {
    {0x1, "Xmp.video.DefaultOn"},
    {0x2, "Xmp.audio.DefaultOn"},
    {0x11, "Xmp.video.SubTDefaultOn"},
};

const MatroskaTag trackForced[] = {
    {0x1, "Xmp.video.TrackForced"},
    {0x2, "Xmp.audio.TrackForced"},
    {0x11, "Xmp.video.SubTTrackForced"},
};

const MatroskaTag trackLacing[] = {
    {0x1, "Xmp.video.TrackLacing"},
    {0x2, "Xmp.audio.TrackLacing"},
    {0x11, "Xmp.video.SubTTrackLacing"},
};

const MatroskaTag codecDecodeAll[] = {
    {0x1, "Xmp.video.CodecDecodeAll"},
    {0x2, "Xmp.audio.CodecDecodeAll"},
    {0x11, "Xmp.video.SubTCodecDecodeAll"},
};

const MatroskaTag codecDownloadUrl[] = {
    {0x1, "Xmp.video.CodecDownloadUrl"},
    {0x2, "Xmp.audio.CodecDownloadUrl"},
    {0x11, "Xmp.video.SubTCodecDownloadUrl"},
};

const MatroskaTag codecSettings[] = {
    {0x1, "Xmp.video.CodecSettings"},
    {0x2, "Xmp.audio.CodecSettings"},
    {0x11, "Xmp.video.SubTCodecSettings"},
};

const MatroskaTag trackCodec[] = {
    {0x1, "Xmp.video.Codec"},
    {0x2, "Xmp.audio.Compressor"},
    {0x11, "Xmp.video.SubTCodec"},
};

const MatroskaTag codecInfo[] = {
    {0x1, "Xmp.video.CodecInfo"},
    {0x2, "Xmp.audio.CodecInfo"},
    {0x11, "Xmp.video.SubTCodecInfo"},
};

const MatroskaTag streamRate[] = {
    {0x1, "Xmp.video.FrameRate"},
    {0x2, "Xmp.audio.DefaultDuration"},
};

/*!
  @brief Function used to calculate Tags, Tags may comprise of more than
      one byte. The first byte calculates size of the Tag and the remaining
      bytes are used to calculate the rest of the Tag.
      Returns Tag Value.
 */
[[nodiscard]] static size_t returnTagValue(const byte* buf, size_t size) {
  enforce(size > 0 && size <= 8, Exiv2::ErrorCode::kerCorruptedMetadata);

  size_t b0 = buf[0] & (0xff >> size);
  size_t tag = b0 << ((size - 1) * 8);
  for (size_t i = 1; i < size; ++i) {
    tag |= static_cast<size_t>(buf[i]) << ((size - i - 1) * 8);
  }

  return tag;
}
}  // namespace Exiv2::Internal

namespace Exiv2 {

using namespace Exiv2::Internal;

MatroskaVideo::MatroskaVideo(BasicIo::UniquePtr io) : Image(ImageType::mkv, mdNone, std::move(io)) {
}  // MatroskaVideo::MatroskaVideo

std::string MatroskaVideo::mimeType() const {
  return "video/matroska";
}

void MatroskaVideo::writeMetadata() {
}

void MatroskaVideo::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());

  // Ensure that this is the correct image type
  if (!isMkvType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "Matroska");
  }

  IoCloser closer(*io_);
  clearMetadata();
  continueTraversing_ = true;
  height_ = width_ = 1;

  xmpData_["Xmp.video.FileSize"] = io_->size() / bytesMB;
  xmpData_["Xmp.video.MimeType"] = mimeType();

  while (continueTraversing_)
    decodeBlock();

  xmpData_["Xmp.video.AspectRatio"] = getAspectRatio(width_, height_);
}

void MatroskaVideo::decodeBlock() {
  byte buf[8];
  io_->read(buf, 1);

  if (io_->eof()) {
    continueTraversing_ = false;
    return;
  }

  uint32_t block_size = findBlockSize(buf[0]);  // 0-8
  if (block_size > 0)
    io_->read(buf + 1, block_size - 1);

  auto tag_id = returnTagValue(buf, block_size);
  const MatroskaTag* tag = Exiv2::find(matroskaTags, tag_id);

  if (!tag) {
    continueTraversing_ = false;
    return;
  }

  // tag->dump(std::cout);

  if (tag->_id == Cues || tag->_id == Cluster) {
    continueTraversing_ = false;
    return;
  }

  io_->read(buf, 1);
  block_size = findBlockSize(buf[0]);  // 0-8

  if (block_size > 0)
    io_->read(buf + 1, block_size - 1);
  size_t size = returnTagValue(buf, block_size);

  if (tag->isComposite() && !tag->isSkipped())
    return;

  const size_t bufMaxSize = 200;

#ifndef SUPPRESS_WARNINGS
  if (!tag->isSkipped() && size > bufMaxSize) {
    EXV_WARNING << "Size " << size << " of Matroska tag 0x" << std::hex << tag->_id << std::dec << " is greater than "
                << bufMaxSize << ": ignoring it.\n";
  }
#endif
  if (tag->isSkipped() || size > bufMaxSize) {
    io_->seek(size, BasicIo::cur);
    return;
  }

  DataBuf buf2(bufMaxSize + 1);
  io_->read(buf2.data(), size);
  switch (tag->_type) {
    case InternalField:
      decodeInternalTags(tag, buf2.data());
      break;
    case String:
    case Utf8:
      decodeStringTags(tag, buf2.data());
      break;
    case Integer:
    case UInteger:
      decodeIntegerTags(tag, buf2.data());
      break;
    case Boolean:
      decodeBooleanTags(tag, buf2.data());
      break;
    case Date:
      decodeDateTags(tag, buf2.data(), size);
      break;
    case Float:
      decodeFloatTags(tag, buf2.data());
      break;
    case Binary:
      break;
    case Master:
      break;
    default:
      break;
  }
}  // MatroskaVideo::decodeBlock

void MatroskaVideo::decodeInternalTags(const MatroskaTag* tag, const byte* buf) {
  uint64_t key = getULongLong(buf, bigEndian);
  if (!key)
    return;

  auto internalMt = [=]() -> const MatroskaTag* {
    switch (tag->_id) {
      case Xmp_video_VideoScanTpye:
        return Exiv2::find(videoScanType, key);
      case Xmp_audio_ChannelType:
        return Exiv2::find(audioChannels, key);
      case Xmp_video_ContentCompressAlgo:
        return Exiv2::find(compressionAlgorithm, key);
      case Xmp_video_ContentEncryptAlgo:
        return Exiv2::find(encryptionAlgorithm, key);
      case Xmp_video_ContentSignAlgo_1:
      case Xmp_video_ContentSignAlgo_2:
        return Exiv2::find(contentSignatureAlgorithm, key);
      case Xmp_video_ContentSignHashAlgo_1:
      case Xmp_video_ContentSignHashAlgo_2:
        return Exiv2::find(contentSignatureHashAlgorithm, key);
      case Xmp_video_ContentEncodingType:
        return Exiv2::find(encodingType, key);
      case Xmp_video_DisplayUnit:
        return Exiv2::find(displayUnit, key);
      case Xmp_video_AspectRatioType:
        return Exiv2::find(aspectRatioType, key);
      case Xmp_video_PhysicalEquivalent:
        return Exiv2::find(chapterPhysicalEquivalent, key);
      case Xmp_video_TranslateCodec:
        return Exiv2::find(chapterTranslateCodec, key);
      case Video_Audio_CodecID:
        return Exiv2::find(trackCodec, key);
      case Video_Audio_CodecName:
        return Exiv2::find(codecInfo, key);
      case CodecDownloadURL:
      case CodecInfoURL:
        return Exiv2::find(codecDownloadUrl, key);
    }
    return nullptr;
  }();
  if (internalMt) {
    xmpData_[tag->_label] = internalMt->_label;
  } else {
    xmpData_[tag->_label] = key;
  }
}

void MatroskaVideo::decodeStringTags(const MatroskaTag* tag, const byte* buf) {
  if (tag->_id == TrackNumber) {
    track_count_++;
    xmpData_[tag->_label] = track_count_;
  } else {
    xmpData_[tag->_label] = buf;
  }
}

void MatroskaVideo::decodeIntegerTags(const MatroskaTag* tag, const byte* buf) {
  uint64_t value = getULongLong(buf, bigEndian);
  if (!value)
    return;

  if (tag->_id == Xmp_video_Width_1 || tag->_id == Xmp_video_Width_2)
    width_ = value;
  if (tag->_id == Xmp_video_Height_1 || tag->_id == Xmp_video_Height_2)
    height_ = value;
  xmpData_[tag->_label] = value;
}

void MatroskaVideo::decodeBooleanTags(const MatroskaTag* tag, const byte* buf) {
  const MatroskaTag* internalMt = nullptr;
  uint64_t key = getULongLong(buf, bigEndian);
  if (!key)
    return;

  switch (tag->_id) {
    case TrackType:  // this tags is used internally only to deduce the type of track (video or audio)
      if (auto f = Exiv2::find(matroskaTrackType, key)) {
        stream_ = f->_id;
      }
      break;
    case TrackUsed:
      internalMt = Exiv2::find(trackEnable, key);
      break;
    case TrackDefault:
      internalMt = Exiv2::find(defaultOn, key);
      break;
    case TrackForced:
      internalMt = Exiv2::find(trackForced, key);
      break;
    case TrackLacing:
      internalMt = Exiv2::find(trackLacing, key);
      break;
    case CodecDecodeAll:
      internalMt = Exiv2::find(codecDecodeAll, key);
      break;
    case CodecSettings:
      internalMt = Exiv2::find(codecSettings, key);
      break;
    case Xmp_video_TagDefault:
      internalMt = tag;
      break;
    default:
      break;
  }

  if (internalMt) {
    xmpData_[internalMt->_label] = "Yes";
  }
}

void MatroskaVideo::decodeDateTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  int64_t duration_in_ms = 0;
  uint64_t value;
  switch (tag->_id) {
    case Xmp_video_Duration:
      if (size <= 4) {
        duration_in_ms =
            static_cast<int64_t>(getFloat(buf, bigEndian) * static_cast<float>(time_code_scale_) * 1000.0f);
      } else {
        duration_in_ms = static_cast<int64_t>(getDouble(buf, bigEndian) * time_code_scale_ * 1000);
      }
      xmpData_[tag->_label] = duration_in_ms;
      break;
    case Xmp_video_DateUTC:
      value = getULongLong(buf, bigEndian);
      if (!value)
        return;
      duration_in_ms = value / 1000000000;
      xmpData_[tag->_label] = duration_in_ms;
      break;

    case TimecodeScale:
      value = getULongLong(buf, bigEndian);
      if (!value)
        return;
      time_code_scale_ = static_cast<double>(value) / static_cast<double>(1000000000);
      xmpData_[tag->_label] = time_code_scale_;
      break;
    default:
      break;
  }
}

void MatroskaVideo::decodeFloatTags(const MatroskaTag* tag, const byte* buf) {
  xmpData_[tag->_label] = getFloat(buf, bigEndian);

  double frame_rate = 0;
  switch (tag->_id) {
    case Xmp_audio_SampleRate:
    case Xmp_audio_OutputSampleRate:
      xmpData_[tag->_label] = getFloat(buf, bigEndian);
      break;
    case VideoFrameRate_DefaultDuration:
    case Xmp_video_FrameRate: {
      uint64_t key = getULongLong(buf, bigEndian);
      if (!key)
        return;
      if (auto internalMt = Exiv2::find(streamRate, key)) {
        switch (stream_) {
          case 1:  // video
            frame_rate = static_cast<double>(1000000000) / static_cast<double>(key);
            break;
          case 2:  // audio
            frame_rate = static_cast<double>(key) / 1000;
            break;
          default:
            break;
        }
        if (frame_rate)
          xmpData_[internalMt->_label] = frame_rate;
      } else
        xmpData_[tag->_label] = "Variable Bit Rate";
    } break;
    default:
      xmpData_[tag->_label] = getFloat(buf, bigEndian);
      break;
  }
}

uint32_t MatroskaVideo::findBlockSize(byte b) {
  if (b & 128)
    return 1;
  if (b & 64)
    return 2;
  if (b & 32)
    return 3;
  if (b & 16)
    return 4;
  if (b & 8)
    return 5;
  if (b & 4)
    return 6;
  if (b & 2)
    return 7;
  if (b & 1)
    return 8;
  return 0;
}

Image::UniquePtr newMkvInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<MatroskaVideo>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isMkvType(BasicIo& iIo, bool advance) {
  bool result = true;
  byte tmpBuf[4];
  iIo.read(tmpBuf, 4);

  if (iIo.error() || iIo.eof())
    return false;

  if (0x1a != tmpBuf[0] || 0x45 != tmpBuf[1] || 0xdf != tmpBuf[2] || 0xa3 != tmpBuf[3]) {
    result = false;
  }

  if (!advance || !result)
    iIo.seek(0, BasicIo::beg);
  return result;
}

}  // namespace Exiv2
