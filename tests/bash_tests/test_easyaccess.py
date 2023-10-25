# -*- coding: utf-8 -*-

from system_tests import CaseMeta, CopyTmpFiles, path

###########################################################
# this test cases forces to loop through all keys 
# and would identify not existing keys
###########################################################
class Empty(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-empty.jpg")

    commands = [
        "$easyaccess_test $filename"
    ]
    stdout = [
	"""Orientation           (                                   ) : 
ISO speed             (                                   ) : 
Date & time original  (                                   ) : 
Flash bias            (                                   ) : 
Exposure mode         (                                   ) : 
Scene mode            (                                   ) : 
Macro mode            (                                   ) : 
Image quality         (                                   ) : 
White balance         (                                   ) : 
Lens name             (                                   ) : 
Saturation            (                                   ) : 
Sharpness             (                                   ) : 
Contrast              (                                   ) : 
Scene capture type    (                                   ) : 
Metering mode         (                                   ) : 
Camera make           (                                   ) : 
Camera model          (                                   ) : 
Exposure time         (                                   ) : 
FNumber               (                                   ) : 
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (                                   ) : 
Max aperture value    (                                   ) : 
Subject distance      (                                   ) : 
Light source          (                                   ) : 
Flash                 (                                   ) : 
Camera serial number  (                                   ) : 
Focal length          (                                   ) : 
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (                                   ) : 
"""
    ]
    stderr = [""] * 1
    retval = [0] * 1

###########################################################
# Canon EOS 300D
###########################################################
@CopyTmpFiles("$data_path/exiv2-canon-eos-300d.jpg")
class Canon1(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-canon-eos-300d.jpg")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.MeteringMode" $filename""",
        "$easyaccess_test $filename MeteringMode",
		"""$exiv2 -u -v -M"del Exif.Photo.ApertureValue" $filename""",
        "$easyaccess_test $filename Aperture",
		"""$exiv2 -u -v -M"del Exif.Photo.FocalLength" $filename""",
        "$easyaccess_test $filename FocalLength"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : right, top
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2003:09:26 08:19:51
Flash bias            (Exif.CanonSi.FlashBias             ) : 0 EV
Exposure mode         (Exif.CanonCs.ExposureProgram       ) : Program (P)
Scene mode            (Exif.CanonCs.EasyMode              ) : Manual
Macro mode            (Exif.CanonCs.Macro                 ) : (0)
Image quality         (Exif.CanonCs.Quality               ) : Fine
White balance         (Exif.CanonSi.WhiteBalance          ) : Auto
Lens name             (Exif.CanonCs.LensType              ) : n/a
Saturation            (Exif.CanonCs.Saturation            ) : High
Sharpness             (Exif.CanonCs.Sharpness             ) : High
Contrast              (Exif.CanonCs.Contrast              ) : High
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : Canon
Camera model          (Exif.Image.Model                   ) : Canon EOS 300D DIGITAL
Exposure time         (Exif.Photo.ExposureTime            ) : 1/80 s
FNumber               (Exif.Photo.FNumber                 ) : F5.6
Shutter speed value   (Exif.Photo.ShutterSpeedValue       ) : 1/80 s
Aperture value        (Exif.Photo.ApertureValue           ) : F5.6
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F3.5
Subject distance      (Exif.CanonSi.SubjectDistance       ) : Infinite
Light source          (                                   ) : 
Flash                 (Exif.Photo.Flash                   ) : No flash
Camera serial number  (Exif.Canon.SerialNumber            ) : 570004681
Focal length          (Exif.Photo.FocalLength             ) : 18.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (Exif.CanonPi.AFPointsUsed          ) : center
""",
    """File 1/1: $filename
Del Exif.Photo.MeteringMode
""",
	"""Metering mode         (Exif.CanonCs.MeteringMode          ) : Evaluative
""",
    """File 1/1: $filename
Del Exif.Photo.ApertureValue
""",
	"""Aperture value        (Exif.CanonSi.ApertureValue         ) : F5.4
""",
    """File 1/1: $filename
Del Exif.Photo.FocalLength
""",
	"""Focal length          (Exif.Canon.FocalLength             ) : 18.0 mm
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Canon Canon EOS 6D
###########################################################
@CopyTmpFiles("$data_path/20220610_MG_7238.exv")
class Canon2(metaclass=CaseMeta):

    filename = path("$tmp_path/20220610_MG_7238.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 800
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2022:06:10 13:03:43
Flash bias            (Exif.CanonSi.FlashBias             ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Aperture priority
Scene mode            (Exif.CanonCs.EasyMode              ) : Manual
Macro mode            (Exif.CanonCs.Macro                 ) : Off
Image quality         (Exif.CanonCs.Quality               ) : RAW
White balance         (Exif.CanonSi.WhiteBalance          ) : Auto
Lens name             (Exif.CanonCs.LensType              ) : Canon EF 35-80mm f/4-5.6
Saturation            (Exif.CanonCs.Saturation            ) : Normal
Sharpness             (Exif.CanonCs.Sharpness             ) : (32767)
Contrast              (Exif.CanonCs.Contrast              ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : Canon
Camera model          (Exif.Image.Model                   ) : Canon EOS 6D
Exposure time         (Exif.Photo.ExposureTime            ) : 1/100 s
FNumber               (Exif.Photo.FNumber                 ) : F5.6
Shutter speed value   (Exif.Photo.ShutterSpeedValue       ) : 1/99 s
Aperture value        (Exif.Photo.ApertureValue           ) : F5.7
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.CanonCs.MaxAperture           ) : F4
Subject distance      (Exif.CanonSi.SubjectDistance       ) : 0 m
Light source          (                                   ) : 
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (Exif.Photo.BodySerialNumber        ) : 113053000536
Focal length          (Exif.Photo.FocalLength             ) : 35.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.CanonSi.AFPointUsed           ) : 0 focus points; none used
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.CanonSi.ISOSpeed              ) : 800
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# FUJIFILM X-T3
###########################################################
@CopyTmpFiles("$data_path/FujiTagsDRangeAutoRating1.jpg")
class Canon2(metaclass=CaseMeta):

    filename = path("$tmp_path/FujiTagsDRangeAutoRating1.jpg")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.LensModel" $filename""",
        "$easyaccess_test $filename LensName"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 800
Date & time original  (                                   ) : 
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Manual
Scene mode            (Exif.Fujifilm.PictureMode          ) : Manual
Macro mode            (                                   ) : 
Image quality         (Exif.Fujifilm.Quality              ) : NORMAL 
White balance         (Exif.Fujifilm.WhiteBalance         ) : Auto
Lens name             (Exif.Photo.LensModel               ) : XF50mmF2 R WR
Saturation            (Exif.Fujifilm.Color                ) : 0 (normal)
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (                                   ) : 
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : FUJIFILM
Camera model          (Exif.Image.Model                   ) : X-T3
Exposure time         (Exif.Photo.ExposureTime            ) : 1/8000 s
FNumber               (Exif.Photo.FNumber                 ) : F2
Shutter speed value   (Exif.Photo.ShutterSpeedValue       ) : 1/8192 s
Aperture value        (Exif.Photo.ApertureValue           ) : F2
Brightness value      (Exif.Photo.BrightnessValue         ) : 2.53
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F2
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No flash
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 50.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (                                   ) : 
""",
    """File 1/1: $filename
Del Exif.Photo.LensModel
""",
	"""Lens name             (Exif.Photo.LensSpecification       ) : 50mm F2
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Nikon D1
###########################################################
class Nikon1(metaclass=CaseMeta):

    filename = path("$data_path/NikonD1.exv")

    commands = [
        "$easyaccess_test $filename"
    ]
    stdout = [
	"""Orientation           (                                   ) : 
ISO speed             (Exif.Nikon1.ISOSpeed               ) : 400
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2005:12:16 08:25:45
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Auto
Scene mode            (                                   ) : 
Macro mode            (                                   ) : 
Image quality         (Exif.Nikon1.Quality                ) : NORMAL 
White balance         (Exif.Nikon1.WhiteBalance           ) : AUTO        
Lens name             (                                   ) : 
Saturation            (                                   ) : 
Sharpness             (                                   ) : 
Contrast              (                                   ) : 
Scene capture type    (                                   ) : 
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : NIKON CORPORATION
Camera model          (Exif.Image.Model                   ) : NIKON D1 
Exposure time         (Exif.Photo.ExposureTime            ) : 1/125 s
FNumber               (Exif.Photo.FNumber                 ) : F5.3
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F4.4
Subject distance      (                                   ) : 
Light source          (                                   ) : 
Flash                 (                                   ) : 
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 78.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (Exif.Nikon1.AFFocusPos             ) : Dynamic area; Center
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Konica Minolta DYNAX 5D
###########################################################
@CopyTmpFiles("$data_path/KonicaMinoltaDYNAX5D.exv")
class KonicaMinolta1(metaclass=CaseMeta):

    filename = path("$tmp_path/KonicaMinoltaDYNAX5D.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Image.Orientation" $filename""",
        "$easyaccess_test $filename Orientation",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2005:10:17 14:47:57
Flash bias            (Exif.Minolta.FlashExposureComp     ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Aperture priority
Scene mode            (Exif.Minolta.SceneMode             ) : Standard
Macro mode            (                                   ) : 
Image quality         (Exif.Minolta.Quality               ) : Extra Fine
White balance         (Exif.MinoltaCs5D.WhiteBalance      ) : Auto
Lens name             (Exif.Minolta.LensID                ) : Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : KONICA MINOLTA 
Camera model          (Exif.Image.Model                   ) : DYNAX 5D
Exposure time         (Exif.Photo.ExposureTime            ) : 1/125 s
FNumber               (Exif.Photo.FNumber                 ) : F8
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 8
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F4.5
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 24.0 mm
Subject location/area (Exif.Photo.SubjectArea             ) : 1504 1000 256 304
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (                                   ) : 
""",
    """File 1/1: $filename
Del Exif.Image.Orientation
""",
	"""Orientation           (Exif.MinoltaCs5D.Rotation          ) : Horizontal (normal)
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.MinoltaCs5D.ISOSpeed          ) : 100
"""
    ]
    stderr = ["""Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory Minolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
"""
]
    retval = [0] * len(commands)

###########################################################
# Nikon E950
###########################################################
@CopyTmpFiles("$data_path/exiv2-nikon-e950.jpg")
class Nikon2(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-nikon-e950.jpg")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 80
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2001:04:06 11:51:40
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Auto
Scene mode            (                                   ) : 
Macro mode            (                                   ) : 
Image quality         (Exif.Nikon2.Quality                ) : (12)
White balance         (Exif.Nikon2.WhiteBalance           ) : Auto
Lens name             (                                   ) : 
Saturation            (                                   ) : 
Sharpness             (                                   ) : 
Contrast              (                                   ) : 
Scene capture type    (                                   ) : 
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : NIKON
Camera model          (Exif.Image.Model                   ) : E950
Exposure time         (Exif.Photo.ExposureTime            ) : 1/77 s
FNumber               (Exif.Photo.FNumber                 ) : F5.5
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F2.5
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No flash
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 12.8 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (                                   ) : 
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.Nikon2.ISOSpeed               ) : 80
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# NIKON Z 6: NikonLd4.LensID and NikonLd4.LensIDNumber
###########################################################
@CopyTmpFiles("$data_path/CH0_0174.exv")
class NikonLd4LensIDx(metaclass=CaseMeta):

    filename = path("$tmp_path/CH0_0174.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.LensModel" $filename""",
        "$easyaccess_test $filename LensName",
    	"""$exiv2 -u -v -M"set Exif.NikonLd4.LensID 13" $filename""",
        "$easyaccess_test $filename LensName",
    	"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed",
    	"""$exiv2 -u -v -M"del Exif.Photo.DateTimeOriginal" $filename""",
        "$easyaccess_test $filename DateTimeOriginal",
    	"""$exiv2 -u -v -M"del Exif.Photo.Saturation" $filename""",
        "$easyaccess_test $filename Saturation",
    	"""$exiv2 -u -v -M"del Exif.Photo.BodySerialNumber" $filename""",
        "$easyaccess_test $filename SerialNumber"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 12800
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2020:12:11 19:05:49
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Aperture priority
Scene mode            (Exif.Photo.SceneCaptureType        ) : Standard
Macro mode            (                                   ) : 
Image quality         (Exif.Nikon3.Quality                ) : RAW    
White balance         (Exif.Nikon3.WhiteBalance           ) : AUTO1       
Lens name             (Exif.Photo.LensModel               ) : 
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : NIKON CORPORATION
Camera model          (Exif.Image.Model                   ) : NIKON Z 6
Exposure time         (Exif.Photo.ExposureTime            ) : 0.769231 s
FNumber               (Exif.Photo.FNumber                 ) : F4
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.NikonLd4.MaxAperture          ) : n/a
Subject distance      (Exif.NikonLd4.FocusDistance        ) : 2.66 m
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No flash
Camera serial number  (Exif.Photo.BodySerialNumber        ) : 6078248
Focal length          (Exif.Photo.FocalLength             ) : 102.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (                                   ) : 
""",
    """File 1/1: $filename
Del Exif.Photo.LensModel
""",
	"""Lens name             (Exif.NikonLd4.LensIDNumber         ) : Nikon AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED
""",
    """File 1/1: $filename
Set Exif.NikonLd4.LensID "13" (Short)
""",
	"""Lens name             (Exif.NikonLd4.LensID               ) : Nikon Nikkor Z 24-70mm f/2.8 S
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.NikonIi.ISO                   ) : 12800
""",
    """File 1/1: $filename
Del Exif.Photo.DateTimeOriginal
""",
	"""Date & time original  (Exif.Image.DateTimeOriginal        ) : 2020:12:11 19:05:49
""",
    """File 1/1: $filename
Del Exif.Photo.Saturation
""",
	"""Saturation            (Exif.NikonPc.Saturation            ) : Normal
""",
    """File 1/1: $filename
Del Exif.Photo.BodySerialNumber
""",
	"""Camera serial number  (Exif.Nikon3.SerialNumber           ) : 6078248
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Pentax K100D
###########################################################
@CopyTmpFiles("$data_path/RAW_PENTAX_K100.exv")
class Pentax1(metaclass=CaseMeta):

    filename = path("$tmp_path/RAW_PENTAX_K100.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed",
		"""$exiv2 -u -v -M"del Exif.Pentax.WhiteBalance" $filename""",
        "$easyaccess_test $filename WhiteBalance",
		"""$exiv2 -u -v -M"del Exif.Photo.Saturation" $filename""",
        "$easyaccess_test $filename Saturation",
		"""$exiv2 -u -v -M"del Exif.Photo.Sharpness" $filename""",
        "$easyaccess_test $filename Sharpness",
		"""$exiv2 -u -v -M"del Exif.Photo.Contrast" $filename""",
        "$easyaccess_test $filename Contrast",
		"""$exiv2 -u -v -M"del Exif.Photo.MeteringMode" $filename""",
        "$easyaccess_test $filename MeteringMode",
		"""$exiv2 -u -v -M"del Exif.Image.Model" $filename""",
        "$easyaccess_test $filename Model",
		"""$exiv2 -u -v -M"del Exif.Photo.ExposureTime" $filename""",
        "$easyaccess_test $filename ExposureTime",
		"""$exiv2 -u -v -M"del Exif.Photo.FNumber" $filename""",
        "$easyaccess_test $filename FNumber",
		"""$exiv2 -u -v -M"del Exif.Photo.Flash" $filename""",
        "$easyaccess_test $filename Flash",
		"""$exiv2 -u -v -M"del Exif.Photo.FocalLength" $filename""",
        "$easyaccess_test $filename FocalLength"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 400
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2009:12:05 13:26:11
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Aperture priority
Scene mode            (Exif.Pentax.PictureMode            ) : Aperture Priority
Macro mode            (                                   ) : 
Image quality         (Exif.Pentax.Quality                ) : RAW
White balance         (Exif.Pentax.WhiteBalance           ) : Auto
Lens name             (Exif.Pentax.LensType               ) : Tamron AF 18-250mm F3.5-6.3 Di II LD Aspherical [IF] Macro
Saturation            (Exif.Photo.Saturation              ) : High
Sharpness             (Exif.Photo.Sharpness               ) : Hard
Contrast              (Exif.Photo.Contrast                ) : Hard
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : PENTAX Corporation 
Camera model          (Exif.Image.Model                   ) : PENTAX K100D       
Exposure time         (Exif.Photo.ExposureTime            ) : 1/750 s
FNumber               (Exif.Photo.FNumber                 ) : F3.5
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (                                   ) : 
Subject distance      (                                   ) : 
Light source          (                                   ) : 
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 18.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (Exif.Pentax.AFPoint                ) : Fixed Center
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.Pentax.ISO                    ) : 400
""",
    """File 1/1: $filename
Del Exif.Pentax.WhiteBalance
""",
	"""White balance         (Exif.Photo.WhiteBalance            ) : Auto
""",
    """File 1/1: $filename
Del Exif.Photo.Saturation
""",
	"""Saturation            (Exif.Pentax.Saturation             ) : Med High
""",
    """File 1/1: $filename
Del Exif.Photo.Sharpness
""",
	"""Sharpness             (Exif.Pentax.Sharpness              ) : Med Hard
""",
    """File 1/1: $filename
Del Exif.Photo.Contrast
""",
	"""Contrast              (Exif.Pentax.Contrast               ) : Med High
""",
    """File 1/1: $filename
Del Exif.Photo.MeteringMode
""",
	"""Metering mode         (Exif.Pentax.MeteringMode           ) : Multi Segment
""",
    """File 1/1: $filename
Del Exif.Image.Model
""",
	"""Camera model          (Exif.Pentax.ModelID                ) : K100D
""",
    """File 1/1: $filename
Del Exif.Photo.ExposureTime
""",
	"""Exposure time         (Exif.Pentax.ExposureTime           ) : 1.33 ms
""",
    """File 1/1: $filename
Del Exif.Photo.FNumber
""",
	"""FNumber               (Exif.Pentax.FNumber                ) : F3.5
""",
    """File 1/1: $filename
Del Exif.Photo.Flash
""",
	"""Flash                 (Exif.Pentax.Flash                  ) : Off, Did not fire
""",
    """File 1/1: $filename
Del Exif.Photo.FocalLength
""",
	"""Focal length          (Exif.Pentax.FocalLength            ) : 18.0 mm
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Pentax K-30
###########################################################
@CopyTmpFiles("$data_path/RAW_PENTAX_K30.exv")
class Pentax2(metaclass=CaseMeta):

    filename = path("$tmp_path/RAW_PENTAX_K30.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed",
		"""$exiv2 -u -v -M"del Exif.PentaxDng.WhiteBalance" $filename""",
        "$easyaccess_test $filename WhiteBalance",
		"""$exiv2 -u -v -M"del Exif.Photo.Saturation" $filename""",
        "$easyaccess_test $filename Saturation",
		"""$exiv2 -u -v -M"del Exif.Photo.Sharpness" $filename""",
        "$easyaccess_test $filename Sharpness",
		"""$exiv2 -u -v -M"del Exif.Photo.Contrast" $filename""",
        "$easyaccess_test $filename Contrast",
		"""$exiv2 -u -v -M"del Exif.Photo.MeteringMode" $filename""",
        "$easyaccess_test $filename MeteringMode",
		"""$exiv2 -u -v -M"del Exif.Image.Model" $filename""",
        "$easyaccess_test $filename Model",
		"""$exiv2 -u -v -M"del Exif.Photo.ExposureTime" $filename""",
        "$easyaccess_test $filename ExposureTime",
		"""$exiv2 -u -v -M"del Exif.Photo.FNumber" $filename""",
        "$easyaccess_test $filename FNumber",
		"""$exiv2 -u -v -M"del Exif.Photo.Flash" $filename""",
        "$easyaccess_test $filename Flash",
		"""$exiv2 -u -v -M"del Exif.Photo.FocalLength" $filename""",
        "$easyaccess_test $filename FocalLength"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2012:07:31 08:56:59
Flash bias            (                                   ) : 
Exposure mode         (Exif.Photo.ExposureProgram         ) : Landscape mode
Scene mode            (Exif.PentaxDng.PictureMode         ) : Unknown (0x010601)
Macro mode            (                                   ) : 
Image quality         (Exif.PentaxDng.Quality             ) : RAW
White balance         (Exif.PentaxDng.WhiteBalance        ) : Auto
Lens name             (Exif.PentaxDng.LensType            ) : smc PENTAX-DA 18-135mm F3.5-5.6 ED AL [IF] DC WR
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Hard
Contrast              (Exif.Photo.Contrast                ) : Hard
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Landscape
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : PENTAX             
Camera model          (Exif.Image.Model                   ) : PENTAX K-30        
Exposure time         (Exif.Photo.ExposureTime            ) : 1/60 s
FNumber               (Exif.Photo.FNumber                 ) : F11
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (                                   ) : 
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (                                   ) : 
Subject distance      (                                   ) : 
Light source          (                                   ) : 
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (Exif.PentaxDng.SerialNumber        ) : 4364069
Focal length          (Exif.Photo.FocalLength             ) : 31.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (Exif.Photo.SensingMethod           ) : One-chip color area
AF point              (Exif.PentaxDng.AFPoint             ) : Auto
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.PentaxDng.ISO                 ) : 100
""",
    """File 1/1: $filename
Del Exif.PentaxDng.WhiteBalance
""",
	"""White balance         (Exif.Photo.WhiteBalance            ) : Auto
""",
    """File 1/1: $filename
Del Exif.Photo.Saturation
""",
	"""Saturation            (Exif.PentaxDng.Saturation          ) : Normal
""",
    """File 1/1: $filename
Del Exif.Photo.Sharpness
""",
	"""Sharpness             (Exif.PentaxDng.Sharpness           ) : Med Hard
""",
    """File 1/1: $filename
Del Exif.Photo.Contrast
""",
	"""Contrast              (Exif.PentaxDng.Contrast            ) : Med High
""",
    """File 1/1: $filename
Del Exif.Photo.MeteringMode
""",
	"""Metering mode         (Exif.PentaxDng.MeteringMode        ) : Multi Segment
""",
    """File 1/1: $filename
Del Exif.Image.Model
""",
	"""Camera model          (Exif.PentaxDng.ModelID             ) : K-30
""",
    """File 1/1: $filename
Del Exif.Photo.ExposureTime
""",
	"""Exposure time         (Exif.PentaxDng.ExposureTime        ) : 16.66 ms
""",
    """File 1/1: $filename
Del Exif.Photo.FNumber
""",
	"""FNumber               (Exif.PentaxDng.FNumber             ) : F11
""",
    """File 1/1: $filename
Del Exif.Photo.Flash
""",
	"""Flash                 (Exif.PentaxDng.Flash               ) : Off, Did not fire
""",
    """File 1/1: $filename
Del Exif.Photo.FocalLength
""",
	"""Focal length          (Exif.PentaxDng.FocalLength         ) : 31.0 mm
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Sony DSC-HX60V
###########################################################
@CopyTmpFiles("$data_path/exiv2-SonyDSC-HX60V.exv")
class Sony1(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-SonyDSC-HX60V.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ISOSpeedRatings" $filename""",
        "$easyaccess_test $filename ISOspeed",
		"""$exiv2 -u -v -M"del Exif.Photo.ExposureProgram" $filename""",
        "$easyaccess_test $filename ExposureMode",
		"""$exiv2 -u -v -M"del Exif.Sony1.ExposureMode" $filename""",
        "$easyaccess_test $filename ExposureMode",
		"""$exiv2 -u -v -M"del Exif.Sony1.JPEGQuality" $filename""",
        "$easyaccess_test $filename ImageQuality",
		"""$exiv2 -u -v -M"del Exif.Sony1.Quality" $filename""",
        "$easyaccess_test $filename ImageQuality",
		"""$exiv2 -u -v -M"del Exif.Sony1.WhiteBalance" $filename""",
        "$easyaccess_test $filename WhiteBalance"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 250
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2014:01:01 00:52:22
Flash bias            (Exif.Sony1.FlashExposureComp       ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Auto
Scene mode            (Exif.Sony1.SceneMode               ) : Standard
Macro mode            (Exif.Sony1.Macro                   ) : Off
Image quality         (Exif.Sony1.JPEGQuality             ) : Fine
White balance         (Exif.Sony1.WhiteBalance            ) : Auto
Lens name             (Exif.Sony1.LensSpec                ) : 24-720mm F3.5-6.3
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : SONY
Camera model          (Exif.Image.Model                   ) : DSC-HX60V
Exposure time         (Exif.Photo.ExposureTime            ) : 1/250 s
FNumber               (Exif.Photo.FNumber                 ) : F6.3
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 7.81016
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F6.3
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 129.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.Sony1.AFPointSelected         ) : n/a
""",
    """File 1/1: $filename
Del Exif.Photo.ISOSpeedRatings
""",
	"""ISO speed             (Exif.Photo.RecommendedExposureIndex) : 250
""",
    """File 1/1: $filename
Del Exif.Photo.ExposureProgram
""",
	"""Exposure mode         (Exif.Sony1.ExposureMode            ) : Program AE
""",
    """File 1/1: $filename
Del Exif.Sony1.ExposureMode
""",
	"""Exposure mode         (Exif.SonyMisc2b.ExposureProgram    ) : Program AE
""",
    """File 1/1: $filename
Del Exif.Sony1.JPEGQuality
""",
	"""Image quality         (Exif.Sony1.Quality                 ) : Fine
""",
    """File 1/1: $filename
Del Exif.Sony1.Quality
""",
	"""Image quality         (Exif.SonyMisc3c.Quality2           ) : JPEG
""",
    """File 1/1: $filename
Del Exif.Sony1.WhiteBalance
""",
	"""White balance         (Exif.Sony1.WhiteBalance2           ) : Auto
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Sony DSLR-A100
###########################################################
@CopyTmpFiles("$data_path/SonyDSLR-A100.exv")
class Sony2(metaclass=CaseMeta):

    filename = path("$tmp_path/SonyDSLR-A100.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.SonyMinolta.LensID" $filename""",
        "$easyaccess_test $filename LensName"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2006:06:09 11:20:37
Flash bias            (Exif.SonyMinolta.FlashExposureComp ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Auto
Scene mode            (Exif.SonyMinolta.SceneMode         ) : Auto
Macro mode            (Exif.Sony1.Macro                   ) : n/a
Image quality         (Exif.Sony1.JPEGQuality             ) : n/a
White balance         (Exif.Sony1MltCsA100.WhiteBalance   ) : Auto
Lens name             (Exif.SonyMinolta.LensID            ) : Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : SONY           
Camera model          (Exif.Image.Model                   ) : DSLR-A100
Exposure time         (Exif.Photo.ExposureTime            ) : 1/200 s
FNumber               (Exif.Photo.FNumber                 ) : F10
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 9.37
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F3.5
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 18.0 mm
Subject location/area (Exif.Photo.SubjectArea             ) : 1448 968 246 294
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.Sony1MltCsA100.LocalAFAreaPoint) : Center
""",
    """File 1/1: $filename
Del Exif.SonyMinolta.LensID
""",
	"""Lens name             (Exif.Sony1.LensID                  ) : Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3
"""
    ]
    stderr = ["""Warning: Directory SonyMinolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory SonyMinolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
Warning: Directory SonyMinolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
""",
    """Warning: Directory SonyMinolta, entry 0x0088: Data area exceeds data buffer, ignoring it.
"""
    ]
    retval = [0] * len(commands)

###########################################################
# Sony ILCE-7M3
###########################################################
@CopyTmpFiles("$data_path/exiv2-pr906.exv")
class Sony3(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-pr906.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Photo.ExposureProgram" $filename""",
        "$easyaccess_test $filename ExposureMode"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2018:10:17 08:55:12
Flash bias            (Exif.Sony2.FlashExposureComp       ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Manual
Scene mode            (Exif.Sony2.SceneMode               ) : Standard
Macro mode            (                                   ) : 
Image quality         (Exif.Sony2.Quality                 ) : RAW + JPEG/HEIF
White balance         (Exif.Sony2.WhiteBalance            ) : Custom
Lens name             (Exif.Photo.LensModel               ) : FE 85mm F1.4 GM
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : SONY
Camera model          (Exif.Image.Model                   ) : ILCE-7M3
Exposure time         (Exif.Photo.ExposureTime            ) : 1/100 s
FNumber               (Exif.Photo.FNumber                 ) : F8
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 6.59922
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F1.4
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Other light source
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 85.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.Sony2.AFPointSelected         ) : Center Zone
""",
    """File 1/1: $filename
Del Exif.Photo.ExposureProgram
""",
	"""Exposure mode         (Exif.Sony2.ExposureMode            ) : Manual
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Sony DSC-RX100M5A
###########################################################
@CopyTmpFiles("$data_path/exiv2-pr2323_coverage3.exv")
class Sony4(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-pr2323_coverage3.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Sony1.Quality" $filename""",
        "$easyaccess_test $filename ImageQuality",
		"""$exiv2 -u -v -M"del Exif.Photo.Saturation" $filename""",
        "$easyaccess_test $filename Saturation",
		"""$exiv2 -u -v -M"del Exif.Photo.Sharpness" $filename""",
        "$easyaccess_test $filename Sharpness",
		"""$exiv2 -u -v -M"del Exif.Photo.Contrast" $filename""",
        "$easyaccess_test $filename Contrast",
		"""$exiv2 -u -v -M"del Exif.Photo.MeteringMode" $filename""",
        "$easyaccess_test $filename MeteringMode",
		"""$exiv2 -u -v -M"del Exif.Image.Model" $filename""",
        "$easyaccess_test $filename Model",
		"""$exiv2 -u -v -M"del Exif.Photo.BrightnessValue" $filename""",
        "$easyaccess_test $filename Brightness",
		"""$exiv2 -u -v -M"del Exif.Photo.Flash" $filename""",
        "$easyaccess_test $filename Flash"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 200
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2018:06:14 11:35:45
Flash bias            (Exif.Sony1.FlashExposureComp       ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Aperture priority
Scene mode            (Exif.Sony1.SceneMode               ) : Standard
Macro mode            (                                   ) : 
Image quality         (Exif.Sony1.Quality                 ) : RAW + JPEG/HEIF
White balance         (Exif.Sony1.WhiteBalance            ) : Auto
Lens name             (Exif.Sony1.LensSpec                ) : 24-70mm F1.8-2.8
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : SONY
Camera model          (Exif.Image.Model                   ) : DSC-RX100M5A
Exposure time         (Exif.Photo.ExposureTime            ) : 1/125 s
FNumber               (Exif.Photo.FNumber                 ) : F3.2
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 5.22031
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F2.8
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Unknown
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 18.5 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.Sony1.AFPointSelected         ) : n/a
""",
    """File 1/1: $filename
Del Exif.Sony1.Quality
""",
	"""Image quality         (Exif.Sony1.Quality2                ) : RAW + extra fine
""",
    """File 1/1: $filename
Del Exif.Photo.Saturation
""",
	"""Saturation            (Exif.Sony1.Saturation              ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Sharpness
""",
	"""Sharpness             (Exif.Sony1.Sharpness               ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Contrast
""",
	"""Contrast              (Exif.Sony1.Contrast                ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.MeteringMode
""",
	"""Metering mode         (Exif.Sony1.MeteringMode2           ) : Multi-segment
""",
    """File 1/1: $filename
Del Exif.Image.Model
""",
	"""Camera model          (Exif.Sony1.SonyModelID             ) : DSC-RX100M5A
""",
    """File 1/1: $filename
Del Exif.Photo.BrightnessValue
""",
	"""Brightness value      (Exif.Sony1.Brightness              ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Flash
""",
	"""Flash                 (Exif.Sony1.FlashAction             ) : Did not fire
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# Sony DSC-RX100M5A
###########################################################
@CopyTmpFiles("$data_path/exiv2-pr906.exv")
class Sony5(metaclass=CaseMeta):

    filename = path("$tmp_path/exiv2-pr906.exv")

    commands = [
        "$easyaccess_test $filename",
		"""$exiv2 -u -v -M"del Exif.Sony2.Quality" $filename""",
        "$easyaccess_test $filename ImageQuality",
		"""$exiv2 -u -v -M"del Exif.Sony2.WhiteBalance" $filename""",
        "$easyaccess_test $filename WhiteBalance",
		"""$exiv2 -u -v -M"del Exif.Photo.LensModel" $filename""",
        "$easyaccess_test $filename LensName",
		"""$exiv2 -u -v -M"del Exif.Sony2.LensID" $filename""",
        "$easyaccess_test $filename LensName",
		"""$exiv2 -u -v -M"del Exif.Photo.Saturation" $filename""",
        "$easyaccess_test $filename Saturation",
		"""$exiv2 -u -v -M"del Exif.Photo.Sharpness" $filename""",
        "$easyaccess_test $filename Sharpness",
		"""$exiv2 -u -v -M"del Exif.Photo.Contrast" $filename""",
        "$easyaccess_test $filename Contrast",
		"""$exiv2 -u -v -M"del Exif.Photo.MeteringMode" $filename""",
        "$easyaccess_test $filename MeteringMode",
		"""$exiv2 -u -v -M"del Exif.Image.Model" $filename""",
        "$easyaccess_test $filename Model",
		"""$exiv2 -u -v -M"del Exif.Photo.BrightnessValue" $filename""",
        "$easyaccess_test $filename Brightness",
		"""$exiv2 -u -v -M"del Exif.Photo.Flash" $filename""",
        "$easyaccess_test $filename Flash"
    ]
    stdout = [
	"""Orientation           (Exif.Image.Orientation             ) : top, left
ISO speed             (Exif.Photo.ISOSpeedRatings         ) : 100
Date & time original  (Exif.Photo.DateTimeOriginal        ) : 2018:10:17 08:55:12
Flash bias            (Exif.Sony2.FlashExposureComp       ) : 0 EV
Exposure mode         (Exif.Photo.ExposureProgram         ) : Manual
Scene mode            (Exif.Sony2.SceneMode               ) : Standard
Macro mode            (                                   ) : 
Image quality         (Exif.Sony2.Quality                 ) : RAW + JPEG/HEIF
White balance         (Exif.Sony2.WhiteBalance            ) : Custom
Lens name             (Exif.Photo.LensModel               ) : FE 85mm F1.4 GM
Saturation            (Exif.Photo.Saturation              ) : Normal
Sharpness             (Exif.Photo.Sharpness               ) : Normal
Contrast              (Exif.Photo.Contrast                ) : Normal
Scene capture type    (Exif.Photo.SceneCaptureType        ) : Standard
Metering mode         (Exif.Photo.MeteringMode            ) : Multi-segment
Camera make           (Exif.Image.Make                    ) : SONY
Camera model          (Exif.Image.Model                   ) : ILCE-7M3
Exposure time         (Exif.Photo.ExposureTime            ) : 1/100 s
FNumber               (Exif.Photo.FNumber                 ) : F8
Shutter speed value   (                                   ) : 
Aperture value        (                                   ) : 
Brightness value      (Exif.Photo.BrightnessValue         ) : 6.59922
Exposure bias         (Exif.Photo.ExposureBiasValue       ) : 0 EV
Max aperture value    (Exif.Photo.MaxApertureValue        ) : F1.4
Subject distance      (                                   ) : 
Light source          (Exif.Photo.LightSource             ) : Other light source
Flash                 (Exif.Photo.Flash                   ) : No, compulsory
Camera serial number  (                                   ) : 
Focal length          (Exif.Photo.FocalLength             ) : 85.0 mm
Subject location/area (                                   ) : 
Flash energy          (                                   ) : 
Exposure index        (                                   ) : 
Sensing method        (                                   ) : 
AF point              (Exif.Sony2.AFPointSelected         ) : Center Zone
""",
    """File 1/1: $filename
Del Exif.Sony2.Quality
""",
	"""Image quality         (Exif.Sony2.Quality2                ) : RAW + extra fine
""",
    """File 1/1: $filename
Del Exif.Sony2.WhiteBalance
""",
	"""White balance         (Exif.Sony2.WhiteBalance2           ) : Manual
""",
    """File 1/1: $filename
Del Exif.Photo.LensModel
""",
	"""Lens name             (Exif.Sony2.LensID                  ) : Manual lens
""",
    """File 1/1: $filename
Del Exif.Sony2.LensID
""",
	"""Lens name             (Exif.Sony2.LensSpec                ) : FE 85mm F1.4
""",
    """File 1/1: $filename
Del Exif.Photo.Saturation
""",
	"""Saturation            (Exif.Sony2.Saturation              ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Sharpness
""",
	"""Sharpness             (Exif.Sony2.Sharpness               ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Contrast
""",
	"""Contrast              (Exif.Sony2.Contrast                ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.MeteringMode
""",
	"""Metering mode         (Exif.Sony2.MeteringMode2           ) : Multi-segment
""",
    """File 1/1: $filename
Del Exif.Image.Model
""",
	"""Camera model          (Exif.Sony2.SonyModelID             ) : ILCE-7M3
""",
    """File 1/1: $filename
Del Exif.Photo.BrightnessValue
""",
	"""Brightness value      (Exif.Sony2.Brightness              ) : 0
""",
    """File 1/1: $filename
Del Exif.Photo.Flash
""",
	"""Flash                 (Exif.Sony2.FlashAction             ) : Did not fire
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)

###########################################################
# test cases for a single specific tag
###########################################################
#Exif.Image.ISOSpeedRatings
class ImageISOSpeedRatings(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-bug479.jpg")

    commands = [
        "$easyaccess_test $filename ISOspeed"
    ]
    stdout = [
	"""ISO speed             (Exif.Image.ISOSpeedRatings         ) : 250
"""
    ]
    stderr = ["""Error: Directory Image: Next pointer is out of bounds; ignored.
"""] * 1
    retval = [0] * 1

#Exif.Sony2.SerialNumber
class Sony2SerialNumber(metaclass=CaseMeta):

    filename = path("$data_path/exiv2-pr2323_coverage1.exv")

    commands = [
        "$easyaccess_test $filename SerialNumber"
    ]
    stdout = [
	"""Camera serial number  (Exif.Sony1.SerialNumber            ) : 12345678
"""
    ]
    stderr = [""] * 1
    retval = [0] * 1

