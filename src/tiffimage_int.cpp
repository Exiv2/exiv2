// SPDX-License-Identifier: GPL-2.0-or-later

#include "tiffimage_int.hpp"
#include "error.hpp"
#include "i18n.h"  // NLS support.
#include "makernote_int.hpp"
#include "sonymn_int.hpp"
#include "tags_int.hpp"
#include "tiffvisitor_int.hpp"

#include <array>
#include <iostream>

// Shortcuts for the newTiffBinaryArray templates.
#define EXV_BINARY_ARRAY(arrayCfg, arrayDef) &newTiffBinaryArray0<arrayCfg, std::size(arrayDef), arrayDef>
#define EXV_SIMPLE_BINARY_ARRAY(arrayCfg) &newTiffBinaryArray1<arrayCfg>
#define EXV_COMPLEX_BINARY_ARRAY(arraySet, cfgSelFct) &newTiffBinaryArray2<std::size(arraySet), arraySet, cfgSelFct>

namespace Exiv2::Internal {
//! Constant for non-encrypted binary arrays
constexpr CryptFct notEncrypted = nullptr;

//! Canon Camera Settings binary array - configuration
constexpr ArrayCfg canonCsCfg = {
    IfdId::canonCsId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // With size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Canon Camera Settings binary array - definition
constexpr ArrayDef canonCsDef[] = {
    {46, ttUnsignedShort, 3}  // Exif.CanonCs.Lens
};

//! Canon Shot Info binary array - configuration
constexpr ArrayCfg canonSiCfg = {
    IfdId::canonSiId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // With size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Canon Panorama binary array - configuration
constexpr ArrayCfg canonPaCfg = {
    IfdId::canonPaId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Canon Custom Function binary array - configuration
constexpr ArrayCfg canonCfCfg = {
    IfdId::canonCfId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // With size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Canon Picture Info binary array - configuration
constexpr ArrayCfg canonPiCfg = {
    IfdId::canonPiId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Canon Time Info binary array - configuration
constexpr ArrayCfg canonTiCfg = {
    IfdId::canonTiId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttSignedLong,      // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // With size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon File Info binary array - configuration
constexpr ArrayCfg canonFiCfg = {
    IfdId::canonFiId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // Has a size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttSignedShort, 1},
};
//! Canon File Info binary array - definition
constexpr ArrayDef canonFiDef[] = {
    {2, ttUnsignedLong, 1},
};

//! Canon Processing Info binary array - configuration
constexpr ArrayCfg canonPrCfg = {
    IfdId::canonPrId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUnsignedShort,   // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // Has a size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttSignedShort, 1},
};

//! Canon canonAfMiAdj Info binary array - configuration
constexpr ArrayCfg canonAfMiAdjCfg = {
    IfdId::canonAfMiAdjId,  // Group for the elements
    invalidByteOrder,       // Use byte order from parent
    ttSignedLong,           // Type for array entry and size element
    notEncrypted,           // Not encrypted
    true,                   // Has a size element
    false,                  // No fillers
    false,                  // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon Vignetting Correction 2 Info binary array - configuration
constexpr ArrayCfg canonVigCor2Cfg = {
    IfdId::canonVigCor2Id,  // Group for the elements
    invalidByteOrder,       // Use byte order from parent
    ttSignedLong,           // Type for array entry and size element
    notEncrypted,           // Not encrypted
    true,                   // Has a size element
    false,                  // No fillers
    false,                  // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon Lighting Optimization Info binary array - configuration
constexpr ArrayCfg canonLiOpCfg = {
    IfdId::canonLiOpId,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttSignedLong,        // Type for array entry and size element
    notEncrypted,        // Not encrypted
    true,                // Has a size element
    false,               // No fillers
    false,               // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon LensInfo binary array - configuration
constexpr ArrayCfg canonLeCfg = {
    IfdId::canonLeId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry and size element
    notEncrypted,      // Not encrypted
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Canon LensInfo binary array - definition
constexpr ArrayDef canonLeDef[] = {
    {0, ttUnsignedByte, 5},  // Serial number
    {5, ttUndefined, 25},    // The array contains 30 bytes
};

//! Canon Ambience Selection Info binary array - configuration
constexpr ArrayCfg canonAmCfg = {
    IfdId::canonAmId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttSignedLong,      // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // Has a size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon MultiExposure Selection Info binary array - configuration
constexpr ArrayCfg canonMeCfg = {
    IfdId::canonMeId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttSignedLong,      // Type for array entry and size element
    notEncrypted,      // Not encrypted
    true,              // Has a size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon Filter Info binary array - configuration
constexpr ArrayCfg canonFilCfg = {
    IfdId::canonFilId,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttSignedLong,       // Type for array entry and size element
    notEncrypted,       // Not encrypted
    true,               // Has a size element
    false,              // No fillers
    false,              // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon Hdr Info binary array - configuration
constexpr ArrayCfg canonHdrCfg = {
    IfdId::canonHdrId,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttSignedLong,       // Type for array entry and size element
    notEncrypted,       // Not encrypted
    true,               // Has a size element
    false,              // No fillers
    false,              // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon AF Config Info binary array - configuration
constexpr ArrayCfg canonAfCCfg = {
    IfdId::canonAfCId,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttSignedLong,       // Type for array entry and size element
    notEncrypted,       // Not encrypted
    true,               // Has a size element
    false,              // No fillers
    false,              // Don't concatenate gaps
    {0, ttSignedLong, 1},
};

//! Canon RawBurst Info binary array - configuration
constexpr ArrayCfg canonRawBCfg = {
    IfdId::canonRawBId,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttSignedLong,        // Type for array entry and size element
    notEncrypted,        // Not encrypted
    true,                // Has a size element
    false,               // No fillers
    false,               // Don't concatenate gaps
    {0, ttUnsignedLong, 1},
};

//! Nikon Vibration Reduction binary array - configuration
constexpr ArrayCfg nikonVrCfg = {
    IfdId::nikonVrId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Vibration Reduction binary array - definition
constexpr ArrayDef nikonVrDef[] = {
    {0, ttUndefined, 4},     // Version
    {7, ttUnsignedByte, 1},  // The array contains 8 bytes
};

//! Nikon Picture Control binary array - configuration
constexpr ArrayCfg nikonPcCfg = {
    IfdId::nikonPcId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Picture Control binary array - definition
constexpr ArrayDef nikonPcDef[] = {
    {0, ttUndefined, 4},  // Version
    {4, ttAsciiString, 20},  {24, ttAsciiString, 20}, {48, ttUnsignedByte, 1},
    {49, ttUnsignedByte, 1}, {50, ttUnsignedByte, 1}, {51, ttUnsignedByte, 1},
    {52, ttUnsignedByte, 1}, {53, ttUnsignedByte, 1}, {54, ttUnsignedByte, 1},
    {55, ttUnsignedByte, 1}, {56, ttUnsignedByte, 1}, {57, ttUnsignedByte, 1},  // The array contains 58 bytes
};

//! Nikon World Time binary array - configuration
constexpr ArrayCfg nikonWtCfg = {
    IfdId::nikonWtId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon World Time binary array - definition
constexpr ArrayDef nikonWtDef[] = {
    {0, ttSignedShort, 1},
    {2, ttUnsignedByte, 1},
    {3, ttUnsignedByte, 1},
};

//! Nikon ISO info binary array - configuration
constexpr ArrayCfg nikonIiCfg = {
    IfdId::nikonIiId,  // Group for the elements
    bigEndian,         // Byte order
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon ISO info binary array - definition
constexpr ArrayDef nikonIiDef[] = {
    {0, ttUnsignedByte, 1},   {4, ttUnsignedShort, 1}, {6, ttUnsignedByte, 1},
    {10, ttUnsignedShort, 1}, {13, ttUnsignedByte, 1},  // The array contains 14 bytes
};

//! Nikon Auto Focus binary array - configuration
constexpr ArrayCfg nikonAfCfg = {
    IfdId::nikonAfId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Auto Focus binary array - definition
constexpr ArrayDef nikonAfDef[] = {
    {0, ttUnsignedByte, 1}, {1, ttUnsignedByte, 1}, {2, ttUnsignedShort, 1},  // The array contains 4 bytes
};

//! Nikon Auto Focus 21 binary array - configuration
constexpr ArrayCfg nikonAf21Cfg = {
    IfdId::nikonAf21Id,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttUndefined,         // Type for array entry
    notEncrypted,        // Not encrypted
    false,               // No size element
    true,                // Write all tags
    true,                // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Auto Focus 21 binary array - definition
constexpr ArrayDef nikonAf21Def[] = {
    {0, ttUndefined, 4},       // Version
    {4, ttUnsignedByte, 1},    // ContrastDetectAF
    {5, ttUnsignedByte, 1},    // AFAreaMode
    {6, ttUnsignedByte, 1},    // PhaseDetectAF
    {7, ttUnsignedByte, 1},    // PrimaryAFPoint
    {8, ttUnsignedByte, 7},    // AFPointsUsed
    {16, ttUnsignedShort, 1},  // AFImageWidth
    {18, ttUnsignedShort, 1},  // AFImageHeight
    {20, ttUnsignedShort, 1},  // AFAreaXPosition
    {22, ttUnsignedShort, 1},  // AFAreaYPosition
    {24, ttUnsignedShort, 1},  // AFAreaWidth
    {26, ttUnsignedShort, 1},  // AFAreaHeight
    {28, ttUnsignedByte, 1},   // ContrastDetectAFInFocus
};
//! Nikon Auto Focus 22 binary array - configuration
constexpr ArrayCfg nikonAf22Cfg = {
    IfdId::nikonAf22Id,  // Group for the elements
    invalidByteOrder,    // Byte order
    ttUndefined,         // Type for array entry
    notEncrypted,        // Not encrypted
    false,               // No size element
    true,                // Write all tags
    true,                // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Auto Focus 22 binary array - definition
constexpr ArrayDef nikonAf22Def[] = {
    {0, ttUndefined, 4},       // Version
    {4, ttUnsignedByte, 1},    // ContrastDetectAF
    {5, ttUnsignedByte, 1},    // AFAreaMode
    {6, ttUnsignedByte, 1},    // PhaseDetectAF
    {7, ttUnsignedByte, 1},    // PrimaryAFPoint
    {8, ttUnsignedByte, 7},    // AFPointsUsed
    {70, ttUnsignedShort, 1},  // AFImageWidth
    {72, ttUnsignedShort, 1},  // AFImageHeight
    {74, ttUnsignedShort, 1},  // AFAreaXPosition
    {76, ttUnsignedShort, 1},  // AFAreaYPosition
    {78, ttUnsignedShort, 1},  // AFAreaWidth
    {80, ttUnsignedShort, 1},  // AFAreaHeight
    {82, ttUnsignedByte, 1},   // ContrastDetectAFInFocus
};

//! Nikon AF2 configuration and definitions
//  https://github.com/Exiv2/exiv2/issues/646
constexpr ArraySet nikonAf2Set[] = {
    {nikonAf21Cfg, nikonAf21Def, std::size(nikonAf21Def)},
    {nikonAf22Cfg, nikonAf22Def, std::size(nikonAf22Def)},
};

//! Nikon AF Fine Tune binary array - configuration
constexpr ArrayCfg nikonAFTCfg = {
    IfdId::nikonAFTId,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon AF Fine Tune binary array - definition
constexpr ArrayDef nikonAFTDef[] = {
    {0, ttUnsignedByte, 1},  // AF Fine Tune on/off
    {1, ttUnsignedByte, 1},  // AF Fine Tune index
    {2, ttUnsignedByte, 1},  // AF Fine Tune value
};

//! Nikon File Info binary array - configuration
constexpr ArrayCfg nikonFiCfg = {
    IfdId::nikonFiId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon File Info binary array - definition
constexpr ArrayDef nikonFiDef[] = {
    {0, ttUndefined, 4},      // Version
    {6, ttUnsignedShort, 1},  // Directory Number
    {8, ttUnsignedShort, 1},  // File Number
};

//! Nikon Multi Exposure binary array - configuration
constexpr ArrayCfg nikonMeCfg = {
    IfdId::nikonMeId,  // Group for the elements
    invalidByteOrder,  // Use byte order from parent
    ttUndefined,       // Type for array entry
    notEncrypted,      // Not encrypted
    false,             // No size element
    true,              // Write all tags
    true,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Multi Exposure binary array - definition
constexpr ArrayDef nikonMeDef[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedLong, 1},   // MultiExposureMode
    {8, ttUnsignedLong, 1},   // MultiExposureShots
    {12, ttUnsignedLong, 1},  // MultiExposureAutoGain
};

//! Nikon Flash Info binary array - configuration 1
constexpr ArrayCfg nikonFl1Cfg = {
    IfdId::nikonFl1Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Flash Info binary array - definition 1
constexpr ArrayDef nikonFl1Def[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedByte, 1},   // FlashSource
    {6, ttUnsignedShort, 1},  // ExternalFlashFirmware
    {8, ttUnsignedByte, 1},   // ExternalFlashFlags
    {11, ttUnsignedByte, 1},  // FlashFocalLength
    {12, ttUnsignedByte, 1},  // RepeatingFlashRate
    {13, ttUnsignedByte, 1},  // RepeatingFlashCount
    {14, ttUnsignedByte, 1},  // FlashGNDistance
    {15, ttUnsignedByte, 1},  // FlashGroupAControlMode
    {16, ttUnsignedByte, 1},  // FlashGroupBControlMode
};
//! Nikon Flash Info binary array - configuration 2
constexpr ArrayCfg nikonFl2Cfg = {
    IfdId::nikonFl2Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Flash Info binary array - definition 2
constexpr ArrayDef nikonFl2Def[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedByte, 1},   // FlashSource
    {6, ttUnsignedShort, 1},  // ExternalFlashFirmware
    {8, ttUnsignedByte, 1},   // ExternalFlashFlags
    {12, ttUnsignedByte, 1},  // FlashFocalLength
    {13, ttUnsignedByte, 1},  // RepeatingFlashRate
    {14, ttUnsignedByte, 1},  // RepeatingFlashCount
    {15, ttUnsignedByte, 1},  // FlashGNDistance
};
//! Nikon Flash Info binary array - configuration 3
constexpr ArrayCfg nikonFl3Cfg = {
    IfdId::nikonFl3Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Flash Info binary array - definition
constexpr ArrayDef nikonFl3Def[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedByte, 1},   // FlashSource
    {6, ttUnsignedShort, 1},  // ExternalFlashFirmware
    {8, ttUnsignedByte, 1},   // ExternalFlashFlags
    {12, ttUnsignedByte, 1},  // FlashFocalLength
    {13, ttUnsignedByte, 1},  // RepeatingFlashRate
    {14, ttUnsignedByte, 1},  // RepeatingFlashCount
    {15, ttUnsignedByte, 1},  // FlashGNDistance
    {16, ttUnsignedByte, 1},  // FlashColorFilter
};
//! Nikon Flash Info 6 (0106) binary array - configuration
constexpr ArrayCfg nikonFl6Cfg = {
    IfdId::nikonFl6Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Flash Info 6 (0106) binary array - definition
constexpr ArrayDef nikonFl6Def[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedByte, 1},   // FlashSource
    {6, ttUnsignedShort, 1},  // ExternalFlashFirmware
    {8, ttUnsignedByte, 1},   // ExternalFlashData1
    {9, ttUnsignedByte, 1},   // ExternalFlashData2
    {10, ttUnsignedByte, 1},  // FlashCompensationMaster
    {12, ttUnsignedByte, 1},  // FlashFocalLength
    {13, ttUnsignedByte, 1},  // RepeatingFlashRate
    {14, ttUnsignedByte, 1},  // RepeatingFlashCount
    {15, ttUnsignedByte, 1},  // FlashGNDistance
    {16, ttUnsignedByte, 1},  // FlashColorFilter
    {17, ttUnsignedByte, 1},  // FlashGroupAControlData
    {18, ttUnsignedByte, 1},  // FlashGroupBCControlData
    {19, ttUnsignedByte, 1},  // FlashCompensationGroupA
    {20, ttUnsignedByte, 1},  // FlashCompensationGroupB
    {21, ttUnsignedByte, 1},  // FlashCompensationGroupC
    {28, ttUnsignedByte, 1},  // CameraExposureCompensation
    {29, ttUnsignedByte, 1},  // CameraFlashCompensation
    {39, ttUnsignedByte, 1},  // FlashMasterOutput
    {40, ttUnsignedByte, 1},  // FlashGroupAOutput
    {41, ttUnsignedByte, 1},  // FlashGroupBOutput
    {42, ttUnsignedByte, 1},  // FlashGroupCOutput
};
//! Nikon Flash Info 7 (0107 and 0108) binary array - configuration
constexpr ArrayCfg nikonFl7Cfg = {
    IfdId::nikonFl7Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Not encrypted
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Flash Info 7 (0107 and 0108) binary array - definition
constexpr ArrayDef nikonFl7Def[] = {
    {0, ttUndefined, 4},      // Version
    {4, ttUnsignedByte, 1},   // FlashSource
    {6, ttUnsignedShort, 1},  // ExternalFlashFirmware
    {8, ttUnsignedByte, 1},   // ExternalFlashData1
    {9, ttUnsignedByte, 1},   // ExternalFlashData2
    {10, ttUnsignedByte, 1},  // FlashCompensationMaster
    {12, ttUnsignedByte, 1},  // FlashFocalLength
    {13, ttUnsignedByte, 1},  // RepeatingFlashRate
    {14, ttUnsignedByte, 1},  // RepeatingFlashCount
    {15, ttUnsignedByte, 1},  // FlashGNDistance
    {16, ttUnsignedByte, 1},  // FlashColorFilter
    {17, ttUnsignedByte, 1},  // FlashGroupAControlData
    {18, ttUnsignedByte, 1},  // FlashGroupBCControlData
    {19, ttUnsignedByte, 1},  // FlashCompensationGroupA
    {20, ttUnsignedByte, 1},  // FlashCompensationGroupB
    {21, ttUnsignedByte, 1},  // FlashCompensationGroupC
    {22, ttUnsignedByte, 1},  // ExternalFlashData3
    {27, ttUnsignedByte, 1},  // CameraFlashOutput
    {28, ttUnsignedByte, 1},  // CameraExposureCompensation
    {29, ttUnsignedByte, 1},  // CameraFlashCompensation
    {37, ttUnsignedByte, 1},  // ExternalFlashData4
    {38, ttUnsignedByte, 1},  // FlashZoomHeadPosition
    {39, ttUnsignedByte, 1},  // FlashMasterOutput
    {40, ttUnsignedByte, 1},  // FlashGroupAOutput
    {41, ttUnsignedByte, 1},  // FlashGroupBOutput
    {42, ttUnsignedByte, 1},  // FlashGroupCOutput
    {67, ttUnsignedByte, 1},  // WirelessFlashData
};

//! Nikon Flash Info Data configurations and definitions
constexpr ArraySet nikonFlSet[] = {
    {nikonFl1Cfg, nikonFl1Def, std::size(nikonFl1Def)}, {nikonFl2Cfg, nikonFl2Def, std::size(nikonFl2Def)},
    {nikonFl3Cfg, nikonFl3Def, std::size(nikonFl3Def)}, {nikonFl6Cfg, nikonFl6Def, std::size(nikonFl6Def)},
    {nikonFl7Cfg, nikonFl7Def, std::size(nikonFl7Def)},
};

//! Nikon Shot Info binary array - configuration 1 (D80)
constexpr ArrayCfg nikonSi1Cfg = {
    IfdId::nikonSi1Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Shot Info binary array - definition 1 (D80)
constexpr ArrayDef nikonSi1Def[] = {
    {0, ttUndefined, 4},        // Version
    {586, ttUnsignedLong, 1},   // ShutterCount
    {1155, ttUnsignedByte, 1},  // The array contains 1156 bytes
};
//! Nikon Shot Info binary array - configuration 2 (D40)
constexpr ArrayCfg nikonSi2Cfg = {
    IfdId::nikonSi2Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Shot Info binary array - definition 2 (D40)
constexpr ArrayDef nikonSi2Def[] = {
    {0, ttUndefined, 4},       // Version
    {582, ttUnsignedLong, 1},  // ShutterCount
    {738, ttUnsignedByte, 1},
    {1112, ttUnsignedByte, 1},  // The array contains 1113 bytes
};
//! Nikon Shot Info binary array - configuration 3 (D300a)
constexpr ArrayCfg nikonSi3Cfg = {
    IfdId::nikonSi3Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Shot Info binary array - definition 3 (D300a)
constexpr ArrayDef nikonSi3Def[] = {
    {0, ttUndefined, 4},        // Version
    {604, ttUnsignedByte, 1},   // ISO
    {633, ttUnsignedLong, 1},   // ShutterCount
    {721, ttUnsignedShort, 1},  // AFFineTuneAdj
    {814, ttUndefined, 4478},   // The array contains 5291 bytes
};
//! Nikon Shot Info binary array - configuration 4 (D300b)
constexpr ArrayCfg nikonSi4Cfg = {
    IfdId::nikonSi4Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Shot Info binary array - definition 4 (D300b)
constexpr ArrayDef nikonSi4Def[] = {
    {0, ttUndefined, 4},        // Version
    {644, ttUnsignedLong, 1},   // ShutterCount
    {732, ttUnsignedShort, 1},  // AFFineTuneAdj
    {826, ttUndefined, 4478},   // The array contains 5303 bytes
};
//! Nikon Shot Info binary array - configuration 5 (ver 02.xx)
constexpr ArrayCfg nikonSi5Cfg = {
    IfdId::nikonSi5Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    false,              // Write all tags (don't know how many)
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Shot Info binary array - definition 5 (ver 01.xx and ver 02.xx)
constexpr ArrayDef nikonSi5Def[] = {
    {0, ttUndefined, 4},       // Version
    {106, ttUnsignedLong, 1},  // ShutterCount1
    {110, ttUnsignedLong, 1},  // DeletedImageCount
    {117, ttUnsignedByte, 1},  // VibrationReduction
    {130, ttUnsignedByte, 1},  // VibrationReduction1
    {343, ttUndefined, 2},     // ShutterCount
    {430, ttUnsignedByte, 1},  // VibrationReduction2
    {598, ttUnsignedByte, 1},  // ISO
    {630, ttUnsignedLong, 1}   // ShutterCount
};
//! Nikon Shot Info binary array - configuration 6 (ver 01.xx)
constexpr ArrayCfg nikonSi6Cfg = {
    IfdId::nikonSi6Id,  // Group for the elements
    bigEndian,          // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Encryption function
    false,              // No size element
    false,              // Write all tags (don't know how many)
    true,               // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Lens Data configurations and definitions
constexpr ArraySet nikonSiSet[] = {
    {nikonSi1Cfg, nikonSi1Def, std::size(nikonSi1Def)},
    {nikonSi2Cfg, nikonSi2Def, std::size(nikonSi2Def)},
    {nikonSi3Cfg, nikonSi3Def, std::size(nikonSi3Def)},
    {nikonSi4Cfg, nikonSi4Def, std::size(nikonSi4Def)},
    {nikonSi5Cfg, nikonSi5Def, std::size(nikonSi5Def)},
    {nikonSi6Cfg, nikonSi5Def, std::size(nikonSi5Def)},  // uses nikonSi5Def
};

//! Nikon Lens Data binary array - configuration 1
constexpr ArrayCfg nikonLd1Cfg = {
    IfdId::nikonLd1Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Encryption function
    false,              // No size element
    true,               // Write all tags
    false,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Lens Data binary array - configuration 2
constexpr ArrayCfg nikonLd2Cfg = {
    IfdId::nikonLd2Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    false,              // Concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Lens Data binary array - configuration 3
constexpr ArrayCfg nikonLd3Cfg = {
    IfdId::nikonLd3Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    false,              // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Lens Data binary array - configuration 3
constexpr ArrayCfg nikonLd4Cfg = {
    IfdId::nikonLd4Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    true,               // Write all tags
    false,              // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
//! Nikon Lens Data binary array - definition
constexpr ArrayDef nikonLdDef[] = {
    {0, ttUndefined, 4},  // Version
};
//! Nikon Lens Data binary array - definition
constexpr ArrayDef nikonLd4Def[] = {
    {0, ttUndefined, 4},       // Version
    {48, ttUnsignedShort, 1},  // LensID
    {54, ttUnsignedShort, 1},  // MaxAperture
    {56, ttUnsignedShort, 1},  // FNumber
    {60, ttUnsignedShort, 1},  // FocalLength
    {78, ttUnsignedShort, 1},  // FocusDistance
};
//! Nikon Lens Data configurations and definitions
constexpr ArraySet nikonLdSet[] = {
    {nikonLd1Cfg, nikonLdDef, std::size(nikonLdDef)},
    {nikonLd2Cfg, nikonLdDef, std::size(nikonLdDef)},
    {nikonLd3Cfg, nikonLdDef, std::size(nikonLdDef)},
    {nikonLd4Cfg, nikonLd4Def, std::size(nikonLd4Def)},
};

//! Nikon Color Balance binary array - configuration 1
constexpr ArrayCfg nikonCb1Cfg = {
    IfdId::nikonCb1Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Encryption function
    false,              // No size element
    false,              // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - configuration 2
constexpr ArrayCfg nikonCb2Cfg = {
    IfdId::nikonCb2Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    false,              // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - configuration 2a
constexpr ArrayCfg nikonCb2aCfg = {
    IfdId::nikonCb2aId,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttUndefined,         // Type for array entry
    nikonCrypt,          // Encryption function
    false,               // No size element
    false,               // Write all tags
    true,                // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - configuration 2b
constexpr ArrayCfg nikonCb2bCfg = {
    IfdId::nikonCb2bId,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttUndefined,         // Type for array entry
    nikonCrypt,          // Encryption function
    false,               // No size element
    false,               // Write all tags
    true,                // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - configuration 3
constexpr ArrayCfg nikonCb3Cfg = {
    IfdId::nikonCb3Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    notEncrypted,       // Encryption function
    false,              // No size element
    false,              // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - configuration 4
constexpr ArrayCfg nikonCb4Cfg = {
    IfdId::nikonCb4Id,  // Group for the elements
    invalidByteOrder,   // Use byte order from parent
    ttUndefined,        // Type for array entry
    nikonCrypt,         // Encryption function
    false,              // No size element
    false,              // Write all tags
    true,               // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Nikon Color Balance binary array - definition 1 (D100)
constexpr ArrayDef nikonCb1Def[] = {
    {0, ttUndefined, 4},       // Version
    {72, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance binary array - definition 2 (D2H)
constexpr ArrayDef nikonCb2Def[] = {
    {0, ttUndefined, 4},       // Version
    {10, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance binary array - definition 2a (D50)
constexpr ArrayDef nikonCb2aDef[] = {
    {0, ttUndefined, 4},       // Version
    {18, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance binary array - definition 2b (D2X=0204,D2Hs=0206,D200=0207,D40=0208)
constexpr ArrayDef nikonCb2bDef[] = {
    {0, ttUndefined, 4},        // Version
    {4, ttUnsignedShort, 140},  // Unknown
    {284, ttUnsignedShort, 3},  // Unknown (encrypted)
    {290, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance binary array - definition 3 (D70)
constexpr ArrayDef nikonCb3Def[] = {
    {0, ttUndefined, 4},       // Version
    {20, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance binary array - definition 4 (D3)
constexpr ArrayDef nikonCb4Def[] = {
    {0, ttUndefined, 4},        // Version
    {4, ttUnsignedShort, 140},  // Unknown
    {284, ttUnsignedShort, 5},  // Unknown (encrypted)
    {294, ttUnsignedShort, 4},  // Color balance levels
};
//! Nikon Color Balance configurations and definitions
constexpr ArraySet nikonCbSet[] = {
    {nikonCb1Cfg, nikonCb1Def, std::size(nikonCb1Def)},    {nikonCb2Cfg, nikonCb2Def, std::size(nikonCb2Def)},
    {nikonCb2aCfg, nikonCb2aDef, std::size(nikonCb2aDef)}, {nikonCb2bCfg, nikonCb2bDef, std::size(nikonCb2bDef)},
    {nikonCb3Cfg, nikonCb3Def, std::size(nikonCb3Def)},    {nikonCb4Cfg, nikonCb4Def, std::size(nikonCb4Def)},
};

//! Minolta Camera Settings (old) binary array - configuration
constexpr ArrayCfg minoCsoCfg = {
    IfdId::minoltaCsOldId,  // Group for the elements
    bigEndian,              // Big endian
    ttUndefined,            // Type for array entry and size element
    notEncrypted,           // Not encrypted
    false,                  // No size element
    false,                  // No fillers
    false,                  // Don't concatenate gaps
    {0, ttUnsignedLong, 1},
};

//! Minolta Camera Settings (new) binary array - configuration
constexpr ArrayCfg minoCsnCfg = {
    IfdId::minoltaCsNewId,  // Group for the elements
    bigEndian,              // Big endian
    ttUndefined,            // Type for array entry and size element
    notEncrypted,           // Not encrypted
    false,                  // No size element
    false,                  // No fillers
    false,                  // Don't concatenate gaps
    {0, ttUnsignedLong, 1},
};

//! Minolta 7D Camera Settings binary array - configuration
constexpr ArrayCfg minoCs7Cfg = {
    IfdId::minoltaCs7DId,  // Group for the elements
    bigEndian,             // Big endian
    ttUndefined,           // Type for array entry and size element
    notEncrypted,          // Not encrypted
    false,                 // No size element
    false,                 // No fillers
    false,                 // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Minolta 7D Camera Settings binary array - definition
constexpr ArrayDef minoCs7Def[] = {
    {60, ttSignedShort, 1},   // Exif.MinoltaCs7D.ExposureCompensation
    {126, ttSignedShort, 1},  // Exif.MinoltaCs7D.ColorTemperature
};

//! Minolta 5D Camera Settings binary array - configuration
constexpr ArrayCfg minoCs5Cfg = {
    IfdId::minoltaCs5DId,  // Group for the elements
    bigEndian,             // Big endian
    ttUndefined,           // Type for array entry and size element
    notEncrypted,          // Not encrypted
    false,                 // No size element
    false,                 // No fillers
    false,                 // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Minolta 5D Camera Settings binary array - definition
constexpr ArrayDef minoCs5Def[] = {
    {146, ttSignedShort, 1},  // Exif.MinoltaCs5D.ColorTemperature
};

// Todo: Performance of the handling of Sony Camera Settings can be
//       improved by defining all known array elements in the definitions
//       sonyCsDef and sonyCs2Def below and enabling the 'concatenate gaps'
//       setting in all four configurations.

//! Sony1 Camera Settings binary array - configuration
constexpr ArrayCfg sony1CsCfg = {
    IfdId::sony1CsId,  // Group for the elements
    bigEndian,         // Big endian
    ttUndefined,       // Type for array entry and size element
    notEncrypted,      // Not encrypted
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Sony1 Camera Settings 2 binary array - configuration
constexpr ArrayCfg sony1Cs2Cfg = {
    IfdId::sony1Cs2Id,  // Group for the elements
    bigEndian,          // Big endian
    ttUndefined,        // Type for array entry and size element
    notEncrypted,       // Not encrypted
    false,              // No size element
    false,              // No fillers
    false,              // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

constexpr ArrayCfg sony2FpCfg = {
    IfdId::sony2FpId,  // Group for the elements
    bigEndian,         // Big endian
    ttUnsignedByte,    // Type for array entry and size element
    sonyTagDecipher,   // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
constexpr ArrayDef sony2FpDef[] = {
    {0x4, ttSignedByte, 1},     // Exif.Sony2Fp.AmbientTemperature
    {0x16, ttUnsignedByte, 1},  // Exif.Sony2Fp.FocusMode
    {0x17, ttUnsignedByte, 1},  // Exif.Sony2Fp.AFAreaMode
    {0x2d, ttUnsignedByte, 1},  // Exif.Sony2Fp.FocusPosition2
};

//! Sony2Fp configurations and definitions
constexpr ArraySet sony2FpSet[] = {
    {sony2FpCfg, sony2FpDef, std::size(sony2FpDef)},
};

constexpr ArrayCfg sonyMisc1Cfg = {
    IfdId::sonyMisc1Id,  // Group for the elements
    bigEndian,           // Big endian
    ttUnsignedByte,      // Type for array entry and size element
    sonyTagDecipher,     // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,               // No size element
    false,               // No fillers
    false,               // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};

constexpr ArrayDef sonyMisc1Def[] = {
    {0x05, ttSignedByte, 1},  // Exif.SonyMisc1.CameraTemperature
};

constexpr ArrayCfg sonyMisc2bCfg = {
    IfdId::sonyMisc2bId,  // Group for the elements
    littleEndian,         // Little endian
    ttUnsignedByte,       // Type for array entry and size element
    sonyTagDecipher,      // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,                // No size element
    false,                // No fillers
    false,                // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};

constexpr ArrayDef sonyMisc2bDef[] = {
    {12, ttUnsignedByte, 1},   // Exif.SonyMisc2b.ExposureProgram
    {14, ttUnsignedByte, 1},   // Exif.SonyMisc2b.IntelligentAuto
    {30, ttUnsignedShort, 1},  // Exif.SonyMisc2b.LensZoomPosition
    {32, ttUnsignedByte, 1},   // Exif.SonyMisc2b.FocusPosition2
};

//! SonyMisc2b configurations and definitions
constexpr ArraySet sonyMisc2bSet[] = {
    {sonyMisc2bCfg, sonyMisc2bDef, std::size(sonyMisc2bDef)},
};

constexpr ArrayCfg sonyMisc3cCfg = {
    IfdId::sonyMisc3cId,  // Group for the elements
    littleEndian,         // Little endian
    ttUnsignedByte,       // Type for array entry and size element
    sonyTagDecipher,      // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,                // No size element
    false,                // No fillers
    false,                // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};

constexpr ArrayDef sonyMisc3cDef[] = {
    {0x09, ttUnsignedByte, 1},   // Exif.SonyMisc3c.ReleaseMode2
    {0x0a, ttUnsignedLong, 1},   // Exif.SonyMisc3c.ShotNumberSincePowerUp
    {0x12, ttUnsignedLong, 1},   // Exif.SonyMisc3c.SequenceImageNumber
    {0x16, ttUnsignedByte, 1},   // Exif.SonyMisc3c.SequenceLength1
    {0x1a, ttUnsignedLong, 1},   // Exif.SonyMisc3c.SequenceFileNumber
    {0x1e, ttUnsignedByte, 1},   // Exif.SonyMisc3c.SequenceLength2
    {0x29, ttUnsignedByte, 1},   // Exif.SonyMisc3c.CameraOrientation
    {0x2a, ttUnsignedByte, 1},   // Exif.SonyMisc3c.Quality2
    {0x47, ttUnsignedShort, 1},  // Exif.SonyMisc3c.SonyImageHeight
    {0x53, ttUnsignedByte, 1},   // Exif.SonyMisc3c.ModelReleaseYear
};

//! SonyMisc3c Settings configurations and definitions
constexpr ArraySet sonyMisc3cSet[] = {
    {sonyMisc3cCfg, sonyMisc3cDef, std::size(sonyMisc3cDef)},
};

constexpr ArrayCfg sonySInfo1Cfg = {
    IfdId::sonySInfo1Id,  // Group for the elements
    littleEndian,         // Little endian
    ttUnsignedByte,       // Type for array entry and size element
    notEncrypted,         // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,                // No size element
    false,                // No fillers
    false,                // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
constexpr ArrayDef sonySInfo1Def[] = {
    {6, ttAsciiString, 20},    // Exif.SonySInfo1.SonyDateTime
    {26, ttUnsignedShort, 1},  // Exif.SonySInfo1.SonyImageHeight
    {28, ttUnsignedShort, 1},  // Exif.SonySInfo1.SonyImageWidth
    {48, ttUnsignedShort, 1},  // Exif.SonySInfo1.FacesDetected
    {52, ttAsciiString, 16},   // Exif.SonySInfo1.MetaVersion
                               //
};

constexpr ArrayCfg sony2010eCfg = {
    IfdId::sony2010eId,  // Group for the elements
    invalidByteOrder,    // inherit from file.  Usually littleEndian
    ttUnsignedByte,      // Type for array entry and size element
    sonyTagDecipher,     // (uint16_t, const byte*, uint32_t, TiffComponent* const);
    false,               // No size element
    false,               // No fillers
    false,               // Don't concatenate gaps
    {0, ttUnsignedByte, 1},
};
constexpr ArrayDef sony2010eDef[] = {
    {0, ttUnsignedLong, 1},      // Exif.Sony2010. SequenceImageNumber
    {4, ttUnsignedLong, 1},      // Exif.Sony2010. SequenceFileNumber
    {8, ttUnsignedLong, 1},      // Exif.Sony2010. ReleaseMode2
    {540, ttUnsignedByte, 1},    // Exif.Sony2010. DigitalZoomRatio
    {556, ttUndefined, 1},       // Exif.Sony2010. SonyDateTime
    {808, ttUnsignedByte, 1},    // Exif.Sony2010. DynamicRangeOptimizer
    {1208, ttUndefined, 1},      // Exif.Sony2010. MeterInfo
    {4444, ttUnsignedByte, 1},   // Exif.Sony2010. ReleaseMode3
    {4448, ttUnsignedByte, 1},   // Exif.Sony2010. ReleaseMode2
    {4456, ttUnsignedByte, 1},   // Exif.Sony2010. SelfTimer
    {4460, ttUnsignedByte, 1},   // Exif.Sony2010. FlashMode
    {4466, ttUnsignedShort, 1},  // Exif.Sony2010. StopsAboveBaseISO
    {4468, ttUnsignedShort, 1},  // Exif.Sony2010. BrightnessValue
    {4472, ttUnsignedByte, 1},   // Exif.Sony2010. DynamicRangeOptimizer
    {4476, ttUnsignedByte, 1},   // Exif.Sony2010. HDRSetting
    {4480, ttSignedShort, 1},    // Exif.Sony2010. ExposureCompensation
    {4502, ttUnsignedByte, 1},   // Exif.Sony2010. PictureProfile
    {4503, ttUnsignedByte, 1},   // Exif.Sony2010. PictureProfile2
    {4507, ttUnsignedByte, 1},   // Exif.Sony2010. PictureEffect2
    {4520, ttUnsignedByte, 1},   // Exif.Sony2010. Quality2
    {4524, ttUnsignedByte, 1},   // Exif.Sony2010. MeteringMode
    {4525, ttUnsignedByte, 1},   // Exif.Sony2010. ExposureProgram
    {4532, ttUnsignedShort, 3},  // Exif.Sony2010. WB_RGBLevels
    {4692, ttUnsignedShort, 1},  // Exif.Sony2010. SonyISO
    {4696, ttUnsignedShort, 1},  // Exif.Sony2010. SonyISO2
    {4728, ttUnsignedShort, 1},  // Exif.Sony2010. FocalLength
    {4730, ttUnsignedShort, 1},  // Exif.Sony2010. MinFocalLength
    {4732, ttUnsignedShort, 1},  // Exif.Sony2010. MaxFocalLength
    {4736, ttUnsignedShort, 1},  // Exif.Sony2010. SonyISO3
    {6256, ttSignedShort, 16},   // Exif.Sony2010. DistortionCorrParams
    {6289, ttUnsignedByte, 1},   // Exif.Sony2010. LensFormat
    {6290, ttUnsignedByte, 1},   // Exif.Sony2010. LensMount
    {6291, ttUnsignedShort, 1},  // Exif.Sony2010. LensType2
    {6294, ttUnsignedShort, 1},  // Exif.Sony2010. LensType
    {6296, ttUnsignedByte, 1},   // Exif.Sony2010. DistortionCorrParamsPresent
    {6297, ttUnsignedByte, 1},   // Exif.Sony2010. DistortionCorrParamsNumber
};

//! Sony1 Camera Settings configurations and definitions
constexpr ArraySet sony2010eSet[] = {
    {sony2010eCfg, sony2010eDef, std::size(sony2010eDef)},
};

//! Sony[12] Camera Settings binary array - definition
constexpr ArrayDef sonyCsDef[] = {
    {12, ttSignedShort, 1},  // Exif.Sony[12]Cs.WhiteBalanceFineTune
};
//! Sony2 Camera Settings binary array - configuration
constexpr ArrayCfg sony2CsCfg = {
    IfdId::sony2CsId,  // Group for the elements
    bigEndian,         // Big endian
    ttUndefined,       // Type for array entry and size element
    notEncrypted,      // Not encrypted
    false,             // No size element
    false,             // No fillers
    false,             // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Sony2 Camera Settings 2 binary array - configuration
constexpr ArrayCfg sony2Cs2Cfg = {
    IfdId::sony2Cs2Id,  // Group for the elements
    bigEndian,          // Big endian
    ttUndefined,        // Type for array entry and size element
    notEncrypted,       // Not encrypted
    false,              // No size element
    false,              // No fillers
    false,              // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Sony[12] Camera Settings 2 binary array - definition
constexpr ArrayDef sonyCs2Def[] = {
    {44, ttUnsignedShort, 1},  // Exif.Sony[12]Cs2.FocusMode
};
//! Sony1 Camera Settings configurations and definitions
constexpr ArraySet sony1CsSet[] = {
    {sony1CsCfg, sonyCsDef, std::size(sonyCsDef)},
    {sony1Cs2Cfg, sonyCs2Def, std::size(sonyCs2Def)},
};
//! Sony2 Camera Settings configurations and definitions
constexpr ArraySet sony2CsSet[] = {
    {sony2CsCfg, sonyCsDef, std::size(sonyCsDef)},
    {sony2Cs2Cfg, sonyCs2Def, std::size(sonyCs2Def)},
};

//! Sony Minolta Camera Settings (old) binary array - configuration
constexpr ArrayCfg sony1MCsoCfg = {
    IfdId::sony1MltCsOldId,  // Group for the elements
    bigEndian,               // Big endian
    ttUndefined,             // Type for array entry and size element
    notEncrypted,            // Not encrypted
    false,                   // No size element
    false,                   // No fillers
    false,                   // Don't concatenate gaps
    {0, ttUnsignedLong, 1},
};

//! Sony Minolta Camera Settings (new) binary array - configuration
constexpr ArrayCfg sony1MCsnCfg = {
    IfdId::sony1MltCsNewId,  // Group for the elements
    bigEndian,               // Big endian
    ttUndefined,             // Type for array entry and size element
    notEncrypted,            // Not encrypted
    false,                   // No size element
    false,                   // No fillers
    false,                   // Don't concatenate gaps
    {0, ttUnsignedLong, 1},
};

//! Sony Minolta 7D Camera Settings binary array - configuration
constexpr ArrayCfg sony1MCs7Cfg = {
    IfdId::sony1MltCs7DId,  // Group for the elements
    bigEndian,              // Big endian
    ttUndefined,            // Type for array entry and size element
    notEncrypted,           // Not encrypted
    false,                  // No size element
    false,                  // No fillers
    false,                  // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};

//! Sony Minolta A100 Camera Settings binary array - configuration
constexpr ArrayCfg sony1MCsA100Cfg = {
    IfdId::sony1MltCsA100Id,  // Group for the elements
    bigEndian,                // Big endian
    ttUndefined,              // Type for array entry and size element
    notEncrypted,             // Not encrypted
    false,                    // No size element
    false,                    // No fillers
    false,                    // Don't concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Sony Minolta A100 Camera Settings binary array - definition
constexpr ArrayDef sony1MCsA100Def[] = {
    {112, ttSignedShort, 1},  // Exif.Sony1MltCsA100.WhiteBalanceFineTune
    {116, ttSignedShort, 1},  // Exif.Sony1MltCsA100.ColorCompensationFilter
    {190, ttSignedShort, 1},  // Exif.Sony1MltCsA100.ColorCompensationFilter2
};

//! Samsung PictureWizard binary array - configuration
constexpr ArrayCfg samsungPwCfg = {
    IfdId::samsungPwId,  // Group for the elements
    invalidByteOrder,    // Use byte order from parent
    ttUnsignedShort,     // Type for array entry
    notEncrypted,        // Not encrypted
    false,               // No size element
    true,                // Write all tags
    true,                // Concatenate gaps
    {0, ttUnsignedShort, 1},
};
//! Samsung PictureWizard binary array - definition
constexpr ArrayDef samsungPwDef[] = {
    {0, ttUnsignedShort, 1},  // Mode
    {2, ttUnsignedShort, 1},  // Color
    {4, ttUnsignedShort, 1},  // Saturation
    {6, ttUnsignedShort, 1},  // Sharpness
    {8, ttUnsignedShort, 1},  // Contrast
};

/*
  This table lists for each group in a tree, its parent group and tag.
  Root identifies the root of a TIFF tree, as there is a need for multiple
  trees. Groups are the nodes of a TIFF tree. A group is an IFD or any
  other composite component.

  With this table, it is possible, for a given group (and tag) to find a
  path, i.e., a list of groups and tags, from the root to that group (tag).
*/
const TiffTreeTable TiffCreator::tiffTreeTable_ = {
    // root      group             parent group      parent tag
    //---------  ----------------- ----------------- ----------
    {{Tag::root, IfdId::ifdIdNotSet}, {IfdId::ifdIdNotSet, Tag::root}},
    {{Tag::root, IfdId::ifd0Id}, {IfdId::ifdIdNotSet, Tag::root}},
    {{Tag::root, IfdId::subImage1Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage2Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage3Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage4Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage5Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage6Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage7Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage8Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::subImage9Id}, {IfdId::ifd0Id, 0x014a}},
    {{Tag::root, IfdId::exifId}, {IfdId::ifd0Id, 0x8769}},
    {{Tag::root, IfdId::gpsId}, {IfdId::ifd0Id, 0x8825}},
    {{Tag::root, IfdId::iopId}, {IfdId::exifId, 0xa005}},
    {{Tag::root, IfdId::ifd1Id}, {IfdId::ifd0Id, Tag::next}},
    {{Tag::root, IfdId::ifd2Id}, {IfdId::ifd1Id, Tag::next}},
    {{Tag::root, IfdId::ifd3Id}, {IfdId::ifd2Id, Tag::next}},
    {{Tag::root, IfdId::olympusId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::olympus2Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::subThumb1Id}, {IfdId::ifd1Id, 0x014a}},
    {{Tag::root, IfdId::olympusEqId}, {IfdId::olympus2Id, 0x2010}},
    {{Tag::root, IfdId::olympusCsId}, {IfdId::olympus2Id, 0x2020}},
    {{Tag::root, IfdId::olympusRdId}, {IfdId::olympus2Id, 0x2030}},
    {{Tag::root, IfdId::olympusRd2Id}, {IfdId::olympus2Id, 0x2031}},
    {{Tag::root, IfdId::olympusIpId}, {IfdId::olympus2Id, 0x2040}},
    {{Tag::root, IfdId::olympusFiId}, {IfdId::olympus2Id, 0x2050}},
    {{Tag::root, IfdId::olympusFe1Id}, {IfdId::olympus2Id, 0x2100}},
    {{Tag::root, IfdId::olympusFe2Id}, {IfdId::olympus2Id, 0x2200}},
    {{Tag::root, IfdId::olympusFe3Id}, {IfdId::olympus2Id, 0x2300}},
    {{Tag::root, IfdId::olympusFe4Id}, {IfdId::olympus2Id, 0x2400}},
    {{Tag::root, IfdId::olympusFe5Id}, {IfdId::olympus2Id, 0x2500}},
    {{Tag::root, IfdId::olympusFe6Id}, {IfdId::olympus2Id, 0x2600}},
    {{Tag::root, IfdId::olympusFe7Id}, {IfdId::olympus2Id, 0x2700}},
    {{Tag::root, IfdId::olympusFe8Id}, {IfdId::olympus2Id, 0x2800}},
    {{Tag::root, IfdId::olympusFe9Id}, {IfdId::olympus2Id, 0x2900}},
    {{Tag::root, IfdId::olympusRiId}, {IfdId::olympus2Id, 0x3000}},
    {{Tag::root, IfdId::fujiId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::canonId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::canonCsId}, {IfdId::canonId, 0x0001}},
    {{Tag::root, IfdId::canonSiId}, {IfdId::canonId, 0x0004}},
    {{Tag::root, IfdId::canonPaId}, {IfdId::canonId, 0x0005}},
    {{Tag::root, IfdId::canonCfId}, {IfdId::canonId, 0x000f}},
    {{Tag::root, IfdId::canonPiId}, {IfdId::canonId, 0x0012}},
    {{Tag::root, IfdId::canonTiId}, {IfdId::canonId, 0x0035}},
    {{Tag::root, IfdId::canonFiId}, {IfdId::canonId, 0x0093}},
    {{Tag::root, IfdId::canonPrId}, {IfdId::canonId, 0x00a0}},
    {{Tag::root, IfdId::canonAfMiAdjId}, {IfdId::canonId, 0x4013}},
    {{Tag::root, IfdId::canonVigCor2Id}, {IfdId::canonId, 0x4016}},
    {{Tag::root, IfdId::canonLiOpId}, {IfdId::canonId, 0x4018}},
    {{Tag::root, IfdId::canonLeId}, {IfdId::canonId, 0x4019}},
    {{Tag::root, IfdId::canonAmId}, {IfdId::canonId, 0x4020}},
    {{Tag::root, IfdId::canonMeId}, {IfdId::canonId, 0x4021}},
    {{Tag::root, IfdId::canonFilId}, {IfdId::canonId, 0x4024}},
    {{Tag::root, IfdId::canonHdrId}, {IfdId::canonId, 0x4025}},
    {{Tag::root, IfdId::canonAfCId}, {IfdId::canonId, 0x4028}},
    {{Tag::root, IfdId::canonRawBId}, {IfdId::canonId, 0x403f}},
    {{Tag::root, IfdId::nikon1Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::nikon2Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::nikon3Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::nikonPvId}, {IfdId::nikon3Id, 0x0011}},
    {{Tag::root, IfdId::nikonVrId}, {IfdId::nikon3Id, 0x001f}},
    {{Tag::root, IfdId::nikonPcId}, {IfdId::nikon3Id, 0x0023}},
    {{Tag::root, IfdId::nikonWtId}, {IfdId::nikon3Id, 0x0024}},
    {{Tag::root, IfdId::nikonIiId}, {IfdId::nikon3Id, 0x0025}},
    {{Tag::root, IfdId::nikonAfId}, {IfdId::nikon3Id, 0x0088}},
    {{Tag::root, IfdId::nikonSi1Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonSi2Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonSi3Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonSi4Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonSi5Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonSi6Id}, {IfdId::nikon3Id, 0x0091}},
    {{Tag::root, IfdId::nikonCb1Id}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonCb2Id}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonCb2aId}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonCb2bId}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonCb3Id}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonCb4Id}, {IfdId::nikon3Id, 0x0097}},
    {{Tag::root, IfdId::nikonLd1Id}, {IfdId::nikon3Id, 0x0098}},
    {{Tag::root, IfdId::nikonLd2Id}, {IfdId::nikon3Id, 0x0098}},
    {{Tag::root, IfdId::nikonLd3Id}, {IfdId::nikon3Id, 0x0098}},
    {{Tag::root, IfdId::nikonLd4Id}, {IfdId::nikon3Id, 0x0098}},
    {{Tag::root, IfdId::nikonMeId}, {IfdId::nikon3Id, 0x00b0}},
    {{Tag::root, IfdId::nikonAf21Id}, {IfdId::nikon3Id, 0x00b7}},
    {{Tag::root, IfdId::nikonAf22Id}, {IfdId::nikon3Id, 0x00b7}},
    {{Tag::root, IfdId::nikonFiId}, {IfdId::nikon3Id, 0x00b8}},
    {{Tag::root, IfdId::nikonAFTId}, {IfdId::nikon3Id, 0x00b9}},
    {{Tag::root, IfdId::nikonFl1Id}, {IfdId::nikon3Id, 0x00a8}},
    {{Tag::root, IfdId::nikonFl2Id}, {IfdId::nikon3Id, 0x00a8}},
    {{Tag::root, IfdId::nikonFl3Id}, {IfdId::nikon3Id, 0x00a8}},
    {{Tag::root, IfdId::nikonFl6Id}, {IfdId::nikon3Id, 0x00a8}},
    {{Tag::root, IfdId::nikonFl7Id}, {IfdId::nikon3Id, 0x00a8}},
    {{Tag::root, IfdId::panasonicId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::pentaxId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::pentaxDngId}, {IfdId::ifd0Id, 0xc634}},
    {{Tag::root, IfdId::samsung2Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::samsungPwId}, {IfdId::samsung2Id, 0x0021}},
    {{Tag::root, IfdId::samsungPvId}, {IfdId::samsung2Id, 0x0035}},
    {{Tag::root, IfdId::sigmaId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::sony1Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::sony2010eId}, {IfdId::sony1Id, 0x2010}},
    {{Tag::root, IfdId::sony2FpId}, {IfdId::sony1Id, 0x9402}},
    {{Tag::root, IfdId::sonyMisc1Id}, {IfdId::sony1Id, 0x9403}},
    {{Tag::root, IfdId::sonyMisc2bId}, {IfdId::sony1Id, 0x9404}},
    {{Tag::root, IfdId::sonyMisc3cId}, {IfdId::sony1Id, 0x9400}},
    {{Tag::root, IfdId::sonySInfo1Id}, {IfdId::sony1Id, 0x3000}},
    {{Tag::root, IfdId::sony1CsId}, {IfdId::sony1Id, 0x0114}},
    {{Tag::root, IfdId::sony1Cs2Id}, {IfdId::sony1Id, 0x0114}},
    {{Tag::root, IfdId::sonyMltId}, {IfdId::sony1Id, 0xb028}},
    {{Tag::root, IfdId::sony1MltCsOldId}, {IfdId::sonyMltId, 0x0001}},
    {{Tag::root, IfdId::sony1MltCsNewId}, {IfdId::sonyMltId, 0x0003}},
    {{Tag::root, IfdId::sony1MltCs7DId}, {IfdId::sonyMltId, 0x0004}},
    {{Tag::root, IfdId::sony1MltCsA100Id}, {IfdId::sonyMltId, 0x0114}},
    {{Tag::root, IfdId::sony2Id}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::sony2010eId}, {IfdId::sony2Id, 0x2010}},
    {{Tag::root, IfdId::sony2FpId}, {IfdId::sony2Id, 0x9402}},
    {{Tag::root, IfdId::sonyMisc1Id}, {IfdId::sony2Id, 0x9403}},
    {{Tag::root, IfdId::sonyMisc2bId}, {IfdId::sony2Id, 0x9404}},
    {{Tag::root, IfdId::sonyMisc3cId}, {IfdId::sony2Id, 0x9400}},
    {{Tag::root, IfdId::sonySInfo1Id}, {IfdId::sony2Id, 0x3000}},
    {{Tag::root, IfdId::sony2CsId}, {IfdId::sony2Id, 0x0114}},
    {{Tag::root, IfdId::sony2Cs2Id}, {IfdId::sony2Id, 0x0114}},
    {{Tag::root, IfdId::minoltaId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::minoltaCsOldId}, {IfdId::minoltaId, 0x0001}},
    {{Tag::root, IfdId::minoltaCsNewId}, {IfdId::minoltaId, 0x0003}},
    {{Tag::root, IfdId::minoltaCs7DId}, {IfdId::minoltaId, 0x0004}},
    {{Tag::root, IfdId::minoltaCs5DId}, {IfdId::minoltaId, 0x0114}},
    {{Tag::root, IfdId::casioId}, {IfdId::exifId, 0x927c}},
    {{Tag::root, IfdId::casio2Id}, {IfdId::exifId, 0x927c}},
    // ---------------------------------------------------------
    // Panasonic RW2 raw images
    {{Tag::pana, IfdId::ifdIdNotSet}, {IfdId::ifdIdNotSet, Tag::pana}},
    {{Tag::pana, IfdId::panaRawId}, {IfdId::ifdIdNotSet, Tag::pana}},
    {{Tag::pana, IfdId::exifId}, {IfdId::panaRawId, 0x8769}},
    {{Tag::pana, IfdId::gpsId}, {IfdId::panaRawId, 0x8825}},
};

/*
  This table describes the layout of each known TIFF group (including
  non-standard structures and IFDs only seen in RAW images).

  The key of the table consists of the first two attributes, (extended) tag
  and group. Tag is the TIFF tag or one of a few extended tags, group
  identifies the IFD or any other composite component.

  Each entry of the table defines for a particular tag and group combination
  the corresponding TIFF component create function.
 */
#define ignoreTiffComponent nullptr
const TiffGroupTable TiffCreator::tiffGroupTable_ = {
    // ext. tag  group             create function
    //---------  ----------------- -----------------------------------------
    // Root directory
    {{Tag::root, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::ifd0Id>},

    // Fujifilm RAF #1402.  Use different root when parsing embedded tiff.
    {{Tag::fuji, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::fujiId>},
    {{0xf000, IfdId::fujiId}, &newTiffSubIfd<IfdId::fujiId>},

    // CR3 images #1475
    {{Tag::cmt2, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::exifId>},
    {{Tag::cmt3, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::canonId>},
    {{Tag::cmt4, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::gpsId>},

    // IFD0
    {{0x8769, IfdId::ifd0Id}, &newTiffSubIfd<IfdId::exifId>},
    {{0x8825, IfdId::ifd0Id}, &newTiffSubIfd<IfdId::gpsId>},
    {{0x0111, IfdId::ifd0Id}, &newTiffImageData<0x0117, IfdId::ifd0Id>},
    {{0x0117, IfdId::ifd0Id}, &newTiffImageSize<0x0111, IfdId::ifd0Id>},
    {{0x0144, IfdId::ifd0Id}, &newTiffImageData<0x0145, IfdId::ifd0Id>},
    {{0x0145, IfdId::ifd0Id}, &newTiffImageSize<0x0144, IfdId::ifd0Id>},
    {{0x0201, IfdId::ifd0Id}, &newTiffImageData<0x0202, IfdId::ifd0Id>},
    {{0x0202, IfdId::ifd0Id}, &newTiffImageSize<0x0201, IfdId::ifd0Id>},
    {{0x014a, IfdId::ifd0Id}, &newTiffSubIfd<IfdId::subImage1Id>},
    {{0xc634, IfdId::ifd0Id}, &newTiffMnEntry},
    {{Tag::next, IfdId::ifd0Id}, &newTiffDirectory<IfdId::ifd1Id>},
    {{Tag::all, IfdId::ifd0Id}, &newTiffEntry},

    // Subdir subImage1
    {{0x0111, IfdId::subImage1Id}, &newTiffImageData<0x0117, IfdId::subImage1Id>},
    {{0x0117, IfdId::subImage1Id}, &newTiffImageSize<0x0111, IfdId::subImage1Id>},
    {{0x0144, IfdId::subImage1Id}, &newTiffImageData<0x0145, IfdId::subImage1Id>},
    {{0x0145, IfdId::subImage1Id}, &newTiffImageSize<0x0144, IfdId::subImage1Id>},
    {{0x0201, IfdId::subImage1Id}, &newTiffImageData<0x0202, IfdId::subImage1Id>},
    {{0x0202, IfdId::subImage1Id}, &newTiffImageSize<0x0201, IfdId::subImage1Id>},
    {{Tag::next, IfdId::subImage1Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage1Id}, &newTiffEntry},

    // Subdir subImage2
    {{0x0111, IfdId::subImage2Id}, &newTiffImageData<0x0117, IfdId::subImage2Id>},
    {{0x0117, IfdId::subImage2Id}, &newTiffImageSize<0x0111, IfdId::subImage2Id>},
    {{0x0144, IfdId::subImage2Id}, &newTiffImageData<0x0145, IfdId::subImage2Id>},
    {{0x0145, IfdId::subImage2Id}, &newTiffImageSize<0x0144, IfdId::subImage2Id>},
    {{0x0201, IfdId::subImage2Id}, &newTiffImageData<0x0202, IfdId::subImage2Id>},
    {{0x0202, IfdId::subImage2Id}, &newTiffImageSize<0x0201, IfdId::subImage2Id>},
    {{Tag::next, IfdId::subImage2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage2Id}, &newTiffEntry},

    // Subdir subImage3
    {{0x0111, IfdId::subImage3Id}, &newTiffImageData<0x0117, IfdId::subImage3Id>},
    {{0x0117, IfdId::subImage3Id}, &newTiffImageSize<0x0111, IfdId::subImage3Id>},
    {{0x0144, IfdId::subImage3Id}, &newTiffImageData<0x0145, IfdId::subImage3Id>},
    {{0x0145, IfdId::subImage3Id}, &newTiffImageSize<0x0144, IfdId::subImage3Id>},
    {{0x0201, IfdId::subImage3Id}, &newTiffImageData<0x0202, IfdId::subImage3Id>},
    {{0x0202, IfdId::subImage3Id}, &newTiffImageSize<0x0201, IfdId::subImage3Id>},
    {{Tag::next, IfdId::subImage3Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage3Id}, &newTiffEntry},

    // Subdir subImage4
    {{0x0111, IfdId::subImage4Id}, &newTiffImageData<0x0117, IfdId::subImage4Id>},
    {{0x0117, IfdId::subImage4Id}, &newTiffImageSize<0x0111, IfdId::subImage4Id>},
    {{0x0144, IfdId::subImage4Id}, &newTiffImageData<0x0145, IfdId::subImage4Id>},
    {{0x0145, IfdId::subImage4Id}, &newTiffImageSize<0x0144, IfdId::subImage4Id>},
    {{0x0201, IfdId::subImage4Id}, &newTiffImageData<0x0202, IfdId::subImage4Id>},
    {{0x0202, IfdId::subImage4Id}, &newTiffImageSize<0x0201, IfdId::subImage4Id>},
    {{Tag::next, IfdId::subImage4Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage4Id}, &newTiffEntry},

    // Subdir subImage5
    {{0x0111, IfdId::subImage5Id}, &newTiffImageData<0x0117, IfdId::subImage5Id>},
    {{0x0117, IfdId::subImage5Id}, &newTiffImageSize<0x0111, IfdId::subImage5Id>},
    {{0x0144, IfdId::subImage5Id}, &newTiffImageData<0x0145, IfdId::subImage5Id>},
    {{0x0145, IfdId::subImage5Id}, &newTiffImageSize<0x0144, IfdId::subImage5Id>},
    {{0x0201, IfdId::subImage5Id}, &newTiffImageData<0x0202, IfdId::subImage5Id>},
    {{0x0202, IfdId::subImage5Id}, &newTiffImageSize<0x0201, IfdId::subImage5Id>},
    {{Tag::next, IfdId::subImage5Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage5Id}, &newTiffEntry},

    // Subdir subImage6
    {{0x0111, IfdId::subImage6Id}, &newTiffImageData<0x0117, IfdId::subImage6Id>},
    {{0x0117, IfdId::subImage6Id}, &newTiffImageSize<0x0111, IfdId::subImage6Id>},
    {{0x0144, IfdId::subImage6Id}, &newTiffImageData<0x0145, IfdId::subImage6Id>},
    {{0x0145, IfdId::subImage6Id}, &newTiffImageSize<0x0144, IfdId::subImage6Id>},
    {{0x0201, IfdId::subImage6Id}, &newTiffImageData<0x0202, IfdId::subImage6Id>},
    {{0x0202, IfdId::subImage6Id}, &newTiffImageSize<0x0201, IfdId::subImage6Id>},
    {{Tag::next, IfdId::subImage6Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage6Id}, &newTiffEntry},

    // Subdir subImage7
    {{0x0111, IfdId::subImage7Id}, &newTiffImageData<0x0117, IfdId::subImage7Id>},
    {{0x0117, IfdId::subImage7Id}, &newTiffImageSize<0x0111, IfdId::subImage7Id>},
    {{0x0144, IfdId::subImage7Id}, &newTiffImageData<0x0145, IfdId::subImage7Id>},
    {{0x0145, IfdId::subImage7Id}, &newTiffImageSize<0x0144, IfdId::subImage7Id>},
    {{0x0201, IfdId::subImage7Id}, &newTiffImageData<0x0202, IfdId::subImage7Id>},
    {{0x0202, IfdId::subImage7Id}, &newTiffImageSize<0x0201, IfdId::subImage7Id>},
    {{Tag::next, IfdId::subImage7Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage7Id}, &newTiffEntry},

    // Subdir subImage8
    {{0x0111, IfdId::subImage8Id}, &newTiffImageData<0x0117, IfdId::subImage8Id>},
    {{0x0117, IfdId::subImage8Id}, &newTiffImageSize<0x0111, IfdId::subImage8Id>},
    {{0x0144, IfdId::subImage8Id}, &newTiffImageData<0x0145, IfdId::subImage8Id>},
    {{0x0145, IfdId::subImage8Id}, &newTiffImageSize<0x0144, IfdId::subImage8Id>},
    {{0x0201, IfdId::subImage8Id}, &newTiffImageData<0x0202, IfdId::subImage8Id>},
    {{0x0202, IfdId::subImage8Id}, &newTiffImageSize<0x0201, IfdId::subImage8Id>},
    {{Tag::next, IfdId::subImage8Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage8Id}, &newTiffEntry},

    // Subdir subImage9
    {{0x0111, IfdId::subImage9Id}, &newTiffImageData<0x0117, IfdId::subImage9Id>},
    {{0x0117, IfdId::subImage9Id}, &newTiffImageSize<0x0111, IfdId::subImage9Id>},
    {{0x0144, IfdId::subImage9Id}, &newTiffImageData<0x0145, IfdId::subImage9Id>},
    {{0x0145, IfdId::subImage9Id}, &newTiffImageSize<0x0144, IfdId::subImage9Id>},
    {{0x0201, IfdId::subImage9Id}, &newTiffImageData<0x0202, IfdId::subImage9Id>},
    {{0x0202, IfdId::subImage9Id}, &newTiffImageSize<0x0201, IfdId::subImage9Id>},
    {{Tag::next, IfdId::subImage9Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subImage9Id}, &newTiffEntry},

    // Exif subdir
    {{0xa005, IfdId::exifId}, &newTiffSubIfd<IfdId::iopId>},
    {{0x927c, IfdId::exifId}, &newTiffMnEntry},
    {{Tag::next, IfdId::exifId}, ignoreTiffComponent},
    {{Tag::all, IfdId::exifId}, &newTiffEntry},

    // GPS subdir
    {{Tag::next, IfdId::gpsId}, ignoreTiffComponent},
    {{Tag::all, IfdId::gpsId}, &newTiffEntry},

    // IOP subdir
    {{Tag::next, IfdId::iopId}, ignoreTiffComponent},
    {{Tag::all, IfdId::iopId}, &newTiffEntry},

    // IFD1
    {{0x0111, IfdId::ifd1Id}, &newTiffThumbData<0x0117, IfdId::ifd1Id>},
    {{0x0117, IfdId::ifd1Id}, &newTiffThumbSize<0x0111, IfdId::ifd1Id>},
    {{0x0144, IfdId::ifd1Id}, &newTiffImageData<0x0145, IfdId::ifd1Id>},
    {{0x0145, IfdId::ifd1Id}, &newTiffImageSize<0x0144, IfdId::ifd1Id>},
    {{0x014a, IfdId::ifd1Id}, &newTiffSubIfd<IfdId::subThumb1Id>},
    {{0x0201, IfdId::ifd1Id}, &newTiffThumbData<0x0202, IfdId::ifd1Id>},
    {{0x0202, IfdId::ifd1Id}, &newTiffThumbSize<0x0201, IfdId::ifd1Id>},
    {{Tag::next, IfdId::ifd1Id}, &newTiffDirectory<IfdId::ifd2Id>},
    {{Tag::all, IfdId::ifd1Id}, &newTiffEntry},

    // Subdir subThumb1
    {{0x0111, IfdId::subThumb1Id}, &newTiffImageData<0x0117, IfdId::subThumb1Id>},
    {{0x0117, IfdId::subThumb1Id}, &newTiffImageSize<0x0111, IfdId::subThumb1Id>},
    {{0x0144, IfdId::subThumb1Id}, &newTiffImageData<0x0145, IfdId::subThumb1Id>},
    {{0x0145, IfdId::subThumb1Id}, &newTiffImageSize<0x0144, IfdId::subThumb1Id>},
    {{0x0201, IfdId::subThumb1Id}, &newTiffImageData<0x0202, IfdId::subThumb1Id>},
    {{0x0202, IfdId::subThumb1Id}, &newTiffImageSize<0x0201, IfdId::subThumb1Id>},
    {{Tag::next, IfdId::subThumb1Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::subThumb1Id}, &newTiffEntry},

    // IFD2 (eg, in Pentax PEF and Canon CR2 files)
    {{0x0111, IfdId::ifd2Id}, &newTiffImageData<0x0117, IfdId::ifd2Id>},
    {{0x0117, IfdId::ifd2Id}, &newTiffImageSize<0x0111, IfdId::ifd2Id>},
    {{0x0144, IfdId::ifd1Id}, &newTiffImageData<0x0145, IfdId::ifd2Id>},
    {{0x0145, IfdId::ifd1Id}, &newTiffImageSize<0x0144, IfdId::ifd2Id>},
    {{0x0201, IfdId::ifd2Id}, &newTiffImageData<0x0202, IfdId::ifd2Id>},
    {{0x0202, IfdId::ifd2Id}, &newTiffImageSize<0x0201, IfdId::ifd2Id>},
    {{Tag::next, IfdId::ifd2Id}, &newTiffDirectory<IfdId::ifd3Id>},
    {{Tag::all, IfdId::ifd2Id}, &newTiffEntry},

    // IFD3 (eg, in Canon CR2 files)
    {{0x0111, IfdId::ifd3Id}, &newTiffImageData<0x0117, IfdId::ifd3Id>},
    {{0x0117, IfdId::ifd3Id}, &newTiffImageSize<0x0111, IfdId::ifd3Id>},
    {{0x0144, IfdId::ifd1Id}, &newTiffImageData<0x0145, IfdId::ifd3Id>},
    {{0x0145, IfdId::ifd1Id}, &newTiffImageSize<0x0144, IfdId::ifd3Id>},
    {{0x0201, IfdId::ifd3Id}, &newTiffImageData<0x0202, IfdId::ifd3Id>},
    {{0x0202, IfdId::ifd3Id}, &newTiffImageSize<0x0201, IfdId::ifd3Id>},
    {{Tag::next, IfdId::ifd3Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::ifd3Id}, &newTiffEntry},

    // Olympus makernote - some Olympus cameras use Minolta structures
    // Todo: Adding such tags will not work (maybe result in a Minolta makernote), need separate groups
    {{0x0001, IfdId::olympusId}, EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)},
    {{0x0003, IfdId::olympusId}, EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)},
    {{Tag::next, IfdId::olympusId}, ignoreTiffComponent},
    {{Tag::all, IfdId::olympusId}, &newTiffEntry},

    // Olympus2 makernote
    {{0x0001, IfdId::olympus2Id}, EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)},
    {{0x0003, IfdId::olympus2Id}, EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)},
    {{0x2010, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusEqId>},
    {{0x2020, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusCsId>},
    {{0x2030, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusRdId>},
    {{0x2031, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusRd2Id>},
    {{0x2040, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusIpId>},
    {{0x2050, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFiId>},
    {{0x2100, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe1Id>},
    {{0x2200, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe2Id>},
    {{0x2300, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe3Id>},
    {{0x2400, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe4Id>},
    {{0x2500, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe5Id>},
    {{0x2600, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe6Id>},
    {{0x2700, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe7Id>},
    {{0x2800, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe8Id>},
    {{0x2900, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusFe9Id>},
    {{0x3000, IfdId::olympus2Id}, &newTiffSubIfd<IfdId::olympusRiId>},
    {{Tag::next, IfdId::olympus2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::olympus2Id}, &newTiffEntry},

    // Olympus2 equipment subdir
    {{Tag::all, IfdId::olympusEqId}, &newTiffEntry},

    // Olympus2 camera settings subdir
    {{0x0101, IfdId::olympusCsId}, &newTiffImageData<0x0102, IfdId::olympusCsId>},
    {{0x0102, IfdId::olympusCsId}, &newTiffImageSize<0x0101, IfdId::olympusCsId>},
    {{Tag::all, IfdId::olympusCsId}, &newTiffEntry},

    // Olympus2 raw development subdir
    {{Tag::all, IfdId::olympusRdId}, &newTiffEntry},

    // Olympus2 raw development 2 subdir
    {{Tag::all, IfdId::olympusRd2Id}, &newTiffEntry},

    // Olympus2 image processing subdir
    {{Tag::all, IfdId::olympusIpId}, &newTiffEntry},

    // Olympus2 focus info subdir
    {{Tag::all, IfdId::olympusFiId}, &newTiffEntry},

    // Olympus2 FE 1 subdir
    {{Tag::all, IfdId::olympusFe1Id}, &newTiffEntry},

    // Olympus2 FE 2 subdir
    {{Tag::all, IfdId::olympusFe2Id}, &newTiffEntry},

    // Olympus2 FE 3 subdir
    {{Tag::all, IfdId::olympusFe3Id}, &newTiffEntry},

    // Olympus2 FE 4 subdir
    {{Tag::all, IfdId::olympusFe4Id}, &newTiffEntry},

    // Olympus2 FE 5 subdir
    {{Tag::all, IfdId::olympusFe5Id}, &newTiffEntry},

    // Olympus2 FE 6 subdir
    {{Tag::all, IfdId::olympusFe6Id}, &newTiffEntry},

    // Olympus2 FE 7 subdir
    {{Tag::all, IfdId::olympusFe7Id}, &newTiffEntry},

    // Olympus2 FE 8 subdir
    {{Tag::all, IfdId::olympusFe8Id}, &newTiffEntry},

    // Olympus2 FE 9 subdir
    {{Tag::all, IfdId::olympusFe9Id}, &newTiffEntry},

    // Olympus2 Raw Info subdir
    {{Tag::all, IfdId::olympusRiId}, &newTiffEntry},

    // Fujifilm makernote
    {{Tag::next, IfdId::fujiId}, ignoreTiffComponent},
    {{Tag::all, IfdId::fujiId}, &newTiffEntry},

    // Canon makernote
    {{0x0001, IfdId::canonId}, EXV_BINARY_ARRAY(canonCsCfg, canonCsDef)},
    {{0x0004, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonSiCfg)},
    {{0x0005, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonPaCfg)},
    {{0x000f, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonCfCfg)},
    {{0x0012, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonPiCfg)},
    {{0x0035, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonTiCfg)},
    {{0x0093, IfdId::canonId}, EXV_BINARY_ARRAY(canonFiCfg, canonFiDef)},
    {{0x00a0, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonPrCfg)},
    {{0x4013, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonAfMiAdjCfg)},
    //  {{    0x4015, IfdId::canonId,          EXV_SIMPLE_BINARY_ARRAY(canonVigCorCfg)   },
    {{0x4016, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonVigCor2Cfg)},
    {{0x4018, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonLiOpCfg)},
    {{0x4019, IfdId::canonId}, EXV_BINARY_ARRAY(canonLeCfg, canonLeDef)},
    {{0x4020, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonAmCfg)},
    {{0x4021, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonMeCfg)},
    {{0x4024, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonFilCfg)},
    {{0x4025, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonHdrCfg)},
    {{0x4028, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonAfCCfg)},
    {{0x403f, IfdId::canonId}, EXV_SIMPLE_BINARY_ARRAY(canonRawBCfg)},
    {{Tag::next, IfdId::canonId}, ignoreTiffComponent},
    {{Tag::all, IfdId::canonId}, &newTiffEntry},

    // Canon makernote composite tags
    {{Tag::all, IfdId::canonCsId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonSiId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonPaId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonCfId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonPiId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonTiId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonFiId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonPrId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonAfMiAdjId}, &newTiffBinaryElement},
    // {{  Tag::all, IfdId::canonVigCorId,    newTiffBinaryElement                      },
    {{Tag::all, IfdId::canonVigCor2Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonLiOpId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonLeId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonAmId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonMeId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonFilId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonHdrId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonAfCId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::canonRawBId}, &newTiffBinaryElement},

    // Nikon1 makernote
    {{Tag::next, IfdId::nikon1Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::nikon1Id}, &newTiffEntry},

    // Nikon2 makernote
    {{Tag::next, IfdId::nikon2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::nikon2Id}, &newTiffEntry},

    // Nikon3 makernote
    {{Tag::next, IfdId::nikon3Id}, ignoreTiffComponent},
    {{0x0011, IfdId::nikon3Id}, &newTiffSubIfd<IfdId::nikonPvId>},
    {{0x001f, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonVrCfg, nikonVrDef)},
    {{0x0023, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonPcCfg, nikonPcDef)},
    {{0x0024, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonWtCfg, nikonWtDef)},
    {{0x0025, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonIiCfg, nikonIiDef)},
    {{0x0088, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonAfCfg, nikonAfDef)},
    {{0x0091, IfdId::nikon3Id}, EXV_COMPLEX_BINARY_ARRAY(nikonSiSet, &nikonSelector)},
    {{0x0097, IfdId::nikon3Id}, EXV_COMPLEX_BINARY_ARRAY(nikonCbSet, &nikonSelector)},
    {{0x0098, IfdId::nikon3Id}, EXV_COMPLEX_BINARY_ARRAY(nikonLdSet, &nikonSelector)},
    {{0x00a8, IfdId::nikon3Id}, EXV_COMPLEX_BINARY_ARRAY(nikonFlSet, &nikonSelector)},
    {{0x00b0, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonMeCfg, nikonMeDef)},
    {{0x00b7, IfdId::nikon3Id}, EXV_COMPLEX_BINARY_ARRAY(nikonAf2Set, &nikonSelector)},
    {{0x00b8, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonFiCfg, nikonFiDef)},
    {{0x00b9, IfdId::nikon3Id}, EXV_BINARY_ARRAY(nikonAFTCfg, nikonAFTDef)},
    {{Tag::all, IfdId::nikon3Id}, &newTiffEntry},

    // Nikon3 makernote preview subdir
    {{0x0201, IfdId::nikonPvId}, &newTiffThumbData<0x0202, IfdId::nikonPvId>},
    {{0x0202, IfdId::nikonPvId}, &newTiffThumbSize<0x0201, IfdId::nikonPvId>},
    {{Tag::next, IfdId::nikonPvId}, ignoreTiffComponent},
    {{Tag::all, IfdId::nikonPvId}, &newTiffEntry},

    // Nikon3 vibration reduction
    {{Tag::all, IfdId::nikonVrId}, &newTiffBinaryElement},

    // Nikon3 picture control
    {{Tag::all, IfdId::nikonPcId}, &newTiffBinaryElement},

    // Nikon3 world time
    {{Tag::all, IfdId::nikonWtId}, &newTiffBinaryElement},

    // Nikon3 ISO info
    {{Tag::all, IfdId::nikonIiId}, &newTiffBinaryElement},

    // Nikon3 auto focus
    {{Tag::all, IfdId::nikonAfId}, &newTiffBinaryElement},

    // Nikon3 auto focus 2
    {{Tag::all, IfdId::nikonAf21Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonAf22Id}, &newTiffBinaryElement},

    // Nikon3 AF Fine Tune
    {{Tag::all, IfdId::nikonAFTId}, &newTiffBinaryElement},

    // Nikon3 file info
    {{Tag::all, IfdId::nikonFiId}, &newTiffBinaryElement},

    // Nikon3 multi exposure
    {{Tag::all, IfdId::nikonMeId}, &newTiffBinaryElement},

    // Nikon3 flash info
    {{Tag::all, IfdId::nikonFl1Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonFl2Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonFl3Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonFl6Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonFl7Id}, &newTiffBinaryElement},

    // Nikon3 shot info
    {{Tag::all, IfdId::nikonSi1Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonSi2Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonSi3Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonSi4Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonSi5Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonSi6Id}, &newTiffBinaryElement},

    // Nikon3 color balance
    {{Tag::all, IfdId::nikonCb1Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonCb2Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonCb2aId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonCb2bId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonCb3Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonCb4Id}, &newTiffBinaryElement},

    // Nikon3 lens data
    {{Tag::all, IfdId::nikonLd1Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonLd2Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonLd3Id}, &newTiffBinaryElement},
    {{Tag::all, IfdId::nikonLd4Id}, &newTiffBinaryElement},

    // Panasonic makernote
    {{Tag::next, IfdId::panasonicId}, ignoreTiffComponent},
    {{Tag::all, IfdId::panasonicId}, &newTiffEntry},

    // Pentax DNG makernote
    {{0x0003, IfdId::pentaxDngId}, &newTiffThumbSize<0x0004, IfdId::pentaxDngId>},
    {{0x0004, IfdId::pentaxDngId}, &newTiffThumbData<0x0003, IfdId::pentaxDngId>},
    {{Tag::next, IfdId::pentaxDngId}, ignoreTiffComponent},
    {{Tag::all, IfdId::pentaxDngId}, &newTiffEntry},

    // Pentax makernote
    {{0x0003, IfdId::pentaxId}, &newTiffThumbSize<0x0004, IfdId::pentaxId>},
    {{0x0004, IfdId::pentaxId}, &newTiffThumbData<0x0003, IfdId::pentaxId>},
    {{Tag::next, IfdId::pentaxId}, ignoreTiffComponent},
    {{Tag::all, IfdId::pentaxId}, &newTiffEntry},

    // Samsung2 makernote
    {{0x0021, IfdId::samsung2Id}, EXV_BINARY_ARRAY(samsungPwCfg, samsungPwDef)},
    {{0x0035, IfdId::samsung2Id}, &newTiffSubIfd<IfdId::samsungPvId>},
    {{Tag::next, IfdId::samsung2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::samsung2Id}, &newTiffEntry},

    // Samsung PictureWizard binary array
    {{Tag::all, IfdId::samsungPwId}, &newTiffBinaryElement},

    // Samsung2 makernote preview subdir
    {{0x0201, IfdId::samsungPvId}, &newTiffThumbData<0x0202, IfdId::samsungPvId>},
    {{0x0202, IfdId::samsungPvId}, &newTiffThumbSize<0x0201, IfdId::samsungPvId>},
    {{Tag::next, IfdId::samsungPvId}, ignoreTiffComponent},
    {{Tag::all, IfdId::samsungPvId}, &newTiffEntry},

    // Sigma/Foveon makernote
    {{Tag::next, IfdId::sigmaId}, ignoreTiffComponent},
    {{Tag::all, IfdId::sigmaId}, &newTiffEntry},

    {{Tag::all, IfdId::sony2010eId}, &newTiffBinaryElement},
    {{0x2010, IfdId::sony1Id}, EXV_COMPLEX_BINARY_ARRAY(sony2010eSet, &sony2010eSelector)},

    // Tag 0x9402 Sony2Fp Focus Position
    {{Tag::all, IfdId::sony2FpId}, &newTiffBinaryElement},
    {{0x9402, IfdId::sony1Id}, EXV_COMPLEX_BINARY_ARRAY(sony2FpSet, &sony2FpSelector)},

    // Tag 0x9404 SonyMisc2b
    {{Tag::all, IfdId::sonyMisc2bId}, &newTiffBinaryElement},
    {{0x9404, IfdId::sony1Id}, EXV_COMPLEX_BINARY_ARRAY(sonyMisc2bSet, &sonyMisc2bSelector)},

    // Tag 0x9400 SonyMisc3c
    {{Tag::all, IfdId::sonyMisc3cId}, &newTiffBinaryElement},
    {{0x9400, IfdId::sony1Id}, EXV_COMPLEX_BINARY_ARRAY(sonyMisc3cSet, &sonyMisc3cSelector)},

    // Tag 0x9403 SonyMisc1
    {{Tag::all, IfdId::sonyMisc1Id}, &newTiffBinaryElement},
    {{0x9403, IfdId::sony1Id}, EXV_BINARY_ARRAY(sonyMisc1Cfg, sonyMisc1Def)},

    // Tag 0x3000 SonySInfo1
    {{Tag::all, IfdId::sonySInfo1Id}, &newTiffBinaryElement},
    {{0x3000, IfdId::sony1Id}, EXV_BINARY_ARRAY(sonySInfo1Cfg, sonySInfo1Def)},

    // Sony1 makernote
    {{0x0114, IfdId::sony1Id}, EXV_COMPLEX_BINARY_ARRAY(sony1CsSet, &sonyCsSelector)},
    {{0xb028, IfdId::sony1Id}, &newTiffSubIfd<IfdId::sonyMltId>},
    {{Tag::next, IfdId::sony1Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::sony1Id}, &newTiffEntry},

    // Sony1 camera settings
    {{Tag::all, IfdId::sony1CsId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::sony1Cs2Id}, &newTiffBinaryElement},

    {{Tag::all, IfdId::sony2010eId}, &newTiffBinaryElement},
    {{0x2010, IfdId::sony2Id}, EXV_COMPLEX_BINARY_ARRAY(sony2010eSet, &sony2010eSelector)},

    // Tag 0x9402 Sony2Fp Focus Position
    {{Tag::all, IfdId::sony2FpId}, &newTiffBinaryElement},
    {{0x9402, IfdId::sony2Id}, EXV_COMPLEX_BINARY_ARRAY(sony2FpSet, &sony2FpSelector)},

    // Tag 0x9403 SonyMisc1
    {{Tag::all, IfdId::sonyMisc1Id}, &newTiffBinaryElement},
    {{0x9403, IfdId::sony2Id}, EXV_BINARY_ARRAY(sonyMisc1Cfg, sonyMisc1Def)},

    // Tag 0x9404 SonyMisc2b
    {{Tag::all, IfdId::sonyMisc2bId}, &newTiffBinaryElement},
    {{0x9404, IfdId::sony2Id}, EXV_COMPLEX_BINARY_ARRAY(sonyMisc2bSet, &sonyMisc2bSelector)},

    // Tag 0x9400 SonyMisc3c
    {{Tag::all, IfdId::sonyMisc3cId}, &newTiffBinaryElement},
    {{0x9400, IfdId::sony2Id}, EXV_COMPLEX_BINARY_ARRAY(sonyMisc3cSet, &sonyMisc3cSelector)},

    // Tag 0x3000 SonySInfo1
    {{Tag::all, IfdId::sonySInfo1Id}, &newTiffBinaryElement},
    {{0x3000, IfdId::sony2Id}, EXV_BINARY_ARRAY(sonySInfo1Cfg, sonySInfo1Def)},

    // Sony2 makernote
    {{0x0114, IfdId::sony2Id}, EXV_COMPLEX_BINARY_ARRAY(sony2CsSet, &sonyCsSelector)},
    {{Tag::next, IfdId::sony2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::sony2Id}, &newTiffEntry},

    // Sony2 camera settings
    {{Tag::all, IfdId::sony2CsId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::sony2Cs2Id}, &newTiffBinaryElement},

    // Sony1 Minolta makernote
    {{0x0001, IfdId::sonyMltId}, EXV_SIMPLE_BINARY_ARRAY(sony1MCsoCfg)},
    {{0x0003, IfdId::sonyMltId}, EXV_SIMPLE_BINARY_ARRAY(sony1MCsnCfg)},
    {{0x0004, IfdId::sonyMltId}, EXV_BINARY_ARRAY(sony1MCs7Cfg, minoCs7Def)},  // minoCs7Def [sic]
    {{0x0088, IfdId::sonyMltId}, &newTiffThumbData<0x0089, IfdId::sonyMltId>},
    {{0x0089, IfdId::sonyMltId}, &newTiffThumbSize<0x0088, IfdId::sonyMltId>},
    {{0x0114, IfdId::sonyMltId}, EXV_BINARY_ARRAY(sony1MCsA100Cfg, sony1MCsA100Def)},
    {{Tag::next, IfdId::sonyMltId}, ignoreTiffComponent},
    {{Tag::all, IfdId::sonyMltId}, &newTiffEntry},

    // Sony1 Minolta makernote composite tags
    {{Tag::all, IfdId::sony1MltCsOldId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::sony1MltCsNewId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::sony1MltCs7DId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::sony1MltCsA100Id}, &newTiffBinaryElement},

    // Minolta makernote
    {{0x0001, IfdId::minoltaId}, EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)},
    {{0x0003, IfdId::minoltaId}, EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)},
    {{0x0004, IfdId::minoltaId}, EXV_BINARY_ARRAY(minoCs7Cfg, minoCs7Def)},
    {{0x0088, IfdId::minoltaId}, &newTiffThumbData<0x0089, IfdId::minoltaId>},
    {{0x0089, IfdId::minoltaId}, &newTiffThumbSize<0x0088, IfdId::minoltaId>},
    {{0x0114, IfdId::minoltaId}, EXV_BINARY_ARRAY(minoCs5Cfg, minoCs5Def)},
    {{Tag::next, IfdId::minoltaId}, ignoreTiffComponent},
    {{Tag::all, IfdId::minoltaId}, &newTiffEntry},

    // Minolta makernote composite tags
    {{Tag::all, IfdId::minoltaCsOldId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::minoltaCsNewId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::minoltaCs7DId}, &newTiffBinaryElement},
    {{Tag::all, IfdId::minoltaCs5DId}, &newTiffBinaryElement},

    // -----------------------------------------------------------------------
    // Root directory of Panasonic RAW images
    {{Tag::pana, IfdId::ifdIdNotSet}, &newTiffDirectory<IfdId::panaRawId>},

    // IFD0 of Panasonic RAW images
    {{0x8769, IfdId::panaRawId}, &newTiffSubIfd<IfdId::exifId>},
    {{0x8825, IfdId::panaRawId}, &newTiffSubIfd<IfdId::gpsId>},
    //        {{    0x0111, IfdId::panaRawId,        newTiffImageData<0x0117, IfdId::panaRawId>       },
    //        {{    0x0117, IfdId::panaRawId,        newTiffImageSize<0x0111, IfdId::panaRawId>       },
    {{Tag::next, IfdId::panaRawId}, ignoreTiffComponent},
    {{Tag::all, IfdId::panaRawId}, &newTiffEntry},

    // Casio makernote
    {{Tag::next, IfdId::casioId}, ignoreTiffComponent},
    {{Tag::all, IfdId::casioId}, &newTiffEntry},

    // Casio2 makernote
    {{Tag::next, IfdId::casio2Id}, ignoreTiffComponent},
    {{Tag::all, IfdId::casio2Id}, &newTiffEntry},

    // -----------------------------------------------------------------------
    // Tags which are not de/encoded
    {{Tag::next, IfdId::ignoreId}, ignoreTiffComponent},
    {{Tag::all, IfdId::ignoreId}, &newTiffEntry},
};

// TIFF mapping table for special decoding and encoding requirements
const TiffMappingInfo TiffMapping::tiffMappingInfo_[] = {
    {"*", Tag::all, IfdId::ignoreId, nullptr, nullptr},  // Do not decode tags with group == IfdId::ignoreId
    {"*", 0x02bc, IfdId::ifd0Id, &TiffDecoder::decodeXmp, nullptr /*done before the tree is traversed*/},
    {"*", 0x83bb, IfdId::ifd0Id, &TiffDecoder::decodeIptc, nullptr /*done before the tree is traversed*/},
    {"*", 0x8649, IfdId::ifd0Id, &TiffDecoder::decodeIptc, nullptr /*done before the tree is traversed*/},
    {"*", 0x0026, IfdId::canonId, &TiffDecoder::decodeCanonAFInfo, nullptr /* Exiv2.Canon.AFInfo is read-only */},
};

DecoderFct TiffMapping::findDecoder(const std::string& make, uint32_t extendedTag, IfdId group) {
  DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
  if (auto td = Exiv2::find(tiffMappingInfo_, TiffMappingInfo::Key{make, extendedTag, group})) {
    // This may set decoderFct to 0, meaning that the tag should not be decoded
    decoderFct = td->decoderFct_;
  }
  return decoderFct;
}

EncoderFct TiffMapping::findEncoder(const std::string& make, uint32_t extendedTag, IfdId group) {
  EncoderFct encoderFct = nullptr;
  if (auto td = Exiv2::find(tiffMappingInfo_, TiffMappingInfo::Key{make, extendedTag, group})) {
    // Returns 0 if no special encoder function is found
    encoderFct = td->encoderFct_;
  }
  return encoderFct;
}

TiffComponent::UniquePtr TiffCreator::create(uint32_t extendedTag, IfdId group) {
  auto tag = static_cast<uint16_t>(extendedTag & 0xffff);
  auto i = tiffGroupTable_.find(TiffGroupKey(extendedTag, group));
  // If the lookup failed then try again with Tag::all.
  if (i == tiffGroupTable_.end()) {
    i = tiffGroupTable_.find(TiffGroupKey(Tag::all, group));
  }
  if (i != tiffGroupTable_.end() && i->second) {
    return i->second(tag, group);
  }
#ifdef EXIV2_DEBUG_MESSAGES
  if (i == tiffGroupTable_.end())
    std::cerr << "Warning: No TIFF structure entry found for ";
  else
    std::cerr << "Warning: No TIFF component creator found for ";
  std::cerr << "extended tag 0x" << std::setw(4) << std::setfill('0') << std::hex << std::right << extendedTag
            << ", group " << groupName(group) << "\n";
#endif
  return nullptr;
}  // TiffCreator::create

TiffPath TiffCreator::getPath(uint32_t extendedTag, IfdId group, uint32_t root) {
  TiffPath ret;
  while (true) {
    ret.emplace(extendedTag, group);
    const auto ts = tiffTreeTable_.find(TiffGroupKey(root, group));
    assert(ts != tiffTreeTable_.end());
    extendedTag = ts->second.second;
    group = ts->second.first;
    if (ts->first == TiffGroupKey(root, IfdId::ifdIdNotSet)) {
      break;
    }
  }
  return ret;
}

ByteOrder TiffParserWorker::decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData,
                                   size_t size, uint32_t root, FindDecoderFct findDecoderFct, TiffHeaderBase* pHeader) {
  // Create standard TIFF header if necessary
  std::unique_ptr<TiffHeaderBase> ph;
  if (!pHeader) {
    ph = std::make_unique<TiffHeader>();
    pHeader = ph.get();
  }

  if (auto rootDir = parse(pData, size, root, pHeader)) {
    auto decoder = TiffDecoder(exifData, iptcData, xmpData, rootDir.get(), findDecoderFct);
    rootDir->accept(decoder);
  }
  return pHeader->byteOrder();

}  // TiffParserWorker::decode

WriteMethod TiffParserWorker::encode(BasicIo& io, const byte* pData, size_t size, ExifData& exifData,
                                     IptcData& iptcData, XmpData& xmpData, uint32_t root, FindEncoderFct findEncoderFct,
                                     TiffHeaderBase* pHeader, OffsetWriter* pOffsetWriter) {
  /*
     1) parse the binary image, if one is provided, and
     2) attempt updating the parsed tree in-place ("non-intrusive writing")
     3) else, create a new tree and write a new TIFF structure ("intrusive
        writing"). If there is a parsed tree, it is only used to access the
        image data in this case.
   */
  WriteMethod writeMethod = wmIntrusive;
  auto parsedTree = parse(pData, size, root, pHeader);
  auto primaryGroups = findPrimaryGroups(parsedTree.get());
  if (parsedTree) {
    // Attempt to update existing TIFF components based on metadata entries
    TiffEncoder encoder(exifData, iptcData, xmpData, parsedTree.get(), false, &primaryGroups, pHeader, findEncoderFct);
    parsedTree->accept(encoder);
    if (!encoder.dirty())
      writeMethod = wmNonIntrusive;
  }
  if (writeMethod == wmIntrusive) {
    auto createdTree = TiffCreator::create(root, IfdId::ifdIdNotSet);
    if (parsedTree) {
      // Copy image tags from the original image to the composite
      TiffCopier copier(createdTree.get(), root, pHeader, &primaryGroups);
      parsedTree->accept(copier);
    }
    // Add entries from metadata to composite
    TiffEncoder encoder(exifData, iptcData, xmpData, createdTree.get(), !parsedTree, &primaryGroups, pHeader,
                        findEncoderFct);
    encoder.add(createdTree.get(), parsedTree.get(), root);
    // Write binary representation from the composite tree
    DataBuf header = pHeader->write();
    auto tempIo = MemIo();
    IoWrapper ioWrapper(tempIo, header.c_data(), header.size(), pOffsetWriter);
    auto imageIdx(std::string::npos);
    createdTree->write(ioWrapper, pHeader->byteOrder(), header.size(), std::string::npos, std::string::npos, imageIdx);
    if (pOffsetWriter)
      pOffsetWriter->writeOffsets(tempIo);
    io.transfer(tempIo);  // may throw
#ifndef SUPPRESS_WARNINGS
    EXV_INFO << "Write strategy: Intrusive\n";
#endif
  }
#ifndef SUPPRESS_WARNINGS
  else {
    EXV_INFO << "Write strategy: Non-intrusive\n";
  }
#endif
  return writeMethod;
}  // TiffParserWorker::encode

TiffComponent::UniquePtr TiffParserWorker::parse(const byte* pData, size_t size, uint32_t root,
                                                 TiffHeaderBase* pHeader) {
  TiffComponent::UniquePtr rootDir;
  if (!pData || size == 0)
    return rootDir;
  if (!pHeader->read(pData, size) || pHeader->offset() >= size) {
    throw Error(ErrorCode::kerNotAnImage, "TIFF");
  }
  rootDir = TiffCreator::create(root, IfdId::ifdIdNotSet);
  if (rootDir) {
    rootDir->setStart(pData + pHeader->offset());
    auto state = TiffRwState{pHeader->byteOrder(), 0};
    auto reader = TiffReader{pData, size, rootDir.get(), state};
    rootDir->accept(reader);
    reader.postProcess();
  }
  return rootDir;

}  // TiffParserWorker::parse

PrimaryGroups TiffParserWorker::findPrimaryGroups(TiffComponent* pSourceDir) {
  PrimaryGroups ret;
  if (!pSourceDir)
    return ret;

  static constexpr auto imageGroups = std::array{
      IfdId::ifd0Id,      IfdId::ifd1Id,      IfdId::ifd2Id,      IfdId::ifd3Id,      IfdId::subImage1Id,
      IfdId::subImage2Id, IfdId::subImage3Id, IfdId::subImage4Id, IfdId::subImage5Id, IfdId::subImage6Id,
      IfdId::subImage7Id, IfdId::subImage8Id, IfdId::subImage9Id,
  };

  for (auto imageGroup : imageGroups) {
    TiffFinder finder(0x00fe, imageGroup);
    pSourceDir->accept(finder);
    auto te = dynamic_cast<const TiffEntryBase*>(finder.result());
    const Value* pV = te ? te->pValue() : nullptr;
    if (pV && pV->typeId() == unsignedLong && pV->count() == 1 && (pV->toInt64() & 1) == 0) {
      ret.push_back(te->group());
    }
  }
  return ret;
}  // TiffParserWorker::findPrimaryGroups

TiffHeaderBase::TiffHeaderBase(uint16_t tag, uint32_t size, ByteOrder byteOrder, uint32_t offset) :
    tag_(tag), size_(size), byteOrder_(byteOrder), offset_(offset) {
}

bool TiffHeaderBase::read(const byte* pData, size_t size) {
  if (!pData || size < 8)
    return false;

  if (pData[0] == 'I' && pData[0] == pData[1]) {
    byteOrder_ = littleEndian;
  } else if (pData[0] == 'M' && pData[0] == pData[1]) {
    byteOrder_ = bigEndian;
  } else {
    return false;
  }
  uint16_t t = getUShort(pData + 2, byteOrder_);
  if (t != 444 && t != 17234 && tag_ != t)
    return false;  // 444 is for the JPEG-XR; 17234 is for DCP
  tag_ = t;
  offset_ = getULong(pData + 4, byteOrder_);

  return true;
}

DataBuf TiffHeaderBase::write() const {
  DataBuf buf(8);
  switch (byteOrder_) {
    case littleEndian:
      buf.write_uint8(0, 'I');
      break;
    case bigEndian:
      buf.write_uint8(0, 'M');
      break;
    case invalidByteOrder:
      break;
  }
  buf.write_uint8(1, buf.read_uint8(0));
  buf.write_uint16(2, tag_, byteOrder_);
  buf.write_uint32(4, 0x00000008, byteOrder_);
  return buf;
}

void TiffHeaderBase::print(std::ostream& os, const std::string& prefix) const {
  std::ios::fmtflags f(os.flags());
  os << prefix << _("TIFF header, offset") << " = 0x" << std::setw(8) << std::setfill('0') << std::hex << std::right
     << offset_;

  switch (byteOrder_) {
    case littleEndian:
      os << ", " << _("little endian encoded");
      break;
    case bigEndian:
      os << ", " << _("big endian encoded");
      break;
    case invalidByteOrder:
      break;
  }
  os << "\n";
  os.flags(f);
}  // TiffHeaderBase::print

ByteOrder TiffHeaderBase::byteOrder() const {
  return byteOrder_;
}

void TiffHeaderBase::setByteOrder(ByteOrder byteOrder) {
  byteOrder_ = byteOrder;
}

uint32_t TiffHeaderBase::offset() const {
  return offset_;
}

void TiffHeaderBase::setOffset(uint32_t offset) {
  offset_ = offset;
}

uint32_t TiffHeaderBase::size() const {
  return size_;
}

uint16_t TiffHeaderBase::tag() const {
  return tag_;
}

bool TiffHeaderBase::isImageTag(uint16_t /*tag*/, IfdId /*group*/, const PrimaryGroups* /*primaryGroups*/) const {
  return false;
}

static bool isTiffImageTagLookup(uint16_t tag, IfdId group) {
  if (group != IfdId::ifd0Id) {
    return false;
  }
  //! List of TIFF image tags
  switch (tag) {
    case 0x00fe:  // Exif.Image.NewSubfileType
    case 0x00ff:  // Exif.Image.SubfileType
    case 0x0100:  // Exif.Image.ImageWidth
    case 0x0101:  // Exif.Image.ImageLength
    case 0x0102:  // Exif.Image.BitsPerSample
    case 0x0103:  // Exif.Image.Compression
    case 0x0106:  // Exif.Image.PhotometricInterpretation
    case 0x010a:  // Exif.Image.FillOrder
    case 0x0111:  // Exif.Image.StripOffsets
    case 0x0115:  // Exif.Image.SamplesPerPixel
    case 0x0116:  // Exif.Image.RowsPerStrip
    case 0x0117:  // Exif.Image.StripByteCounts
    case 0x011a:  // Exif.Image.XResolution
    case 0x011b:  // Exif.Image.YResolution
    case 0x011c:  // Exif.Image.PlanarConfiguration
    case 0x0122:  // Exif.Image.GrayResponseUnit
    case 0x0123:  // Exif.Image.GrayResponseCurve
    case 0x0124:  // Exif.Image.T4Options
    case 0x0125:  // Exif.Image.T6Options
    case 0x0128:  // Exif.Image.ResolutionUnit
    case 0x0129:  // Exif.Image.PageNumber
    case 0x012d:  // Exif.Image.TransferFunction
    case 0x013d:  // Exif.Image.Predictor
    case 0x013e:  // Exif.Image.WhitePoint
    case 0x013f:  // Exif.Image.PrimaryChromaticities
    case 0x0140:  // Exif.Image.ColorMap
    case 0x0141:  // Exif.Image.HalftoneHints
    case 0x0142:  // Exif.Image.TileWidth
    case 0x0143:  // Exif.Image.TileLength
    case 0x0144:  // Exif.Image.TileOffsets
    case 0x0145:  // Exif.Image.TileByteCounts
    case 0x014c:  // Exif.Image.InkSet
    case 0x014d:  // Exif.Image.InkNames
    case 0x014e:  // Exif.Image.NumberOfInks
    case 0x0150:  // Exif.Image.DotRange
    case 0x0151:  // Exif.Image.TargetPrinter
    case 0x0152:  // Exif.Image.ExtraSamples
    case 0x0153:  // Exif.Image.SampleFormat
    case 0x0154:  // Exif.Image.SMinSampleValue
    case 0x0155:  // Exif.Image.SMaxSampleValue
    case 0x0156:  // Exif.Image.TransferRange
    case 0x0157:  // Exif.Image.ClipPath
    case 0x0158:  // Exif.Image.XClipPathUnits
    case 0x0159:  // Exif.Image.YClipPathUnits
    case 0x015a:  // Exif.Image.Indexed
    case 0x015b:  // Exif.Image.JPEGTables
    case 0x0200:  // Exif.Image.JPEGProc
    case 0x0201:  // Exif.Image.JPEGInterchangeFormat
    case 0x0202:  // Exif.Image.JPEGInterchangeFormatLength
    case 0x0203:  // Exif.Image.JPEGRestartInterval
    case 0x0205:  // Exif.Image.JPEGLosslessPredictors
    case 0x0206:  // Exif.Image.JPEGPointTransforms
    case 0x0207:  // Exif.Image.JPEGQTables
    case 0x0208:  // Exif.Image.JPEGDCTables
    case 0x0209:  // Exif.Image.JPEGACTables
    case 0x0211:  // Exif.Image.YCbCrCoefficients
    case 0x0212:  // Exif.Image.YCbCrSubSampling
    case 0x0213:  // Exif.Image.YCbCrPositioning
    case 0x0214:  // Exif.Image.ReferenceBlackWhite
    case 0x828d:  // Exif.Image.CFARepeatPatternDim
    case 0x828e:  // Exif.Image.CFAPattern
    // case 0x8773:  // Exif.Image.InterColorProfile
    case 0x8824:  // Exif.Image.SpectralSensitivity
    case 0x8828:  // Exif.Image.OECF
    case 0x9102:  // Exif.Image.CompressedBitsPerPixel
    case 0x9217:  // Exif.Image.SensingMethod
      return true;
    default:
      return false;
  }
}

bool isTiffImageTag(uint16_t tag, IfdId group) {
  const bool result = isTiffImageTagLookup(tag, group);
#ifdef EXIV2_DEBUG_MESSAGES
  if (result) {
    ExifKey key(tag, groupName(group));
    std::cerr << "Image tag: " << key << " (3)\n";
  } else {
    std::cerr << "Not an image tag: " << tag << " (4)\n";
  }
#endif
  return result;
}

TiffHeader::TiffHeader(ByteOrder byteOrder, uint32_t offset, bool hasImageTags) :
    TiffHeaderBase(42, 8, byteOrder, offset), hasImageTags_(hasImageTags) {
}

bool TiffHeader::isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const {
  if (!hasImageTags_) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "No image tags in this image\n";
#endif
    return false;
  }
#ifdef EXIV2_DEBUG_MESSAGES
  ExifKey key(tag, groupName(group));
#endif
  // If there are primary groups and none matches group, we're done
  if (pPrimaryGroups && !pPrimaryGroups->empty() &&
      std::find(pPrimaryGroups->begin(), pPrimaryGroups->end(), group) == pPrimaryGroups->end()) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Not an image tag: " << key << " (1)\n";
#endif
    return false;
  }
  // All tags of marked primary groups other than IFD0 are considered
  // image tags. That should take care of NEFs until we know better.
  if (pPrimaryGroups && !pPrimaryGroups->empty() && group != IfdId::ifd0Id) {
#ifdef EXIV2_DEBUG_MESSAGES
    ExifKey key(tag, groupName(group));
    std::cerr << "Image tag: " << key << " (2)\n";
#endif
    return true;
  }
  // Finally, if tag, group is one of the TIFF image tags -> bingo!
  return isTiffImageTag(tag, group);
}  // TiffHeader::isImageTag

void OffsetWriter::setOrigin(OffsetId id, uint32_t origin, ByteOrder byteOrder) {
  offsetList_[id] = OffsetData{origin, 0, byteOrder};
}

void OffsetWriter::setTarget(OffsetId id, uint32_t target) {
  auto it = offsetList_.find(id);
  if (it != offsetList_.end())
    it->second.target_ = target;
}

void OffsetWriter::writeOffsets(BasicIo& io) const {
  for (const auto& [_, off] : offsetList_) {
    io.seek(off.origin_, BasicIo::beg);
    byte buf[4] = {0, 0, 0, 0};
    l2Data(buf, off.target_, off.byteOrder_);
    io.write(buf, 4);
  }
}

}  // namespace Exiv2::Internal
