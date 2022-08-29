// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "sonymn_int.hpp"

#include "error.hpp"
#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "minoltamn_int.hpp"
#include "tiffcomposite_int.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <array>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
// -- Standard Sony Makernotes tags ---------------------------------------------------------------

//! Lookup table to translate Sony image quality values to readable labels
constexpr TagDetails sonyImageQuality[] = {{0, N_("RAW")},
                                           {1, N_("Super Fine")},
                                           {2, N_("Fine")},
                                           {3, N_("Standard")},
                                           {4, N_("Economy")},
                                           {5, N_("Extra Fine")},
                                           {6, N_("RAW + JPEG/HEIF")},
                                           {7, N_("Compressed RAW")},
                                           {8, N_("Compressed RAW + JPEG")},
                                           {9, N_("Light")},
                                           {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony white balance (main group) values to readable labels
constexpr TagDetails sonyWhiteBalanceStd[] = {{0x00, N_("Auto")},     {0x01, N_("Color Temperature/Color Filter")},
                                              {0x10, N_("Daylight")}, {0x20, N_("Cloudy")},
                                              {0x30, N_("Shade")},    {0x40, N_("Tungsten")},
                                              {0x50, N_("Flash")},    {0x60, N_("Fluorescent")},
                                              {0x70, N_("Custom")},   {0x80, N_("Underwater")}};

//! Lookup table to translate Sony auto HDR (part 1) values to readable labels
constexpr TagDetails sonyHDRStdPart1[] = {{0x00, N_("Off")}, {0x01, N_("Auto")}, {0x10, "1.0 EV"}, {0x11, "1.5 EV"},
                                          {0x12, "2.0 EV"},  {0x13, "2.5 EV"},   {0x14, "3.0 EV"}, {0x15, "3.5 EV"},
                                          {0x16, "4.0 EV"},  {0x17, "4.5 EV"},   {0x18, "5.0 EV"}, {0x19, "5.5 EV"},
                                          {0x1a, "6.0 EV"}};

//! Lookup table to translate Sony auto HDR (part 2) values to readable labels
constexpr TagDetails sonyHDRStdPart2[] = {
    {0, N_("Uncorrected image")}, {1, N_("HDR image (good)")}, {2, N_("HDR (fail 1)")}, {3, N_("HDR (fail 2)")}};

//! Lookup table to translate Sony off/on/(n/a) (1) values to readable labels
constexpr TagDetails sonyOffOnNA1[] = {{0, N_("Off")}, {1, N_("On")}, {256, N_("n/a")}};

//! Lookup table to translate Sony off/on/(n/a) (2) values to readable labels
constexpr TagDetails sonyOffOnNA2[] = {{0, N_("Off")}, {1, N_("On")}, {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony no/yes values to readable labels
constexpr TagDetails sonyNoYes[] = {{0, N_("No")}, {1, N_("Yes")}};

//! Lookup table to translate Sony picture effect values to readable labels
constexpr TagDetails sonyPictureEffect[] = {{0, N_("Off")},
                                            {1, N_("Toy Camera")},
                                            {2, N_("Pop Color")},
                                            {3, N_("Posterization")},
                                            {4, N_("Posterization B/W")},
                                            {5, N_("Retro Photo")},
                                            {6, N_("Soft High Key")},
                                            {7, N_("Partial Color (red)")},
                                            {8, N_("Partial Color (green)")},
                                            {9, N_("Partial Color (blue)")},
                                            {10, N_("Partial Color (yellow)")},
                                            {13, N_("High Contrast Monochrome")},
                                            {16, N_("Toy Camera (normal)")},
                                            {17, N_("Toy Camera (cool)")},
                                            {18, N_("Toy Camera (warm)")},
                                            {19, N_("Toy Camera (green)")},
                                            {20, N_("Toy Camera (magenta)")},
                                            {32, N_("Soft Focus (low)")},
                                            {33, N_("Soft Focus")},
                                            {34, N_("Soft Focus (high)")},
                                            {48, N_("Miniature (auto)")},
                                            {49, N_("Miniature (top)")},
                                            {50, N_("Miniature (middle horizontal)")},
                                            {51, N_("Miniature (bottom)")},
                                            {52, N_("Miniature (left)")},
                                            {53, N_("Miniature (middle vertical)")},
                                            {54, N_("Miniature (right)")},
                                            {64, N_("HDR Painting (low)")},
                                            {65, N_("HDR Painting")},
                                            {66, N_("HDR Painting (high)")},
                                            {80, N_("Rich-tone Monochrome")},
                                            {97, N_("Watercolor")},
                                            {98, N_("Watercolor 2")},
                                            {112, N_("Illustration (low)")},
                                            {113, N_("Illustration")},
                                            {114, N_("Illustration (high)")}};

//! Lookup table to translate Sony soft skin effect values to readable labels
constexpr TagDetails sonySoftSkinEffect[] = {
    {0, N_("Off")}, {1, N_("Low")}, {2, N_("Mid")}, {3, N_("High")}, {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony vignetting correction values to readable labels
constexpr TagDetails sonyVignettingCorrection[] = {{0, N_("Off")}, {2, N_("Auto")}, {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony lateral chromatic aberration values to readable labels
constexpr TagDetails sonyLateralChromaticAberration[] = {{0, N_("Off")}, {2, N_("Auto")}, {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony distortion correction settings values to readable labels
constexpr TagDetails sonyDistortionCorrectionSettings[] = {{0, N_("Off")}, {2, N_("Auto")}, {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony flash action values to readable labels
constexpr TagDetails sonyFlashAction[] = {{0, N_("Did not fire")},
                                          {1, N_("Flash fired")},
                                          {2, N_("External flash fired")},
                                          {3, N_("Wireless controlled flash fired")}};

//! Lookup table to translate Sony auto focus point selected (set 1) values to readable labels
constexpr TagDetails sonyAFPointSelectedSet1[] = {{0, N_("Auto")},
                                                  {1, N_("Center")},
                                                  {2, N_("Top")},
                                                  {3, N_("Upper-right")},
                                                  {4, N_("Right")},
                                                  {5, N_("Lower-right")},
                                                  {6, N_("Bottom")},
                                                  {7, N_("Lower-left")},
                                                  {8, N_("Left")},
                                                  {9, N_("Upper-left")},
                                                  {10, N_("Far Right")},
                                                  {11, N_("Far Left")},
                                                  {12, N_("Upper-middle")},
                                                  {13, N_("Near Right")},
                                                  {14, N_("Lower-middle")},
                                                  {15, N_("Near Left")},
                                                  {16, N_("Upper Far Right")},
                                                  {17, N_("Lower Far Right")},
                                                  {18, N_("Lower Far Left")},
                                                  {19, N_("Upper Far Left")}};

//! Lookup table to translate Sony auto focus point selected (set 2) values to readable labels
constexpr TagDetails sonyAFPointSelectedSet2[] = {
    {0, N_("Auto")}, {1, "A6"},   {2, "A7"},   {3, "B2"},   {4, "B3"},
    {5, "B4"},       {6, "B5"},   {7, "B6"},   {8, "B7"},   {9, "B8"},
    {10, "B9"},      {11, "B10"}, {12, "C1"},  {13, "C2"},  {14, "C3"},
    {15, "C4"},      {16, "C5"},  {17, "C6"},  {18, "C7"},  {19, "C8"},
    {20, "C9"},      {21, "C10"}, {22, "C11"}, {23, "D1"},  {24, "D2"},
    {25, "D3"},      {26, "D4"},  {27, "D5"},  {28, "D6"},  {29, "D7"},
    {30, "D8"},      {31, "D9"},  {32, "D10"}, {33, "D11"}, {34, "E1"},
    {35, "E2"},      {36, "E3"},  {37, "E4"},  {38, "E5"},  {39, N_("E6 (Center)")},
    {40, "E7"},      {41, "E8"},  {42, "E9"},  {43, "E10"}, {44, "E11"},
    {45, "F1"},      {46, "F2"},  {47, "F3"},  {48, "F4"},  {49, "F5"},
    {50, "F6"},      {51, "F7"},  {52, "F8"},  {53, "F9"},  {54, "F10"},
    {55, "F11"},     {56, "G1"},  {57, "G2"},  {58, "G3"},  {59, "G4"},
    {60, "G5"},      {61, "G6"},  {62, "G7"},  {63, "G8"},  {64, "G9"},
    {65, "G10"},     {66, "G11"}, {67, "H2"},  {68, "H3"},  {69, "H4"},
    {70, "H5"},      {71, "H6"},  {72, "H7"},  {73, "H8"},  {74, "H9"},
    {75, "H10"},     {76, "I5"},  {77, "I6"},  {78, "I7"},  {128, N_("Auto")}};

//! Lookup table to translate Sony auto focus point selected (set 3) values to readable labels
constexpr TagDetails sonyAFPointSelectedSet3[] = {{0, N_("Auto")},
                                                  {93, "A5"},
                                                  {94, "A6"},
                                                  {95, "A7"},
                                                  {106, "B2"},
                                                  {107, "B3"},
                                                  {108, "B4"},
                                                  {110, "B5"},
                                                  {111, "B6"},
                                                  {112, "B7"},
                                                  {114, "B8"},
                                                  {115, "B9"},
                                                  {116, "B10"},
                                                  {122, "C1"},
                                                  {123, "C2"},
                                                  {124, "C3"},
                                                  {215, "C4"},
                                                  {127, "C5"},
                                                  {128, "C6"},
                                                  {129, "C7"},
                                                  {131, "C8"},
                                                  {132, "C9"},
                                                  {133, "C10"},
                                                  {134, "C11"},
                                                  {139, "D1"},
                                                  {140, "D2"},
                                                  {141, "D3"},
                                                  {142, "D4"},
                                                  {144, "D5"},
                                                  {145, "D6"},
                                                  {146, "D7"},
                                                  {148, "D8"},
                                                  {149, "D9"},
                                                  {150, "D10"},
                                                  {151, "D11"},
                                                  {156, "E1"},
                                                  {157, "E2"},
                                                  {158, "E3"},
                                                  {159, "E4"},
                                                  {161, "E5"},
                                                  {162, N_("E6 (Center")},
                                                  {163, "E7"},
                                                  {165, "E8"},
                                                  {166, "E9"},
                                                  {167, "E10"},
                                                  {168, "E11"},
                                                  {173, "F1"},
                                                  {174, "F2"},
                                                  {175, "F3"},
                                                  {176, "F4"},
                                                  {178, "F5"},
                                                  {179, "F6"},
                                                  {180, "F7"},
                                                  {182, "F8"},
                                                  {183, "F9"},
                                                  {184, "F10"},
                                                  {185, "F11"},
                                                  {190, "G1"},
                                                  {191, "G2"},
                                                  {192, "G3"},
                                                  {193, "G4"},
                                                  {195, "G5"},
                                                  {196, "G6"},
                                                  {197, "G7"},
                                                  {199, "G8"},
                                                  {200, "G9"},
                                                  {201, "G10"},
                                                  {202, "G11"},
                                                  {208, "H2"},
                                                  {209, "H3"},
                                                  {210, "H4"},
                                                  {212, "H5"},
                                                  {213, "H6"},
                                                  {214, "H7"},
                                                  {216, "H8"},
                                                  {217, "H9"},
                                                  {218, "H10"},
                                                  {229, "I5"},
                                                  {230, "I6"},
                                                  {231, "I7"}};

//! Lookup table to translate Sony auto focus point selected (set 4) values to readable labels
constexpr TagDetails sonyAFPointSelectedSet4[] = {
    {0, N_("n/a")},         {1, N_("Top Left Zone")},    {2, N_("Top Zone")},   {3, N_("Top Right Zone")},
    {4, N_("Left Zone")},   {5, N_("Center Zone")},      {6, N_("Right Zone")}, {7, N_("Bottom Left Zone")},
    {8, N_("Bottom Zone")}, {9, N_("Bottom Right Zone")}};

//! Lookup table to translate Sony auto focus point selected (set 5) values to readable labels
constexpr TagDetails sonyAFPointSelectedSet5[] = {
    {0, N_("n/a")},           {1, N_("Center Zone")},   {2, N_("Top Zone")},          {3, N_("Right Zone")},
    {4, N_("Left Zone")},     {5, N_("Bottom Zone")},   {6, N_("Bottom Right Zone")}, {7, N_("Bottom Left Zone")},
    {8, N_("Top Left Zone")}, {9, N_("Top Right Zone")}};

//! Lookup table to translate Sony auto focus points used (set 1) values to readable labels
constexpr TagDetailsBitlistSorted sonyAFPointsUsedSet1[] = {{0, N_("Center")},           {1, N_("Top")},
                                                            {2, N_("Upper-right")},      {3, N_("Right")},
                                                            {4, N_("Lower-right")},      {5, N_("Bottom")},
                                                            {6, N_("Lower-left")},       {7, N_("Left")},
                                                            {8, N_("Upper-left")},       {9, N_("Far right")},
                                                            {10, N_("Far left")},        {11, N_("Upper-middle")},
                                                            {12, N_("Near right")},      {13, N_("Lower-middle")},
                                                            {14, N_("Near left")},       {15, N_("Upper far right")},
                                                            {16, N_("Lower far right")}, {17, N_("Lower far left")},
                                                            {18, N_("Upper far left")}};

//! Lookup table to translate Sony auto focus points used (set 2) values to readable labels
constexpr TagDetailsBitlistSorted sonyAFPointsUsedSet2[] = {
    {0, "A5"},   {1, "A6"},   {2, "A7"},   {3, "B2"},   {4, "B3"},   {5, "B4"},   {6, "B5"},   {7, "B6"},
    {8, "B7"},   {9, "B8"},   {10, "B9"},  {11, "B10"}, {12, "C1"},  {13, "C2"},  {14, "C3"},  {15, "C4"},
    {16, "C5"},  {17, "C6"},  {18, "C7"},  {19, "C8"},  {20, "C9"},  {21, "C10"}, {22, "C11"}, {23, "D1"},
    {24, "D2"},  {25, "D3"},  {26, "D4"},  {27, "D5"},  {28, "D6"},  {29, "D7"},  {30, "D8"},  {31, "D9"},
    {32, "D10"}, {33, "D11"}, {34, "E1"},  {35, "E2"},  {36, "E3"},  {37, "E4"},  {38, "E5"},  {39, N_("E6")},
    {40, "E7"},  {41, "E8"},  {42, "E9"},  {43, "E10"}, {44, "E11"}, {45, "F1"},  {46, "F2"},  {47, "F3"},
    {48, "F4"},  {49, "F5"},  {50, "F6"},  {51, "F7"},  {52, "F8"},  {53, "F9"},  {54, "F10"}, {55, "F11"},
    {56, "G1"},  {57, "G2"},  {58, "G3"},  {59, "G4"},  {60, "G5"},  {61, "G6"},  {62, "G7"},  {63, "G8"},
    {64, "G9"},  {65, "G10"}, {66, "G11"}, {67, "H2"},  {68, "H3"},  {69, "H4"},  {70, "H5"},  {71, "H6"},
    {72, "H7"},  {73, "H8"},  {74, "H9"},  {75, "H10"}, {76, "I5"},  {77, "I6"},  {78, "I7"},  {128, N_("Auto")}};

//! Lookup table to translate Sony focus mode 2 values to readable labels
constexpr TagDetails sonyFocusMode2[] = {{0, N_("Manual")}, {2, N_("AF-S")}, {3, N_("AF-C")},
                                         {4, N_("AF-A")},   {6, N_("DMF")},  {7, N_("AF-D")}};

//! Lookup table to translate Sony auto focus area mode setting (set 1) values to readable labels
constexpr TagDetails sonyAFAreaModeSettingSet1[] = {
    {0, N_("Wide")}, {4, N_("Local")}, {8, N_("Zone")}, {9, N_("Spot")}};

//! Lookup table to translate Sony auto focus area mode setting (set 2) values to readable labels
constexpr TagDetails sonyAFAreaModeSettingSet2[] = {{0, N_("Wide")},
                                                    {1, N_("Center")},
                                                    {3, N_("Flexible Spot")},
                                                    {4, N_("Flexible Spot (LA-EA4)")},
                                                    {9, N_("Center (LA-EA4)")},
                                                    {11, N_("Zone")},
                                                    {12, N_("Expanded flexible spot")}};

//! Lookup table to translate Sony auto focus area mode setting (set 3) values to readable labels
constexpr TagDetails sonyAFAreaModeSettingSet3[] = {
    {0, N_("Wide")}, {4, N_("Flexible spot")}, {8, N_("Zone")}, {9, N_("Center")}, {12, N_("Expanded flexible spot")}};

//! Lookup table to translate Sony auto focus tracking values to readable labels
constexpr TagDetails sonyAFTracking[] = {{0, N_("Off")}, {1, N_("Face tracking")}, {2, N_("Lock on AF")}};

//! Lookup table to translate Sony multi-frame noise reduction effect values to readable labels
constexpr TagDetails sonyMultiFrameNREffect[] = {{0, N_("Normal")}, {1, N_("High")}};

//! Lookup table to translate Sony variable low pass filter values to readable labels
constexpr StringTagDetails sonyVariableLowPassFilter[] = {
    {"0 0", N_("n/a")}, {"1 0", N_("Off")}, {"1 1", N_("Standard")}, {"1 2", N_("High")}, {"65535 65535", N_("n/a")}};

//! Lookup table to translate Sony RAW file type values to readable labels
constexpr TagDetails sonyRAWFileType[] = {
    {0, N_("Compressed RAW")}, {1, N_("Uncompressed RAW")}, {2, N_("Lossless Compressed RAW")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony metering mode 2 values to readable labels
constexpr TagDetails sonyMeteringMode2[] = {{0x100, N_("Multi-segment")},   {0x200, N_("Center-weighted average")},
                                            {0x301, N_("Spot (Standard)")}, {0x302, N_("Spot (Large)")},
                                            {0x400, N_("Average")},         {0x500, N_("Highlight")}};

//! Lookup table to translate Sony priority set in automatic white balance values to readable labels
constexpr TagDetails sonyPrioritySetInAWB[] = {{0, N_("Standard")}, {1, N_("Ambience")}, {2, N_("White")}};

//! Lookup table to translate Sony quality 2 (main group) values to readable labels
constexpr StringTagDetails sonyQuality2Std[] = {{"0 0", N_("n/a")},
                                                {"0 1", N_("Standard")},
                                                {"0 2", N_("Fine")},
                                                {"0 3", N_("Extra fine")},
                                                {"0 4", N_("Light")},
                                                {"1 0", N_("RAW")},
                                                {"1 1", N_("RAW + standard")},
                                                {"1 2", N_("RAW + fine")},
                                                {"1 3", N_("RAW + extra fine")},
                                                {"1 4", N_("RAW + light")}};

//! Lookup table to translate Sony JPEG/HEIF switch values to readable labels
constexpr TagDetails sonyJPEGHEIFSwitch[] = {{0, "JPEG"}, {1, "HEIF"}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony model ID values to readable labels
//  FORMAT: Uses a space before alternative models and caveats
//  NOTE:   Keep the array format in sync with the getModel() function
constexpr TagDetails sonyModelId[] = {{0, N_("Multiple camera models")},
                                      {2, "DSC-R1"},
                                      {256, "DSLR-A100"},
                                      {257, "DSLR-A900"},
                                      {258, "DSLR-A700"},
                                      {259, "DSLR-A200"},
                                      {260, "DSLR-A350"},
                                      {261, "DSLR-A300"},
                                      {262, "DSLR-A900 (APS-C mode)"},
                                      {263, "DSLR-A380 / DSLR-A390"},
                                      {264, "DSLR-A330"},
                                      {265, "DSLR-A230"},
                                      {266, "DSLR-A290"},
                                      {269, "DSLR-A850"},
                                      {270, "DSLR-A850 (APS-C mode)"},
                                      {273, "DSLR-A550"},
                                      {274, "DSLR-A500"},
                                      {275, "DSLR-A450"},
                                      {278, "NEX-5"},
                                      {279, "NEX-3"},
                                      {280, "SLT-A33"},
                                      {281, "SLT-A55 / SLT-A55V"},
                                      {282, "DSLR-A560"},
                                      {283, "DSLR-A580"},
                                      {284, "NEX-C3"},
                                      {285, "SLT-A35"},
                                      {286, "SLT-A65 / SLT-A65V"},
                                      {287, "SLT-A77 / SLT-A77V"},
                                      {288, "NEX-5N"},
                                      {289, "NEX-7"},
                                      {290, "NEX-VG20E"},
                                      {291, "SLT-A37"},
                                      {292, "SLT-A57"},
                                      {293, "NEX-F3"},
                                      {294, "SLT-A99 / SLT-A99V"},
                                      {295, "NEX-6"},
                                      {296, "NEX-5R"},
                                      {297, "DSC-RX100"},
                                      {298, "DSC-RX1"},
                                      {299, "NEX-VG900"},
                                      {300, "NEX-VG30E"},
                                      {302, "ILCE-3000 / ILCE-3500"},
                                      {303, "SLT-A58"},
                                      {305, "NEX-3N"},
                                      {306, "ILCE-7"},
                                      {307, "NEX-5T"},
                                      {308, "DSC-RX100M2"},
                                      {309, "DSC-RX10"},
                                      {310, "DSC-RX1R"},
                                      {311, "ILCE-7R"},
                                      {312, "ILCE-6000"},
                                      {313, "ILCE-5000"},
                                      {317, "DSC-RX100M3"},
                                      {318, "ILCE-7S"},
                                      {319, "ILCA-77M2"},
                                      {339, "ILCE-5100"},
                                      {340, "ILCE-7M2"},
                                      {341, "DSC-RX100M4"},
                                      {342, "DSC-RX10M2"},
                                      {344, "DSC-RX1RM2"},
                                      {346, "ILCE-QX1"},
                                      {347, "ILCE-7RM2"},
                                      {350, "ILCE-7SM2"},
                                      {353, "ILCA-68"},
                                      {354, "ILCA-99M2"},
                                      {355, "DSC-RX10M3"},
                                      {356, "DSC-RX100M5"},
                                      {357, "ILCE-6300"},
                                      {358, "ILCE-9"},
                                      {360, "ILCE-6500"},
                                      {362, "ILCE-7RM3"},
                                      {363, "ILCE-7M3"},
                                      {364, "DSC-RX0"},
                                      {365, "DSC-RX10M4"},
                                      {366, "DSC-RX100M6"},
                                      {367, "DSC-HX99"},
                                      {369, "DSC-RX100M5A"},
                                      {371, "ILCE-6400"},
                                      {372, "DSC-RX0M2"},
                                      {374, "DSC-RX100M7"},
                                      {375, "ILCE-7RM4"},
                                      {376, "ILCE-9M2"},
                                      {378, "ILCE-6600"},
                                      {379, "ILCE-6100"},
                                      {380, "ZV-1"},
                                      {381, "ILCE-7C"},
                                      {382, "ZV-E10"},
                                      {383, "ILCE-7SM3"},
                                      {384, "ILCE-1"},
                                      {385, "ILME-FX3"},
                                      {386, "ILCE-7RM3A"},
                                      {387, "ILCE-7RM4A"},
                                      {388, "ILCE-7M4"}};

//! Lookup table to translate Sony creative style (main group) values to readable labels
constexpr StringTagDetails sonyCreativeStyleStd[] = {{"AdobeRGB", N_("Adobe RGB")},
                                                     {"Autumnleaves", N_("Autumn leaves")},
                                                     {"BW", N_("Black and White")},
                                                     {"Clear", N_("Clear")},
                                                     {"Deep", N_("Deep")},
                                                     {"FL", N_("FL")},
                                                     {"IN", "IN"},
                                                     {"Landscape", N_("Landscape")},
                                                     {"Light", N_("Light")},
                                                     {"Neutral", N_("Neutral")},
                                                     {"None", N_("None")},
                                                     {"Portrait", N_("Portrait")},
                                                     {"Real", N_("Real")},
                                                     {"SH", N_("SH")},
                                                     {"Sepia", N_("Sepia")},
                                                     {"Standard", N_("Standard")},
                                                     {"Sunset", N_("Sunset")},
                                                     {"Vivid", N_("Vivid")},
                                                     {"VV2", N_("VV2")}};

//! Lookup table to translate Sony file format values to readable labels
constexpr StringTagDetails sonyFileFormat[] = {
    {"0 0 0 2", "JPEG"},      {"1 0 0 0", "SR2 1.0"},   {"2 0 0 0", "ARW 1.0"},   {"3 0 0 0", "ARW 2.0"},
    {"3 1 0 0", "ARW 2.1"},   {"3 2 0 0", "ARW 2.2"},   {"3 3 0 0", "ARW 2.3"},   {"3 3 1 0", "ARW 2.3.1"},
    {"3 3 2 0", "ARW 2.3.2"}, {"3 3 3 0", "ARW 2.3.3"}, {"3 3 5 0", "ARW 2.3.5"}, {"4 0 0 0", "ARW 4.0"}};

//! Lookup table to translate Sony dynamic range optimizer values to readable labels
constexpr TagDetails print0xb025[] = {{0, N_("Off")},
                                      {1, N_("Standard")},
                                      {2, N_("Advanced Auto")},
                                      {3, N_("Auto")},
                                      {8, N_("Advanced Lv1")},
                                      {9, N_("Advanced Lv2")},
                                      {10, N_("Advanced Lv3")},
                                      {11, N_("Advanced Lv4")},
                                      {12, N_("Advanced Lv5")},
                                      {16, "Lv1"},
                                      {17, "Lv2"},
                                      {18, "Lv3"},
                                      {19, "Lv4"},
                                      {20, "Lv5"}};

//! Lookup table to translate Sony color mode values to readable labels
constexpr TagDetails sonyColorMode[] = {
    {0, N_("Standard")},       {1, N_("Vivid")},        {2, N_("Portrait")},
    {3, N_("Landscape")},      {4, N_("Sunset")},       {5, N_("Night View/Portrait")},
    {6, N_("Black & White")},  {7, N_("Adobe RGB")},    {12, N_("Neutral")},
    {13, N_("Clear")},         {14, N_("Deep")},        {15, N_("Light")},
    {16, N_("Autumn leaves")}, {17, N_("Sepia")},       {18, N_("FL")},
    {19, N_("Vivid 2")},       {20, N_("IN")},          {21, N_("SH")},
    {100, N_("Neutral")},      {101, N_("Clear")},      {102, N_("Deep")},
    {103, N_("Light")},        {104, N_("Night view")}, {105, N_("Autumn leaves")},
    {255, N_("Off")},          {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony exposure mode values to readable labels
constexpr TagDetails sonyExposureMode[] = {{0, N_("Program AE")},
                                           {1, N_("Portrait")},
                                           {2, N_("Beach")},
                                           {3, N_("Sports")},
                                           {4, N_("Snow")},
                                           {5, N_("Landscape")},
                                           {6, N_("Auto")},
                                           {7, N_("Aperture-priority AE")},
                                           {8, N_("Shutter speed priority AE")},
                                           {9, N_("Night Scene/Twilight")},
                                           {10, N_("Hi-Speed Shutter")},
                                           {11, N_("Twilight Portrait")},
                                           {12, N_("Soft Snap/Portrait")},
                                           {13, N_("Fireworks")},
                                           {14, N_("Smile Shutter")},
                                           {15, N_("Manual")},
                                           {18, N_("High Sensitivity")},
                                           {19, N_("Macro")},
                                           {20, N_("Advanced Sports Shooting")},
                                           {29, N_("Underwater")},
                                           {33, N_("Food")},
                                           {34, N_("Sweep Panorama")},
                                           {35, N_("Handheld Night Shot")},
                                           {36, N_("Anti Motion Blur")},
                                           {37, N_("Pet")},
                                           {38, N_("Backlight Correction HDR")},
                                           {39, N_("Superior Auto")},
                                           {40, N_("Background Defocus")},
                                           {41, N_("Soft Skin")},
                                           {42, N_("3D Image")},
                                           {0xffff, N_("n/a")}};

//! Lookup table to translate Sony JPEG quality values to readable labels
constexpr TagDetails sonyJPEGQuality[] = {
    {0, N_("Standard")}, {1, N_("Fine")}, {2, N_("Extra Fine")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony anti-blur values to readable labels
constexpr TagDetails sonyAntiBlur[] = {
    {0, N_("Off")}, {1, N_("On (Continuous)")}, {2, N_("On (Shooting)")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony dynamic range optimizer 2 values to readable labels
constexpr TagDetails print0xb04f[] = {{0, N_("Off")}, {1, N_("Standard")}, {2, N_("Plus")}};

//! Lookup table to translate Sony intelligent auto values to readable labels
constexpr TagDetails sonyIntelligentAuto[] = {{0, N_("Off")}, {1, N_("On")}, {2, N_("Advanced")}};

//! Lookup table to translate Sony white balance 2 values to readable labels
constexpr TagDetails sonyWhiteBalance2[] = {{0, N_("Auto")},
                                            {4, N_("Manual")},
                                            {5, N_("Daylight")},
                                            {6, N_("Cloudy")},
                                            {7, N_("Cool White Fluorescent")},
                                            {8, N_("Day White Fluorescent")},
                                            {9, N_("Daylight Fluorescent")},
                                            {10, N_("Incandescent2")},
                                            {11, N_("Warm White Fluorescent")},
                                            {14, N_("Incandescent")},
                                            {15, N_("Flash")},
                                            {17, N_("Underwater 1 (Blue Water)")},
                                            {18, N_("Underwater 2 (Green Water)")},
                                            {19, N_("Underwater Auto")}};

//! Lookup table to translate Sony focus mode values to readable labels
constexpr TagDetails sonyFocusMode[] = {{1, "AF-S"}, {2, "AF-C"}, {4, N_("Permanent-AF")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony auto focus mode (set 1) values to readable labels
constexpr TagDetails sonyAFModeSet1[] = {{0, N_("Default")},   {1, N_("Multi")},          {2, N_("Center")},
                                         {3, N_("Spot")},      {4, N_("Flexible Spot")},  {6, N_("Touch")},
                                         {14, N_("Tracking")}, {15, N_("Face Detected")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony auto focus mode (set 2) values to readable labels
constexpr TagDetails sonyAFModeSet2[] = {{0, N_("Multi")},
                                         {1, N_("Center")},
                                         {2, N_("Spot")},
                                         {3, N_("Flexible spot")},
                                         {10, N_("Selective (for miniature effect)")},
                                         {14, N_("Tracking")},
                                         {15, N_("Face tracking")},
                                         {255, N_("Manual")}};

//! Lookup table to translate Sony auto focus illuminator values to readable labels
constexpr TagDetails sonyAFIlluminator[] = {{0, N_("Off")}, {1, N_("Auto")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony macro values to readable labels
constexpr TagDetails sonyMacro[] = {{0, N_("Off")}, {1, N_("On")}, {2, N_("Close Focus")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony flash level values to readable labels
constexpr TagDetails sonyFlashLevel[] = {
    {-32768, N_("Low")}, {-9, "-3.0 EV"}, {-8, "-2.7 EV"},  {-7, "-2.3 EV"},    {-6, "-2.0 EV"},   {-5, "-1.7 EV"},
    {-4, "-1.3 EV"},     {-3, "-1.0 EV"}, {-2, "-0.7 EV"},  {-1, "-0.3 EV"},    {0, N_("Normal")}, {1, "+0.3 EV"},
    {2, "+0.7 EV"},      {3, "+1.0 EV"},  {4, "+1.3 EV"},   {5, "+1.7 EV"},     {6, "+2.0 EV"},    {7, "+2.3 EV"},
    {8, "+2.7 EV"},      {9, "+3.0 EV"},  {128, N_("n/a")}, {32767, N_("High")}};

//! Lookup table to translate Sony release mode values to readable labels
constexpr TagDetails sonyReleaseMode[] = {{0, N_("Normal")},
                                          {2, N_("Continuous")},
                                          {5, N_("Exposure Bracketing")},
                                          {6, N_("White Balance Bracketing")},
                                          {8, N_("DRO Bracketing")},
                                          {0xffff, N_("n/a")}};

//! Lookup table to translate Sony sequence number values to readable labels
constexpr TagDetails sonySequenceNumber[] = {{0, N_("Single")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony focus mode 3 values to readable labels
constexpr TagDetails sonyFocusMode3[] = {
    {0, N_("Manual")}, {2, N_("AF-S")}, {3, N_("AF-C")}, {5, N_("Semi-manual")}, {6, N_("DMF")}};

//! Lookup table to translate Sony high ISO noise reduction 2 values to readable labels
constexpr TagDetails sonyHighISONoiseReduction2[] = {
    {0, N_("Normal")}, {1, N_("High")}, {2, N_("Low")}, {3, N_("Off")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony release mode 2 values to readable labels
constexpr TagDetails sonyReleaseMode2[] = {{0, N_("Normal")},
                                           {1, N_("Continuous")},
                                           {2, N_("Continuous - Exposure Bracketing")},
                                           {3, N_("DRO or White Balance Bracketing")},
                                           {5, N_("Continuous - Burst")},
                                           {6, N_("Single Frame - Capture During Movie")},
                                           {7, N_("Continuous - Sweep Panorama")},
                                           {8, N_("Continuous - Anti-Motion Blur, Hand-held Twilight")},
                                           {9, N_("Continuous - HDR")},
                                           {10, N_("Continuous - Background defocus")},
                                           {13, N_("Continuous - 3D Sweep Panorama")},
                                           {15, N_("Continuous - High Resolution Sweep Panorama")},
                                           {16, N_("Continuous - 3D Image")},
                                           {17, N_("Continuous - Burst 2")},
                                           {18, N_("Normal - iAuto+")},
                                           {19, N_("Continuous - Speed/Advance Priority")},
                                           {20, N_("Continuous - Multi-Frame NR")},
                                           {23, N_("Single-frame - Exposure Bracketing")},
                                           {26, N_("Continuous Low")},
                                           {27, N_("Continuous - High Sensitivity")},
                                           {28, N_("Smile Shutter")},
                                           {29, N_("Continuous - Tele-zoom Advance Priority")},
                                           {146, N_("Single Frame - Movie Capture")}};

//! Lookup table to translate Sony long exposure noise reduction values to readable labels
constexpr TagDetails sonyLongExposureNoiseReduction[] = {
    {0x00000000, N_("Off")},         {0x00000001, N_("On (unused)")}, {0x00010001, N_("On (dark subtracted)")},
    {0xffff0000, N_("Off (65535)")}, {0xffff0001, N_("On (65535)")},  {0xffffffff, N_("n/a")}};

//! Lookup table to translate Sony high ISO Noise reduction values to readable labels
constexpr TagDetails sonyHighISONoiseReductionStd[] = {{0, N_("Off")},  {1, N_("Low")},    {2, N_("Normal")},
                                                       {3, N_("High")}, {256, N_("Auto")}, {0xffff, N_("n/a")}};

static auto getModel(const ExifData* metadata, std::string& val) {
  auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == asciiString) {
    val = pos->toString(0);
    return true;
  }

  // NOTE: As using the translated SonyModelID value, need to be synchronized with the array format
  pos = metadata->findKey(ExifKey("Exif.Sony1.SonyModelID"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedShort) {
    std::string temp = pos->print(metadata);
    if (temp.find(' ') == std::string::npos) {
      val = temp;
      return true;
    }
    val = "";
    return false;
  }
  pos = metadata->findKey(ExifKey("Exif.Sony2.SonyModelID"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedShort) {
    std::string temp = pos->print(metadata);
    if (temp.find(' ') == std::string::npos) {
      val = temp;
      return true;
    }
    val = "";
    return false;
  }

  val = "";
  return false;
}

static auto getAFAreaModeSetting(const ExifData* metadata, uint32_t& val) {
  auto pos = metadata->findKey(ExifKey("Exif.Sony1.AFAreaModeSetting"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedByte) {
    std::ostringstream oss;
    pos->write(oss, metadata);
    if (oss.str() == _("n/a")) {
      val = 0;
      return false;
    }

    val = pos->toUint32(0);
    return true;
  }
  pos = metadata->findKey(ExifKey("Exif.Sony2.AFAreaModeSetting"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedByte) {
    std::ostringstream oss;
    pos->write(oss, metadata);
    if (oss.str() == _("n/a")) {
      val = 0;
      return false;
    }

    val = pos->toUint32(0);
    return true;
  }

  val = 0;
  return false;
}

static auto getMetaVersion(const ExifData* metadata, std::string& val) {
  const auto pos = metadata->findKey(ExifKey("Exif.SonySInfo1.MetaVersion"));

  if (pos != metadata->end() && pos->typeId() == asciiString) {
    std::string temp = pos->toString();
    if (temp.length() != 0) {
      val = temp;
      return true;
    }
  }
  val = "";
  return false;
}

static auto getFocusMode2(const ExifData* metadata, uint32_t& val) {
  auto pos = metadata->findKey(ExifKey("Exif.Sony1.FocusMode2"));

  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedByte) {
    std::ostringstream oss;
    pos->write(oss, metadata);
    if (oss.str() == _("n/a")) {
      val = 0;
      return false;
    }

    val = pos->toUint32(0);
    return true;
  }
  pos = metadata->findKey(ExifKey("Exif.Sony2.FocusMode2"));
  if (pos != metadata->end() && pos->size() != 0 && pos->typeId() == unsignedByte) {
    std::ostringstream oss;
    pos->write(oss, metadata);
    if (oss.str() == _("n/a")) {
      val = 0;
      return false;
    }
    val = pos->toUint32(0);
    return true;
  }

  val = 0;
  return false;
}

std::ostream& SonyMakerNote::printWhiteBalanceFineTune(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedLong) {
    os << "(" << value << ")";
    return os;
  }
  // Sony writes the tag as an unsignedLong but treat it as a signedLong. Source:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L681
  os << static_cast<int32_t>(value.toUint32(0));
  return os;
}

std::ostream& SonyMakerNote::printMultiBurstMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != undefined) {
    os << "(" << value << ")";
    return os;
  }
  // Some cameras do not set the type to undefined. Source:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L763

  printMinoltaSonyBoolValue(os, value, metadata);
  return os;
}

std::ostream& SonyMakerNote::printMultiBurstSize(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }
  // Some cameras do not set the type to unsignedShort. Source:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L771

  os << value.toUint32(0);
  return os;
}

std::ostream& SonyMakerNote::printAutoHDRStd(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedLong) {
    os << "(" << value << ")";
    return os;
  }

  // Sony writes the tag as an unsignedLong but treat it as 2 unsignedShort values. Source:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L887

  const auto v0 = value.toUint32(0);
  EXV_PRINT_TAG(sonyHDRStdPart1)(os, (v0 & 0x00ff), metadata);
  os << ", ";
  EXV_PRINT_TAG(sonyHDRStdPart2)(os, (v0 >> 16), metadata);

  return os;
}

std::ostream& SonyMakerNote::printWBShiftABGM(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 2 || value.typeId() != signedLong) {
    os << "(" << value << ")";
    return os;
  }
  // Examples of Output:
  // 1. "A/B: 0, G/M: 0"
  // 2. "A/B: 1B, G/M: 2M"

  const auto v0 = value.toInt64(0);
  const auto v1 = value.toInt64(1);

  os << "A/B: ";
  if (v0 == 0) {
    os << 0;
  } else if (v0 < 0) {
    os << "A" << -v0;
  } else {
    os << "B" << v0;
  }

  os << ", G/M: ";
  if (v1 == 0) {
    os << 0;
  } else if (v1 < 0) {
    os << "G" << -v1;
  } else {
    os << "M" << v1;
  }
  return os;
}

std::ostream& SonyMakerNote::printFocusMode2(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  // Tag only valid for certain camera models. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1123
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }
  const auto v0 = value.toUint32(0);

  constexpr std::array models{"DSC-RX10M4", "DSC-RX100M6", "DSC-RX100M7", "DSC-RX100M5A", "DSC-HX99", "DSC-RX0M2"};
  if (!startsWith(model, "DSC-") ||
      std::any_of(models.begin(), models.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyFocusMode2)(os, v0, metadata);
    return os;
  }

  os << _("n/a");

  return os;
}

std::ostream& SonyMakerNote::printAFAreaModeSetting(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  // Tag only valid for certain camera models. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1139
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }
  const auto v0 = value.toUint32(0);

  constexpr std::array models1{"SLT-", "HV"};
  if (std::any_of(models1.begin(), models1.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyAFAreaModeSettingSet1)(os, v0, metadata);
    return os;
  }

  constexpr std::array models2{"NEX-",        "ILCE-",        "ILME-",    "DSC-RX10M4", "DSC-RX100M6",
                               "DSC-RX100M7", "DSC-RX100M5A", "DSC-HX99", "DSC-RX0M2"};
  if (std::any_of(models2.begin(), models2.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyAFAreaModeSettingSet2)(os, v0, metadata);
    return os;
  }

  if (startsWith(model, "ILCA-")) {
    EXV_PRINT_TAG(sonyAFAreaModeSettingSet3)(os, v0, metadata);
    return os;
  }

  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printFlexibleSpotPosition(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 2 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }

  // Tag only valid for certain camera models. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1189
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  constexpr std::array models{"NEX-",        "ILCE-",        "ILME-",    "DSC-RX10M4", "DSC-RX100M6",
                              "DSC-RX100M7", "DSC-RX100M5A", "DSC-HX99", "DSC-RX0M2"};
  if (std::any_of(models.begin(), models.end(), [&model](auto& m) { return startsWith(model, m); })) {
    os << value.toUint32(0) << ", " << value.toUint32(1);
    return os;
  }

  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printAFPointSelected(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  // Tag only valid for certain camera models. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1203
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  uint32_t aFAreaModeSetting = 0;
  const auto status = getAFAreaModeSetting(metadata, aFAreaModeSetting);
  constexpr std::array models1{"SLT-", "HV-"};
  constexpr std::array models2{"ILCE-", "ILME-"};
  constexpr std::array models3{"ILCA-68", "ILCA-77M2"};
  constexpr std::array models4{"NEX-", "ILCE-", "ILME-"};

  if (std::any_of(models1.begin(), models1.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet1)(os, value.toUint32(0), metadata);
    return os;
  } else if (std::any_of(models2.begin(), models2.end(), [&model](auto& m) { return startsWith(model, m); }) &&
             status && aFAreaModeSetting == 4) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet1)(os, value.toUint32(0), metadata);
    return os;
  } else if (std::any_of(models3.begin(), models3.end(), [&model](auto& m) { return startsWith(model, m); }) &&
             status && aFAreaModeSetting != 8) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet2)(os, value, metadata);
    return os;
  } else if (startsWith(model, "ILCA-99M2") && status && aFAreaModeSetting != 8) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet3)(os, value, metadata);
    return os;
  } else if (startsWith(model, "ILCA-") && status && aFAreaModeSetting == 8) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet4)(os, value.toUint32(0), metadata);
    return os;
  } else if (std::any_of(models4.begin(), models4.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyAFPointSelectedSet5)(os, value.toUint32(0), metadata);
    return os;
  } else {
    os << _("n/a");
    return os;
  }
}

std::ostream& SonyMakerNote::printAFPointsUsed(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  constexpr std::array models1{"ILCA-", "DSC-"};
  constexpr std::array models2{"ILCA-68", "ILCA-77M2"};

  if (std::none_of(models1.begin(), models1.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG_BITLIST_ALL_LE(sonyAFPointsUsedSet1)(os, value, metadata);
    return os;
  } else if (std::any_of(models2.begin(), models2.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG_BITLIST_ALL_LE(sonyAFPointsUsedSet2)(os, value, metadata);
    return os;
  } else {
    os << _("n/a");
    return os;
  }
}

std::ostream& SonyMakerNote::printAFTracking(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  // Tag only valid for certain camera models. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1353
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  constexpr std::array models{"DSC-RX10M4", "DSC-RX100M6", "DSC-RX100M7", "DSC-RX100M5A", "DSC-HX99", "DSC-RX0M2"};
  if (!startsWith(model, "DSC-") ||
      std::any_of(models.begin(), models.end(), [&model](auto& m) { return startsWith(model, m); })) {
    EXV_PRINT_TAG(sonyAFTracking)(os, value.toUint32(0), metadata);
    return os;
  }

  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printFocalPlaneAFPointsUsed(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }
  if (value.toUint32(0) == 0) {
    os << _("None");
    return os;
  }
  os << "(" << value << ")";
  return os;
}

std::ostream& SonyMakerNote::printWBShiftABGMPrecise(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 2 || value.typeId() != signedLong) {
    os << "(" << value << ")";
    return os;
  }
  std::ios::fmtflags f(os.flags());

  const auto temp0 = static_cast<double>(value.toInt64(0)) / double(1000.0);
  const auto temp1 = static_cast<double>(value.toInt64(1)) / double(1000.0);

  os << "A/B: ";
  if (temp0 == 0) {
    os << 0;
  } else if (temp0 < 0) {
    os << "A" << std::fixed << std::setprecision(2) << -temp0;
  } else {
    os << "B" << std::fixed << std::setprecision(2) << temp0;
  }
  os << ", G/M: ";
  if (temp1 == 0) {
    os << 0;
  } else if (temp1 < 0) {
    os << "G" << std::fixed << std::setprecision(2) << -temp1;
  } else {
    os << "M" << std::fixed << std::setprecision(2) << temp1;
  }

  os.flags(f);
  return os;
}

std::ostream& SonyMakerNote::printExposureStandardAdjustment(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != signedRational) {
    os << "(" << value << ")";
    return os;
  }

  std::ios::fmtflags f(os.flags());

  const auto [r, s] = value.toRational();
  os << std::fixed << std::setprecision(1) << (double(r) / double(s));
  os.flags(f);

  return os;
}

std::ostream& SonyMakerNote::printPixelShiftInfo(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 6 || value.typeId() != undefined) {
    os << "(" << value << ")";
    return os;
  }

  // Tag format:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1504
  if (value.toString() == "0 0 0 0 0 0") {
    os << _("n/a");
    return os;
  }

  // Convert from little endian format
  const auto groupID =
      (value.toUint32(3) << 24) + (value.toUint32(2) << 16) + (value.toUint32(1) << 8) + value.toUint32(0);

  std::ios::fmtflags f(os.flags());

  os << "Group " << std::setw(2) << std::setfill('0') << ((groupID >> 17) & 0x1f) << std::setw(2) << std::setfill('0')
     << ((groupID >> 12) & 0x1f) << std::setw(2) << std::setfill('0') << ((groupID >> 6) & 0x3f) << std::setw(2)
     << std::setfill('0') << (groupID & 0x3f);

  os << ", Shot " << value.toUint32(4) << "/" << value.toUint32(5) << " (0x" << std::hex << (groupID >> 22) << ")";
  os.flags(f);
  return os;
}

std::ostream& SonyMakerNote::printFocusFrameSize(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 6 || value.typeId() != undefined) {
    os << "(" << value << ")";
    return os;
  }

  // Tag is written as undefined type but is used as unsignedShort. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L1578

  if (value.toUint32(4) == 0 && value.toUint32(5) == 0) {
    os << _("n/a");
    return os;
  }
  // Convert from little endian format
  os << ((value.toUint32(1) << 8) + value.toUint32(0)) << "x" << ((value.toUint32(3) << 8) + value.toUint32(2));
  return os;
}

std::ostream& SonyMakerNote::printColorTemperature(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedLong) {
    os << "(" << value << ")";
    return os;
  }
  const auto v0 = value.toUint32(0);
  switch (v0) {
    case 0:
      os << _("Auto");
      break;
    case 0xffffffff:
      os << _("n/a");
      break;
    default:
      os << v0 << " K";
      break;
  }

  return os;
}

std::ostream& SonyMakerNote::printColorCompensationFilter(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1 || value.typeId() != unsignedLong) {
    os << "(" << value << ")";
    return os;
  }
  // Tag is written as an unsignedLong but used as a signedLong. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2093

  int32_t temp = static_cast<int32_t>(value.toUint32(0));
  os << "G/M: ";
  if (temp == 0)
    os << "0";
  else if (temp < 0)
    os << "G" << -temp;
  else
    os << "M" << temp;

  return os;
}

static void findLensSpecFlags(const Value& value, std::string& flagsStart, std::string& flagsEnd) {
  struct LensSpecFlags {
    const int64_t mask;  // Contains all the bits set in the flags.val_ array values
    const std::array<TagDetails, 4> flags;
    bool prepend;
  };
  static constexpr std::array<LensSpecFlags, 8> lSFArray = {
      LensSpecFlags{0x4000, {{{0x4000, "PZ"}}}, true},
      LensSpecFlags{0x0300, {{{0x0100, "DT"}, {0x0200, "FE"}, {0x0300, "E"}}}, true},
      LensSpecFlags{
          0x00e0, {{{0x0020, "STF"}, {0x0040, N_("Reflex")}, {0x0060, N_("Macro")}, {0x0080, N_("Fisheye")}}}, false},
      LensSpecFlags{0x000c, {{{0x0004, "ZA"}, {0x0008, "G"}}}, false},
      LensSpecFlags{0x0003, {{{0x0001, "SSM"}, {0x0002, "SAM"}}}, false},
      LensSpecFlags{0x8000, {{{0x8000, "OSS"}}}, false},
      LensSpecFlags{0x2000, {{{0x2000, "LE"}}}, false},
      LensSpecFlags{0x0800, {{{0x0800, "II"}}}, false}};

  // When processing, a bitwise 'AND' selects a compatible LensSpecFlags entry,
  // then search inside the 'flags' array for one match.
  //
  // See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L10545

  const auto joinedV0V7 = ((value.toUint32(0) << 8) + value.toUint32(7));
  auto temp = 0;
  for (const auto& i : lSFArray) {
    temp = i.mask & joinedV0V7;
    if (temp) {  // Check if a flag matches in the current LensSpecFlags
      const auto it =
          std::find_if(i.flags.begin(), i.flags.end(), [temp](const TagDetails& td) { return (temp == td.val_); });

      if (it == i.flags.end()) {
        // Should never get in here. LensSpecFlags.mask should contain all the
        // bits in all the LensSpecFlags.flags.val_ entries
        throw Error(ErrorCode::kerErrorMessage,
                    std::string("LensSpecFlags mask doesn't match the bits in the flags array"));
      }
      if (i.prepend)
        flagsStart = (flagsStart.empty() ? it->label_ : it->label_ + std::string(" ") + flagsStart);
      else
        flagsEnd = (flagsEnd.empty() ? it->label_ : flagsEnd + std::string(" ") + it->label_);
    }
  }
}

std::ostream& SonyMakerNote::printLensSpec(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 8 || value.typeId() != unsignedByte) {
    os << "(" << value << ")";
    return os;
  }

  // Tag uses 8 bytes in the format:
  // <Flgs 1> <Flgs 2> <Focal len min> <Focal len max 1> <Focal len max 2> <Aperture min> <Aperture max> <Flags
  // 3>
  //   (0)       (1)         (2)              (3)               (4)             (5)            (6)          (7)
  //
  // Bytes 2-6 are each interpreted as 2 nibbles which are used as decimal. Nibbles have a value less than 10.
  // e.g., 36 == 0x24, converts to "2" and "4".
  // Optional elements (==0) are <Flgs 1>, <Flgs 2>, <Focal len max 1>, <Focal len max 2>, <Aperture max> and
  // <Flgs 3>
  //
  // Values for the tag are only set with compatible lenses, otherwise all 8 are set to 0.
  //
  // Examples of final output:
  // 1. "FE 90mm F2.8 Macro G OSS"
  // 2. "E PZ 16-50mm F3.5-5.6 OSS"
  // 3. "DT 18-55mm F3.5-5.6 SAM"
  // 4. "28-100mm F1.8-4.9"
  // 5. "35mm F2.0"
  //
  // See:
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2170

  const auto focalLenMin = value.toUint32(2);
  const auto focalLenMax1 = value.toUint32(3);
  const auto focalLenMax2 = value.toUint32(4);
  const auto appertureMin = value.toUint32(5);
  const auto apertureMax = value.toUint32(6);

  if (value.toString() == "0 0 0 0 0 0 0 0" || focalLenMin == 0 || appertureMin == 0) {
    os << _("Unknown");
    return os;
  }

  std::string flagsStart;
  std::string flagsEnd;
  findLensSpecFlags(value, flagsStart, flagsEnd);

  // Output <Flgs 1>
  if (!flagsStart.empty())
    os << flagsStart << " ";

  // Output <Focal len min>
  auto temp = ((focalLenMin >> 4) & 0x0f);
  if (temp != 0)  // Remove leading zero
    os << temp;

  os << (focalLenMin & 0x0f);

  if (focalLenMax1 != 0 || focalLenMax2 != 0) {
    os << "-";
  }

  if (focalLenMax1 != 0) {
    // Output <Focal len max 1>
    temp = ((focalLenMax1 >> 4) & 0x0f);
    if (temp != 0)  // Remove leading zero
      os << temp;
    os << (focalLenMax1 & 0x0f);

    // Output <Focal len max 2>
    os << ((focalLenMax2 >> 4) & 0x0f) << (focalLenMax2 & 0x0f);
  } else {
    // Output <Focal len max 2>
    if (focalLenMax2 != 0)
      os << ((focalLenMax2 >> 4) & 0x0f) << (focalLenMax2 & 0x0f);
  }

  os << "mm";

  // Output <Aperture min>
  os << " F" << ((appertureMin >> 4) & 0x0f) << "." << (appertureMin & 0x0f);

  // Output <Aperture max>
  if (apertureMax != 0)
    os << "-" << ((apertureMax >> 4) & 0x0f) << "." << (apertureMax & 0x0f);

  // Output <Flags 3>
  if (!flagsEnd.empty())
    os << " " << flagsEnd;
  return os;
}

std::ostream& SonyMakerNote::printImageSize(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 2 || value.typeId() != unsignedLong) {
    os << "(" << value << ")";
    return os;
  }
  // Values are stored as Height then Width
  os << value.toString(1) << " x " << value.toString(0);

  return os;
}

std::ostream& SonyMakerNote::printFocusMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }
  // Only valid for certain models of camera. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2255

  std::string metaVersion;
  if (!getMetaVersion(metadata, metaVersion) || metaVersion != "DC7303320222000") {
    EXV_PRINT_TAG(sonyFocusMode)(os, value.toUint32(0), metadata);
    return os;
  }

  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printAFMode(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }

  // Only valid for certain models of camera. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2275
  std::string metaVersion;
  if (!getMetaVersion(metadata, metaVersion) || metaVersion != "DC7303320222000") {
    EXV_PRINT_TAG(sonyAFModeSet1)(os, value.toUint32(0), metadata);
    return os;
  }

  uint32_t focusMode2 = 0;
  if (getFocusMode2(metadata, focusMode2) && focusMode2 != 0) {
    EXV_PRINT_TAG(sonyAFModeSet2)(os, value.toUint32(0), metadata);
    return os;
  }

  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printFocusMode3(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }

  // Only valid for certain models of camera. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2411
  std::string metaVersion;
  if (getMetaVersion(metadata, metaVersion) && metaVersion == "DC7303320222000") {
    EXV_PRINT_TAG(sonyFocusMode3)(os, value.toUint32(0), metadata);
    return os;
  }
  os << _("n/a");
  return os;
}

std::ostream& SonyMakerNote::printHighISONoiseReduction2(std::ostream& os, const Value& value,
                                                         const ExifData* metadata) {
  if (value.count() != 1 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    return os;
  }

  // Only valid for certain models of camera. See
  // https://github.com/exiftool/exiftool/blob/1e17485cbb372a502e5b9d052d01303db735e6fa/lib/Image/ExifTool/Sony.pm#L2437
  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  if (startsWith(model, "DSC-") || startsWith(model, "Stellar")) {
    EXV_PRINT_TAG(sonyHighISONoiseReduction2)(os, value.toUint32(0), metadata);
    return os;
  }

  os << _("n/a");
  return os;
}

// Sony MakerNote Tag Info
constexpr TagInfo SonyMakerNote::tagInfo_[] = {
    {0x0102, "Quality", N_("Quality"), N_("Image quality"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     EXV_PRINT_TAG(sonyImageQuality)},
    {0x0104, "FlashExposureComp", N_("Flash Exposure Compensation"), N_("Flash exposure compensation in EV"),
     IfdId::sony1Id, SectionId::makerTags, signedRational, -1, print0x9204},
    {0x0105, "Teleconverter", N_("Teleconverter Model"), N_("Teleconverter Model"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, printMinoltaSonyTeleconverterModel},
    {0x0112, "WhiteBalanceFineTune", N_("White Balance Fine Tune"), N_("White Balance Fine Tune"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, printWhiteBalanceFineTune},
    {0x0115, "WhiteBalance", N_("White balance"), N_("White balance"), IfdId::sony1Id, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(sonyWhiteBalanceStd)},
    {0x1000, "MultiBurstMode", N_("Multi Burst Mode"), N_("Multi Burst Mode"), IfdId::sony1Id, SectionId::makerTags,
     undefined, -1, printMultiBurstMode},
    {0x1001, "MultiBurstImageWidth", N_("Multi Burst Image Width"), N_("Multi Burst Image Width"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, printMultiBurstSize},
    {0x1002, "MultiBurstImageHeight", N_("Multi Burst Image Height"), N_("Multi Burst Image Height"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, printMultiBurstSize},
    {0x2001, "PreviewImage", N_("Preview Image"), N_("JPEG preview image"), IfdId::sony1Id, SectionId::makerTags,
     undefined, -1, printValue},
    {0x2002, "Rating", "Rating", N_("Rating"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x2004, "Contrast", "Contrast", N_("Contrast"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1, printValue},
    {0x2005, "Saturation", "Saturation", N_("Saturation"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1,
     printValue},
    {0x2006, "Sharpness", "Sharpness", N_("Sharpness"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1,
     printValue},
    {0x2007, "Brightness", "Brightness", N_("Brightness"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1,
     printValue},
    {0x2008, "LongExposureNoiseReduction", "Long exposure noise reduction", N_("Long exposure noise reduction"),
     IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyLongExposureNoiseReduction)},
    {0x2009, "HighISONoiseReduction", "High ISO noise reduction", N_("High ISO noise reduction"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyHighISONoiseReductionStd)},
    {0x200a, "AutoHDR", N_("Auto high dynamic range mode"), N_("Auto high dynamic range mode"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, printAutoHDRStd},
    {0x200b, "MultiFrameNoiseReduction", N_("Multi frame noise reduction"), N_("Multi frame noise reduction"),
     IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyOffOnNA1)},
    {0x200e, "PictureEffect", N_("Picture effect"), N_("Picture effect"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyPictureEffect)},
    {0x200f, "SoftSkinEffect", N_("Soft skin effect"), N_("Soft skin effect"), IfdId::sony1Id, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(sonySoftSkinEffect)},
    {0x2011, "VignettingCorrection", N_("Vignetting correction"), N_("Vignetting correction"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyVignettingCorrection)},
    {0x2012, "LateralChromaticAberration", N_("Lateral chromatic aberration"), N_("Lateral chromatic aberration"),
     IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyLateralChromaticAberration)},
    {0x2013, "DistortionCorrectionSetting", N_("Distortion correction setting"), N_("Distortion correction setting"),
     IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyDistortionCorrectionSettings)},
    {0x2014, "WBShiftABGM", N_("White balance Shift (amber/blue, green/magenta)"),
     N_("First number is amber/blue, second is green/magenta"), IfdId::sony1Id, SectionId::makerTags, signedLong, 2,
     printWBShiftABGM},
    {0x2016, "AutoPortraitFramed", N_("Auto portrait framed"), N_("Indicates if the Portrait Framing feature was used"),
     IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyNoYes)},
    {0x2017, "FlashAction", N_("Flash action"), N_("Flash action"), IfdId::sony1Id, SectionId::makerTags, unsignedLong,
     -1, EXV_PRINT_TAG(sonyFlashAction)},
    {0x201a, "ElectronicFrontCurtainShutter", N_("Electronic front curtain shutter"),
     N_("Electronic front curtain shutter"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     printMinoltaSonyBoolValue},
    {0x201b, "FocusMode2", N_("Focus mode 2"), N_("Focus mode 2"), IfdId::sony1Id, SectionId::makerTags, unsignedByte,
     -1, printFocusMode2},
    {0x201c, "AFAreaModeSetting", N_("Auto focus area mode setting"), N_("Auto focus area mode setting"),
     IfdId::sony1Id, SectionId::makerTags, unsignedByte, -1, printAFAreaModeSetting},
    {0x201d, "FlexibleSpotPosition", N_("Flexible spot position"),
     N_("X and Y co-ordinates or the auto focus point. Only valid when AFAreaModeSetting is Flexible"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, 2, printFlexibleSpotPosition},
    {0x201e, "AFPointSelected", N_("Auto focus point selected"), N_("Auto focus point selected"), IfdId::sony1Id,
     SectionId::makerTags, unsignedByte, -1, printAFPointSelected},
    {0x2020, "AFPointsUsed", N_("Auto focus points used"), N_("Auto focus points used"), IfdId::sony1Id,
     SectionId::makerTags, unsignedByte, -1, printAFPointsUsed},
    {0x2021, "AFTracking", N_("Auto focus tracking"), N_("Auto focus tracking"), IfdId::sony1Id, SectionId::makerTags,
     unsignedByte, -1, printAFTracking},
    {0x2022, "FocalPlaneAFPointsUsed", N_("Focal plane auto focus points used"),
     N_("Focal plane auto focus points used"), IfdId::sony1Id, SectionId::makerTags, unsignedByte, -1,
     printFocalPlaneAFPointsUsed},
    {0x2023, "MultiFrameNREffect", N_("Multi frame noise reduction effect"), N_("Multi frame noise reduction effect"),
     IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyMultiFrameNREffect)},
    {0x2026, "WBShiftABGMPrecise", N_("White balance shift (amber/blue, green/magenta) precise"),
     N_("First number is amber/blue, second is green/magenta"), IfdId::sony1Id, SectionId::makerTags, signedLong, 2,
     printWBShiftABGMPrecise},
    {0x2027, "FocusLocation", N_("Focus location"),
     N_("Location in the image where the camera focused, used for Playback Zoom. Origin is top left with the first "
        "co-ordinate to the right and the second co-ordinate down. If focus location information cannot be obtained, "
        "the center of the image is used"),
     IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x2028, "VariableLowPassFilter", N_("Variable low pass filter"), N_("Variable low pass filter"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_STRING_TAG_2(sonyVariableLowPassFilter)},
    {0x2029, "RAWFileType", N_("RAW file type"), N_("RAW file type"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyRAWFileType)},
    {0x202b, "PrioritySetInAWB", N_("Priority set in automatic white balance"),
     N_("Priority set in automatic white balance"), IfdId::sony1Id, SectionId::makerTags, unsignedByte, -1,
     EXV_PRINT_TAG(sonyPrioritySetInAWB)},
    {0x202c, "MeteringMode2", N_("Metering mode 2"), N_("Metering mode 2"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyMeteringMode2)},
    {0x202d, "ExposureStandardAdjustment", N_("Exposure standard adjustment"), N_("Exposure standard adjustment"),
     IfdId::sony1Id, SectionId::makerTags, signedRational, -1, printExposureStandardAdjustment},
    {0x202e, "Quality2", N_("Quality 2"), N_("Image quality 2"), IfdId::sony1Id, SectionId::makerTags, unsignedShort, 2,
     EXV_PRINT_STRING_TAG_2(sonyQuality2Std)},
    {0x202f, "PixelShiftInfo", N_("Pixel shift info"),
     N_("Pixel shift information consisting of the group ID and shot number"), IfdId::sony1Id, SectionId::makerTags,
     undefined, -1, printPixelShiftInfo},
    {0x2031, "SerialNumber", N_("Serial number"), N_("Serial number"), IfdId::sony1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x2032, "Shadows", N_("Shadows"), N_("Shadows"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1, printValue},
    {0x2033, "Highlights", N_("Highlights"), N_("Highlights"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1,
     printValue},
    {0x2034, "Fade", N_("Fade"), N_("Fade"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1, printValue},
    {0x2035, "SharpnessRange", N_("Sharpness range"), N_("Sharpness range"), IfdId::sony1Id, SectionId::makerTags,
     signedLong, -1, printValue},
    {0x2036, "Clarity", N_("Clarity"), N_("Clarity"), IfdId::sony1Id, SectionId::makerTags, signedLong, -1, printValue},
    {0x2037, "FocusFrameSize", N_("Focus frame size"),
     N_("Width and height of the focus frame, centered on FocusLocation"), IfdId::sony1Id, SectionId::makerTags,
     undefined, -1, printFocusFrameSize},
    {0x2039, "JPEGHEIFSwitch", N_("JPEG/HEIF switch"), N_("JPEG/HEIF switch"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyJPEGHEIFSwitch)},
    {0xb000, "FileFormat", N_("File Format"), N_("File Format"), IfdId::sony1Id, SectionId::makerTags, unsignedByte, -1,
     EXV_PRINT_STRING_TAG_4(sonyFileFormat)},
    {0xb001, "SonyModelID", N_("Sony Model ID"),
     N_("Similar to Exif.Image.Model but can contain multiple entries or conditions"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyModelId)},
    {0xb020, "CreativeStyle", N_("Creative style"), N_("Creative style"), IfdId::sony1Id, SectionId::makerTags,
     asciiString, -1, EXV_PRINT_STRING_TAG_1(sonyCreativeStyleStd)},
    {0xb021, "ColorTemperature", N_("Color Temperature"), N_("Color Temperature"), IfdId::sony1Id, SectionId::makerTags,
     unsignedLong, -1, printColorTemperature},
    {0xb022, "ColorCompensationFilter", N_("Color Compensation Filter"),
     N_("Color Compensation Filter, (green/magenta)"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     printColorCompensationFilter},
    {0xb023, "SceneMode", N_("Scene Mode"), N_("Scene Mode"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     printMinoltaSonySceneMode},
    {0xb024, "ZoneMatching", N_("Zone Matching"), N_("Zone Matching"), IfdId::sony1Id, SectionId::makerTags,
     unsignedLong, -1, printMinoltaSonyZoneMatching},
    {0xb025, "DynamicRangeOptimizer", N_("Dynamic Range Optimizer"), N_("Dynamic Range Optimizer"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(print0xb025)},
    {0xb026, "ImageStabilization", N_("Image stabilization"), N_("Image stabilization"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(sonyOffOnNA2)},
    {0xb027, "LensID", N_("Lens ID"), N_("Lens ID"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     printMinoltaSonyLensID},
    {0xb029, "ColorMode", N_("Color Mode"), N_("Color Mode"), IfdId::sony1Id, SectionId::makerTags, unsignedLong, -1,
     EXV_PRINT_TAG(sonyColorMode)},
    {0xb02a, "LensSpec", N_("Lens specification"),
     N_("Similar to Exif.Photo.LensModel but includes optional additional flags before and after (e.g., DT, Macro). "
        "Works with most Sony cameras"),
     IfdId::sony1Id, SectionId::makerTags, unsignedByte, -1, printLensSpec},
    {0xb02b, "FullImageSize", N_("Full Image Size"), N_("Full image Size (width x height)"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, printImageSize},
    {0xb02c, "PreviewImageSize", N_("Preview image Size"), N_("Preview image size (width x height)"), IfdId::sony1Id,
     SectionId::makerTags, unsignedLong, -1, printImageSize},
    {0xb040, "Macro", N_("Macro"), N_("Macro"), IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(sonyMacro)},
    {0xb041, "ExposureMode", N_("Exposure Mode"), N_("Exposure Mode"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyExposureMode)},
    {0xb042, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1,
     printFocusMode},
    {0xb043, "AFMode", N_("Auto focus area mode"), N_("Auto focus area mode"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, printAFMode},
    {0xb044, "AFIlluminator", N_("Auto focus illuminator"), N_("Auto focus illuminator"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyAFIlluminator)},
    {0xb047, "JPEGQuality", N_("JPEG Quality"), N_("JPEG Quality"), IfdId::sony1Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(sonyJPEGQuality)},
    {0xb048, "FlashLevel", N_("Flash Level"), N_("Flash Level"), IfdId::sony1Id, SectionId::makerTags, signedShort, -1,
     EXV_PRINT_TAG(sonyFlashLevel)},
    {0xb049, "ReleaseMode", N_("Release Mode"), N_("Release Mode"), IfdId::sony1Id, SectionId::makerTags, unsignedShort,
     -1, EXV_PRINT_TAG(sonyReleaseMode)},
    {0xb04a, "SequenceNumber", N_("Sequence Number"), N_("Shot number in continuous burst mode"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG_NO_ERROR(sonySequenceNumber)},
    {0xb04b, "AntiBlur", N_("Anti-Blur"), N_("Anti-Blur"), IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1,
     EXV_PRINT_TAG(sonyAntiBlur)},
    {0xb04e, "FocusMode3", N_("Focus mode 3"), N_("Focus mode 3"), IfdId::sony1Id, SectionId::makerTags, unsignedShort,
     -1, printFocusMode3},
    {0xb04f, "DynamicRangeOptimizer2", N_("Dynamic Range Optimizer 2"), N_("Dynamic Range Optimizer 2"), IfdId::sony1Id,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(print0xb04f)},
    {0xb050, "HighISONoiseReduction2", N_("High ISO noise reduction 2"), N_("High ISO noise reduction 2"),
     IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1, printHighISONoiseReduction2},
    {0xb052, "IntelligentAuto", N_("Intelligent Auto"), N_("Intelligent Auto"), IfdId::sony1Id, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(sonyIntelligentAuto)},
    {0xb054, "WhiteBalance2", N_("White balance 2"),
     N_("White balance 2. Decoding the Fluorescent settings matches the EXIF standard, which is different from "
        "the names used by Sony for some models"),
     IfdId::sony1Id, SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(sonyWhiteBalance2)},
    // End of list marker
    {0xffff, "(UnknownSony1MakerNoteTag)", "(UnknownSony1MakerNoteTag)", N_("Unknown Sony1MakerNote tag"),
     IfdId::sony1Id, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* SonyMakerNote::tagList() {
  return tagInfo_;
}

// -- Sony camera settings ---------------------------------------------------------------

//! Lookup table to translate Sony camera settings drive mode values to readable labels
constexpr TagDetails sonyDriveModeStd[] = {
    {0x01, N_("Single Frame")},
    {0x02, N_("Continuous High")},
    {0x04, N_("Self-timer 10 sec")},
    {0x05, N_("Self-timer 2 sec, Mirror Lock-up")},
    {0x06, N_("Single-frame Bracketing")},
    {0x07, N_("Continuous Bracketing")},
    {0x0a, N_("Remote Commander")},
    {0x0b, N_("Mirror Lock-up")},
    {0x12, N_("Continuous Low")},
    {0x18, N_("White Balance Bracketing Low")},
    {0x19, N_("D-Range Optimizer Bracketing Low")},
    {0x28, N_("White Balance Bracketing High")},
    {0x29, N_("D-Range Optimizer Bracketing High")},
    {0x29, N_("D-Range Optimizer Bracketing High")}  // To silence compiler warning
};

//! Lookup table to translate Sony camera settings focus mode values to readable labels
constexpr TagDetails sonyCSFocusMode[] = {{0, N_("Manual")}, {1, "AF-S"}, {2, "AF-C"}, {3, "AF-A"}};

//! Lookup table to translate Sony camera settings metering mode values to readable labels
constexpr TagDetails sonyMeteringMode[] = {
    {1, N_("Multi-segment")}, {2, N_("Center weighted average")}, {4, N_("Spot")}};

//! Lookup table to translate Sony camera settings creative style values to readable labels
constexpr TagDetails sonyCreativeStyle[] = {
    {1, N_("Standard")},      {2, N_("Vivid")},     {3, N_("Portrait")},
    {4, N_("Landscape")},     {5, N_("Sunset")},    {6, N_("Night View/Portrait")},
    {8, N_("Black & White")}, {9, N_("Adobe RGB")}, {11, N_("Neutral")},
    {12, N_("Clear")},        {13, N_("Deep")},     {14, N_("Light")},
    {15, N_("Autumn")},       {16, N_("Sepia")}};

//! Lookup table to translate Sony camera settings flash mode values to readable labels
constexpr TagDetails sonyFlashMode[] = {
    {0, N_("ADI")},
    {1, N_("TTL")},
};

//! Lookup table to translate Sony AF illuminator values to readable labels
constexpr TagDetails sonyAFIlluminatorCS[] = {{0, N_("Auto")}, {1, N_("Off")}, {0xffff, N_("n/a")}};

//! Lookup table to translate Sony camera settings image style values to readable labels
constexpr TagDetails sonyImageStyle[] = {{1, N_("Standard")},    {2, N_("Vivid")},       {3, N_("Portrait")},
                                         {4, N_("Landscape")},   {5, N_("Sunset")},      {7, N_("Night View/Portrait")},
                                         {8, N_("B&W")},         {9, N_("Adobe RGB")},   {11, N_("Neutral")},
                                         {129, N_("StyleBox1")}, {130, N_("StyleBox2")}, {131, N_("StyleBox3")},
                                         {132, N_("StyleBox4")}, {133, N_("StyleBox5")}, {134, N_("StyleBox6")}};

//! Lookup table to translate Sony camera settings exposure program values to readable labels
constexpr TagDetails sonyExposureProgram[] = {{0, N_("Auto")},
                                              {1, N_("Manual")},
                                              {2, N_("Program AE")},
                                              {3, N_("Aperture-priority AE")},
                                              {4, N_("Shutter speed priority AE")},
                                              {8, N_("Program Shift A")},
                                              {9, N_("Program Shift S")},
                                              {16, N_("Portrait")},
                                              {17, N_("Sports")},
                                              {18, N_("Sunset")},
                                              {19, N_("Night Portrait")},
                                              {20, N_("Landscape")},
                                              {21, N_("Macro")},
                                              {35, N_("Auto No Flash")}};

//! Lookup table to translate Sony camera settings image size values to readable labels
constexpr TagDetails sonyImageSize[] = {{1, N_("Large")}, {2, N_("Medium")}, {3, N_("Small")}};

//! Lookup table to translate Sony aspect ratio values to readable labels
constexpr TagDetails sonyAspectRatio[] = {{1, "3:2"}, {2, "16:9"}};

//! Lookup table to translate Sony exposure level increments values to readable labels
constexpr TagDetails sonyExposureLevelIncrements[] = {{33, "1/3 EV"}, {50, "1/2 EV"}};

// Sony Camera Settings Tag Info
// NOTE: all are for A200, A230, A300, A350, A700, A850 and A900 Sony model excepted
// some entries which are only relevant with A700.

// Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

constexpr TagInfo SonyMakerNote::tagInfoCs_[] = {
    // NOTE: A700 only
    {0x0004, "DriveMode", N_("Drive Mode"), N_("Drive Mode"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(sonyDriveModeStd)},
    // NOTE: A700 only
    {0x0006, "WhiteBalanceFineTune", N_("White Balance Fine Tune"), N_("White Balance Fine Tune"), IfdId::sony1CsId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0010, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(sonyCSFocusMode)},
    {0x0011, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyAFAreaMode},
    {0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"), N_("Local AF Area Point"), IfdId::sony1CsId,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint},
    {0x0015, "MeteringMode", N_("Metering Mode"), N_("Metering Mode"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)},
    {0x0016, "ISOSetting", N_("ISO Setting"), N_("ISO Setting"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0018, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"), N_("Dynamic Range Optimizer Mode"),
     IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode},
    {0x0019, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"), N_("Dynamic Range Optimizer Level"),
     IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x001A, "CreativeStyle", N_("Creative Style"), N_("Creative Style"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)},
    {0x001C, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x001D, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x001E, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x001F, "ZoneMatchingValue", N_("Zone Matching Value"), N_("Zone Matching Value"), IfdId::sony1CsId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0022, "Brightness", N_("Brightness"), N_("Brightness"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x0023, "FlashMode", N_("FlashMode"), N_("FlashMode"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(sonyFlashMode)},
    // NOTE: A700 only
    {0x0028, "PrioritySetupShutterRelease", N_("Priority Setup Shutter Release"), N_("Priority Setup Shutter Release"),
     IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyPrioritySetupShutterRelease},
    // NOTE: A700 only
    {0x0029, "AFIlluminator", N_("AF Illuminator"), N_("AF Illuminator"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyAFIlluminatorCS)},
    // NOTE: A700 only
    {0x002A, "AFWithShutter", N_("AF With Shutter"), N_("AF With Shutter"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyBoolInverseValue},
    // NOTE: A700 only
    {0x002B, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"), N_("Long Exposure Noise Reduction"),
     IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    // NOTE: A700 only
    {0x002C, "HighISONoiseReduction", N_("High ISO NoiseReduction"), N_("High ISO NoiseReduction"), IfdId::sony1CsId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    // NOTE: A700 only
    {0x002D, "ImageStyle", N_("Image Style"), N_("Image Style"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort,
     1, EXV_PRINT_TAG(sonyImageStyle)},
    {0x003C, "ExposureProgram", N_("Exposure Program"), N_("Exposure Program"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)},
    {0x003D, "ImageStabilization", N_("Image Stabilization"), N_("Image Stabilization"), IfdId::sony1CsId,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyBoolValue},
    {0x003F, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyRotation},
    {0x0054, "SonyImageSize", N_("Sony Image Size"), N_("Sony Image Size"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)},
    {0x0055, "AspectRatio", N_("Aspect Ratio"), N_("Aspect Ratio"), IfdId::sony1CsId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyAspectRatio)},
    {0x0056, "Quality", N_("Quality"), N_("Quality"), IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyQualityCs},
    {0x0058, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Exposure Level Increments"),
     IfdId::sony1CsId, SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(sonyExposureLevelIncrements)},
    // End of list marker
    {0xffff, "(UnknownSony1CsTag)", "(UnknownSony1CsTag)", N_("Unknown Sony1 Camera Settings tag"), IfdId::sony1CsId,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* SonyMakerNote::tagListCs() {
  return tagInfoCs_;
}

// -- Sony camera settings 2 ---------------------------------------------------------------

// Sony Camera Settings Tag Version 2 Info
// NOTE: for A330, A380, A450, A500, A550 Sony model

// Warnings: Exiftool database give a list of tags shorted in decimal mode, not hexadecimal.

constexpr TagInfo SonyMakerNote::tagInfoCs2_[] = {
    {0x0010, "FocusMode", N_("Focus Mode"), N_("Focus Mode"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(sonyCSFocusMode)},
    {0x0011, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), IfdId::sony1Cs2Id, SectionId::makerTags,
     unsignedShort, 1, printMinoltaSonyAFAreaMode},
    {0x0012, "LocalAFAreaPoint", N_("Local AF Area Point"), N_("Local AF Area Point"), IfdId::sony1Cs2Id,
     SectionId::makerTags, unsignedShort, 1, printMinoltaSonyLocalAFAreaPoint},
    {0x0013, "MeteringMode", N_("Metering Mode"), N_("Metering Mode"), IfdId::sony1Cs2Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyMeteringMode)},
    {0x0014, "ISOSetting", N_("ISO Setting"), N_("ISO Setting"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0016, "DynamicRangeOptimizerMode", N_("Dynamic Range Optimizer Mode"), N_("Dynamic Range Optimizer Mode"),
     IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1, printMinoltaSonyDynamicRangeOptimizerMode},
    {0x0017, "DynamicRangeOptimizerLevel", N_("Dynamic Range Optimizer Level"), N_("Dynamic Range Optimizer Level"),
     IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0018, "CreativeStyle", N_("Creative Style"), N_("Creative Style"), IfdId::sony1Cs2Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyCreativeStyle)},
    {0x0019, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x001A, "Contrast", N_("Contrast"), N_("Contrast"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {0x001B, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0023, "FlashMode", N_("FlashMode"), N_("FlashMode"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(sonyFlashMode)},
    {0x003C, "ExposureProgram", N_("Exposure Program"), N_("Exposure Program"), IfdId::sony1Cs2Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyExposureProgram)},
    {0x003F, "Rotation", N_("Rotation"), N_("Rotation"), IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1,
     printMinoltaSonyRotation},
    {0x0054, "SonyImageSize", N_("Sony Image Size"), N_("Sony Image Size"), IfdId::sony1Cs2Id, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(sonyImageSize)},
    // End of list marker
    {0xffff, "(UnknownSony1Cs2Tag)", "(UnknownSony1Cs2Tag)", N_("Unknown Sony1 Camera Settings 2 tag"),
     IfdId::sony1Cs2Id, SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* SonyMakerNote::tagListCs2() {
  return tagInfoCs2_;
}

//! Lookup table to translate Sony2Fp AF Area Mode values to readable labels
constexpr TagDetails sony2FpAFAreaMode[] = {{0, N_("Multi")},
                                            {1, N_("Center")},
                                            {2, N_("Spot")},
                                            {3, N_("Flexible Spot")},
                                            {10, N_("Selective (for Miniature effect)")},
                                            {11, N_("Zone")},
                                            {12, N_("Expanded Flexible Spot")},
                                            {14, N_("Tracking")},
                                            {15, N_("Face Tracking")},
                                            {20, N_("Animal Eye Tracking")},
                                            {255, N_("Manual")}};

//! Sony Tag 9402 Sony2Fp (FocusPosition)
constexpr TagInfo SonyMakerNote::tagInfoFp_[] = {
    {0x04, "AmbientTemperature", N_("Ambient temperature"), N_("Temperature of the surroundings (in degrees Celsius)"),
     IfdId::sony2FpId, SectionId::makerTags, signedByte, 1, printSony2FpAmbientTemperature},
    {0x16, "FocusMode", N_("Focus mode"), N_("Focus mode"), IfdId::sony2FpId, SectionId::makerTags, unsignedByte, 1,
     printSony2FpFocusMode},
    {0x17, "AFAreaMode", N_("AF area mode"), N_("Auto focus area mode"), IfdId::sony2FpId, SectionId::makerTags,
     unsignedByte, 1, EXV_PRINT_TAG(sony2FpAFAreaMode)},
    {0x2d, "FocusPosition2", N_("Focus position 2"), N_("Focus position 2"), IfdId::sony2FpId, SectionId::makerTags,
     unsignedByte, 1, printSony2FpFocusPosition2},
    // End of list marker
    {0xffff, "(UnknownSony2FpTag)", "(Unknown Sony2Fp tag)", "(Unknown Sony2Fp tag)", IfdId::sony2FpId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* SonyMakerNote::tagListFp() {
  return tagInfoFp_;
}

std::ostream& SonyMakerNote::printSony2FpAmbientTemperature(std::ostream& os, const Value& value,
                                                            const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  auto pos = metadata->findKey(ExifKey("Exif.Sony2Fp.0x0002"));
  if (pos != metadata->end() && pos->count() == 1 && pos->toInt64() == 255)
    return os << value << " °C";

  return os << N_("n/a");
}

std::ostream& SonyMakerNote::printSony2FpFocusMode(std::ostream& os, const Value& value, const ExifData*) {
  if (value.count() != 1)
    os << value;
  else {
    const auto val = (value.toInt64() & 0x7F);
    switch (val) {
      case 0:
        os << N_("Manual");
        break;
      case 2:
        os << N_("AF-S");
        break;
      case 3:
        os << N_("AF-C");
        break;
      case 4:
        os << N_("AF-A");
        break;
      case 6:
        os << N_("DMF");
        break;
      default:
        os << "(" << val << ")";
    }
  }

  return os;
}

std::ostream& SonyMakerNote::printSony2FpFocusPosition2(std::ostream& os, const Value& value,
                                                        const ExifData* metadata) {
  if (value.count() != 1)
    os << "(" << value << ")";
  else {
    std::string model;
    if (!getModel(metadata, model)) {
      os << "(" << value << ")";
      return os;
    }

    // Ranges of models that do not support this tag
    for (auto& m : {"DSC-", "Stellar"}) {
      if (model.find(m) == 0) {
        os << N_("n/a");
        return os;
      }
    }
    const auto val = value.toInt64();
    if (val == 255)
      os << N_("Infinity");
    else
      os << val;
  }
  return os;
}

//! Sony Tag 9403 SonyMisc1
constexpr TagInfo SonyMakerNote::tagInfoSonyMisc1_[] = {
    {0x05, "CameraTemperature", N_("Camera temperature"), N_("Internal camera temperature (in degrees Celsius)"),
     IfdId::sonyMisc1Id, SectionId::makerTags, signedByte, -1, printSonyMisc1CameraTemperature},
    // End of list marker
    {0xffff, "(UnknownSonyMisc1Tag)", "(UnknownSonyMisc1Tag)", "(UnknownSonyMisc1Tag)", IfdId::sonyMisc1Id,
     SectionId::makerTags, unsignedByte, -1, printValue}};

const TagInfo* SonyMakerNote::tagListSonyMisc1() {
  return tagInfoSonyMisc1_;
}

std::ostream& SonyMakerNote::printSonyMisc1CameraTemperature(std::ostream& os, const Value& value,
                                                             const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  auto pos = metadata->findKey(ExifKey("Exif.SonyMisc1.0x0004"));
  if (pos != metadata->end() && pos->count() == 1 && pos->toInt64() != 0 && pos->toInt64() < 100)
    return os << value << " °C";

  return os << N_("n/a");
}

//! Lookup table to translate Sony Exposure Program 3 values to readable labels
constexpr TagDetails sonyExposureProgram3[] = {{0, N_("Program AE")},
                                               {1, N_("Aperture-priority AE")},
                                               {2, N_("Shutter speed priority AE")},
                                               {3, N_("Manual")},
                                               {4, N_("Auto")},
                                               {5, N_("iAuto")},
                                               {6, N_("Superior Auto")},
                                               {7, N_("iAuto+")},
                                               {8, N_("Portrait")},
                                               {9, N_("Landscape")},
                                               {10, N_("Twilight")},
                                               {11, N_("Twilight Portrait")},
                                               {12, N_("Sunset")},
                                               {14, N_("Action (High speed)")},
                                               {16, N_("Sports")},
                                               {17, N_("Handheld Night Shot")},
                                               {18, N_("Anti Motion Blur")},
                                               {19, N_("High Sensitivity")},
                                               {21, N_("Beach")},
                                               {22, N_("Snow")},
                                               {23, N_("Fireworks")},
                                               {26, N_("Underwater")},
                                               {27, N_("Gourmet")},
                                               {28, N_("Pet")},
                                               {29, N_("Macro")},
                                               {30, N_("Backlight Correction HDR")},
                                               {33, N_("Sweep Panorama")},
                                               {36, N_("Background Defocus")},
                                               {37, N_("Soft Skin")},
                                               {42, N_("3D Image")},
                                               {43, N_("Cont. Priority AE")},
                                               {45, N_("Document")},
                                               {46, N_("Party")}};

//! Sony Tag 9404b SonyMisc2b tags
constexpr TagInfo SonyMakerNote::tagInfoSonyMisc2b_[] = {
    {12, "ExposureProgram", N_("Exposure program"), N_("Exposure program"), IfdId::sonyMisc2bId, SectionId::makerTags,
     unsignedByte, -1, EXV_PRINT_TAG(sonyExposureProgram3)},
    {14, "IntelligentAuto", N_("Intelligent auto"), N_("Whether intelligent auto was used"), IfdId::sonyMisc2bId,
     SectionId::makerTags, unsignedByte, -1, printMinoltaSonyBoolValue},
    {30, "LensZoomPosition", N_("Lens zoom position"), N_("Lens zoom position (in %)"), IfdId::sonyMisc2bId,
     SectionId::makerTags, unsignedShort, -1, printSonyMisc2bLensZoomPosition},
    {32, "FocusPosition2", N_("Focus position 2"), N_("Focus position 2"), IfdId::sonyMisc2bId, SectionId::makerTags,
     unsignedByte, -1, printSonyMisc2bFocusPosition2},
    // End of list marker
    {0xffff, "(UnknownSonyMisc2bTag)", "(Unknown SonyMisc2b tag)", "(Unknown SonyMisc2b tag)", IfdId::sonyMisc2bId,
     SectionId::makerTags, unsignedByte, -1, printValue}};

const TagInfo* SonyMakerNote::tagListSonyMisc2b() {
  return tagInfoSonyMisc2b_;
}

std::ostream& SonyMakerNote::printSonyMisc2bLensZoomPosition(std::ostream& os, const Value& value,
                                                             const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  // Models that do not support this tag
  for (auto& m : {"SLT-", "HV", "ILCA-"}) {
    if (model.find(m) != std::string::npos)
      return os << N_("n/a");
  }

  os << std::round(value.toInt64() / 10.24) << "%";

  return os;
}

std::ostream& SonyMakerNote::printSonyMisc2bFocusPosition2(std::ostream& os, const Value& value,
                                                           const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  // Models that do not support this tag
  for (auto& m : {"SLT-", "HV", "ILCA-"}) {
    if (model.find(m) != std::string::npos)
      return os << N_("n/a");
  }

  return os << value;
}

//! Lookup table to translate Sony camera SonyMisc3c sequence length 1 values to readable labels
constexpr TagDetails sonyMisc3cSequenceLength1[] = {{0, N_("Continuous")},
                                                    {1, N_("1 shot")},
                                                    {2, N_("2 shots")},
                                                    {3, N_("3 shots")},
                                                    {4, N_("4 shots")},
                                                    {5, N_("5 shots")},
                                                    {6, N_("6 shots")},
                                                    {7, N_("7 shots")},
                                                    {9, N_("9 shots")},
                                                    {10, N_("10 shots")},
                                                    {12, N_("12 shots")},
                                                    {16, N_("16 shots")},
                                                    {100, N_("Continuous - iSweep Panorama")},
                                                    {200, N_("Continuous - Sweep Panorama")}};

//! Lookup table to translate Sony camera SonyMisc3c sequence length 2 values to readable labels
constexpr TagDetails sonyMisc3cSequenceLength2[] = {{0, N_("Continuous")}, {1, N_("1 file")},   {2, N_("2 files")},
                                                    {3, N_("3 files")},    {5, N_("5 files")},  {7, N_("7 files")},
                                                    {9, N_("9 files")},    {10, N_("10 files")}};

//! Lookup table to translate Sony camera SonyMisc3c, camera orientation values to readable labels
constexpr TagDetails sonyMisc3cCameraOrientation[] = {
    {1, N_("Horizontal (normal)")}, {3, N_("Rotate 180°")}, {6, N_("Rotate 90° CW")}, {8, N_("Rotate 270° CW")}};

//! SonyMisc3c tags (Tag 9400c)
constexpr TagInfo SonyMakerNote::tagInfoSonyMisc3c_[] = {
    {9, "ReleaseMode2", N_("Release mode 2"), N_("Release mode 2"), IfdId::sonyMisc3cId, SectionId::makerTags,
     unsignedByte, -1, EXV_PRINT_TAG(sonyReleaseMode2)},
    {10, "ShotNumberSincePowerUp", N_("Shot number since power up"),
     N_("Number of photos taken since the camera was powered up"), IfdId::sonyMisc3cId, SectionId::makerTags,
     unsignedLong, -1, printSonyMisc3cShotNumberSincePowerUp},
    {18, "SequenceImageNumber", N_("Sequence image number"), N_("Number of images captured in burst sequence"),
     IfdId::sonyMisc3cId, SectionId::makerTags, unsignedLong, -1, printSonyMisc3cSequenceNumber},
    // In Exiftool, "SequenceLength1" is called "SequenceLength. Renamed due to clash of names."
    {22, "SequenceLength1", N_("Sequence length 1"), N_("Length of the sequence of photos taken"), IfdId::sonyMisc3cId,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cSequenceLength1)},
    {26, "SequenceFileNumber", N_("Sequence file number"), N_("File number in burst sequence"), IfdId::sonyMisc3cId,
     SectionId::makerTags, unsignedLong, -1, printSonyMisc3cSequenceNumber},
    // In Exiftool, "SequenceLength2" is called "SequenceLength". Renamed due to clash of names."
    {30, "SequenceLength2", N_("Sequence length 2"), N_("Length of the sequence of photos taken"), IfdId::sonyMisc3cId,
     SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cSequenceLength2)},
    {41, "CameraOrientation", N_("Camera orientation"), N_("Orientation of the camera when the photo was taken"),
     IfdId::sonyMisc3cId, SectionId::makerTags, unsignedByte, -1, EXV_PRINT_TAG(sonyMisc3cCameraOrientation)},
    {42, "Quality2", N_("Quality 2"), N_("Quality 2"), IfdId::sonyMisc3cId, SectionId::makerTags, unsignedByte, -1,
     printSonyMisc3cQuality2},
    {71, "SonyImageHeight", N_("Sony image height"), N_("Height of the image"), IfdId::sonyMisc3cId,
     SectionId::makerTags, unsignedShort, -1, printSonyMisc3cSonyImageHeight},
    {83, "ModelReleaseYear", N_("Model release year"), N_("Year that the model of camera was released"),
     IfdId::sonyMisc3cId, SectionId::makerTags, unsignedByte, -1, printSonyMisc3cModelReleaseYear},
    // End of list marker
    {0xffff, "(UnknownSonyMisc3c)", "(Unknown SonyMisc3c Tag)", N_("Unknown SonyMisc23 tag"), IfdId::sonyMisc3cId,
     SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* SonyMakerNote::tagListSonyMisc3c() {
  return tagInfoSonyMisc3c_;
}

std::ostream& SonyMakerNote::printSonyMisc3cShotNumberSincePowerUp(std::ostream& os, const Value& value,
                                                                   const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  // Models that support this tag
  static constexpr auto models = std::array{
      "ILCA-68",     "ILCA-77M2",   "ILCA-99M2",  "ILCE-5000", "ILCE-5100",  "ILCE-6000",  "ILCE-6300",
      "ILCE-6500",   "ILCE-7",      "ILCE-7M2",   "ILCE-7R",   "ILCE-7RM2",  "ILCE-7S",    "ILCE-7SM2",
      "ILCE-QX1",    "DSC-HX350",   "DSC-HX400V", "DSC-HX60V", "DSC-HX80",   "DSC-HX90",   "DSC-HX90V",
      "DSC-QX30",    "DSC-RX0",     "DSC-RX1RM2", "DSC-RX10",  "DSC-RX10M2", "DSC-RX10M3", "DSC-RX100M3",
      "DSC-RX100M4", "DSC-RX100M5", "DSC-WX220",  "DSC-WX350", "DSC-WX500",
  };

  bool f = std::find(models.begin(), models.end(), model) != models.end();
  if (f)
    return os << value.toInt64();
  return os << N_("n/a");
}

std::ostream& SonyMakerNote::printSonyMisc3cSequenceNumber(std::ostream& os, const Value& value, const ExifData*) {
  return (value.count() != 1) ? os << "(" << value << ")" : os << (value.toInt64() + 1);
}

std::ostream& SonyMakerNote::printSonyMisc3cQuality2(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  const auto val = value.toInt64();

  // Value is interpreted differently if model is in list or not
  for (auto& m : {"ILCE-1", "ILCE-7SM3", "ILME-FX3"}) {
    if (m == model) {
      switch (val) {
        case 1:
          return os << N_("JPEG");
        case 2:
          return os << N_("Raw");
        case 3:
          return os << N_("Raw + JPEG");
        case 4:
          return os << N_("HEIF");
        case 6:
          return os << N_("Raw + HEIF");
        default:
          return os << "(" << val << ")";
      }
    }
  }

  switch (val) {
    case 0:
      return os << N_("JPEG");
    case 1:
      return os << N_("Raw");
    case 2:
      return os << N_("Raw + JPEG");
    case 3:
      return os << N_("Raw + MPO");
    default:
      os << "(" << val << ")";
  }
  return os;
}

std::ostream& SonyMakerNote::printSonyMisc3cSonyImageHeight(std::ostream& os, const Value& value,
                                                            const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  // Models that do not support this tag
  const auto models = std::array{"ILCE-1", "ILCE-7SM3", "ILME-FX3"};
  bool f = std::find(models.begin(), models.end(), model) != models.end();
  if (f)
    return os << N_("n/a");

  const auto val = value.toInt64();
  return val > 0 ? os << (8 * val) : os << N_("n/a");
}

std::ostream& SonyMakerNote::printSonyMisc3cModelReleaseYear(std::ostream& os, const Value& value,
                                                             const ExifData* metadata) {
  if (value.count() != 1)
    return os << "(" << value << ")";

  std::string model;
  if (!getModel(metadata, model)) {
    os << "(" << value << ")";
    return os;
  }

  // Models that do not support this tag
  const auto models = std::array{"ILCE-1", "ILCE-7SM3", "ILME-FX3"};
  bool f = std::find(models.begin(), models.end(), model) != models.end();
  if (f)
    return os << N_("n/a");

  const auto val = value.toInt64();
  if (val > 99)
    return os << "(" << val << ")";

  if (val == 0)
    return os << "2000";

  return os << "20" << val;
}

//! Sony SInfo1 tags (ShotInfo - Tag 3000)
constexpr TagInfo SonyMakerNote::tagInfoSonySInfo1_[] = {
    // TODO: As "FaceInfo1" (72) and "FaceInfo2" (94) are not added, including
    //       "FaceInfoOffset" (2) and "FaceInfoLength" (50) does not make sense.
    //       The values are all connected and changing one without the rest will
    //       corrupt the data.
    {6, "SonyDateTime", N_("Sony date/time"), N_("Date and time when the photo was captured"), IfdId::sonySInfo1Id,
     SectionId::makerTags, asciiString, -1, printValue},
    {26, "SonyImageHeight", N_("Sony image height"), N_("Height of the image"), IfdId::sonySInfo1Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {28, "SonyImageWidth", N_("Sony image width"), N_("Width of the image"), IfdId::sonySInfo1Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {48, "FacesDetected", N_("Faces detected"), N_("Number of faces detected in the image"), IfdId::sonySInfo1Id,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {52, "MetaVersion", N_("Meta version"), N_("Sony meta version"), IfdId::sonySInfo1Id, SectionId::makerTags,
     asciiString, -1, printValue},
    // TODO: Add FaceInfo1 (72) and FaceInfo2 (94) which are sub-groups of tags.
    // End of list marker
    {0xffff, "(UnknownsonySInfo1Tag)", "(Unknown SonySInfo1 Tag)", "(Unknown SonySInfo1 Tag)", IfdId::sonySInfo1Id,
     SectionId::makerTags, unsignedByte, -1, printValue}};

const TagInfo* SonyMakerNote::tagListSonySInfo1() {
  return tagInfoSonySInfo1_;
}

//! Sony Tag 2010 Sony2010 (Miscellaneous)
constexpr TagInfo SonyMakerNote::tagInfo2010e_[] = {
    {0, "SequenceImageNumber", N_("Sequence Image Number"), N_("Sequence Image Number"), IfdId::sony2010eId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {4, "SequenceFileNumber", N_("SequenceFileNumber"), N_("SequenceFileNumber"), IfdId::sony2010eId,
     SectionId::makerTags, unsignedLong, 1, printValue},
    {8, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), IfdId::sony2010eId, SectionId::makerTags, unsignedLong,
     1, printValue},
    {540, "DigitalZoomRatio", N_("DigitalZoomRatio"), N_("DigitalZoomRatio"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {556, "SonyDateTime", N_("SonyDateTime"), N_("SonyDateTime"), IfdId::sony2010eId, SectionId::makerTags, undefined,
     1, printValue},
    {808, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), IfdId::sony2010eId,
     SectionId::makerTags, unsignedByte, 1, printValue},
    {1208, "MeterInfo", N_("MeterInfo"), N_("MeterInfo"), IfdId::sony2010eId, SectionId::makerTags, undefined, 1,
     printValue},
    {4444, "ReleaseMode3", N_("ReleaseMode3"), N_("ReleaseMode3"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4448, "ReleaseMode2", N_("ReleaseMode2"), N_("ReleaseMode2"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4456, "SelfTimer", N_("SelfTimer"), N_("SelfTimer"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {4460, "FlashMode", N_("FlashMode"), N_("FlashMode"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {4466, "StopsAboveBaseISO", N_("StopsAboveBaseISO"), N_("StopsAboveBaseISO"), IfdId::sony2010eId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {4468, "BrightnessValue", N_("BrightnessValue"), N_("BrightnessValue"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {4472, "DynamicRangeOptimizer", N_("DynamicRangeOptimizer"), N_("DynamicRangeOptimizer"), IfdId::sony2010eId,
     SectionId::makerTags, unsignedByte, 1, printValue},
    {4476, "HDRSetting", N_("HDRSetting"), N_("HDRSetting"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {4480, "ExposureCompensation", N_("ExposureCompensation"), N_("ExposureCompensation"), IfdId::sony2010eId,
     SectionId::makerTags, signedShort, 1, printValue},
    {4502, "PictureProfile", N_("PictureProfile"), N_("PictureProfile"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4503, "PictureProfile2", N_("PictureProfile2"), N_("PictureProfile2"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4507, "PictureEffect2", N_("PictureEffect2"), N_("PictureEffect2"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4520, "Quality2", N_("Quality2"), N_("Quality2"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {4524, "MeteringMode", N_("MeteringMode"), N_("MeteringMode"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4525, "ExposureProgram", N_("ExposureProgram"), N_("ExposureProgram"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedByte, 1, printValue},
    {4532, "WB_RGBLevels", N_("WB_RGBLevels"), N_("WB_RGBLevels"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedShort, 3, printValue},
    {4692, "SonyISO", N_("SonyISO"), N_("SonyISO"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {4696, "SonyISO2", N_("SonyISO2"), N_("SonyISO2"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {4728, "FocalLength", N_("FocalLength"), N_("FocalLength"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {4730, "MinFocalLength", N_("MinFocalLength"), N_("MinFocalLength"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {4732, "MaxFocalLength", N_("MaxFocalLength"), N_("MaxFocalLength"), IfdId::sony2010eId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {4736, "SonyISO3", N_("SonyISO3"), N_("SonyISO3"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {6256, "DistortionCorrParams", N_("DistortionCorrParams"), N_("DistortionCorrParams"), IfdId::sony2010eId,
     SectionId::makerTags, signedShort, 16, printValue},
    {6289, "LensFormat", N_("LensFormat"), N_("LensFormat"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {6290, "LensMount", N_("LensMount"), N_("LensMount"), IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1,
     printValue},
    {6291, "LensType2", N_("LensType2"), N_("LensType2"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {6294, "LensType", N_("LensType"), N_("LensType"), IfdId::sony2010eId, SectionId::makerTags, unsignedShort, 1,
     printValue},
    {6296, "DistortionCorrParamsPresent", N_("DistortionCorrParamsPresent"), N_("DistortionCorrParamsPresent"),
     IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1, printValue},
    {6297, "DistortionCorrParamsNumber", N_("DistortionCorrParamsNumber"), N_("DistortionCorrParamsNumber"),
     IfdId::sony2010eId, SectionId::makerTags, unsignedByte, 1, printValue},
    // End of list marker
    {0xffff, "(UnknownSony2010eTag)", "(UnknownSony2010eTag)", "(UnknownSony2010eTag)", IfdId::sony2010eId,
     SectionId::makerTags, unsignedByte, 1, printValue},
};

const TagInfo* SonyMakerNote::tagList2010e() {
  return tagInfo2010e_;
}

// https://github.com/Exiv2/exiv2/pull/906#issuecomment-504338797
static DataBuf sonyTagCipher(uint16_t /* tag */, const byte* bytes, size_t size, TiffComponent* /*object*/,
                             bool bDecipher) {
  DataBuf b(bytes, size);  // copy the data

  // initialize the code table
  byte code[256];
  for (uint32_t i = 0; i < 249; i++) {
    if (bDecipher) {
      code[(i * i * i) % 249] = i;
    } else {
      code[i] = (i * i * i) % 249;
    }
  }
  for (uint32_t i = 249; i < 256; i++) {
    code[i] = i;
  }

  // code byte-by-byte
  for (uint32_t i = 0; i < size; i++) {
    b.write_uint8(i, code[bytes[i]]);
  }

  return b;
}

DataBuf sonyTagDecipher(uint16_t tag, const byte* bytes, size_t size, TiffComponent* object) {
  return sonyTagCipher(tag, bytes, size, object, true);
}
DataBuf sonyTagEncipher(uint16_t tag, const byte* bytes, size_t size, TiffComponent* object) {
  return sonyTagCipher(tag, bytes, size, object, false);
}

}  // namespace Exiv2::Internal
