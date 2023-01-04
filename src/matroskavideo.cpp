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
#include "error.hpp"
#include "futils.hpp"
#include "matroskavideo.hpp"
#include "tags.hpp"
#include "tags_int.hpp"

// + standard includes
#include <array>
#include <cassert>
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

const std::array<MatroskaTag, 198> matroskaTags = {
    MatroskaTag(ChapterDisplay, "ChapterDisplay", Master, Composite),
    MatroskaTag(TrackType, "TrackType", Boolean, Process),
    MatroskaTag(ChapterString, "ChapterString", String, Skip),
    MatroskaTag(Video_Audio_CodecID, "Video.Audio.CodecID", InternalField, Skip),  // process
    MatroskaTag(TrackDefault, "TrackDefault", Boolean, Process),
    MatroskaTag(ChapterTrackNumber, "ChapterTrackNumber", UInteger, Skip),
    MatroskaTag(Slices, "Slices", Master, Composite),
    MatroskaTag(ChapterTrack, "ChapterTrack", Master, Composite),
    MatroskaTag(ChapterTimeStart, "ChapterTimeStart", UInteger, Skip),
    MatroskaTag(ChapterTimeEnd, "ChapterTimeEnd", UInteger, Skip),
    MatroskaTag(CueRefTime, "CueRefTime", UInteger, Skip),
    MatroskaTag(CueRefCluster, "CueRefCluster", UInteger, Skip),
    MatroskaTag(ChapterFlagHidden, "ChapterFlagHidden", UInteger, Skip),
    MatroskaTag(Xmp_video_VideoScanTpye, "Xmp.video.VideoScanTpye", InternalField, Process),
    MatroskaTag(BlockDuration, "BlockDuration", UInteger, Skip),
    MatroskaTag(TrackLacing, "TrackLacing", Boolean, Process),
    MatroskaTag(Xmp_audio_ChannelType, "Xmp.audio.ChannelType", InternalField, Process),
    MatroskaTag(BlockGroup, "BlockGroup", Master, Composite),
    MatroskaTag(Block, "Block", Binary, Skip),
    MatroskaTag(BlockVirtual, "BlockVirtual", Binary, Skip),
    MatroskaTag(SimpleBlock, "SimpleBlock", Binary, Skip),
    MatroskaTag(CodecState, "CodecState", Binary, Skip),
    MatroskaTag(BlockAdditional, "BlockAdditional", UInteger, Skip),
    MatroskaTag(BlockMore, "BlockMore", Master, Composite),
    MatroskaTag(Position, "Position", UInteger, Skip),
    MatroskaTag(CodecDecodeAll, "CodecDecodeAll", Boolean, Process),
    MatroskaTag(PrevSize, "PrevSize", UInteger, Skip),
    MatroskaTag(TrackEntry, "TrackEntry", Master, Composite),
    MatroskaTag(EncryptedBlock, "EncryptedBlock", Binary, Skip),
    MatroskaTag(Xmp_video_Width_1, "Xmp.video.Width", UInteger, Process),
    MatroskaTag(CueTime, "CueTime", UInteger, Skip),
    MatroskaTag(Xmp_audio_SampleRate, "Xmp.audio.SampleRate", Float, Process),
    MatroskaTag(ChapterAtom, "ChapterAtom", Master, Composite),
    MatroskaTag(CueTrackPositions, "CueTrackPositions", Master, Composite),
    MatroskaTag(TrackUsed, "TrackUsed", Boolean, Process),
    MatroskaTag(Xmp_video_Height_1, "Xmp.video.Height", Integer, Process),
    MatroskaTag(CuePoint, "CuePoint", Master, Composite),
    MatroskaTag(CRC_32, "CRC_32", Binary, Skip),
    MatroskaTag(BlockAdditionalID, "BlockAdditionalID", UInteger, Skip),
    MatroskaTag(LaceNumber, "LaceNumber", UInteger, Skip),
    MatroskaTag(FrameNumber, "FrameNumber", UInteger, Skip),
    MatroskaTag(Delay, "Delay", UInteger, Skip),
    MatroskaTag(ClusterDuration, "ClusterDuration", Float, Skip),
    MatroskaTag(TrackNumber, "Xmp.video.TotalStream", String, Process),
    MatroskaTag(CueReference, "CueReference", Master, Composite),
    MatroskaTag(Video, "Video", Master, Composite),
    MatroskaTag(Audio, "Audio", Master, Composite),
    MatroskaTag(Timecode, "Timecode", UInteger, Skip),
    MatroskaTag(TimeSlice, "TimeSlice", Master, Composite),
    MatroskaTag(CueCodecState, "CueCodecState", UInteger, Skip),
    MatroskaTag(CueRefCodecState, "CueRefCodecState", UInteger, Skip),
    MatroskaTag(Void, "Void", Binary, Skip),
    MatroskaTag(BlockAddID, "BlockAddID", UInteger, Skip),
    MatroskaTag(CueClusterPosition, "CueClusterPosition", UInteger, Skip),
    MatroskaTag(CueTrack, "CueTrack", UInteger, Skip),
    MatroskaTag(ReferencePriority, "ReferencePriority", UInteger, Skip),
    MatroskaTag(ReferenceBlock, "ReferenceBlock", Integer, Skip),
    MatroskaTag(ReferenceVirtual, "ReferenceVirtual", Integer, Skip),
    MatroskaTag(Xmp_video_ContentCompressAlgo, "Xmp.video.ContentCompressAlgo", InternalField, Process),
    MatroskaTag(ContentCompressionSettings, "ContentCompressionSettings", Binary, Skip),
    MatroskaTag(Xmp_video_DocType, "Xmp.video.DocType", String, Process),
    MatroskaTag(Xmp_video_DocTypeReadVersion, "Xmp.video.DocTypeReadVersion", Integer, Process),
    MatroskaTag(Xmp_video_EBMLVersion, "Xmp.video.EBMLVersion", Integer, Process),
    MatroskaTag(Xmp_video_DocTypeVersion, "Xmp.video.DocTypeVersion", Integer, Process),
    MatroskaTag(EBMLMaxIDLength, "EBMLMaxIDLength", UInteger, Skip),
    MatroskaTag(EBMLMaxSizeLength, "EBMLMaxSizeLength", UInteger, Skip),
    MatroskaTag(Xmp_video_EBMLReadVersion, "Xmp.video.EBMLReadVersion", UInteger, Process),
    MatroskaTag(ChapterLanguage, "ChapterLanguage", String, Skip),
    MatroskaTag(ChapterCountry, "ChapterCountry", Utf8, Skip),
    MatroskaTag(SegmentFamily, "SegmentFamily", Binary, Skip),
    MatroskaTag(Xmp_video_DateUTC, "Xmp.video.DateUTC", Date, Process),
    MatroskaTag(Xmp_video_TagLanguage, "Xmp.video.TagLanguage", String, Process),
    MatroskaTag(Xmp_video_TagDefault, "Xmp.video.TagDefault", Boolean, Process),
    MatroskaTag(TagBinary, "TagBinary", Binary, Skip),
    MatroskaTag(Xmp_video_TagString, "Xmp.video.TagString", String, Process),
    MatroskaTag(Xmp_video_Duration, "Xmp.video.Duration", Date, Process),
    MatroskaTag(ChapterProcessPrivate, "ChapterProcessPrivate", Master, Skip),
    MatroskaTag(ChapterFlagEnabled, "ChapterFlagEnabled", Boolean, Skip),
    MatroskaTag(Xmp_video_TagName, "Xmp.video.TagName", String, Process),
    MatroskaTag(EditionEntry, "EditionEntry", Master, Composite),
    MatroskaTag(EditionUID, "EditionUID", UInteger, Skip),
    MatroskaTag(EditionFlagHidden, "EditionFlagHidden", Boolean, Skip),
    MatroskaTag(EditionFlagDefault, "EditionFlagDefault", Boolean, Skip),
    MatroskaTag(EditionFlagOrdered, "EditionFlagOrdered", Boolean, Skip),
    MatroskaTag(Xmp_video_AttachFileData, "Xmp.video.AttachFileData", String, Process),
    MatroskaTag(Xmp_video_AttachFileMIME, "Xmp.video.AttachFileMIME", String, Process),
    MatroskaTag(Xmp_video_AttachFileName, "Xmp.video.AttachFileName", String, Process),
    MatroskaTag(AttachedFileReferral, "AttachedFileReferral", Binary, Skip),
    MatroskaTag(Xmp_video_AttachFileDesc, "Xmp.video.AttachFileDesc", String, Process),
    MatroskaTag(Xmp_video_AttachFileUID, "Xmp.video.AttachFileUID", UInteger, Process),
    MatroskaTag(Xmp_video_ContentEncryptAlgo, "Xmp.video.ContentEncryptAlgo", InternalField, Process),
    MatroskaTag(ContentEncryptionKeyID, "ContentEncryptionKeyID", Binary, Skip),
    MatroskaTag(ContentSignature, "ContentSignature", Binary, Skip),
    MatroskaTag(ContentSignatureKeyID, "ContentSignatureKeyID", Binary, Skip),
    MatroskaTag(Xmp_video_ContentSignAlgo_1, "Xmp.video.ContentSignAlgo", InternalField, Process),
    MatroskaTag(Xmp_video_ContentSignHashAlgo_1, "Xmp.video.ContentSignHashAlgo", InternalField, Process),
    MatroskaTag(Xmp_video_MuxingApp, "Xmp.video.MuxingApp", String, Process),
    MatroskaTag(Seek, "Seek", Master, Composite),
    MatroskaTag(ContentEncodingOrder, "ContentEncodingOrder", UInteger, Skip),
    MatroskaTag(ContentEncodingScope, "ContentEncodingScope", UInteger, Skip),
    MatroskaTag(Xmp_video_ContentEncodingType, "Xmp.video.ContentEncodingType", InternalField, Process),
    MatroskaTag(ContentCompression, "ContentCompression", Master, Composite),
    MatroskaTag(ContentEncryption, "ContentEncryption", Master, Composite),
    MatroskaTag(CueRefNumber, "CueRefNumber", UInteger, Skip),
    MatroskaTag(Xmp_video_TrackName, "Xmp.video.TrackName", String, Process),
    MatroskaTag(CueBlockNumber, "CueBlockNumber", UInteger, Skip),
    MatroskaTag(TrackOffset, "TrackOffset", Integer, Skip),
    MatroskaTag(SeekID, "SeekID", Binary, Skip),
    MatroskaTag(SeekPosition, "SeekPosition", UInteger, Skip),
    MatroskaTag(Stereo3DMode, "Stereo3DMode", UInteger, Skip),
    MatroskaTag(Xmp_video_CropBottom, "Xmp.video.CropBottom", Integer, Process),
    MatroskaTag(Xmp_video_Width_2, "Xmp.video.Width", Integer, Process),
    MatroskaTag(Xmp_video_DisplayUnit, "Xmp.video.DisplayUnit", InternalField, Process),
    MatroskaTag(Xmp_video_AspectRatioType, "Xmp.video.AspectRatioType", InternalField, Process),
    MatroskaTag(Xmp_video_Height_2, "Xmp.video.Height", Integer, Process),
    MatroskaTag(Xmp_video_CropTop, "Xmp.video.CropTop", Integer, Process),
    MatroskaTag(Xmp_video_CropLeft, "Xmp.video.CropLeft", Integer, Process),
    MatroskaTag(Xmp_video_CropRight, "Xmp.video.CropRight", Integer, Process),
    MatroskaTag(TrackForced, "TrackForced", Boolean, Process),
    MatroskaTag(MaxBlockAdditionID, "MaxBlockAdditionID", UInteger, Skip),
    MatroskaTag(Xmp_video_WritingApp, "Xmp.video.WritingApp", String, Process),
    MatroskaTag(SilentTracks, "SilentTracks", Master, Composite),
    MatroskaTag(SilentTrackNumber, "SilentTrackNumber", UInteger, Skip),
    MatroskaTag(AttachedFile, "AttachedFile", Master, Composite),
    MatroskaTag(ContentEncoding, "ContentEncoding", Master, Composite),
    MatroskaTag(Xmp_audio_BitsPerSample, "Xmp.audio.BitsPerSample", Integer, Process),
    MatroskaTag(CodecPrivate, "CodecPrivate", Binary, Skip),
    MatroskaTag(Targets, "Targets", Master, Composite),
    MatroskaTag(Xmp_video_PhysicalEquivalent, "Xmp.video.PhysicalEquivalent", InternalField, Process),
    MatroskaTag(TagChapterUID, "TagChapterUID", UInteger, Skip),
    MatroskaTag(TagTrackUID, "TagTrackUID", UInteger, Skip),
    MatroskaTag(TagAttachmentUID, "TagAttachmentUID", UInteger, Skip),
    MatroskaTag(TagEditionUID, "TagEditionUID", UInteger, Skip),
    MatroskaTag(Xmp_video_TargetType, "Xmp.video.TargetType", String, Process),
    MatroskaTag(SignedElement, "SignedElement", Binary, Skip),
    MatroskaTag(TrackTranslate, "TrackTranslate", Master, Composite),
    MatroskaTag(TrackTranslateTrackID, "TrackTranslateTrackID", Binary, Skip),
    MatroskaTag(TrackTranslateCodec, "TrackTranslateCodec", UInteger, Skip),
    MatroskaTag(TrackTranslateEditionUID, "TrackTranslateEditionUID", UInteger, Skip),
    MatroskaTag(SimpleTag, "SimpleTag", Master, Composite),
    MatroskaTag(TargetTypeValue, "TargetTypeValue", UInteger, Skip),
    MatroskaTag(ChapterProcessCommand, "ChapterProcessCommand", Master, Composite),
    MatroskaTag(ChapterProcessTime, "ChapterProcessTime", UInteger, Skip),
    MatroskaTag(ChapterTranslate, "ChapterTranslate", Master, Composite),
    MatroskaTag(ChapterProcessData, "ChapterProcessData", Binary, Skip),
    MatroskaTag(ChapterProcess, "ChapterProcess", Master, Composite),
    MatroskaTag(ChapterProcessCodecID, "ChapterProcessCodecID", UInteger, Skip),
    MatroskaTag(ChapterTranslateID, "ChapterTranslateID", Binary, Skip),
    MatroskaTag(Xmp_video_TranslateCodec, "Xmp.video.TranslateCodec", InternalField, Process),
    MatroskaTag(ChapterTranslateEditionUID, "ChapterTranslateEditionUID", UInteger, Skip),
    MatroskaTag(ContentEncodings, "ContentEncodings", Master, Composite),
    MatroskaTag(MinCache, "MinCache", UInteger, Skip),
    MatroskaTag(MaxCache, "MaxCache", UInteger, Skip),
    MatroskaTag(ChapterSegmentUID, "ChapterSegmentUID", Binary, Skip),
    MatroskaTag(ChapterSegmentEditionUID, "ChapterSegmentEditionUID", UInteger, Skip),
    MatroskaTag(TrackOverlay, "TrackOverlay", UInteger, Skip),
    MatroskaTag(Tag, "Tag", Master, Composite),
    MatroskaTag(SegmentFileName, "SegmentFileName", Utf8, Skip),
    MatroskaTag(SegmentUID, "SegmentUID", Binary, Skip),
    MatroskaTag(ChapterUID, "ChapterUID", UInteger, Skip),
    MatroskaTag(TrackUID, "TrackUID", UInteger, Skip),
    MatroskaTag(TrackAttachmentUID, "TrackAttachmentUID", UInteger, Skip),
    MatroskaTag(BlockAdditions, "BlockAdditions", Master, Composite),
    MatroskaTag(Xmp_audio_OutputSampleRate, "Xmp.audio.OutputSampleRate", Float, Process),
    MatroskaTag(Xmp_video_Title, "Xmp.video.Title", String, Process),
    MatroskaTag(ChannelPositions, "ChannelPositions", Binary, Skip),
    MatroskaTag(SignatureElements, "SignatureElements", Master, Composite),
    MatroskaTag(SignatureElementList, "SignatureElementList", Master, Composite),
    MatroskaTag(Xmp_video_ContentSignAlgo_2, "Xmp.video.ContentSignAlgo", InternalField, Process),
    MatroskaTag(Xmp_video_ContentSignHashAlgo_2, "Xmp.video.ContentSignHashAlgo", InternalField, Process),
    MatroskaTag(SignaturePublicKey, "SignaturePublicKey", Binary, Skip),
    MatroskaTag(Signature, "Signature", Binary, Skip),
    MatroskaTag(TrackLanguage, "TrackLanguage", String,
                Skip),  // Process :  see values here https://www.loc.gov/standards/iso639-2/php/code_list.php
    MatroskaTag(TrackTimecodeScale, "TrackTimecodeScale", Float, Skip),
    MatroskaTag(Xmp_video_FrameRate, "Xmp.video.FrameRate", Float, Process),
    MatroskaTag(VideoFrameRate_DefaultDuration, "VideoFrameRate.DefaultDuration", Float, Skip),
    MatroskaTag(Video_Audio_CodecName, "Video.Audio.CodecName", InternalField, Process),
    MatroskaTag(CodecDownloadURL, "CodecDownloadURL", InternalField, Process),
    MatroskaTag(TimecodeScale, "Xmp.video.TimecodeScale", Date, Process),
    MatroskaTag(ColorSpace, "ColorSpace", String, Process),
    MatroskaTag(Xmp_video_OpColor, "Xmp.video.OpColor", Float, Skip),
    MatroskaTag(CodecSettings, "CodecSettings", Boolean, Process),
    MatroskaTag(CodecInfoURL, "CodecInfoURL", InternalField, Process),
    MatroskaTag(PrevFileName, "PrevFileName", Utf8, Skip),
    MatroskaTag(PrevUID, "PrevUID", Binary, Skip),
    MatroskaTag(NextFileName, "NextFileName", Utf8, Skip),
    MatroskaTag(NextUID, "NextUID", Binary, Skip),
    MatroskaTag(Chapters, "Chapters", Master, Skip),
    MatroskaTag(SeekHead, "SeekHead", Master, Composite),
    MatroskaTag(Tags, "Tags", Master, Composite),
    MatroskaTag(Info, "Info", Master, Composite),
    MatroskaTag(Tracks, "Tracks", Master, Composite),
    MatroskaTag(SegmentHeader, "SegmentHeader", Master, Composite),
    MatroskaTag(Attachments, "Attachments", Master, Composite),
    MatroskaTag(EBMLHeader, "EBMLHeader", Master, Composite),
    MatroskaTag(SignatureSlot, "SignatureSlot", Master, Composite),
    MatroskaTag(Cues, "Cues", Master, Composite),
    MatroskaTag(Cluster, "Cluster", Master, Composite)};

std::array<MatroskaTag, 7> matroskaTrackType = {
    MatroskaTag(0x1, "Video"),     MatroskaTag(0x2, "Audio"),    MatroskaTag(0x3, "Complex"), MatroskaTag(0x10, "Logo"),
    MatroskaTag(0x11, "Subtitle"), MatroskaTag(0x12, "Buttons"), MatroskaTag(0x20, "Control")};

const std::array<MatroskaTag, 4> compressionAlgorithm = {MatroskaTag(0, "zlib "), MatroskaTag(1, "bzlib"),
                                                         MatroskaTag(2, "lzo1x"), MatroskaTag(3, "Header Stripping")};

const std::array<MatroskaTag, 4> audioChannels = {MatroskaTag(1, "Mono"), MatroskaTag(2, "Stereo"),
                                                  MatroskaTag(5, "5.1 Surround Sound"),
                                                  MatroskaTag(7, "7.1 Surround Sound")};

const std::array<MatroskaTag, 5> displayUnit = {MatroskaTag(0x0, "Pixels"), MatroskaTag(0x1, "cm"),
                                                MatroskaTag(0x2, "inches"), MatroskaTag(0x3, "display aspect ratio"),
                                                MatroskaTag(0x2, "unknown")};

const std::array<MatroskaTag, 6> encryptionAlgorithm = {MatroskaTag(0, "Not Encrypted"), MatroskaTag(1, "DES"),
                                                        MatroskaTag(2, "3DES"),          MatroskaTag(3, "Twofish"),
                                                        MatroskaTag(4, "Blowfish"),      MatroskaTag(5, "AES")};

const std::array<MatroskaTag, 7> chapterPhysicalEquivalent = {
    MatroskaTag(10, "Index"), MatroskaTag(20, "Track"),    MatroskaTag(30, "Session"),      MatroskaTag(40, "Layer"),
    MatroskaTag(50, "Side"),  MatroskaTag(60, "CD / DVD"), MatroskaTag(70, "Set / Package")};

const std::array<MatroskaTag, 2> encodingType = {MatroskaTag(0, "Compression"), MatroskaTag(1, "Encryption")};

const std::array<MatroskaTag, 2> videoScanType = {MatroskaTag(0, "Progressive"), MatroskaTag(1, "Interlaced")};

const std::array<MatroskaTag, 2> chapterTranslateCodec = {MatroskaTag(0, "Matroska Script"),
                                                          MatroskaTag(1, "DVD Menu")};

const std::array<MatroskaTag, 3> aspectRatioType = {MatroskaTag(0, "Free Resizing"),
                                                    MatroskaTag(1, "Keep Aspect Ratio"), MatroskaTag(2, "Fixed")};

const std::array<MatroskaTag, 2> contentSignatureAlgorithm = {MatroskaTag(0, "Not Signed"), MatroskaTag(1, "RSA")};

const std::array<MatroskaTag, 3> contentSignatureHashAlgorithm = {MatroskaTag(0, "Not Signed"),
                                                                  MatroskaTag(1, "SHA1-160"), MatroskaTag(2, "MD5")};

const std::array<MatroskaTag, 3> trackEnable = {MatroskaTag(0x1, "Xmp.video.Enabled"),
                                                MatroskaTag(0x2, "Xmp.audio.Enabled"),
                                                MatroskaTag(0x11, "Xmp.video.SubTEnabled")};

const std::array<MatroskaTag, 3> defaultOn = {MatroskaTag(0x1, "Xmp.video.DefaultOn"),
                                              MatroskaTag(0x2, "Xmp.audio.DefaultOn"),
                                              MatroskaTag(0x11, "Xmp.video.SubTDefaultOn")};

const std::array<MatroskaTag, 3> trackForced = {MatroskaTag(0x1, "Xmp.video.TrackForced"),
                                                MatroskaTag(0x2, "Xmp.audio.TrackForced"),
                                                MatroskaTag(0x11, "Xmp.video.SubTTrackForced")};

const std::array<MatroskaTag, 3> trackLacing = {MatroskaTag(0x1, "Xmp.video.TrackLacing"),
                                                MatroskaTag(0x2, "Xmp.audio.TrackLacing"),
                                                MatroskaTag(0x11, "Xmp.video.SubTTrackLacing")};

const std::array<MatroskaTag, 3> codecDecodeAll = {MatroskaTag(0x1, "Xmp.video.CodecDecodeAll"),
                                                   MatroskaTag(0x2, "Xmp.audio.CodecDecodeAll"),
                                                   MatroskaTag(0x11, "Xmp.video.SubTCodecDecodeAll")};

const std::array<MatroskaTag, 3> codecDownloadUrl = {MatroskaTag(0x1, "Xmp.video.CodecDownloadUrl"),
                                                     MatroskaTag(0x2, "Xmp.audio.CodecDownloadUrl"),
                                                     MatroskaTag(0x11, "Xmp.video.SubTCodecDownloadUrl")};

const std::array<MatroskaTag, 3> codecSettings = {MatroskaTag(0x1, "Xmp.video.CodecSettings"),
                                                  MatroskaTag(0x2, "Xmp.audio.CodecSettings"),
                                                  MatroskaTag(0x11, "Xmp.video.SubTCodecSettings")};

const std::array<MatroskaTag, 3> trackCodec = {MatroskaTag(0x1, "Xmp.video.Codec"),
                                               MatroskaTag(0x2, "Xmp.audio.Compressor"),
                                               MatroskaTag(0x11, "Xmp.video.SubTCodec")};

const std::array<MatroskaTag, 3> codecInfo = {MatroskaTag(0x1, "Xmp.video.CodecInfo"),
                                              MatroskaTag(0x2, "Xmp.audio.CodecInfo"),
                                              MatroskaTag(0x11, "Xmp.video.SubTCodecInfo")};

const std::array<MatroskaTag, 2> streamRate = {MatroskaTag(0x1, "Xmp.video.FrameRate"),
                                               MatroskaTag(0x2, "Xmp.audio.DefaultDuration")};

/*!
  @brief Function used to calculate Tags, Tags may comprise of more than
      one byte. The first byte calculates size of the Tag and the remaining
      bytes are used to calculate the rest of the Tag.
      Returns Tag Value.
 */
[[nodiscard]] size_t returnTagValue(const byte* buf, size_t size) {
  assert(size > 0 && size <= 8);

  size_t b0 = buf[0] & (0xff >> size);
  size_t tag = b0 << ((size - 1) * 8);
  for (size_t i = 1; i < size; ++i) {
    tag |= static_cast<size_t>(buf[i]) << ((size - i - 1) * 8);
  }

  return tag;
}

/*!
    @brief Function used to convert buffer data into Integeral information,
        information stored in BigEndian format
 */
[[nodiscard]] bool convertToUint64(const byte* buf, size_t size, uint64_t& value) {
  uint64_t ret = 0;
  for (size_t i = 0; i < size; ++i) {
    ret |= static_cast<uint64_t>(buf[i]) << ((size - i - 1) * 8);
  }

  if (ret < std::numeric_limits<uint64_t>::min())
    return false;
  if (ret > std::numeric_limits<uint64_t>::max())
    return false;

  value = ret;
  return true;
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

  xmpData_["Xmp.video.FileName"] = io_->path();
  xmpData_["Xmp.video.FileSize"] = io_->size() / bytesMB;
  xmpData_["Xmp.video.MimeType"] = mimeType();

  while (continueTraversing_)
    decodeBlock();

  aspectRatio();
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
  const MatroskaTag* tag = findTag(matroskaTags, tag_id);

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
      decodeInternalTags(tag, buf2.data(), size);
      break;
    case String:
    case Utf8:
      decodeStringTags(tag, buf2.data());
      break;
    case Integer:
    case UInteger:
      decodeIntegerTags(tag, buf2.data(), size);
      break;
    case Boolean:
      decodeBooleanTags(tag, buf2.data(), size);
      break;
    case Date:
      decodeDateTags(tag, buf2.data(), size);
      break;
    case Float:
      decodeFloatTags(tag, buf2.data(), size);
      break;
    case Binary:
      break;
    case Master:
      break;
    default:
      break;
  }
}  // MatroskaVideo::decodeBlock

void MatroskaVideo::decodeInternalTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  const MatroskaTag* internalMt = nullptr;
  uint64_t key = 0;
  if (!convertToUint64(buf, size, key))
    return;

  switch (tag->_id) {
    case Xmp_video_VideoScanTpye:
      internalMt = findTag(videoScanType, key);
      break;
    case Xmp_audio_ChannelType:
      internalMt = findTag(audioChannels, key);
      break;
    case Xmp_video_ContentCompressAlgo:
      internalMt = findTag(compressionAlgorithm, key);
      break;
    case Xmp_video_ContentEncryptAlgo:
      internalMt = findTag(encryptionAlgorithm, key);
      break;
    case Xmp_video_ContentSignAlgo_1:
    case Xmp_video_ContentSignAlgo_2:
      internalMt = findTag(contentSignatureAlgorithm, key);
      break;
    case Xmp_video_ContentSignHashAlgo_1:
    case Xmp_video_ContentSignHashAlgo_2:
      internalMt = findTag(contentSignatureHashAlgorithm, key);
      break;
    case Xmp_video_ContentEncodingType:
      internalMt = findTag(encodingType, key);
      break;
    case Xmp_video_DisplayUnit:
      internalMt = findTag(displayUnit, key);
      break;
    case Xmp_video_AspectRatioType:
      internalMt = findTag(aspectRatioType, key);
      break;
    case Xmp_video_PhysicalEquivalent:
      internalMt = findTag(chapterPhysicalEquivalent, key);
      break;
    case Xmp_video_TranslateCodec:
      internalMt = findTag(chapterTranslateCodec, key);
      break;
    case Video_Audio_CodecID:
      internalMt = findTag(trackCodec, key);
      break;
    case Video_Audio_CodecName:
      internalMt = findTag(codecInfo, key);
      break;
    case CodecDownloadURL:
    case CodecInfoURL:
      internalMt = findTag(codecDownloadUrl, key);
      break;
    default:
      break;
  }
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

void MatroskaVideo::decodeIntegerTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  uint64_t value = 0;
  if (!convertToUint64(buf, size, value))
    return;

  if (tag->_id == Xmp_video_Width_1 || tag->_id == Xmp_video_Width_2)
    width_ = value;
  if (tag->_id == Xmp_video_Height_1 || tag->_id == Xmp_video_Height_2)
    height_ = value;
  xmpData_[tag->_label] = value;
}

void MatroskaVideo::decodeBooleanTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  std::string str("No");
  const MatroskaTag* internalMt = nullptr;
  uint64_t key = 0;
  if (!convertToUint64(buf, size, key))
    return;

  switch (tag->_id) {
    case TrackType:  // this tags is used internally only to deduce the type of track (video or audio)
      internalMt = findTag(matroskaTrackType, key);
      stream_ = internalMt->_id;
      internalMt = nullptr;
      break;
    case TrackUsed:
      internalMt = findTag(trackEnable, key);
      break;
    case TrackDefault:
      internalMt = findTag(defaultOn, key);
      break;
    case TrackForced:
      internalMt = findTag(trackForced, key);
      break;
    case TrackLacing:
      internalMt = findTag(trackLacing, key);
      break;
    case CodecDecodeAll:
      internalMt = findTag(codecDecodeAll, key);
      break;
    case CodecSettings:
      internalMt = findTag(codecSettings, key);
      break;
    case Xmp_video_TagDefault:
      internalMt = tag;
      break;
    default:
      break;
  }

  if (internalMt) {
    str = "Yes";
    xmpData_[internalMt->_label] = str;
  }
}

void MatroskaVideo::decodeDateTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  int64_t duration_in_ms = 0;
  uint64_t value = 0;
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

      if (!convertToUint64(buf, size, value))
        return;
      duration_in_ms = value / 1000000000;
      xmpData_[tag->_label] = duration_in_ms;
      break;

    case TimecodeScale:
      if (!convertToUint64(buf, size, value))
        return;
      time_code_scale_ = static_cast<double>(value) / static_cast<double>(1000000000);
      xmpData_[tag->_label] = time_code_scale_;
      break;
    default:
      break;
  }
}

void MatroskaVideo::decodeFloatTags(const MatroskaTag* tag, const byte* buf, size_t size) {
  xmpData_[tag->_label] = getFloat(buf, bigEndian);

  double frame_rate = 0;
  switch (tag->_id) {
    case Xmp_audio_SampleRate:
    case Xmp_audio_OutputSampleRate:
      xmpData_[tag->_label] = getFloat(buf, bigEndian);
      break;
    case VideoFrameRate_DefaultDuration:
    case Xmp_video_FrameRate: {
      uint64_t key = 0;
      if (!convertToUint64(buf, size, key))
        return;
      const MatroskaTag* internalMt = findTag(streamRate, key);
      if (internalMt) {
        switch (stream_) {
          case 1:  // video
            frame_rate = static_cast<double>(1000000000) / static_cast<double>(key);
            break;
          case 2:  // audio
            frame_rate = static_cast<double>(key / 1000);
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

void MatroskaVideo::aspectRatio() {
  double aspectRatio = static_cast<double>(width_) / static_cast<double>(height_);
  aspectRatio = floor(aspectRatio * 10) / 10;
  xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

  auto aR = static_cast<int>((aspectRatio * 10.0) + 0.1);

  switch (aR) {
    case 13:
      xmpData_["Xmp.video.AspectRatio"] = "4:3";
      break;
    case 17:
      xmpData_["Xmp.video.AspectRatio"] = "16:9";
      break;
    case 10:
      xmpData_["Xmp.video.AspectRatio"] = "1:1";
      break;
    case 16:
      xmpData_["Xmp.video.AspectRatio"] = "16:10";
      break;
    case 22:
      xmpData_["Xmp.video.AspectRatio"] = "2.21:1";
      break;
    case 23:
      xmpData_["Xmp.video.AspectRatio"] = "2.35:1";
      break;
    case 12:
      xmpData_["Xmp.video.AspectRatio"] = "5:4";
      break;
    default:
      xmpData_["Xmp.video.AspectRatio"] = aspectRatio;
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
    image.reset();
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