// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2012 Andreas Huggel <ahuggel@gmx.net>
 *
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
/*
  File:      riffvideo.cpp
  Version:   $Rev$
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
             Mahesh Hegde for GSoC 2013 mailto:maheshmhegade at gmail dot com <maheshmhegade@gmail.com>
  History:   18-Jun-12, AB: created
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "riffvideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "tiffimage_int.hpp"
#include "string.h"
#include "utilsvideo.hpp"

// + standard includes
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2{
    namespace Internal{

    /*!
      @brief Dummy TIFF header structure.
     */
    class DummyTiffHeader : public TiffHeaderBase{
    public:
        //! @name Creators
        //@{
        //! Default constructor
        DummyTiffHeader(ByteOrder byteOrder);
        //! Destructor
        ~DummyTiffHeader();
        //@}

        //! @name Manipulators
        //@{
        //! Dummy read function. Does nothing and returns true.
        bool read(const byte* pData, uint64_t size);
        //@}

    }; // class TiffHeader

    DummyTiffHeader::DummyTiffHeader(ByteOrder byteOrder)
        : TiffHeaderBase(42, 0, byteOrder, 0){
    }

    DummyTiffHeader::~DummyTiffHeader(){
    }

    bool DummyTiffHeader::read(const byte* /*pData*/, uint64_t /*size*/){
        return true;
    }

    //! RIFF INFO tags supported Under Exiv2.
    extern const TagVocabulary infoTags[] ={
        {   "AGES", "Xmp.video.Rated" },
        {   "CMNT", "Xmp.video.Comment" },
        {   "CODE", "Xmp.video.EncodedBy" },
        {   "COMM", "Xmp.video.Comment" },
        {   "DIRC", "Xmp.video.Director" },
        {   "DISP", "Xmp.audio.SchemeTitle" },
        {   "DTIM", "Xmp.video.DateTimeOriginal" },
        {   "GENR", "Xmp.video.Genre" },
        {   "IARL", "Xmp.video.ArchivalLocation" },
        {   "IART", "Xmp.video.Artist" },
        {   "IAS1", "Xmp.video.Edit1" },
        {   "IAS2", "Xmp.video.Edit2" },
        {   "IAS3", "Xmp.video.Edit3" },
        {   "IAS4", "Xmp.video.Edit4" },
        {   "IAS5", "Xmp.video.Edit5" },
        {   "IAS6", "Xmp.video.Edit6" },
        {   "IAS7", "Xmp.video.Edit7" },
        {   "IAS8", "Xmp.video.Edit8" },
        {   "IAS9", "Xmp.video.Edit9" },
        {   "IBSU", "Xmp.video.BaseURL" },
        {   "ICAS", "Xmp.audio.DefaultStream" },
        {   "ICDS", "Xmp.video.CostumeDesigner" },
        {   "ICMS", "Xmp.video.Commissioned" },
        {   "ICMT", "Xmp.video.Comment" },
        {   "ICNM", "Xmp.video.Cinematographer" },
        {   "ICNT", "Xmp.video.Country" },
        {   "ICOP", "Xmp.video.Copyright" },
        {   "ICRD", "Xmp.video.DateTimeDigitized" },
        {   "ICRP", "Xmp.video.Cropped" },
        {   "IDIM", "Xmp.video.Dimensions	" },
        {   "IDPI", "Xmp.video.DotsPerInch" },
        {   "IDST", "Xmp.video.DistributedBy" },
        {   "IEDT", "Xmp.video.EditedBy" },
        {   "IENC", "Xmp.video.EncodedBy" },
        {   "IENG", "Xmp.video.Engineer" },
        {   "IGNR", "Xmp.video.Genre" },
        {   "IKEY", "Xmp.video.PerformerKeywords" },
        {   "ILGT", "Xmp.video.Lightness" },
        {   "ILGU", "Xmp.video.LogoURL" },
        {   "ILIU", "Xmp.video.LogoIconURL" },
        {   "ILNG", "Xmp.video.Language" },
        {   "IMBI", "Xmp.video.InfoBannerImage" },
        {   "IMBU", "Xmp.video.InfoBannerURL" },
        {   "IMED", "Xmp.video.Medium" },
        {   "IMIT", "Xmp.video.InfoText" },
        {   "IMIU", "Xmp.video.InfoURL" },
        {   "IMUS", "Xmp.video.MusicBy" },
        {   "INAM", "Xmp.video.Title" },
        {   "IPDS", "Xmp.video.ProductionDesigner" },
        {   "IPLT", "Xmp.video.NumOfColors" },
        {   "IPRD", "Xmp.video.Product" },
        {   "IPRO", "Xmp.video.ProducedBy" },
        {   "IRIP", "Xmp.video.RippedBy" },
        {   "IRTD", "Xmp.video.Rating" },
        {   "ISBJ", "Xmp.video.Subject" },
        {   "ISFT", "Xmp.video.Software" },
        {   "ISGN", "Xmp.video.SecondaryGenre" },
        {   "ISHP", "Xmp.video.Sharpness" },
        {   "ISRC", "Xmp.video.Source" },
        {   "ISRF", "Xmp.video.SourceForm" },
        {   "ISTD", "Xmp.video.ProductionStudio" },
        {   "ISTR", "Xmp.video.Starring" },
        {   "ITCH", "Xmp.video.Technician" },
        {   "IWMU", "Xmp.video.WatermarkURL" },
        {   "IWRI", "Xmp.video.WrittenBy" },
        {   "LANG", "Xmp.video.Language" },
        {   "LOCA", "Xmp.video.LocationInfo" },
        {   "PRT1", "Xmp.video.Part" },
        {   "PRT2", "Xmp.video.NumOfParts" },
        {   "RATE", "Xmp.video.Rate" },
        {   "STAR", "Xmp.video.Starring" },
        {   "STAT", "Xmp.video.Statistics" },
        {   "TAPE", "Xmp.video.TapeName" },
        {   "TCDO", "Xmp.video.EndTimecode" },
        {   "TCOD", "Xmp.video.StartTimecode" },
        {   "TITL", "Xmp.video.Title" },
        {   "TLEN", "Xmp.video.Length" },
        {   "TORG", "Xmp.video.Organization" },
        {   "TRCK", "Xmp.video.TrackNumber" },
        {   "TURL", "Xmp.video.URL" },
        {   "TVER", "Xmp.video.SoftwareVersion" },
        {   "VMAJ", "Xmp.video.VegasVersionMajor" },
        {   "VMIN", "Xmp.video.VegasVersionMinor" },
        {   "YEAR", "Xmp.video.Year" }
    };

    //! Audio Encoding Information.
    extern const TagDetails audioEncodingValues[] ={
        {   0x1, "Microsoft PCM" },
        {   0x2, "Microsoft ADPCM" },
        {   0x3, "Microsoft IEEE float" },
        {   0x4, "Compaq VSELP" },
        {   0x5, "IBM CVSD" },
        {   0x6, "Microsoft a-Law" },
        {   0x7, "Microsoft u-Law" },
        {   0x8, "Microsoft DTS" },
        {   0x9, "DRM" },
        {   0xa, "WMA 9 Speech" },
        {   0xb, "Microsoft Windows Media RT Voice" },
        {   0x10, "OKI-ADPCM" },
        {   0x11, "Intel IMA/DVI-ADPCM" },
        {   0x12, "Videologic Mediaspace ADPCM" },
        {   0x13, "Sierra ADPCM" },
        {   0x14, "Antex G.723 ADPCM" },
        {   0x15, "DSP Solutions DIGISTD" },
        {   0x16, "DSP Solutions DIGIFIX" },
        {   0x17, "Dialoic OKI ADPCM" },
        {   0x18, "Media Vision ADPCM" },
        {   0x19, "HP CU" },
        {   0x1a, "HP Dynamic Voice" },
        {   0x20, "Yamaha ADPCM" },
        {   0x21, "SONARC Speech Compression" },
        {   0x22, "DSP Group True Speech" },
        {   0x23, "Echo Speech Corp." },
        {   0x24, "Virtual Music Audiofile AF36" },
        {   0x25, "Audio Processing Tech." },
        {   0x26, "Virtual Music Audiofile AF10" },
        {   0x27, "Aculab Prosody 1612" },
        {   0x28, "Merging Tech. LRC" },
        {   0x30, "Dolby AC2" },
        {   0x31, "Microsoft GSM610" },
        {   0x32, "MSN Audio" },
        {   0x33, "Antex ADPCME" },
        {   0x34, "Control Resources VQLPC" },
        {   0x35, "DSP Solutions DIGIREAL" },
        {   0x36, "DSP Solutions DIGIADPCM" },
        {   0x37, "Control Resources CR10" },
        {   0x38, "Natural MicroSystems VBX ADPCM" },
        {   0x39, "Crystal Semiconductor IMA ADPCM" },
        {   0x3a, "Echo Speech ECHOSC3" },
        {   0x3b, "Rockwell ADPCM" },
        {   0x3c, "Rockwell DIGITALK" },
        {   0x3d, "Xebec Multimedia" },
        {   0x40, "Antex G.721 ADPCM" },
        {   0x41, "Antex G.728 CELP" },
        {   0x42, "Microsoft MSG723" },
        {   0x43, "IBM AVC ADPCM" },
        {   0x45, "ITU-T G.726" },
        {   0x50, "Microsoft MPEG" },
        {   0x51, "RT23 or PAC" },
        {   0x52, "InSoft RT24" },
        {   0x53, "InSoft PAC" },
        {   0x55, "MP3" },
        {   0x59, "Cirrus" },
        {   0x60, "Cirrus Logic" },
        {   0x61, "ESS Tech. PCM" },
        {   0x62, "Voxware Inc." },
        {   0x63, "Canopus ATRAC" },
        {   0x64, "APICOM G.726 ADPCM" },
        {   0x65, "APICOM G.722 ADPCM" },
        {   0x66, "Microsoft DSAT" },
        {   0x67, "Micorsoft DSAT DISPLAY" },
        {   0x69, "Voxware Byte Aligned" },
        {   0x70, "Voxware AC8" },
        {   0x71, "Voxware AC10" },
        {   0x72, "Voxware AC16" },
        {   0x73, "Voxware AC20" },
        {   0x74, "Voxware MetaVoice" },
        {   0x75, "Voxware MetaSound" },
        {   0x76, "Voxware RT29HW" },
        {   0x77, "Voxware VR12" },
        {   0x78, "Voxware VR18" },
        {   0x79, "Voxware TQ40" },
        {   0x7a, "Voxware SC3" },
        {   0x7b, "Voxware SC3" },
        {   0x80, "Soundsoft" },
        {   0x81, "Voxware TQ60" },
        {   0x82, "Microsoft MSRT24" },
        {   0x83, "AT&T G.729A" },
        {   0x84, "Motion Pixels MVI MV12" },
        {   0x85, "DataFusion G.726" },
        {   0x86, "DataFusion GSM610" },
        {   0x88, "Iterated Systems Audio" },
        {   0x89, "Onlive" },
        {   0x8a, "Multitude, Inc. FT SX20" },
        {   0x8b, "Infocom ITS A/S G.721 ADPCM" },
        {   0x8c, "Convedia G729" },
        {   0x8d, "Not specified congruency, Inc." },
        {   0x91, "Siemens SBC24" },
        {   0x92, "Sonic Foundry Dolby AC3 APDIF" },
        {   0x93, "MediaSonic G.723" },
        {   0x94, "Aculab Prosody 8kbps" },
        {   0x97, "ZyXEL ADPCM" },
        {   0x98, "Philips LPCBB" },
        {   0x99, "Studer Professional Audio Packed" },
        {   0xa0, "Malden PhonyTalk" },
        {   0xa1, "Racal Recorder GSM" },
        {   0xa2, "Racal Recorder G720.a" },
        {   0xa3, "Racal G723.1" },
        {   0xa4, "Racal Tetra ACELP" },
        {   0xb0, "NEC AAC NEC Corporation" },
        {   0xff, "AAC" },
        {   0x100, "Rhetorex ADPCM" },
        {   0x101, "IBM u-Law" },
        {   0x102, "IBM a-Law" },
        {   0x103, "IBM ADPCM" },
        {   0x111, "Vivo G.723" },
        {   0x112, "Vivo Siren" },
        {   0x120, "Philips Speech Processing CELP" },
        {   0x121, "Philips Speech Processing GRUNDIG" },
        {   0x123, "Digital G.723" },
        {   0x125, "Sanyo LD ADPCM" },
        {   0x130, "Sipro Lab ACEPLNET" },
        {   0x131, "Sipro Lab ACELP4800" },
        {   0x132, "Sipro Lab ACELP8V3" },
        {   0x133, "Sipro Lab G.729" },
        {   0x134, "Sipro Lab G.729A" },
        {   0x135, "Sipro Lab Kelvin" },
        {   0x136, "VoiceAge AMR" },
        {   0x140, "Dictaphone G.726 ADPCM" },
        {   0x150, "Qualcomm PureVoice" },
        {   0x151, "Qualcomm HalfRate" },
        {   0x155, "Ring Zero Systems TUBGSM" },
        {   0x160, "Microsoft Audio1" },
        {   0x161, "Windows Media Audio V2 V7 V8 V9 / DivX audio (WMA) / Alex AC3 Audio" },
        {   0x162, "Windows Media Audio Professional V9" },
        {   0x163, "Windows Media Audio Lossless V9" },
        {   0x164, "WMA Pro over S/PDIF" },
        {   0x170, "UNISYS NAP ADPCM" },
        {   0x171, "UNISYS NAP ULAW" },
        {   0x172, "UNISYS NAP ALAW" },
        {   0x173, "UNISYS NAP 16K" },
        {   0x174, "MM SYCOM ACM SYC008 SyCom Technologies" },
        {   0x175, "MM SYCOM ACM SYC701 G726L SyCom Technologies" },
        {   0x176, "MM SYCOM ACM SYC701 CELP54 SyCom Technologies" },
        {   0x177, "MM SYCOM ACM SYC701 CELP68 SyCom Technologies" },
        {   0x178, "Knowledge Adventure ADPCM" },
        {   0x180, "Fraunhofer IIS MPEG2AAC" },
        {   0x190, "Digital Theater Systems DTS DS" },
        {   0x200, "Creative Labs ADPCM" },
        {   0x202, "Creative Labs FASTSPEECH8" },
        {   0x203, "Creative Labs FASTSPEECH10" },
        {   0x210, "UHER ADPCM" },
        {   0x215, "Ulead DV ACM" },
        {   0x216, "Ulead DV ACM" },
        {   0x220, "Quarterdeck Corp." },
        {   0x230, "I-Link VC" },
        {   0x240, "Aureal Semiconductor Raw Sport" },
        {   0x241, "ESST AC3" },
        {   0x250, "Interactive Products HSX" },
        {   0x251, "Interactive Products RPELP" },
        {   0x260, "Consistent CS2" },
        {   0x270, "Sony SCX" },
        {   0x271, "Sony SCY" },
        {   0x272, "Sony ATRAC3" },
        {   0x273, "Sony SPC" },
        {   0x280, "TELUM Telum Inc." },
        {   0x281, "TELUMIA Telum Inc." },
        {   0x285, "Norcom Voice Systems ADPCM" },
        {   0x300, "Fujitsu FM TOWNS SND" },
        {   0x301, "Fujitsu (not specified)" },
        {   0x302, "Fujitsu (not specified)" },
        {   0x303, "Fujitsu (not specified)" },
        {   0x304, "Fujitsu (not specified)" },
        {   0x305, "Fujitsu (not specified)" },
        {   0x306, "Fujitsu (not specified)" },
        {   0x307, "Fujitsu (not specified)" },
        {   0x308, "Fujitsu (not specified)" },
        {   0x350, "Micronas Semiconductors, Inc. Development" },
        {   0x351, "Micronas Semiconductors, Inc. CELP833" },
        {   0x400, "Brooktree Digital" },
        {   0x401, "Intel Music Coder (IMC)" },
        {   0x402, "Ligos Indeo Audio" },
        {   0x450, "QDesign Music" },
        {   0x500, "On2 VP7 On2 Technologies" },
        {   0x501, "On2 VP6 On2 Technologies" },
        {   0x680, "AT&T VME VMPCM" },
        {   0x681, "AT&T TCP" },
        {   0x700, "YMPEG Alpha (dummy for MPEG-2 compressor)" },
        {   0x8ae, "ClearJump LiteWave (lossless)" },
        {   0x1000, "Olivetti GSM" },
        {   0x1001, "Olivetti ADPCM" },
        {   0x1002, "Olivetti CELP" },
        {   0x1003, "Olivetti SBC" },
        {   0x1004, "Olivetti OPR" },
        {   0x1100, "Lernout & Hauspie" },
        {   0x1101, "Lernout & Hauspie CELP codec" },
        {   0x1102, "Lernout & Hauspie SBC codec" },
        {   0x1103, "Lernout & Hauspie SBC codec" },
        {   0x1104, "Lernout & Hauspie SBC codec" },
        {   0x1400, "Norris Comm. Inc." },
        {   0x1401, "ISIAudio" },
        {   0x1500, "AT&T Soundspace Music Compression" },
        {   0x181c, "VoxWare RT24 speech codec" },
        {   0x181e, "Lucent elemedia AX24000P Music codec" },
        {   0x1971, "Sonic Foundry LOSSLESS" },
        {   0x1979, "Innings Telecom Inc. ADPCM" },
        {   0x1c07, "Lucent SX8300P speech codec" },
        {   0x1c0c, "Lucent SX5363S G.723 compliant codec" },
        {   0x1f03, "CUseeMe DigiTalk (ex-Rocwell)" },
        {   0x1fc4, "NCT Soft ALF2CD ACM" },
        {   0x2000, "FAST Multimedia DVM" },
        {   0x2001, "Dolby DTS (Digital Theater System)" },
        {   0x2002, "RealAudio 1 / 2 14.4" },
        {   0x2003, "RealAudio 1 / 2 28.8" },
        {   0x2004, "RealAudio G2 / 8 Cook (low bitrate)" },
        {   0x2005, "RealAudio 3 / 4 / 5 Music (DNET)" },
        {   0x2006, "RealAudio 10 AAC (RAAC)" },
        {   0x2007, "RealAudio 10 AAC+ (RACP)" },
        {   0x2500, "Reserved range to 0x2600 Microsoft" },
        {   0x3313, "makeAVIS (ffvfw fake AVI sound from AviSynth scripts)" },
        {   0x4143, "Divio MPEG-4 AAC audio" },
        {   0x4201, "Nokia adaptive multirate" },
        {   0x4243, "Divio G726 Divio, Inc." },
        {   0x434c, "LEAD Speech" },
        {   0x564c, "LEAD Vorbis" },
        {   0x5756, "WavPack Audio" },
        {   0x674f, "Ogg Vorbis (mode 1)" },
        {   0x6750, "Ogg Vorbis (mode 2)" },
        {   0x6751, "Ogg Vorbis (mode 3)" },
        {   0x676f, "Ogg Vorbis (mode 1+)" },
        {   0x6770, "Ogg Vorbis (mode 2+)" },
        {   0x6771, "Ogg Vorbis (mode 3+)" },
        {   0x7000, "3COM NBX 3Com Corporation" },
        {   0x706d, "FAAD AAC" },
        {   0x7a21, "GSM-AMR (CBR, no SID)" },
        {   0x7a22, "GSM-AMR (VBR, including SID)" },
        {   0xa100, "Comverse Infosys Ltd. G723 1" },
        {   0xa101, "Comverse Infosys Ltd. AVQSBC" },
        {   0xa102, "Comverse Infosys Ltd. OLDSBC" },
        {   0xa103, "Symbol Technologies G729A" },
        {   0xa104, "VoiceAge AMR WB VoiceAge Corporation" },
        {   0xa105, "Ingenient Technologies Inc. G726" },
        {   0xa106, "ISO/MPEG-4 advanced audio Coding" },
        {   0xa107, "Encore Software Ltd G726" },
        {   0xa109, "Speex ACM Codec xiph.org" },
        {   0xdfac, "DebugMode SonicFoundry Vegas FrameServer ACM Codec" },
        {   0xe708, "Unknown -" },
        {   0xf1ac, "Free Lossless Audio Codec FLAC" },
        {   0xfffe, "Extensible" },
        {   0xffff, "Development" }
    };

    //! Nikon tags.
    extern const TagDetails nikonAVITags[] ={
        {   0x0003, "Xmp.video.Make" },
        {   0x0004, "Xmp.video.Model" },
        {   0x0005, "Xmp.video.Software" },
        {   0x0006, "Xmp.video.Equipment" },
        {   0x0007, "Xmp.video.Orientation" },
        {   0x0008, "Xmp.video.ExposureTime" },
        {   0x0009, "Xmp.video.FNumber" },
        {   0x000a, "Xmp.video.ExposureCompensation" },
        {   0x000b, "Xmp.video.MaxApertureValue" },
        {   0x000c, "Xmp.video.MeteringMode" },
        {   0x000f, "Xmp.video.FocalLength" },
        {   0x0010, "Xmp.video.XResolution" },
        {   0x0011, "Xmp.video.YResolution" },
        {   0x0012, "Xmp.video.ResolutionUnit" },
        {   0x0013, "Xmp.video.DateTimeOriginal" },
        {   0x0014, "Xmp.video.DateTimeDigitized" },
        {   0x0016, "Xmp.video.duration" },
        {   0x0018, "Xmp.video.FocusMode" },
        {   0x001b, "Xmp.video.DigitalZoomRatio" },
        {   0x001d, "Xmp.video.ColorMode" },
        {   0x001e, "Xmp.video.Sharpness" },
        {   0x001f, "Xmp.video.WhiteBalance" },
        {   0x0020, "Xmp.video.ColorNoiseReduction" }
    };

    /*
    extern const TagDetails orientation[] =  {
        {   1, "Horizontal (normal)" },
        {   2, "Mirror horizontal" },
        {   3, "Rotate 180" },
        {   4, "Mirror vertical" },
        {   5, "Mirror horizontal and rotate 270 CW" },
        {   6, "Rotate 90 CW" },
        {   7, "Mirror horizontal and rotate 90 CW" },
        {   8, "Rotate 270 CW" }
    };
     */
    //! Metering tags.
    extern const TagDetails meteringMode[] ={
        {   0, "Unknown" },
        {   1, "Average" },
        {   2, "Center-weighted average" },
        {   3, "Spot" },
        {   4, "Multi-spot" },
        {   5, "Multi-segment" },
        {   6, "Partial" },
        { 255, "Other" }
    };

    //!Resolution tag.
    extern const TagDetails resolutionUnit[] ={
        {   1, "None" },
        {   2, "inches" },
        {   3, "cm" }
    };

    int64_t i;     //!< temporary variable.
    int64_t j;     //!< temporary variable.

    //! To select specific bytes with in a chunk.
    enum streamTypeInfo{
        Audio = 1, MIDI, Text, Video
    };

    //! To select specific bytes with in a chunk.
    enum streamHeaderTags{
        codec = 1, sampleRate = 5, sampleCount = 8, quality = 10, sampleSize
    };

    //! To select specific bytes with in a chunk.
    enum bmptags{
        imageWidth, imageHeight, planes, bitDepth, compression, imageLength, pixelsPerMeterX, pixelsPerMeterY, numColors, numImportantColors
    };

    //! To select specific bytes with in a chunk.
    enum audioFormatTags{
        encoding, numberOfChannels, audioSampleRate, avgBytesPerSec = 4, bitsPerSample = 7
    };

    //! To select specific bytes with in a chunk.
    enum aviHeaderTags{
        frameRate, maxDataRate, frameCount = 4,  initialFrames ,streamCount, suggestedBufferSize ,imageWidth_h, imageHeight_h
    };
    }
}                                      // namespace Internal, Exiv2

namespace Exiv2{
using namespace Exiv2::Internal;

class RiffVideo::Private{
public:
    Private(){
        streamType_ = 0;
        m_decodeMetaData = true;
    }
    //! Variable which stores current stream being processsed.
    int streamType_;
    //! Variable to decide whether to decode the metadata or just reveal file structure (Lists inside RIFF)
    bool m_decodeMetaData;
    //!variable to decide whether write metadata is possible or not (by default false)
    bool m_modifyMetadata;
    //!abstract metadata holding variable (its data about data about data!)
    RiffMetaSkeleton m_riffFileSkeleton;
};

RiffVideo::RiffVideo(BasicIo::AutoPtr io)
    : Image(ImageType::riff, mdNone, io), d(new Private){
    d->m_modifyMetadata = false;
    d->m_decodeMetaData = true;
} // RiffVideo::RiffVideo

RiffVideo::~RiffVideo(){
    delete d;
}

std::string RiffVideo::mimeType() const{
    return "video/riff";
}

/*!
 * \brief RiffVideo::findChunkPositions:
 *                   Return a list of chunk positions for a given chunk Id
 * \param chunkId
 * \return:          if No chunk of chunkId exists,then size of returned
 *                   vector will be zero
 */
std::vector<int32_t> RiffVideo::findChunkPositions(const char* chunkId){
    DataBuf chkId((uint64_t)5);
    chkId.pData_[4] = '\0';
    uint64_t i;

    std::vector<int32_t> positions;
    for ( i=0; i<d->m_riffFileSkeleton.m_primitiveChunks.size(); i++){
        io_->seek(d->m_riffFileSkeleton.m_primitiveChunks[i]->m_chunkLocation,BasicIo::beg);
        io_->read(chkId.pData_,4);
        if(UtilsVideo::compareTagValue(chkId,chunkId)) positions.push_back((int32_t)io_->tell());
    }
    return positions;
}

//Same as above but returns only header locations
std::vector<int32_t> RiffVideo::findHeaderPositions(const char* headerId){
    DataBuf hdrId((uint64_t)5);
    hdrId.pData_[4] = '\0';
    uint64_t i;

    std::vector<int32_t> positions;
    for (i=0;i<d->m_riffFileSkeleton.m_headerChunks.size();i++){
        io_->seek((d->m_riffFileSkeleton.m_headerChunks[i]->m_headerLocation + 4),BasicIo::beg);
        io_->read(hdrId.pData_,4);
        if(UtilsVideo::compareTagValue(hdrId,headerId)) positions.push_back((int32_t)io_->tell() - 8);
    }
    return positions;
}

void RiffVideo::writeMetadata(){
    if (io_->open() != 0) throw Error(9, io_->path(), strError());
    IoCloser closer(*io_);
    doWriteMetadata();
    io_->close();
    return;
} // RiffVideo::writeMetadata

/*!
 * \brief RiffVideo::doWriteMetadata:
 *                   Search for all the supported ChunkIds in Exiv2 and
 *                   write the modified data to the file.
 */
void RiffVideo::doWriteMetadata(){
    //pre-write checks to make sure that we are writing to provided Riff file only
    if (!io_->isopen()) throw Error(20);
    if (!isRiffType(*io_, false)){
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    uint64_t bufMinSize = 4;

    //some initial variable declarations and initialization before traversing through chunks
    DataBuf chkId(bufMinSize+1);
    DataBuf chkSize(bufMinSize+1);

    chkId.pData_[4] = '\0';
    chkSize.pData_[4] = '\0';

    //skip "RIFF" "chkSize" "AVI"
    io_->read(chkId.pData_, bufMinSize);
    io_->read(chkSize.pData_, bufMinSize);
    io_->read(chkId.pData_, bufMinSize);

    d->m_decodeMetaData = false;
    d->m_modifyMetadata = true;

    //write metadata in order

    //find and move to avih position
    int32_t dummyLong = 0;
    uint64_t i,j,maxCount;

    std::vector<int32_t> strhChunkPositions = findChunkPositions("STRH");

    std::vector<std::string> primitiveFlags;
    //Add new tags to the beginning of the container prmitiveFlags and handle conditions to the end of case statements.
    primitiveFlags.push_back("STRN");
    primitiveFlags.push_back("FMT ");
    primitiveFlags.push_back("STRF");
    primitiveFlags.push_back("STRH");
    primitiveFlags.push_back("AVIH");

    maxCount = (uint64_t) primitiveFlags.size();
    for (j=0; j<maxCount; j++){
        std::vector<int32_t> primitivePositions = findChunkPositions(primitiveFlags.back().c_str());
        primitiveFlags.pop_back();
        for (i=0; i<(uint64_t)primitivePositions.size(); i++){
            io_->seek(primitivePositions[i]+4,BasicIo::beg);
            switch (j){
            case 0: aviHeaderTagsHandler(dummyLong); break;
            case 1: setStreamType();
                streamHandler(dummyLong);
                break;
            case 2: io_->seek(strhChunkPositions[i]+4,BasicIo::beg);
                setStreamType();
                io_->seek(primitivePositions[i]+4,BasicIo::beg);
                streamFormatHandler(dummyLong);
                break;
            case 3: d->streamType_ = Audio;
                streamHandler(dummyLong);
                break;
            case 4: dateTimeOriginal(dummyLong); break;
            default:break;
            }
        }
        primitivePositions.clear();
    }

    std::vector<std::string> headerFlags;
    //Add new tags to the beginning of the container headerFlags and handle conditions to the end of case statements.
    headerFlags.push_back("INFO");
    headerFlags.push_back("NCDT");
    headerFlags.push_back("ODML");
    headerFlags.push_back("IDIT");

    maxCount = (uint64_t) headerFlags.size();
    for (j=0; j<maxCount; j++){
        std::vector<int32_t> headerPositions = findHeaderPositions(headerFlags.back().c_str());
        headerFlags.pop_back();

        for (i=0; i<(uint64_t)headerPositions.size(); i++){
            io_->seek(headerPositions[i],BasicIo::beg);
            switch (j){
            case 0: io_->seek(headerPositions[i]+8,BasicIo::beg);
                dateTimeOriginal(dummyLong);
                break;
            case 1: odmlTagsHandler(); break;
            case 2: nikonTagsHandler(); break;
            case 3: infoTagsHandler(); break;
                //TODO :implement write functionality for header chunks and exif and iptc metadata
            default: break;
            }
        }
        headerPositions.clear();
    }
    return;
}// RiffVideo::doWriteMetadata

void RiffVideo::readMetadata(){
    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isRiffType(*io_, false)){
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "RIFF");
    }

    IoCloser closer(*io_);
    clearMetadata();

    xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.FileName"] = io_->path();
    xmpData_["Xmp.video.MimeType"] = mimeType();

    DataBuf chkId((const int32_t)5);
    DataBuf chkSize((const int32_t)5);
    DataBuf chkHeader((const int32_t)5);

    chkId.pData_[4] = '\0';
    chkHeader.pData_[4] = '\0';

    io_->read(chkId.pData_, 4);
    xmpData_["Xmp.video.Container"] = chkId.pData_;

    //skip file size
    io_->read(chkSize.pData_, 4);
    io_->read(chkHeader.pData_, 4);
    xmpData_["Xmp.video.FileType"] = chkHeader.pData_;

    d->m_decodeMetaData = true;
    decodeBlock();
    return;
} // RiffVideo::readMetadata

void RiffVideo::decodeBlock(){
    DataBuf chkId((const int32_t)5);
    chkId.pData_[4] = '\0' ;

    io_->read(chkId.pData_, 4);

    if(io_->eof()) return;
    else if(UtilsVideo::compareTagValue(chkId, "LIST") || UtilsVideo::compareTagValue(chkId, "JUNK")){
        io_->seek(-4,BasicIo::cur);
        tagDecoder();
    }
    return;
} // RiffVideo::decodeBlock

void RiffVideo::tagDecoder(){
    DataBuf chkMainId((const int32_t)5);
    chkMainId.pData_[4] = '\0';
    io_->read(chkMainId.pData_, 4);

    if(io_->eof()) return;

    if (UtilsVideo::compareTagValue(chkMainId, "LIST")){
        DataBuf listSize((const int32_t)5);
        DataBuf chkHeader((const int32_t)5);

        listSize.pData_[4] = '\0';
        chkHeader.pData_[4] = '\0';

        io_->read(listSize.pData_, 4);
        uint64_t listsize = Exiv2::getULong(listSize.pData_, littleEndian);
        io_->read(chkHeader.pData_, 4);

        HeaderChunk* tmpHeaderChunk = new HeaderChunk();
        memcpy((Exiv2::byte* )tmpHeaderChunk->m_headerId,(const Exiv2::byte*)chkHeader.pData_,5);

        int32_t listend = io_->tell() + listsize - 4 ;

        IoPosition position = PremitiveChunk;
        while( (position == PremitiveChunk) && (io_->tell() < listend) ){
            DataBuf chkId((const int32_t)5);
            DataBuf chkSize((const int32_t)5);
            chkId.pData_[4] = '\0';
            chkSize.pData_[4] = '\0';

            io_->read(chkId.pData_, 4);
            io_->read(chkSize.pData_, 4);

            uint64_t size = Exiv2::getULong(chkSize.pData_, littleEndian);

            const char allPrimitiveFlags[][5]={"JUNK","AVIH","FMT ",
                                               "STRH","STRF","STRN","STRD"};
            const char allHeaderFlags[][5]={"NCDT","ODML","MOVI","INFO"};

            if(UtilsVideo::compareTagValue(chkId,allPrimitiveFlags,(int32_t)(sizeof(allPrimitiveFlags)/5))){
                PrimitiveChunk* tmpPremitiveChunk = new PrimitiveChunk();
                memcpy((Exiv2::byte* )tmpPremitiveChunk->m_chunkId,(const Exiv2::byte*)chkId.pData_,5);
                tmpPremitiveChunk->m_chunkLocation = io_->tell() - 8;
                tmpPremitiveChunk->m_chunkSize = size;
                d->m_riffFileSkeleton.m_primitiveChunks.push_back(tmpPremitiveChunk);
            }

            if(UtilsVideo::compareTagValue(chkHeader,allHeaderFlags,(int32_t)(sizeof(allHeaderFlags)/5))){
                tmpHeaderChunk->m_headerLocation = io_->tell() - 16;
                tmpHeaderChunk->m_headerSize = listsize ;
                d->m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
                position = RiffVideo::TraversingChunk;
            }
            //to handle AVI file formats
            if(!d->m_decodeMetaData && UtilsVideo::compareTagValue(chkId,allPrimitiveFlags,(int32_t)(sizeof(allPrimitiveFlags)/5))) io_->seek(size,BasicIo::cur);

            else if(!d->m_decodeMetaData && UtilsVideo::compareTagValue(chkHeader,allHeaderFlags,(int)(sizeof(allHeaderFlags)/5))) io_->seek((listsize - 12),BasicIo::cur);

            else if(UtilsVideo::compareTagValue(chkId, "JUNK")) junkHandler(size);
            //primitive chunks note:compare chkId
            else if(UtilsVideo::compareTagValue(chkId, "AVIH")) aviHeaderTagsHandler(size);
            else if(UtilsVideo::compareTagValue(chkId, "STRH")){
                //since strh and strf exist sequential stream type can be set once
                setStreamType();
                streamHandler(size);
            }
            else if(UtilsVideo::compareTagValue(chkId,"STRF") || UtilsVideo::compareTagValue(chkId, "FMT ")){
                if(UtilsVideo::compareTagValue(chkId,"FMT ")) d->streamType_ = Audio;
                streamFormatHandler(size);
            }
            else if(UtilsVideo::compareTagValue(chkId, "STRN")) dateTimeOriginal(size, 1);
            else if(UtilsVideo::compareTagValue(chkId, "STRD")) streamDataTagHandler(size);
            //add more else if to support more primitive chunks below if any
            //TODO:add indx support

            //recursive call to decode LIST by breaking out of the loop
            else if(UtilsVideo::compareTagValue(chkId, "LIST")){
                position = RiffVideo::TraversingChunk;
                io_->seek(-8,BasicIo::cur);
            }

            //header chunks note:compare chkHeader
            else if(UtilsVideo::compareTagValue(chkHeader, "INFO")){
                io_->seek(-16,BasicIo::cur);
                infoTagsHandler();
            }
            else if(UtilsVideo::compareTagValue(chkHeader, "NCDT")){
                io_->seek(-16,BasicIo::cur);
                nikonTagsHandler();
            }
            else if(UtilsVideo::compareTagValue(chkHeader, "ODML")){
                io_->seek(-16,BasicIo::cur);
                odmlTagsHandler();
            }
            //only way to exit from readMetadata()
            else if(UtilsVideo::compareTagValue(chkHeader, "MOVI") || UtilsVideo::compareTagValue(chkMainId, "DATA")) io_->seek(listsize-12,BasicIo::cur);
            //Add read functionality for custom chunk headers or user created tags here with more (else if) before else.

            //skip block with unknown chunk ID (primitive chunk)
            else if(io_->tell() < listend){
                position = RiffVideo::TraversingChunk;
                io_->seek((listend -  (io_->tell()+4)) ,BasicIo::cur);
            }

            //skip chunk with unknown headerId
            else io_->seek(io_->tell()+size, BasicIo::cur);

            if(UtilsVideo::compareTagValue(chkId,allPrimitiveFlags,(int32_t)(sizeof(allPrimitiveFlags)/5)) && (size % 2 != 0)) io_->seek(1,BasicIo::cur);

            if(UtilsVideo::compareTagValue(chkId,allHeaderFlags,(int32_t)(sizeof(allHeaderFlags)/5)) && (listsize % 2 != 0)) io_->seek(1,BasicIo::cur);
        }
    }

    //AVIX can have JUNK chunk directly inside RIFF chunk
    else if(UtilsVideo::compareTagValue(chkMainId, "JUNK")){
        DataBuf junkSize((const int32_t)5);
        junkSize.pData_[4] = '\0';
        io_->read(junkSize.pData_, 4);
        uint64_t size = Exiv2::getULong(junkSize.pData_, littleEndian);
        if(!d->m_decodeMetaData){
            HeaderChunk* tmpHeaderChunk = new HeaderChunk();
            tmpHeaderChunk->m_headerLocation = io_->tell() - 8;
            tmpHeaderChunk->m_headerSize = size;
            d->m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
            io_->seek(size,BasicIo::cur);
        }
        else junkHandler(size);
    }
    else if(UtilsVideo::compareTagValue(chkMainId, "IDIT")){
        DataBuf dataSize((const int32_t)5);
        dataSize.pData_[4] = '\0';
        io_->read(dataSize.pData_, 4);
        uint64_t size = Exiv2::getULong(dataSize.pData_, littleEndian);
        if(!d->m_decodeMetaData){
            HeaderChunk* tmpHeaderChunk = new HeaderChunk();
            tmpHeaderChunk->m_headerLocation = io_->tell() - 8;
            tmpHeaderChunk->m_headerSize = size;
            d->m_riffFileSkeleton.m_headerChunks.push_back(tmpHeaderChunk);
            io_->seek(size,BasicIo::cur);
        }
        else dateTimeOriginal(size);

    }

    tagDecoder();
    return;
} // RiffVideo::tagDecoder

void RiffVideo::streamDataTagHandler(int32_t size){
    const int32_t bufMinSize = 20000;
    DataBuf buf(bufMinSize);
    buf.pData_[4] = '\0';
    uint64_t cur_pos = io_->tell();

    io_->read(buf.pData_, 8);

    if(UtilsVideo::compareTagValue(buf, "AVIF")){

        if (size - 4 < 0){
#ifndef SUPPRESS_WARNINGS
            EXV_ERROR   << " Exif Tags found in this RIFF file are not of valid size ."
                        << " Entries considered invalid. Not Processed.\n";
#endif
        }
        else{
            io_->read(buf.pData_, size - 4);

            IptcData iptcData;
            XmpData  xmpData;
            DummyTiffHeader tiffHeader(littleEndian);
            TiffParserWorker::decode(exifData_,
                                     iptcData,
                                     xmpData,
                                     buf.pData_,
                                     buf.size_,
                                     Tag::root,
                                     TiffMapping::findDecoder,
                                     &tiffHeader);

#ifndef SUPPRESS_WARNINGS
            if (!iptcData.empty()) EXV_WARNING << "Ignoring IPTC information encoded in the Exif data.\n";
            if (!xmpData.empty()) EXV_WARNING << "Ignoring XMP information encoded in the Exif data.\n";
#endif
        }
    }
    // TODO decode CasioData and ZORA Tag
    io_->seek(cur_pos + size, BasicIo::beg);
    return;
} // RiffVideo::streamDataTagHandler

void RiffVideo::dateTimeOriginal(int32_t size, int32_t i){
    uint64_t cur_pos = io_->tell();
    if(!d->m_modifyMetadata){
        io_->seek(-4,BasicIo::cur);
        DataBuf dataLength(2);
        dataLength.pData_[1] = '\0';

        io_->read(dataLength.pData_,1);
        uint64_t bufMinSize = (uint64_t)Exiv2::getShort(dataLength.pData_, littleEndian);
        DataBuf buf((uint64_t)bufMinSize);
        io_->seek(3,BasicIo::cur);
        io_->read(buf.pData_, bufMinSize);

        if(!i) xmpData_["Xmp.video.DateUT"] = buf.pData_;
        else xmpData_["Xmp.video.StreamName"] = buf.pData_;
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else{
        DataBuf dataLength(2);
        dataLength.pData_[1] = '\0';
        io_->read(dataLength.pData_,1);
        io_->seek(3,BasicIo::cur);
        int32_t bufMinSize = (uint64_t)Exiv2::getShort(dataLength.pData_, littleEndian);

        if(xmpData_["Xmp.video.DateUT"].count() > 0) writeStringData(xmpData_["Xmp.video.DateUT"],bufMinSize);
        else if(xmpData_["Xmp.video.StreamName"].count() >0) writeStringData(xmpData_["Xmp.video.StreamName"],bufMinSize);
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    return;
} // RiffVideo::dateTimeOriginal

void RiffVideo::odmlTagsHandler(){
    const int32_t bufMinSize = 4;
    DataBuf buf(bufMinSize + 1);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_,4);

    if(!d->m_modifyMetadata){
        uint64_t size = Exiv2::getULong(buf.pData_, littleEndian);
        uint64_t size2 = size;

        uint64_t cur_pos = io_->tell();
        io_->read(buf.pData_, 4); size -= 4;

        while(size > 0){
            io_->read(buf.pData_, 4); size -= 4;
            if(UtilsVideo::compareTagValue(buf,"DMLH")){
                io_->read(buf.pData_, 4); size -= 4;
                io_->read(buf.pData_, 4); size -= 4;
                xmpData_["Xmp.video.TotalFrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
            }
        }
        io_->seek(cur_pos + size2, BasicIo::beg);
    }
    else{
        if(xmpData_["Xmp.video.TotalFrameCount"].count() > 0){
            uint64_t tmpSize = Exiv2::getULong(buf.pData_, littleEndian);
            while(tmpSize > 0){
                io_->read(buf.pData_,4); tmpSize -= 4;
                if(UtilsVideo::compareTagValue(buf,"DMLH")){
                    io_->seek(4,BasicIo::cur);
                    writeLongData(xmpData_["Xmp.video.TotalFrameCount"]);
                    return;
                }
            }
        }
    }
    return;
} // RiffVideo::odmlTagsHandler

void RiffVideo::skipListData(){
    const int32_t bufMinSize = 4;
    DataBuf buf(bufMinSize+1);
    buf.pData_[4] = '\0';
    io_->seek(-12, BasicIo::cur);
    io_->read(buf.pData_, 4);
    uint64_t size = Exiv2::getULong(buf.pData_, littleEndian);

    uint64_t cur_pos = io_->tell();
    io_->seek(cur_pos + size, BasicIo::beg);
    return;
} // RiffVideo::skipListData

void RiffVideo::nikonTagsHandler(){
    const int32_t bufMinSize = 100;
    DataBuf buf(bufMinSize), buf2(4+1);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);

    int32_t internal_size = 0, tagID = 0, dataSize = 0, tempSize, size = Exiv2::getULong(buf.pData_, littleEndian);
    tempSize = size; char str[9] = " . . . ";
    uint64_t internal_pos, cur_pos; internal_pos = cur_pos = io_->tell();

    const TagDetails* td;
    double denominator = 1;
    io_->read(buf.pData_, 4); tempSize -= 4;

    while((int32_t)tempSize > 0){
        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 4);
        io_->read(buf2.pData_, 4);
        int32_t temp = internal_size = Exiv2::getULong(buf2.pData_, littleEndian);
        internal_pos = io_->tell(); tempSize -= (internal_size + 8);

        if(UtilsVideo::compareTagValue(buf, "NCVR")){
            while((int32_t)temp > 3){
                std::memset(buf.pData_, 0x0, buf.size_);
                io_->read(buf.pData_, 2);
                tagID = Exiv2::getULong(buf.pData_, littleEndian);
                io_->read(buf.pData_, 2);
                dataSize = Exiv2::getULong(buf.pData_, littleEndian);
                temp -= (4 + dataSize);

                if(tagID == 0x0001){
                    if (dataSize <= 0){
#ifndef SUPPRESS_WARNINGS
                        EXV_ERROR   << " Makernotes found in this RIFF file are not of valid size ."
                                    << " Entries considered invalid. Not Processed.\n";
#endif
                    }
                    else if(!d->m_modifyMetadata){
                        io_->read(buf.pData_, dataSize);
                        xmpData_["Xmp.video.MakerNoteType"] = buf.pData_;
                    }
                    else writeStringData(xmpData_["Xmp.video.MakerNoteType"],dataSize);
                }
                else if (tagID == 0x0002){
                    if (dataSize <= 0){
                    }
                    else if(!d->m_modifyMetadata){
                        while(dataSize){
                            std::memset(buf.pData_, 0x0, buf.size_); io_->read(buf.pData_, 1);
                            str[(4 - dataSize) * 2] = (char)(Exiv2::getULong(buf.pData_, littleEndian) + 48);
                            --dataSize;
                        }
                        xmpData_["Xmp.video.MakerNoteVersion"] = str;
                    }
                    //this is necessary block
                    else if(xmpData_["Xmp.video.MakerNoteVersion"].count() > 0){
                        int32_t originalDataSize = dataSize;
                        std::string makerNoteVersion = xmpData_["Xmp.video.MakerNoteVersion"].toString();
                        Exiv2::byte* rawMakerNoteVersion = new byte[originalDataSize];
                        int32_t tmpCounter = 0;
                        int32_t i;
                        for(i=(int32_t)min((int32_t)dataSize,(int32_t)makerNoteVersion.size());
                            i>=(int32_t)0; i--){
                            rawMakerNoteVersion[tmpCounter] = (Exiv2::byte) (makerNoteVersion[(4 - i) * 2] - 48);
                            tmpCounter++;
                        }
                        if((int32_t)originalDataSize > (int32_t)makerNoteVersion.size()){
                            for(i=(int32_t)(makerNoteVersion.size()); i<originalDataSize; i++) rawMakerNoteVersion[i] = (Exiv2::byte)0;
                        }
                        io_->write(rawMakerNoteVersion,originalDataSize);
                        delete[] rawMakerNoteVersion;
                    }
                    else io_->seek(dataSize,BasicIo::cur);
                }
            }
        }
        else if(UtilsVideo::compareTagValue(buf, "NCTG")){
            if(!d->m_modifyMetadata){
                while((int32_t)temp > 3){
                    std::memset(buf.pData_, 0x0, buf.size_);
                    io_->read(buf.pData_, 2);
                    tagID = Exiv2::getULong(buf.pData_, littleEndian);
                    io_->read(buf.pData_, 2);
                    dataSize = Exiv2::getULong(buf.pData_, littleEndian);
                    temp -= (4 + dataSize);
                    td = find(nikonAVITags , tagID);

                    if (dataSize <= 0){
                    }
                    else{
                        switch (tagID)
                        {
                        case 0x0003: case 0x0004: case 0x0005: case 0x0006:
                        case 0x0013: case 0x0014: case 0x0018: case 0x001d:
                        case 0x001e: case 0x001f: case 0x0020:
                            io_->read(buf.pData_, dataSize);
                            xmpData_[exvGettext(td->label_)] = buf.pData_;
                            break;
                        case 0x0007: case 0x0010: case 0x0011: case 0x000c:
                        case 0x0012:
                            io_->read(buf.pData_, dataSize);
                            xmpData_[exvGettext(td->label_)] = Exiv2::getULong(buf.pData_, littleEndian);
                            break;
                        case 0x0008: case 0x0009: case 0x000a: case 0x000b:
                        case 0x000f: case 0x001b: case 0x0016:

                            io_->read(buf.pData_, dataSize);
                            buf2.pData_[0] = buf.pData_[4];
                            buf2.pData_[1] = buf.pData_[5];
                            buf2.pData_[2] = buf.pData_[6];
                            buf2.pData_[3] = buf.pData_[7];
                            denominator = (double)Exiv2::getLong(buf2.pData_, littleEndian);

                            if (denominator != 0) xmpData_[exvGettext(td->label_)] = (double)Exiv2::getLong(buf.pData_, littleEndian) / denominator;
                            else xmpData_[exvGettext(td->label_)] = 0;
                            break;

                        default: io_->read(buf.pData_, dataSize);
                            break;
                        }
                    }
                }
            }
            else{
                while((int32_t)temp > 3){
                    std::memset(buf.pData_, 0x0, buf.size_);
                    io_->read(buf.pData_, 2);
                    tagID = Exiv2::getULong(buf.pData_, littleEndian);
                    io_->read(buf.pData_, 2);
                    dataSize = Exiv2::getULong(buf.pData_, littleEndian);
                    temp -= (4 + dataSize);
                    td = find(nikonAVITags , tagID);

                    if (dataSize <= 0){
                    }
                    else{
                        switch (tagID){
                        case 0x0003: case 0x0004: case 0x0005: case 0x0006:
                        case 0x0013: case 0x0014: case 0x0018: case 0x001d:
                        case 0x001e: case 0x001f: case 0x0020:
                            writeStringData(xmpData_[exvGettext(td->label_)],dataSize);
                            break;
                        case 0x0007: case 0x0010: case 0x0011: case 0x000c:
                        case 0x0012:
                            writeLongData(xmpData_[exvGettext(td->label_)],dataSize);
                            break;
                        case 0x0008: case 0x0009: case 0x000a: case 0x000b:
                        case 0x000f: case 0x001b: case 0x0016:

                            io_->read(buf.pData_, dataSize);
                            buf2.pData_[0] = buf.pData_[4];
                            buf2.pData_[1] = buf.pData_[5];
                            buf2.pData_[2] = buf.pData_[6];
                            buf2.pData_[3] = buf.pData_[7];
                            denominator = (double)Exiv2::getLong(buf2.pData_, littleEndian);

                            //Variation
                            if(xmpData_[exvGettext(td->label_)].count() > 0){
                                io_->seek(-dataSize,BasicIo::cur);
                                int32_t nikonData = (int32_t)(xmpData_[exvGettext(td->label_)].toFloat()*denominator);
                                io_->write((Exiv2::byte*)&nikonData,4);
                                io_->seek((dataSize-4),BasicIo::cur);
                            }
                            else io_->seek(dataSize,BasicIo::cur);
                            break;

                        default: io_->read(buf.pData_, dataSize);
                            break;
                        }
                    }
                }
            }

        }

        else if(UtilsVideo::compareTagValue(buf, "NCTH")){//TODO Nikon Thumbnail Image
        }

        else if(UtilsVideo::compareTagValue(buf, "NCVW")){//TODO Nikon Preview Image
        }

        io_->seek(internal_pos + internal_size, BasicIo::beg);
    }

    if (size ==0) io_->seek(cur_pos + 4, BasicIo::beg);
    else io_->seek(cur_pos + size, BasicIo::beg);
    return;
} // RiffVideo::nikonTagsHandler

void RiffVideo::infoTagsHandler(){
    const int32_t bufMinSize = 4;
    DataBuf buf(bufMinSize + 1);
    buf.pData_[4] = '\0';
    const int32_t cur_pos = io_->tell();
    io_->read(buf.pData_, 4);
    int32_t size = Exiv2::getULong(buf.pData_, littleEndian);
    io_->seek(4,BasicIo::cur);

    int32_t dataLenght = size;
    const TagVocabulary* tv;

    int32_t infoSize;
    if(!d->m_modifyMetadata){
        const char allPrimitiveFlags[][5]={"GENR","DIRC","IART","ICNM","IEDT","IENG","ILNG",
                                           "IMUS","INAM","IPRO","ISRC","ISTR","LANG","LOCA",
                                           "TAPE","YEAR","IPRD","IKEY","ICOP","ICNT","CMNT"};
        while(dataLenght > 12){
            io_->read(buf.pData_, 4);
            if(UtilsVideo::compareTagValue(buf,allPrimitiveFlags,(int32_t)(sizeof(allPrimitiveFlags)/5))){
                PrimitiveChunk* tmpPremitiveChunk = new PrimitiveChunk();
                memcpy((Exiv2::byte*)tmpPremitiveChunk->m_chunkId,(const Exiv2::byte*)buf.pData_,5);
                tmpPremitiveChunk->m_chunkLocation = io_->tell() - 4;
                d->m_riffFileSkeleton.m_primitiveChunks.push_back(tmpPremitiveChunk);
            }

            tv = find(infoTags , Exiv2::toString(buf.pData_));
            io_->read(buf.pData_, 4);
            infoSize = Exiv2::getULong(buf.pData_, littleEndian);
            dataLenght -= infoSize;
            if(tv && infoSize >  4){
                DataBuf tmpBuf((uint64_t)infoSize+1);
                tmpBuf.pData_[infoSize] = '\0';
                io_->read(tmpBuf.pData_,infoSize);
                xmpData_[exvGettext(tv->label_)] = tmpBuf.pData_;
            }
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else{
        //First write the tags which already exist in a file
        std::vector<std::string> existingInfoTags;
        while(dataLenght > 0){
            io_->read(buf.pData_, 4);
            tv = find(infoTags , Exiv2::toString( buf.pData_));
            io_->read(buf.pData_, 4);
            infoSize = Exiv2::getULong(buf.pData_, littleEndian);
            dataLenght -= infoSize;

            if(tv){
                if(xmpData_[exvGettext(tv->label_)].count() >0){
                    existingInfoTags.push_back((std::string)tv->voc_);
                    writeStringData(xmpData_[exvGettext(tv->label_)],infoSize);
                }
            }
        }

        //Secondly write tags which are supported by Digikam,
        //Note that create new LIST chunk is created if tag did not exist before
        std::vector<std::pair<std::string,std::string> > writethis;
        for(int32_t i =0; i< (int32_t)(sizeof(infoTags)/sizeof(infoTags[0])); i++){
            const std::string infoTagData = xmpData_[infoTags[i].label_].toString();
            const std::string infoTagFlag = (std::string)infoTags[i].voc_;
            if((xmpData_[infoTags[i].label_].count() > 0) &&
                    ((findChunkPositions(infoTags[i].voc_)).size() == 0)
                    && !(std::find(existingInfoTags.begin(), existingInfoTags.end(), infoTagFlag) != existingInfoTags.end()))
                writethis.push_back(make_pair(infoTagData,infoTagFlag));
        }

        if(writethis.size() > 0){
            io_->seek(cur_pos, BasicIo::beg);
            writeNewSubChunks(writethis);
        }

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    return;
} // RiffVideo::infoTagsHandler

void RiffVideo::junkHandler(int32_t size){
    const int32_t bufMinSize = size;

    if (size < 0){
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR   << " Junk Data found in this RIFF file are not of valid size ."
                    << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + 4, BasicIo::beg);
    }

    else{
        DataBuf buf(bufMinSize+1), buf2(4+1);
        uint64_t cur_pos = io_->tell();

        io_->read(buf.pData_, 4);
        //! Pentax Metadata and Tags
        if(UtilsVideo::compareTagValue(buf, "PENT")){
            io_->seek(cur_pos + 18, BasicIo::beg);
            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.Make"] = buf.pData_;

            io_->read(buf.pData_, 50);
            xmpData_["Xmp.video.Model"] = buf.pData_;

            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 8);
            buf2.pData_[0] = buf.pData_[4]; buf2.pData_[1] = buf.pData_[5];
            buf2.pData_[2] = buf.pData_[6]; buf2.pData_[3] = buf.pData_[7];
            xmpData_["Xmp.video.FNumber"] = (double)Exiv2::getLong(buf.pData_, littleEndian) / (double)Exiv2::getLong(buf2.pData_, littleEndian);;

            io_->seek(cur_pos + 131, BasicIo::beg);
            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.DateTimeOriginal"] = buf.pData_;

            io_->read(buf.pData_, 26);
            xmpData_["Xmp.video.DateTimeDigitized"] = buf.pData_;

            io_->seek(cur_pos + 299, BasicIo::beg);
            std::memset(buf.pData_, 0x0, buf.size_);

            io_->read(buf.pData_, 2);
            Exiv2::XmpTextValue tv(Exiv2::toString(Exiv2::getLong(buf.pData_, littleEndian)));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:width"), &tv);

            io_->read(buf.pData_, 2);
            tv.read(Exiv2::toString(Exiv2::getLong(buf.pData_, littleEndian)));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:height"), &tv);

            io_->read(buf.pData_, 4);

            /* TODO - Storing the image Thumbnail in Base64 Format


            uint64_t length = Exiv2::getLong(buf.pData_, littleEndian);
            io_->read(buf.pData_, length);

            char *rawStr = Exiv2::toString(buf.pData_);
            char *encodedStr;

            SXMPUtils::EncodeToBase64(rawStr, encodedStr);

            tv.read(Exiv2::toString(encodedStr));
            xmpData_.add(Exiv2::XmpKey("Xmp.xmp.Thumbnails/xmpGImg:image"), &tv);
        */
        }
        else{
            io_->seek(cur_pos, BasicIo::beg);
            io_->read(buf.pData_, size);
            xmpData_["Xmp.video.Junk"] = buf.pData_;
        }

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    return;
} // RiffVideo::junkHandler

void RiffVideo::aviHeaderTagsHandler(int32_t size){
    if(!d->m_modifyMetadata){
        const int32_t bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] = '\0';
        int32_t width = 0, height = 0, frame_count = 0;
        double frame_rate = 1;

        uint64_t cur_pos = io_->tell(),i;

        for( i = 0; i <= 9; i++){
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, bufMinSize);

            switch(i){
            case frameRate: xmpData_["Xmp.video.MicroSecPerFrame"] = Exiv2::getULong(buf.pData_, littleEndian);
                frame_rate = (double)1000000/(double)Exiv2::getULong(buf.pData_, littleEndian); break;
            case (maxDataRate): xmpData_["Xmp.video.MaxDataRate"] = (double)Exiv2::getULong(buf.pData_, littleEndian)/(double)1024; break;
            case frameCount: xmpData_["Xmp.video.FrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                frame_count = Exiv2::getULong(buf.pData_, littleEndian); break;
            case initialFrames: xmpData_["Xmp.video.InitialFrames"] = Exiv2::getULong(buf.pData_, littleEndian); break;
            case streamCount: xmpData_["Xmp.video.StreamCount"] = Exiv2::getULong(buf.pData_, littleEndian); break;
            case suggestedBufferSize: xmpData_["Xmp.video.SuggestedBufferSize"] = Exiv2::getULong(buf.pData_, littleEndian); break;
            case imageWidth_h: width = Exiv2::getULong(buf.pData_, littleEndian);
                xmpData_["Xmp.video.Width"] = width; break;
            case imageHeight_h: height = Exiv2::getULong(buf.pData_, littleEndian);
                xmpData_["Xmp.video.Height"] = height; break;
            }
        }

        fillAspectRatio(width, height);
        fillDuration(frame_rate, frame_count);

        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else//overwrite metadata from current xmpData_ container{
        for(int32_t i = 0; i <= 9; i++){
            switch(i){
            case frameRate: writeLongData(xmpData_["Xmp.video.MicroSecPerFrame"]); break;
            case (maxDataRate): if(xmpData_["Xmp.video.MaxDataRate"].count() > 0 ){
                    int32_t maxDataRate = (int32_t)(xmpData_["Xmp.video.MaxDataRate"].toFloat()*1024);
                    io_->write((Exiv2::byte*)&maxDataRate,4);
                }
                else io_->seek(4,BasicIo::cur);
                break;
            case frameCount: writeLongData(xmpData_["Xmp.video.FrameCount"]); break;
            case initialFrames: writeLongData(xmpData_["Xmp.video.InitialFrames"]); break;
            case streamCount: writeLongData(xmpData_["Xmp.video.StreamCount"]); break;
            case suggestedBufferSize: writeLongData(xmpData_["Xmp.video.SuggestedBufferSize"]); break;
            case imageWidth_h: writeLongData(xmpData_["Xmp.video.Width"]); break;
            case imageHeight_h: writeLongData(xmpData_["Xmp.video.Height"]); break;
            default: io_->seek(4,BasicIo::cur); break;
            }
        }
    return;
} // RiffVideo::aviHeaderTagsHandler

void RiffVideo::setStreamType(){
    DataBuf chkId((uint64_t)5);
    io_->read(chkId.pData_,(uint64_t)4);
    if(UtilsVideo::compareTagValue(chkId, "VIDS"))
        d->streamType_ = Video;
    else if (UtilsVideo::compareTagValue(chkId, "AUDS"))
        d->streamType_ = Audio;
    return;
}//RiffVideo::setStreamType

void RiffVideo::streamHandler(int32_t size){
    if (!d->m_modifyMetadata){
        uint64_t bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] =  '\0';
        int32_t divisor = 1;
        uint64_t cur_pos = io_->tell() - 4;

        int32_t i;
        for( i=1; i<=25; i++){
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, bufMinSize);

            switch(i){
            case codec:
                if(d->streamType_ == Video) xmpData_["Xmp.video.Codec"] = buf.pData_;
                else if (d->streamType_ == Audio) xmpData_["Xmp.audio.Codec"] = buf.pData_;
                else xmpData_["Xmp.video.Codec"] = buf.pData_;
                break;
            case sampleRate: divisor=Exiv2::getULong(buf.pData_, littleEndian); break;
            case (sampleRate+1): if(d->streamType_ == Video) xmpData_["Xmp.video.FrameRate"] = returnSampleRate(buf,divisor);
                else if (d->streamType_ == Audio) xmpData_["Xmp.audio.SampleRate"] = returnSampleRate(buf,divisor);
                else xmpData_["Xmp.video.StreamSampleRate"] = returnSampleRate(buf,divisor);
                break;
            case sampleCount: if(d->streamType_ == Video) xmpData_["Xmp.video.FrameCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if (d->streamType_ == Audio) xmpData_["Xmp.audio.SampleCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                else xmpData_["Xmp.video.StreamSampleCount"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case quality: if(d->streamType_ == Video) xmpData_["Xmp.video.VideoQuality"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if(d->streamType_ != Audio) xmpData_["Xmp.video.StreamQuality"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            case sampleSize: if(d->streamType_ == Video) xmpData_["Xmp.video.VideoSampleSize"] = Exiv2::getULong(buf.pData_, littleEndian);
                else if(d->streamType_ != Audio) xmpData_["Xmp.video.StreamSampleSize"] = Exiv2::getULong(buf.pData_, littleEndian);
                break;
            }

        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else{
        DataBuf chkId((uint64_t)5);
        chkId.pData_[4] = '\0';
        const int32_t bufMinSize = 4;

        int32_t multiplier;
        if(d->streamType_ == Video){
            writeStringData(xmpData_["Xmp.video.Codec"],4,12);
            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            //Variation
            if(xmpData_["Xmp.video.FrameRate"].count() > 0){
                int32_t frameRate = (int32_t)xmpData_["Xmp.video.FrameRate"].toFloat()*multiplier;
                io_->write((Exiv2::byte*)&frameRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else io_->seek(8,BasicIo::cur);

            writeLongData(xmpData_["Xmp.video.FrameCount"],4,4);
            writeLongData(xmpData_["Xmp.video.VideoQuality"],4);
            writeLongData(xmpData_["Xmp.video.VideoSampleSize"],4);
        }
        else if(d->streamType_ == Audio){
            writeStringData(xmpData_["Xmp.audio.Codec"],4,12);

            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            //Variation
            if(xmpData_["Xmp.audio.SampleRate"].count() > 0){
                int32_t sampleRate = (int32_t)xmpData_["Xmp.audio.SampleRate"].toLong()*multiplier;
                io_->write((Exiv2::byte*)&sampleRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else io_->seek(8,BasicIo::cur);
            writeLongData(xmpData_["Xmp.audio.SampleCount"],4);
        }
        else{
            writeLongData(xmpData_["Xmp.video.Codec"],4,12);
            io_->read(chkId.pData_,bufMinSize);
            multiplier = Exiv2::getULong(chkId.pData_, littleEndian);

            //Variation
            if(xmpData_["Xmp.video.StreamSampleRate"].count() > 0){
                int32_t streamSampleRate = (int32_t)xmpData_["Xmp.video.StreamSampleRate"].toLong()*multiplier;
                io_->write((Exiv2::byte*)&streamSampleRate,4);
                io_->seek(4,BasicIo::cur);
            }
            else io_->seek(8,BasicIo::cur);

            writeLongData(xmpData_["Xmp.video.StreamSampleCount"],4,4);
            writeLongData(xmpData_["Xmp.video.StreamQuality"],4);
            writeLongData(xmpData_["Xmp.video.StreamSampleSize"],4);
        }
    }
    return;
} // RiffVideo::streamHandler

void RiffVideo::streamFormatHandler(int32_t size){
    if (!d->m_modifyMetadata){
        const int32_t bufMinSize = 4;
        DataBuf buf(bufMinSize+1);
        buf.pData_[4] = '\0';
        uint64_t cur_pos = io_->tell();

        if(d->streamType_ == Video){
            io_->read(buf.pData_, bufMinSize);
            for(int32_t i = 0; i <= 9; i++){
                std::memset(buf.pData_, 0x0, buf.size_);
                switch(i){
                //Will be used in case of debugging
                case imageWidth: io_->read(buf.pData_, bufMinSize); break;
                case imageHeight: io_->read(buf.pData_, bufMinSize); break;
                case planes: io_->read(buf.pData_, 2);
                    xmpData_["Xmp.video.Planes"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case bitDepth: io_->read(buf.pData_, 2);
                    xmpData_["Xmp.video.PixelDepth"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case compression: io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.Compressor"] = buf.pData_; break;
                case imageLength: io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.ImageLength"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case pixelsPerMeterX: io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.PixelPerMeterX"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case pixelsPerMeterY: io_->read(buf.pData_, bufMinSize);
                    xmpData_["Xmp.video.PixelPerMeterY"] = Exiv2::getULong(buf.pData_, littleEndian); break;
                case numColors: io_->read(buf.pData_, bufMinSize);
                    if(Exiv2::getULong(buf.pData_, littleEndian) == 0) xmpData_["Xmp.video.NumOfColours"] = "Unspecified";
                    else xmpData_["Xmp.video.NumOfColours"] = Exiv2::getULong(buf.pData_, littleEndian);
                    break;
                case numImportantColors: io_->read(buf.pData_, bufMinSize);
                    if(Exiv2::getULong(buf.pData_, littleEndian)) xmpData_["Xmp.video.NumIfImpColours"] = Exiv2::getULong(buf.pData_, littleEndian);
                    else xmpData_["Xmp.video.NumOfImpColours"] = "All";
                    break;
                }
            }
        }
        else if(d->streamType_ == Audio){
            int32_t c = 0;
            const TagDetails* td;
            for(int32_t i = 0; i <= 7; i++){
                io_->read(buf.pData_, 2);

                switch(i){
                case encoding: td = find(audioEncodingValues , Exiv2::getUShort(buf.pData_, littleEndian));
                    if(td) xmpData_["Xmp.audio.Compressor"] = exvGettext(td->label_);
                    else xmpData_["Xmp.audio.Compressor"] = Exiv2::getUShort(buf.pData_, littleEndian);
                    break;
                case numberOfChannels: c = Exiv2::getUShort(buf.pData_, littleEndian);
                    if(c == 1) xmpData_["Xmp.audio.ChannelType"] = "Mono";
                    else if(c == 2) xmpData_["Xmp.audio.ChannelType"] = "Stereo";
                    else if(c == 5) xmpData_["Xmp.audio.ChannelType"] = "5.1 Surround Sound";
                    else if(c == 7) xmpData_["Xmp.audio.ChannelType"] = "7.1 Surround Sound";
                    else xmpData_["Xmp.audio.ChannelType"] = "Mono";
                    break;
                case audioSampleRate: xmpData_["Xmp.audio.SampleRate"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case avgBytesPerSec: xmpData_["Xmp.audio.SampleType"] = Exiv2::getUShort(buf.pData_, littleEndian); break;
                case bitsPerSample: xmpData_["Xmp.audio.BitsPerSample"] = Exiv2::getUShort(buf.pData_,littleEndian);
                    io_->read(buf.pData_, 2); break;
                }
            }
        }
        io_->seek(cur_pos + size, BasicIo::beg);
    }
    else{
        if(d->streamType_ == Video){
            DataBuf chkId((uint64_t) 5);
            chkId.pData_[4] = '\0';
            io_->read(chkId.pData_,4);

            writeLongData(xmpData_["Xmp.video.Width"],4);
            writeLongData(xmpData_["Xmp.video.Height"],4);
            writeShortData(xmpData_["Xmp.video.Planes"]);
            writeShortData(xmpData_["Xmp.video.PixelDepth"]);
            writeStringData(xmpData_["Xmp.video.Compressor"],4);
            writeLongData(xmpData_["Xmp.video.ImageLength"],4);
            writeLongData(xmpData_["Xmp.video.PixelPerMeterX"],4);
            writeLongData(xmpData_["Xmp.video.PixelPerMeterY"],4);
            writeLongData(xmpData_["Xmp.video.NumOfColours"],4);

            //Variation
            if(xmpData_["Xmp.video.NumOfImpColours"].count() > 0){
                if(std::strcmp(xmpData_["Xmp.video.NumOfImpColours"].toString().c_str(),"All")){
                    const int32_t numImportantColours = (int32_t) 0;
                    io_->write((Exiv2::byte*)&numImportantColours,4);
                }
                else writeLongData(xmpData_["Xmp.video.NumOfImpColours"],4);
            }
            else io_->seek(4,BasicIo::cur);
        }
        else if(d->streamType_ == Audio){
            const RevTagDetails* rtd;
            //Variation
            if(xmpData_["Xmp.audio.Compressor"].count() > 0){
                RevTagDetails revTagDetails[(sizeof(audioEncodingValues)/
                                             sizeof(audioEncodingValues[0]))];
                reverseTagDetails(audioEncodingValues,revTagDetails,
                                  (sizeof(audioEncodingValues)
                                   /sizeof(audioEncodingValues[0])));
                rtd = find(revTagDetails,xmpData_["Xmp.audio.Compressor"].toString());
                if(rtd){
                    uint16_t compressorCode= (uint16_t)rtd->val_;
                    io_->write((Exiv2::byte*)&compressorCode,2);
                }
                else io_->seek(2,BasicIo::cur);
            }
            else io_->seek(2,BasicIo::cur);

            if(xmpData_["Xmp.audio.ChannelType"].count() > 0){
                const int32_t sizeChan = xmpData_["Xmp.audio.ChannelType"].size();
                DataBuf tmpBuf(sizeChan+1);
                tmpBuf.pData_[sizeChan+1] = '\0';
                const std::string audioChannealType = xmpData_["Xmp.audio.ChannelType"].toString();
                int32_t i;
                for( i=0; i<=sizeChan;i++) tmpBuf.pData_[i] = (Exiv2::byte)audioChannealType[i];
                uint16_t channelType = 0;
                if(UtilsVideo::simpleBytesComparision(tmpBuf,"MONO",sizeChan)) channelType = 1;
                else if(UtilsVideo::simpleBytesComparision(tmpBuf,"STEREO",sizeChan)) channelType = 2;
                else if(UtilsVideo::simpleBytesComparision(tmpBuf,"5.1 SURROUND SOUND",sizeChan)) channelType = 5;
                else if(UtilsVideo::simpleBytesComparision(tmpBuf,"7.1 SURROUND SOUND",sizeChan)) channelType = 7;
                io_->write((Exiv2::byte*)&channelType,2);
            }
            else io_->seek(2,BasicIo::cur);

            writeShortData(xmpData_["Xmp.audio.SampleRate"],2,2);
            writeShortData(xmpData_["Xmp.audio.SampleType"],2,4);
            writeShortData(xmpData_["Xmp.audio.BitsPerSample"],2);
        }
    }
    return;
} // RiffVideo::streamFormatHandler

double RiffVideo::returnSampleRate(Exiv2::DataBuf& buf, int32_t divisor){
    return ((double)Exiv2::getULong(buf.pData_, littleEndian) / (double)divisor);
} // RiffVideo::returnSampleRate

const char* RiffVideo::printAudioEncoding(uint64_t i){
    const TagDetails* td;
    td = find(audioEncodingValues , i);
    if(td) return exvGettext(td->label_);

    return "Undefined";
} // RiffVideo::printAudioEncoding

void RiffVideo::fillAspectRatio(int32_t width, int32_t height){
    double aspectRatio = (double)width / (double)height;
    aspectRatio = floor(aspectRatio*10) / 10;
    xmpData_["Xmp.video.AspectRatio"] = aspectRatio;

    int32_t aR = (int32_t) ((aspectRatio*10.0)+0.1);

    switch  (aR) {
    case 13 : xmpData_["Xmp.video.AspectRatio"] = "4:3"		; break;
    case 17 : xmpData_["Xmp.video.AspectRatio"] = "16:9"	; break;
    case 10 : xmpData_["Xmp.video.AspectRatio"] = "1:1"		; break;
    case 16 : xmpData_["Xmp.video.AspectRatio"] = "16:10"	; break;
    case 22 : xmpData_["Xmp.video.AspectRatio"] = "2.21:1"  ; break;
    case 23 : xmpData_["Xmp.video.AspectRatio"] = "2.35:1"  ; break;
    case 12 : xmpData_["Xmp.video.AspectRatio"] = "5:4"     ; break;
    default : xmpData_["Xmp.video.AspectRatio"] = aspectRatio;break;
    }
    return;
} // RiffVideo::fillAspectRatio

void RiffVideo::fillDuration(double frame_rate, int32_t frame_count){
    if(frame_rate == 0) return;

    uint64_t duration = static_cast<uint64_t>((double)frame_count * (double)1000 / (double)frame_rate);
    xmpData_["Xmp.video.FileDataRate"] = (double)io_->size()/(double)(1048576*duration);
    xmpData_["Xmp.video.Duration"] = duration; //Duration in number of seconds
    return;
} // RiffVideo::fillDuration

void RiffVideo::reverseTagDetails(const TagDetails inputTagVocabulary[],
                                  RevTagDetails  outputTagVocabulary[] ,int32_t size){
    for(int32_t i=0; i<size ;i++){
        outputTagVocabulary[i].label_ = inputTagVocabulary[i].label_;
        outputTagVocabulary[i].val_ = inputTagVocabulary[i].val_;
    }
    return;
} // RiffVideo::reverseTagDetails

Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool /*create*/){
    Image::AutoPtr image(new RiffVideo(io));
    if (!image->good()) image.reset();
    return image;
}

/*!
 * \brief RiffVideo::copyRestOfTheFile
 * Move all data to next locations,Currently its blockwise copy,which is inefficient for a
 * huge video files,and should be changed to inserting technique without moving huge data
 * like Bundle technology as suggested by Robin.
 * \param oldSavedData
 * \return
 */
bool RiffVideo::copyRestOfTheFile(DataBuf oldSavedData){

    int32_t writeSize = oldSavedData.size_;
    int32_t readSize = writeSize;
    DataBuf readBuf((int32_t)max(readSize,(int32_t)1024));
    DataBuf writeBuf((int32_t)max(readSize,(int32_t)1024));

    memcpy(writeBuf.pData_,oldSavedData.pData_,readSize);
    bool lastStream = false,hasData = true;

    while(hasData){
        if((io_->size() - io_->tell()) < 1024){
            hasData = false;
            lastStream = true;
            readSize = (io_->size() - io_->tell());
        }

        io_->read(readBuf.pData_,readSize);
        io_->seek(-readSize,BasicIo::cur);
        io_->write(writeBuf.pData_,writeSize);

        memcpy(writeBuf.pData_,readBuf.pData_,readSize);
        writeSize = readSize;
        if(lastStream) io_->write(writeBuf.pData_,readSize);
    }
    return true;
}

/*!
 * \brief RiffVideo::writeNewSubChunks
 *                   precondition:INFO List chunk must exist,atleast with LIST tag,
 *                   and size field with size one(INFO tag which appear next)
 *
 *            |----LIST(Must exist before calling this method)
 *            |----Size(Must exist, atleast with size 1 if no INFO subchunks exist before)
 *                     |----INFO (Must exist,exactly with same HeaderChunkId INFO)
 *
 * \param chunkData
 * \return
 */
bool RiffVideo::writeNewSubChunks(std::vector<std::pair<std::string,std::string> > chunkData){
    bool  result = true;
    int32_t  addSize = 0;
    for (vector<std::pair<std::string,std::string> >::iterator it = chunkData.begin();
         it != chunkData.end(); it++){
        std::pair<std::string,std::string> unitChunk = (*it);
        addSize += (int32_t)unitChunk.first.size() + (int32_t)8;
        addSize += addSize%2;
    }
    const int32_t cur_pos = io_->tell();
    DataBuf buf((uint64_t)5);
    buf.pData_[4] = '\0';
    io_->seek(4,BasicIo::beg);
    io_->read(buf.pData_,4);
    io_->seek(-4,BasicIo::cur);

    const int32_t originalSize = Exiv2::getULong(buf.pData_, littleEndian);
    int32_t newRiffSize = originalSize + addSize ;
    io_->write((Exiv2::byte*)&newRiffSize,4);

    io_->seek(cur_pos,BasicIo::beg);
    io_->read(buf.pData_,4);
    io_->seek(-4,BasicIo::cur);

    const int32_t originalListSize = Exiv2::getULong(buf.pData_, littleEndian);
    int32_t newListSize = originalListSize + addSize ;
    io_->write((Exiv2::byte*)&newListSize,4);

    io_->read(buf.pData_,4);
    std::vector<DataBuf> previousData;
    uint64_t i;

    for (vector<std::pair<std::string,std::string> >::iterator it = chunkData.begin();
         it != chunkData.end(); it++){
        std::pair<std::string,std::string> unitChunk = (*it);
        std::string chunkId = unitChunk.second;
        std::string tmpChunkData = unitChunk.first;
        uint64_t chunkSize = tmpChunkData.size();
        chunkSize += chunkSize%2;

        DataBuf copiedData((int32_t)(chunkSize+8));

        io_->read(copiedData.pData_,(chunkSize+8));
        io_->seek(((-1)*(chunkSize+8)),BasicIo::cur);
        previousData.push_back(copiedData);
        Exiv2::byte rawChunkId[4] = {(Exiv2::byte)chunkId[0],(Exiv2::byte)chunkId[1],
                                     (Exiv2::byte)chunkId[2],(Exiv2::byte)chunkId[3]};
        Exiv2::byte* rawChunkData = new byte[(int32_t)chunkSize];

        for(i=0; i < chunkSize; i++) rawChunkData[i] = (Exiv2::byte)tmpChunkData[i];
        if(tmpChunkData.size()%2 != 0) rawChunkData[(int32_t)(chunkSize-1)] = (Exiv2::byte)0;

        io_->write(rawChunkId,4);
        io_->write((Exiv2::byte*)&chunkSize,4);
        io_->write(rawChunkData,chunkSize);
        delete[] rawChunkData;
    }

    int32_t accessIndex = 0;
    int32_t netNewMetadataSize = 0;
    for (i=0; i < (int32_t)previousData.size(); i++) netNewMetadataSize += previousData[i].size_;

    DataBuf metadataPacket((uint64_t)netNewMetadataSize);
    for (i=0; i < previousData.size(); i++){
        for(int32_t assignIndex=0; assignIndex<previousData[i].size_; assignIndex++){
            metadataPacket.pData_[accessIndex] = previousData[i].pData_[assignIndex];
            accessIndex++;
        }
    }

    copyRestOfTheFile(metadataPacket);
    return result;
}

void RiffVideo::writeStringData(Exiv2::Xmpdatum xmpStringData, int32_t size, int32_t skipOffset){
    if(xmpStringData.count() > 0){
        std::string dataString = xmpStringData.toString();
        Exiv2::byte* rawData = new byte[(uint8_t)size];
        int32_t newSize = (int32_t)dataString.size();
        for(int32_t i=0; i<min(newSize,size); i++) rawData[i] = (Exiv2::byte)dataString[i];
        if(size > newSize) for(int32_t i=newSize; i<size; i++) rawData[i] = (Exiv2::byte)0;
        io_->write(rawData,size);
        io_->seek((int)skipOffset,BasicIo::cur);
        delete[] rawData;
    }
    else io_->seek((size+skipOffset),BasicIo::cur);
}

void RiffVideo::writeLongData(Exiv2::Xmpdatum xmpIntData, int32_t size, int32_t skipOffset){
    if(xmpIntData.count() > 0){
        int32_t  rawIntData = (int32_t)xmpIntData.toLong();
        io_->write((Exiv2::byte*)&rawIntData,size);
        io_->seek(skipOffset,BasicIo::cur);
    }
    else io_->seek((size+skipOffset),BasicIo::cur);
}

void RiffVideo::writeFloatData(Exiv2::Xmpdatum xmpIntData, int32_t size, int32_t skipOffset){
    if(xmpIntData.count() > 0){
        int32_t  rawIntData = (int32_t)xmpIntData.toFloat();
        io_->write((Exiv2::byte*)&rawIntData,size);
        io_->seek(skipOffset,BasicIo::cur);
    }
    else io_->seek((size+skipOffset),BasicIo::cur);
}

void RiffVideo::writeShortData(Exiv2::Xmpdatum xmpIntData, int16_t size, int32_t skipOffset){
    if(xmpIntData.count() > 0){
        int16_t  rawIntData = (int16_t)xmpIntData.toLong();
        io_->write((Exiv2::byte*)&rawIntData,size);
        io_->seek(skipOffset,BasicIo::cur);
    }
    else io_->seek((size+skipOffset),BasicIo::cur);
}

bool isRiffType(BasicIo& iIo, bool advance){
    const int32_t len = 2;
    const unsigned char RiffVideoId[4] = { 'R', 'I', 'F' ,'F'};
    Exiv2::byte* buf = new byte[len];
    iIo.read(buf, len);
    if (iIo.error() || iIo.eof()){
        delete[] buf;
        return false;
    }
    bool matched = (memcmp(buf, RiffVideoId, len) == 0);
    if (!advance || !matched) iIo.seek(-len, BasicIo::cur);
    delete[] buf;
    return matched;
}
}                                       // namespace Exiv2

