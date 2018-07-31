# -*- coding: utf-8 -*-
import unittest
import os

import system_tests

@unittest.skipUnless(os.getenv('TZ') == 'UTC', "Testcase only works with the timezone set to UTC")
class Exiv2jsonRecursiveJsonTreeWithXMP(metaclass=system_tests.CaseMeta):

    url = "http://dev.exiv2.org/issues/1054"

    filename1 = system_tests.path("$data_path/BlueSquare.xmp")
    filename2 = system_tests.path("$data_path/exiv2-bug784.jpg")

    commands = [ "$exiv2json $filename1",
                 "$exiv2json x $filename1",
                 "$exiv2json $filename2",
               ]

    stdout = [ """{
	"Exif": {
		"Image": {
			"ImageWidth": 360,
			"ImageLength": 216,
			"Orientation": 1,
			"XResolution": [
				720000,
				10000
			],
			"YResolution": [
				720000,
				10000
			],
			"ResolutionUnit": 2,
			"DateTime": "2005:09:07 22:09:51",
			"ImageDescription": "XMPFiles BlueSquare test file, created in Photoshop CS2, saved as .psd, .jpg, and .tif.",
			"Make": "Nikon"
		},
		"Photo": {
			"ColorSpace": 1,
			"PixelXDimension": 360,
			"PixelYDimension": 216,
			"DateTimeDigitized": "2005:09:07 22:07:40"
		}
	},
	"Iptc": {
		"Application2": {
			"ObjectName": "Blue Square Test File - .jpg",
			"Keywords": "XMP",
			"Keywords": "Blue Square",
			"Keywords": "test file",
			"Keywords": "Photoshop",
			"Keywords": ".jpg",
			"DigitizationDate": "2005-09-07",
			"Caption": "XMPFiles BlueSquare test file, created in Photoshop CS2, saved as .psd, .jpg, and .tif."
		},
		"Envelope": {
			"CharacterSet": "%G"
		}
	},
	"Xmp": {
		"dc": {
			"format": "image\/jpeg",
			"title": {
				"lang": {
					"x-default": "Blue Square Test File - .jpg",
					"en-US": "Blue Square Test File - .jpg",
					"de-CH": "Blaues Quadrat Test Datei - .jpg"
				}
			},
			"description": {
				"lang": {
					"x-default": "XMPFiles BlueSquare test file, created in Photoshop CS2, saved as .psd, .jpg, and .tif."
				}
			},
			"subject": "XMP, Blue Square, test file, Photoshop, .jpg"
		},
		"xmp": {
			"CreatorTool": "Adobe Photoshop CS2 Macintosh",
			"CreateDate": "2005-09-07T15:07:40-07:00",
			"ModifyDate": "2005-09-07T15:09:51-07:00",
			"MetadataDate": "2006-04-10T13:37:10-07:00"
		},
		"xmpMM": {
			"DocumentID": "uuid:9A3B7F52214211DAB6308A7391270C13",
			"InstanceID": "uuid:B59AC1B3214311DAB6308A7391270C13",
			"DerivedFrom": {
				"stRef": {
					"instanceID": "uuid:9A3B7F4F214211DAB6308A7391270C13",
					"documentID": "uuid:9A3B7F4E214211DAB6308A7391270C13"
				}
			}
		},
		"photoshop": {
			"ColorMode": "3",
			"ICCProfile": "sRGB IEC61966-2.1"
		},
		"tiff": {
			"Orientation": "1",
			"XResolution": "720000\/10000",
			"YResolution": "720000\/10000",
			"ResolutionUnit": "2",
			"ImageWidth": "360",
			"ImageLength": "216",
			"NativeDigest": "256,257,258,259,262,274,277,284,530,531,282,283,296,301,318,319,529,532,306,270,271,272,305,315,33432;D0485928256FC8D17D036C26919E106D",
			"Make": "Nikon",
			"BitsPerSample": "8, 8, 8"
		},
		"exif": {
			"PixelXDimension": "360",
			"PixelYDimension": "216",
			"ColorSpace": "1",
			"NativeDigest": "36864,40960,40961,37121,37122,40962,40963,37510,40964,36867,36868,33434,33437,34850,34852,34855,34856,37377,37378,37379,37380,37381,37382,37383,37384,37385,37386,37396,41483,41484,41486,41487,41488,41492,41493,41495,41728,41729,41730,41985,41986,41987,41988,41989,41990,41991,41992,41993,41994,41995,41996,42016,0,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,23,24,25,26,27,28,30;76DBD9F0A5E7ED8F62B4CE8EFA6478B4"
		},
		"xmlns": {
			"DerivedFrom": "",
			"dc": "http:\/\/purl.org\/dc\/elements\/1.1\/",
			"exif": "http:\/\/ns.adobe.com\/exif\/1.0\/",
			"photoshop": "http:\/\/ns.adobe.com\/photoshop\/1.0\/",
			"tiff": "http:\/\/ns.adobe.com\/tiff\/1.0\/",
			"xmp": "http:\/\/ns.adobe.com\/xap\/1.0\/",
			"xmpMM": "http:\/\/ns.adobe.com\/xap\/1.0\/mm\/"
		}
	}
}
""",
    """{
	"Xmp": {
		"dc": {
			"format": "image\/jpeg",
			"title": {
				"lang": {
					"x-default": "Blue Square Test File - .jpg",
					"en-US": "Blue Square Test File - .jpg",
					"de-CH": "Blaues Quadrat Test Datei - .jpg"
				}
			},
			"description": {
				"lang": {
					"x-default": "XMPFiles BlueSquare test file, created in Photoshop CS2, saved as .psd, .jpg, and .tif."
				}
			},
			"subject": "XMP, Blue Square, test file, Photoshop, .jpg"
		},
		"xmp": {
			"CreatorTool": "Adobe Photoshop CS2 Macintosh",
			"CreateDate": "2005-09-07T15:07:40-07:00",
			"ModifyDate": "2005-09-07T15:09:51-07:00",
			"MetadataDate": "2006-04-10T13:37:10-07:00"
		},
		"xmpMM": {
			"DocumentID": "uuid:9A3B7F52214211DAB6308A7391270C13",
			"InstanceID": "uuid:B59AC1B3214311DAB6308A7391270C13",
			"DerivedFrom": {
				"stRef": {
					"instanceID": "uuid:9A3B7F4F214211DAB6308A7391270C13",
					"documentID": "uuid:9A3B7F4E214211DAB6308A7391270C13"
				}
			}
		},
		"photoshop": {
			"ColorMode": "3",
			"ICCProfile": "sRGB IEC61966-2.1"
		},
		"tiff": {
			"Orientation": "1",
			"XResolution": "720000\/10000",
			"YResolution": "720000\/10000",
			"ResolutionUnit": "2",
			"ImageWidth": "360",
			"ImageLength": "216",
			"NativeDigest": "256,257,258,259,262,274,277,284,530,531,282,283,296,301,318,319,529,532,306,270,271,272,305,315,33432;D0485928256FC8D17D036C26919E106D",
			"Make": "Nikon",
			"BitsPerSample": "8, 8, 8"
		},
		"exif": {
			"PixelXDimension": "360",
			"PixelYDimension": "216",
			"ColorSpace": "1",
			"NativeDigest": "36864,40960,40961,37121,37122,40962,40963,37510,40964,36867,36868,33434,33437,34850,34852,34855,34856,37377,37378,37379,37380,37381,37382,37383,37384,37385,37386,37396,41483,41484,41486,41487,41488,41492,41493,41495,41728,41729,41730,41985,41986,41987,41988,41989,41990,41991,41992,41993,41994,41995,41996,42016,0,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,20,22,23,24,25,26,27,28,30;76DBD9F0A5E7ED8F62B4CE8EFA6478B4"
		},
		"xmlns": {
			"DerivedFrom": "",
			"dc": "http:\/\/purl.org\/dc\/elements\/1.1\/",
			"exif": "http:\/\/ns.adobe.com\/exif\/1.0\/",
			"photoshop": "http:\/\/ns.adobe.com\/photoshop\/1.0\/",
			"tiff": "http:\/\/ns.adobe.com\/tiff\/1.0\/",
			"xmp": "http:\/\/ns.adobe.com\/xap\/1.0\/",
			"xmpMM": "http:\/\/ns.adobe.com\/xap\/1.0\/mm\/"
		}
	}
}
""",
    """{
	"Exif": {
		"Image": {
			"Orientation": 1,
			"XResolution": [
				72,
				1
			],
			"YResolution": [
				72,
				1
			],
			"ResolutionUnit": 2,
			"YCbCrPositioning": 1,
			"Copyright": "Public Domain. Do whatever you like with this image",
			"ExifTag": 232
		},
		"Photo": {
			"ExifVersion": "48 50 50 49"
		}
	},
	"Iptc": {
		"Envelope": {
			"CharacterSet": "%G"
		},
		"Application2": {
			"RecordVersion": 4,
			"Keywords": "1st",
			"Keywords": "2nd",
			"Keywords": "next1",
			"Keywords": "next2",
			"Keywords": "root",
			"Keywords": "root0",
			"Copyright": "Public Domain. Do whatever you like with this image"
		}
	},
	"Xmp": {
		"dc": {
			"format": "image\/jpeg",
			"rights": {
				"lang": {
					"x-default": "Public Domain. Do whatever you like with this image"
				}
			},
			"subject": "1st, 2nd, next1, next2, root, root0"
		},
		"xmpMM": {
			"DocumentID": "004D48F936062EF5085A81BF96D4C494",
			"OriginalDocumentID": "004D48F936062EF5085A81BF96D4C494",
			"InstanceID": "xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853",
			"History": [
				{
					"stEvt": {
						"action": "saved",
						"instanceID": "xmp.iid:f74f0d02-e921-134e-8107-1dda17aad853",
						"when": "2015-03-24T20:35:55-05:00",
						"softwareAgent": "Adobe Photoshop Lightroom 4.4 (Windows)",
						"changed": "\/metadata"
					}
				}
			]
		},
		"xmp": {
			"MetadataDate": "2015-03-24T20:35:55-05:00"
		},
		"crs": {
			"RawFileName": "exiv2.lr.jpg"
		},
		"lr": {
			"hierarchicalSubject": "root0|next1|next2, root|1st|2nd"
		},
		"xmlns": {
			"crs": "http:\/\/ns.adobe.com\/camera-raw-settings\/1.0\/",
			"dc": "http:\/\/purl.org\/dc\/elements\/1.1\/",
			"lr": "http:\/\/ns.adobe.com\/lightroom\/1.0\/",
			"stEvt": "http:\/\/ns.adobe.com\/xap\/1.0\/sType\/ResourceEvent#",
			"xmp": "http:\/\/ns.adobe.com\/xap\/1.0\/",
			"xmpMM": "http:\/\/ns.adobe.com\/xap\/1.0\/mm\/"
		}
	}
}
"""
    ]
    stderr = [""] * len(commands)
    retval = [0] * len(commands)
