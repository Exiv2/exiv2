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

//#ifdef EXV_ENABLE_VIDEO
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
#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
namespace Internal {

//! List of composite tags. They are skipped and the child tags are read immediately
constexpr uint64_t compositeTagsList[] = {
    0x0000,    0x000e,    0x000f,    0x0020,    0x0026,    0x002e,    0x0036,    0x0037,   0x003b, 0x005b,    0x0060,
    0x0061,    0x0068,    0x05b9,    0x0dbb,    0x1034,    0x1035,    0x1854,    0x21a7,   0x2240, 0x23c0,    0x2624,
    0x27c8,    0x2911,    0x2924,    0x2944,    0x2d80,    0x3373,    0x35a1,    0x3e5b,   0x3e7b, 0x14d9b74, 0x254c367,
    0x549a966, 0x654ae6b, 0x8538067, 0x941a469, 0xa45dfa3, 0xb538667, 0xc53bb6b, 0xf43b675};

//! List of tags which are ignored, i.e., tag and value won't be read
constexpr uint64_t ignoredTagsList[] = {
    0x0021,  0x0023,   0x0033,   0x0071,   0x0077,  0x006c,   0x0067,  0x007b, 0x02f2, 0x02f3, 0x1031, 0x1032, 0x13ab,
    0x13ac,  0x15ee,   0x23a2,   0x23c6,   0x2e67,  0x33a4,   0x33c5,  0x3446, 0x2de7, 0x2df8, 0x26bf, 0x28ca, 0x3384,
    0x13b8,  0x037e,   0x0485,   0x18d7,   0x0005,  0x0009,   0x0011,  0x0012, 0x0016, 0x0017, 0x0018, 0x0022, 0x0024,
    0x0025,  0x0027,   0x002b,   0x002f,   0x003f,  0x004b,   0x004c,  0x004d, 0x004e, 0x004f, 0x006a, 0x006b, 0x006e,
    0x007a,  0x007d,   0x0255,   0x3eb5,   0x3ea5,  0x3d7b,   0x33c4,  0x2fab, 0x2ebc, 0x29fc, 0x29a5, 0x2955, 0x2933,
    0x135f,  0x2922,   0x26a5,   0x26fc,   0x2532,  0x23c9,   0x23c4,  0x23c5, 0x137f, 0x1378, 0x07e2, 0x07e3, 0x07e4,
    0x0675,  0x05bc,   0x05bd,   0x05db,   0x05dd,  0x0598,   0x050d,  0x0444, 0x037c,

    0x3314f, 0x43a770, 0x1eb923, 0x1cb923, 0xeb524, 0x1c83ab, 0x1e83bb};

/*!
  Tag Look-up list for Matroska Type Video Files
  The Tags have been categorized in 4 categories. Which are
  mentioned as a comment in front of them.
  s   --  Tag to be Skipped
  sd  --  Tag to be Skipped along with its data
  u   --  Tag used directly for storing metadata
  ui  --  Tag used only internally
 */
constexpr std::array<Exiv2::MatroskaTags, 198> matroskaTags = {
    Exiv2::MatroskaTags(0x0000, "ChapterDisplay"),                     // s
    Exiv2::MatroskaTags(0x0003, "TrackType"),                          // ui
    Exiv2::MatroskaTags(0x0005, "ChapterString"),                      // sd
    Exiv2::MatroskaTags(0x0006, "VideoCodecID/AudioCodecID/CodecID"),  // ui
    Exiv2::MatroskaTags(0x0008, "TrackDefault"),                       // ui
    Exiv2::MatroskaTags(0x0009, "ChapterTrackNumber"),                 // sd
    Exiv2::MatroskaTags(0x000e, "Slices"),                             // s
    Exiv2::MatroskaTags(0x000f, "ChapterTrack"),                       // s
    Exiv2::MatroskaTags(0x0011, "ChapterTimeStart"),                   // sd
    Exiv2::MatroskaTags(0x0012, "ChapterTimeEnd"),                     // sd
    Exiv2::MatroskaTags(0x0016, "CueRefTime"),                         // sd
    Exiv2::MatroskaTags(0x0017, "CueRefCluster"),                      // sd
    Exiv2::MatroskaTags(0x0018, "ChapterFlagHidden"),                  // sd
    Exiv2::MatroskaTags(0x001a, "Xmp.video.VideoScanTpye"),            // u
    Exiv2::MatroskaTags(0x001b, "BlockDuration"),                      // s
    Exiv2::MatroskaTags(0x001c, "TrackLacing"),                        // ui
    Exiv2::MatroskaTags(0x001f, "Xmp.audio.ChannelType"),              // u
    Exiv2::MatroskaTags(0x0020, "BlockGroup"),                         // s
    Exiv2::MatroskaTags(0x0021, "Block"),                              // sd
    Exiv2::MatroskaTags(0x0022, "BlockVirtual"),                       // sd
    Exiv2::MatroskaTags(0x0023, "SimpleBlock"),                        // sd
    Exiv2::MatroskaTags(0x0024, "CodecState"),                         // sd
    Exiv2::MatroskaTags(0x0025, "BlockAdditional"),                    // sd
    Exiv2::MatroskaTags(0x0026, "BlockMore"),                          // s
    Exiv2::MatroskaTags(0x0027, "Position"),                           // sd
    Exiv2::MatroskaTags(0x002a, "CodecDecodeAll"),                     // ui
    Exiv2::MatroskaTags(0x002b, "PrevSize"),                           // sd
    Exiv2::MatroskaTags(0x002e, "TrackEntry"),                         // s
    Exiv2::MatroskaTags(0x002f, "EncryptedBlock"),                     // sd
    Exiv2::MatroskaTags(0x0030, "Xmp.video.Width"),                    // u
    Exiv2::MatroskaTags(0x0033, "CueTime"),                            // sd
    Exiv2::MatroskaTags(0x0035, "Xmp.audio.SampleRate"),               // u
    Exiv2::MatroskaTags(0x0036, "ChapterAtom"),                        // s
    Exiv2::MatroskaTags(0x0037, "CueTrackPositions"),                  // s
    Exiv2::MatroskaTags(0x0039, "TrackUsed"),                          // ui
    Exiv2::MatroskaTags(0x003a, "Xmp.video.Height"),                   // u
    Exiv2::MatroskaTags(0x003b, "CuePoint"),                           // s
    Exiv2::MatroskaTags(0x003f, "CRC-32"),                             // sd
    Exiv2::MatroskaTags(0x004b, "BlockAdditionalID"),                  // sd
    Exiv2::MatroskaTags(0x004c, "LaceNumber"),                         // sd
    Exiv2::MatroskaTags(0x004d, "FrameNumber"),                        // sd
    Exiv2::MatroskaTags(0x004e, "Delay"),                              // sd
    Exiv2::MatroskaTags(0x004f, "ClusterDuration"),                    // sd
    Exiv2::MatroskaTags(0x0057, "TrackNumber"),                        // ui
    Exiv2::MatroskaTags(0x005b, "CueReference"),                       // s
    Exiv2::MatroskaTags(0x0060, "Video"),                              // s
    Exiv2::MatroskaTags(0x0061, "Audio"),                              // s
    Exiv2::MatroskaTags(0x0067, "Timecode"),                           // sd
    Exiv2::MatroskaTags(0x0068, "TimeSlice"),                          // s
    Exiv2::MatroskaTags(0x006a, "CueCodecState"),                      // sd
    Exiv2::MatroskaTags(0x006b, "CueRefCodecState"),                   // sd
    Exiv2::MatroskaTags(0x006c, "Void"),                               // sd
    Exiv2::MatroskaTags(0x006e, "BlockAddID"),                         // sd
    Exiv2::MatroskaTags(0x0071, "CueClusterPosition"),                 // sd
    Exiv2::MatroskaTags(0x0077, "CueTrack"),                           // sd
    Exiv2::MatroskaTags(0x007a, "ReferencePriority"),                  // sd
    Exiv2::MatroskaTags(0x007b, "ReferenceBlock"),                     // sd
    Exiv2::MatroskaTags(0x007d, "ReferenceVirtual"),                   // sd
    Exiv2::MatroskaTags(0x0254, "Xmp.video.ContentCompressAlgo"),      // u
    Exiv2::MatroskaTags(0x0255, "ContentCompressionSettings"),         // sd
    Exiv2::MatroskaTags(0x0282, "Xmp.video.DocType"),                  // u
    Exiv2::MatroskaTags(0x0285, "Xmp.video.DocTypeReadVersion"),       // u
    Exiv2::MatroskaTags(0x0286, "Xmp.video.EBMLVersion"),              // u
    Exiv2::MatroskaTags(0x0287, "Xmp.video.DocTypeVersion"),           // u
    Exiv2::MatroskaTags(0x02f2, "EBMLMaxIDLength"),                    // sd
    Exiv2::MatroskaTags(0x02f3, "EBMLMaxSizeLength"),                  // sd
    Exiv2::MatroskaTags(0x02f7, "Xmp.video.EBMLReadVersion"),          // u
    Exiv2::MatroskaTags(0x037c, "ChapterLanguage"),                    // sd
    Exiv2::MatroskaTags(0x037e, "ChapterCountry"),                     // sd
    Exiv2::MatroskaTags(0x0444, "SegmentFamily"),                      // sd
    Exiv2::MatroskaTags(
        0x0461,
        "Xmp.video.DateUTC"),  // Date Time Original - measured in seconds relatively to Jan 01, 2001, 0:00:00 GMT+0h
    Exiv2::MatroskaTags(0x047a, "Xmp.video.TagLanguage"),                     // u
    Exiv2::MatroskaTags(0x0484, "Xmp.video.TagDefault"),                      // u
    Exiv2::MatroskaTags(0x0485, "TagBinary"),                                 // sd
    Exiv2::MatroskaTags(0x0487, "Xmp.video.TagString"),                       // u
    Exiv2::MatroskaTags(0x0489, "Xmp.video.Duration"),                        // u
    Exiv2::MatroskaTags(0x050d, "ChapterProcessPrivate"),                     // sd
    Exiv2::MatroskaTags(0x0598, "ChapterFlagEnabled"),                        // sd
    Exiv2::MatroskaTags(0x05a3, "Xmp.video.TagName"),                         // u
    Exiv2::MatroskaTags(0x05b9, "EditionEntry"),                              // s
    Exiv2::MatroskaTags(0x05bc, "EditionUID"),                                // sd
    Exiv2::MatroskaTags(0x05bd, "EditionFlagHidden"),                         // sd
    Exiv2::MatroskaTags(0x05db, "EditionFlagDefault"),                        // sd
    Exiv2::MatroskaTags(0x05dd, "EditionFlagOrdered"),                        // sd
    Exiv2::MatroskaTags(0x065c, "Xmp.video.AttachFileData"),                  // u
    Exiv2::MatroskaTags(0x0660, "Xmp.video.AttachFileMIME"),                  // u
    Exiv2::MatroskaTags(0x066e, "Xmp.video.AttachFileName"),                  // u
    Exiv2::MatroskaTags(0x0675, "AttachedFileReferral"),                      // sd
    Exiv2::MatroskaTags(0x067e, "Xmp.video.AttachFileDesc"),                  // u
    Exiv2::MatroskaTags(0x06ae, "Xmp.video.AttachFileUID"),                   // u
    Exiv2::MatroskaTags(0x07e1, "Xmp.video.ContentEncryptAlgo"),              // u
    Exiv2::MatroskaTags(0x07e2, "ContentEncryptionKeyID"),                    // sd
    Exiv2::MatroskaTags(0x07e3, "ContentSignature"),                          // sd
    Exiv2::MatroskaTags(0x07e4, "ContentSignatureKeyID"),                     // sd
    Exiv2::MatroskaTags(0x07e5, "Xmp.video.ContentSignAlgo"),                 // u
    Exiv2::MatroskaTags(0x07e6, "Xmp.video.ContentSignHashAlgo"),             // u
    Exiv2::MatroskaTags(0x0d80, "Xmp.video.MuxingApp"),                       // u
    Exiv2::MatroskaTags(0x0dbb, "Seek"),                                      // s
    Exiv2::MatroskaTags(0x1031, "ContentEncodingOrder"),                      // sd
    Exiv2::MatroskaTags(0x1032, "ContentEncodingScope"),                      // sd
    Exiv2::MatroskaTags(0x1033, "Xmp.video.ContentEncodingType"),             // u
    Exiv2::MatroskaTags(0x1034, "ContentCompression"),                        // s
    Exiv2::MatroskaTags(0x1035, "ContentEncryption"),                         // s
    Exiv2::MatroskaTags(0x135f, "CueRefNumber"),                              // sd
    Exiv2::MatroskaTags(0x136e, "Xmp.video.TrackName"),                       // u
    Exiv2::MatroskaTags(0x1378, "CueBlockNumber"),                            // sd
    Exiv2::MatroskaTags(0x137f, "TrackOffset"),                               // sd
    Exiv2::MatroskaTags(0x13ab, "SeekID"),                                    // sd
    Exiv2::MatroskaTags(0x13ac, "SeekPosition"),                              // sd
    Exiv2::MatroskaTags(0x13b8, "Stereo3DMode"),                              // sd
    Exiv2::MatroskaTags(0x14aa, "Xmp.video.CropBottom"),                      // ui
    Exiv2::MatroskaTags(0x14b0, "Xmp.video.Width"),                           // u
    Exiv2::MatroskaTags(0x14b2, "Xmp.video.DisplayUnit"),                     // u
    Exiv2::MatroskaTags(0x14b3, "Xmp.video.AspectRatioType"),                 // u
    Exiv2::MatroskaTags(0x14ba, "Xmp.video.Height"),                          // u
    Exiv2::MatroskaTags(0x14bb, "Xmp.video.CropTop"),                         // ui
    Exiv2::MatroskaTags(0x14cc, "Xmp.video.CropLeft"),                        // ui
    Exiv2::MatroskaTags(0x14dd, "Xmp.video.CropRight"),                       // ui
    Exiv2::MatroskaTags(0x15aa, "TrackForced"),                               // ui
    Exiv2::MatroskaTags(0x15ee, "MaxBlockAdditionID"),                        // sd
    Exiv2::MatroskaTags(0x1741, "Xmp.video.WritingApp"),                      // u
    Exiv2::MatroskaTags(0x1854, "SilentTracks"),                              // s
    Exiv2::MatroskaTags(0x18d7, "SilentTrackNumber"),                         // sd
    Exiv2::MatroskaTags(0x21a7, "AttachedFile"),                              // s
    Exiv2::MatroskaTags(0x2240, "ContentEncoding"),                           // s
    Exiv2::MatroskaTags(0x2264, "Xmp.audio.BitsPerSample"),                   // u
    Exiv2::MatroskaTags(0x23a2, "CodecPrivate"),                              // sd
    Exiv2::MatroskaTags(0x23c0, "Targets"),                                   // s
    Exiv2::MatroskaTags(0x23c3, "Xmp.video.PhysicalEquivalent"),              // u
    Exiv2::MatroskaTags(0x23c4, "TagChapterUID"),                             // sd
    Exiv2::MatroskaTags(0x23c5, "TagTrackUID"),                               // sd
    Exiv2::MatroskaTags(0x23c6, "TagAttachmentUID"),                          // sd
    Exiv2::MatroskaTags(0x23c9, "TagEditionUID"),                             // sd
    Exiv2::MatroskaTags(0x23ca, "Xmp.video.TargetType"),                      // u
    Exiv2::MatroskaTags(0x2532, "SignedElement"),                             // sd
    Exiv2::MatroskaTags(0x2624, "TrackTranslate"),                            // s
    Exiv2::MatroskaTags(0x26a5, "TrackTranslateTrackID"),                     // sd
    Exiv2::MatroskaTags(0x26bf, "TrackTranslateCodec"),                       // sd
    Exiv2::MatroskaTags(0x26fc, "TrackTranslateEditionUID"),                  // sd
    Exiv2::MatroskaTags(0x27c8, "SimpleTag"),                                 // s
    Exiv2::MatroskaTags(0x28ca, "TargetTypeValue"),                           // sd
    Exiv2::MatroskaTags(0x2911, "ChapterProcessCommand"),                     // s
    Exiv2::MatroskaTags(0x2922, "ChapterProcessTime"),                        // sd
    Exiv2::MatroskaTags(0x2924, "ChapterTranslate"),                          // s
    Exiv2::MatroskaTags(0x2933, "ChapterProcessData"),                        // sd
    Exiv2::MatroskaTags(0x2944, "ChapterProcess"),                            // s
    Exiv2::MatroskaTags(0x2955, "ChapterProcessCodecID"),                     // sd
    Exiv2::MatroskaTags(0x29a5, "ChapterTranslateID"),                        // sd
    Exiv2::MatroskaTags(0x29bf, "Xmp.video.TranslateCodec"),                  // u
    Exiv2::MatroskaTags(0x29fc, "ChapterTranslateEditionUID"),                // sd
    Exiv2::MatroskaTags(0x2d80, "ContentEncodings"),                          // s
    Exiv2::MatroskaTags(0x2de7, "MinCache"),                                  // sd
    Exiv2::MatroskaTags(0x2df8, "MaxCache"),                                  // sd
    Exiv2::MatroskaTags(0x2e67, "ChapterSegmentUID"),                         // sd
    Exiv2::MatroskaTags(0x2ebc, "ChapterSegmentEditionUID"),                  // sd
    Exiv2::MatroskaTags(0x2fab, "TrackOverlay"),                              // sd
    Exiv2::MatroskaTags(0x3373, "Tag"),                                       // s
    Exiv2::MatroskaTags(0x3384, "SegmentFileName"),                           // sd
    Exiv2::MatroskaTags(0x33a4, "SegmentUID"),                                // sd
    Exiv2::MatroskaTags(0x33c4, "ChapterUID"),                                // sd
    Exiv2::MatroskaTags(0x33c5, "TrackUID"),                                  // sd
    Exiv2::MatroskaTags(0x3446, "TrackAttachmentUID"),                        // sd
    Exiv2::MatroskaTags(0x35a1, "BlockAdditions"),                            // s
    Exiv2::MatroskaTags(0x38b5, "Xmp.audio.OutputSampleRate"),                // u
    Exiv2::MatroskaTags(0x3ba9, "Xmp.video.Title"),                           // u
    Exiv2::MatroskaTags(0x3d7b, "ChannelPositions"),                          // sd
    Exiv2::MatroskaTags(0x3e5b, "SignatureElements"),                         // s
    Exiv2::MatroskaTags(0x3e7b, "SignatureElementList"),                      // s
    Exiv2::MatroskaTags(0x3e8a, "Xmp.video.ContentSignAlgo"),                 // u
    Exiv2::MatroskaTags(0x3e9a, "Xmp.video.ContentSignHashAlgo"),             // u
    Exiv2::MatroskaTags(0x3ea5, "SignaturePublicKey"),                        // sd
    Exiv2::MatroskaTags(0x3eb5, "Signature"),                                 // sd
    Exiv2::MatroskaTags(0x2b59c, "TrackLanguage"),                            // ui
    Exiv2::MatroskaTags(0x3314f, "TrackTimecodeScale"),                       // sd
    Exiv2::MatroskaTags(0x383e3, "Xmp.video.FrameRate"),                      // u
    Exiv2::MatroskaTags(0x3e383, "VideoFrameRate/DefaultDuration"),           // ui
    Exiv2::MatroskaTags(0x58688, "VideoCodecName/AudioCodecName/CodecName"),  // ui
    Exiv2::MatroskaTags(0x6b240, "CodecDownloadURL"),                         // ui
    Exiv2::MatroskaTags(0xad7b1, "TimecodeScale"),                            // ui
    Exiv2::MatroskaTags(0xeb524, "ColorSpace"),                               // sd
    Exiv2::MatroskaTags(0xfb523, "Xmp.video.OpColor"),                        // u
    Exiv2::MatroskaTags(0x1a9697, "CodecSettings"),                           // ui
    Exiv2::MatroskaTags(0x1b4040, "CodecInfoURL"),                            // ui
    Exiv2::MatroskaTags(0x1c83ab, "PrevFileName"),                            // sd
    Exiv2::MatroskaTags(0x1cb923, "PrevUID"),                                 // sd
    Exiv2::MatroskaTags(0x1e83bb, "NextFileName"),                            // sd
    Exiv2::MatroskaTags(0x1eb923, "NextUID"),                                 // sd
    Exiv2::MatroskaTags(0x43a770, "Chapters"),                                // sd
    Exiv2::MatroskaTags(0x14d9b74, "SeekHead"),                               // s
    Exiv2::MatroskaTags(0x254c367, "Tags"),                                   // s
    Exiv2::MatroskaTags(0x549a966, "Info"),                                   // s
    Exiv2::MatroskaTags(0x654ae6b, "Tracks"),                                 // s
    Exiv2::MatroskaTags(0x8538067, "SegmentHeader"),                          // s
    Exiv2::MatroskaTags(0x941a469, "Attachments"),                            // s
    Exiv2::MatroskaTags(0xa45dfa3, "EBMLHeader"),                             // s
    Exiv2::MatroskaTags(0xb538667, "SignatureSlot"),                          // s
    Exiv2::MatroskaTags(0xc53bb6b, "Cues"),                                   // s
    Exiv2::MatroskaTags(0xf43b675, "Cluster"),                                // s
};

constexpr std::array<MatroskaTags, 7> matroskaTrackType = {
    Exiv2::MatroskaTags(0x1, "Video"),   Exiv2::MatroskaTags(0x2, "Audio"),     Exiv2::MatroskaTags(0x3, "Complex"),
    Exiv2::MatroskaTags(0x10, "Logo"),   Exiv2::MatroskaTags(0x11, "Subtitle"), Exiv2::MatroskaTags(0x12, "Buttons"),
    Exiv2::MatroskaTags(0x20, "Control")};

constexpr std::array<Exiv2::MatroskaTags, 4> compressionAlgorithm = {
    Exiv2::MatroskaTags(0, "zlib "), Exiv2::MatroskaTags(1, "bzlib"), Exiv2::MatroskaTags(2, "lzo1x"),
    Exiv2::MatroskaTags(3, "Header Stripping")};

constexpr std::array<Exiv2::MatroskaTags, 4> audioChannels = {
    Exiv2::MatroskaTags(1, "Mono"), Exiv2::MatroskaTags(2, "Stereo"), Exiv2::MatroskaTags(5, "5.1 Surround Sound"),
    Exiv2::MatroskaTags(7, "7.1 Surround Sound")};

constexpr std::array<Exiv2::MatroskaTags, 3> displayUnit = {
    Exiv2::MatroskaTags(0x0, "Pixels"), Exiv2::MatroskaTags(0x1, "cm"), Exiv2::MatroskaTags(0x2, "inches")};

constexpr std::array<Exiv2::MatroskaTags, 6> encryptionAlgorithm = {
    Exiv2::MatroskaTags(0, "Not Encrypted"), Exiv2::MatroskaTags(1, "DES"),      Exiv2::MatroskaTags(2, "3DES"),
    Exiv2::MatroskaTags(3, "Twofish"),       Exiv2::MatroskaTags(4, "Blowfish"), Exiv2::MatroskaTags(5, "AES")};

constexpr std::array<Exiv2::MatroskaTags, 7> chapterPhysicalEquivalent = {
    Exiv2::MatroskaTags(10, "Index"),         Exiv2::MatroskaTags(20, "Track"), Exiv2::MatroskaTags(30, "Session"),
    Exiv2::MatroskaTags(40, "Layer"),         Exiv2::MatroskaTags(50, "Side"),  Exiv2::MatroskaTags(60, "CD / DVD"),
    Exiv2::MatroskaTags(70, "Set / Package"),
};

constexpr std::array<Exiv2::MatroskaTags, 2> encodingType = {Exiv2::MatroskaTags(0, "Compression"),
                                                             Exiv2::MatroskaTags(1, "Encryption")};

constexpr std::array<Exiv2::MatroskaTags, 2> videoScanType = {Exiv2::MatroskaTags(0, "Progressive"),
                                                              Exiv2::MatroskaTags(1, "Interlaced")};

constexpr std::array<Exiv2::MatroskaTags, 2> chapterTranslateCodec = {Exiv2::MatroskaTags(0, "Matroska Script"),
                                                                      Exiv2::MatroskaTags(1, "DVD Menu")};

constexpr std::array<Exiv2::MatroskaTags, 3> aspectRatioType = {Exiv2::MatroskaTags(0, "Free Resizing"),
                                                                Exiv2::MatroskaTags(1, "Keep Aspect Ratio"),
                                                                Exiv2::MatroskaTags(2, "Fixed")};

constexpr std::array<Exiv2::MatroskaTags, 2> contentSignatureAlgorithm = {Exiv2::MatroskaTags(0, "Not Signed"),
                                                                          Exiv2::MatroskaTags(1, "RSA")};

constexpr std::array<Exiv2::MatroskaTags, 3> contentSignatureHashAlgorithm = {
    Exiv2::MatroskaTags(0, "Not Signed"), Exiv2::MatroskaTags(1, "SHA1-160"), Exiv2::MatroskaTags(2, "MD5")};

constexpr std::array<Exiv2::MatroskaTags, 3> trackEnable = {Exiv2::MatroskaTags(0x1, "Xmp.video.Enabled"),
                                                            Exiv2::MatroskaTags(0x2, "Xmp.audio.Enabled"),
                                                            Exiv2::MatroskaTags(0x11, "Xmp.video.SubTEnabled")};

constexpr std::array<Exiv2::MatroskaTags, 3> defaultOn = {Exiv2::MatroskaTags(0x1, "Xmp.video.DefaultOn"),
                                                          Exiv2::MatroskaTags(0x2, "Xmp.audio.DefaultOn"),
                                                          Exiv2::MatroskaTags(0x11, "Xmp.video.SubTDefaultOn")};

constexpr std::array<Exiv2::MatroskaTags, 3> trackForced = {Exiv2::MatroskaTags(0x1, "Xmp.video.TrackForced"),
                                                            Exiv2::MatroskaTags(0x2, "Xmp.audio.TrackForced"),
                                                            Exiv2::MatroskaTags(0x11, "Xmp.video.SubTTrackForced")};

constexpr std::array<Exiv2::MatroskaTags, 3> trackLacing = {Exiv2::MatroskaTags(0x1, "Xmp.video.TrackLacing"),
                                                            Exiv2::MatroskaTags(0x2, "Xmp.audio.TrackLacing"),
                                                            Exiv2::MatroskaTags(0x11, "Xmp.video.SubTTrackLacing")};

constexpr std::array<Exiv2::MatroskaTags, 3> codecDecodeAll = {
    Exiv2::MatroskaTags(0x1, "Xmp.video.CodecDecodeAll"), Exiv2::MatroskaTags(0x2, "Xmp.audio.CodecDecodeAll"),
    Exiv2::MatroskaTags(0x11, "Xmp.video.SubTCodecDecodeAll")};

constexpr std::array<Exiv2::MatroskaTags, 3> codecDownloadUrl = {
    Exiv2::MatroskaTags(0x1, "Xmp.video.CodecDownloadUrl"), Exiv2::MatroskaTags(0x2, "Xmp.audio.CodecDownloadUrl"),
    Exiv2::MatroskaTags(0x11, "Xmp.video.SubTCodecDownloadUrl")};

constexpr std::array<Exiv2::MatroskaTags, 3> codecSettings = {Exiv2::MatroskaTags(0x1, "Xmp.video.CodecSettings"),
                                                              Exiv2::MatroskaTags(0x2, "Xmp.audio.CodecSettings"),
                                                              Exiv2::MatroskaTags(0x11, "Xmp.video.SubTCodecSettings")};

constexpr std::array<Exiv2::MatroskaTags, 3> trackCodec = {Exiv2::MatroskaTags(0x1, "Xmp.video.Codec"),
                                                           Exiv2::MatroskaTags(0x2, "Xmp.audio.Compressor"),
                                                           Exiv2::MatroskaTags(0x11, "Xmp.video.SubTCodec")};

constexpr std::array<Exiv2::MatroskaTags, 3> trackLanguage = {Exiv2::MatroskaTags(0x1, "Xmp.video.TrackLang"),
                                                              Exiv2::MatroskaTags(0x2, "Xmp.audio.TrackLang"),
                                                              Exiv2::MatroskaTags(0x11, "Xmp.video.SubTLang")};

constexpr std::array<Exiv2::MatroskaTags, 3> codecInfo = {Exiv2::MatroskaTags(0x1, "Xmp.video.CodecInfo"),
                                                          Exiv2::MatroskaTags(0x2, "Xmp.audio.CodecInfo"),
                                                          Exiv2::MatroskaTags(0x11, "Xmp.video.SubTCodecInfo")};

constexpr std::array<Exiv2::MatroskaTags, 2> streamRate = {Exiv2::MatroskaTags(0x1, "Xmp.video.FrameRate"),
                                                           Exiv2::MatroskaTags(0x2, "Xmp.audio.DefaultDuration")};

/*!
  @brief Function used to calculate Tags, Tags may comprise of more than
      one byte. The first byte calculates size of the Tag and the remaining
      bytes are used to calculate the rest of the Tag.
      Returns Tag Value.
 */
[[nodiscard]] uint64_t returnTagValue(const byte* buf, size_t size) {
  assert(size > 0 && size <= 8);

  uint64_t b0 = buf[0] & (0xff >> size);
  uint64_t tag = b0 << ((size - 1) * 8);
  for (size_t i = 1; i < size; ++i) {
    tag |= static_cast<uint64_t>(buf[i]) << ((size - i - 1) * 8);
  }

  return tag;
}

/*!
    @brief Function used to convert buffer data into numerical information,
        information stored in BigEndian format
 */
[[nodiscard]] uint64_t returnValue(const byte* buf, size_t size) {
  uint64_t ret = 0;
  for (size_t i = 0; i < size; ++i) {
    ret |= static_cast<uint64_t>(buf[i]) << ((size - i - 1) * 8);
  }

  return ret;
}

}  // namespace Internal
}  // namespace Exiv2

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
  xmpData_["Xmp.video.FileSize"] = (double)io_->size() / (double)1048576;
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

  uint32_t sz = findBlockSize(buf[0]);  // 0-8
  if (sz > 0)
    io_->read(buf + 1, sz - 1);

  const MatroskaTags* mt = findTag(matroskaTags, returnTagValue(buf, sz));

  if (!mt) {
    continueTraversing_ = false;
    return;
  }

  if (mt->first == 0xc53bb6b || mt->first == 0xf43b675) {
    continueTraversing_ = false;
    return;
  }

  bool skip = find(compositeTagsList, mt->first) != 0;
  bool ignore = find(ignoredTagsList, mt->first) != 0;

  io_->read(buf, 1);
  sz = findBlockSize(buf[0]);  // 0-8

  if (sz > 0)
    io_->read(buf + 1, sz - 1);
  uint64_t size = returnTagValue(buf, sz);

  if (skip && !ignore)
    return;

  const uint64_t bufMinSize = 200;

#ifndef SUPPRESS_WARNINGS
  if (!ignore && size > bufMinSize) {
    EXV_WARNING << "Size " << size << " of Matroska tag 0x" << std::hex << mt->first << std::dec << " is greater than "
                << bufMinSize << ": ignoring it.\n";
  }
#endif
  if (ignore || size > bufMinSize) {
    io_->seek(size, BasicIo::cur);
    return;
  }

  DataBuf buf2(bufMinSize + 1);
  std::fill(buf2.begin(), buf2.end(), 0x0);
  size_t s = static_cast<size_t>(size);
  io_->read(buf2.data(), s);
  contentManagement(mt, buf2.data(), s);
}  // MatroskaVideo::decodeBlock

void MatroskaVideo::contentManagement(const MatroskaTags* mt, const byte* buf, size_t size) {
  int64_t duration_in_ms = 0;
  static double time_code_scale = 1.0, temp = 0;
  static uint64_t stream = 0, track_count = 0;
  char str[4] = "No";
  const MatroskaTags* internalMt = 0;

  switch (mt->first) {
    case 0x0282:
    case 0x0d80:
    case 0x1741:
    case 0x3ba9:
    case 0x066e:
    case 0x0660:
    case 0x065c:
    case 0x067e:
    case 0x047a:
    case 0x0487:
    case 0x05a3:
    case 0x136e:
    case 0x23ca:
    case 0xeb524:
      xmpData_[mt->second] = buf;
      break;

    case 0x0030:
    case 0x003a:
    case 0x0287:
    case 0x14b0:
    case 0x14ba:
    case 0x285:
    case 0x06ae:
    case 0x0286:
    case 0x02f7:
    case 0x2264:
    case 0x14aa:
    case 0x14bb:
    case 0x14cc:
    case 0x14dd:
      xmpData_[mt->second] = returnValue(buf, size);

      if (mt->first == 0x0030 || mt->first == 0x14b0) {
        width_ = returnValue(buf, size);
      } else if (mt->first == 0x003a || mt->first == 0x14ba) {
        height_ = returnValue(buf, size);
      }
      break;

    case 0x001a:
    case 0x001f:
    case 0x0254:
    case 0x07e1:
    case 0x07e5:
    case 0x07e6:
    case 0x1033:
    case 0x14b2:
    case 0x14b3:
    case 0x23c3:
    case 0x29bf:
    case 0x3e8a:
    case 0x3e9a:
      switch (mt->first) {
        case 0x001a:
          internalMt = findTag(videoScanType, returnValue(buf, size));
          break;
        case 0x001f:
          internalMt = findTag(audioChannels, returnValue(buf, size));
          break;
        case 0x0254:
          internalMt = findTag(compressionAlgorithm, returnValue(buf, size));
          break;
        case 0x07e1:
          internalMt = findTag(encryptionAlgorithm, returnValue(buf, size));
          break;
        case 0x1033:
          internalMt = findTag(encodingType, returnValue(buf, size));
          break;
        case 0x3e8a:
        case 0x07e5:
          internalMt = findTag(contentSignatureAlgorithm, returnValue(buf, size));
          break;
        case 0x3e9a:
        case 0x07e6:
          internalMt = findTag(contentSignatureHashAlgorithm, returnValue(buf, size));
          break;
        case 0x14b2:
          internalMt = findTag(displayUnit, returnValue(buf, size));
          break;
        case 0x14b3:
          internalMt = findTag(aspectRatioType, returnValue(buf, size));
          break;
        case 0x23c3:
          internalMt = findTag(chapterPhysicalEquivalent, returnValue(buf, size));
          break;
        case 0x29bf:
          internalMt = findTag(chapterTranslateCodec, returnValue(buf, size));
          break;
      }
      if (internalMt)
        xmpData_[mt->second] = internalMt->second;
      break;

    case 0x0035:
    case 0x38b5:
      xmpData_[mt->second] = getFloat(buf, bigEndian);
      break;

    case 0x0039:
    case 0x0008:
    case 0x15aa:
    case 0x001c:
    case 0x002a:
    case 0x1a9697:
    case 0x0484:
      if (returnValue(buf, size))
        strcpy(str, "Yes");
      switch (mt->first) {
        case 0x0039:
          internalMt = findTag(trackEnable, stream);
          break;
        case 0x0008:
          internalMt = findTag(defaultOn, stream);
          break;
        case 0x15aa:
          internalMt = findTag(trackForced, stream);
          break;
        case 0x001c:
          internalMt = findTag(trackLacing, stream);
          break;
        case 0x002a:
          internalMt = findTag(codecDecodeAll, stream);
          break;
        case 0x1a9697:
          internalMt = findTag(codecSettings, stream);
          break;
        case 0x0484:
          internalMt = mt;
          break;
      }
      if (internalMt)
        xmpData_[internalMt->second] = str;

      break;

    case 0x0006:
    case 0x2b59c:
    case 0x58688:
    case 0x6b240:
    case 0x1b4040:
      switch (mt->first) {
        case 0x0006:
          internalMt = findTag(trackCodec, stream);
          break;
        case 0x2b59c:
          internalMt = findTag(trackLanguage, stream);
          break;
        case 0x58688:
          internalMt = findTag(codecInfo, stream);
          break;
        case 0x6b240:
        case 0x1b4040:
          internalMt = findTag(codecDownloadUrl, stream);
          break;
      }
      if (internalMt)
        xmpData_[internalMt->second] = buf;
      break;

    case 0x0489:
    case 0x0461:
      switch (mt->first) {
        case 0x0489:
          if (size <= 4) {
            duration_in_ms =
                static_cast<int64_t>(getFloat(buf, bigEndian) * static_cast<float>(time_code_scale) * 1000.0f);
          } else {
            duration_in_ms = static_cast<int64_t>(getDouble(buf, bigEndian) * time_code_scale * 1000);
          }
          break;
        case 0x0461: {
          duration_in_ms = returnValue(buf, size) / 1000000000;
          break;
        }
      }
      xmpData_[mt->second] = duration_in_ms;
      break;

    case 0x0057:
      track_count++;
      xmpData_["Xmp.video.TotalStream"] = track_count;
      break;

    case 0xad7b1:
      time_code_scale = (double)returnValue(buf, size) / (double)1000000000;
      xmpData_["Xmp.video.TimecodeScale"] = time_code_scale;
      break;

    case 0x0003:
      internalMt = findTag(matroskaTrackType, returnValue(buf, size));
      stream = internalMt->first;
      break;

    case 0x3e383:
    case 0x383e3:
      internalMt = findTag(streamRate, stream);
      if (returnValue(buf, size)) {
        switch (stream) {
          case 1:
            temp = (double)1000000000 / (double)returnValue(buf, size);
            break;
          case 2:
            temp = static_cast<double>(returnValue(buf, size) / 1000);
            break;
        }
        if (internalMt)
          xmpData_[internalMt->second] = temp;
      } else if (internalMt)
        xmpData_[internalMt->second] = "Variable Bit Rate";
      break;

    default:
      break;
  }
}  // MatroskaVideo::contentManagement

void MatroskaVideo::aspectRatio() {
  // TODO - Make a better unified method to handle all cases of Aspect Ratio

  double aspectRatio = (double)width_ / (double)height_;
  aspectRatio = floor(aspectRatio * 10) / 10;
  xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

  int aR = (int)((aspectRatio * 10.0) + 0.1);

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
  else if (b & 64)
    return 2;
  else if (b & 32)
    return 3;
  else if (b & 16)
    return 4;
  else if (b & 8)
    return 5;
  else if (b & 4)
    return 6;
  else if (b & 2)
    return 7;
  else if (b & 1)
    return 8;
  else
    return 0;
}

Image::UniquePtr newMkvInstance(BasicIo::UniquePtr io, bool /*create*/) {
  Image::UniquePtr image(new MatroskaVideo(std::move(io)));
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