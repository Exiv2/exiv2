# -*- coding: utf-8 -*-

import system_tests
# test needs system_tests.vv.enable_bmff=1
vv=system_tests.verbose_version()
enable_bmff = 'enable_bmff'
bSkip = not (enable_bmff in vv and vv[enable_bmff] == '1')

class pr_1475_2021_heic(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/2021-02-13-1929.heic"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2  -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.Make                              Ascii       6  Apple
Exif.Image.DateTime                          Ascii      20  2021:02:13 11:03:31
Exif.Photo.ExposureTime                      Rational    1  1/30 s
Exif.Photo.ExposureProgram                   Short       1  Auto
Exif.Photo.DateTimeOriginal                  Ascii      20  2021:02:13 11:03:31
Exif.Photo.DateTimeDigitized                 Ascii      20  2021:02:13 11:03:31
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ExposureMode                      Short       1  Auto
Exif.GPSInfo.GPSDateStamp                    Ascii      11  2021:02:13
""","""Exiv2::BmffImage::boxHandler: ftyp        0->36 brand: heic
Exiv2::BmffImage::boxHandler: meta       36->3380 
  Exiv2::BmffImage::boxHandler: hdlr       48->34 
  Exiv2::BmffImage::boxHandler: dinf       82->36 
  Exiv2::BmffImage::boxHandler: pitm      118->14 
  Exiv2::BmffImage::boxHandler: iinf      132->1085 
    Exiv2::BmffImage::boxHandler: infe      146->21 ID =   1 hvc1 
    Exiv2::BmffImage::boxHandler: infe      167->21 ID =   2 hvc1 
    Exiv2::BmffImage::boxHandler: infe      188->21 ID =   3 hvc1 
    Exiv2::BmffImage::boxHandler: infe      209->21 ID =   4 hvc1 
    Exiv2::BmffImage::boxHandler: infe      230->21 ID =   5 hvc1 
    Exiv2::BmffImage::boxHandler: infe      251->21 ID =   6 hvc1 
    Exiv2::BmffImage::boxHandler: infe      272->21 ID =   7 hvc1 
    Exiv2::BmffImage::boxHandler: infe      293->21 ID =   8 hvc1 
    Exiv2::BmffImage::boxHandler: infe      314->21 ID =   9 hvc1 
    Exiv2::BmffImage::boxHandler: infe      335->21 ID =  10 hvc1 
    Exiv2::BmffImage::boxHandler: infe      356->21 ID =  11 hvc1 
    Exiv2::BmffImage::boxHandler: infe      377->21 ID =  12 hvc1 
    Exiv2::BmffImage::boxHandler: infe      398->21 ID =  13 hvc1 
    Exiv2::BmffImage::boxHandler: infe      419->21 ID =  14 hvc1 
    Exiv2::BmffImage::boxHandler: infe      440->21 ID =  15 hvc1 
    Exiv2::BmffImage::boxHandler: infe      461->21 ID =  16 hvc1 
    Exiv2::BmffImage::boxHandler: infe      482->21 ID =  17 hvc1 
    Exiv2::BmffImage::boxHandler: infe      503->21 ID =  18 hvc1 
    Exiv2::BmffImage::boxHandler: infe      524->21 ID =  19 hvc1 
    Exiv2::BmffImage::boxHandler: infe      545->21 ID =  20 hvc1 
    Exiv2::BmffImage::boxHandler: infe      566->21 ID =  21 hvc1 
    Exiv2::BmffImage::boxHandler: infe      587->21 ID =  22 hvc1 
    Exiv2::BmffImage::boxHandler: infe      608->21 ID =  23 hvc1 
    Exiv2::BmffImage::boxHandler: infe      629->21 ID =  24 hvc1 
    Exiv2::BmffImage::boxHandler: infe      650->21 ID =  25 hvc1 
    Exiv2::BmffImage::boxHandler: infe      671->21 ID =  26 hvc1 
    Exiv2::BmffImage::boxHandler: infe      692->21 ID =  27 hvc1 
    Exiv2::BmffImage::boxHandler: infe      713->21 ID =  28 hvc1 
    Exiv2::BmffImage::boxHandler: infe      734->21 ID =  29 hvc1 
    Exiv2::BmffImage::boxHandler: infe      755->21 ID =  30 hvc1 
    Exiv2::BmffImage::boxHandler: infe      776->21 ID =  31 hvc1 
    Exiv2::BmffImage::boxHandler: infe      797->21 ID =  32 hvc1 
    Exiv2::BmffImage::boxHandler: infe      818->21 ID =  33 hvc1 
    Exiv2::BmffImage::boxHandler: infe      839->21 ID =  34 hvc1 
    Exiv2::BmffImage::boxHandler: infe      860->21 ID =  35 hvc1 
    Exiv2::BmffImage::boxHandler: infe      881->21 ID =  36 hvc1 
    Exiv2::BmffImage::boxHandler: infe      902->21 ID =  37 hvc1 
    Exiv2::BmffImage::boxHandler: infe      923->21 ID =  38 hvc1 
    Exiv2::BmffImage::boxHandler: infe      944->21 ID =  39 hvc1 
    Exiv2::BmffImage::boxHandler: infe      965->21 ID =  40 hvc1 
    Exiv2::BmffImage::boxHandler: infe      986->21 ID =  41 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1007->21 ID =  42 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1028->21 ID =  43 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1049->21 ID =  44 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1070->21 ID =  45 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1091->21 ID =  46 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1112->21 ID =  47 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1133->21 ID =  48 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1154->21 ID =  49 grid 
    Exiv2::BmffImage::boxHandler: infe     1175->21 ID =  50 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1196->21 ID =  51 Exif  *** Exif ***
  Exiv2::BmffImage::boxHandler: iref     1217->148 
  Exiv2::BmffImage::boxHandler: iprp     1365->1203 
    Exiv2::BmffImage::boxHandler: ipco     1373->876 
      Exiv2::BmffImage::boxHandler: colr     1381->560 
      Exiv2::BmffImage::boxHandler: hvcC     1941->112 
      Exiv2::BmffImage::boxHandler: ispe     2053->20 pixelWidth_, pixelHeight_ = 512, 512
      Exiv2::BmffImage::boxHandler: ispe     2073->20 pixelWidth_, pixelHeight_ = 4032, 3024
      Exiv2::BmffImage::boxHandler: irot     2093->9 
      Exiv2::BmffImage::boxHandler: pixi     2102->16 
      Exiv2::BmffImage::boxHandler: hvcC     2118->111 
      Exiv2::BmffImage::boxHandler: ispe     2229->20 pixelWidth_, pixelHeight_ = 320, 240
    Exiv2::BmffImage::boxHandler: ipma     2249->319 
  Exiv2::BmffImage::boxHandler: idat     2568->16 
  Exiv2::BmffImage::boxHandler: iloc     2584->832 
      2592 |       16 |   ID |    1 |  23265, 15453
      2608 |       16 |   ID |    2 |  38718, 21882
      2624 |       16 |   ID |    3 |  60600, 20196
      2640 |       16 |   ID |    4 |  80796, 22234
      2656 |       16 |   ID |    5 | 103030, 41489
      2672 |       16 |   ID |    6 | 144519, 40845
      2688 |       16 |   ID |    7 | 185364, 34948
      2704 |       16 |   ID |    8 | 220312, 17650
      2720 |       16 |   ID |    9 | 237962, 15024
      2736 |       16 |   ID |   10 | 252986, 44647
      2752 |       16 |   ID |   11 | 297633, 38092
      2768 |       16 |   ID |   12 | 335725, 43666
      2784 |       16 |   ID |   13 | 379391, 48523
      2800 |       16 |   ID |   14 | 427914, 46992
      2816 |       16 |   ID |   15 | 474906, 36063
      2832 |       16 |   ID |   16 | 510969, 20737
      2848 |       16 |   ID |   17 | 531706, 25756
      2864 |       16 |   ID |   18 | 557462, 58455
      2880 |       16 |   ID |   19 | 615917, 44896
      2896 |       16 |   ID |   20 | 660813, 39730
      2912 |       16 |   ID |   21 | 700543, 38596
      2928 |       16 |   ID |   22 | 739139, 31795
      2944 |       16 |   ID |   23 | 770934, 33214
      2960 |       16 |   ID |   24 | 804148, 18858
      2976 |       16 |   ID |   25 | 823006, 35122
      2992 |       16 |   ID |   26 | 858128, 41015
      3008 |       16 |   ID |   27 | 899143, 40362
      3024 |       16 |   ID |   28 | 939505, 38288
      3040 |       16 |   ID |   29 | 977793, 34497
      3056 |       16 |   ID |   30 | 1012290, 31806
      3072 |       16 |   ID |   31 | 1044096, 25360
      3088 |       16 |   ID |   32 | 1069456, 19672
      3104 |       16 |   ID |   33 | 1089128, 25400
      3120 |       16 |   ID |   34 | 1114528, 34515
      3136 |       16 |   ID |   35 | 1149043, 20863
      3152 |       16 |   ID |   36 | 1169906, 25975
      3168 |       16 |   ID |   37 | 1195881, 33532
      3184 |       16 |   ID |   38 | 1229413, 31296
      3200 |       16 |   ID |   39 | 1260709, 29869
      3216 |       16 |   ID |   40 | 1290578, 16479
      3232 |       16 |   ID |   41 | 1307057, 17323
      3248 |       16 |   ID |   42 | 1324380, 17331
      3264 |       16 |   ID |   43 | 1341711, 13357
      3280 |       16 |   ID |   44 | 1355068, 22206
      3296 |       16 |   ID |   45 | 1377274, 31859
      3312 |       16 |   ID |   46 | 1409133, 26694
      3328 |       16 |   ID |   47 | 1435827, 23620
      3344 |       16 |   ID |   48 | 1459447, 15007
      3360 |       16 |   ID |   49 |      0,     8
      3376 |       16 |   ID |   50 |   3432, 17469
      3392 |       16 |   ID |   51 |  20901,  2364
Exiv2::BMFF Exif: ID = 51 from,length = 20901,2364
Exiv2::BmffImage::boxHandler: mdat     3416->1  (1474454)
Exiv2::BmffImage::boxHandler: __%   1474446->2641356760 
""",""]

class pr_1475_IMG_3578_heic(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/IMG_3578.heic"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2  -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.Make                              Ascii       6  Apple
Exif.Image.DateTime                          Ascii      20  2020:04:17 10:03:40
Exif.Photo.ExposureTime                      Rational    1  1/60 s
Exif.Photo.ExposureProgram                   Short       1  Auto
Exif.Photo.DateTimeOriginal                  Ascii      20  2020:04:17 10:03:40
Exif.Photo.DateTimeDigitized                 Ascii      20  2020:04:17 10:03:40
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ExposureMode                      Short       1  Auto
""","""Exiv2::BmffImage::boxHandler: ftyp        0->32 brand: heic
Exiv2::BmffImage::boxHandler: meta       32->3380 
  Exiv2::BmffImage::boxHandler: hdlr       44->34 
  Exiv2::BmffImage::boxHandler: dinf       78->36 
  Exiv2::BmffImage::boxHandler: pitm      114->14 
  Exiv2::BmffImage::boxHandler: iinf      128->1085 
    Exiv2::BmffImage::boxHandler: infe      142->21 ID =   1 hvc1 
    Exiv2::BmffImage::boxHandler: infe      163->21 ID =   2 hvc1 
    Exiv2::BmffImage::boxHandler: infe      184->21 ID =   3 hvc1 
    Exiv2::BmffImage::boxHandler: infe      205->21 ID =   4 hvc1 
    Exiv2::BmffImage::boxHandler: infe      226->21 ID =   5 hvc1 
    Exiv2::BmffImage::boxHandler: infe      247->21 ID =   6 hvc1 
    Exiv2::BmffImage::boxHandler: infe      268->21 ID =   7 hvc1 
    Exiv2::BmffImage::boxHandler: infe      289->21 ID =   8 hvc1 
    Exiv2::BmffImage::boxHandler: infe      310->21 ID =   9 hvc1 
    Exiv2::BmffImage::boxHandler: infe      331->21 ID =  10 hvc1 
    Exiv2::BmffImage::boxHandler: infe      352->21 ID =  11 hvc1 
    Exiv2::BmffImage::boxHandler: infe      373->21 ID =  12 hvc1 
    Exiv2::BmffImage::boxHandler: infe      394->21 ID =  13 hvc1 
    Exiv2::BmffImage::boxHandler: infe      415->21 ID =  14 hvc1 
    Exiv2::BmffImage::boxHandler: infe      436->21 ID =  15 hvc1 
    Exiv2::BmffImage::boxHandler: infe      457->21 ID =  16 hvc1 
    Exiv2::BmffImage::boxHandler: infe      478->21 ID =  17 hvc1 
    Exiv2::BmffImage::boxHandler: infe      499->21 ID =  18 hvc1 
    Exiv2::BmffImage::boxHandler: infe      520->21 ID =  19 hvc1 
    Exiv2::BmffImage::boxHandler: infe      541->21 ID =  20 hvc1 
    Exiv2::BmffImage::boxHandler: infe      562->21 ID =  21 hvc1 
    Exiv2::BmffImage::boxHandler: infe      583->21 ID =  22 hvc1 
    Exiv2::BmffImage::boxHandler: infe      604->21 ID =  23 hvc1 
    Exiv2::BmffImage::boxHandler: infe      625->21 ID =  24 hvc1 
    Exiv2::BmffImage::boxHandler: infe      646->21 ID =  25 hvc1 
    Exiv2::BmffImage::boxHandler: infe      667->21 ID =  26 hvc1 
    Exiv2::BmffImage::boxHandler: infe      688->21 ID =  27 hvc1 
    Exiv2::BmffImage::boxHandler: infe      709->21 ID =  28 hvc1 
    Exiv2::BmffImage::boxHandler: infe      730->21 ID =  29 hvc1 
    Exiv2::BmffImage::boxHandler: infe      751->21 ID =  30 hvc1 
    Exiv2::BmffImage::boxHandler: infe      772->21 ID =  31 hvc1 
    Exiv2::BmffImage::boxHandler: infe      793->21 ID =  32 hvc1 
    Exiv2::BmffImage::boxHandler: infe      814->21 ID =  33 hvc1 
    Exiv2::BmffImage::boxHandler: infe      835->21 ID =  34 hvc1 
    Exiv2::BmffImage::boxHandler: infe      856->21 ID =  35 hvc1 
    Exiv2::BmffImage::boxHandler: infe      877->21 ID =  36 hvc1 
    Exiv2::BmffImage::boxHandler: infe      898->21 ID =  37 hvc1 
    Exiv2::BmffImage::boxHandler: infe      919->21 ID =  38 hvc1 
    Exiv2::BmffImage::boxHandler: infe      940->21 ID =  39 hvc1 
    Exiv2::BmffImage::boxHandler: infe      961->21 ID =  40 hvc1 
    Exiv2::BmffImage::boxHandler: infe      982->21 ID =  41 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1003->21 ID =  42 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1024->21 ID =  43 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1045->21 ID =  44 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1066->21 ID =  45 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1087->21 ID =  46 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1108->21 ID =  47 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1129->21 ID =  48 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1150->21 ID =  49 grid 
    Exiv2::BmffImage::boxHandler: infe     1171->21 ID =  50 hvc1 
    Exiv2::BmffImage::boxHandler: infe     1192->21 ID =  51 Exif  *** Exif ***
  Exiv2::BmffImage::boxHandler: iref     1213->148 
  Exiv2::BmffImage::boxHandler: iprp     1361->1203 
    Exiv2::BmffImage::boxHandler: ipco     1369->876 
      Exiv2::BmffImage::boxHandler: colr     1377->560 
      Exiv2::BmffImage::boxHandler: hvcC     1937->112 
      Exiv2::BmffImage::boxHandler: ispe     2049->20 pixelWidth_, pixelHeight_ = 512, 512
      Exiv2::BmffImage::boxHandler: ispe     2069->20 pixelWidth_, pixelHeight_ = 4032, 3024
      Exiv2::BmffImage::boxHandler: irot     2089->9 
      Exiv2::BmffImage::boxHandler: pixi     2098->16 
      Exiv2::BmffImage::boxHandler: hvcC     2114->111 
      Exiv2::BmffImage::boxHandler: ispe     2225->20 pixelWidth_, pixelHeight_ = 320, 240
    Exiv2::BmffImage::boxHandler: ipma     2245->319 
  Exiv2::BmffImage::boxHandler: idat     2564->16 
  Exiv2::BmffImage::boxHandler: iloc     2580->832 
      2588 |       16 |   ID |    1 |  10931, 11943
      2604 |       16 |   ID |    2 |  22874, 24817
      2620 |       16 |   ID |    3 |  47691, 20110
      2636 |       16 |   ID |    4 |  67801, 19449
      2652 |       16 |   ID |    5 |  87250, 19714
      2668 |       16 |   ID |    6 | 106964, 20223
      2684 |       16 |   ID |    7 | 127187, 18124
      2700 |       16 |   ID |    8 | 145311,  8745
      2716 |       16 |   ID |    9 | 154056, 21761
      2732 |       16 |   ID |   10 | 175817, 25378
      2748 |       16 |   ID |   11 | 201195, 16664
      2764 |       16 |   ID |   12 | 217859, 18638
      2780 |       16 |   ID |   13 | 236497, 22750
      2796 |       16 |   ID |   14 | 259247, 15801
      2812 |       16 |   ID |   15 | 275048, 15063
      2828 |       16 |   ID |   16 | 290111, 15671
      2844 |       16 |   ID |   17 | 305782, 19360
      2860 |       16 |   ID |   18 | 325142, 19415
      2876 |       16 |   ID |   19 | 344557, 29895
      2892 |       16 |   ID |   20 | 374452, 21193
      2908 |       16 |   ID |   21 | 395645, 46228
      2924 |       16 |   ID |   22 | 441873,  9356
      2940 |       16 |   ID |   23 | 451229, 14175
      2956 |       16 |   ID |   24 | 465404, 16734
      2972 |       16 |   ID |   25 | 482138, 32304
      2988 |       16 |   ID |   26 | 514442, 44251
      3004 |       16 |   ID |   27 | 558693, 21635
      3020 |       16 |   ID |   28 | 580328, 27907
      3036 |       16 |   ID |   29 | 608235,  8462
      3052 |       16 |   ID |   30 | 616697, 17997
      3068 |       16 |   ID |   31 | 634694, 22077
      3084 |       16 |   ID |   32 | 656771, 17266
      3100 |       16 |   ID |   33 | 674037, 19623
      3116 |       16 |   ID |   34 | 693660, 22028
      3132 |       16 |   ID |   35 | 715688, 22078
      3148 |       16 |   ID |   36 | 737766, 21911
      3164 |       16 |   ID |   37 | 759677, 20937
      3180 |       16 |   ID |   38 | 780614, 35413
      3196 |       16 |   ID |   39 | 816027, 53869
      3212 |       16 |   ID |   40 | 869896, 33923
      3228 |       16 |   ID |   41 | 903819, 18130
      3244 |       16 |   ID |   42 | 921949, 16268
      3260 |       16 |   ID |   43 | 938217, 23620
      3276 |       16 |   ID |   44 | 961837, 31499
      3292 |       16 |   ID |   45 | 993336, 33513
      3308 |       16 |   ID |   46 | 1026849, 37642
      3324 |       16 |   ID |   47 | 1064491, 37734
      3340 |       16 |   ID |   48 | 1102225, 20085
      3356 |       16 |   ID |   49 |      0,     8
      3372 |       16 |   ID |   50 |   3428,  5479
      3388 |       16 |   ID |   51 |   8907,  2024
Exiv2::BMFF Exif: ID = 51 from,length = 8907,2024
Exiv2::BmffImage::boxHandler: mdat     3412->1  (1122310)
Exiv2::BmffImage::boxHandler: ....  1122302->4115011359 
""",""]

class pr_1475_Stonehenge_heic(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/Stonehenge.heic"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 -g Image.Make -g Date -g Xm -g Expo -g Flash $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["""Exif.Image.Make                              Ascii      18  NIKON CORPORATION
Exif.Image.DateTime                          Ascii      20  2015:07:16 20:25:28
Exif.Photo.ExposureTime                      Rational    1  1/400 s
Exif.Photo.ExposureProgram                   Short       1  Not defined
Exif.Photo.DateTimeOriginal                  Ascii      20  2015:07:16 15:38:54
Exif.Photo.DateTimeDigitized                 Ascii      20  2015:07:16 15:38:54
Exif.Photo.ExposureBiasValue                 SRational   1  0 EV
Exif.Photo.Flash                             Short       1  No, compulsory
Exif.Photo.FlashpixVersion                   Undefined   4  1.00
Exif.Photo.ExposureMode                      Short       1  Auto
Xmp.aux.SerialNumber                         XmpText     7  2567806
Xmp.aux.FlashCompensation                    XmpText     3  0/1
Xmp.aux.LensInfo                             XmpText    20  18/1 250/1 7/2 63/10
Xmp.aux.Lens                                 XmpText    23  18.0-250.0 mm f/3.5-6.3
Xmp.aux.ImageNumber                          XmpText     4  9608
Xmp.aux.LensID                               XmpText    20  -7910804157571773682
Xmp.xmp.Rating                               XmpText     1  0
Xmp.xmp.CreatorTool                          XmpText     9  Ver.1.00 
Xmp.xmp.CreateDate                           XmpText    22  2015-07-16T15:38:54.00
Xmp.xmp.ModifyDate                           XmpText    28  2015-07-16T20:25:28.00+01:00
Xmp.cm2e.Father                              XmpText    11  Robin Mills
Xmp.cm2e.Family                              XmpBag      0  
Xmp.photoshop.DateCreated                    XmpText    22  2015-07-16T15:38:54.00
Xmp.dc.description                           LangAlt     1  lang="x-default" Classic View
Xmp.dc.Family                                XmpBag      1  Robin
""","""Exiv2::BmffImage::boxHandler: ftyp        0->24 brand: heic
Exiv2::BmffImage::boxHandler: meta       24->508 
  Exiv2::BmffImage::boxHandler: hdlr       36->34 
  Exiv2::BmffImage::boxHandler: dinf       70->36 
  Exiv2::BmffImage::boxHandler: pitm      106->14 
  Exiv2::BmffImage::boxHandler: iinf      120->97 
    Exiv2::BmffImage::boxHandler: infe      134->21 ID =   1 hvc1 
    Exiv2::BmffImage::boxHandler: infe      155->21 ID =   2 Exif  *** Exif ***
    Exiv2::BmffImage::boxHandler: infe      176->41 ID =   3 mime  *** XMP ***
  Exiv2::BmffImage::boxHandler: iref      217->40 
  Exiv2::BmffImage::boxHandler: iprp      257->217 
    Exiv2::BmffImage::boxHandler: ipco      265->185 
      Exiv2::BmffImage::boxHandler: colr      273->19 
      Exiv2::BmffImage::boxHandler: hvcC      292->113 
      Exiv2::BmffImage::boxHandler: ispe      405->20 pixelWidth_, pixelHeight_ = 300, 200
      Exiv2::BmffImage::boxHandler: irot      425->9 
      Exiv2::BmffImage::boxHandler: pixi      434->16 
    Exiv2::BmffImage::boxHandler: ipma      450->24 
  Exiv2::BmffImage::boxHandler: iloc      474->58 
       482 |       14 |   ID |    1 |   4780, 13461
       496 |       14 |   ID |    2 |    548,  1082
       510 |       14 |   ID |    3 |   1630,  3150
Exiv2::BMFF Exif: ID = 2 from,length = 548,1082
Exiv2::BMFF XMP: ID = 3 from,length = 1630,3150
Exiv2::BmffImage::boxHandler: mdat      532->1  (18241)
Exiv2::BmffImage::boxHandler: Q.&.    18233->2350779715 
""","""<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:aux="http://ns.adobe.com/exif/1.0/aux/"
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
    xmlns:cm2e="http://clanmills.com/exiv2/"
    xmlns:photoshop="http://ns.adobe.com/photoshop/1.0/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
   aux:SerialNumber="2567806"
   aux:FlashCompensation="0/1"
   aux:LensInfo="18/1 250/1 7/2 63/10"
   aux:Lens="18.0-250.0 mm f/3.5-6.3"
   aux:ImageNumber="9608"
   aux:LensID="-7910804157571773682"
   xmp:Rating="0"
   xmp:CreatorTool="Ver.1.00 "
   xmp:CreateDate="2015-07-16T15:38:54.00"
   xmp:ModifyDate="2015-07-16T20:25:28.00+01:00"
   cm2e:Father="Robin Mills"
   photoshop:DateCreated="2015-07-16T15:38:54.00">
   <cm2e:Family>
    <rdf:Bag/>
   </cm2e:Family>
   <dc:description>
    <rdf:Alt>
     <rdf:li xml:lang="x-default">Classic View</rdf:li>
    </rdf:Alt>
   </dc:description>
   <dc:Family>
    <rdf:Bag>
     <rdf:li>Robin</rdf:li>
    </rdf:Bag>
   </dc:Family>
  </rdf:Description>
 </rdf:RDF>
</x:xmpmeta>
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                                                                                                    
                           
<?xpacket end="w"?>"""]

class pr_1475_heic_heic(metaclass=system_tests.CaseMeta):
    url = "https://github.com/Exiv2/exiv2/pull/1475"
    filename = "$data_path/heic.heic"
    if bSkip:
        commands=[]
        retval=[]
        stdin=[]
        stderr=[]
        stdout=[]
        print("*** test skipped.  requires enable_bmff=1***")
    else:
        commands = ["$exiv2 -pa $filename"
                   ,"$exiv2 -pS $filename"
                   ,"$exiv2 -pX $filename"
                   ]
        retval = [ 0  ] * len(commands)
        stderr = [ "" ] * len(commands)
        stdin  = [ "" ] * len(commands)
        stdout = ["","""Exiv2::BmffImage::boxHandler: ftyp        0->28 brand: mif1
Exiv2::BmffImage::boxHandler: meta       28->921 
  Exiv2::BmffImage::boxHandler: hdlr       40->33 
  Exiv2::BmffImage::boxHandler: pitm       73->14 
  Exiv2::BmffImage::boxHandler: iloc       87->88 
        95 |       18 |   ID | 20004 |    957,333704
       113 |       18 |   ID | 20005 | 334669, 24523
       131 |       18 |   ID | 20006 | 359200,330132
       149 |       18 |   ID | 20007 | 689340, 28758
  Exiv2::BmffImage::boxHandler: iinf      175->140 
  Exiv2::BmffImage::boxHandler: iref      315->40 
  Exiv2::BmffImage::boxHandler: iprp      355->594 
    Exiv2::BmffImage::boxHandler: ipco      363->550 
      Exiv2::BmffImage::boxHandler: hvcC      371->126 
      Exiv2::BmffImage::boxHandler: ispe      497->20 pixelWidth_, pixelHeight_ = 1280, 854
      Exiv2::BmffImage::boxHandler: hvcC      517->125 
      Exiv2::BmffImage::boxHandler: ispe      642->20 pixelWidth_, pixelHeight_ = 320, 212
      Exiv2::BmffImage::boxHandler: hvcC      662->126 
      Exiv2::BmffImage::boxHandler: hvcC      788->125 
    Exiv2::BmffImage::boxHandler: ipma      913->36 
Exiv2::BmffImage::boxHandler: mdat      949->333712 
Exiv2::BmffImage::boxHandler: mdat   334661->24531 
Exiv2::BmffImage::boxHandler: mdat   359192->330140 
Exiv2::BmffImage::boxHandler: mdat   689332->28766 
Exiv2::BmffImage::boxHandler: mdat   718098->16 
""",""]

