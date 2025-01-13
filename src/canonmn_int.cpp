// SPDX-License-Identifier: GPL-2.0-or-later
/*
  File:      canonmn.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             David Cannings (dc) <david@edeca.net>
             Andi Clemens (ac) <andi.clemens@gmx.net>
 */
// included header files
#include "canonmn_int.hpp"

#include "error.hpp"
#include "exif.hpp"
#include "i18n.h"  // NLS support.
#include "makernote_int.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "value.hpp"

// + standard includes
#include <cmath>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
//! OffOn, multiple tags
constexpr TagDetails canonOffOn[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

std::ostream& printCsLensTypeByMetadata(std::ostream& os, const Value& value, const ExifData* metadata);

//! Special treatment pretty-print function for non-unique lens ids.
std::ostream& printCsLensFFFF(std::ostream& os, const Value& value, const ExifData* metadata);

//! ModelId, tag 0x0010
constexpr TagDetails canonModelId[] = {
    {0x00000412, "EOS M50 / Kiss M"},
    {0x00000801, "PowerShot SX740 HS"},
    {0x00000804, "Powershot G5 X Mark II"},
    {0x00000805, "PowerShot SX70 HS"},
    {0x00000808, "PowerShot G7 X Mark III"},
    {0x00000811, "EOS M6 Mark II"},
    {0x00000812, "EOS M200"},
    {0x01010000, "PowerShot A30"},
    {0x01040000, "PowerShot S300 / Digital IXUS 300 / IXY Digital 300"},
    {0x01060000, "PowerShot A20"},
    {0x01080000, "PowerShot A10"},
    {0x01090000, "PowerShot S110 / Digital IXUS v / IXY Digital 200"},
    {0x01100000, "PowerShot G2"},
    {0x01110000, "PowerShot S40"},
    {0x01120000, "PowerShot S30"},
    {0x01130000, "PowerShot A40"},
    {0x01140000, "EOS D30"},
    {0x01150000, "PowerShot A100"},
    {0x01160000, "PowerShot S200 / Digital IXUS v2 / IXY Digital 200a"},
    {0x01170000, "PowerShot A200"},
    {0x01180000, "PowerShot S330 / Digital IXUS 330 / IXY Digital 300a"},
    {0x01190000, "PowerShot G3"},
    {0x01210000, "PowerShot S45"},
    {0x01230000, "PowerShot SD100 / Digital IXUS II / IXY Digital 30"},
    {0x01240000, "PowerShot S230 / Digital IXUS v3 / IXY Digital 320"},
    {0x01250000, "PowerShot A70"},
    {0x01260000, "PowerShot A60"},
    {0x01270000, "PowerShot S400 / Digital IXUS 400 / IXY Digital 400"},
    {0x01290000, "PowerShot G5"},
    {0x01300000, "PowerShot A300"},
    {0x01310000, "PowerShot S50"},
    {0x01340000, "PowerShot A80"},
    {0x01350000, "PowerShot SD10 / Digital IXUS i / IXY Digital L"},
    {0x01360000, "PowerShot S1 IS"},
    {0x01370000, "PowerShot Pro1"},
    {0x01380000, "PowerShot S70"},
    {0x01390000, "PowerShot S60"},
    {0x01400000, "PowerShot G6"},
    {0x01410000, "PowerShot S500 / Digital IXUS 500 / IXY Digital 500"},
    {0x01420000, "PowerShot A75"},
    {0x01440000, "PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a"},
    {0x01450000, "PowerShot A400"},
    {0x01470000, "PowerShot A310"},
    {0x01490000, "PowerShot A85"},
    {0x01520000, "PowerShot S410 / Digital IXUS 430 / IXY Digital 450"},
    {0x01530000, "PowerShot A95"},
    {0x01540000, "PowerShot SD300 / Digital IXUS 40 / IXY Digital 50"},
    {0x01550000, "PowerShot SD200 / Digital IXUS 30 / IXY Digital 40"},
    {0x01560000, "PowerShot A520"},
    {0x01570000, "PowerShot A510"},
    {0x01590000, "PowerShot SD20 / Digital IXUS i5 / IXY Digital L2"},
    {0x01640000, "PowerShot S2 IS"},
    {0x01650000, "PowerShot SD430 / Digital IXUS Wireless / IXY Digital Wireless"},
    {0x01660000, "PowerShot SD500 / Digital IXUS 700 / IXY Digital 600"},
    {0x01668000, "EOS D60"},
    {0x01700000, "PowerShot SD30 / Digital IXUS i Zoom / IXY Digital L3"},
    {0x01740000, "PowerShot A430"},
    {0x01750000, "PowerShot A410"},
    {0x01760000, "PowerShot S80"},
    {0x01780000, "PowerShot A620"},
    {0x01790000, "PowerShot A610"},
    {0x01800000, "PowerShot SD630 / Digital IXUS 65 / IXY Digital 80"},
    {0x01810000, "PowerShot SD450 / Digital IXUS 55 / IXY Digital 60"},
    {0x01820000, "PowerShot TX1"},
    {0x01870000, "PowerShot SD400 / Digital IXUS 50 / IXY Digital 55"},
    {0x01880000, "PowerShot A420"},
    {0x01890000, "PowerShot SD900 / Digital IXUS 900 Ti / IXY Digital 1000"},
    {0x01900000, "PowerShot SD550 / Digital IXUS 750 / IXY Digital 700"},
    {0x01920000, "PowerShot A700"},
    {0x01940000, "PowerShot SD700 IS / Digital IXUS 800 IS / IXY Digital 800 IS"},
    {0x01950000, "PowerShot S3 IS"},
    {0x01960000, "PowerShot A540"},
    {0x01970000, "PowerShot SD600 / Digital IXUS 60 / IXY Digital 70"},
    {0x01980000, "PowerShot G7"},
    {0x01990000, "PowerShot A530"},
    {0x02000000, "PowerShot SD800 IS / Digital IXUS 850 IS / IXY Digital 900 IS"},
    {0x02010000, "PowerShot SD40 / Digital IXUS i7 / IXY Digital L4"},
    {0x02020000, "PowerShot A710 IS"},
    {0x02030000, "PowerShot A640"},
    {0x02040000, "PowerShot A630"},
    {0x02090000, "PowerShot S5 IS"},
    {0x02100000, "PowerShot A460"},
    {0x02120000, "PowerShot SD850 IS / Digital IXUS 950 IS / IXY Digital 810 IS"},
    {0x02130000, "PowerShot A570 IS"},
    {0x02140000, "PowerShot A560"},
    {0x02150000, "PowerShot SD750 / Digital IXUS 75 / IXY Digital 90"},
    {0x02160000, "PowerShot SD1000 / Digital IXUS 70 / IXY Digital 10"},
    {0x02180000, "PowerShot A550"},
    {0x02190000, "PowerShot A450"},
    {0x02230000, "PowerShot G9"},
    {0x02240000, "PowerShot A650 IS"},
    {0x02260000, "PowerShot A720 IS"},
    {0x02290000, "PowerShot SX100 IS"},
    {0x02300000, "PowerShot SD950 IS / Digital IXUS 960 IS / IXY Digital 2000 IS"},
    {0x02310000, "PowerShot SD870 IS / Digital IXUS 860 IS / IXY Digital 910 IS"},
    {0x02320000, "PowerShot SD890 IS / Digital IXUS 970 IS / IXY Digital 820 IS"},
    {0x02360000, "PowerShot SD790 IS / Digital IXUS 90 IS / IXY Digital 95 IS"},
    {0x02370000, "PowerShot SD770 IS / Digital IXUS 85 IS / IXY Digital 25 IS"},
    {0x02380000, "PowerShot A590 IS"},
    {0x02390000, "PowerShot A580"},
    {0x02420000, "PowerShot A470"},
    {0x02430000, "PowerShot SD1100 IS / Digital IXUS 80 IS / IXY Digital 20 IS"},
    {0x02460000, "PowerShot SX1 IS"},
    {0x02470000, "PowerShot SX10 IS"},
    {0x02480000, "PowerShot A1000 IS"},
    {0x02490000, "PowerShot G10"},
    {0x02510000, "PowerShot A2000 IS"},
    {0x02520000, "PowerShot SX110 IS"},
    {0x02530000, "PowerShot SD990 IS / Digital IXUS 980 IS / IXY Digital 3000 IS"},
    {0x02540000, "PowerShot SD880 IS / Digital IXUS 870 IS / IXY Digital 920 IS"},
    {0x02550000, "PowerShot E1"},
    {0x02560000, "PowerShot D10"},
    {0x02570000, "PowerShot SD960 IS / Digital IXUS 110 IS / IXY Digital 510 IS"},
    {0x02580000, "PowerShot A2100 IS"},
    {0x02590000, "PowerShot A480"},
    {0x02600000, "PowerShot SX200 IS"},
    {0x02610000, "PowerShot SD970 IS / Digital IXUS 990 IS / IXY Digital 830 IS"},
    {0x02620000, "PowerShot SD780 IS / Digital IXUS 100 IS / IXY Digital 210 IS"},
    {0x02630000, "PowerShot A1100 IS"},
    {0x02640000, "PowerShot SD1200 IS / Digital IXUS 95 IS / IXY Digital 110 IS"},
    {0x02700000, "PowerShot G11"},
    {0x02710000, "PowerShot SX120 IS"},
    {0x02720000, "PowerShot S90"},
    {0x02750000, "PowerShot SX20 IS"},
    {0x02760000, "PowerShot SD980 IS / Digital IXUS 200 IS / IXY Digital 930 IS"},
    {0x02770000, "PowerShot SD940 IS / Digital IXUS 120 IS / IXY Digital 220 IS"},
    {0x02800000, "PowerShot A495"},
    {0x02810000, "PowerShot A490"},
    {0x02820000, "PowerShot A3100/A3150 IS"},
    {0x02830000, "PowerShot A3000 IS"},
    {0x02840000, "PowerShot SD1400 IS / IXUS 130 / IXY 400F"},
    {0x02850000, "PowerShot SD1300 IS / IXUS 105 / IXY 200F"},
    {0x02860000, "PowerShot SD3500 IS / IXUS 210 / IXY 10S"},
    {0x02870000, "PowerShot SX210 IS"},
    {0x02880000, "PowerShot SD4000 IS / IXUS 300 HS / IXY 30S"},
    {0x02890000, "PowerShot SD4500 IS / IXUS 1000 HS / IXY 50S"},
    {0x02920000, "PowerShot G12"},
    {0x02930000, "PowerShot SX30 IS"},
    {0x02940000, "PowerShot SX130 IS"},
    {0x02950000, "PowerShot S95"},
    {0x02980000, "PowerShot A3300 IS"},
    {0x02990000, "PowerShot A3200 IS"},
    {0x03000000, "PowerShot ELPH 500 HS / IXUS 310 HS / IXY 31S"},
    {0x03010000, "PowerShot Pro90 IS"},
    {0x03010001, "PowerShot A800"},
    {0x03020000, "PowerShot ELPH 100 HS / IXUS 115 HS / IXY 210F"},
    {0x03030000, "PowerShot SX230 HS"},
    {0x03040000, "PowerShot ELPH 300 HS / IXUS 220 HS / IXY 410F"},
    {0x03050000, "PowerShot A2200"},
    {0x03060000, "PowerShot A1200"},
    {0x03070000, "PowerShot SX220 HS"},
    {0x03080000, "PowerShot G1 X"},
    {0x03090000, "PowerShot SX150 IS"},
    {0x03100000, "PowerShot ELPH 510 HS / IXUS 1100 HS / IXY 51S"},
    {0x03110000, "PowerShot S100 (new)"},
    {0x03120000, "PowerShot ELPH 310 HS / IXUS 230 HS / IXY 600F"},
    {0x03130000, "PowerShot SX40 HS"},
    {0x03140000, "IXY 32S"},
    {0x03160000, "PowerShot A1300"},
    {0x03170000, "PowerShot A810"},
    {0x03180000, "PowerShot ELPH 320 HS / IXUS 240 HS / IXY 420F"},
    {0x03190000, "PowerShot ELPH 110 HS / IXUS 125 HS / IXY 220F"},
    {0x03200000, "PowerShot D20"},
    {0x03210000, "PowerShot A4000 IS"},
    {0x03220000, "PowerShot SX260 HS"},
    {0x03230000, "PowerShot SX240 HS"},
    {0x03240000, "PowerShot ELPH 530 HS / IXUS 510 HS / IXY 1"},
    {0x03250000, "PowerShot ELPH 520 HS / IXUS 500 HS / IXY 3"},
    {0x03260000, "PowerShot A3400 IS"},
    {0x03270000, "PowerShot A2400 IS"},
    {0x03280000, "PowerShot A2300"},
    {0x03320000, "PowerShot S100V"},
    {0x03330000, "PowerShot G15"},
    {0x03340000, "PowerShot SX50 HS"},
    {0x03350000, "PowerShot SX160 IS"},
    {0x03360000, "PowerShot S110 (new)"},
    {0x03370000, "PowerShot SX500 IS"},
    {0x03380000, "PowerShot N"},
    {0x03390000, "IXUS 245 HS / IXY 430F"},
    {0x03400000, "PowerShot SX280 HS"},
    {0x03410000, "PowerShot SX270 HS"},
    {0x03420000, "PowerShot A3500 IS"},
    {0x03430000, "PowerShot A2600"},
    {0x03440000, "PowerShot SX275 HS"},
    {0x03450000, "PowerShot A1400"},
    {0x03460000, "PowerShot ELPH 130 IS / IXUS 140 / IXY 110F"},
    {0x03470000, "PowerShot ELPH 115/120 IS / IXUS 132/135 / IXY 90F/100F"},
    {0x03490000, "PowerShot ELPH 330 HS / IXUS 255 HS / IXY 610F"},
    {0x03510000, "PowerShot A2500"},
    {0x03540000, "PowerShot G16"},
    {0x03550000, "PowerShot S120"},
    {0x03560000, "PowerShot SX170 IS"},
    {0x03580000, "PowerShot SX510 HS"},
    {0x03590000, "PowerShot S200 (new)"},
    {0x03600000, "IXY 620F"},
    {0x03610000, "PowerShot N100"},
    {0x03640000, "PowerShot G1 X Mark II"},
    {0x03650000, "PowerShot D30"},
    {0x03660000, "PowerShot SX700 HS"},
    {0x03670000, "PowerShot SX600 HS"},
    {0x03680000, "PowerShot ELPH 140 IS / IXUS 150 / IXY 130"},
    {0x03690000, "PowerShot ELPH 135 / IXUS 145 / IXY 120"},
    {0x03700000, "PowerShot ELPH 340 HS / IXUS 265 HS / IXY 630"},
    {0x03710000, "PowerShot ELPH 150 IS / IXUS 155 / IXY 140"},
    {0x03740000, "EOS M3"},
    {0x03750000, "PowerShot SX60 HS"},
    {0x03760000, "PowerShot SX520 HS"},
    {0x03770000, "PowerShot SX400 IS"},
    {0x03780000, "PowerShot G7 X"},
    {0x03790000, "PowerShot N2"},
    {0x03800000, "PowerShot SX530 HS"},
    {0x03820000, "PowerShot SX710 HS"},
    {0x03830000, "PowerShot SX610 HS"},
    {0x03840000, "EOS M10"},
    {0x03850000, "PowerShot G3 X"},
    {0x03860000, "PowerShot ELPH 165 HS / IXUS 165 / IXY 160"},
    {0x03870000, "PowerShot ELPH 160 / IXUS 160"},
    {0x03880000, "PowerShot ELPH 350 HS / IXUS 275 HS / IXY 640"},
    {0x03890000, "PowerShot ELPH 170 IS / IXUS 170"},
    {0x03910000, "PowerShot SX410 IS"},
    {0x03930000, "PowerShot G9 X"},
    {0x03940000, "EOS M5"},
    {0x03950000, "PowerShot G5 X"},
    {0x03970000, "PowerShot G7 X Mark II"},
    {0x03980000, "EOS M100"},
    {0x03990000, "PowerShot ELPH 360 HS / IXUS 285 HS / IXY 650"},
    {0x04010000, "PowerShot SX540 HS"},
    {0x04020000, "PowerShot SX420 IS"},
    {0x04030000, "PowerShot ELPH 190 IS / IXUS 180 / IXY 190"},
    {0x04040000, "PowerShot G1"},
    {0x04040001, "PowerShot ELPH 180 IS / IXUS 175 / IXY 180"},
    {0x04050000, "PowerShot SX720 HS"},
    {0x04060000, "PowerShot SX620 HS"},
    {0x04070000, "EOS M6"},
    {0x04100000, "PowerShot G9 X Mark II"},
    {0x04150000, "PowerShot ELPH 185 / IXUS 185 / IXY 200"},
    {0x04160000, "PowerShot SX430 IS"},
    {0x04170000, "PowerShot SX730 HS"},
    {0x04180000, "PowerShot G1 X Mark III"},
    {0x06040000, "PowerShot S100 / Digital IXUS / IXY Digital"},
    {0x4007d673, "DC19/DC21/DC22"},
    {0x4007d674, "XH A1"},
    {0x4007d675, "HV10"},
    {0x4007d676, "MD130/MD140/MD150/MD160/ZR850"},
    {0x4007d777, "DC50"},
    {0x4007d778, "HV20"},
    {0x4007d779, "DC211"},
    {0x4007d77a, "HG10"},
    {0x4007d77b, "HR10"},
    {0x4007d77d, "MD255/ZR950"},
    {0x4007d81c, "HF11"},
    {0x4007d878, "HV30"},
    {0x4007d87c, "XH A1S"},
    {0x4007d87e, "DC301/DC310/DC311/DC320/DC330"},
    {0x4007d87f, "FS100"},
    {0x4007d880, "HF10"},
    {0x4007d882, "HG20/HG21"},
    {0x4007d925, "HF21"},
    {0x4007d926, "HF S11"},
    {0x4007d978, "HV40"},
    {0x4007d987, "DC410/DC411/DC420"},
    {0x4007d988, "FS19/FS20/FS21/FS22/FS200"},
    {0x4007d989, "HF20/HF200"},
    {0x4007d98a, "HF S10/S100"},
    {0x4007da8e, "HF R10/R16/R17/R18/R100/R106"},
    {0x4007da8f, "HF M30/M31/M36/M300/M306"},
    {0x4007da90, "HF S20/S21/S200"},
    {0x4007da92, "FS31/FS36/FS37/FS300/FS305/FS306/FS307"},
    {0x4007dca0, "EOS C300"},
    {0x4007dda9, "HF G25"},
    {0x4007dfb4, "XC10"},
    {0x4007e1c3, "EOS C200"},
    {0x80000001, "EOS-1D"},
    {0x80000167, "EOS-1DS"},
    {0x80000168, "EOS 10D"},
    {0x80000169, "EOS-1D Mark III"},
    {0x80000170, "EOS Digital Rebel / 300D / Kiss Digital"},
    {0x80000174, "EOS-1D Mark II"},
    {0x80000175, "EOS 20D"},
    {0x80000176, "EOS Digital Rebel XSi / 450D / Kiss X2"},
    {0x80000188, "EOS-1Ds Mark II"},
    {0x80000189, "EOS Digital Rebel XT / 350D / Kiss Digital N"},
    {0x80000190, "EOS 40D"},
    {0x80000213, "EOS 5D"},
    {0x80000215, "EOS-1Ds Mark III"},
    {0x80000218, "EOS 5D Mark II"},
    {0x80000219, "WFT-E1"},
    {0x80000232, "EOS-1D Mark II N"},
    {0x80000234, "EOS 30D"},
    {0x80000236, "EOS Digital Rebel XTi / 400D / Kiss Digital X"},
    {0x80000241, "WFT-E2"},
    {0x80000246, "WFT-E3"},
    {0x80000250, "EOS 7D"},
    {0x80000252, "EOS Rebel T1i / 500D / Kiss X3"},
    {0x80000254, "EOS Rebel XS / 1000D / Kiss F"},
    {0x80000261, "EOS 50D"},
    {0x80000269, "EOS-1D X"},
    {0x80000270, "EOS Rebel T2i / 550D / Kiss X4"},
    {0x80000271, "WFT-E4"},
    {0x80000273, "WFT-E5"},
    {0x80000281, "EOS-1D Mark IV"},
    {0x80000285, "EOS 5D Mark III"},
    {0x80000286, "EOS Rebel T3i / 600D / Kiss X5"},
    {0x80000287, "EOS 60D"},
    {0x80000288, "EOS Rebel T3 / 1100D / Kiss X50"},
    {0x80000289, "EOS 7D Mark II"},
    {0x80000297, "WFT-E2 II"},
    {0x80000298, "WFT-E4 II"},
    {0x80000301, "EOS Rebel T4i / 650D / Kiss X6i"},
    {0x80000302, "EOS 6D"},
    {0x80000324, "EOS-1D C"},
    {0x80000325, "EOS 70D"},
    {0x80000326, "EOS Rebel T5i / 700D / Kiss X7i"},
    {0x80000327, "EOS Rebel T5 / 1200D / Kiss X70 / Hi"},
    {0x80000328, "EOS-1D X Mark II"},
    {0x80000331, "EOS M"},
    {0x80000346, "EOS Rebel SL1 / 100D / Kiss X7"},
    {0x80000347, "EOS Rebel T6s / 760D / 8000D"},
    {0x80000349, "EOS 5D Mark IV"},
    {0x80000350, "EOS 80D"},
    {0x80000355, "EOS M2"},
    {0x80000382, "EOS 5DS"},
    {0x80000393, "EOS Rebel T6i / 750D / Kiss X8i"},
    {0x80000401, "EOS 5DS R"},
    {0x80000404, "EOS Rebel T6 / 1300D / Kiss X80"},
    {0x80000405, "EOS Rebel T7i / 800D / Kiss X9i"},
    {0x80000406, "EOS 6D Mark II"},
    {0x80000408, "EOS 77D / 9000D"},
    {0x80000417, "EOS Rebel SL2 / 200D / Kiss X9"},
    {0x80000421, "EOS R5"},
    {0x80000422, "EOS Rebel T100 / 4000D / 3000D"},
    {0x80000424, "EOS R / Ra"},
    {0x80000428, "EOS-1D X Mark III"},
    {0x80000432, "EOS Rebel T7 / 2000D / 1500D / Kiss X90"},
    {0x80000433, "EOS RP"},
    {0x80000435, "EOS Rebel T8i / 850D / Kiss X10i"},
    {0x80000436, "EOS Rebel SL3 / 250D / 200D Mark II / Kiss X10"},
    {0x80000437, "EOS 90D"},
    {0x80000450, "EOS R3"},
    {0x80000453, "EOS R6"},
    {0x80000464, "EOS R7"},
    {0x80000465, "EOS R10"},
    {0x80000467, "PowerShot ZOOM"},
    {0x80000468, "EOS M50 Mark II / Kiss M2"},
    {0x80000480, "EOS R50"},
    {0x80000481, "EOS R6 Mark II"},
    {0x80000487, "EOS R8"},
    {0x80000491, "PowerShot V10"},
    {0x80000495, "EOS R1"},
    {0x80000496, "EOS R5 Mark II"},
    {0x80000498, "EOS R100"},
    {0x80000520, "EOS D2000C"},
    {0x80000560, "EOS D6000C"},
};

//! SerialNumberFormat, tag 0x0015
constexpr TagDetails canonSerialNumberFormat[] = {
    {0x90000000, N_("Format 1")},
    {0xa0000000, N_("Format 2")},
};

//! SuperMacro, tag 0x001a
constexpr TagDetails canonSuperMacro[] = {
    {0, N_("Off")},
    {1, N_("On (1)")},
    {2, N_("On (2)")},
};

// DateStampMode, tag 0x001c
constexpr TagDetails canonDateStampMode[] = {
    {0, N_("Off")},
    {1, N_("Date")},
    {2, N_("Date & Time")},
};

// Categories, tag 0x0023
[[maybe_unused]] constexpr TagDetails canonCategories[] = {
    {0x0001, N_("People")}, {0x0002, N_("Scenery")}, {0x0004, N_("Events")}, {0x0008, N_("User 1")},
    {0x0016, N_("User 2")}, {0x0032, N_("User 3")},  {0x0064, N_("To Do")},
};

//! PictureStyle Values
constexpr TagDetails canonPictureStyle[] = {
    {0x00, N_("None")},        {0x01, N_("Standard")},       {0x02, N_("Portrait")},    {0x03, N_("High Saturation")},
    {0x04, N_("Adobe RGB")},   {0x05, N_("Low Saturation")}, {0x06, N_("CM Set 1")},    {0x07, N_("CM Set 2")},
    {0x21, N_("User Def. 1")}, {0x22, N_("User Def. 2")},    {0x23, N_("User Def. 3")}, {0x41, N_("PC 1")},
    {0x42, N_("PC 2")},        {0x43, N_("PC 3")},           {0x81, N_("Standard")},    {0x82, N_("Portrait")},
    {0x83, N_("Landscape")},   {0x84, N_("Neutral")},        {0x85, N_("Faithful")},    {0x86, N_("Monochrome")},
    {0x87, N_("Auto")},        {0x88, N_("Fine Detail")},
};

//! WhiteBalance, multiple tags
constexpr TagDetails canonSiWhiteBalance[] = {
    {0, N_("Auto")},        {1, N_("Daylight")},
    {2, N_("Cloudy")},      {3, N_("Tungsten")},
    {4, N_("Fluorescent")}, {5, N_("Flash")},
    {6, N_("Custom")},      {7, N_("Black & White")},
    {8, N_("Shade")},       {9, N_("Manual Temperature (Kelvin)")},
    {10, N_("PC Set 1")},   {11, N_("PC Set 2")},
    {12, N_("PC Set 3")},   {14, N_("Daylight Fluorescent")},
    {15, N_("Custom 1")},   {16, N_("Custom 2")},
    {17, N_("Underwater")}, {18, N_("Custom 3")},
    {19, N_("Custom 3")},   {20, N_("PC Set 4")},
    {21, N_("PC Set 5")},   {23, N_("Auto (ambience priority)")},
};

//! ColorSpace, tag 0x00b4
constexpr TagDetails canonColorSpace[] = {
    {1, N_("sRGB")},
    {2, N_("Adobe RGB")},
    {65535, N_("n/a")},
};

//! Canon AF Area Mode, tag 0x2601
constexpr TagDetails canonAFAreaMode[] = {
    {0, N_("Off (Manual Focus)")},
    {1, N_("AF Point Expansion (surround)")},
    {2, N_("Single-point AF")},
    {4, N_("Multi-point AF")},
    {5, N_("Face Detect AF")},
    {6, N_("Face + Tracking")},
    {7, N_("Zone AF")},
    {8, N_("AF Point Expansion (4 point)")},
    {9, N_("Spot AF")},
    {10, N_("AF Point Expansion (8 point)")},
    {11, N_("Flexizone Multi (49 point)")},
    {12, N_("Flexizone Multi (9 point)")},
    {13, N_("Flexizone Single")},
    {14, N_("Large Zone AF")},
};

// Canon MakerNote Tag Info
constexpr TagInfo CanonMakerNote::tagInfo_[] = {
    {0x0000, "0x0000", "0x0000", N_("Unknown"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0001, "CameraSettings", N_("Camera Settings"), N_("Various camera settings"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0002, "FocalLength", N_("Focal Length"), N_("Focal length"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printFocalLength},
    {0x0003, "CanonFlashInfo", "Canon Flash Info", N_("Canon Flash Info"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0004, "ShotInfo", N_("Shot Info"), N_("Shot information"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0005, "Panorama", N_("Panorama"), N_("Panorama"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0006, "ImageType", N_("Image Type"), N_("Image type"), IfdId::canonId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0007, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), IfdId::canonId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0008, "FileNumber", N_("File Number"), N_("File number"), IfdId::canonId, SectionId::makerTags, unsignedLong, -1,
     print0x0008},
    {0x0009, "OwnerName", N_("Owner Name"), N_("Owner Name"), IfdId::canonId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x000a, "0x000a", N_("0x000a"), N_("Unknown"), IfdId::canonId, SectionId::makerTags, unsignedLong, -1,
     print0x000a},
    {0x000c, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), IfdId::canonId, SectionId::makerTags,
     unsignedLong, -1, print0x000c},
    {0x000d, "CameraInfo", N_("Camera Info"), N_("Camera info"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x000e, "FileLength", N_("FileLength"), N_("FileLength"), IfdId::canonId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    {0x000f, "CustomFunctions2", N_("Custom Functions 2"), N_("Custom Functions 2"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0010, "ModelID", N_("ModelID"), N_("Model ID"), IfdId::canonId, SectionId::makerTags, unsignedLong, -1,
     EXV_PRINT_TAG(canonModelId)},
    {0x0011, "MovieInfo", N_("MovieInfo"), N_("Movie info"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0012, "PictureInfo", N_("Picture Info"), N_("Picture info"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0013, "ThumbnailImageValidArea", N_("Thumbnail Image Valid Area"), N_("Thumbnail image valid area"),
     IfdId::canonId, SectionId::makerTags, signedShort, -1, printValue},
    {0x0015, "SerialNumberFormat", N_("Serial Number Format"), N_("Serial number format"), IfdId::canonId,
     SectionId::makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonSerialNumberFormat)},
    {0x001a, "SuperMacro", N_("Super Macro"), N_("Super macro"), IfdId::canonId, SectionId::makerTags, signedShort, -1,
     EXV_PRINT_TAG(canonSuperMacro)},
    {0x001c, "DateStampMode", N_("DateStampMode"), N_("Date stamp mode"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(canonDateStampMode)},
    {0x001d, "MyColors", N_("MyColors"), N_("My_Colors"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x001e, "FirmwareRevision", N_("FirmwareRevision"), N_("Firmware revision"), IfdId::canonId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    // {0x0023, "Categories", N_("Categories"), N_("Categories"), IfdId::canonId, SectionId::makerTags, unsignedLong -1,
    // EXV_PRINT_TAG(canonCategories)},
    {0x0024, "FaceDetect1", N_("FaceDetect1"), N_("FaceDetect1"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0025, "FaceDetect2", N_("FaceDetect2"), N_("FaceDetect2"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0026, "AFInfo", N_("AF Info"), N_("AF info"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0027, "ContrastInfo", N_("ContrastInfo"), N_("ContrastInfo"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0028, "ImageUniqueID", N_("ImageUniqueID"), N_("ImageUniqueID"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0029, "WBInfo", N_("WBInfo"), N_("WBInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x002f, "FaceDetect3", N_("FaceDetect3"), N_("FaceDetect3"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0035, "TimeInfo", N_("Time Info"), N_("Time zone information"), IfdId::canonId, SectionId::makerTags, signedLong,
     -1, printValue},
    {0x0038, "BatteryType", N_("BatteryType"), N_("BatteryType"), IfdId::canonId, SectionId::makerTags, unsignedLong,
     -1, printValue},
    {0x003c, "AFInfo3", N_("AFInfo3"), N_("AFInfo3"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0081, "RawDataOffset", N_("RawDataOffset"), N_("RawDataOffset"), IfdId::canonId, SectionId::makerTags,
     signedLong, -1, printValue},
    {0x0083, "OriginalDecisionDataOffset", N_("Original Decision Data Offset"), N_("Original decision data offset"),
     IfdId::canonId, SectionId::makerTags, signedLong, -1, printValue},
    // {0x0090, "CustomFunctions1D", N_("CustomFunctions1D"), N_("CustomFunctions1D"), IfdId::canonId,
    // SectionId::makerTags, unsignedShort, -1, printValue}, // ToDo {0x0091, "PersonalFunctions",
    // N_("PersonalFunctions"), N_("PersonalFunctions"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
    // printValue}, // ToDo {0x0092, "PersonalFunctionValues", N_("PersonalFunctionValues"),
    // N_("PersonalFunctionValues"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue}, // ToDo
    {0x0093, "CanonFileInfo", N_("CanonFileInfo"), N_("CanonFileInfo"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0094, "AFPointsInFocus1D", N_("AFPointsInFocus1D"), N_("AFPointsInFocus1D"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0095, "LensModel", N_("Lens Model"), N_("Lens model"), IfdId::canonId, SectionId::makerTags, asciiString, -1,
     printValue},
    {0x0096, "InternalSerialNumber", N_("Internal Serial Number"), N_("Internal serial number"), IfdId::canonId,
     SectionId::makerTags, asciiString, -1, printValue},
    {0x0097, "DustRemovalData", N_("Dust Removal Data"), N_("Dust removal data"), IfdId::canonId, SectionId::makerTags,
     asciiString, -1, printValue},
    {0x0099, "CustomFunctions", N_("Custom Functions"), N_("Custom functions"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x009a, "AspectInfo", N_("AspectInfo"), N_("AspectInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x00a0, "ProcessingInfo", N_("Processing Info"), N_("Processing info"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00a1, "ToneCurveTable", N_("ToneCurveTable"), N_("ToneCurveTable"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00a2, "SharpnessTable", N_("SharpnessTable"), N_("SharpnessTable"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00a3, "SharpnessFreqTable", N_("SharpnessFreqTable"), N_("SharpnessFreqTable"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00a4, "WhiteBalanceTable", N_("White Balance Table"), N_("White balance table"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00a9, "ColorBalance", N_("ColorBalance"), N_("ColorBalance"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00aa, "MeasuredColor", N_("Measured Color"), N_("Measured color"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00ae, "ColorTemperature", N_("ColorTemperature"), N_("ColorTemperature"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00b0, "CanonFlags", N_("CanonFlags"), N_("CanonFlags"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x00b1, "ModifiedInfo", N_("ModifiedInfo"), N_("ModifiedInfo"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00b2, "ToneCurveMatching", N_("ToneCurveMatching"), N_("ToneCurveMatching"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00b3, "WhiteBalanceMatching", N_("WhiteBalanceMatching"), N_("WhiteBalanceMatching"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00b4, "ColorSpace", N_("ColorSpace"), N_("ColorSpace"), IfdId::canonId, SectionId::makerTags, signedShort, -1,
     EXV_PRINT_TAG(canonColorSpace)},
    {0x00b5, "0x00b5", "0x00b5", N_("Unknown"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00b6, "PreviewImageInfo", "PreviewImageInfo", N_("PreviewImageInfo"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x00c0, "0x00c0", "0x00c0", N_("Unknown"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00c1, "0x00c1", "0x00c1", N_("Unknown"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x00d0, "VRDOffset", N_("VRD Offset"), N_("VRD offset"), IfdId::canonId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    {0x00e0, "SensorInfo", N_("Sensor Info"), N_("Sensor info"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    // AFInfo2 structure has a special decoder, see decodeCanonAFInfo in
    // tiffvisitor_int.cpp that decoder useses the below tags from
    // 0x2600 to 0x2611
    {0x2600, "AFInfoSize", N_("AF InfoSize"), N_("AF InfoSize"), IfdId::canonId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x2601, "AFAreaMode", N_("AF Area Mode"), N_("AF Area Mode"), IfdId::canonId, SectionId::makerTags, signedShort,
     -1, EXV_PRINT_TAG(canonAFAreaMode)},
    {0x2602, "AFNumPoints", N_("AF NumPoints"), N_("AF NumPoints"), IfdId::canonId, SectionId::makerTags, signedShort,
     -1, printValue},
    {0x2603, "AFValidPoints", N_("AF ValidPoints"), N_("AF ValidPoints"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x2604, "AFCanonImageWidth", N_("AF ImageWidth"), N_("AF ImageWidth"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x2605, "AFCanonImageHeight", N_("AF ImageHeight"), N_("AF ImageHeight"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x2606, "AFImageWidth", N_("AF Width"), N_("AF Width"), IfdId::canonId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x2607, "AFImageHeight", N_("AF Height"), N_("AF Height"), IfdId::canonId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x2608, "AFAreaWidths", N_("AF Area Widths"), N_("AF Area Widths"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x2609, "AFAreaHeights", N_("AF Area Heights"), N_("AF Area Heights"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x260a, "AFXPositions", N_("AF X Positions"), N_("AF X Positions"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x260b, "AFYPositions", N_("AF Y Positions"), N_("AF Y Positions"), IfdId::canonId, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x260c, "AFPointsInFocus", N_("AF Points in Focus"), N_("AF Points in Focus"), IfdId::canonId,
     SectionId::makerTags, signedShort, -1, printBitmask},
    {0x260d, "AFPointsSelected", N_("AF Points Selected"), N_("AF Points Selected"), IfdId::canonId,
     SectionId::makerTags, signedShort, -1, printBitmask},
    {0x260e, "AFPointsUnusable", N_("AF Points Unusable"), N_("AF Points Unusable"), IfdId::canonId,
     SectionId::makerTags, signedShort, -1, printBitmask},
    {0x260f, "0x260f", "0x260f", N_("0x260f"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x2610, "0x2610", "0x2610", N_("0x2610"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x2611, "AFFineRotation", N_("AF Fine Rotation"),
     N_("Mathematically positive (i.e. anti-clockwise) rotation of every AF rectangle in centidegrees"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, printValue},
    {0x4001, "ColorData", N_("Color Data"), N_("Color data"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4002, "CRWParam", N_("CRWParam"), N_("CRWParam"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4003, "ColorInfo", N_("ColorInfo"), N_("ColorInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4005, "Flavor", N_("Flavor"), N_("Flavor"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x4008, "PictureStyleUserDef", N_("PictureStyleUserDef"), N_("PictureStyleUserDef"), IfdId::canonId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonPictureStyle)},
    // {0x4009, "PictureStylePC", N_("PictureStylePC"), N_("PictureStylePC"), IfdId::canonId, SectionId::makerTags,
    // unsignedShort, -1, EXV_PRINT_TAG(canonPictureStyle)},
    {0x4010, "CustomPictureStyleFileName", N_("CustomPictureStyleFileName"), N_("CustomPictureStyleFileName"),
     IfdId::canonId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x4013, "AFMicroAdj", N_("AFMicroAdj"), N_("AFMicroAdj"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4015, "VignettingCorr", N_("VignettingCorr"), N_("VignettingCorr"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x4016, "VignettingCorr2", N_("VignettingCorr2"), N_("VignettingCorr2"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x4018, "LightingOpt", N_("LightingOpt"), N_("LightingOpt"), IfdId::canonId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x4019, "LensInfo", N_("LensInfo"), N_("LensInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4020, "AmbienceInfo", N_("AmbienceInfo"), N_("AmbienceInfo"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x4021, "MultiExp", N_("MultiExp"), N_("MultiExp"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4024, "FilterInfo", N_("FilterInfo"), N_("FilterInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4025, "HDRInfo", N_("HDRInfo"), N_("HDRInfo"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x4028, "AFConfig", N_("AFConfig"), N_("AFConfig"), IfdId::canonId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x403f, "RawBurstModeRoll", N_("RawBurstModeRoll"), N_("RawBurstModeRoll"), IfdId::canonId, SectionId::makerTags,
     unsignedShort, -1, printValue},
    // End of list marker
    {0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", N_("Unknown CanonMakerNote tag"),
     IfdId::canonId, SectionId::makerTags, asciiString, -1, printValue},
};

const TagInfo* CanonMakerNote::tagList() {
  return tagInfo_;
}

// Canon Movie Info Tag
constexpr TagInfo CanonMakerNote::tagInfoMv_[] = {
    {0x0001, "FrameRate", N_("FrameRate"), N_("FrameRate"), IfdId::canonMvId, SectionId::makerTags, unsignedShort, -1,
     printValue},
    {0x0002, "FrameCount", N_("FrameCount"), N_("FrameCount"), IfdId::canonMvId, SectionId::makerTags, unsignedShort,
     -1, printValue},
    {0x0004, "FrameCount", N_("FrameCount"), N_("FrameCount"), IfdId::canonMvId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    {0x0006, "FrameRate", N_("FrameCount"), N_("FrameCount"), IfdId::canonMvId, SectionId::makerTags, unsignedRational,
     -1, printValue},
    {0x006a, "Duration", N_("Duration"), N_("Duration"), IfdId::canonMvId, SectionId::makerTags, unsignedLong, -1,
     printValue},
    {0x006c, "AudioBitrate", N_("Audio Bitrate"), N_("Audio Bitrate"), IfdId::canonMvId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x006e, "AudioSampleRate", N_("Audio Sample Rate"), N_("Audio Sample Rate"), IfdId::canonMvId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0070, "AudioChannels", N_("Audio Channels"), N_("Audio Channels"), IfdId::canonMvId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x0074, "VideoCodec", N_("Video Codec"), N_("Video Codec"), IfdId::canonMvId, SectionId::makerTags, asciiString,
     -1, printValue},
};

const TagInfo* CanonMakerNote::tagListMv() {
  return tagInfoMv_;
}

// MyColors, tag 0x001d
constexpr TagDetails canonMyColors[] = {
    {0, N_("Off")},        {1, N_("Positive Film")}, {2, N_("Light Skin Tone")}, {3, N_("Dark Skin Tone")},
    {4, N_("Vivid Blue")}, {5, N_("Vivid Green")},   {6, N_("Vivid Red")},       {7, N_("Color Accent")},
    {8, N_("Color Swap")}, {9, N_("Custom")},        {12, N_("Vivid")},          {13, N_("Neutral")},
    {14, N_("Sepia")},     {15, N_("B&W")},
};

// Canon My Colors Info Tag
constexpr TagInfo CanonMakerNote::tagInfoMc_[] = {
    {0x0002, "MyColorMode", N_("My Color Mode"), N_("My Color Mode"), IfdId::canonMyColorID, SectionId::makerTags,
     unsignedShort, -1, EXV_PRINT_TAG(canonMyColors)},
};

const TagInfo* CanonMakerNote::tagListMc() {
  return tagInfoMc_;
}

// Canon FaceDetect 1 Info Tag
constexpr TagInfo CanonMakerNote::tagInfoFcd1_[] = {
    {0x0002, "FacesDetected", N_("Faces Detected"), N_("Faces Detected"), IfdId::canonFcd1Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
    {0x0003, "FacesDetectedFrameSize", N_("Faces Detected Frame Size"), N_("Faces Detected Frame Size"),
     IfdId::canonFcd1Id, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0008, "Face1Position", N_("Face 1 Position"), N_("Face 1 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x000a, "Face2Position", N_("Face 2 Position"), N_("Face 2 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x000c, "Face3Position", N_("Face 3 Position"), N_("Face 3 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x000e, "Face4Position", N_("Face 4 Position"), N_("Face 4 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0010, "Face5Position", N_("Face 5 Position"), N_("Face 5 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0012, "Face6Position", N_("Face 6 Position"), N_("Face 6 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0014, "Face7Position", N_("Face 7 Position"), N_("Face 7 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0016, "Face8Position", N_("Face 8 Position"), N_("Face 8 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
    {0x0018, "Face9Position", N_("Face 9 Position"), N_("Face 9 Position"), IfdId::canonFcd1Id, SectionId::makerTags,
     signedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListFcd1() {
  return tagInfoFcd1_;
}

// Canon FaceDetect 2 Info Tag
constexpr TagInfo CanonMakerNote::tagInfoFcd2_[] = {
    {0x0001, "FaceWidth", N_("Face Width"), N_("Faces Width"), IfdId::canonFcd2Id, SectionId::makerTags, unsignedByte,
     -1, printValue},
    {0x0002, "FacesDetected", N_("Faces Detected"), N_("Faces Detected"), IfdId::canonFcd2Id, SectionId::makerTags,
     unsignedByte, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListFcd2() {
  return tagInfoFcd2_;
}

// Canon ContrastInfo, tag 0x001d
constexpr TagDetails canonContrastInfo[] = {
    {0x0, N_("Off")},
    {0x8, N_("On")},
    {0xfff, N_("n/a")},
};

// Canon Contrast Info Tag
constexpr TagInfo CanonMakerNote::tagInfoCo_[] = {
    {0x0004, "IntelligentContrast", N_("Intelligent Contrast"), N_("Intelligent Contrast"), IfdId::canonContrastId,
     SectionId::makerTags, unsignedShort, -1, EXV_PRINT_TAG(canonContrastInfo)},
};

const TagInfo* CanonMakerNote::tagListCo() {
  return tagInfoCo_;
}

// Canon WhiteBalance Info Tag
constexpr TagInfo CanonMakerNote::tagInfoWbi_[] = {
    {0x0002, "WB_GRGBLevelsAuto", N_("WB_G RGB Levels Auto"), N_("WB_G RGB Levels Auto"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x000a, "WB_GRGBLevelsDaylight", N_("WB_G RGB Levels Daylight"), N_("WB_G RGB Levels Daylight"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0012, "WB_GRGBLevelsCloudy", N_("WB_G RGB Levels Cloudy"), N_("WB_G RGB Levels Cloudy"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x001a, "WB_GRGBLevelsTungsten", N_("WB_G RGB Levels Tungsten"), N_("WB_G RGB Levels Tungsten"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0022, "WB_GRGBLevelsFluorescent", N_("WB_G RGB Levels Fluorescent"), N_("WB_G RGB Levels Fluorescent"),
     IfdId::canonWbId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x002a, "WB_GRGBLevelsFluorHigh", N_("WB_G RGB Levels Fluorescent High"), N_("WB_G RGB Levels Fluorescent High"),
     IfdId::canonWbId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0032, "WB_GRGBLevelsFlash", N_("WB_G RGB Levels Flash"), N_("WB_G RGB Levels Flash"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x003a, "WB_GRGBLevelsUnderwater", N_("WB_G RGB Levels Underwater"), N_("WB_G RGB Levels Underwater"),
     IfdId::canonWbId, SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0042, "WB_GRGBLevelsCustom1", N_("WB_G RGB Levels Custom 1"), N_("WB_G RGB Levels Custom 1"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x004a, "WB_GRGBLevelsCustom2", N_("WB_G RGB Levels Custom 2"), N_("WB_G RGB Levels Custom 2"), IfdId::canonWbId,
     SectionId::makerTags, unsignedLong, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListWbi() {
  return tagInfoWbi_;
}

// Canon FaceDetect 3 Info Tag
constexpr TagInfo CanonMakerNote::tagInfoFcd3_[] = {
    {0x0003, "FacesDetected", N_("Face Detected"), N_("Faces Detected"), IfdId::canonFcd3Id, SectionId::makerTags,
     unsignedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListFcd3() {
  return tagInfoFcd3_;
}

/*
// Canon Aspect Info, tag 0x001d
constexpr TagDetails canonAspectInfo[] = {
    { 0, N_("3:2") },
    { 1, N_("1:1") },
    { 2, N_("4:3") },
    { 7, N_("16:9") },
    { 8, N_("4:5") },
    { 12, N_("3:2 (APS-H crop)") },
    { 13, N_("3:2 (APS-C crop)") }
};
*/

// Canon Aspect Info Tag
constexpr TagInfo CanonMakerNote::tagInfoAs_[] = {
    {0x0000, "AspectRatio", N_("Aspect Ratio"), N_("Aspect Ratio"), IfdId::canonAsId, SectionId::makerTags,
     unsignedLong, -1, printValue},
    {0x0001, "CroppedImageWidth", N_("Cropped Image Width"), N_("Cropped Image Width"), IfdId::canonAsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0002, "CroppedImageHeight", N_("Cropped Image Height"), N_("Cropped Image Height"), IfdId::canonAsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0003, "CroppedImageLeft", N_("Cropped Image Left"), N_("Cropped Image Left"), IfdId::canonAsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0004, "CroppedImageTop", N_("Cropped Image Top"), N_("Cropped Image Top"), IfdId::canonAsId,
     SectionId::makerTags, unsignedLong, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListAs() {
  return tagInfoAs_;
}

// Canon Color Balance Info Tag
constexpr TagInfo CanonMakerNote::tagInfoCbi_[] = {
    {0x0001, "WB_RGGBLevelsAuto", N_("WB_RGGB Levels Auto"), N_("WB_RGGB Levels Auto"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0005, "WB_RGGBLevelsDaylight", N_("WB_RGGB Levels Daylight"), N_("WB_RGGB Levels Daylight"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x000d, "WB_RGGBLevelsShade", N_("WB_RGGB Levels Shade"), N_("WB_RGGB Levels Shade"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x001a, "WB_RGGBLevelsCloudy", N_("WB_RGGB Levels Cloudy"), N_("WB_RGGB Levels Cloudy"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0011, "WB_RGGBLevelsTungsten", N_("WB_RGGB Levels Tungsten"), N_("WB_RGGB Levels Tungsten"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0015, "WB_RGGBLevelsFluorescent", N_("WB_RGGB Levels Fluorescent"), N_("WB_RGGB Levels Fluorescent"),
     IfdId::canonCbId, SectionId::makerTags, signedShort, -1, printValue},
    {0x0032, "WB_RGGBLevelsFlash", N_("WB_RGGB Levels Flash"), N_("WB_RGGB Levels Flash"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x001d, "WB_RGGBLevelsCustomBlackLevels", N_("WB_RGGB Levels Custom Black Levels"),
     N_("WB_RGGB Levels Custom Black Levels"), IfdId::canonCbId, SectionId::makerTags, signedShort, -1, printValue},
    {0x0021, "WB_RGGBLevelsKelvin", N_("WB_RGGB Levels Kelvin"), N_("WB_RGGB Levels Kelvin"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0025, "WB_RGGBBlackLevels", N_("WB_RGGB Black Levels"), N_("WB_RGGB Black Levels"), IfdId::canonCbId,
     SectionId::makerTags, signedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListCbi() {
  return tagInfoCbi_;
}

// Canon Flags Tag
constexpr TagInfo CanonMakerNote::tagInfoFl_[] = {
    {0x0001, "ModifiedParamFlag", N_("Modified Param Flag"), N_("Modified Param Flag"), IfdId::canonFlId,
     SectionId::makerTags, signedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListFl() {
  return tagInfoFl_;
}

// Canon Modified ToneCurve Info, tag 0x0001
constexpr TagDetails canonModifiedToneCurve[] = {
    {0, N_("Standard")},
    {1, N_("Manual")},
    {2, N_("Custom")},
};

// Canon Modified Sharpness Freq Info, tag 0x0002
constexpr TagDetails canonModifiedSharpnessFreq[] = {
    {0, N_("n/a")}, {1, N_("Lowest")}, {2, N_("Low")}, {3, N_("Standard")}, {4, N_("High")}, {5, N_("Highest")},
};

// Canon ModifiedInfo Tag
constexpr TagInfo CanonMakerNote::tagInfoMo_[] = {
    {0x0001, "ModifiedToneCurve", N_("Modified ToneCurve"), N_("Modified ToneCurve"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonModifiedToneCurve)},
    {0x0002, "ModifiedSharpness", N_("Modified Sharpness"), N_("Modified Sharpness"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonModifiedSharpnessFreq)},
    {0x0003, "ModifiedSharpnessFreq", N_("Modified Sharpness Freq"), N_("Modified Sharpness Freq"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x0004, "ModifiedSensorRedLevel", N_("Modified Sensor Red Level"), N_("Modified Sensor Red Level"),
     IfdId::canonMoID, SectionId::makerTags, signedShort, -1, printValue},
    {0x0005, "ModifiedSensorBlueLevel", N_("Modified Sensor Blue Level"), N_("Modified Sensor Blue Level"),
     IfdId::canonMoID, SectionId::makerTags, signedShort, -1, printValue},
    {0x0006, "ModifiedWhiteBalanceRed", N_("Modified White Balance Red"), N_("Modified White Balance Red"),
     IfdId::canonMoID, SectionId::makerTags, signedShort, -1, printValue},
    {0x0007, "ModifiedWhiteBalanceBlue", N_("Modified White Balance Blue"), N_("Modified White Balance Blue"),
     IfdId::canonMoID, SectionId::makerTags, signedShort, -1, printValue},
    {0x0008, "ModifiedWhiteBalance", N_("Modified White Balance"), N_("Modified White Balance"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonSiWhiteBalance)},
    {0x0009, "ModifiedColorTemp", N_("Modified Color Temp"), N_("Modified Color Temp"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x000a, "ModifiedPictureStyle", N_("Modified Picture Style"), N_("Modified Picture Style"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonPictureStyle)},
    {0x000b, "ModifiedDigitalGain", N_("Modified Param Flag"), N_("Modified Param Flag"), IfdId::canonMoID,
     SectionId::makerTags, signedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListMo() {
  return tagInfoMo_;
}

// Canon Preview Quality Info, tag 0x0001
constexpr TagDetails canonPreviewQuality[] = {
    {-1, N_("n/a")},      {1, N_("Economy")}, {2, N_("Normal")},         {3, N_("Fine")},        {4, N_("RAW")},
    {5, N_("Superfine")}, {7, N_("CRAW")},    {130, N_("Normal Movie")}, {131, N_("Movie (2)")},
};

// Canon Preview Image Info Tag
constexpr TagInfo CanonMakerNote::tagInfoPreI_[] = {
    {0x0001, "PreviewQuality", N_("Preview Quality"), N_("Preview Quality"), IfdId::canonPreID, SectionId::makerTags,
     unsignedLong, -1, EXV_PRINT_TAG(canonPreviewQuality)},
    {0x0002, "PreviewImageLength", N_("Preview Image Length"), N_("Preview Image Length"), IfdId::canonPreID,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0003, "PreviewImageWidth", N_("Preview Image Width"), N_("Preview Image Width"), IfdId::canonPreID,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0004, "PreviewImageHeight", N_("Preview Image Height"), N_("Preview Image Height"), IfdId::canonPreID,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0005, "PreviewImageStart", N_("Preview Image Start"), N_("Preview Image Start"), IfdId::canonPreID,
     SectionId::makerTags, unsignedLong, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListPreI() {
  return tagInfoPreI_;
}

// Canon Color Info Tag
constexpr TagInfo CanonMakerNote::tagInfoCi_[] = {
    {0x0001, "Saturation", N_("Saturation"), N_("Saturation"), IfdId::canonCiId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x0002, "ColorTone", N_("Color Tone"), N_("Color Tone"), IfdId::canonCiId, SectionId::makerTags, signedShort, -1,
     printValue},
    {0x0003, "ColorSpace", N_("Color Space"), N_("Color Space"), IfdId::canonCiId, SectionId::makerTags, signedShort,
     -1, EXV_PRINT_TAG(canonColorSpace)},
};

const TagInfo* CanonMakerNote::tagListCi() {
  return tagInfoCi_;
}

// Canon AFMicroAdjMode Quality Info, tag 0x0001
constexpr TagDetails canonAFMicroAdjMode[] = {
    {0, N_("Disable")},
    {1, N_("Adjust all by the same amount")},
    {2, N_("Adjust by lens")},
};

// Canon AFMicroAdj Info Tag
constexpr TagInfo CanonMakerNote::tagInfoAfMiAdj_[] = {
    {0x0001, "AFMicroAdjMode", N_("AFMicroAdjMode"), N_("AFMicroAdjMode"), IfdId::canonAfMiAdjId, SectionId::makerTags,
     signedLong, -1, EXV_PRINT_TAG(canonAFMicroAdjMode)},
    {0x0002, "AFMicroAdjValue", N_("AF Micro Adj Value"), N_("AF Micro Adj Value"), IfdId::canonAfMiAdjId,
     SectionId::makerTags, signedRational, -1, printValue},
    {0xffff, "(UnknownCanonAFMicroAdjTag)", "(UnknownCanonAFMicroAdjTag)", N_("Unknown Canon AFMicroAdj tag"),
     IfdId::canonAfMiAdjId, SectionId::makerTags, signedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListAfMiAdj() {
  return tagInfoAfMiAdj_;
}

// Canon VignettingCorr Tag
constexpr TagInfo CanonMakerNote::tagInfoVigCor_[] = {
    {0x0000, "VignettingCorrVersion", N_("Vignetting Corr Version"), N_("Vignetting Corr Version"),
     IfdId::canonVigCorId, SectionId::makerTags, unsignedShort, -1, printValue},
    {0x0002, "PeripheralLighting", N_("Peripheral Lighting"), N_("Peripheral Lighting"), IfdId::canonVigCorId,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0003, "DistortionCorrection", N_("Distortion Correction"), N_("Distortion Correction"), IfdId::canonVigCorId,
     SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0004, "ChromaticAberrationCorr", N_("Chromatic Aberration Corr"), N_("Chromatic Aberration Corr"),
     IfdId::canonVigCorId, SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0005, "ChromaticAberrationCorr", N_("Chromatic Aberration Corr"), N_("Chromatic Aberration Corr"),
     IfdId::canonVigCorId, SectionId::makerTags, signedShort, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0006, "PeripheralLightingValue", N_("Peripheral Lighting Value"), N_("Peripheral Lighting Value"),
     IfdId::canonVigCorId, SectionId::makerTags, signedShort, -1, printValue},
    {0x0009, "DistortionCorrectionValue", N_("Distortion Correction Value"), N_("Distortion Correction Value"),
     IfdId::canonVigCorId, SectionId::makerTags, signedShort, -1, printValue},
    {0x000b, "OriginalImageWidth", N_("Original Image Width"), N_("Original Image Width"), IfdId::canonVigCorId,
     SectionId::makerTags, signedShort, -1, printValue},
    {0x000c, "OriginalImageHeight", N_("Original Image Height"), N_("Original Image Height"), IfdId::canonVigCorId,
     SectionId::makerTags, signedShort, -1, printValue},
};

const TagInfo* CanonMakerNote::tagListVigCor() {
  return tagInfoVigCor_;
}

// Canon VignettingCorr2 Tag
constexpr TagInfo CanonMakerNote::tagInfoVigCor2_[] = {
    {0x0005, "PeripheralLightingSetting", N_("Peripheral Lighting Setting"), N_("Peripheral Lighting Setting"),
     IfdId::canonVigCor2Id, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0006, "ChromaticAberrationSetting", N_("Chromatic Aberration Setting"), N_("Chromatic Aberration Setting"),
     IfdId::canonVigCor2Id, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0007, "DistortionCorrectionSetting", N_("Distortion Correction Setting"), N_("Distortion Correction Setting"),
     IfdId::canonVigCor2Id, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
    {0xffff, "(UnknownVignettingCorr2Tag)", "(UnknownVignettingCorr2Tag)", N_("UnknownVignettingCorr2Tag  Tag"),
     IfdId::canonVigCor2Id, SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListVigCor2() {
  return tagInfoVigCor2_;
}

// Canon AutoLightingOptimizer, tag 0x0002
constexpr TagDetails canonAutoLightingOptimizer[] = {
    {0, N_("Standard")},
    {1, N_("Low")},
    {2, N_("Strong")},
    {3, N_("Off")},
};

// Canon HighISONoiseReduction, tag 0x0004
constexpr TagDetails canonLongExposureNoiseReduction[] = {
    {0, N_("Off")},
    {1, N_("Auto")},
    {2, N_("On")},
};

// Canon HighISONoiseReduction, tag 0x0005
constexpr TagDetails canonHighISONoiseReduction[] = {
    {0, N_("Standard")},
    {1, N_("Low")},
    {2, N_("Strong")},
    {3, N_("Off")},
};

// Canon DigitalLensOptimizer, tag 0x000a
constexpr TagDetails canonDigitalLensOptimizer[] = {
    {0, N_("Off")},
    {1, N_("Standard")},
    {2, N_("High")},
};

// Canon DualPixelRaw, tag 0x000b
constexpr TagDetails canonDualPixelRaw[] = {
    {0, N_("Off")},
    {1, N_("On")},
};

// Canon LightingOpt Tag
constexpr TagInfo CanonMakerNote::tagInfoLiOp_[] = {
    {0x0001, "PeripheralIlluminationCorr", N_("Peripheral Lighting Setting"), N_("Peripheral Lighting Setting"),
     IfdId::canonLiOpId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0002, "AutoLightingOptimizer", N_("Auto Lighting Optimizer"), N_("Auto Lighting Optimizer"), IfdId::canonLiOpId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAutoLightingOptimizer)},
    {0x0003, "HighlightTonePriority", N_("Highlight Tone Priority"), N_("Highlight Tone Priority"), IfdId::canonLiOpId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOffOn)},
    {0x0004, "LongExposureNoiseReduction", N_("Long Exposure Noise Reduction"), N_("Long Exposure Noise Reduction"),
     IfdId::canonLiOpId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonLongExposureNoiseReduction)},
    {0x0005, "HighISONoiseReduction", N_("High ISO Noise Reduction"), N_("High ISO Noise Reduction"),
     IfdId::canonLiOpId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonHighISONoiseReduction)},
    {0x000a, "DigitalLensOptimizer", N_("Digital Lens Optimizer"), N_("Digital Lens Optimizer"), IfdId::canonLiOpId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonDigitalLensOptimizer)},
    {0x000b, "DualPixelRaw", N_("Dual Pixel Raw"), N_("Dual Pixel Raw"), IfdId::canonLiOpId, SectionId::makerTags,
     signedLong, -1, EXV_PRINT_TAG(canonDualPixelRaw)},
    {0xffff, "(UnknownLightingOptimizationTag)", "(UnknownLightingOptimizationTag)",
     N_("UnknownLightingOptimizationTag Selection Tag"), IfdId::canonLiOpId, SectionId::makerTags, signedLong, 1,
     printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListLiOp() {
  return tagInfoLiOp_;
}

// Canon LensInfo Tag
constexpr TagInfo CanonMakerNote::tagInfoLe_[] = {
    {0x0000, "LensSerialNumber", N_("Lens Serial Number"),
     N_("Lens Serial Number. Convert each byte to hexadecimal to get two "
        "digits of the lens serial number."),
     IfdId::canonLeId, SectionId::makerTags, unsignedByte, -1, printLe0x0000},
    {0xffff, "(UnknownCanonLensInfoTag)", "(UnknownCanonLensInfoTag)", N_("UnknownCanonLensInfoTag"), IfdId::canonLeId,
     SectionId::makerTags, undefined, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListLe() {
  return tagInfoLe_;
}

// Canon AmbienceSelection, tag 0x0001
constexpr TagDetails canonAmbienceSelection[] = {
    {0, N_("Standard")}, {1, N_("Vivid")},    {2, N_("Warm")},   {3, N_("Soft")},       {4, N_("Cool")},
    {5, N_("Intense")},  {6, N_("Brighter")}, {7, N_("Darker")}, {8, N_("Monochrome")},
};

// Canon Ambience Tag
constexpr TagInfo CanonMakerNote::tagInfoAm_[] = {
    {0x0001, "AmbienceSelection", N_("Ambience Selection"), N_("Ambience Selection"), IfdId::canonAmId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAmbienceSelection)},
    {0xffff, "(AmbienceSelectionTag)", "(AmbienceSelectionTag)", N_("UAmbience Selection Tag"), IfdId::canonAmId,
     SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListAm() {
  return tagInfoAm_;
}

// Canon MultiExposure, tag 0x0001
constexpr TagDetails canonMultiExposure[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("On (RAW)")},
};

// Canon MultiExposureControl, tag 0x0001
constexpr TagDetails canonMultiExposureControl[] = {
    {0, N_("Additive")},
    {1, N_("Average")},
    {2, N_("Bright (comparative)")},
    {3, N_("Dark (comparative)")},
};

// Canon MultiExp Tag
constexpr TagInfo CanonMakerNote::tagInfoMe_[] = {
    {0x0001, "MultiExposure", N_("Multi Exposure"), N_("Multi Exposure"), IfdId::canonMeId, SectionId::makerTags,
     signedLong, -1, EXV_PRINT_TAG(canonMultiExposure)},
    {0x0002, "MultiExposureControl", N_("Multi Exposure Control"), N_("Multi Exposure Control"), IfdId::canonMeId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonMultiExposureControl)},
    {0x0003, "MultiExposureShots", N_("Multi Exposure Shots"), N_("Multi Exposure Shots"), IfdId::canonMeId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonMultiExposure)},
    {0xffff, "(UnknownMultiExposureTag)", "(UnknownMultiExposureTag)", N_("UnknownMultiExposureTag"), IfdId::canonMeId,
     SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListMe() {
  return tagInfoMe_;
}

// Canon FilterInfo, tag 0x0001
constexpr TagDetails canonFilterInfo[] = {
    {-1, N_("Off")},
};

// Canon MiniatureFilterOrientation, tag 0x0001
constexpr TagDetails canonMiniatureFilterOrientation[] = {
    {0, N_("Horizontal")},
    {1, N_("Vertical")},
};

//  Canon Filter Info Tag
constexpr TagInfo CanonMakerNote::tagInfoFil_[] = {
    {0x0101, "GrainyBWFilter", N_("Grainy BW Filter"), N_("Grainy BW Filter"), IfdId::canonFilId, SectionId::makerTags,
     asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0201, "SoftFocusFilter", N_("Soft Focus Filter"), N_("Soft Focus Filter"), IfdId::canonFilId,
     SectionId::makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0301, "ToyCameraFilter", N_("Toy Camera Filter"), N_("Toy Camera Filter"), IfdId::canonFilId,
     SectionId::makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0401, "MiniatureFilter", N_("Miniature Filter"), N_("Miniature Filter"), IfdId::canonFilId, SectionId::makerTags,
     asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0402, "MiniatureFilterOrientation", N_("Miniature Filter Orientation"), N_("Miniature Filter Orientation"),
     IfdId::canonFilId, SectionId::makerTags, asciiString, -1, EXV_PRINT_TAG(canonMiniatureFilterOrientation)},
    {0x0403, "MiniatureFilterPosition", N_("Miniature Filter Position"), N_("Miniature Filter Position"),
     IfdId::canonFilId, SectionId::makerTags, asciiString, -1, printValue},
    {0x0404, "MiniatureFilterParameter", N_("Miniature Filter Parameter"), N_("Miniature Filter Parameter"),
     IfdId::canonFilId, SectionId::makerTags, asciiString, -1, printValue},
    {0x0501, "FisheyeFilter", N_("Fisheye Filter"), N_("Fisheye Filter"), IfdId::canonFilId, SectionId::makerTags,
     asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0601, "PaintingFilter", N_("Painting Filter"), N_("Painting Filter"), IfdId::canonFilId, SectionId::makerTags,
     asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0x0701, "WatercolorFilter", N_("Watercolor Filter"), N_("Watercolor Filter"), IfdId::canonFilId,
     SectionId::makerTags, asciiString, -1, EXV_PRINT_TAG(canonFilterInfo)},
    {0xffff, "(UnknownFilterTag)", "(UnknownFilterTag)", N_("UnknownFilterTag"), IfdId::canonFilId,
     SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListFil() {
  return tagInfoFil_;
}

// Canon HDR, tag 0x0001
constexpr TagDetails canonHdr[] = {
    {0, N_("Off")},
    {1, N_("On")},
    {2, N_("On (RAW")},
};

// Canon HDREffect, tag 0x0001
constexpr TagDetails canonHdrEffect[] = {
    {0, N_("Natural")},    {1, N_("Art (standard)")}, {2, N_("Art (vivid)")},
    {3, N_("Art (bold)")}, {4, N_("Art (embossed)")},
};

// Canon HDR Info Tag
constexpr TagInfo CanonMakerNote::tagInfoHdr_[] = {
    {0x0001, "HDR", N_("HDR"), N_("HDR"), IfdId::canonHdrId, SectionId::makerTags, signedLong, -1,
     EXV_PRINT_TAG(canonHdr)},
    {0x0002, "HDREffect", N_("HDR Effect"), N_("HDR Effect"), IfdId::canonHdrId, SectionId::makerTags, signedLong, -1,
     EXV_PRINT_TAG(canonHdrEffect)},
    {0xffff, "(UnknownHDRTag)", "(UnknownHDRTag)", N_("Unknown Canon HDR Tag"), IfdId::canonHdrId, SectionId::makerTags,
     signedLong, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListHdr() {
  return tagInfoHdr_;
}

// Canon AIServoFirstImage, tag 0x0001
constexpr TagDetails canonAIServoFirstImage[] = {
    {0, N_("Equal Priority")},
    {1, N_("Release Priority")},
    {2, N_("Focus Priority")},
};

// Canon AIServoSecondImage, tag 0x0001
constexpr TagDetails canonAIServoSecondImage[] = {
    {0, N_("Equal Priority")},        {1, N_("Release Priority")},    {2, N_("Focus Priority")},
    {3, N_("Release High Priority")}, {4, N_("Focus High Priority")},
};

// Canon USMLensElectronicMF, tag 0x0001
constexpr TagDetails canonUSMLensElectronicMF[] = {
    {0, N_("Enable After AF")},
    {1, N_("Disable After AF")},
    {2, N_("Disable in AF Mode")},
};

// Canon AFAssistBeam, tag 0x0001
constexpr TagDetails canonAFAssistBeam[] = {
    {0, N_("Enable")},
    {1, N_("Disable")},
    {2, N_("IR AF Assist Beam Mode")},
};

// Canon OneShotAFRelease, tag 0x0001
constexpr TagDetails canonOneShotAFRelease[] = {
    {0, N_("Focus Priority")},
    {1, N_("Release Priority")},
};

// Canon AutoAFPointSelEOSiTRAF, tag 0x0001
constexpr TagDetails canonAutoAFPointSelEOSiTRAF[] = {
    {0, N_("Enable")},
    {1, N_("Disable")},
};

// Canon LensDriveWhenAFImpossible, tag 0x0001
constexpr TagDetails canonLensDriveWhenAFImpossible[] = {
    {0, N_("Continue Focus Search")},
    {1, N_("Stop Focus Search")},
};

// Canon SelectAFAreaSelectionMode, tag 0x0001
constexpr TagDetails canonSelectAFAreaSelectionMode[] = {
    {0, N_("Single-Point-AF")}, {1, N_("Auto")},
    {2, N_("Zone AF")},         {3, N_("AF Point Expansion (4 point)")},
    {4, N_("Spot AF")},         {5, N_("AF Point Expansion (8 point)")},
};

// Canon AFAreaSelectionMethod, tag 0x0001
constexpr TagDetails canonAFAreaSelectionMethod[] = {
    {0, N_("M-Fn Button")},
    {1, N_("Main Dial")},
};

// Canon OrientationLinkedAF, tag 0x0001
constexpr TagDetails canonOrientationLinkedAF[] = {
    {0, N_("Same for Vert/Horiz Points")},
    {1, N_("Separate for Vert/Horiz Points")},
    {2, N_("Separate Area+Points")},
};

// Canon ManualAFPointSelPattern, tag 0x0001
constexpr TagDetails canonManualAFPointSelPattern[] = {
    {0, N_("Stops at AF Area Edges")},
    {1, N_("Continuous")},
};

// Canon AFPointDisplayDuringFocus, tag 0x0001
constexpr TagDetails canonAFPointDisplayDuringFocus[] = {
    {0, N_("Selected (constant)")}, {1, N_("All (constant)")}, {1, N_("Selected (pre-AF, focused)")},
    {1, N_("Selected (focused)")},  {1, N_("Disabled")},
};

// Canon VFDisplayIllumination, tag 0x0001
constexpr TagDetails canonAVFDisplayIllumination[] = {
    {0, N_("Auto")},
    {1, N_("Enable")},
    {2, N_("Disable")},
};

// Canon AFStatusViewfinder, tag 0x0001
constexpr TagDetails canonAFStatusViewfinder[] = {
    {0, N_("Auto")},
    {1, N_("Enable")},
    {2, N_("Disable")},
};

// Canon InitialAFPointInServo, tag 0x0001
constexpr TagDetails canonInitialAFPointInServo[] = {
    {0, N_("Initial AF Point Selected")},
    {1, N_("Manual AF Point")},
    {2, N_("Auto")},
};

// Canon AFConfig Tags
constexpr TagInfo CanonMakerNote::tagInfoAfC_[] = {
    {0x0001, "AFConfigTool", N_("AF Config Tool"), N_("AF Config Tool"), IfdId::canonAfCId, SectionId::makerTags,
     signedLong, -1, printValue},
    {0x0002, "AFTrackingSensitivity", N_("AF Tracking Sensitivity"), N_("AF Tracking Sensitivity"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, printValue},
    {0x0003, "AFAccelDecelTracking", N_("AF Accel Decel Tracking"), N_("AF Accel Decel Tracking"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, printValue},
    {0x0004, "AFPointSwitching", N_("AF Point Switching"), N_("AF Point Switching"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, printValue},
    {0x0005, "AIServoFirstImage", N_("AI Servo First Image"), N_("AI Servo First Image"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAIServoFirstImage)},
    {0x0006, "AIServoSecondImage", N_("AI Servo Second Image"), N_("AI Servo Second Image"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAIServoSecondImage)},
    {0x0007, "USMLensElectronicMF", N_("USM Lens Electronic MF"), N_("USM Lens Electronic MF"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonUSMLensElectronicMF)},
    {0x0008, "AFAssistBeam", N_("AF Assist Beam"), N_("AF Assist Beam"), IfdId::canonAfCId, SectionId::makerTags,
     signedLong, -1, EXV_PRINT_TAG(canonAFAssistBeam)},
    {0x0009, "OneShotAFRelease", N_("One Shot AF Release"), N_("One Shot AF Release"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOneShotAFRelease)},
    {0x000a, "AutoAFPointSelEOSiTRAF", N_("Auto AF Point Sel EOS iTRAF"), N_("Auto AF Point Sel EOS iTRAF"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAutoAFPointSelEOSiTRAF)},
    {0x000b, "LensDriveWhenAFImpossible", N_("Lens Drive When AF Impossible"), N_("Lens Drive When AF Impossible"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonLensDriveWhenAFImpossible)},
    {0x000c, "SelectAFAreaSelectionMode", N_("Select AF Area Selection Mode"), N_("Select AF Area Selection Mode"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonSelectAFAreaSelectionMode)},
    {0x000d, "AFAreaSelectionMethod", N_("AF Area Selection Method"), N_("AF Area Selection Method"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFAreaSelectionMethod)},
    {0x000e, "OrientationLinkedAF", N_("Orientation Linked AF"), N_("Orientation Linked AF"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonOrientationLinkedAF)},
    {0x000f, "ManualAFPointSelPattern", N_("Manual AF Point Sel Pattern"), N_("Manual AF Point Sel Pattern"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonManualAFPointSelPattern)},
    {0x0010, "AFPointDisplayDuringFocus", N_("AF Point Display During Focus"), N_("AF Point Display During Focus"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFPointDisplayDuringFocus)},
    {0x0011, "VFDisplayIllumination", N_("VF Display Illumination"), N_("VF Display Illumination"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAVFDisplayIllumination)},
    {0x0012, "AFStatusViewfinder", N_("AF Status Viewfinder"), N_("AF Status Viewfinder"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonAFStatusViewfinder)},
    {0x0013, "InitialAFPointInServo", N_("Initial AF Point In Servo"), N_("Initial AF Point In Servo"),
     IfdId::canonAfCId, SectionId::makerTags, signedLong, -1, EXV_PRINT_TAG(canonInitialAFPointInServo)},
    {0xffff, "(UnknownCanonAFTag)", "(UnknownCanonAFTag)", N_("UnknownCanonAFTag"), IfdId::canonAfCId,
     SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListAfC() {
  return tagInfoAfC_;
}

// Canon RawBurstInfo Info Tag
constexpr TagInfo CanonMakerNote::tagInfoRawB_[] = {
    {0x0001, "RawBurstImageNum", N_("Raw Burst Image Num"), N_("Raw Burst Image Num"), IfdId::canonRawBId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0x0002, "RawBurstImageCount", N_("Raw Burst Image Count"), N_("Raw Burst Image Count"), IfdId::canonRawBId,
     SectionId::makerTags, unsignedLong, -1, printValue},
    {0xffff, "(UnknownRawBurstTag)", "(UnknownRawBurstTag)", N_("UnknownRawBurstTag"), IfdId::canonRawBId,
     SectionId::makerTags, signedLong, 1, printValue}  // important to add end of tag
};

const TagInfo* CanonMakerNote::tagListRawB() {
  return tagInfoRawB_;
}

//! Macro, tag 0x0001
constexpr TagDetails canonCsMacro[] = {
    {1, N_("On")},
    {2, N_("Off")},
};

//! Quality, tag 0x0003
constexpr TagDetails canonCsQuality[] = {
    {-1, N_("n/a")}, {0, N_("unknown")},   {1, N_("Economy")}, {2, N_("Normal")},         {3, N_("Fine")},
    {4, N_("RAW")},  {5, N_("Superfine")}, {7, N_("CRAW")},    {130, N_("Normal Movie")}, {131, N_("Movie (2)")},
};

//! FlashMode, tag 0x0004
constexpr TagDetails canonCsFlashMode[] = {
    {0, N_("Off")},          {1, N_("Auto")},      {2, N_("On")},
    {3, N_("Red-eye")},      {4, N_("Slow sync")}, {5, N_("Auto + red-eye")},
    {6, N_("On + red-eye")}, {16, N_("External")}, {16, N_("External")}  // To silence compiler warning
};

//! DriveMode, tag 0x0005
constexpr TagDetails canonCsDriveMode[] = {
    {0, N_("Single / timer")},
    {1, N_("Continuous")},
    {2, N_("Movie")},
    {3, N_("Continuous, speed priority")},
    {3, N_("Continuous, tracking priority")},
    {4, N_("Continuous, low")},
    {5, N_("Continuous, high")},
    {6, N_("Silent Single")},
    {9, N_("Single, Silent")},
    {10, N_("Continuous, Silent")},
};

//! FocusMode, tag 0x0007
constexpr TagDetails canonCsFocusMode[] = {
    {0, N_("One shot AF")},      {1, N_("AI servo AF")},
    {2, N_("AI focus AF")},      {3, N_("Manual focus (3)")},
    {4, N_("Single")},           {5, N_("Continuous")},
    {6, N_("Manual focus (6)")}, {16, N_("Pan focus")},
    {256, N_("AF + MF")},        {512, N_("Movie Snap Focus")},
    {519, N_("Movie Servo AF")}, {519, N_("Movie Servo AF")}  // To silence compiler warning
};

//! RecordMode, tag 0x0009
constexpr TagDetails canonCsRecordMode[] = {
    {1, N_("JPEG")}, {2, N_("CRW+THM")},   {3, N_("AVI+THM")}, {4, N_("TIF")},      {5, N_("TIF+JPEG")},
    {6, N_("CR2")},  {7, N_("CR2+JPEG")},  {9, N_("MOV")},     {10, N_("MP4")},     {11, N_("CRM")},
    {12, N_("CR3")}, {13, N_("CR3+JPEG")}, {14, N_("HIF")},    {15, N_("CR3+HIF")},
};

//! ImageSize, tag 0x000a
constexpr TagDetails canonCsImageSize[] = {
    {0, N_("Large")},
    {1, N_("Medium")},
    {2, N_("Small")},
    {5, N_("Medium 1")},
    {6, N_("Medium 2")},
    {7, N_("Medium 3")},
    {8, N_("Postcard")},
    {9, N_("Widescreen")},
    {10, N_("Medium Widescreen")},
    {14, N_("Small 1")},
    {15, N_("Small 2")},
    {16, N_("Small 3")},
    {128, N_("640x480 Movie")},
    {129, N_("Medium Movie")},
    {130, N_("Small Movie")},
    {137, N_("1280x720 Movie")},
    {142, N_("1920x1080 Movie")},
    {143, N_("4096x2160 Movie")},
};

//! EasyMode, tag 0x000b
constexpr TagDetails canonCsEasyMode[] = {
    {0, N_("Full auto")},
    {1, N_("Manual")},
    {2, N_("Landscape")},
    {3, N_("Fast shutter")},
    {4, N_("Slow shutter")},
    {5, N_("Night")},
    {6, N_("Gray Scale")},
    {7, N_("Sepia")},
    {8, N_("Portrait")},
    {9, N_("Sports")},
    {10, N_("Macro")},
    {11, N_("Black & White")},
    {12, N_("Pan focus")},
    {13, N_("Vivid")},
    {14, N_("Neutral")},
    {15, N_("Flash Off")},
    {16, N_("Long Shutter")},
    {17, N_("Super Macro")},
    {18, N_("Foliage")},
    {19, N_("Indoor")},
    {20, N_("Fireworks")},
    {21, N_("Beach")},
    {22, N_("Underwater")},
    {23, N_("Snow")},
    {24, N_("Kids & Pets")},
    {25, N_("Night Snapshot")},
    {26, N_("Digital Macro")},
    {27, N_("My Colors")},
    {28, N_("Movie Snap")},
    {29, N_("Super Macro 2")},
    {30, N_("Color Accent")},
    {31, N_("Color Swap")},
    {32, N_("Aquarium")},
    {33, N_("ISO 3200")},
    {34, N_("ISO 6400")},
    {35, N_("Creative Light Effect")},
    {36, N_("Easy")},
    {37, N_("Quick Shot")},
    {38, N_("Creative Auto")},
    {39, N_("Zoom Blur")},
    {40, N_("Low Light")},
    {41, N_("Nostalgic")},
    {42, N_("Super Vivid")},
    {43, N_("Poster Effect")},
    {44, N_("Face Self-timer")},
    {45, N_("Smile")},
    {46, N_("Wink Self-timer")},
    {47, N_("Fisheye Effect")},
    {48, N_("Miniature Effect")},
    {49, N_("High-speed Burst")},
    {50, N_("Best Image Selection")},
    {51, N_("High Dynamic Range")},
    {52, N_("Handheld Night Scene")},
    {53, N_("Movie Digest")},
    {54, N_("Live View Control")},
    {55, N_("Discreet")},
    {56, N_("Blur Reduction")},
    {57, N_("Monochrome")},
    {58, N_("Toy Camera Effect")},
    {59, N_("Scene Intelligent Auto")},
    {60, N_("High-speed Burst HQ")},
    {61, N_("Smooth Skin")},
    {62, N_("Soft Focus")},
    {257, N_("Spotlight")},
    {258, N_("Night 2")},
    {259, N_("Night+")},
    {260, N_("Super Night")},
    {261, N_("Sunset")},
    {263, N_("Night Scene")},
    {264, N_("Surface")},
    {265, N_("Low Light 2")},
};

//! DigitalZoom, tag 0x000c
constexpr TagDetails canonCsDigitalZoom[] = {
    {0, N_("None")}, {1, "2x"}, {2, "4x"}, {3, N_("Other")}, {3, N_("Other")}  // To silence compiler warning
};

//! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
constexpr TagDetails canonCsLnh[] = {
    {0xffff, N_("Low")},
    {0x0000, N_("Normal")},
    {0x0001, N_("High")},
};

//! ISOSpeeds, tag 0x0010
constexpr TagDetails canonCsISOSpeed[] = {
    {0, N_("n/a")},       {14, N_("Auto High")}, {15, N_("Auto")}, {16, "50"},       {17, "100"},      {18, "200"},
    {19, "400"},          {20, "800"},           {16464, "80"},    {16484, "100"},   {16509, "125"},   {16544, "160"},
    {16584, "200"},       {16634, "250"},        {16704, "320"},   {16784, "400"},   {16884, "500"},   {17024, "640"},
    {17184, "800"},       {17384, "1000"},       {17634, "1250"},  {17984, "1600"},  {18384, "2000"},  {18884, "2500"},
    {19584, "3200"},      {20384, "4000"},       {21384, "5000"},  {22784, "6400"},  {24384, "8000"},  {26384, "10000"},
    {29184, "12800"},     {29184, "16000"},      {29184, "20000"}, {29184, "25600"}, {29184, "32000"}, {29184, "40000"},
    {29184, "H1(51200)"}, {29184, "H2(102400)"},
};

//! MeteringMode, tag 0x0011
constexpr TagDetails canonCsMeteringMode[] = {
    {0, N_("Default")},    {1, N_("Spot")},    {2, N_("Average")},
    {3, N_("Evaluative")}, {4, N_("Partial")}, {5, N_("Center-weighted average")},
};

//! FocusType, tag 0x0012
constexpr TagDetails canonCsFocusType[] = {
    {0, N_("Manual")},     {1, N_("Auto")},        {2, N_("Not known")},    {3, N_("Macro")},
    {4, N_("Very close")}, {5, N_("Close")},       {6, N_("Middle range")}, {7, N_("Far range")},
    {8, N_("Pan focus")},  {9, N_("Super macro")}, {10, N_("Infinity")},
};

//! AFPoint, tag 0x0013
constexpr TagDetails canonCsAfPoint[] = {
    {0x2005, N_("Manual AF point selection")},
    {0x3000, N_("None (MF)")},
    {0x3001, N_("Auto-selected")},
    {0x3002, N_("Right")},
    {0x3003, N_("Center")},
    {0x3004, N_("Left")},
    {0x4001, N_("Auto AF point selection")},
    {0x4006, N_("Face Detect")},
};

//! ExposureProgram, tag 0x0014
constexpr TagDetails canonCsExposureProgram[] = {
    {0, N_("Easy shooting (Auto)")},
    {1, N_("Program (P)")},
    {2, N_("Shutter priority (Tv)")},
    {3, N_("Aperture priority (Av)")},
    {4, N_("Manual (M)")},
    {5, N_("A-DEP")},
    {6, N_("M-DEP")},
    {7, N_("Bulb")},
};

//! LensType, tag 0x0016
constexpr TagDetails canonCsLensType[] = {
    {1, "Canon EF 50mm f/1.8"},
    {2, "Canon EF 28mm f/2.8"},
    {2, "Sigma 24mm f/2.8 Super Wide II"},  // 1
    {3, "Canon EF 135mm f/2.8 Soft"},
    {4, "Canon EF 35-105mm f/3.5-4.5"},
    {4, "Sigma UC Zoom 35-135mm f/4-5.6"},  // 1
    {5, "Canon EF 35-70mm f/3.5-4.5"},
    {6, "Canon EF 28-70mm f/3.5-4.5"},
    {6, "Sigma 18-50mm f/3.5-5.6 DC"},          // 1
    {6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"},  // 2
    {6, "Tokina AF 193-2 19-35mm f/3.5-4.5"},   // 3
    {6, "Sigma 28-80mm f/3.5-5.6 II Macro"},    // 4
    {6, "Sigma 28-300mm f/3.5-6.3 DG Macro"},   // 5
    {7, "Canon EF 100-300mm f/5.6L"},
    {8, "Canon EF 100-300mm f/5.6"},
    {8, "Sigma 70-300mm f/4-5.6 [APO] DG Macro"},  // 1
    {8, "Tokina AT-X 242 AF 24-200mm f/3.5-5.6"},  // 2
    {9, "Canon EF 70-210mm f/4"},
    {9, "Sigma 55-200mm f/4-5.6 DC"},  // 1
    {10, "Canon EF 50mm f/2.5 Macro"},
    {10, "Sigma 50mm f/2.8 EX"},              // 1
    {10, "Sigma 28mm f/1.8"},                 // 2
    {10, "Sigma 105mm f/2.8 Macro EX"},       // 3
    {10, "Sigma 70mm f/2.8 EX DG Macro EF"},  // 4
    {11, "Canon EF 35mm f/2"},
    {13, "Canon EF 15mm f/2.8 Fisheye"},
    {14, "Canon EF 50-200mm f/3.5-4.5L"},
    {15, "Canon EF 50-200mm f/3.5-4.5"},
    {16, "Canon EF 35-135mm f/3.5-4.5"},
    {17, "Canon EF 35-70mm f/3.5-4.5A"},
    {18, "Canon EF 28-70mm f/3.5-4.5"},
    {20, "Canon EF 100-200mm f/4.5A"},
    {21, "Canon EF 80-200mm f/2.8L"},
    {22, "Canon EF 20-35mm f/2.8L"},
    {22, "Tokina AT-X 280 AF Pro 28-80mm f/2.8 Aspherical"},  // 1
    {23, "Canon EF 35-105mm f/3.5-4.5"},
    {24, "Canon EF 35-80mm f/4-5.6 Power Zoom"},
    {25, "Canon EF 35-80mm f/4-5.6 Power Zoom"},
    {26, "Canon EF 100mm f/2.8 Macro"},
    {26, "Cosina 100mm f/3.5 Macro AF"},                   // 1
    {26, "Tamron SP AF 90mm f/2.8 Di Macro"},              // 2
    {26, "Tamron SP AF 180mm f/3.5 Di Macro"},             // 3
    {26, "Carl Zeiss Planar T* 50mm f/1.4"},               // 4
    {26, "Voigtlander APO Lanthar 125mm f/2.5 SL Macro"},  // 5
    {26, "Carl Zeiss Planar T 85mm f/1.4 ZE"},             // 6
    {27, "Canon EF 35-80mm f/4-5.6"},
    {28, "Canon EF 80-200mm f/4.5-5.6"},
    {28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"},               // 1
    {28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"},  // 2
    {28, "Tamron AF 70-300mm f/4-5.6 Di LD 1:2 Macro"},                 // 3
    {28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"},                    // 4
    {29, "Canon EF 50mm f/1.8 II"},
    {30, "Canon EF 35-105mm f/4.5-5.6"},
    {31, "Canon EF 75-300mm f/4-5.6"},
    {31, "Tamron SP AF 300mm f/2.8 LD IF"},  // 1
    {32, "Canon EF 24mm f/2.8"},
    {32, "Sigma 15mm f/2.8 EX Fisheye"},  // 1
    {33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"},
    {33, "Voigtlander Color Skopar 20mm f/3.5 SLII Aspherical"},  // 1
    {33, "Voigtlander APO-Lanthar 90mm f/3.5 SLII Close Focus"},  // 2
    {33, "Carl Zeiss Distagon T* 15mm f/2.8 ZE"},                 // 3
    {33, "Carl Zeiss Distagon T* 18mm f/3.5 ZE"},                 // 4
    {33, "Carl Zeiss Distagon T* 21mm f/2.8 ZE"},                 // 5
    {33, "Carl Zeiss Distagon T* 25mm f/2 ZE"},                   // 6
    {33, "Carl Zeiss Distagon T* 28mm f/2 ZE"},                   // 7
    {33, "Carl Zeiss Distagon T* 35mm f/2 ZE"},                   // 8
    {33, "Carl Zeiss Distagon T* 35mm f/1.4 ZE"},                 // 9
    {33, "Carl Zeiss Planar T* 50mm f/1.4 ZE"},                   // 10
    {33, "Carl Zeiss Makro-Planar T* 50mm f/2 ZE"},               // 11
    {33, "Carl Zeiss Makro-Planar T* 100mm f/2 ZE"},              // 12
    {33, "Carl Zeiss Apo-Sonnar T* 135mm f/2 ZE"},                // 13
    {35, "Canon EF 35-80mm f/4-5.6 II"},
    {36, "Canon EF 38-76mm f/4.5-5.6"},
    {37, "Canon EF 35-80mm f/4-5.6 III"},
    {37, "Tamron 70-200mm f/2.8 Di LD IF Macro"},                            // 1
    {37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"},  // 2
    {37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF]"},       // 3
    {37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro"},  // 4
    {38, "Canon EF 80-200mm f/4.5-5.6 II"},
    {39, "Canon EF 75-300mm f/4-5.6"},
    {40, "Canon EF 28-80mm f/3.5-5.6"},
    {41, "Canon EF 28-90mm f/4-5.6"},
    {42, "Canon EF 28-200mm f/3.5-5.6"},
    {42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro"},  // 1
    {43, "Canon EF 28-105mm f/4-5.6"},
    {44, "Canon EF 90-300mm f/4.5-5.6"},
    {45, "Canon EF-S 18-55mm f/3.5-5.6 [II]"},
    {46, "Canon EF 28-90mm f/4-5.6"},
    {47, "Zeiss Milvus 35mm f/2"},
    {47, "Zeiss Milvus 50mm f/2 Makro"},  // 1
    {47, "Zeiss Milvus 135mm f/2 ZE"},    // 2
    {48, "Canon EF-S 18-55mm f/3.5-5.6 IS"},
    {49, "Canon EF-S 55-250mm f/4-5.6 IS"},
    {50, "Canon EF-S 18-200mm f/3.5-5.6 IS"},
    {51, "Canon EF-S 18-135mm f/3.5-5.6 IS"},
    {52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"},
    {53, "Canon EF-S 18-55mm f/3.5-5.6 III"},
    {54, "Canon EF-S 55-250mm f/4-5.6 IS II"},
    {60, "Irix 11mm f/4"},
    {80, "Canon TS-E 50mm f/2.8L Macro"},
    {81, "Canon TS-E 90mm f/2.8L Macro"},
    {82, "Canon TS-E 135mm f/4L Macro"},
    {94, "Canon TS-E 17mm f/4L"},
    {95, "Canon TS-E 24mm f/3.5L II"},
    {103, "Samyang AF 14mm f/2.8 EF"},
    {103, "Rokinon SP 14mm f/2.4"},     // 1
    {103, "Rokinon AF 14mm f/2.8 EF"},  // 2
    {106, "Rokinon SP / Samyang XP 35mm f/1.2"},
    {112, "Sigma 28mm f/1.5 FF High-speed Prime"},
    {112, "Sigma 40mm f/1.5 FF High-speed Prime"},   // 1
    {112, "Sigma 105mm f/1.5 FF High-speed Prime"},  // 2
    {117, "Tamron 35-150mm f/2.8-4.0 Di VC OSD"},
    {117, "Tamron SP 15-30mm f/2.8 Di VC USD G2"},
    {117, "Tamron SP 35mm f/1.4 Di USD"},  // 1
    {124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"},
    {125, "Canon TS-E 24mm f/3.5L"},
    {126, "Canon TS-E 45mm f/2.8"},
    {127, "Canon TS-E 90mm f/2.8"},
    {127, "Tamron 18-200mm f/3.5-6.3 Di II VC"},  // 1
    {129, "Canon EF 300mm f/2.8L USM"},
    {130, "Canon EF 50mm f/1.0L USM"},
    {131, "Canon EF 28-80mm f/2.8-4L USM"},
    {131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"},      // 1
    {131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"},  // 2
    {131, "Sigma 17-70mm f/2.8-4.5 DC Macro"},            // 3
    {131, "Sigma APO 50-150mm f/2.8 [II] EX DC HSM"},     // 4
    {131, "Sigma APO 120-300mm f/2.8 EX DG HSM"},         // 5
    {131, "Sigma 70-200mm f/2.8 APO EX HSM"},             // 6
    {131, "Sigma 28-70mm f/2.8-4 DG"},                    // 7
    {132, "Canon EF 1200mm f/5.6L USM"},
    {134, "Canon EF 600mm f/4L IS USM"},
    {135, "Canon EF 200mm f/1.8L USM"},
    {136, "Canon EF 300mm f/2.8L USM"},
    {136, "Tamron SP 15-30mm f/2.8 Di VC USD"},  // 1
    {137, "Canon EF 85mm f/1.2L USM"},
    {137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"},          // 1
    {137, "Sigma 50-200mm f/4-5.6 DC OS HSM"},           // 2
    {137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"},         // 3
    {137, "Sigma 24-70mm f/2.8 IF EX DG HSM"},           // 4
    {137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"},         // 5
    {137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM | C"},  // 6
    {137, "Sigma 17-50mm f/2.8 OS HSM"},                 // 7
    {137, "Sigma 18-200mm f/3.5-6.3 DC OS HSM [II]"},    // 8
    {137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"},  // 9
    {137, "Sigma 8-16mm f/4.5-5.6 DC HSM"},              // 10
    {137, "Tamron SP 17-50mm f/2.8 XR Di II VC"},        // 11
    {137, "Tamron SP 60mm f/2 Macro Di II"},             // 12
    {137, "Sigma 10-20mm f/3.5 EX DC HSM"},              // 13
    {137, "Tamron SP 24-70mm f/2.8 Di VC USD"},          // 14
    {137, "Sigma 18-35mm f/1.8 DC HSM | A"},             // 15
    {137, "Sigma 12-24mm f/4.5-5.6 DG HSM II"},          // 16
    {137, "Sigma 70-300mm f/4-5.6 DG OS"},               // 17
    {138, "Canon EF 28-80mm f/2.8-4L"},
    {139, "Canon EF 400mm f/2.8L USM"},
    {140, "Canon EF 500mm f/4.5L USM"},
    {141, "Canon EF 500mm f/4.5L USM"},
    {142, "Canon EF 300mm f/2.8L IS USM"},
    {143, "Canon EF 500mm f/4L IS USM"},
    {143, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"},  // 1
    {143, "Sigma 24-105mm f/4 DG OS HSM | A"},       // 2
    {144, "Canon EF 35-135mm f/4-5.6 USM"},
    {145, "Canon EF 100-300mm f/4.5-5.6 USM"},
    {146, "Canon EF 70-210mm f/3.5-4.5 USM"},
    {147, "Canon EF 35-135mm f/4-5.6 USM"},
    {148, "Canon EF 28-80mm f/3.5-5.6 USM"},
    {149, "Canon EF 100mm f/2 USM"},
    {150, "Canon EF 14mm f/2.8L USM"},
    {150, "Sigma 20mm EX f/1.8"},             // 1
    {150, "Sigma 30mm f/1.4 DC HSM"},         // 2
    {150, "Sigma 24mm f/1.8 DG Macro EX"},    // 3
    {150, "Sigma 28mm f/1.8 DG Macro EX"},    // 4
    {150, "Sigma 18-35mm f/1.8 DC HSM | A"},  // 5
    {151, "Canon EF 200mm f/2.8L USM"},
    {152, "Canon EF 300mm f/4L IS USM"},
    {152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"},  // 1
    {152, "Sigma 14mm f/2.8 EX Aspherical HSM"},            // 2
    {152, "Sigma 10-20mm f/4-5.6"},                         // 3
    {152, "Sigma 100-300mm f/4"},                           // 4
    {152, "Sigma 300-800mm f/5.6 APO EX DG HSM"},           // 5
    {153, "Canon EF 35-350mm f/3.5-5.6L USM"},
    {153, "Sigma 50-500mm f/4-6.3 APO HSM EX"},                               // 1
    {153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"},        // 2
    {153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro"},  // 3
    {153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"},        // 4
    {154, "Canon EF 20mm f/2.8 USM"},
    {154, "Zeiss Milvus 21mm f/2.8"},     // 1
    {154, "Zeiss Milvus 15mm f/2.8 ZE"},  // 2
    {154, "Zeiss Milvus 18mm f/2.8 ZE"},  // 3
    {155, "Canon EF 85mm f/1.8 USM"},
    {155, "Sigma 14mm f/1.8 DG HSM | A"},  // 1
    {156, "Canon EF 28-105mm f/3.5-4.5 USM"},
    {156, "Tamron SP 70-300mm f/4-5.6 Di VC USD"},          // 1
    {156, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"},  // 2
    {160, "Canon EF 20-35mm f/3.5-4.5 USM"},
    {160, "Tamron AF 19-35mm f/3.5-4.5"},                              // 1
    {160, "Tokina AT-X 124 AF Pro DX 12-24mm f/4"},                    // 2
    {160, "Tokina AT-X 107 AF DX 10-17mm f/3.5-4.5 Fisheye"},          // 3
    {160, "Tokina AT-X 116 AF Pro DX 11-16mm f/2.8"},                  // 4
    {160, "Tokina AT-X 11-20 f/2.8 PRO DX Aspherical 11-20mm f/2.8"},  // 5
    {161, "Canon EF 28-70mm f/2.8L USM"},
    {161, "Sigma 24-70mm f/2.8 EX"},                                     // 1
    {161, "Sigma 28-70mm f/2.8 EX"},                                     // 2
    {161, "Sigma 24-60mm f/2.8 EX DG"},                                  // 3
    {161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"},                // 4
    {161, "Tamron 90mm f/2.8"},                                          // 5
    {161, "Tamron SP AF 17-35mm f/2.8-4 Di LD Aspherical IF"},           // 6
    {161, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"},  // 7
    {161, "Tokina AT-X 24-70mm f/2.8 PRO FX (IF)"},                      // 8
    {162, "Canon EF 200mm f/2.8L USM"},
    {163, "Canon EF 300mm f/4L"},
    {164, "Canon EF 400mm f/5.6L"},
    {165, "Canon EF 70-200mm f/2.8L USM"},
    {166, "Canon EF 70-200mm f/2.8L USM + 1.4x"},
    {167, "Canon EF 70-200mm f/2.8L USM + 2x"},
    {168, "Canon EF 28mm f/1.8 USM"},
    {168, "Sigma 50-100mm f/1.8 DC HSM | A"},  // 1
    {169, "Canon EF 17-35mm f/2.8L USM"},
    {169, "Sigma 18-200mm f/3.5-6.3 DC OS"},              // 1
    {169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"},    // 2
    {169, "Sigma 18-50mm f/2.8 Macro"},                   // 3
    {169, "Sigma 50mm f/1.4 EX DG HSM"},                  // 4
    {169, "Sigma 85mm f/1.4 EX DG HSM"},                  // 5
    {169, "Sigma 30mm f/1.4 EX DC HSM"},                  // 6
    {169, "Sigma 35mm f/1.4 DG HSM"},                     // 7
    {169, "Sigma 35mm f/1.5 FF High-Speed Prime | 017"},  // 8
    {169, "Sigma 70mm f/2.8 Macro EX DG"},                // 9
    {170, "Canon EF 200mm f/2.8L II USM"},
    {170, "Sigma 300mm f/2.8 APO EX DG HSM"},  // 1
    {170, "Sigma 800mm f/5.6 APO EX DG HSM"},  // 2
    {171, "Canon EF 300mm f/4L USM"},
    {172, "Canon EF 400mm f/5.6L USM"},
    {172, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"},  // 1
    {172, "Sigma 500mm f/4.5 APO EX DG HSM"},        // 2
    {173, "Canon EF 180mm Macro f/3.5L USM"},
    {173, "Sigma 180mm EX HSM Macro f/3.5"},              // 1
    {173, "Sigma APO Macro 150mm f/2.8 EX DG HSM"},       // 2
    {173, "Sigma 10mm f/2.8 EX DC Fisheye"},              // 3
    {173, "Sigma 15mm f/2.8 EX DG Diagonal Fisheye"},     // 4
    {173, "Venus Laowa 100mm f/2.8 2X Ultra Macro APO"},  // 5
    {173, "Sigma 150-500mm f/5-6.3 APO DG OS HSM + 2x"},  // 6
    {174, "Canon EF 135mm f/2L USM"},
    {174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"},    // 1
    {174, "Sigma 50-500mm f/4.5-6.3 APO DG OS HSM"},   // 2
    {174, "Sigma 150-500mm f/5-6.3 APO DG OS HSM"},    // 3
    {174, "Zeiss Milvus 100mm f/2 Makro"},             // 4
    {174, "Sigma APO 50-150mm f/2.8 EX DC OS HSM"},    // 5
    {174, "Sigma APO 120-300mm f/2.8 EX DG OS HSM"},   // 6
    {174, "Sigma 120-300mm f/2.8 DG OS HSM S013"},     // 7
    {174, "Sigma 120-400mm f/4.5-5.6 APO DG OS HSM"},  // 8
    {174, "Sigma 200-500mm f/2.8 APO EX DG"},          // 9
    {175, "Canon EF 400mm f/2.8L USM"},
    {176, "Canon EF 24-85mm f/3.5-4.5 USM"},
    {177, "Canon EF 300mm f/4L IS USM"},
    {178, "Canon EF 28-135mm f/3.5-5.6 IS"},
    {179, "Canon EF 24mm f/1.4L USM"},
    {180, "Canon EF 35mm f/1.4L USM"},
    {180, "Sigma 50mm f/1.4 DG HSM | A"},                 // 1
    {180, "Sigma 24mm f/1.4 DG HSM | A"},                 // 2
    {180, "Zeiss Milvus 50mm f/1.4"},                     // 3
    {180, "Zeiss Milvus 85mm f/1.4"},                     // 4
    {180, "Zeiss Otus 28mm f/1.4 ZE"},                    // 5
    {180, "Sigma 24mm f/1.5 FF High-Speed Prime | 017"},  // 6
    {180, "Sigma 50mm f/1.5 FF High-Speed Prime | 017"},  // 7
    {180, "Sigma 85mm f/1.5 FF High-Speed Prime | 017"},  // 8
    {180, "Tokina Opera 50mm f/1.4 FF"},                  // 9
    {180, "Sigma 20mm f/1.4 DG HSM | A"},                 // 10
    {181, "Canon EF 100-400mm f/4.5-5.6L IS USM + 1.4x"},
    {181, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 1.4x"},  // 1
    {182, "Canon EF 100-400mm f/4.5-5.6L IS USM + 2x"},
    {182, "Sigma 150-600mm f/5-6.3 DG OS HSM | S + 2x"},  // 1
    {183, "Canon EF 100-400mm f/4.5-5.6L IS USM"},
    {183, "Sigma 150mm f/2.8 EX DG OS HSM APO Macro"},  // 1
    {183, "Sigma 105mm f/2.8 EX DG OS HSM Macro"},      // 2
    {183, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"},  // 3
    {183, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"},     // 4
    {183, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"},     // 5
    {183, "Sigma 100-400mm f/5-6.3 DG OS HSM"},         // 6
    {183, "Sigma 180mm f/3.5 APO Macro EX DG IF HSM"},  // 7
    {184, "Canon EF 400mm f/2.8L USM + 2x"},
    {185, "Canon EF 600mm f/4L IS USM"},
    {186, "Canon EF 70-200mm f/4L USM"},
    {187, "Canon EF 70-200mm f/4L USM + 1.4x"},
    {188, "Canon EF 70-200mm f/4L USM + 2x"},
    {189, "Canon EF 70-200mm f/4L USM + 2.8x"},
    {190, "Canon EF 100mm f/2.8 Macro USM"},
    {191, "Canon EF 400mm f/4 DO IS"},
    {191, "Sigma 500mm f/4 DG OS HSM"},  // 1
    {193, "Canon EF 35-80mm f/4-5.6 USM"},
    {194, "Canon EF 80-200mm f/4.5-5.6 USM"},
    {195, "Canon EF 35-105mm f/4.5-5.6 USM"},
    {196, "Canon EF 75-300mm f/4-5.6 USM"},
    {197, "Canon EF 75-300mm f/4-5.6 IS USM"},
    {197, "Sigma 18-300mm f/3.5-6.3 DC Macro OS HSM"},  // 1
    {198, "Canon EF 50mm f/1.4 USM"},
    {198, "Zeiss Otus 55mm f/1.4 ZE"},    // 1
    {198, "Zeiss Otus 85mm f/1.4 ZE"},    // 2
    {198, "Zeiss Milvus 25mm f/1.4"},     // 3
    {198, "Zeiss Otus 100mm f/1.4"},      // 4
    {198, "Zeiss Milvus 35mm f/1.4 ZE"},  // 5
    {198, "Yongnuo YN 35mm f/2"},         // 6
    {199, "Canon EF 28-80mm f/3.5-5.6 USM"},
    {200, "Canon EF 75-300mm f/4-5.6 USM"},
    {201, "Canon EF 28-80mm f/3.5-5.6 USM"},
    {202, "Canon EF 28-80mm f/3.5-5.6 USM IV"},
    {208, "Canon EF 22-55mm f/4-5.6 USM"},
    {209, "Canon EF 55-200mm f/4.5-5.6"},
    {210, "Canon EF 28-90mm f/4-5.6 USM"},
    {211, "Canon EF 28-200mm f/3.5-5.6 USM"},
    {212, "Canon EF 28-105mm f/4-5.6 USM"},
    {213, "Canon EF 90-300mm f/4.5-5.6 USM"},
    {213, "Tamron SP 150-600mm f/5-6.3 Di VC USD"},         // 1
    {213, "Tamron 16-300mm f/3.5-6.3 Di II VC PZD Macro"},  // 2
    {213, "Tamron SP 35mm f/1.8 Di VC USD"},                // 3
    {213, "Tamron SP 45mm f/1.8 Di VC USD"},                // 4
    {214, "Canon EF-S 18-55mm f/3.5-5.6 USM"},
    {215, "Canon EF 55-200mm f/4.5-5.6 II USM"},
    {217, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"},
    {220, "Yongnuo YN 50mm f/1.8"},
    {224, "Canon EF 70-200mm f/2.8L IS USM"},
    {225, "Canon EF 70-200mm f/2.8L IS USM + 1.4x"},
    {226, "Canon EF 70-200mm f/2.8L IS USM + 2x"},
    {227, "Canon EF 70-200mm f/2.8L IS USM + 2.8x"},
    {228, "Canon EF 28-105mm f/3.5-4.5 USM"},
    {229, "Canon EF 16-35mm f/2.8L USM"},
    {230, "Canon EF 24-70mm f/2.8L USM"},
    {231, "Canon EF 17-40mm f/4L USM"},
    {231, "Sigma 12-24mm f/4 DG HSM A016"},  // 1
    {232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"},
    {233, "Canon EF 28-300mm f/3.5-5.6L IS USM"},
    {234, "Canon EF-S 17-85mm f/4-5.6 IS USM"},
    {234, "Tokina AT-X 12-28 PRO DX 12-28mm f/4"},  // 1
    {235, "Canon EF-S 10-22mm f/3.5-4.5 USM"},
    {236, "Canon EF-S 60mm f/2.8 Macro USM"},
    {237, "Canon EF 24-105mm f/4L IS USM"},
    {238, "Canon EF 70-300mm f/4-5.6 IS USM"},
    {239, "Canon EF 85mm f/1.2L II USM"},
    {239, "Rokinon SP 85mm f/1.2"},  // 1
    {240, "Canon EF-S 17-55mm f/2.8 IS USM"},
    {240, "Sigma 17-50mm f/2.8 EX DC OS HSM"},  // 1
    {241, "Canon EF 50mm f/1.2L USM"},
    {242, "Canon EF 70-200mm f/4L IS USM"},
    {243, "Canon EF 70-200mm f/4L IS USM + 1.4x"},
    {244, "Canon EF 70-200mm f/4L IS USM + 2x"},
    {245, "Canon EF 70-200mm f/4L IS USM + 2.8x"},
    {246, "Canon EF 16-35mm f/2.8L II USM"},
    {247, "Canon EF 14mm f/2.8L II USM"},
    {248, "Canon EF 200mm f/2L IS USM"},
    {248, "Sigma 24-35mm f/2 DG HSM | A"},               // 1
    {248, "Sigma 135mm f/2 FF High-Speed Prime | 017"},  // 2
    {248, "Sigma 24-35mm f/2.2 FF Zoom | 017"},          // 3
    {248, "Sigma 135mm f/1.8 DG HSM A017"},              // 4
    {249, "Canon EF 800mm f/5.6L IS USM"},
    {250, "Canon EF 24mm f/1.4L II USM"},
    {250, "Sigma 20mm f/1.4 DG HSM | A"},                 // 1
    {250, "Sigma 20mm f/1.5 FF High-Speed Prime | 017"},  // 2
    {250, "Tokina Opera 16-28mm f/2.8 FF"},               // 3
    {250, "Sigma 85mm f/1.4 DG HSM A016"},                // 4
    {251, "Canon EF 70-200mm f/2.8L IS II USM"},
    {251, "Canon EF 70-200mm f/2.8L IS III USM"},  // 1
    {252, "Canon EF 70-200mm f/2.8L IS II USM + 1.4x"},
    {252, "Canon EF 70-200mm f/2.8L IS III USM + 1.4x"},  // 1
    {253, "Canon EF 70-200mm f/2.8L IS II USM + 2x"},
    {253, "Canon EF 70-200mm f/2.8L IS III USM + 2x"},  // 1
    {254, "Canon EF 100mm f/2.8L Macro IS USM"},        // 1
    {254, "Tamron SP 90mm f/2.8 Di VC USD Macro 1:1"},  // 2
    {255, "Sigma 24-105mm f/4 DG OS HSM | A"},
    {255, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"},  // 1
    {368, "Sigma 14-24mm f/2.8 DG HSM | A"},
    {368, "Sigma 20mm f/1.4 DG HSM | A"},             // 1
    {368, "Sigma 50mm f/1.4 DG HSM | A"},             // 2
    {368, "Sigma 40mm f/1.4 DG HSM | A"},             // 3
    {368, "Sigma 60-600mm f/4.5-6.3 DG OS HSM | S"},  // 4
    {368, "Sigma 28mm f/1.4 DG HSM | A"},             // 5
    {368, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"},   // 6
    {368, "Sigma 85mm f/1.4 DG HSM | A"},             // 7
    {368, "Sigma 105mm f/1.4 DG HSM"},                // 8
    {368, "Sigma 70mm f/2.8 DG Macro"},               // 9
    {368, "Sigma 18-35mm f/1.8 DC HSM | A"},          // 10
    {368, "Sigma 35mm f/1.4 DG HSM | A"},             // 11
    {488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"},
    {489, "Canon EF 70-300mm f/4-5.6L IS USM"},
    {490, "Canon EF 8-15mm f/4L Fisheye USM"},
    {491, "Canon EF 300mm f/2.8L IS II USM"},
    {491, "Tamron SP 70-200mm f/2.8 Di VC USD G2"},   // 1
    {491, "Tamron 18-400mm f/3.5-6.3 Di II VC HLD"},  // 2
    {491, "Tamron 100-400mm f/4.5-6.3 Di VC USD"},    // 3
    {491, "Tamron 70-210mm f/4 Di VC USD"},           // 4
    {491, "Tamron 70-210mm f/4 Di VC USD + 1.4x"},    // 5
    {491, "Tamron SP 24-70mm f/2.8 Di VC USD G2"},    // 6
    {492, "Canon EF 400mm f/2.8L IS II USM"},
    {493, "Canon EF 500mm f/4L IS II USM"},
    {493, "Canon EF 24-105mm f/4L IS USM"},  // 1
    {494, "Canon EF 600mm f/4L IS II USM"},
    {495, "Canon EF 24-70mm f/2.8L II USM"},
    {495, "Sigma 24-70mm f/2.8 DG OS HSM | A"},  // 1
    {496, "Canon EF 200-400mm f/4L IS USM"},
    {499, "Canon EF 200-400mm f/4L IS USM + 1.4x"},
    {502, "Canon EF 28mm f/2.8 IS USM"},
    {502, "Tamron 35mm f/1.8 Di VC USD"},  // 1
    {503, "Canon EF 24mm f/2.8 IS USM"},
    {504, "Canon EF 24-70mm f/4L IS USM"},
    {505, "Canon EF 35mm f/2 IS USM"},
    {506, "Canon EF 400mm f/4 DO IS II USM"},
    {507, "Canon EF 16-35mm f/4L IS USM"},
    {508, "Canon EF 11-24mm f/4L USM"},
    {508, "Tamron 10-24mm f/3.5-4.5 Di II VC HLD"},  // 1
    {624, "Sigma 50-100mm f/1.8 DC HSM Art"},
    {624, "Sigma 70-200mm f/2.8 DG OS HSM | S"},
    {747, "Canon EF 100-400mm f/4.5-5.6L IS II USM"},
    {747, "Tamron SP 150-600mm f/5-6.3 Di VC USD G2"},  // 1
    {748, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"},
    {748, "Tamron 100-400mm f/4.5-6.3 Di VC USD + 1.4x"},  // 1
    {748, "Tamron 70-210mm f/4 Di VC USD + 2x"},           // 2
    {749, "Tamron 100-400mm f/4.5-6.3 Di VC USD + 2x"},
    {749, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 2x"},  // 1
    {750, "Canon EF 35mm f/1.4L II USM"},
    {750, "Tamron SP 85mm f/1.8 Di VC USD"},  // 1
    {750, "Tamron SP 45mm f/1.8 Di VC USD"},  // 2
    {751, "Canon EF 16-35mm f/2.8L III USM"},
    {752, "Canon EF 24-105mm f/4L IS II USM"},
    {753, "Canon EF 85mm f/1.4L IS USM"},
    {754, "Canon EF 70-200mm f/4L IS II USM"},
    {757, "Canon EF 400mm f/2.8L IS III USM"},
    {758, "Canon EF 600mm f/4L IS III USM"},
    {1136, "Sigma 24-70mm f/2.8 DG OS HSM | Art 017"},
    {4142, "Canon EF-S 18-135mm f/3.5-5.6 IS STM"},
    {4143, "Canon EF-M 18-55mm f/3.5-5.6 IS STM"},
    {4143, "Tamron 18-200mm f/3.5-6.3 Di III VC"},  // 1
    {4144, "Canon EF 40mm f/2.8 STM"},
    {4145, "Canon EF-M 22mm f/2 STM"},
    {4146, "Canon EF-S 18-55mm f/3.5-5.6 IS STM"},
    {4147, "Canon EF-M 11-22mm f/4-5.6 IS STM"},
    {4148, "Canon EF-S 55-250mm f/4-5.6 IS STM"},
    {4149, "Canon EF-M 55-200mm f/4.5-6.3 IS STM"},
    {4150, "Canon EF-S 10-18mm f/4.5-5.6 IS STM"},
    {4152, "Canon EF 24-105mm f/3.5-5.6 IS STM"},
    {4153, "Canon EF-M 15-45mm f/3.5-6.3 IS STM"},
    {4154, "Canon EF-S 24mm f/2.8 STM"},
    {4155, "Canon EF-M 28mm f/3.5 Macro IS STM"},
    {4156, "Canon EF 50mm f/1.8 STM"},
    {4157, "Canon EF-M 18-150mm f/3.5-6.3 IS STM"},
    {4158, "Canon EF-S 18-55mm f/4-5.6 IS STM"},
    {4159, "Canon EF-M 32mm f/1.4 STM"},
    {4160, "Canon EF-S 35mm f/2.8 Macro IS STM"},
    {4208, "Sigma 56mm f/1.4 DC DN | C"},
    {4208, "Sigma 30mm f/1.4 DC DN | C"},
    {36910, "Canon EF 70-300mm f/4-5.6 IS II USM"},
    {36912, "Canon EF-S 18-135mm f/3.5-5.6 IS USM"},
    {61491, "Canon CN-E 14mm T3.1 L F"},
    {61492, "Canon CN-E 24mm T1.5 L F"},
    {61494, "Canon CN-E 85mm T1.3 L F"},
    {61495, "Canon CN-E 135mm T2.2 L F"},
    {61496, "Canon CN-E 35mm T1.5 L F"},
    // All RF lenses seem to share the LensType value 61182;
    // unique RFLensType tag below is to be preferred instead.
    // Please keep this list in sync w/ RFLensType list
    {61182, "Canon RF 50mm F1.2 L USM"},
    {61182, "Canon RF 24-105mm F4 L IS USM"},
    {61182, "Canon RF 28-70mm F2 L USM"},
    {61182, "Canon RF 35mm F1.8 MACRO IS STM"},
    {61182, "Canon RF 85mm F1.2 L USM"},
    {61182, "Canon RF 85mm F1.2 L USM DS"},
    {61182, "Canon RF 24-70mm F2.8 L IS USM"},
    {61182, "Canon RF 15-35mm F2.8 L IS USM"},
    {61182, "Canon RF 24-240mm F4-6.3 IS USM"},
    {61182, "Canon RF 70-200mm F2.8 L IS USM"},
    {61182, "Canon RF 85mm F2 MACRO IS STM"},
    {61182, "Canon RF 600mm F11 IS STM"},
    {61182, "Canon RF 600mm F11 IS STM + RF1.4x"},
    {61182, "Canon RF 600mm F11 IS STM + RF2x"},
    {61182, "Canon RF 800mm F11 IS STM"},
    {61182, "Canon RF 800mm F11 IS STM + RF1.4x"},
    {61182, "Canon RF 800mm F11 IS STM + RF2x"},
    {61182, "Canon RF 24-105mm F4-7.1 IS STM"},
    {61182, "Canon RF 100-500mm F4.5-7.1 L IS USM"},
    {61182, "Canon RF 100-500mm F4.5-7.1 L IS USM + RF1.4x"},
    {61182, "Canon RF 100-500mm F4.5-7.1 L IS USM + RF2x"},
    {61182, "Canon RF 70-200mm F4 L IS USM"},
    {61182, "Canon RF 100mm F2.8 L MACRO IS USM"},
    {61182, "Canon RF 50mm F1.8 STM"},
    {61182, "Canon RF 14-35mm F4 L IS USM"},
    {61182, "Canon RF-S 18-45mm F4.5-6.3 IS STM"},
    {61182, "Canon RF 100-400mm F5.6-8 IS USM"},
    {61182, "Canon RF 100-400mm F5.6-8 IS USM + RF1.4x"},
    {61182, "Canon RF 100-400mm F5.6-8 IS USM + RF2x"},
    {61182, "Canon RF-S 18-150mm F3.5-6.3 IS STM"},
    {61182, "Canon RF 24mm F1.8 MACRO IS STM"},
    {61182, "Canon RF 16mm F2.8 STM"},
    {61182, "Canon RF 400mm F2.8 L IS USM"},
    {61182, "Canon RF 400mm F2.8 L IS USM + RF1.4x"},
    {61182, "Canon RF 400mm F2.8 L IS USM + RF2x"},
    {61182, "Canon RF 600mm F4 L IS USM"},
    {61182, "Canon RF 600mm F4 L IS USM + RF1.4x"},
    {61182, "Canon RF 600mm F4 L IS USM + RF2x"},
    {61182, "Canon RF 800mm F5.6 L IS USM"},
    {61182, "Canon RF 800mm F5.6 L IS USM + RF1.4x"},
    {61182, "Canon RF 800mm F5.6 L IS USM + RF2x"},
    {61182, "Canon RF 1200mm F8 L IS USM"},
    {61182, "Canon RF 1200mm F8 L IS USM + RF1.4x"},
    {61182, "Canon RF 1200mm F8 L IS USM + RF2x"},
    {61182, "Canon RF 5.2mm F2.8 L Dual Fisheye 3D VR"},
    {61182, "Canon RF 15-30mm F4.5-6.3 IS STM"},
    {61182, "Canon RF 135mm F1.8 L IS USM"},
    {61182, "Canon RF 24-50mm F4.5-6.3 IS STM"},
    {61182, "Canon RF-S 55-210mm F5-7.1 IS STM"},
    {61182, "Canon RF 100-300mm F2.8 L IS USM"},
    {61182, "Canon RF 100-300mm F2.8 L IS USM + RF1.4x"},
    {61182, "Canon RF 100-300mm F2.8 L IS USM + RF2x"},
    {61182, "Canon RF 200-800mm F6.3-9 IS USM"},
    {61182, "Canon RF 200-800mm F6.3-9 IS USM + RF1.4x"},
    {61182, "Canon RF 200-800mm F6.3-9 IS USM + RF2x"},
    {61182, "Canon RF 10-20mm F4 L IS STM"},
    {61182, "Canon RF 28mm F2.8 STM"},
    {61182, "Canon RF 24-105mm F2.8 L IS USM Z"},
    {61182, "Canon RF-S 10-18mm F4.5-6.3 IS STM"},
    {61182, "Canon RF 35mm F1.4 L VCM"},
    {61182, "Canon RF-S 3.9mm F3.5 STM Dual Fisheye"},
    {61182, "Canon RF 28-70mm F2.8 IS STM"},
    {61182, "Canon RF 70-200mm F2.8 L IS USM Z"},
    {61182, "Canon RF 70-200mm F2.8 L IS USM Z + RF1.4x"},
    {61182, "Canon RF 70-200mm F2.8 L IS USM Z + RF2x"},
    {61182, "Canon RF 50mm F1.4 L VCM"},
    {61182, "Canon RF 24mm F1.4 L VCM"},
    {65535, N_("n/a")},
};

//! FlashActivity, tag 0x001c
constexpr TagDetails canonCsFlashActivity[] = {
    {0, N_("Did not fire")},
    {1, N_("Fired")},
};

//! FlashDetails, tag 0x001d
constexpr TagDetailsBitmask canonCsFlashDetails[] = {
    {0x4000, N_("External flash")},  {0x2000, N_("Internal flash")},
    {0x0001, N_("Manual")},          {0x0002, N_("TTL")},
    {0x0004, N_("A-TTL")},           {0x0008, N_("E-TTL")},
    {0x0010, N_("FP sync enabled")}, {0x0080, N_("2nd-curtain sync used")},
    {0x0800, N_("FP sync used")},
};

//! FocusContinuous, tag 0x0020
constexpr TagDetails canonCsFocusContinuous[] = {
    {0, N_("Single")},
    {1, N_("Continuous")},
    {8, N_("Manual")},
};

//! AESetting, tag 0x0021
constexpr TagDetails canonCsAESetting[] = {
    {0, N_("Normal AE")}, {1, N_("Exposure compensation")},
    {2, N_("AE lock")},   {3, N_("AE lock + exposure compensation")},
    {4, N_("No AE")},     {65535, N_("n/a")},
};

//! ImageStabilization, tag 0x0022
constexpr TagDetails canonCsImageStabilization[] = {
    {0, N_("Off")},           {1, N_("On")},
    {2, N_("Shoot Only")},    {3, N_("Panning")},
    {4, N_("Dynamic")},       {256, N_("Off (2)")},
    {257, N_("On (2)")},      {258, N_("Shoot Only (2)")},
    {259, N_("Panning (2)")}, {260, N_("Dynamic (2)")},
};

//! SpotMeteringMode, tag 0x0027
constexpr TagDetails canonCsSpotMeteringMode[] = {
    {0, N_("Center")},
    {1, N_("AF Point")},
    {65535, N_("n/a")},
};

//! PhotoEffect, tag 0x0028
constexpr TagDetails canonCsPhotoEffect[] = {
    {0, N_("Off")}, {1, N_("Vivid")},  {2, N_("Neutral")},         {3, N_("Smooth")},  {4, N_("Sepia")},
    {5, N_("B&W")}, {6, N_("Custom")}, {100, N_("My color data")}, {65535, N_("n/a")},
};

//! ManualFlashOutput, tag 0x0029
[[maybe_unused]] constexpr TagDetails canonCsManualFlashOutput[] = {
    {0x0000, N_("n/a")}, {0x0500, N_("Full")}, {0x0502, N_("Medium")}, {0x0504, N_("Low")}, {0x7fff, N_("n/a")},
};

//! SRAWQuality, tag 0x002e
constexpr TagDetails canonCsSRAWQuality[] = {
    {0, N_("n/a")},
    {1, N_("sRAW1 (mRAW)")},
    {2, N_("sRAW2 (sRAW)")},
};

// Canon Camera Settings Tag Info
constexpr TagInfo CanonMakerNote::tagInfoCs_[] = {
    {0x0001, "Macro", N_("Macro"), N_("Macro mode"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsMacro)},
    {0x0002, "Selftimer", N_("Selftimer"), N_("Self timer"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     printCs0x0002},
    {0x0003, "Quality", N_("Quality"), N_("Quality"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsQuality)},
    {0x0004, "FlashMode", N_("Flash Mode"), N_("Flash mode setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsFlashMode)},
    {0x0005, "DriveMode", N_("Drive Mode"), N_("Drive mode setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsDriveMode)},
    {0x0006, "0x0006", "0x0006", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsFocusMode)},
    {0x0008, "0x0008", "0x0008", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1, printValue},
    {0x0009, "RecordMode", "RecordMode", N_("Record Mode"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsRecordMode)},
    {0x000a, "ImageSize", N_("Image Size"), N_("Image size"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsImageSize)},
    {0x000b, "EasyMode", N_("Easy Mode"), N_("Easy shooting mode"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonCsEasyMode)},
    {0x000c, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonCsDigitalZoom)},
    {0x000d, "Contrast", N_("Contrast"), N_("Contrast setting"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsLnh)},
    {0x000e, "Saturation", N_("Saturation"), N_("Saturation setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsLnh)},
    {0x000f, "Sharpness", N_("Sharpness"), N_("Sharpness setting"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonCsLnh)},
    {0x0010, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsISOSpeed)},
    {0x0011, "MeteringMode", N_("Metering Mode"), N_("Metering mode setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsMeteringMode)},
    {0x0012, "FocusType", N_("Focus Type"), N_("Focus type setting"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsFocusType)},
    {0x0013, "AFPoint", N_("AF Point"), N_("AF point selected"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsAfPoint)},
    {0x0014, "ExposureProgram", N_("Exposure Program"), N_("Exposure mode setting"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsExposureProgram)},
    {0x0015, "0x0015", "0x0015", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1, printValue},
    {0x0016, "LensType", N_("Lens Type"), N_("Lens type"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     printCsLensType},
    {0x0017, "Lens", N_("Lens"),
     N_("'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm"), IfdId::canonCsId,
     SectionId::makerTags, unsignedShort, 3, printCsLens},
    {0x0018, "ShortFocal", N_("Short Focal"), N_("Short focal"), IfdId::canonCsId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0019, "FocalUnits", N_("Focal Units"), N_("Focal units"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     printValue},
    {0x001a, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, printSi0x0015},
    {0x001b, "MinAperture", N_("Min Aperture"), N_("Min aperture"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, printSi0x0015},
    {0x001c, "FlashActivity", N_("Flash Activity"), N_("Flash activity"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsFlashActivity)},
    {0x001d, "FlashDetails", N_("Flash Details"), N_("Flash details"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG_BITMASK(canonCsFlashDetails)},
    {0x001e, "0x001e", "0x001e", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1, printValue},
    {0x001f, "0x001f", "0x001f", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1, printValue},
    {0x0020, "FocusContinuous", N_("Focus Continuous"), N_("Focus continuous setting"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusContinuous)},
    {0x0021, "AESetting", N_("AESetting"), N_("AE setting"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonCsAESetting)},
    {0x0022, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageStabilization)},
    {0x0023, "DisplayAperture", N_("Display Aperture"), N_("Display aperture"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, printValue},
    {0x0024, "ZoomSourceWidth", N_("Zoom Source Width"), N_("Zoom source width"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0025, "ZoomTargetWidth", N_("Zoom Target Width"), N_("Zoom target width"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0026, "0x0026", "0x0026", N_("Unknown"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1, printValue},
    {0x0027, "SpotMeteringMode", N_("Spot Metering Mode"), N_("Spot metering mode"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSpotMeteringMode)},
    {0x0028, "PhotoEffect", N_("Photo Effect"), N_("Photo effect"), IfdId::canonCsId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonCsPhotoEffect)},
    {0x0029, "ManualFlashOutput", N_("Manual Flash Output"), N_("Manual flash output"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)},
    {0x002a, "ColorTone", N_("Color Tone"), N_("Color tone"), IfdId::canonCsId, SectionId::makerTags, signedShort, 1,
     printValue},
    {0x002e, "SRAWQuality", N_("SRAW Quality Tone"), N_("SRAW quality"), IfdId::canonCsId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)},
    // End of list marker
    {0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), IfdId::canonCsId,
     SectionId::makerTags, signedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListCs() {
  return tagInfoCs_;
}

//! AFPointUsed, tag 0x000e
constexpr TagDetailsBitmask canonSiAFPointUsed[] = {
    {0x0004, N_("left")},
    {0x0002, N_("center")},
    {0x0001, N_("right")},
};

//! FlashBias, tag 0x000f
constexpr TagDetails canonSiFlashBias[] = {
    {0xffc0, "-2 EV"},    {0xffcc, "-1.67 EV"}, {0xffd0, "-1.50 EV"}, {0xffd4, "-1.33 EV"}, {0xffe0, "-1 EV"},
    {0xffec, "-0.67 EV"}, {0xfff0, "-0.50 EV"}, {0xfff4, "-0.33 EV"}, {0x0000, "0 EV"},     {0x000c, "0.33 EV"},
    {0x0010, "0.50 EV"},  {0x0014, "0.67 EV"},  {0x0020, "1 EV"},     {0x002c, "1.33 EV"},  {0x0030, "1.50 EV"},
    {0x0034, "1.67 EV"},  {0x0040, "2 EV"},
};

constexpr TagDetails cameraType[] = {
    {0, "n/a"}, {248, "EOS High-end"}, {250, "Compact"}, {252, "EOS Mid-range"}, {255, "DV Camera"},
};

constexpr TagDetails autoExposureBracketing[] = {
    {65535, "On"}, {0, "Off"}, {1, "On (shot 1)"}, {2, "On (shot 2)"}, {3, "On (shot 3)"},
};

constexpr TagDetails slowShutter[] = {
    {65535, "n/a"}, {0, "Off"}, {1, "Night Scene"}, {2, "On"}, {3, "None"},
};

constexpr TagDetails autoRotate[] = {
    {-1, "n/a"}, {0, "None"}, {1, "Rotate 90 CW"}, {2, "Rotate 180"}, {3, "Rotate 270 CW"},
};

constexpr TagDetails ndRotate[] = {
    {65535, "n/a"},
    {0, "Off"},
    {1, "On"},
};

constexpr TagDetails selfTimer2[] = {
    {65535, "n/a"},
    {0, "Selftimer 2s"},
    {1, "Selftimer 10s"},
};

// Canon Shot Info Tag
constexpr TagInfo CanonMakerNote::tagInfoSi_[] = {
    {0x0001, "AutoISO", N_("AutoISO"), N_("AutoISO"), IfdId::canonSiId, SectionId::makerTags, unsignedShort, 1,
     printSi0x0001},
    {0x0002, "ISOSpeed", N_("ISO Speed Used"), N_("ISO speed used"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0002},
    {0x0003, "MeasuredEV", N_("Measured EV"), N_("Measured EV"), IfdId::canonSiId, SectionId::makerTags, unsignedShort,
     1, printSi0x0003},
    {0x0004, "TargetAperture", N_("Target Aperture"), N_("Target Aperture"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0015},
    {0x0005, "TargetShutterSpeed", N_("Target Shutter Speed"), N_("Target shutter speed"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, printSi0x0016},
    {0x0006, "ExposureCompensation", "Exposure Compensation", N_("Exposure Compensation"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0007, "WhiteBalance", N_("White Balance"), N_("White balance setting"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)},
    {0x0008, "SlowShutter", N_("Slow Shutter"), N_("Slow shutter"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(slowShutter)},
    {0x0009, "Sequence", N_("Sequence"), N_("Sequence number (if in a continuous burst)"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, printSi0x0009},
    {0x000a, "OpticalZoomCode", "Optical Zoom Code", N_("Optical Zoom Code"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x000b, "0x000b", "0x000b", N_("Unknown"), IfdId::canonSiId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000c, "CameraTemperature", N_("Camera Temperature"), N_("Camera temperature"), IfdId::canonSiId,
     SectionId::makerTags, signedShort, 1, printSi0x000c},
    {0x000d, "FlashGuideNumber", N_("Flash Guide Number"), N_("Flash guide number"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, printSi0x000d},
    {0x000e, "AFPointUsed", N_("AF Point Used"), N_("AF point used"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x000e},
    {0x000f, "FlashBias", N_("Flash Bias"), N_("Flash bias"), IfdId::canonSiId, SectionId::makerTags, unsignedShort, 1,
     EXV_PRINT_TAG(canonSiFlashBias)},
    {0x0010, "AutoExposureBracketing", N_("Auto Exposure Bracketing"), N_("Auto exposure bracketing"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(autoExposureBracketing)},
    {0x0011, "AEBBracketValue", "AEB Bracket Value", N_("AEB Bracket Value"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0012, "ControlMode", "Control Mode", N_("Control Mode"), IfdId::canonSiId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0013, "SubjectDistance", N_("Subject Distance"), N_("Subject distance"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0013},
    {0x0014, "0x0014", "0x0014", N_("Unknown"), IfdId::canonSiId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0015, "ApertureValue", N_("Aperture Value"), N_("Aperture"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0015},
    {0x0016, "ShutterSpeedValue", N_("Shutter Speed Value"), N_("Shutter speed"), IfdId::canonSiId,
     SectionId::makerTags, unsignedShort, 1, printSi0x0016},
    {0x0017, "MeasuredEV2", N_("Measured EV 2"), N_("Measured EV 2"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0017},
    {0x0018, "BulbDuration", N_("Bulb Duration"), N_("Bulb duration"), IfdId::canonSiId, SectionId::makerTags,
     unsignedShort, 1, printSi0x0018},
    {0x0019, "0x0019", "0x0019", N_("Unknown"), IfdId::canonSiId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x001a, "CameraType", N_("Camera Type"), N_("Camera type"), IfdId::canonSiId, SectionId::makerTags, unsignedShort,
     1, EXV_PRINT_TAG(cameraType)},
    {0x001b, "AutoRotate", N_("Auto Rotate"), N_("Auto rotate"), IfdId::canonSiId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(autoRotate)},
    {0x001c, "NDFilter", N_("ND Filter"), N_("ND filter"), IfdId::canonSiId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(ndRotate)},
    {0x001d, "SelfTimer2", N_("Self Timer 2"), N_("Self timer2"), IfdId::canonSiId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(selfTimer2)},
    {0x0021, "FlashOutput", N_("Flash Output"), N_("Flash output"), IfdId::canonSiId, SectionId::makerTags, signedShort,
     1, printValue},
    // End of list marker
    {0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), IfdId::canonCsId,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListSi() {
  return tagInfoSi_;
}

//! PanoramaDirection, tag 0x0005
constexpr TagDetails canonPaDirection[] = {
    {0, N_("Left to right")}, {1, N_("Right to left")},          {2, N_("Bottom to top")},
    {3, N_("Top to bottom")}, {4, N_("2x2 matrix (Clockwise)")},
};

// Canon Panorama Info
constexpr TagInfo CanonMakerNote::tagInfoPa_[] = {
    {0x0002, "PanoramaFrame", N_("Panorama Frame"), N_("Panorama frame number"), IfdId::canonPaId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0005, "PanoramaDirection", N_("Panorama Direction"), N_("Panorama direction"), IfdId::canonPaId,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonPaDirection)},
    // End of list marker
    {0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", N_("Unknown Canon Panorama tag"), IfdId::canonPaId,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListPa() {
  return tagInfoPa_;
}

// Canon Custom Function Tag Info
constexpr TagInfo CanonMakerNote::tagInfoCf_[] = {
    {0x0001, "NoiseReduction", N_("Noise Reduction"), N_("Long exposure noise reduction"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0002, "ShutterAeLock", N_("Shutter Ae Lock"), N_("Shutter/AE lock buttons"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0003, "MirrorLockup", N_("Mirror Lockup"), N_("Mirror lockup"), IfdId::canonCfId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0004, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Tv/Av and exposure level"),
     IfdId::canonCfId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0005, "AFAssist", N_("AF Assist"), N_("AF assist light"), IfdId::canonCfId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0006, "FlashSyncSpeedAv", N_("Flash Sync Speed Av"), N_("Shutter speed in Av mode"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0007, "AEBSequence", N_("AEB Sequence"), N_("AEB sequence/auto cancellation"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0008, "ShutterCurtainSync", N_("Shutter Curtain Sync"), N_("Shutter curtain sync"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0009, "LensAFStopButton", N_("Lens AF Stop Button"), N_("Lens AF stop button Fn. Switch"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000a, "FillFlashAutoReduction", N_("Fill Flash Auto Reduction"), N_("Auto reduction of fill flash"),
     IfdId::canonCfId, SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000b, "MenuButtonReturn", N_("Menu Button Return"), N_("Menu button return position"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000c, "SetButtonFunction", N_("Set Button Function"), N_("SET button func. when shooting"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000d, "SensorCleaning", N_("Sensor Cleaning"), N_("Sensor cleaning"), IfdId::canonCfId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x000e, "SuperimposedDisplay", N_("Superimposed Display"), N_("Superimposed display"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x000f, "ShutterReleaseNoCFCard", N_("Shutter Release No CF Card"), N_("Shutter Release W/O CF Card"),
     IfdId::canonCfId, SectionId::makerTags, unsignedShort, 1, printValue},
    // End of list marker
    {0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", N_("Unknown Canon Custom Function tag"), IfdId::canonCfId,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListCf() {
  return tagInfoCf_;
}

//! AFPointsUsed, tag 0x0016
constexpr TagDetailsBitmask canonPiAFPointsUsed[] = {
    {0x01, N_("right")}, {0x02, N_("mid-right")}, {0x04, N_("bottom")}, {0x08, N_("center")},
    {0x10, N_("top")},   {0x20, N_("mid-left")},  {0x40, N_("left")},
};

//! AFPointsUsed20D, tag 0x001a
constexpr TagDetailsBitmask canonPiAFPointsUsed20D[] = {
    {0x001, N_("top")},        {0x002, N_("upper-left")},  {0x004, N_("upper-right")},
    {0x008, N_("left")},       {0x010, N_("center")},      {0x020, N_("right")},
    {0x040, N_("lower-left")}, {0x080, N_("lower-right")}, {0x100, N_("bottom")},
};

// Canon Picture Info Tag
constexpr TagInfo CanonMakerNote::tagInfoPi_[] = {
    {0x0002, "ImageWidth", N_("Image Width"), N_("Image width"), IfdId::canonPiId, SectionId::makerTags, unsignedShort,
     1, printValue},
    {0x0003, "ImageHeight", N_("Image Height"), N_("Image height"), IfdId::canonPiId, SectionId::makerTags,
     unsignedShort, 1, printValue},
    {0x0004, "ImageWidthAsShot", N_("Image Width As Shot"), N_("Image width (as shot)"), IfdId::canonPiId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0005, "ImageHeightAsShot", N_("Image Height As Shot"), N_("Image height (as shot)"), IfdId::canonPiId,
     SectionId::makerTags, unsignedShort, 1, printValue},
    {0x0016, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), IfdId::canonPiId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed)},
    {0x001a, "AFPointsUsed20D", N_("AF Points Used 20D"), N_("AF points used (20D)"), IfdId::canonPiId,
     SectionId::makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed20D)},
    // End of list marker
    {0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", N_("Unknown Canon Picture Info tag"), IfdId::canonPiId,
     SectionId::makerTags, unsignedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListPi() {
  return tagInfoPi_;
}

//! BracketMode, tag 0x0003
constexpr TagDetails canonBracketMode[] = {
    {0, N_("Off")}, {1, N_("AEB")}, {2, N_("FEB")}, {3, N_("ISO")}, {4, N_("WB")},
};

//! RawJpgSize, tag 0x0007
constexpr TagDetails canonRawJpgSize[] = {
    {-1, N_("n/a")},
    {0, N_("Large")},
    {1, N_("Medium")},
    {2, N_("Small")},
    {5, N_("Medium 1")},
    {6, N_("Medium 2")},
    {7, N_("Medium 3")},
    {8, N_("Postcard")},
    {9, N_("Widescreen")},
    {10, N_("Medium Widescreen")},
    {14, N_("Small 1")},
    {15, N_("Small 2")},
    {16, N_("Small 3")},
    {128, N_("640x480 Movie")},
    {129, N_("Medium Movie")},
    {130, N_("Small Movie")},
    {137, N_("1280x720 Movie")},
    {142, N_("1920x1080 Movie")},
    {143, N_("4096x2160 Movie")},
};

//! NoiseReduction, tag 0x0008
constexpr TagDetails canonNoiseReduction[] = {
    {0, N_("Off")}, {1, N_("On 1")}, {2, N_("On 2")}, {3, N_("On")}, {4, N_("Auto")},
};

//! WBBracketMode, tag 0x0009
constexpr TagDetails canonWBBracketMode[] = {
    {0, N_("Off")},
    {1, N_("On (shift AB)")},
    {2, N_("On (shift GM)")},
};

//! FilterEffect, tag 0x000e
constexpr TagDetails canonFilterEffect[] = {
    {0, N_("None")}, {1, N_("Yellow")}, {2, N_("Orange")}, {3, N_("Red")}, {4, N_("Green")},
};

//! ToningEffect, tag 0x000e
constexpr TagDetails canonToningEffect[] = {
    {0, N_("None")}, {1, N_("Sepia")}, {2, N_("Blue")}, {3, N_("Purple")}, {4, N_("Green")},
};

//! RFLensType, tag 0x003D
// from https://github.com/exiftool/exiftool/blob/13.12/lib/Image/ExifTool/Canon.pm#L6942
constexpr TagDetails canonRFLensType[] = {
    {0, N_("n/a")},
    {257, "Canon RF 50mm F1.2 L USM"},
    {258, "Canon RF 24-105mm F4 L IS USM"},
    {259, "Canon RF 28-70mm F2 L USM"},
    {260, "Canon RF 35mm F1.8 MACRO IS STM"},
    {261, "Canon RF 85mm F1.2 L USM"},
    {262, "Canon RF 85mm F1.2 L USM DS"},
    {263, "Canon RF 24-70mm F2.8 L IS USM"},
    {264, "Canon RF 15-35mm F2.8 L IS USM"},
    {265, "Canon RF 24-240mm F4-6.3 IS USM"},
    {266, "Canon RF 70-200mm F2.8 L IS USM"},
    {267, "Canon RF 85mm F2 MACRO IS STM"},
    {268, "Canon RF 600mm F11 IS STM"},
    {269, "Canon RF 600mm F11 IS STM + RF1.4x"},
    {270, "Canon RF 600mm F11 IS STM + RF2x"},
    {271, "Canon RF 800mm F11 IS STM"},
    {272, "Canon RF 800mm F11 IS STM + RF1.4x"},
    {273, "Canon RF 800mm F11 IS STM + RF2x"},
    {274, "Canon RF 24-105mm F4-7.1 IS STM"},
    {275, "Canon RF 100-500mm F4.5-7.1 L IS USM"},
    {276, "Canon RF 100-500mm F4.5-7.1 L IS USM + RF1.4x"},
    {277, "Canon RF 100-500mm F4.5-7.1 L IS USM + RF2x"},
    {278, "Canon RF 70-200mm F4 L IS USM"},
    {279, "Canon RF 100mm F2.8 L MACRO IS USM"},
    {280, "Canon RF 50mm F1.8 STM"},
    {281, "Canon RF 14-35mm F4 L IS USM"},
    {282, "Canon RF-S 18-45mm F4.5-6.3 IS STM"},
    {283, "Canon RF 100-400mm F5.6-8 IS USM"},
    {284, "Canon RF 100-400mm F5.6-8 IS USM + RF1.4x"},
    {285, "Canon RF 100-400mm F5.6-8 IS USM + RF2x"},
    {286, "Canon RF-S 18-150mm F3.5-6.3 IS STM"},
    {287, "Canon RF 24mm F1.8 MACRO IS STM"},
    {288, "Canon RF 16mm F2.8 STM"},
    {289, "Canon RF 400mm F2.8 L IS USM"},
    {290, "Canon RF 400mm F2.8 L IS USM + RF1.4x"},
    {291, "Canon RF 400mm F2.8 L IS USM + RF2x"},
    {292, "Canon RF 600mm F4 L IS USM"},
    {293, "Canon RF 600mm F4 L IS USM + RF1.4x"},
    {294, "Canon RF 600mm F4 L IS USM + RF2x"},
    {295, "Canon RF 800mm F5.6 L IS USM"},
    {296, "Canon RF 800mm F5.6 L IS USM + RF1.4x"},
    {297, "Canon RF 800mm F5.6 L IS USM + RF2x"},
    {298, "Canon RF 1200mm F8 L IS USM"},
    {299, "Canon RF 1200mm F8 L IS USM + RF1.4x"},
    {300, "Canon RF 1200mm F8 L IS USM + RF2x"},
    {301, "Canon RF 5.2mm F2.8 L Dual Fisheye 3D VR"},
    {302, "Canon RF 15-30mm F4.5-6.3 IS STM"},
    {303, "Canon RF 135mm F1.8 L IS USM"},
    {304, "Canon RF 24-50mm F4.5-6.3 IS STM"},
    {305, "Canon RF-S 55-210mm F5-7.1 IS STM"},
    {306, "Canon RF 100-300mm F2.8 L IS USM"},
    {307, "Canon RF 100-300mm F2.8 L IS USM + RF1.4x"},
    {308, "Canon RF 100-300mm F2.8 L IS USM + RF2x"},
    {309, "Canon RF 200-800mm F6.3-9 IS USM"},
    {310, "Canon RF 200-800mm F6.3-9 IS USM + RF1.4x"},
    {311, "Canon RF 200-800mm F6.3-9 IS USM + RF2x"},
    {312, "Canon RF 10-20mm F4 L IS STM"},
    {313, "Canon RF 28mm F2.8 STM"},
    {314, "Canon RF 24-105mm F2.8 L IS USM Z"},
    {315, "Canon RF-S 10-18mm F4.5-6.3 IS STM"},
    {316, "Canon RF 35mm F1.4 L VCM"},
    {317, "Canon RF-S 3.9mm F3.5 STM Dual Fisheye"},
    {318, "Canon RF 28-70mm F2.8 IS STM"},
    {319, "Canon RF 70-200mm F2.8 L IS USM Z"},
    {320, "Canon RF 70-200mm F2.8 L IS USM Z + RF1.4x"},
    {321, "Canon RF 70-200mm F2.8 L IS USM Z + RF2x"},
    {325, "Canon RF 50mm F1.4 L VCM"},
    {326, "Canon RF 24mm F1.4 L VCM"},
};

// Canon File Info Tag
constexpr TagInfo CanonMakerNote::tagInfoFi_[] = {
    {0x0001, "FileNumber", N_("File Number"), N_("File Number"), IfdId::canonFiId, SectionId::makerTags, unsignedLong,
     1, printFiFileNumber},
    {0x0003, "BracketMode", N_("Bracket Mode"), N_("Bracket Mode"), IfdId::canonFiId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonBracketMode)},
    {0x0004, "BracketValue", N_("Bracket Value"), N_("Bracket Value"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, printValue},
    {0x0005, "BracketShotNumber", N_("Bracket Shot Number"), N_("Bracket Shot Number"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0006, "RawJpgQuality", N_("Raw Jpg Quality"), N_("Raw Jpg Quality"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonCsQuality)},
    {0x0007, "RawJpgSize", N_("Raw Jpg Size"), N_("Raw Jpg Size"), IfdId::canonFiId, SectionId::makerTags, signedShort,
     1, EXV_PRINT_TAG(canonRawJpgSize)},
    {0x0008, "NoiseReduction", N_("Noise Reduction"), N_("Noise Reduction"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonNoiseReduction)},
    {0x0009, "WBBracketMode", N_("WB Bracket Mode"), N_("WB Bracket Mode"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonWBBracketMode)},
    {0x000c, "WBBracketValueAB", N_("WB Bracket Value AB"), N_("WB Bracket Value AB"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x000d, "WBBracketValueGM", N_("WB Bracket Value GM"), N_("WB Bracket Value GM"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x000e, "FilterEffect", N_("Filter Effect"), N_("Filter Effect"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonFilterEffect)},
    {0x000f, "ToningEffect", N_("Toning Effect"), N_("Toning Effect"), IfdId::canonFiId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonToningEffect)},
    {0x0010, "MacroMagnification", N_("Macro Magnification"), N_("Macro magnification"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0013, "LiveViewShooting", N_("Live View Shooting"), N_("Live view shooting"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)},
    {0x0014, "FocusDistanceUpper", N_("Focus Distance Upper"), N_("Focus Distance Upper"), IfdId::canonFiId,
     SectionId::makerTags, unsignedShort, 1, printFiFocusDistance},
    {0x0015, "FocusDistanceLower", N_("Focus Distance Lower"), N_("Focus Distance Lower"), IfdId::canonFiId,
     SectionId::makerTags, unsignedShort, 1, printFiFocusDistance},
    {0x0019, "FlashExposureLock", N_("Flash Exposure Lock"), N_("Flash exposure lock"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)},
    {0x003D, "RFLensType", N_("RF Lens Type"), N_("RF Lens Type"), IfdId::canonFiId, SectionId::makerTags,
     unsignedShort, 1, EXV_PRINT_TAG(canonRFLensType)},
    // End of list marker
    {0xffff, "(UnknownCanonFiTag)", "(UnknownCanonFiTag)", N_("Unknown Canon File Info tag"), IfdId::canonFiId,
     SectionId::makerTags, signedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListFi() {
  return tagInfoFi_;
}

//! Tone Curve Values
constexpr TagDetails canonToneCurve[] = {
    {0, N_("Standard")},
    {1, N_("Manual")},
    {2, N_("Custom")},
};

//! Sharpness Frequency Values
constexpr TagDetails canonSharpnessFrequency[] = {
    {0, N_("n/a")}, {1, N_("Lowest")}, {2, N_("Low")}, {3, N_("Standard")}, {4, N_("High")}, {5, N_("Highest")},
};

// Canon Processing Info Tag
constexpr TagInfo CanonMakerNote::tagInfoPr_[] = {
    {0x0001, "ToneCurve", N_("ToneCurve"), N_("Tone curve"), IfdId::canonPrId, SectionId::makerTags, signedShort, 1,
     EXV_PRINT_TAG(canonToneCurve)},
    {0x0002, "Sharpness", N_("Sharpness"), N_("Sharpness"), IfdId::canonPrId, SectionId::makerTags, signedShort, 1,
     printValue},
    {0x0003, "SharpnessFrequency", N_("SharpnessFrequency"), N_("Sharpness frequency"), IfdId::canonPrId,
     SectionId::makerTags, signedShort, 1, EXV_PRINT_TAG(canonSharpnessFrequency)},
    {0x0004, "SensorRedLevel", N_("SensorRedLevel"), N_("Sensor red level"), IfdId::canonPrId, SectionId::makerTags,
     signedShort, 1, printValue},
    {0x0005, "SensorBlueLevel", N_("SensorBlueLevel"), N_("Sensor blue level"), IfdId::canonPrId, SectionId::makerTags,
     signedShort, 1, printValue},
    {0x0006, "WhiteBalanceRed", N_("WhiteBalanceRed"), N_("White balance red"), IfdId::canonPrId, SectionId::makerTags,
     signedShort, 1, printValue},
    {0x0007, "WhiteBalanceBlue", N_("WhiteBalanceBlue"), N_("White balance blue"), IfdId::canonPrId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x0008, "WhiteBalance", N_("WhiteBalance"), N_("White balance"), IfdId::canonPrId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)},
    {0x0009, "ColorTemperature", N_("ColorTemperature"), N_("Color Temperature"), IfdId::canonPrId,
     SectionId::makerTags, signedShort, 1, printValue},
    {0x000a, "PictureStyle", N_("PictureStyle"), N_("Picture style"), IfdId::canonPrId, SectionId::makerTags,
     signedShort, 1, EXV_PRINT_TAG(canonPictureStyle)},
    {0x000b, "DigitalGain", N_("DigitalGain"), N_("Digital gain"), IfdId::canonPrId, SectionId::makerTags, signedShort,
     1, printValue},
    {0x000c, "WBShiftAB", N_("WBShiftAB"), N_("WBShift AB"), IfdId::canonPrId, SectionId::makerTags, signedShort, 1,
     printValue},
    {0x000d, "WBShiftGM", N_("WBShiftGM"), N_("WB Shift GM"), IfdId::canonPrId, SectionId::makerTags, signedShort, 1,
     printValue},
    {0xffff, "(UnknownCanonPrTag)", "(UnknownCanonPrTag)", N_("Unknown Canon Processing Info tag"), IfdId::canonPrId,
     SectionId::makerTags, signedShort, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListPr() {
  return tagInfoPr_;
}

//! canonTimeZoneCity - array of cityID/cityName used by Canon
constexpr TagDetails canonTimeZoneCity[] = {
    {0x0000, N_("n/a")},
    {0x0001, N_("Chatham Islands")},
    {0x0002, N_("Wellington")},
    {0x0003, N_("Solomon Islands")},
    {0x0004, N_("Sydney")},
    {0x0005, N_("Adelaide")},
    {0x0006, N_("Tokyo")},
    {0x0007, N_("Hong Kong")},
    {0x0008, N_("Bangkok")},
    {0x0009, N_("Yangon")},
    {0x000a, N_("Dhaka")},
    {0x000b, N_("Kathmandu")},
    {0x000c, N_("Delhi")},
    {0x000d, N_("Karachi")},
    {0x000e, N_("Kabul")},
    {0x000f, N_("Dubai")},
    {0x0010, N_("Tehran")},
    {0x0011, N_("Moscow")},
    {0x0012, N_("Cairo")},
    {0x0013, N_("Paris")},
    {0x0014, N_("London")},
    {0x0015, N_("Azores")},
    {0x0016, N_("Fernando de Noronha")},
    {0x0017, N_("Sao Paulo")},
    {0x0018, N_("Newfoundland")},
    {0x0019, N_("Santiago")},
    {0x001a, N_("Caracas")},
    {0x001b, N_("New York")},
    {0x001c, N_("Chicago")},
    {0x001d, N_("Denver")},
    {0x001e, N_("Los Angeles")},
    {0x001f, N_("Anchorage")},
    {0x0020, N_("Honolulu")},
    {0x0021, N_("Samoa")},
    {0x7ffe, N_("(not set)")},
};

// Canon Time Info Tag
constexpr TagInfo CanonMakerNote::tagInfoTi_[] = {
    {0x0001, "TimeZone", N_("Time zone offset"), N_("Time zone offset in minutes"), IfdId::canonTiId,
     SectionId::makerTags, signedLong, 1, printValue},
    {0x0002, "TimeZoneCity", N_("Time zone city"), N_("Time zone city"), IfdId::canonTiId, SectionId::makerTags,
     signedLong, 1, EXV_PRINT_TAG(canonTimeZoneCity)},
    {0x0003, "DaylightSavings", N_("Daylight Savings"), N_("Daylight Saving Time"), IfdId::canonTiId,
     SectionId::makerTags, signedLong, 1, printValue},
    {0xffff, "(UnknownCanonTiTag)", "(UnknownCanonTiTag)", N_("Unknown Canon Time Info tag"), IfdId::canonTiId,
     SectionId::makerTags, signedLong, 1, printValue},
};

const TagInfo* CanonMakerNote::tagListTi() {
  return tagInfoTi_;
}

std::ostream& CanonMakerNote::printFiFileNumber(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (!metadata || value.typeId() != unsignedLong || value.count() == 0) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  auto pos = metadata->findKey(ExifKey("Exif.Image.Model"));
  if (pos == metadata->end())
    return os << "(" << value << ")";

  // Ported from Exiftool
  std::string model = pos->toString();
  if (Internal::contains(model, "20D") || Internal::contains(model, "350D") ||
      model.substr(model.size() - 8, 8) == "REBEL XT" || Internal::contains(model, "Kiss Digital N")) {
    uint32_t val = value.toUint32();
    uint32_t dn = (val & 0xffc0) >> 6;
    uint32_t fn = ((val >> 16) & 0xff) + ((val & 0x3f) << 8);
    os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
    os.flags(f);
    return os;
  }
  if (Internal::contains(model, "30D") || Internal::contains(model, "400D") || Internal::contains(model, "REBEL XTi") ||
      Internal::contains(model, "Kiss Digital X") || Internal::contains(model, "K236")) {
    uint32_t val = value.toUint32();
    uint32_t dn = (val & 0xffc00) >> 10;
    while (dn < 100)
      dn += 0x40;
    uint32_t fn = ((val & 0x3ff) << 4) + ((val >> 20) & 0x0f);
    os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
    os.flags(f);
    return os;
  }

  os.flags(f);
  return os << "(" << value << ")";
}

std::ostream& CanonMakerNote::printFocalLength(std::ostream& os, const Value& value, const ExifData* metadata) {
  std::ios::fmtflags f(os.flags());
  if (!metadata || value.count() < 4 || value.typeId() != unsignedShort) {
    os.flags(f);
    return os << value;
  }

  ExifKey key("Exif.CanonCs.Lens");
  auto pos = metadata->findKey(key);
  if (pos != metadata->end() && pos->value().count() >= 3 && pos->value().typeId() == unsignedShort) {
    float fu = pos->value().toFloat(2);
    if (fu != 0.0F) {
      float fl = value.toFloat(1) / fu;
      std::ostringstream oss;
      oss.copyfmt(os);
      os << std::fixed << std::setprecision(1);
      os << fl << " mm";
      os.copyfmt(oss);
      os.flags(f);
      return os;
    }
  }

  os.flags(f);
  return os << value;
}

std::ostream& CanonMakerNote::print0x0008(std::ostream& os, const Value& value, const ExifData*) {
  std::string n = value.toString();
  if (n.length() < 4)
    return os << "(" << n << ")";
  return os << n.substr(0, n.length() - 4) << "-" << n.substr(n.length() - 4);
}

std::ostream& CanonMakerNote::print0x000a(std::ostream& os, const Value& value, const ExifData*) {
  std::istringstream is(value.toString());
  uint32_t l = 0;
  is >> l;
  return os << std::setw(4) << std::setfill('0') << std::hex << ((l & 0xffff0000) >> 16) << std::setw(5)
            << std::setfill('0') << std::dec << (l & 0x0000ffff);
}

std::ostream& CanonMakerNote::print0x000c(std::ostream& os, const Value& value, const ExifData* exifData) {
  std::istringstream is(value.toString());

  if (!exifData) {
    return os << value;
  }

  ExifKey key("Exif.Canon.ModelID");
  auto pos = exifData->findKey(key);
  // if model is EOS D30
  if (pos != exifData->end() && pos->value().count() == 1 && pos->value().toInt64() == 0x01140000) {
    uint32_t l = 0;
    is >> l;
    return os << std::setw(4) << std::setfill('0') << std::hex << ((l & 0xffff0000) >> 16) << std::setw(5)
              << std::setfill('0') << std::dec << (l & 0x0000ffff);
  }
  return os << value;
}

std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  if (auto l = value.toInt64(); l == 0) {
    os << "Off";
  } else {
    os << l / 10.0 << " s";
  }
  return os;
}

std::ostream& printCsLensFFFF(std::ostream& os, const Value& value, const ExifData* metadata) {
  try {
    // 1140
    const auto itModel = metadata->findKey(ExifKey("Exif.Image.Model"));
    const auto itLens = metadata->findKey(ExifKey("Exif.CanonCs.Lens"));
    const auto itApert = metadata->findKey(ExifKey("Exif.CanonCs.MaxAperture"));

    if (itModel != metadata->end() && itModel->value().toString() == "Canon EOS 30D" && itLens != metadata->end() &&
        itLens->value().toString() == "24 24 1" && itApert != metadata->end() &&
        itApert->value().toString() == "95"  // F2.8
    ) {
      return os << "Canon EF-S 24mm f/2.8 STM";
    }
  } catch (const std::exception&) {
  }

  return EXV_PRINT_TAG(canonCsLensType)(os, value, metadata);
}

/**
 * @brief convert string to float w/o considering locale
 *
 * Using std:stof to convert strings to float takes into account the locale
 * and thus leads to wrong results when converting e.g. "5.6" with a DE locale
 * which expects "," as decimal instead of ".". See GitHub issue #2746
 *
 * Use std::from_chars once that's properly supported by compilers.
 *
 * @param str string to convert
 * @return float value of string
 */
float string_to_float(std::string const& str) {
  float val{};
  std::stringstream ss;
  std::locale c_locale("C");
  ss.imbue(c_locale);
  ss << str;
  ss >> val;

  if (ss.fail()) {
    throw Error(ErrorCode::kerErrorMessage, "canonmn_int.cpp:string_to_float failed for: ", str);
  }

  return val;
}

std::ostream& printCsLensTypeByMetadata(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (!metadata || value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  auto const lensType = value.toInt64();

  if (lensType == 0xffff) {
    return printCsLensFFFF(os, value, metadata);
  }

  // get the values we need from the metadata container
  ExifKey lensKey("Exif.CanonCs.Lens");
  auto pos = metadata->findKey(lensKey);
  // catch possible error cases
  if (pos == metadata->end() || pos->value().count() < 3 || pos->value().typeId() != unsignedShort ||
      pos->value().toFloat(2) == 0.0F) {
    os << "Unknown Lens (" << lensType << ")";
    return os;
  }

  auto const exifFlMin = static_cast<int>(static_cast<float>(pos->value().toInt64(1)) / pos->value().toFloat(2));
  auto const exifFlMax = static_cast<int>(static_cast<float>(pos->value().toInt64(0)) / pos->value().toFloat(2));

  ExifKey aperKey("Exif.CanonCs.MaxAperture");
  pos = metadata->findKey(aperKey);
  if (pos == metadata->end() || pos->value().count() != 1 || pos->value().typeId() != unsignedShort) {
    os << "Unknown Lens (" << lensType << ")";
    return os;
  }

  auto exifAperMax = fnumber(canonEv(static_cast<int16_t>(pos->value().toInt64(0))));

  // regex to extract short and tele focal length, max aperture at short and tele position
  // and the teleconverter factor from the lens label
  std::regex const lens_regex(
      // anything at the start
      ".*?"
      // maybe min focal length and hyphen, surely max focal length e.g.: 24-70mm
      "(?:([0-9]+)-)?([0-9]+)mm"
      // anything in-between
      ".*?"
      // maybe short focal length max aperture and hyphen, surely at least single max aperture e.g.: f/4.5-5.6
      // short and tele indicate apertures at the short (focal_length_min) and tele (focal_length_max)
      // position of the lens
      "(?:(?:f\\/)|T|F)(?:([0-9]+(?:\\.[0-9]+)?)-)?([0-9]+(?:\\.[0-9])?)"
      // check if there is a teleconverter pattern e.g. + 1.4x
      "(?:.*?\\+.*?([0-9.]+)x)?");

  bool unmatched = true;
  // we loop over all our lenses to print out all matching lenses
  // if we have multiple possibilities, they are concatenated by "*OR*"
  for (auto&& [val, label] : canonCsLensType) {
    if (val != lensType) {
      continue;
    }

    std::cmatch base_match;
    if (!std::regex_search(label, base_match, lens_regex)) {
      // this should never happen, as it would indicate the lens is specified incorrectly
      // in the CanonCsLensType array
      throw Error(ErrorCode::kerErrorMessage, "Lens regex didn't match for: ", label);
    }

    auto tc = base_match[5].length() > 0 ? string_to_float(base_match[5].str()) : 1.f;

    auto flMax = static_cast<int>(string_to_float(base_match[2].str()) * tc);
    int flMin = base_match[1].length() > 0 ? static_cast<int>(string_to_float(base_match[1].str()) * tc) : flMax;

    auto aperMaxTele = string_to_float(base_match[4].str()) * tc;
    auto aperMaxShort = base_match[3].length() > 0 ? string_to_float(base_match[3].str()) * tc : aperMaxTele;

    if (flMin != exifFlMin || flMax != exifFlMax || exifAperMax < (aperMaxShort - .1 * tc) ||
        exifAperMax > (aperMaxTele + .1 * tc)) {
      continue;
    }

    if (unmatched) {
      unmatched = false;
      os << label;
      continue;
    }

    os << " *OR* " << label;
  }

  // if the entire for loop left us with unmatched==false
  // we weren't able to find a single matching lens :(
  if (unmatched) {
    os << "Unknown Lens (" << lensType << ")";
  }
  return os;
}

//! printCsLensType by searching the config file if necessary
std::ostream& CanonMakerNote::printCsLensType(std::ostream& os, const Value& value, const ExifData* metadata) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << "(" << value << ")";

  // #1034
  const std::string undefined("undefined");
  const std::string section("canon");
  if (Internal::readExiv2Config(section, value.toString(), undefined) != undefined) {
    return os << Internal::readExiv2Config(section, value.toString(), undefined);
  }

  // try our best to determine the lens based on metadata
  // sometimes the result will be a set of multiple choices
  return printCsLensTypeByMetadata(os, value, metadata);
}

std::ostream& CanonMakerNote::printCsLens(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());

  if (value.count() < 3 || value.typeId() != unsignedShort) {
    os << "(" << value << ")";
    os.flags(f);
    return os;
  }

  float fu = value.toFloat(2);
  if (fu == 0.0F)
    return os << value;
  float len1 = value.toInt64(0) / fu;
  float len2 = value.toInt64(1) / fu;
  std::ostringstream oss;
  oss.copyfmt(os);
  os << std::fixed << std::setprecision(1);
  if (len1 == len2) {
    os << len1 << " mm";
  } else {
    os << len2 << " - " << len1 << " mm";
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& CanonMakerNote::printLe0x0000(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedByte || value.size() != 5)
    return os << "(" << value << ")";
  std::ios::fmtflags f(os.flags());
  std::ostringstream oss;
  oss.copyfmt(os);
  for (size_t i = 0; i < value.size(); ++i) {
    os << std::setw(2) << std::setfill('0') << std::hex << value.toInt64(i);
  }
  os.copyfmt(oss);
  os.flags(f);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0001(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.typeId() == unsignedShort && value.count() > 0) {
    os << std::pow(2.0F, canonEv(value.toInt64()) / 32) * 100.0F;
  }
  os.flags(f);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.typeId() == unsignedShort && value.count() > 0) {
    // Ported from Exiftool by Will Stokes
    os << std::pow(2.0F, canonEv(value.toInt64())) * 100.0F / 32.0F;
  }
  os.flags(f);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0003(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() == unsignedShort && value.count() > 0) {
    // The offset of '5' seems to be ok for most Canons (see Exiftool)
    // It might be explained by the fact, that most Canons have a longest
    // exposure of 30s which is 5 EV below 1s
    // see also printSi0x0017
    std::ostringstream oss;
    oss.copyfmt(os);
    auto res = std::lround(100.0 * (static_cast<short>(value.toInt64()) / 32.0 + 5.0));
    os << std::fixed << std::setprecision(2) << res / 100.0;
    os.copyfmt(oss);
  }
  return os;
}

std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  const auto l = value.toInt64();
  os << l << "";
  // Todo: determine unit
  return os;
}

std::ostream& CanonMakerNote::printSi0x000c(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toInt64() == 0)
    return os << "--";

  return os << value.toInt64() - 128 << " °C";
}

std::ostream& CanonMakerNote::printSi0x000d(std::ostream& os, const Value& value, const ExifData*) {
  if (value.toInt64() == 65535)
    return os << "--";

  return os << value.toInt64() / 32;
}

std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os, const Value& value, const ExifData* pExifData) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  const auto l = value.toUint32();
  const auto num = (l & 0xf000U) >> 12;
  os << num << " focus points; ";
  if (auto used = l & 0x0fffU; used == 0) {
    os << "none";
  } else {
    EXV_PRINT_TAG_BITMASK(canonSiAFPointUsed)(os, value, pExifData);
  }
  os << " used";
  return os;
}

std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  if (auto l = value.toInt64(); l == 0xffff) {
    os << "Infinite";
  } else {
    os << value.toInt64() / 100.0 << " m";
  }
  os.flags(f);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0015(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  std::ostringstream oss;
  oss.copyfmt(os);
  const auto val = static_cast<int16_t>(value.toInt64());
  if (val < 0)
    return os << value;
  os << std::setprecision(2) << "F" << fnumber(canonEv(val));
  os.copyfmt(oss);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0016(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  auto [u, r] = exposureTime(canonEv(value.toInt64()));
  os << u;
  if (r > 1) {
    os << "/" << r;
  }
  os.flags(f);
  return os << " s";
}

std::ostream& CanonMakerNote::printSi0x0017(std::ostream& os, const Value& value, const ExifData*) {
  if (value.typeId() != unsignedShort || value.count() == 0)
    return os << value;

  std::ostringstream oss;
  oss.copyfmt(os);
  os << std::fixed << std::setprecision(2) << (value.toInt64() / 8.0) - 6.0;
  os.copyfmt(oss);
  return os;
}

std::ostream& CanonMakerNote::printSi0x0018(std::ostream& os, const Value& value, const ExifData*) {
  return os << value.toInt64() / 10;
}

std::ostream& CanonMakerNote::printFiFocusDistance(std::ostream& os, const Value& value, const ExifData*) {
  std::ios::fmtflags f(os.flags());
  if (value.typeId() != signedShort || value.count() == 0)
    return os << value;

  std::ostringstream oss;
  oss.copyfmt(os);
  os << std::fixed << std::setprecision(2);

  if (auto l = value.toInt64(); l == -1) {
    os << "Infinite";
  } else {
    os << value.toInt64() / 100.0 << " m";
  }

  os.copyfmt(oss);
  os.flags(f);
  return os;
}

// *****************************************************************************
// free functions

float canonEv(int64_t val) {
  // temporarily remove sign
  int sign = 1;
  if (val < 0) {
    sign = -1;
    val = -val;
  }
  // remove fraction
  const auto remainder = val & 0x1f;
  val -= remainder;
  auto frac = static_cast<float>(remainder);
  // convert 1/3 (0x0c) and 2/3 (0x14) codes
  if (frac == 0x0c) {
    frac = 32.0F / 3;
  } else if (frac == 0x14) {
    frac = 64.0F / 3;
  } else if ((val == 160) && (frac == 0x08)) {  // for Sigma f/6.3 lenses that report f/6.2 to camera
    frac = 30.0F / 3;
  }
  return sign * (val + frac) / 32.0F;
}

}  // namespace Exiv2::Internal
