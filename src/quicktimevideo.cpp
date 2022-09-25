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
#include "quicktimevideo.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
// + standard includes
#include <array>
#include <cmath>
#include <iostream>
#include <string>
// *****************************************************************************
// class member definitions
namespace Exiv2 {
namespace Internal {

extern const TagVocabulary qTimeFileType[] = {
    {"3g2a", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-0 V1.0"},
    {"3g2b", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-A V1.0.0"},
    {"3g2c", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-B v1.0"},
    {"3ge6", "3GPP (.3GP) Release 6 MBMS Extended Presentations"},
    {"3ge7", "3GPP (.3GP) Release 7 MBMS Extended Presentations"},
    {"3gg6", "3GPP Release 6 General Profile"},
    {"3gp1", "3GPP Media (.3GP) Release 1 (probably non-existent)"},
    {"3gp2", "3GPP Media (.3GP) Release 2 (probably non-existent)"},
    {"3gp3", "3GPP Media (.3GP) Release 3 (probably non-existent)"},
    {"3gp4", "3GPP Media (.3GP) Release 4"},
    {"3gp5", "3GPP Media (.3GP) Release 5"},
    {"3gp6", "3GPP Media (.3GP) Release 6 Streaming Servers"},
    {"3gs7", "3GPP Media (.3GP) Release 7 Streaming Servers"},
    {"CAEP", "Canon Digital Camera"},
    {"CDes", "Convergent Design"},
    {"F4A ", "Audio for Adobe Flash Player 9+ (.F4A)"},
    {"F4B ", "Audio Book for Adobe Flash Player 9+ (.F4B)"},
    {"F4P ", "Protected Video for Adobe Flash Player 9+ (.F4P)"},
    {"F4V ", "Video for Adobe Flash Player 9+ (.F4V)"},
    {"JP2 ", "JPEG 2000 Image (.JP2) [ISO 15444-1 ?]"},
    {"JP20", "Unknown, from GPAC samples (prob non-existent)"},
    {"KDDI", "3GPP2 EZmovie for KDDI 3G cellphones"},
    {"M4A ", "Apple iTunes AAC-LC (.M4A) Audio"},
    {"M4B ", "Apple iTunes AAC-LC (.M4B) Audio Book"},
    {"M4P ", "Apple iTunes AAC-LC (.M4P) AES Protected Audio"},
    {"M4V ", "Apple iTunes Video (.M4V) Video"},
    {"M4VH", "Apple TV (.M4V)"},
    {"M4VP", "Apple iPhone (.M4V)"},
    {"MPPI", "Photo Player, MAF [ISO/IEC 23000-3]"},
    {"MSNV", "MPEG-4 (.MP4) for SonyPSP"},
    {"NDAS", "MP4 v2 [ISO 14496-14] Nero Digital AAC Audio"},
    {"NDSC", "MPEG-4 (.MP4) Nero Cinema Profile"},
    {"NDSH", "MPEG-4 (.MP4) Nero HDTV Profile"},
    {"NDSM", "MPEG-4 (.MP4) Nero Mobile Profile"},
    {"NDSP", "MPEG-4 (.MP4) Nero Portable Profile"},
    {"NDSS", "MPEG-4 (.MP4) Nero Standard Profile"},
    {"NDXC", "H.264/MPEG-4 AVC (.MP4) Nero Cinema Profile"},
    {"NDXH", "H.264/MPEG-4 AVC (.MP4) Nero HDTV Profile"},
    {"NDXM", "H.264/MPEG-4 AVC (.MP4) Nero Mobile Profile"},
    {"NDXP", "H.264/MPEG-4 AVC (.MP4) Nero Portable Profile"},
    {"NDXS", "H.264/MPEG-4 AVC (.MP4) Nero Standard Profile"},
    {"NIKO", "Nikon"},
    {"ROSS", "Ross Video"},
    {"avc1", "MP4 Base w/ AVC ext [ISO 14496-12:2005]"},
    {"caqv", "Casio Digital Camera"},
    {"da0a", "DMB MAF w/ MPEG Layer II aud, MOT slides, DLS, JPG/PNG/MNG images"},
    {"da0b", "DMB MAF, extending DA0A, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"da1a", "DMB MAF audio with ER-BSAC audio, JPG/PNG/MNG images"},
    {"da1b", "DMB MAF, extending da1a, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"da2a", "DMB MAF aud w/ HE-AAC v2 aud, MOT slides, DLS, JPG/PNG/MNG images"},
    {"da2b", "DMB MAF, extending da2a, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"da3a", "DMB MAF aud with HE-AAC aud, JPG/PNG/MNG images"},
    {"da3b", "DMB MAF, extending da3a w/ BIFS, 3GPP timed text, DID, TVA, REL, IPMP"},
    {"dmb1", "DMB MAF supporting all the components defined in the specification"},
    {"dmpf", "Digital Media Project"},
    {"drc1", "Dirac (wavelet compression), encapsulated in ISO base media (MP4)"},
    {"dv1a", "DMB MAF vid w/ AVC vid, ER-BSAC aud, BIFS, JPG/PNG/MNG images, TS"},
    {"dv1b", "DMB MAF, extending dv1a, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"dv2a", "DMB MAF vid w/ AVC vid, HE-AAC v2 aud, BIFS, JPG/PNG/MNG images, TS"},
    {"dv2b", "DMB MAF, extending dv2a, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"dv3a", "DMB MAF vid w/ AVC vid, HE-AAC aud, BIFS, JPG/PNG/MNG images, TS"},
    {"dv3b", "DMB MAF, extending dv3a, with 3GPP timed text, DID, TVA, REL, IPMP"},
    {"dvr1", "DVB (.DVB) over RTP"},
    {"dvt1", "DVB (.DVB) over MPEG-2 Transport Stream"},
    {"isc2", "ISMACryp 2.0 Encrypted File"},
    {"iso2", "MP4 Base Media v2 [ISO 14496-12:2005]"},
    {"isom", "MP4 Base Media v1 [IS0 14496-12:2003]"},
    {"jpm ", "JPEG 2000 Compound Image (.JPM) [ISO 15444-6]"},
    {"jpx ", "JPEG 2000 with extensions (.JPX) [ISO 15444-2]"},
    {"mj2s", "Motion JPEG 2000 [ISO 15444-3] Simple Profile"},
    {"mjp2", "Motion JPEG 2000 [ISO 15444-3] General Profile"},
    {"mmp4", "MPEG-4/3GPP Mobile Profile (.MP4/3GP) (for NTT)"},
    {"mp21", "MPEG-21 [ISO/IEC 21000-9]"},
    {"mp41", "MP4 v1 [ISO 14496-1:ch13]"},
    {"mp42", "MP4 v2 [ISO 14496-14]"},
    {"mp71", "MP4 w/ MPEG-7 Metadata [per ISO 14496-12]"},
    {"mqt ", "Sony / Mobile QuickTime (.MQV) US Patent 7,477,830 (Sony Corp)"},
    {"niko", "Nikon"},
    {"odcf", "OMA DCF DRM Format 2.0 (OMA-TS-DRM-DCF-V2_0-20060303-A)"},
    {"opf2", "OMA PDCF DRM Format 2.1 (OMA-TS-DRM-DCF-V2_1-20070724-C)"},
    {"opx2", "OMA PDCF DRM + XBS extensions (OMA-TS-DRM_XBS-V1_0-20070529-C)"},
    {"pana", "Panasonic Digital Camera"},
    {"qt  ", "Apple QuickTime (.MOV/QT)"},
    {"sdv ", "SD Memory Card Video"},
    {"ssc1", "Samsung stereoscopic, single stream"},
    {"ssc2", "Samsung stereoscopic, dual stream"}};

extern const TagVocabulary handlerClassTags[] = {{"dhlr", "Data Handler"}, {"mhlr", "Media Handler"}};

extern const TagVocabulary handlerTypeTags[] = {{"alis", "Alias Data"},
                                                {"crsm", "Clock Reference"},
                                                {"hint", "Hint Track"},
                                                {"ipsm", "IPMP"},
                                                {"m7sm", "MPEG-7 Stream"},
                                                {"mdir", "Metadata"},
                                                {"mdta", "Metadata Tags"},
                                                {"mjsm", "MPEG-J"},
                                                {"ocsm", "Object Content"},
                                                {"odsm", "Object Descriptor"},
                                                {"sdsm", "Scene Description"},
                                                {"soun", "Audio Track"},
                                                {"text", "Text"},
                                                {"tmcd", "Time Code"},
                                                {"url ", "URL"},
                                                {"vide", "Video Track"}};

extern const TagVocabulary vendorIDTags[] = {{"FFMP", "FFmpeg"},
                                             {"appl", "Apple"},
                                             {"olym", "Olympus"},
                                             {"GIC ", "General Imaging Co."},
                                             {"fe20", "Olympus (fe20)"},
                                             {"pana", "Panasonic"},
                                             {"KMPI", "Konica-Minolta"},
                                             {"kdak", "Kodak"},
                                             {"pent", "Pentax"},
                                             {"NIKO", "Nikon"},
                                             {"leic", "Leica"},
                                             {"pr01", "Olympus (pr01)"},
                                             {"SMI ", "Sorenson Media Inc."},
                                             {"mino", "Minolta"},
                                             {"sany", "Sanyo"},
                                             {"ZORA", "Zoran Corporation"},
                                             {"niko", "Nikon"}};

extern const TagVocabulary cameraByteOrderTags[] = {{"II", "Little-endian (Intel, II)"},
                                                    {"MM", "Big-endian (Motorola, MM)"}};

extern const TagDetails graphicsModetags[] = {{0x0, "srcCopy"},          {0x1, "srcOr"},
                                              {0x2, "srcXor"},           {0x3, "srcBic"},
                                              {0x4, "notSrcCopy"},       {0x5, "notSrcOr"},
                                              {0x6, "notSrcXor"},        {0x7, "notSrcBic"},
                                              {0x8, "patCopy"},          {0x9, "patOr"},
                                              {0xa, "patXor"},           {0xb, "patBic"},
                                              {0xc, "notPatCopy"},       {0xd, "notPatOr"},
                                              {0xe, "notPatXor"},        {0xf, "notPatBic"},
                                              {0x20, "blend"},           {0x21, "addPin"},
                                              {0x22, "addOver"},         {0x23, "subPin"},
                                              {0x24, "transparent"},     {0x25, "addMax"},
                                              {0x26, "subOver"},         {0x27, "addMin"},
                                              {0x31, "grayishTextOr"},   {0x32, "hilite"},
                                              {0x40, "ditherCopy"},      {0x100, "Alpha"},
                                              {0x101, "White Alpha"},    {0x102, "Pre-multiplied Black Alpha"},
                                              {0x110, "Component Alpha"}};

extern const TagVocabulary userDatatags[] = {
    {"AllF", "PlayAllFrames"},
    {"CNCV", "CompressorVersion"},
    {"CNFV", "FirmwareVersion"},
    {"CNMN", "Model"},
    {"CNTH", "CanonCNTH"},
    {"DcMD", "DcMD"},
    {"FFMV", "FujiFilmFFMV"},
    {"INFO", "SamsungINFO"},
    {"LOOP", "LoopStyle"},
    {"MMA0", "MinoltaMMA0"},
    {"MMA1", "MinoltaMMA1"},
    {"MVTG", "FujiFilmMVTG"},
    {"NCDT", "NikonNCDT"},
    {"PANA", "PanasonicPANA"},
    {"PENT", "PentaxPENT"},
    {"PXMN", "MakerNotePentax5b"},
    {"PXTH", "PentaxPreview"},
    {"QVMI", "CasioQVMI"},
    {"SDLN", "PlayMode"},
    {"SelO", "PlaySelection"},
    {"TAGS", "KodakTags/KonicaMinoltaTags/MinoltaTags/NikonTags/OlympusTags/PentaxTags/SamsungTags/SanyoMOV/SanyoMP4"},
    {"WLOC", "WindowLocation"},
    {"XMP_", "XMP"},
    {"Xtra", "Xtra"},
    {"hinf", "HintTrackInfo"},
    {"hinv", "HintVersion"},
    {"hnti", "Hint"},
    {"meta", "Meta"},
    {"name", "Name"},
    {"ptv ", "PrintToVideo"},
    {"scrn", "OlympusPreview"},
    {"thmb", "MakerNotePentax5a/OlympusThumbnail"},
};

extern const TagVocabulary userDataReferencetags[] = {
    {"CNCV", "Xmp.video.CompressorVersion"},
    {"CNFV", "Xmp.video.FirmwareVersion"},
    {"CNMN", "Xmp.video.Model"},
    {"NCHD", "Xmp.video.MakerNoteType"},
    {"WLOC", "Xmp.video.WindowLocation"},
    {"SDLN", "Xmp.video.PlayMode"},
    {"FFMV", "Xmp.video.StreamName"},
    {"SelO", "Xmp.video.PlaySelection"},
    {"name", "Xmp.video.Name"},
    {"vndr", "Xmp.video.Vendor"},
    {" ART", "Xmp.video.Artist"},
    {" alb", "Xmp.video.Album"},
    {" arg", "Xmp.video.Arranger"},
    {" ark", "Xmp.video.ArrangerKeywords"},
    {" cmt", "Xmp.video.Comment"},
    {" cok", "Xmp.video.ComposerKeywords"},
    {" com", "Xmp.video.Composer"},
    {" cpy", "Xmp.video.Copyright"},
    {" day", "Xmp.video.CreateDate"},
    {" dir", "Xmp.video.Director"},
    {" ed1", "Xmp.video.Edit1"},
    {" ed2", "Xmp.video.Edit2"},
    {" ed3", "Xmp.video.Edit3"},
    {" ed4", "Xmp.video.Edit4"},
    {" ed5", "Xmp.video.Edit5"},
    {" ed6", "Xmp.video.Edit6"},
    {" ed7", "Xmp.video.Edit7"},
    {" ed8", "Xmp.video.Edit8"},
    {" ed9", "Xmp.video.Edit9"},
    {" enc", "Xmp.video.Encoder"},
    {" fmt", "Xmp.video.Format"},
    {" gen", "Xmp.video.Genre"},
    {" grp", "Xmp.video.Grouping"},
    {" inf", "Xmp.video.Information"},
    {" isr", "Xmp.video.ISRCCode"},
    {" lab", "Xmp.video.RecordLabelName"},
    {" lal", "Xmp.video.RecordLabelURL"},
    {" lyr", "Xmp.video.Lyrics"},
    {" mak", "Xmp.video.Make"},
    {" mal", "Xmp.video.MakerURL"},
    {" mod", "Xmp.video.Model"},
    {" nam", "Xmp.video.Title"},
    {" pdk", "Xmp.video.ProducerKeywords"},
    {" phg", "Xmp.video.RecordingCopyright"},
    {" prd", "Xmp.video.Producer"},
    {" prf", "Xmp.video.Performers"},
    {" prk", "Xmp.video.PerformerKeywords"},
    {" prl", "Xmp.video.PerformerURL"},
    {" req", "Xmp.video.Requirements"},
    {" snk", "Xmp.video.SubtitleKeywords"},
    {" snm", "Xmp.video.Subtitle"},
    {" src", "Xmp.video.SourceCredits"},
    {" swf", "Xmp.video.SongWriter"},
    {" swk", "Xmp.video.SongWriterKeywords"},
    {" swr", "Xmp.video.SoftwareVersion"},
    {" too", "Xmp.video.Encoder"},
    {" trk", "Xmp.video.Track"},
    {" wrt", "Xmp.video.Composer"},
    {" xyz", "Xmp.video.GPSCoordinates"},
    {"CMbo", "Xmp.video.CameraByteOrder"},
    {"Cmbo", "Xmp.video.CameraByteOrder"},
};

extern const TagDetails NikonNCTGTags[] = {
    {0x0001, "Xmp.video.Make"},
    {0x0002, "Xmp.video.Model"},
    {0x0003, "Xmp.video.Software"},
    {0x0011, "Xmp.video.CreationDate"},
    {0x0012, "Xmp.video.DateTimeOriginal"},
    {0x0013, "Xmp.video.FrameCount"},
    {0x0016, "Xmp.video.FrameRate"},
    {0x0022, "Xmp.video.FrameWidth"},
    {0x0023, "Xmp.video.FrameHeight"},
    {0x0032, "Xmp.audio.channelType"},
    {0x0033, "Xmp.audio.BitsPerSample"},
    {0x0034, "Xmp.audio.sampleRate"},
    {0x1108822, "Xmp.video.ExposureProgram"},
    {0x1109204, "Xmp.video.ExposureCompensation"},
    {0x1109207, "Xmp.video.MeteringMode"},
    {0x110a434, "Xmp.video.LensModel"},
    {0x1200000, "Xmp.video.GPSVersionID"},
    {0x1200001, "Xmp.video.GPSLatitudeRef"},
    {0x1200002, "Xmp.video.GPSLatitude"},
    {0x1200003, "Xmp.video.GPSLongitudeRef"},
    {0x1200004, "Xmp.video.GPSLongitude"},
    {0x1200005, "Xmp.video.GPSAltitudeRef"},
    {0x1200006, "Xmp.video.GPSAltitude"},
    {0x1200007, "Xmp.video.GPSTimeStamp"},
    {0x1200008, "Xmp.video.GPSSatellites"},
    {0x1200010, "Xmp.video.GPSImgDirectionRef"},
    {0x1200011, "Xmp.video.GPSImgDirection"},
    {0x1200012, "Xmp.video.GPSMapDatum"},
    {0x120001d, "Xmp.video.GPSDateStamp"},
    {0x2000001, "Xmp.video.MakerNoteVersion"},
    {0x2000005, "Xmp.video.WhiteBalance"},
    {0x200000b, "Xmp.video.WhiteBalanceFineTune"},
    {0x200001e, "Xmp.video.ColorSpace"},
    {0x2000023, "Xmp.video.PictureControlData"},
    {0x2000024, "Xmp.video.WorldTime"},
    {0x200002c, "Xmp.video.UnknownInfo"},
    {0x2000032, "Xmp.video.UnknownInfo2"},
    {0x2000039, "Xmp.video.LocationInfo"},
    {0x2000083, "Xmp.video.LensType"},
    {0x2000084, "Xmp.video.LensModel"},
    {0x20000ab, "Xmp.video.VariProgram"},
};

extern const TagDetails NikonColorSpace[] = {
    {1, "sRGB"},
    {2, "Adobe RGB"},
};

extern const TagVocabulary NikonGPS_Latitude_Longitude_ImgDirection_Reference[] = {
    {"N", "North"}, {"S", "South"}, {"E", "East"}, {"W", "West"}, {"M", "Magnetic North"}, {"T", "True North"},
};

extern const TagDetails NikonGPSAltitudeRef[] = {
    {0, "Above Sea Level"},
    {1, "Below Sea Level"},
};

extern const TagDetails NikonExposureProgram[] = {
    {0, "Not Defined"},
    {1, "Manual"},
    {2, "Program AE"},
    {3, "Aperture-priority AE"},
    {4, "Shutter speed priority AE"},
    {5, "Creative (Slow speed)"},
    {6, "Action (High speed)"},
    {7, "Portrait"},
    {8, "Landscape"},
};

extern const TagDetails NikonMeteringMode[] = {
    {0, "Unknown"}, {1, "Average"},    {2, "Center-weighted average"},
    {3, "Spot"},    {4, "Multi-spot"}, {5, "Multi-segment"},
    {6, "Partial"}, {255, "Other"},
};

extern const TagDetails PictureControlAdjust[] = {
    {0, "Default Settings"},
    {1, "Quick Adjust"},
    {2, "Full Control"},
};

//! Contrast and Sharpness
extern const TagDetails NormalSoftHard[] = {{0, "Normal"}, {1, "Soft"}, {2, "Hard"}};

//! Saturation
extern const TagDetails Saturation[] = {{0, "Normal"}, {1, "Low"}, {2, "High"}};

//! YesNo, used for DaylightSavings
extern const TagDetails YesNo[] = {{0, "No"}, {1, "Yes"}};

//! DateDisplayFormat
extern const TagDetails DateDisplayFormat[] = {{0, "Y/M/D"}, {1, "M/D/Y"}, {2, "D/M/Y"}};

extern const TagDetails FilterEffect[] = {
    {0x80, "Off"}, {0x81, "Yellow"}, {0x82, "Orange"}, {0x83, "Red"}, {0x84, "Green"}, {0xff, "n/a"},
};

extern const TagDetails ToningEffect[] = {
    {0x80, "B&W"},         {0x81, "Sepia"},      {0x82, "Cyanotype"},  {0x83, "Red"},
    {0x84, "Yellow"},      {0x85, "Green"},      {0x86, "Blue-green"}, {0x87, "Blue"},
    {0x88, "Purple-blue"}, {0x89, "Red-purple"}, {0xff, "n/a"},
};

extern const TagDetails whiteBalance[] = {
    {0, "Auto"}, {1, "Daylight"}, {2, "Shade"}, {3, "Fluorescent"}, {4, "Tungsten"}, {5, "Manual"},
};

enum movieHeaderTags {
  MovieHeaderVersion,
  CreateDate,
  ModifyDate,
  TimeScale,
  Duration,
  PreferredRate,
  PreferredVolume,
  PreviewTime = 18,
  PreviewDuration,
  PosterTime,
  SelectionTime,
  SelectionDuration,
  CurrentTime,
  NextTrackID
};
enum trackHeaderTags {
  TrackHeaderVersion,
  TrackCreateDate,
  TrackModifyDate,
  TrackID,
  TrackDuration = 5,
  TrackLayer = 8,
  TrackVolume,
  ImageWidth = 19,
  ImageHeight
};
enum mediaHeaderTags {
  MediaHeaderVersion,
  MediaCreateDate,
  MediaModifyDate,
  MediaTimeScale,
  MediaDuration,
  MediaLanguageCode
};
enum handlerTags { HandlerClass = 1, HandlerType, HandlerVendorID };
enum videoHeaderTags { GraphicsMode = 2, OpColor };
enum stream { Video, Audio, Hint, Null, GenMediaHeader };
enum imageDescTags {
  codec,
  VendorID = 4,
  SourceImageWidth_Height = 7,
  XResolution,
  YResolution,
  CompressorName = 10,
  BitDepth
};
enum audioDescTags { AudioFormat, AudioVendorID = 4, AudioChannels, AudioSampleRate = 7, MOV_AudioFormat = 13 };

/*!
  @brief Function used to check equality of a Tags with a
      particular string (ignores case while comparing).
  @param buf Data buffer that will contain Tag to compare
  @param str char* Pointer to string
  @return Returns true if the buffer value is equal to string.
 */
bool equalsQTimeTag(Exiv2::DataBuf& buf, const char* str) {
  for (int i = 0; i < 4; ++i)
    if (tolower(buf.data()[i]) != tolower(str[i]))
      return false;
  return true;
}

/*!
  @brief Function used to ignore Tags and values stored in them,
      since they are not necessary as metadata information
  @param buf Data buffer that will contain Tag to compare
  @return Returns true, if Tag is found in the ignoreList[]
 */
bool ignoreList(Exiv2::DataBuf& buf) {
  const char ignoreList[13][5] = {
      "mdat", "edts", "junk", "iods", "alis", "stsc", "stsz", "stco", "ctts", "stss", "skip", "wide", "cmvd",
  };

  for (int i = 0; i < 13; ++i)
    if (equalsQTimeTag(buf, ignoreList[i]))
      return true;

  return false;
}

/*!
  @brief Function used to ignore Tags, basically Tags which
      contain other tags inside them, since they are not necessary
      as metadata information
  @param buf Data buffer that will contain Tag to compare
  @return Returns true, if Tag is found in the ignoreList[]
 */
bool dataIgnoreList(Exiv2::DataBuf& buf) {
  const char ignoreList[8][5] = {
      "moov", "mdia", "minf", "dinf", "alis", "stbl", "cmov", "meta",
  };

  for (int i = 0; i < 8; ++i)
    if (equalsQTimeTag(buf, ignoreList[i]))
      return true;

  return false;
}
}  // namespace Internal
}  // namespace Exiv2

namespace Exiv2 {

using namespace Exiv2::Internal;

QuickTimeVideo::QuickTimeVideo(BasicIo::UniquePtr io) : Image(ImageType::qtime, mdNone, std::move(io)), timeScale_(1) {
}  // QuickTimeVideo::QuickTimeVideo

std::string QuickTimeVideo::mimeType() const {
  return "video/quicktime";
}

void QuickTimeVideo::writeMetadata() {
}

void QuickTimeVideo::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());

  // Ensure that this is the correct image type
  if (!isQTimeType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "QuickTime");
  }

  IoCloser closer(*io_);
  clearMetadata();
  continueTraversing_ = true;
  height_ = width_ = 1;

  xmpData_["Xmp.video.FileSize"] = (double)io_->size() / (double)1048576;
  xmpData_["Xmp.video.MimeType"] = mimeType();

  while (continueTraversing_)
    decodeBlock();

  aspectRatio();
}  // QuickTimeVideo::readMetadata

void QuickTimeVideo::decodeBlock(std::string const& entered_from) {
  const long bufMinSize = 4;
  DataBuf buf(bufMinSize + 1);
  uint64_t size = 0;
  buf.data()[4] = '\0';

  io_->read(buf.data(), 4);
  if (io_->eof()) {
    continueTraversing_ = false;
    return;
  }

  size = buf.read_uint32(0, bigEndian);

  io_->readOrThrow(buf.data(), 4);

  // we have read 2x 4 bytes
  size_t hdrsize = 8;

  if (size == 1) {
    // The box size is encoded as a uint64_t, so we need to read another 8 bytes.
    DataBuf data(8);
    hdrsize += 8;
    io_->readOrThrow(data.data(), data.size());
    size = data.read_uint64(0, bigEndian);
  } else if (size == 0) {
    if (entered_from == "meta") {
      size = buf.read_uint32(0, bigEndian);
      io_->readOrThrow(buf.data(), 4, Exiv2::ErrorCode::kerCorruptedMetadata);
    }
  }

  enforce(size >= hdrsize, Exiv2::ErrorCode::kerCorruptedMetadata);
  enforce(size - hdrsize <= io_->size() - io_->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  enforce(size - hdrsize <= std::numeric_limits<size_t>::max(), Exiv2::ErrorCode::kerCorruptedMetadata);

  // std::cerr<<"Tag=>"<<buf.data()<<"     size=>"<<size-hdrsize << std::endl;
  const size_t newsize = static_cast<size_t>(size - hdrsize);
  if (newsize > buf.size()) {
    buf.resize(newsize);
  }
  tagDecoder(buf, newsize);
}  // QuickTimeVideo::decodeBlock

void QuickTimeVideo::tagDecoder(Exiv2::DataBuf& buf, size_t size) {
  assert(buf.size() > 4);

  if (ignoreList(buf))
    discard(size);

  else if (dataIgnoreList(buf)) {
    decodeBlock(Exiv2::toString(buf.data()));
  } else if (equalsQTimeTag(buf, "ftyp"))
    fileTypeDecoder(size);

  else if (equalsQTimeTag(buf, "trak"))
    setMediaStream();

  else if (equalsQTimeTag(buf, "mvhd"))
    movieHeaderDecoder(size);

  else if (equalsQTimeTag(buf, "tkhd"))
    trackHeaderDecoder(size);

  else if (equalsQTimeTag(buf, "mdhd"))
    mediaHeaderDecoder(size);

  else if (equalsQTimeTag(buf, "hdlr"))
    handlerDecoder(size);

  else if (equalsQTimeTag(buf, "vmhd"))
    videoHeaderDecoder(size);

  else if (equalsQTimeTag(buf, "udta"))
    userDataDecoder(size);

  else if (equalsQTimeTag(buf, "dref"))
    multipleEntriesDecoder();

  else if (equalsQTimeTag(buf, "stsd"))
    sampleDesc(size);

  else if (equalsQTimeTag(buf, "stts"))
    timeToSampleDecoder();

  else if (equalsQTimeTag(buf, "pnot"))
    previewTagDecoder(size);

  else if (equalsQTimeTag(buf, "tapt"))
    trackApertureTagDecoder(size);

  else if (equalsQTimeTag(buf, "keys"))
    keysTagDecoder(size);

  else if (equalsQTimeTag(buf, "url ")) {
    Exiv2::DataBuf url(size + 1);
    io_->readOrThrow(url.data(), size);
    url.write_uint8(size, 0);
    if (currentStream_ == Video)
      xmpData_["Xmp.video.URL"] = Exiv2::toString(url.data());
    else if (currentStream_ == Audio)
      xmpData_["Xmp.audio.URL"] = Exiv2::toString(url.data());
  }

  else if (equalsQTimeTag(buf, "urn ")) {
    Exiv2::DataBuf urn(size + 1);
    io_->readOrThrow(urn.data(), size);
    urn.write_uint8(size, 0);
    if (currentStream_ == Video)
      xmpData_["Xmp.video.URN"] = Exiv2::toString(urn.data());
    else if (currentStream_ == Audio)
      xmpData_["Xmp.audio.URN"] = Exiv2::toString(urn.data());
  }

  else if (equalsQTimeTag(buf, "dcom")) {
    Exiv2::DataBuf dcom(size + 1);
    io_->readOrThrow(dcom.data(), size);
    dcom.write_uint8(size, 0);
    xmpData_["Xmp.video.Compressor"] = Exiv2::toString(dcom.data());
  }

  else if (equalsQTimeTag(buf, "smhd")) {
    io_->readOrThrow(buf.data(), 4);
    io_->readOrThrow(buf.data(), 4);
    xmpData_["Xmp.audio.Balance"] = buf.read_uint16(0, bigEndian);
  }

  else {
    discard(size);
  }
}  // QuickTimeVideo::tagDecoder

void QuickTimeVideo::discard(size_t size) {
  size_t cur_pos = io_->tell();
  io_->seek(cur_pos + size, BasicIo::beg);
}  // QuickTimeVideo::discard

void QuickTimeVideo::previewTagDecoder(size_t size) {
  DataBuf buf(4);
  size_t cur_pos = io_->tell();
  io_->readOrThrow(buf.data(), 4);
  xmpData_["Xmp.video.PreviewDate"] = buf.read_uint32(0, bigEndian);
  io_->readOrThrow(buf.data(), 2);
  xmpData_["Xmp.video.PreviewVersion"] = getShort(buf.data(), bigEndian);

  io_->readOrThrow(buf.data(), 4);
  if (equalsQTimeTag(buf, "PICT"))
    xmpData_["Xmp.video.PreviewAtomType"] = "QuickDraw Picture";
  else
    xmpData_["Xmp.video.PreviewAtomType"] = std::string{buf.c_str(), 4};

  io_->seek(cur_pos + size, BasicIo::beg);
}  // QuickTimeVideo::previewTagDecoder

void QuickTimeVideo::keysTagDecoder(size_t size) {
  DataBuf buf(4);
  size_t cur_pos = io_->tell();
  io_->readOrThrow(buf.data(), 4);
  xmpData_["Xmp.video.PreviewDate"] = buf.read_uint32(0, bigEndian);
  io_->readOrThrow(buf.data(), 2);
  xmpData_["Xmp.video.PreviewVersion"] = getShort(buf.data(), bigEndian);

  io_->readOrThrow(buf.data(), 4);
  if (equalsQTimeTag(buf, "PICT"))
    xmpData_["Xmp.video.PreviewAtomType"] = "QuickDraw Picture";
  else
    xmpData_["Xmp.video.PreviewAtomType"] = std::string{buf.c_str(), 4};

  io_->seek(cur_pos + size, BasicIo::beg);
}  // QuickTimeVideo::keysTagDecoder

void QuickTimeVideo::trackApertureTagDecoder(size_t size) {
  DataBuf buf(4), buf2(2);
  size_t cur_pos = io_->tell();
  byte n = 3;

  while (n--) {
    io_->seek(static_cast<long>(4), BasicIo::cur);
    io_->readOrThrow(buf.data(), 4);

    if (equalsQTimeTag(buf, "clef")) {
      io_->seek(static_cast<long>(4), BasicIo::cur);
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.CleanApertureWidth"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.CleanApertureHeight"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
    }

    else if (equalsQTimeTag(buf, "prof")) {
      io_->seek(static_cast<long>(4), BasicIo::cur);
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.ProductionApertureWidth"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.ProductionApertureHeight"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
    }

    else if (equalsQTimeTag(buf, "enof")) {
      io_->seek(static_cast<long>(4), BasicIo::cur);
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.EncodedPixelsWidth"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      xmpData_["Xmp.video.EncodedPixelsHeight"] =
          Exiv2::toString(buf.read_uint16(0, bigEndian)) + "." + Exiv2::toString(buf2.read_uint16(0, bigEndian));
    }
  }
  io_->seek(static_cast<long>(cur_pos + size), BasicIo::beg);
}  // QuickTimeVideo::trackApertureTagDecoder

void QuickTimeVideo::CameraTagsDecoder(size_t size_external) {
  size_t cur_pos = io_->tell();
  DataBuf buf(50), buf2(4);
  const TagDetails* td;

  io_->readOrThrow(buf.data(), 4);
  if (equalsQTimeTag(buf, "NIKO")) {
    io_->seek(cur_pos, BasicIo::beg);

    io_->readOrThrow(buf.data(), 24);
    xmpData_["Xmp.video.Make"] = Exiv2::toString(buf.data());
    io_->readOrThrow(buf.data(), 14);
    xmpData_["Xmp.video.Model"] = Exiv2::toString(buf.data());
    io_->readOrThrow(buf.data(), 4);
    xmpData_["Xmp.video.ExposureTime"] = "1/" + Exiv2::toString(ceil(buf.read_uint32(0, littleEndian) / (double)10));
    io_->readOrThrow(buf.data(), 4);
    io_->readOrThrow(buf2.data(), 4);
    xmpData_["Xmp.video.FNumber"] = buf.read_uint32(0, littleEndian) / (double)buf2.read_uint32(0, littleEndian);
    io_->readOrThrow(buf.data(), 4);
    io_->readOrThrow(buf2.data(), 4);
    xmpData_["Xmp.video.ExposureCompensation"] =
        buf.read_uint32(0, littleEndian) / (double)buf2.read_uint32(0, littleEndian);
    io_->readOrThrow(buf.data(), 10);
    io_->readOrThrow(buf.data(), 4);
    td = find(whiteBalance, buf.read_uint32(0, littleEndian));
    if (td)
      xmpData_["Xmp.video.WhiteBalance"] = exvGettext(td->label_);
    io_->readOrThrow(buf.data(), 4);
    io_->readOrThrow(buf2.data(), 4);
    xmpData_["Xmp.video.FocalLength"] = buf.read_uint32(0, littleEndian) / (double)buf2.read_uint32(0, littleEndian);
    io_->seek(static_cast<long>(95), BasicIo::cur);
    io_->readOrThrow(buf.data(), 48);
    xmpData_["Xmp.video.Software"] = Exiv2::toString(buf.data());
    io_->readOrThrow(buf.data(), 4);
    xmpData_["Xmp.video.ISO"] = buf.read_uint32(0, littleEndian);
  }

  io_->seek(cur_pos + size_external, BasicIo::beg);
}  // QuickTimeVideo::CameraTagsDecoder

void QuickTimeVideo::userDataDecoder(size_t size_external) {
  size_t cur_pos = io_->tell();
  const TagVocabulary* td;
  const TagVocabulary *tv, *tv_internal;

  const long bufMinSize = 100;
  DataBuf buf(bufMinSize);
  size_t size_internal = size_external;
  std::memset(buf.data(), 0x0, buf.size());

  while ((size_internal / 4 != 0) && (size_internal > 0)) {
    buf.data()[4] = '\0';
    io_->readOrThrow(buf.data(), 4);
    const size_t size = buf.read_uint32(0, bigEndian);
    if (size > size_internal)
      break;
    size_internal -= size;
    io_->readOrThrow(buf.data(), 4);

    if (buf.data()[0] == 169)
      buf.data()[0] = ' ';
    td = find(userDatatags, Exiv2::toString(buf.data()));

    tv = find(userDataReferencetags, Exiv2::toString(buf.data()));

    if (size <= 12)
      break;

    else if (equalsQTimeTag(buf, "DcMD") || equalsQTimeTag(buf, "NCDT"))
      userDataDecoder(size - 8);

    else if (equalsQTimeTag(buf, "NCTG"))
      NikonTagsDecoder(size - 8);

    else if (equalsQTimeTag(buf, "TAGS"))
      CameraTagsDecoder(size - 8);

    else if (equalsQTimeTag(buf, "CNCV") || equalsQTimeTag(buf, "CNFV") || equalsQTimeTag(buf, "CNMN") ||
             equalsQTimeTag(buf, "NCHD") || equalsQTimeTag(buf, "FFMV")) {
      io_->readOrThrow(buf.data(), size - 8);
      xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.data());
    }

    else if (equalsQTimeTag(buf, "CMbo") || equalsQTimeTag(buf, "Cmbo")) {
      io_->readOrThrow(buf.data(), 2);
      buf.data()[2] = '\0';
      tv_internal = find(cameraByteOrderTags, Exiv2::toString(buf.data()));

      if (tv_internal)
        xmpData_[exvGettext(tv->label_)] = exvGettext(tv_internal->label_);
      else
        xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.data());
    }

    else if (tv) {
      const size_t tv_size = size - 12;
      if (tv_size > buf.size()) {
        enforce(tv_size <= io_->size() - io_->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
        buf.resize(tv_size);
      }
      io_->readOrThrow(buf.data(), 4);
      io_->readOrThrow(buf.data(), tv_size);
      xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.data());
    }

    else if (td)
      tagDecoder(buf, size - 8);
  }

  io_->seek(cur_pos + size_external, BasicIo::beg);
}  // QuickTimeVideo::userDataDecoder

void QuickTimeVideo::NikonTagsDecoder(size_t size_external) {
  size_t cur_pos = io_->tell();
  DataBuf buf(200), buf2(4 + 1);
  unsigned long TagID = 0;
  unsigned short dataLength = 0, dataType = 2;
  const TagDetails *td, *td2;

  for (int i = 0; i < 100; i++) {
    io_->readOrThrow(buf.data(), 4);
    TagID = buf.read_uint32(0, bigEndian);
    td = find(NikonNCTGTags, TagID);

    io_->readOrThrow(buf.data(), 2);
    dataType = buf.read_uint16(0, bigEndian);

    std::memset(buf.data(), 0x0, buf.size());
    io_->readOrThrow(buf.data(), 2);

    if (TagID == 0x2000023) {
      size_t local_pos = io_->tell();
      dataLength = buf.read_uint16(0, bigEndian);
      std::memset(buf.data(), 0x0, buf.size());

      io_->readOrThrow(buf.data(), 4);
      xmpData_["Xmp.video.PictureControlVersion"] = Exiv2::toString(buf.data());
      io_->readOrThrow(buf.data(), 20);
      xmpData_["Xmp.video.PictureControlName"] = Exiv2::toString(buf.data());
      io_->readOrThrow(buf.data(), 20);
      xmpData_["Xmp.video.PictureControlBase"] = Exiv2::toString(buf.data());
      io_->readOrThrow(buf.data(), 4);
      std::memset(buf.data(), 0x0, buf.size());

      io_->readOrThrow(buf.data(), 1);
      td2 = find(PictureControlAdjust, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.PictureControlAdjust"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.PictureControlAdjust"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      td2 = find(NormalSoftHard, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.PictureControlQuickAdjust"] = exvGettext(td2->label_);

      io_->readOrThrow(buf.data(), 1);
      td2 = find(NormalSoftHard, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.Sharpness"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.Sharpness"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      td2 = find(NormalSoftHard, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.Contrast"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.Contrast"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      td2 = find(NormalSoftHard, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.Brightness"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.Brightness"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      td2 = find(Saturation, (int)buf.data()[0] & 7);
      if (td2)
        xmpData_["Xmp.video.Saturation"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.Saturation"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      xmpData_["Xmp.video.HueAdjustment"] = (int)buf.data()[0] & 7;

      io_->readOrThrow(buf.data(), 1);
      td2 = find(FilterEffect, (int)buf.data()[0]);
      if (td2)
        xmpData_["Xmp.video.FilterEffect"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.FilterEffect"] = (int)buf.data()[0];

      io_->readOrThrow(buf.data(), 1);
      td2 = find(ToningEffect, (int)buf.data()[0]);
      if (td2)
        xmpData_["Xmp.video.ToningEffect"] = exvGettext(td2->label_);
      else
        xmpData_["Xmp.video.ToningEffect"] = (int)buf.data()[0];

      io_->readOrThrow(buf.data(), 1);
      xmpData_["Xmp.video.ToningSaturation"] = (int)buf.data()[0];

      io_->seek(local_pos + dataLength, BasicIo::beg);
    }

    else if (TagID == 0x2000024) {
      size_t local_pos = io_->tell();
      dataLength = buf.read_uint16(0, bigEndian);
      std::memset(buf.data(), 0x0, buf.size());

      io_->readOrThrow(buf.data(), 2);
      xmpData_["Xmp.video.TimeZone"] = Exiv2::getShort(buf.data(), bigEndian);
      io_->readOrThrow(buf.data(), 1);
      td2 = find(YesNo, (int)buf.data()[0]);
      if (td2)
        xmpData_["Xmp.video.DayLightSavings"] = exvGettext(td2->label_);

      io_->readOrThrow(buf.data(), 1);
      td2 = find(DateDisplayFormat, (int)buf.data()[0]);
      if (td2)
        xmpData_["Xmp.video.DateDisplayFormat"] = exvGettext(td2->label_);

      io_->seek(local_pos + dataLength, BasicIo::beg);
    }

    else if (dataType == 2 || dataType == 7) {
      dataLength = buf.read_uint16(0, bigEndian);
      std::memset(buf.data(), 0x0, buf.size());

      // Sanity check with an "unreasonably" large number
      if (dataLength > 200) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be larger than 200."
                  << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + dataLength, BasicIo::beg);
      } else {
        io_->readOrThrow(buf.data(), dataLength);
      }

      if (td) {
        xmpData_[exvGettext(td->label_)] = Exiv2::toString(buf.data());
      }
    } else if (dataType == 4) {
      dataLength = buf.read_uint16(0, bigEndian) * 4;
      std::memset(buf.data(), 0x0, buf.size());
      io_->readOrThrow(buf.data(), 4);
      if (td)
        xmpData_[exvGettext(td->label_)] = Exiv2::toString(buf.read_uint32(0, bigEndian));

      // Sanity check with an "unreasonably" large number
      if (dataLength > 200 || dataLength < 4) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                  << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + dataLength - 4, BasicIo::beg);
      } else
        io_->readOrThrow(buf.data(), dataLength - 4);
    } else if (dataType == 3) {
      dataLength = buf.read_uint16(0, bigEndian) * 2;
      std::memset(buf.data(), 0x0, buf.size());
      io_->readOrThrow(buf.data(), 2);
      if (td)
        xmpData_[exvGettext(td->label_)] = Exiv2::toString(buf.read_uint16(0, bigEndian));

      // Sanity check with an "unreasonably" large number
      if (dataLength > 200 || dataLength < 2) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                  << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + dataLength - 2, BasicIo::beg);
      } else
        io_->readOrThrow(buf.data(), dataLength - 2);
    } else if (dataType == 5) {
      dataLength = buf.read_uint16(0, bigEndian) * 8;
      std::memset(buf.data(), 0x0, buf.size());
      io_->readOrThrow(buf.data(), 4);
      io_->readOrThrow(buf2.data(), 4);
      if (td)
        xmpData_[exvGettext(td->label_)] =
            Exiv2::toString((double)buf.read_uint32(0, bigEndian) / (double)buf2.read_uint32(0, bigEndian));

      // Sanity check with an "unreasonably" large number
      if (dataLength > 200 || dataLength < 8) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                  << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + dataLength - 8, BasicIo::beg);
      } else
        io_->readOrThrow(buf.data(), dataLength - 8);
    } else if (dataType == 8) {
      dataLength = buf.read_uint16(0, bigEndian) * 2;
      std::memset(buf.data(), 0x0, buf.size());
      io_->readOrThrow(buf.data(), 2);
      io_->readOrThrow(buf2.data(), 2);
      if (td)
        xmpData_[exvGettext(td->label_)] =
            Exiv2::toString(buf.read_uint16(0, bigEndian)) + " " + Exiv2::toString(buf2.read_uint16(0, bigEndian));

      // Sanity check with an "unreasonably" large number
      if (dataLength > 200 || dataLength < 4) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Xmp.video Nikon Tags, dataLength was found to be of inapropriate size."
                  << " Entries considered invalid. Not Processed.\n";
#endif
        io_->seek(io_->tell() + dataLength - 4, BasicIo::beg);
      } else
        io_->readOrThrow(buf.data(), dataLength - 4);
    }
  }

  io_->seek(cur_pos + size_external, BasicIo::beg);
}  // QuickTimeVideo::NikonTagsDecoder

void QuickTimeVideo::setMediaStream() {
  size_t current_position = io_->tell();
  DataBuf buf(4 + 1);

  while (!io_->eof()) {
    io_->readOrThrow(buf.data(), 4);
    if (equalsQTimeTag(buf, "hdlr")) {
      io_->readOrThrow(buf.data(), 4);
      io_->readOrThrow(buf.data(), 4);
      io_->readOrThrow(buf.data(), 4);

      if (equalsQTimeTag(buf, "vide"))
        currentStream_ = Video;
      else if (equalsQTimeTag(buf, "soun"))
        currentStream_ = Audio;
      else if (equalsQTimeTag(buf, "hint"))
        currentStream_ = Hint;
      else
        currentStream_ = GenMediaHeader;
      break;
    }
  }

  io_->seek(current_position, BasicIo::beg);
}  // QuickTimeVideo::setMediaStream

void QuickTimeVideo::timeToSampleDecoder() {
  DataBuf buf(4 + 1);
  io_->readOrThrow(buf.data(), 4);
  io_->readOrThrow(buf.data(), 4);
  size_t noOfEntries, totalframes = 0, timeOfFrames = 0;
  noOfEntries = buf.read_uint32(0, bigEndian);
  size_t temp;

  for (unsigned long i = 1; i <= noOfEntries; i++) {
    io_->readOrThrow(buf.data(), 4);
    temp = buf.read_uint32(0, bigEndian);
    totalframes += temp;
    io_->readOrThrow(buf.data(), 4);
    timeOfFrames += temp * buf.read_uint32(0, bigEndian);
  }
  if (currentStream_ == Video)
    xmpData_["Xmp.video.FrameRate"] = (double)totalframes * (double)timeScale_ / (double)timeOfFrames;
}  // QuickTimeVideo::timeToSampleDecoder

void QuickTimeVideo::sampleDesc(size_t size) {
  DataBuf buf(100);
  size_t cur_pos = io_->tell();
  io_->readOrThrow(buf.data(), 4);
  io_->readOrThrow(buf.data(), 4);
  size_t noOfEntries;
  noOfEntries = buf.read_uint32(0, bigEndian);

  for (unsigned long i = 1; i <= noOfEntries; i++) {
    if (currentStream_ == Video)
      imageDescDecoder();
    else if (currentStream_ == Audio)
      audioDescDecoder();
  }
  io_->seek(cur_pos + size, BasicIo::beg);
}  // QuickTimeVideo::sampleDesc

void QuickTimeVideo::audioDescDecoder() {
  DataBuf buf(40);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';
  io_->readOrThrow(buf.data(), 4);
  size_t size = 82;

  const TagVocabulary* td;

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);
    switch (i) {
      case AudioFormat:
        td = find(qTimeFileType, Exiv2::toString(buf.data()));
        if (td)
          xmpData_["Xmp.audio.Compressor"] = exvGettext(td->label_);
        else
          xmpData_["Xmp.audio.Compressor"] = Exiv2::toString(buf.data());
        break;
      case AudioVendorID:
        td = find(vendorIDTags, Exiv2::toString(buf.data()));
        if (td)
          xmpData_["Xmp.audio.VendorID"] = exvGettext(td->label_);
        break;
      case AudioChannels:
        xmpData_["Xmp.audio.ChannelType"] = buf.read_uint16(0, bigEndian);
        xmpData_["Xmp.audio.BitsPerSample"] = (buf.data()[2] * 256 + buf.data()[3]);
        break;
      case AudioSampleRate:
        xmpData_["Xmp.audio.SampleRate"] =
            buf.read_uint16(0, bigEndian) + ((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
        break;
      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), static_cast<long>(size % 4));  // cause size is so small, this cast should be right.
}  // QuickTimeVideo::audioDescDecoder

void QuickTimeVideo::imageDescDecoder() {
  DataBuf buf(40);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';
  io_->readOrThrow(buf.data(), 4);
  size_t size = 82;

  const TagVocabulary* td;

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);

    switch (i) {
      case codec:
        td = find(qTimeFileType, Exiv2::toString(buf.data()));
        if (td)
          xmpData_["Xmp.video.Codec"] = exvGettext(td->label_);
        else
          xmpData_["Xmp.video.Codec"] = Exiv2::toString(buf.data());
        break;
      case VendorID:
        td = find(vendorIDTags, Exiv2::toString(buf.data()));
        if (td)
          xmpData_["Xmp.video.VendorID"] = exvGettext(td->label_);
        break;
      case SourceImageWidth_Height:
        xmpData_["Xmp.video.SourceImageWidth"] = buf.read_uint16(0, bigEndian);
        xmpData_["Xmp.video.SourceImageHeight"] = (buf.data()[2] * 256 + buf.data()[3]);
        break;
      case XResolution:
        xmpData_["Xmp.video.XResolution"] =
            buf.read_uint16(0, bigEndian) + ((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
        break;
      case YResolution:
        xmpData_["Xmp.video.YResolution"] =
            buf.read_uint16(0, bigEndian) + ((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
        io_->readOrThrow(buf.data(), 3);
        size -= 3;
        break;
      case CompressorName:
        io_->readOrThrow(buf.data(), 32);
        size -= 32;
        xmpData_["Xmp.video.Compressor"] = Exiv2::toString(buf.data());
        break;
      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), static_cast<long>(size % 4));
  xmpData_["Xmp.video.BitDepth"] = static_cast<int>(buf.read_uint8(0));
}  // QuickTimeVideo::imageDescDecoder

void QuickTimeVideo::multipleEntriesDecoder() {
  DataBuf buf(4 + 1);
  io_->readOrThrow(buf.data(), 4);
  io_->readOrThrow(buf.data(), 4);
  size_t noOfEntries;

  noOfEntries = buf.read_uint32(0, bigEndian);

  for (unsigned long i = 1; i <= noOfEntries; i++)
    decodeBlock();
}  // QuickTimeVideo::multipleEntriesDecoder

void QuickTimeVideo::videoHeaderDecoder(size_t size) {
  DataBuf buf(3);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[2] = '\0';
  currentStream_ = Video;

  const TagDetails* td;

  for (int i = 0; size / 2 != 0; size -= 2, i++) {
    io_->readOrThrow(buf.data(), 2);

    switch (i) {
      case GraphicsMode:
        td = find(graphicsModetags, buf.read_uint16(0, bigEndian));
        if (td)
          xmpData_["Xmp.video.GraphicsMode"] = exvGettext(td->label_);
        break;
      case OpColor:
        xmpData_["Xmp.video.OpColor"] = buf.read_uint16(0, bigEndian);
        break;
      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), size % 2);
}  // QuickTimeVideo::videoHeaderDecoder

void QuickTimeVideo::handlerDecoder(size_t size) {
  size_t cur_pos = io_->tell();
  DataBuf buf(100);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';

  const TagVocabulary* tv;

  for (int i = 0; i < 5; i++) {
    io_->readOrThrow(buf.data(), 4);

    switch (i) {
      case HandlerClass:
        tv = find(handlerClassTags, Exiv2::toString(buf.data()));
        if (tv) {
          if (currentStream_ == Video)
            xmpData_["Xmp.video.HandlerClass"] = exvGettext(tv->label_);
          else if (currentStream_ == Audio)
            xmpData_["Xmp.audio.HandlerClass"] = exvGettext(tv->label_);
        }
        break;
      case HandlerType:
        tv = find(handlerTypeTags, Exiv2::toString(buf.data()));
        if (tv) {
          if (currentStream_ == Video)
            xmpData_["Xmp.video.HandlerType"] = exvGettext(tv->label_);
          else if (currentStream_ == Audio)
            xmpData_["Xmp.audio.HandlerType"] = exvGettext(tv->label_);
        }
        break;
      case HandlerVendorID:
        tv = find(vendorIDTags, Exiv2::toString(buf.data()));
        if (tv) {
          if (currentStream_ == Video)
            xmpData_["Xmp.video.HandlerVendorID"] = exvGettext(tv->label_);
          else if (currentStream_ == Audio)
            xmpData_["Xmp.audio.HandlerVendorID"] = exvGettext(tv->label_);
        }
        break;
    }
  }
  io_->seek(cur_pos + size, BasicIo::beg);
}  // QuickTimeVideo::handlerDecoder

void QuickTimeVideo::fileTypeDecoder(size_t size) {
  DataBuf buf(5);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';
  Exiv2::Value::UniquePtr v = Exiv2::Value::create(Exiv2::xmpSeq);
  const TagVocabulary* td;

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);
    td = find(qTimeFileType, Exiv2::toString(buf.data()));

    switch (i) {
      case 0:
        if (td)
          xmpData_["Xmp.video.MajorBrand"] = exvGettext(td->label_);
        break;
      case 1:
        xmpData_["Xmp.video.MinorVersion"] = buf.read_uint32(0, bigEndian);
        break;
      default:
        if (td)
          v->read(exvGettext(td->label_));
        else
          v->read(Exiv2::toString(buf.data()));
        break;
    }
  }
  xmpData_.add(Exiv2::XmpKey("Xmp.video.CompatibleBrands"), v.get());
  io_->readOrThrow(buf.data(), size % 4);
}  // QuickTimeVideo::fileTypeDecoder

void QuickTimeVideo::mediaHeaderDecoder(size_t size) {
  DataBuf buf(5);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';
  int64_t time_scale = 1;

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);

    switch (i) {
      case MediaHeaderVersion:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaHeaderVersion"] = static_cast<int>(buf.read_uint8(0));
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaHeaderVersion"] = static_cast<int>(buf.read_uint8(0));
        break;
      case MediaCreateDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaCreateDate"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaCreateDate"] = buf.read_uint32(0, bigEndian);
        break;
      case MediaModifyDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaModifyDate"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaModifyDate"] = buf.read_uint32(0, bigEndian);
        break;
      case MediaTimeScale:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaTimeScale"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaTimeScale"] = buf.read_uint32(0, bigEndian);
        time_scale = buf.read_uint32(0, bigEndian);
        if (time_scale <= 0)
          time_scale = 1;
        break;
      case MediaDuration:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaDuration"] = time_scale ? buf.read_uint32(0, bigEndian) / time_scale : 0;
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaDuration"] = time_scale ? buf.read_uint32(0, bigEndian) / time_scale : 0;
        break;
      case MediaLanguageCode:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.MediaLangCode"] = buf.read_uint16(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.MediaLangCode"] = buf.read_uint16(0, bigEndian);
        break;

      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), size % 4);
}  // QuickTimeVideo::mediaHeaderDecoder

void QuickTimeVideo::trackHeaderDecoder(size_t size) {
  DataBuf buf(5);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';
  int64_t temp = 0;

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);

    switch (i) {
      case TrackHeaderVersion:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackHeaderVersion"] = static_cast<int>(buf.read_uint8(0));
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackHeaderVersion"] = static_cast<int>(buf.read_uint8(0));
        break;
      case TrackCreateDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackCreateDate"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackCreateDate"] = buf.read_uint32(0, bigEndian);
        break;
      case TrackModifyDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackModifyDate"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackModifyDate"] = buf.read_uint32(0, bigEndian);
        break;
      case TrackID:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackID"] = buf.read_uint32(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackID"] = buf.read_uint32(0, bigEndian);
        break;
      case TrackDuration:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackDuration"] = timeScale_ ? buf.read_uint32(0, bigEndian) / timeScale_ : 0;
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackDuration"] = timeScale_ ? buf.read_uint32(0, bigEndian) / timeScale_ : 0;
        break;
      case TrackLayer:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackLayer"] = buf.read_uint16(0, bigEndian);
        else if (currentStream_ == Audio)
          xmpData_["Xmp.audio.TrackLayer"] = buf.read_uint16(0, bigEndian);
        break;
      case TrackVolume:
        if (currentStream_ == Video)
          xmpData_["Xmp.video.TrackVolume"] = (static_cast<int>(buf.read_uint8(0)) + (buf.data()[2] * 0.1)) * 100;
        else if (currentStream_ == Audio)
          xmpData_["Xmp.video.TrackVolume"] = (static_cast<int>(buf.read_uint8(0)) + (buf.data()[2] * 0.1)) * 100;
        break;
      case ImageWidth:
        if (currentStream_ == Video) {
          temp = buf.read_uint16(0, bigEndian) + static_cast<int64_t>((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
          xmpData_["Xmp.video.Width"] = temp;
          width_ = temp;
        }
        break;
      case ImageHeight:
        if (currentStream_ == Video) {
          temp = buf.read_uint16(0, bigEndian) + static_cast<int64_t>((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
          xmpData_["Xmp.video.Height"] = temp;
          height_ = temp;
        }
        break;
      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), size % 4);
}  // QuickTimeVideo::trackHeaderDecoder

void QuickTimeVideo::movieHeaderDecoder(size_t size) {
  DataBuf buf(5);
  std::memset(buf.data(), 0x0, buf.size());
  buf.data()[4] = '\0';

  for (int i = 0; size / 4 != 0; size -= 4, i++) {
    io_->readOrThrow(buf.data(), 4);

    switch (i) {
      case MovieHeaderVersion:
        xmpData_["Xmp.video.MovieHeaderVersion"] = static_cast<int>(buf.read_uint8(0));
        break;
      case CreateDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        xmpData_["Xmp.video.DateUTC"] = buf.read_uint32(0, bigEndian);
        break;
      case ModifyDate:
        // A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
        xmpData_["Xmp.video.ModificationDate"] = buf.read_uint32(0, bigEndian);
        break;
      case TimeScale:
        xmpData_["Xmp.video.TimeScale"] = buf.read_uint32(0, bigEndian);
        timeScale_ = buf.read_uint32(0, bigEndian);
        if (timeScale_ <= 0)
          timeScale_ = 1;
        break;
      case Duration:
        if (timeScale_ != 0) {  // To prevent division by zero
          xmpData_["Xmp.video.Duration"] = buf.read_uint32(0, bigEndian) * 1000 / timeScale_;
        }
        break;
      case PreferredRate:
        xmpData_["Xmp.video.PreferredRate"] =
            buf.read_uint16(0, bigEndian) + ((buf.data()[2] * 256 + buf.data()[3]) * 0.01);
        break;
      case PreferredVolume:
        xmpData_["Xmp.video.PreferredVolume"] = (static_cast<int>(buf.read_uint8(0)) + (buf.data()[2] * 0.1)) * 100;
        break;
      case PreviewTime:
        xmpData_["Xmp.video.PreviewTime"] = buf.read_uint32(0, bigEndian);
        break;
      case PreviewDuration:
        xmpData_["Xmp.video.PreviewDuration"] = buf.read_uint32(0, bigEndian);
        break;
      case PosterTime:
        xmpData_["Xmp.video.PosterTime"] = buf.read_uint32(0, bigEndian);
        break;
      case SelectionTime:
        xmpData_["Xmp.video.SelectionTime"] = buf.read_uint32(0, bigEndian);
        break;
      case SelectionDuration:
        xmpData_["Xmp.video.SelectionDuration"] = buf.read_uint32(0, bigEndian);
        break;
      case CurrentTime:
        xmpData_["Xmp.video.CurrentTime"] = buf.read_uint32(0, bigEndian);
        break;
      case NextTrackID:
        xmpData_["Xmp.video.NextTrackID"] = buf.read_uint32(0, bigEndian);
        break;
      default:
        break;
    }
  }
  io_->readOrThrow(buf.data(), size % 4);
}  // QuickTimeVideo::movieHeaderDecoder

void QuickTimeVideo::aspectRatio() {
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
}  // QuickTimeVideo::aspectRatio

Image::UniquePtr newQTimeInstance(BasicIo::UniquePtr io, bool /*create*/) {
  Image::UniquePtr image(new QuickTimeVideo(std::move(io)));
  if (!image->good()) {
    image.reset();
  }
  return image;
}

bool isQTimeType(BasicIo& iIo, bool advance) {
  auto buf = DataBuf(12);
  iIo.read(buf.data(), 12);

  if (iIo.error() || iIo.eof()) {
    return false;
  }
  auto qTimeTags = std::array{"PICT", "free", "ftyp", "junk", "mdat", "moov", "pict", "pnot", "skip", "uuid", "wide"};

  bool matched = false;

  for (auto const& tag : qTimeTags) {
    auto tmp = buf.cmpBytes(4, tag, 4);
    if (tmp == 0) {
      // we only match if we actually know the video type. This is done
      // to avoid matching just on ftyp because bmffimage also has that
      // header.
      auto td = find(qTimeFileType, std::string{buf.c_str(8), 4});
      if (td) {
        matched = true;
      }
      break;
    }
  }

  if (!advance || !matched) {
    iIo.seek(static_cast<long>(0), BasicIo::beg);
  }

  return matched;
}

}  // namespace Exiv2
