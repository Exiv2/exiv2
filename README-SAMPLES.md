| Travis        | AppVeyor      | GitLab| Codecov| Repology| Chat |
|:-------------:|:-------------:|:-----:|:------:|:-------:|:----:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=0.27-maintenance)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/0.27-maintenance?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/0.27-maintenance) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/0.27-maintenance/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/0.27-maintenance) | [![codecov](https://codecov.io/gh/Exiv2/exiv2/branch/0.27-maintenance/graph/badge.svg)](https://codecov.io/gh/Exiv2/exiv2) | [![Packaging status](https://repology.org/badge/tiny-repos/exiv2.svg)](https://repology.org/metapackage/exiv2/versions) | [![#exiv2-chat on matrix.org](matrix-standard-vector-logo-xs.png)](https://matrix.to/#/#exiv2-chat:matrix.org) |

![Exiv2](exiv2.png)

# Exiv2 Sample Applications

Exiv2 is a C++ library and a command line utility to read, write, delete and modify Exif, IPTC, XMP and ICC image metadata.  Exiv2 also features a collection of sample and test command-line programs.  Please be aware that while the program _**exiv2**_ enjoys full support from Team Exiv2, the other programs have been written for test, documentation or development purposes.  You are expected to read the code to discover the specification of programs other than _**exiv2**_.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="TOC1">

### Sample Programs

The following programs are build and installed in /usr/local/bin.

| Name | Purpose | More information | Code |
|:---  |:---  |:---              |:-- |
| _**addmoddel**_   | Demonstrates Exiv2 library APIs to add, modify or delete metadata | [addmoddel](#addmoddel) | [addmoddel.cpp](samples/addmoddel.cpp) |
| _**exifcomment**_ | Set Exif.Photo.UserComment in an image  | [exifcomment](#exifcomment) | [exifcomment.cpp](samples/exifcomment.cpp) |
| _**exifdata**_    | Prints _**Exif**_ metadata in different formats in an image | [exifdata](#exifdata) | [exifdata.cpp](samples/exifdata.cpp) |
| _**exifprint**_   | Print _**Exif**_ metadata in images<br>Miscellaneous other features | [exifprint](#exifprint)| [exifprint.cpp](samples/exifprint.cpp) |
| _**exifvalue**_   | Prints the value of a single _**Exif**_ tag in a file | [exifvalue](#exifvalue) | [exifvalue.cpp](samples/exifvalue.cpp) |
| _**exiv2**_       | Utility to read and write image metadata, including Exif, IPTC, XMP, image comments, ICC Profile, thumbnails, image previews and many vendor makernote tags.<br>This is the primary test tool used by Team Exiv2 and can exercise almost all code in the library.  Due to the extensive capability of this utility, the APIs used are usually less obvious for casual code inspection. | [exiv2 manpage](exiv2.md)<br>[https://exiv2.org/sample.html](https://exiv2.org/sample.html) | |
| _**exiv2json**_   | Extracts data from image in JSON format.<br>This program also contains a parser to recursively parse Xmp metadata into vectors and objects. | [exiv2json](#exiv2json) | [exiv2json.cpp](samples/exiv2json.cpp) |
| _**geotag**_      | Reads GPX data and updates images with GPS Tags | [geotag](#geotag) | [geotag.cpp](samples/geotag.cpp) |
| _**iptceasy**_    | Demonstrates read, set or modify IPTC metadata | [iptceasy](#iptceasy) | [iptceasy.cpp](samples/iptceasy.cpp) |
| _**iptcprint**_   | Demonstrates Exiv2 library APIs to print Iptc data | [iptceasy](#iptceasy) | [iptcprint.cpp](samples/iptcprint.cpp) |
| _**metacopy**_    | Demonstrates copying metadata from one image to another | [metacopy](#metacopy) | [metacopy.cpp](samples/metacopy.cpp) |
| _**mrwthumb**_    | Sample program to extract a Minolta thumbnail from the makernote | [mrwthumb](#mrwthumb) | [mrwthumb.cpp](samples/mrwthumb.cpp) |
| _**taglist**_     | Print a simple comma separated list of tags defined in Exiv2 | [taglist](#taglist) |
| _**xmpdump**_     | Sample program to dump the XMP packet of an image | [xmpdump](#xmpdump) |
| _**xmpparse**_    | Read an XMP packet from a file, parse it and print all (known) properties. | [xmpparse](#xmpparse) | [xmpparse.cpp](samples/xmpparse.cpp) |
| _**xmpprint**_    | Read an XMP from a file, parse it and print all (known) properties.. | [xmpprint](#xmpprint) | [xmpprint.cpp](samples/xmpprint.cpp) |
| _**xmpsample**_   | Demonstrates Exiv2 library high level XMP classes | [xmpsample](#xmpsample) | [xmpsample.cpp](samples/exmpsample.cpp) |

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="TOC2">

### Test Programs

As Exiv2 is open source, we publish all our materials.  The following programs are actively used in our test harness.  Some were written during feature development of features and their on-going use may be limited, or even obsolete.  In general these programs are published as source and Team Exiv2 will not provide support to users.

| Name | Kind | More information |
|:---  |:---  |:---              |
| _**conntest**_ | Test http/https/ftp/ssh/sftp connection | [conntest](#conntest) |
| _**convert-test**_ | Conversion test driver | [convert-test](#convert-test) |
| _**easyaccess-test**_ | Sample program using high-level metadata access functions | [easyaccess-test](#easyaccess-test) |
| _**getopt-test**_ | Sample program to test getopt() | [getopt-test](#getopt-test) |
| _**ini-test**_ | Shows simple usage of the INIReader class | [ini-test](#ini-test) |
| _**iotest**_ | Test programs for BasicIo functions. | [iotest](#iotest) |
| _**iptctest**_ | Sample program test Iptc reading and writing. | [iptctest](#iptctest) |
| _**key-test**_ | Key unit tests | [key-test](#key-test) |
| _**largeiptc-test**_ | Test for large (>65535 bytes) IPTC buffer | [largeiptc-test](#largeiptc-test) |
| _**mmap-test**_ | Simple mmap tests | [mmap-test](#mmap-test) |
| _**path-test**_ | Test path IO | [path-test](#path-test) |
| _**prevtest**_ | Test access to preview images | [prevtest](#prevtest) |
| _**remotetest**_ | Tester application for testing remote i/o. | [remotetest](#remotetest) |
| _**stringto-test**_ | Test conversions from string to long, float and Rational types. | [stringto-test](#stringto-test) |
| _**tiff-test**_ | Simple TIFF write test | [tiff-test](#tiff-test) |
| _**write-test**_ | ExifData write unit tests | [write-test](#write-test) |
| _**write2-test**_ | ExifData write unit tests for Exif data created from scratch | [write2-test](#write2-test) |
| _**xmpparser-test**_ | Read an XMP packet from a file, parse and re-serialize it. | [xmpparser-test](#xmpparser-test)|

[Sample](#TOC1) Programs [Test](#TOC2) Programs

## 2 Sample Program Descriptions

<div id="addmoddel">

#### addmoddel

```
Usage: addmoddel file
```

Demonstrates Exiv2 library APIs to add, modify or delete metadata.  _Code: [addmoddel.cpp](samples/addmoddel.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="exifcomment">

#### exifcomment

```
Usage: exifcomment file
```

This is a simple program that demonstrates how to set _**Exif.Photo.UserComment**_ in an image. _Code: [exifcomment.cpp](samples/exifcomment.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="exifdata">

### exifdata

```
Usage: exifdata file format
formats: csv | json | wolf | xml
```

This is a simple program to demonstrate dumping _**Exif**_ metadata in common formats.  _Code: [exifdata.cpp](samples/exifdata.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="exifprint">

#### exifprint

```
Usage: exifprint [ [--lint] path | --version | --version-test ]
```

| Arguments | Description |
|:--        |:---  |
| path    | Path to image |
| --lint path | Path to image.  Type metadata test |
| --version     | Print version information from build |
| --version-test   | Tests Exiv2 VERSION API |

This program demonstrates how to print _**Exif**_ metadata in an image.  This program is also discussed in the platform ReadMe.txt file included in a build bundle.  The option **--version** was added to enable the user to build a test application which dumps the build information.  The option **--version-test** was added to test the macro EXIV2\_TEST\_VERSION() in **include/exiv2/version.hpp**.

You can process the metadata in two different ways.  The default prints the metadata.  The option --lint instructs exifprint to compare the type of the metadata to the standard.

There is another unique feature of this program.  It is the only test/sample program which can use the EXV\_UNICODE\_PATH build feature of Exiv2 on Windows.

_Code: [exifprint.cpp](samples/exifprint.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="exifvalue">

#### exifvalue

```
Usage: exifvalue file tag
```
Prints the value of a single _**Exif**_ tag in a file. _Code: [exifvalue.cpp](samples/exifvalue.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="exiv2json">

#### exiv2json

```
Usage: exiv2json [-option] file
Option: all | exif | iptc | xmp | filesystem
```
| Arguments | Description |
|:--        |:---  |
| all      | All metadata |
| filesystem    | Filesystem metadata |
| exif     | Exif metadata |
| iptc     | Iptc metadata |
| xmp      | Xmp metadata |
| file   | path to image |

This program dumps metadata from an image in JSON format. _Code: [exiv2json.cpp](samples/exiv2json.cpp)_

exiv2json has a recursive parser to encode XMP into Vectors and Objects.  XMP data is XMP and can contain XMP `Bag` and `Seq` which are converted to JSON Objects and Arrays.  Exiv2 presents data in the format:  [Family.Group.Tagname](exiv2.md#exiv2_key_syntax).  For XMP, results in "flat" output such as:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --print x Stonehenge.jpg
Xmp.xmp.Rating                               XmpText     1  0
Xmp.xmp.ModifyDate                           XmpText    25  2015-07-16T20:25:28+01:00
Xmp.cm2e.Father                              XmpText    11  Robin Mills
Xmp.cm2e.Family                              XmpBag      0
Xmp.dc.description                           LangAlt     1  lang="x-default" Classic View
Xmp.dc.Family                                XmpBag      1  Robin
```

exiv2json parses the Exiv2 [Family.Group.Tagname](exiv2.md#exiv2_key_syntax) data and restores the structure of the original data in JSON.  _Code: [exiv2json.cpp](samples/exiv2json.cpp)_

```
$ exiv2json -xmp http://clanmills.com/Stonehenge.jpg
{
  "Xmp": {
    "xmp": {
      "Rating": "0",
      "ModifyDate": "2015-07-16T20:25:28+01:00"
    },
    "dc": {
      "description": {
        "lang": {
          "x-default": "Classic View"
        }
      }
    },
    "xmlns": {
      "dc": "http:\/\/purl.org\/dc\/elements\/1.1\/",
      "xmp": "http:\/\/ns.adobe.com\/xap\/1.0\/"
    }
  }
}
$
```

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="geotag">

#### geotag

```
Usage: geotag {-help|-version|-dst|-dryrun|-ascii|-verbose|-adjust value|-tz value|-delta value}+ path+
```

Geotag reads one or more GPX files and adds GPS Tags to images.  _Code: [geotag.cpp](samples/geotag.cpp)_

If the path is a directory, geotag will read all the files in the directory.  It constructs a time dictionary of position data, then updates every image with GPS Tags.

| Arguments | Description |
|:--        |:---  |
| -help     | print usage statement |
| -version  | prints data and time of compiling geotag.cpp |
| -dst      | Apply 1 hour adjustment for daylight saving time. |
| -dryrun   | Read arguments and print report.  Does not modify images. |
| -ascii    | Output in ascii (not UTF8).  Prints `deg` instead of &deg;. |
| -verbose  | Report progress. |
| -adjust  value | Add/subtract time from image data. |
| -tz      value | Specify time zone. For example PST = -8:00 |
| -delta   value | Correction between Image DataTime and GPS time. |
| path+ | One or more directories, image paths or gpx paths.  Directories are searched for gpx and images |

I use this program frequently.  I have a little Canon camera which I take when I run.  My Samsung Galaxy Watch uploads my runs to Strava and I download the GPX.  If I'm in another time-zone and have forgotten to change the time setting in the camera, I use `-adjust` to alter the images.  The GPX time is always correct, however the camera is normally off by seconds or minutes.  This option enables you to correct for inaccuracy in the setting of the camera time.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="iptceasy">

#### iptceasy

```
Usage: iptceasy file
Reads and writes raw metadata. Use -h option for help.
```

Demonstrates read, set or modify IPTC metadata. _Code: [iptceasy.cpp](samples/iptceasy.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="iptcprint">

#### iptcprint

```
Usage: iptcprint file
Reads and writes raw metadata. Use -h option for help.
```

Demonstrates Exiv2 library APIs to print Iptc data. _Code: [iptcprint.cpp](samples/iptcprint.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="metacopy">

#### metacopy

```
Usage: metacopy [-iecxaph] readfile writefile
Reads and writes raw metadata. Use -h option for help.
```

Metacopy is used to copy a complete metadata block from one file to another.  _Code: [metacopy.cpp](samples/metacopy.cpp)_

Please note that some metadata such as Exif.Photo.PixelXDimension is considered to be part of the image and will not be copied.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="mrwthumb">

#### mrwthumb

```
Usage: mrwthumb file
```

Sample program to extract a Minolta thumbnail from the makernote. _Code: [mrwthumb.cpp](samples/mrwthumb.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="xmpparse">

#### xmpparse

```
Usage: xmpparse file
```

Read an XMP packet from a file, parse it and print all (known) properties.  _Code: [xmpparse.cpp](samples/xmpparse.cpp)_


[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="xmpprint">

#### xmpprint

```
Usage: xmpprint file
```

Read an XMP from a file, parse it and print all (known) properties. _Code: [xmpprint.cpp](samples/xmpprint.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="xmpsample">

#### xmpsample

```
Usage: xmpsample file
```

Demonstrates Exiv2 library high level XMP classes.  _Code: [xmpsample.cpp](samples/xmpsample.cpp)_

[Sample](#TOC1) Programs [Test](#TOC2) Programs

## 3 Test Program Descriptions

<div id="conntest">

#### conntest

```
Usage: conntest url {-http1_0}
```

Test http/https/ftp/ssh/sftp connection

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="convert-test">

#### convert-test

```
Usage: convert-test file
```

Conversion test driver

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="easyaccess-test">

#### easyaccess-test

```
Usage: ..\build\bin\easyaccess-test.exe file [category [category ...]]
Categories: Orientation | ISOspeed | DateTimeOriginal | FlashBias | ExposureMode | SceneMode |
            MacroMode | ImageQuality | WhiteBalance | LensName | Saturation | Sharpness |
            Contrast | SceneCaptureType | MeteringMode | Make | Model | ExposureTime | FNumber |
            ShutterSpeed | Aperture | Brightness | ExposureBias | MaxAperture | SubjectDistance |
            LightSource | Flash | SerialNumber | FocalLength | SubjectArea | FlashEnergy |
            ExposureIndex | SensingMethod | AFpoint
```

Sample program using high-level metadata access functions. Without specification of a category, metadata for all categories are shown.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="getopt-test">

#### getopt-test

```
Usage: getopt-test
```
This program is used to test the function **getopt()**.  Prior to Exiv2 v0.27, the sample programs used the platform's C Runtime Library function **getopt()**.  Visual Studio builds used code in src/getopt.cpp.  Due to differences in the platform **getopt()**, the code in src/getopt.cpp was modified and adopted on every platforms.  This test program was added for test and debug purpose.  Please note that src/getopt.cpp is compiled and linked into the sample application and is not part of the Exiv2 library.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="ini-test">

#### ini-test

```
Usage: ini-test
```

This program is used to test reading the file ini-test.  This program was added in Exiv2 v0.26 when the ~/.exiv2 file was added to the Exiv2 architecture.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="iotest">

#### iotest

```
Usage: iotest filein fileout1 fileout2 [remote [blocksize]]
copy filein to fileout1 and copy filein to fileout2
fileout1 and fileout2 are overwritten and should match filein exactly

You may optionally provide the URL of a remote file to be copied to filein
If you use `remote`, you may optionally provide a blocksize for the copy buffer (default 10k)
```

Test programs for BasicIo functions.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="iptctest">

#### iptctest

```
Usage: iptctest image
Commands read from stdin.
```

Sample program test Iptc reading and writing.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="key-test">

#### key-test

```
Usage: key-test
```

Key unit tests

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="largeiptc-test">

#### largeiptc-test

```
Usage: largeiptc-test image datafile
```

Test for large (>65535 bytes) IPTC buffer

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="mmap-test">

#### mmap-test

```
Usage: mmap-test file
```

Simple mmap tests

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="path-test">

#### path-test

```
Usage: path-test file
```

Test path IO

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="prevtest">

#### prevtest

```
Usage: prevtest file
```

Test access to preview images

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="remotetest">

#### remotetest

```
Usage: remotetest file {--nocurl | --curl}
```

Tester application for testing remote i/o.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="stringto-test">

#### stringto-test

```
Usage: stringto-test
```

Test conversions from string to long, float and Rational types.

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="taglist">

#### taglist

```
$ taglist --help
Usage: taglist [--help]
           [--group name|
            Groups|Exif|Canon|CanonCs|CanonSi|CanonCf|Fujifilm|Minolta|Nikon1|Nikon2|Nikon3|Olympus|
            Panasonic|Pentax|Sigma|Sony|Iptc|
            dc|xmp|xmpRights|xmpMM|xmpBJ|xmpTPg|xmpDM|pdf|photoshop|crs|tiff|exif|aux|iptc|all|ALL
           ]
Print Exif tags, MakerNote tags, or Iptc datasets
```

Print a simple comma separated list of tags defined in Exiv2

This program encodes the library's tag definitions in ascii.
The data from this program is formatted as HTML on the web-site.  https://exiv2.org/metadata.html

For example, to show the binary definition of Group `Nikon3`:

```
$ taglist Nikon3
Version,    1,  0x0001, Nikon3, Exif.Nikon3.Version,    Undefined,  Nikon Makernote version
ISOSpeed,   2,  0x0002, Nikon3, Exif.Nikon3.ISOSpeed,   Short,  ISO speed setting
ColorMode,  3,  0x0003, Nikon3, Exif.Nikon3.ColorMode,  Ascii,  Color mode
Quality,    4,  0x0004, Nikon3, Exif.Nikon3.Quality,    Ascii,  Image quality setting
WhiteBalance,   5,  0x0005, Nikon3, Exif.Nikon3.WhiteBalance,   Ascii,  White balance
Sharpening, 6,  0x0006, Nikon3, Exif.Nikon3.Sharpening, Ascii,  Image sharpening setting
Focus,  7,  0x0007, Nikon3, Exif.Nikon3.Focus,  Ascii,  Focus mode
FlashSetting,   8,  0x0008, Nikon3, Exif.Nikon3.FlashSetting,   Ascii,  Flash setting
FlashDevice,    9,  0x0009, Nikon3, Exif.Nikon3.FlashDevice,    Ascii,  Flash device
...
```

We can see those tags being used:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --print a --grep Nikon3 Stonehenge.jpg
Exif.Nikon3.Version                          Undefined   4  2.11
Exif.Nikon3.ISOSpeed                         Short       2  200
...
```

This information is formatted (search Nikon (format 3) MakerNote Tags): [https://exiv2.org/tags-nikon.html](https://exiv2.org/tags-nikon.html)

#### taglist all

These options are provided to list every tag known to Exiv2.  The option `all` prints the [Group.Tagnames](exiv2.md#exiv2_key_syntax) for every Exif tag.  The option `ALL` prints the [Group.Tagnames](exiv2.md#exiv2_key_syntax) for every Exif tag, followed by the TagInfo for that tag.  For example:

```bash
$ taglist all | grep ISOSpeed$
Photo.ISOSpeed
PanasonicRaw.ISOSpeed
CanonCs.ISOSpeed
CanonSi.ISOSpeed
Casio2.ISOSpeed
MinoltaCs5D.ISOSpeed
MinoltaCs7D.ISOSpeed
Nikon1.ISOSpeed
Nikon2.ISOSpeed
Nikon3.ISOSpeed
Olympus.ISOSpeed
Olympus2.ISOSpeed
```

```bash
$ taglist ALL | grep ISOSpeed,
Photo.ISOSpeed,	34867,	0x8833,	Photo,	Exif.Photo.ISOSpeed,	Long,	This tag indicates the ISO speed value of a camera or input device that is defined in ISO 12232. When recording this tag, the PhotographicSensitivity and SensitivityType tags shall also be recorded.
PanasonicRaw.ISOSpeed,	23,	0x0017,	PanasonicRaw,	Exif.PanasonicRaw.ISOSpeed,	Short,	ISO speed setting
CanonCs.ISOSpeed,	16,	0x0010,	CanonCs,	Exif.CanonCs.ISOSpeed,	SShort,	ISO speed setting
CanonSi.ISOSpeed,	2,	0x0002,	CanonSi,	Exif.CanonSi.ISOSpeed,	Short,	ISO speed used
Casio2.ISOSpeed,	20,	0x0014,	Casio2,	Exif.Casio2.ISOSpeed,	Short,	ISO Speed
MinoltaCs5D.ISOSpeed,	38,	0x0026,	MinoltaCs5D,	Exif.MinoltaCs5D.ISOSpeed,	Short,	ISO speed setting
MinoltaCs7D.ISOSpeed,	28,	0x001c,	MinoltaCs7D,	Exif.MinoltaCs7D.ISOSpeed,	Short,	ISO speed setting
Nikon1.ISOSpeed,	2,	0x0002,	Nikon1,	Exif.Nikon1.ISOSpeed,	Short,	ISO speed setting
Nikon2.ISOSpeed,	6,	0x0006,	Nikon2,	Exif.Nikon2.ISOSpeed,	Short,	ISO speed setting
Nikon3.ISOSpeed,	2,	0x0002,	Nikon3,	Exif.Nikon3.ISOSpeed,	Short,	ISO speed setting
Olympus.ISOSpeed,	4097,	0x1001,	Olympus,	Exif.Olympus.ISOSpeed,	SRational,	ISO speed value
Olympus2.ISOSpeed,	4097,	0x1001,	Olympus,	Exif.Olympus.ISOSpeed,	SRational,	ISO speed value
Sony1MltCs7D.ISOSpeed,	28,	0x001c,	MinoltaCs7D,	Exif.MinoltaCs7D.ISOSpeed,	Short,	ISO speed setting
```



[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="tiff-test">

#### tiff-test

```
Usage: tiff-test file
```

Simple TIFF write test

[Sample](#TOC1) Programs [Test](#TOC2) Programs


<div id="write-test">

#### write-test

```
Usage: write-test file case
where case is an integer between 1 and 11
```

ExifData write unit tests

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="write2-test">

#### write2-test

```
Usage: write2-test file
```

ExifData write unit tests for Exif data created from scratch

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="xmpdump">

#### xmpdump

```
Usage: xmpdump file
```

Sample program to dump the XMP packet of an image

[Sample](#TOC1) Programs [Test](#TOC2) Programs

<div id="xmpparser-test">

#### xmpparser-test

```
Usage: xmpparser-test file
```

Read an XMP packet from a file, parse and re-serialize it.

[Sample](#TOC1) Programs [Test](#TOC2) Programs



Robin Mills<br>
robin@clanmills.com<br>
Revised: 2021-09-21
