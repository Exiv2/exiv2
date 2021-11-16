<div id="exiv2">

# EXIV2(1)

<div id="name">

# NAME
exiv2 - Image metadata manipulation tool

<div id="synopsis">

# 1 SYNOPSIS
**exiv2** [ **option** [ *arg* ] ]+ [ *action* ] *file* ...

<div id="description">

# 2 DESCRIPTION
**exiv2** is a program to read and write image metadata, including 
Exif, IPTC, XMP, image comments, ICC Profile, thumbnails, image 
previews and many vendor makernote tags. The program optionally 
converts between Exif, IPTC and XMP tags, as recommended by their 
respective standards/specifications and the Metadata Working Group 
guidelines.

<div id="TOC">

### TABLE OF CONTENTS
1.  [SYNOPSIS](#synopsis)
2.  [DESCRIPTION](#description)
3.  [FILE TYPES](#file_types)
4.  [ACTIONS](#actions)
5.  [COMMAND SUMMARY](#cmd_summary)
6.  [OPTIONS](#options)
7.  [EXIV2 GROUPS, TYPES AND VALUES](#groups_types_values)
     1.  [Exiv2 key syntax](#exiv2_key_syntax)
     2.  [Exiv2 tags](#exiv2_tags)
     3.  [Exif/IPTC/XMP types](#exiv2_types)
     4.  [Multiple elements](#multi_elements)
     5.  [Duplicate tags](#multi_tags)
     6.  [Date/Time formats](#date_time_fmts)
     7.  [Exif 'Comment' values](#exif_comment_values)
     8.  [XMP namespaces](#xmp_namespaces)
     9.  [XMP LangAlt values](#langalt_values)
     10. [XMP structs](#xmp_structs)
8.  [PREVIEW IMAGES AND THUMBNAILS](#preview_images)
9.  [ICC PROFILES](#icc_profiles)
10. [IMAGE COMMENTS](#image_comments)
11. ['MODIFY' COMMANDS](#modify_cmds)
     1.  [Quotations with 'modify' commands](#quotes_modify)
     2.  ['Modify' command format](#mod_cmd_format)
     3.  [Modifying a value](#mod_value)
     4.  ['Modify' examples](#mod_examples)
     5.  ['Modify' command file](#mod_cmd_file)
12. [CONFIGURATION FILE](#config_file)
13. [EXAMPLES](#examples)
14. [RETURN VALUE](#return_value)
15. [ENVIRONMENT](#environment)
16. [NOTES](#notes)
17. [BUGS](#bugs)
18. [COPYRIGHT](#copyright)
19. [AUTHORS](#authors)
20. [SEE ALSO](#see_also)

<div id="file_types">

# 3 FILE TYPES

The following image formats and metadata categories are supported:<br>

Type | Exif       | IPTC       | XMP        | Image Comments | ICC Profile | Thumbnail
|:---|:----       |:----       |:----       |:----           |:----        |:---- 
ARW  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
AVIF | Read       | Read       | Read       | -              | -           | Read
BMP  | -          | -          | -          | -              | -           | -
CR2  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
CR3  | Read       | Read       | Read       | -              | -           | Read
CRW  | Read/Write | -          | -          | Read/Write     | -           | Read/Write
DNG  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
EPS  | -          | -          | Read/Write |                | -           | -
EXV  | Read/Write | Read/Write | Read/Write | Read/Write     | Read/Write  | Read/Write
GIF  | -          | -          | -          | -              | -           | -
HEIC | Read       | Read       | Read       | -              | -           | Read
HEIF | Read       | Read       | Read       | -              | -           | Read
JP2  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
JPEG | Read/Write | Read/Write | Read/Write | Read/Write     | Read/Write  | Read/Write
JXL  | Read       | Read       | Read       | -              | -           | Read
MRW  | Read       | Read       | Read       | -              | -           | Read
NEF  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
ORF  | Read/Write | Read/Write | Read/Write | -              | -           | Read/Write
PEF  | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
PGF  | Read/Write | Read/Write | Read/Write | Read/Write     | Read/Write  | -
PNG  | -          | Read/Write | Read/Write | -              | Read/Write  | Read/Write
PSD  | Read/Write | Read/Write | Read/Write | -              | -           | Read/Write
RAF  | Read       | Read       | Read       | -              | -           | Read
RW2  | Read       | Read       | Read       | -              | -           | Read
SR2  | Read       | Read       | Read       | -              | -           | Read
SRW  | Read/Write | Read/Write | Read/Write | -              | -           | Read/Write
TGA  | -          | -          | -          | -              | -           | -
TIFF | Read/Write | Read/Write | Read/Write | -              | Read/Write  | Read/Write
WEBP | Read/Write | -          | Read/Write | -              | Read/Write  | Read/Write
XMP  | -          |-           | Read/Write | -              | -           | -

- Support for GIF, TGA and BMP images is minimal: the image format is
recognized, a MIME type assigned to it and the height and width of the
image are determined.

- Reading other TIFF-like RAW image formats, which are not listed in 
the table, may also work.

- Some image formats allow an extra interal type of metadata. Only 
partial support exists for the RAF format.

- Support for BMFF types such as AVIF, CR3, HEIF and HEIC is a build 
option. To check if this is enabled, use `exiv2 --version --verbose --grep bmff` 
and see if `enable_bmff=1`.

- Naked codestream JXL files do not contain Exif, IPTC or XMP metadata.


[TOC](#TOC)

<div id="actions">

# 4 ACTIONS
The *action* argument is only required if it is not clear from the
*options* which action is implied.

<div id="pr_print">

### pr | print
Print image metadata, the default is [--print s](#print_mod). This is also 
the default action (i.e., `exiv2 image.jpg`).

<div id="ex_extract">

### ex | extract
Extract metadata to "raw" metadata (\*.exv), XMP sidecar (\*.xmp), 
preview image, thumbnail or ICC profile file. The default is [--extract XXei](#extract_tgt3). 
Use [--location dir](#location_dir) to direct the output to a different 
directory. Modification commands can be applied on-the-fly.

<div id="in_insert">

### in | insert
Insert metadata from corresponding "raw" metadata (\*.exv), XMP sidecar (\*.xmp), 
thumbnail or ICC profile files, the default is [--insert XXeix](#insert_tgt2). 
Use [--location dir](#location_dir) to direct the input from a different 
directory and [--suffix suf](#suffix_suf) for the output extension. Since 
files of any supported format can be used as input files, this command 
can be used to copy the metadata between files of different formats. 

<div id="rm_delete">

### rm | delete
Delete image metadata from the files, the default is [--delete a](#delete_tgt1).

<div id="ad_adjust">

### ad | adjust
Adjust Exif timestamps by the given time. Requires at least one of the
options [--adjust time](#adjust_time), [--years +-n](#years_n), 
[--months +-n](#months_n) or [--days +-n](#days_n). See [TZ environment variable](#TZ).

<div id="mo_modify">

### mo | modify
Apply commands to modify the Exif, IPTC and XMP metadata of image files. 
Requires option [--comment txt](#comment_txt), [--modify cmdfile](#modify_cmdfile) 
or [--Modify cmd](#Modify_cmd).

<div id="mv_rename">

### mv | rename
Rename files and/or set file timestamps according to the Exif create
timestamp. The default filename format is in [--rename fmt](#rename_fmt). 
Uses the value of [Exif.Photo.DateTimeOriginal](https://www.exiv2.org/tags.html) 
or, [Exif.Image.DateTime](https://www.exiv2.org/tags.html) to determine 
the timestamp. The filename format can be set with [--rename fmt](#rename_fmt), 
timestamp options are [--timestamp](#timestamp) and [--Timestamp](#Timestamp). 
See [TZ environment variable](#TZ).

<div id="fi_fixiso">

### fi | fixiso
Copy the ISO setting from one of the proprietary Nikon or Canon
makernote ISO tags to the regular Exif ISO tag,
[Exif.Photo.ISOSpeedRatings](https://www.exiv2.org/tags.html). Does not 
overwrite an existing Exif ISO tag.

<div id="fc_fixcom">

### fc | fixcom
Fix the character encoding of Exif Unicode user comments. Decodes the
comment using the auto-detected or specified character encoding and
writes it back in UCS-2. Use option [--encode enc](#encode_enc) to 
specify the current encoding of the comment if necessary.

[TOC](#TOC)

<div id="cmd_summary">

# 5 COMMAND SUMMARY

**exiv2** [ **option** [ *arg* ] ]+ [ *action* ] *file* ...<br>

Where *file* is one or more files containing image metadata. These can 
optionally be specified using a URL (http, https, ftp, sftp, data 
and file supported) or a wildcard pattern (e.g., *image1.tiff image2.jpg*, 
*https://clanmills.com/Stonehenge.jpg* or *\*.jpg*)

| **Option** *arg* | **Long option** *arg*  | Description                                                               |
|:------           |:----                   |:----                                                                      |
| **-a** *time*    | **--adjust** *time*    | Automatically modify metadata time stamps. For the [adjust](#ad_adjust) action [[...]](#adjust_time) |
| **-b**           | **--binary**           | Obsolete and should not be used. Reserved for use with the test suite     |
| **-c** *txt*     | **--comment** *txt*    | JPEG comment string to set in the image. For the [modify](#mo_modify) action [[...]](#comment_txt) |
| **-d** *tgt1*    | **--delete** *tgt1*    | Delete target(s) for the [delete](#rm_delete) action [[...]](#delete_tgt1) |
| **-D** *+-n*     | **--days** *+-n*       | Automated adjustment of the days in metadata dates [[...]](#days_n)       |
| **-e** *tgt3*    | **--extract** *tgt3*   | Extract target(s) for the [extract](#ex_extract) action [[...]](#extract_tgt3) |
| **-f**           | **--force**            | Do not prompt before overwriting existing files. For the [rename](#mv_rename) and [extract](#ex_extract) actions [[...]](#force_Force) |
| **-F**           | **--Force**            | Do not prompt before renaming files. For the [rename](#mv_rename) and [extract](#ex_extract) actions [[...]](#force_Force) |
| **-g** *str*     | **--grep** *str*       | Only output where *str* matches in output text [[...]](#grep_str)         |
| **-h**           | **--help**             | Display help and exit [[...]](#help)                                      |
| **-i** *tgt2*    | **--insert** *tgt2*    | Insert target(s) for the [insert](#in_insert) action [[...]](#insert_tgt2) |
| **-k**           | **--keep**             | Preserve file timestamps when updating files [[...]](#keep)               |
| **-K** *key*     | **--key** *key*        | Report a key. Similar to [--grep str](#grep_str), however *key* must match exactly [[...]](#key_key) |
| **-l** *dir*     | **--location** *dir*   | Location (directory) for files to be inserted or extracted [[...]](#location_dir) |
| **-m** *cmdfile* | **--modify** *cmdfile* | Read commands from a file. For the [modify](#mo_modify) action [[...]](#modify_cmdfile) |
| **-M** *cmd*     | **--Modify** *cmd*     | Modify the metadata with the command. For the [modify](#mo_modify) action [[...]](#Modify_cmd) |
| **-n** *enc*     | **--encode** *enc*     | Charset to decode Exif Unicode user comments [[...]](#encode_enc)         |
| **-O** *+-n*     | **--months** *+-n*     | Automated adjustment of the months in metadata dates [[...]](#months_n)   |
| **-p** *mod*     | **--print** *mod*      | Print report (common reports) [[...]](#print_mod)                         |
| **-P** *flg*     | **--Print** *flg*      | Print report (fine grained control) [[...]](#Print_flgs)                  |
| **-q**           | **--quiet**            | Silence warnings and error messages [[...]](#quiet)                       |
| **-Q** *lvl*     | **--log** *lvl*        | Set the log-level [[...]](#log_lvl)                                       |
| **-r** *fmt*     | **--rename** *fmt*     | Filename format for the [rename](#mv_rename) action [[...]](#rename_fmt)  |
| **-S** *suf*     | **--suffix** *suf*     | Use suffix for source files when using the [insert](#in_insert) action [[...]](#suffix_suf) |
| **-t**           | **--timestamp**        | Set the file timestamp from Exif metadata. For the [rename](#mv_rename) action [[...]](#timestamp) |
| **-T**           | **--Timestamp**        | Only set the file timestamp from Exif metadata. For the [rename](#mv_rename) action [[...]](#Timestamp) |
| **-u**           | **--unknown**          | Show unknown tags [[...]](#unknown)                                       |
| **-v**           | **--verbose**          | Verbose [[...]](#verbose)                                                 |
| **-V**           | **--version**          | Show the program version and exit [[...]](#version)                       |
| **-Y** *+-n*     | **--years** *+-n*      | Automated adjustment of the years in metadata dates [[...]](#years_n)     |

<div id="cmd_summary_flgs">

The arguments for those options are:

| *arg*     | Description                                                                |
|:------    |:----                                                                       |
| *action*  | pr \| ex \| in \| rm \| ad \| mo \| mv \| fi \| fc<br>(print, extract, insert, delete, adjust, modify, rename, fixiso, fixcom) |
| *cmd*     | (**set** \| **add**) *key* [ [*type*] *value* ] \| **del** *key* [*type*] \| **reg** *prefix* *namespace*<br>(see ['Modify' command format](#mod_cmd_format)) |
| *enc*     | Values defined in [iconv_open(3)](https://linux.die.net/man/3/iconv_open) (e.g., UTF-8) |
| *flg*     | E \| I \| X \| x \| g \| k \| l \| n \| y \| c \| s \| v \| t \| h<br>(Exif, IPTC, XMP, num, grp, key, label, name, type, count, size, vanilla, translated, hex) |
| *fmt*     | Default format: %Y%m%d_%H%M%S                                              |
| *key*     | See [Exiv2 key syntax](#exiv2_key_syntax)                                  |
| *lvl*     | d \| i \| w \| e \| m<br>(debug, info, warning, error, mute)               |
| *mod*     | s \| a \| e \| t \| v \| h \| i \| x \| c \| p \| C \| R \| S \| X<br>(summary, all, Exif, translated, vanilla, hex, IPTC, XMP, comment, preview, ICC Profile, Recursive Structure, Simple Structure, raw XMP) |
| *suf*     | '.' then the file's extension (e.g., '.txt')                               |
| *time*    | [+\|-]HH[:MM[:SS]]<br>(Default is **+** when **+**/**-** are missing) |
| *tgt1*    | a \| c \| e \| i \| I \| t \| x \| C \| -<br>(all, comment, Exif, IPTC, IPTC all, thumbnail, XMP, ICC Profile, stdin/out) |
| *tgt2*    | a \| c \| e \| i \| t \| x \| C \| X \| XX \| -<br>(all, comment, Exif, IPTC, thumbnail, XMP, ICC Profile, SideCar, Raw metadata, stdin/out) |
| *tgt3*    | a \| e \| i \| p \| t \| x \| C \| X \| XX \| -<br>(all, Exif, IPTC, preview, thumbnail, XMP, ICC Profile, SideCar, Raw metadata, stdin/out) |
| *type*    | An Exif, IPTC or XMP tag type (e.g., xmpText). See [Exif/IPTC/XMP types](#exiv2_types) |
| *+-n*     | The amount to change in the date (e.g., -3). Default is **+** when **+**/**-** are missing |

[TOC](#TOC)

<div id="options">

# 6 OPTIONS

<div id="help">

### **-h**, **--help**
Display help and exit.

<div id="version">

### **-V**, **--version**
Show the program version and exit.

When **--version** is combined with [--verbose](#verbose), build 
information is printed to standard output along with a list of shared 
libraries which have been loaded into memory. Verbose version is 
supported on Windows (MSVC, Cygwin and MinGW builds), macOS and Linux 
and is provided for test and debugging. The library name and version 
number are always printed, even if output is filtered with 
[--grep str](#grep_str).

<div id="verbose">

### **-v**, **--verbose**
Be verbose during the program run.

<div id="quiet">

### **-q**, **--quiet**
Silence warnings and error messages during the program run. Note that 
options **--quiet** and [--verbose](#verbose) can be used at the same 
time.

<div id="log_lvl">

### **-Q** *lvl*, **--log** *lvl*
Set the log-level to 'd'(ebug), 'i'(nfo), 'w'(arning), 'e'(rror)
or 'm'(ute), with the *lvl* chosen including those below it 
('d' \<- 'i' \<- 'w' \<- 'e'). The default log-level is 'w'. **--log** *m* 
is equivalent to [--quiet](#quiet). All log messages are written to 
standard error.

<div id="unknown">

### **-u**, **--unknown**
Show unknown tags. Default is to suppress tags which don't have a name 
(e.g., Exif.SonyMisc3c.0x022b).

<div id="grep_str">

### **-g** *str*, **--grep** *str*
When printing tags, display only those where *str* is found in 
the key (see [Exiv2 key syntax](#exiv2_key_syntax)). When 
**--grep** *str* is used with [--verbose](#verbose) 
[--version](#version), lines are included where *str* matches in the 
variable or value.

Multiple **--grep** *str* options can be used to output additional data:

```
$ exiv2 --verbose --version --grep webready --grep time
exiv2 1.0.0.9
time=11:01:53
enable_webready=1
xmlns=mediapro:http://ns.iview-multimedia.com/mediapro/1.0/
```

When the [--print mod](#print_mod) and [--Print flgs](#Print_flgs) 
options are not specified, the default is [--print a](#print_mod):

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --grep Date Stonehenge.jpg
Exif.Image.DateTime                          Ascii      20  2015:07:16 20:25:28
Exif.Photo.DateTimeOriginal                  Ascii      20  2015:07:16 15:38:54
Exif.Photo.DateTimeDigitized                 Ascii      20  2015:07:16 15:38:54
Exif.NikonWt.DateDisplayFormat               Byte        1  Y/M/D
Exif.GPSInfo.GPSDateStamp                    Ascii      11  2015:07:16
Xmp.xmp.ModifyDate                           XmpText    25  2015-07-16T20:25:28+01:00
```

You may use [--print mod](#print_mod) or [--Print flgs](#Print_flgs) to 
further filter output ([--print s](#print_mod) is ignored):

```
$ exiv2 --print x --grep Date Stonehenge.jpg
Xmp.xmp.ModifyDate                           XmpText    25  2015-07-16T20:25:28+01:00
```	

*str* can contain an optional */i* modifier at the end, to indicate case 
insensitivity:

```
$ exiv2 --print px --grep date/i Stonehenge.jpg
Xmp.xmp.ModifyDate                           XmpText    25  2015-07-16T20:25:28+01:00
```

<div id="key_key">

### **-K** *key*, **--key** *key*
Only reports tags for a given *key*, which must match the key exactly 
(See [Exiv2 key syntax](#exiv2_key_syntax)).

Multiple **--key** *key* options can be used to report more than a 
single key:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --key Exif.Photo.DateTimeDigitized --key Exif.Photo.DateTimeOriginal Stonehenge.jpg
Exif.Photo.DateTimeOriginal                  Ascii      20  2015:07:16 15:38:54
Exif.Photo.DateTimeDigitized                 Ascii      20  2015:07:16 15:38:54
```

<div id="encode_enc">

### **-n** *enc*, **--encode** *enc*
Charset to use when decoding Exif Unicode user comments, where *enc* is
a name understood by [iconv_open(3)](https://linux.die.net/man/3/iconv_open) 
(e.g., 'UTF-8'). See [Exif 'Comment' values](#exif_comment_values).

<div id="keep">

### **-k**, **--keep**
Preserve file timestamps when updating files. Can be used with
all options which update files and is ignored by read-only
options.

<div id="timestamp">

### **-t**, **--timestamp**
Set the file timestamp according to the Exif create timestamp in
addition to renaming the file (overrides [--keep](#keep)). This option is
only used with the [rename](#mv_rename) action. See [TZ](#TZ) environment 
variable.

<div id="Timestamp">

### **-T**, **--Timestamp**
Only set the file timestamp according to the Exif create timestamp, do
not rename the file (overrides [--keep](#keep)). This option is only used
with the [rename](#mv_rename) action. See [TZ environment variable](#TZ).

<div id="force_Force">

### **-f**, **--force** or **-F**, **--Force**
These options are used by the [rename](#mv_rename) and 
[extract](#ex_extract) actions to determine the file overwrite policy. 
The options override the default behavior, which is to prompt the user if 
the filename already exists. These options are usually combined with 
[--verbose](#verbose), to provide additional status output.

The [rename](#mv_rename) action will overwrite files when **--force** is 
used. Instead, if **--Force** is used and the file already exists, the 
new filename is appended with '_1' ('_2', ...) to prevent data loss.

The [extract](#ex_extract) action will overwrite files when either 
**--force** or **--Force** is used.

For example, renaming a file using **--Force**, where the same filename 
has already been renamed:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --verbose --Force rename Stonehenge.jpg
File 1/1: Stonehenge.jpg
Renaming file to ./20150716_153854.jpg

$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --verbose --Force rename Stonehenge.jpg
File 1/1: Stonehenge.jpg
Renaming file to ./20150716_153854_1.jpg
```

<div id="rename_fmt">

### **-r** *fmt*, **--rename** *fmt*
Filename format for the [rename](#mv_rename) action (See [TZ](#TZ) 
environment variable). The *fmt* string follows the definitions in 
[strftime(3)](https://linux.die.net/man/3/strftime), using 
[Exif.Photo.DateTimeOriginal](https://www.exiv2.org/tags.html) or 
[Exif.Image.DateTime](https://www.exiv2.org/tags.html) as the reference 
date and time. In addition, the following special character sequences are 
also provided:

| Variable       | Description                                     |
|:------         |:----                                            |
| :basename:     | Original filename without extension             |
| :dirname:      | Name of the directory holding the original file |
| :parentname:   | Name of parent directory                        |

The default *fmt* is %Y%m%d_%H%M%S

For example, renaming a file when *fmt* is made up of the basename, day, 
short month and long year:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --verbose --rename ':basename:_%d_%b_%Y' Stonehenge.jpg
File 1/1: Stonehenge.jpg
Renaming file to ./Stonehenge_16_Jul_2015.jpg
```

If the filename to rename to already exists and [--Force](#force_Force) 
and [--force](#force_Force) are not used, the user is prompted for an 
action.

| Option      | Description                                                       |
|:------      |:----                                                              |
| [O]verwrite | Rename the file, overriding the existing one                      |
| [r]ename    | Rename the file, but append '_1' ('_2', ...) to the new filename  |
| [s]kip  :   | Cancel renaming this file                                         |

For example, when renaming a second Stonehenge.jpg file:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --verbose --rename ':basename:_%d_%b_%Y' Stonehenge.jpg
File 1/1: Stonehenge.jpg
exiv2.exe: File `./Stonehenge_16_Jul_2015.jpg' exists. [O]verwrite, [r]ename or [s]kip? r
Renaming file to ./Stonehenge_16_Jul_2015_1.jpg
```

<div id="adjust_time">

### **-a** *time*, **--adjust** *time*
Adjusts the times in standard Exif tags, where *time* is in the format: 
[+\|-]HH[:MM[:SS]]. This option is only used with the [adjust](#ad_adjust) 
action. See [TZ](#TZ) environment variable.

Examples of *time* are:

| Time     | Description                  |
|:------   |:----                         |
| +1       | Adds one hour                |
| 1:01     | Adds one hour and one minute |
| -0:00:30 | Subtracts 30 seconds         |

<div id="years_n">

### **-Y** *+-n*, **--years** *+-n*
Time adjustment by a positive or negative number of years, for
the [adjust](#ad_adjust) action. See [TZ environment variable](#TZ).

<div id="months_n">

### **-O** *+-n*, **--months** *+-n*
Time adjustment by a positive or negative number of months, for
the [adjust](#ad_adjust) action. See [TZ environment variable](#TZ).

<div id="days_n">

### **-D** *+-n*, **--days** *+-n*
Time adjustment by a positive or negative number of days, for
the [adjust](#ad_adjust) action. See [TZ environment variable](#TZ).

<div id="print_mod">

### **-p** *mod*, **--print** *mod* 
Print mode for the [print](#pr_print) action (see 
[DESCRIPTION](#file_types), for metadata support in a file type). 
Where *mod* is one of:

| Option | Description                                                                          |
|:------ |:----                                                                                 |
| s      | A summary of the Exif metadata (the default for the [print](#pr_print) action)       |
| a      | Exif, IPTC and XMP tags (shortcut for [--Print kyct](#Print_flgs))                   |
| e      | Plain (untranslated) Exif tags (shortcut for [--Print Ekycv](#Print_flgs))           |
| t      | Interpreted (translated) Exif tag values (shortcut for [--Print Ekyct](#Print_flgs)) |
| v      | Plain (untranslated) Exif tag values (shortcut for [--Print Exgnycv](#Print_flgs))   |
| h      | Hex dump of the Exif data (shortcut for [--Print Exgnycsh](#Print_flgs))             |
| i      | Translated IPTC tags (shortcut for [--Print Ikyct](#Print_flgs))                     |
| x      | Translated XMP tags (shortcut for [--Print Xkyct](#Print_flgs))                      |
| c      | JPEG comment (see [IMAGE COMMENTS](#image_comments))                                 |
| p      | List available image previews, sorted by size in pixels (see [PREVIEW IMAGES AND THUMBNAILS](#preview_images)) |
| C      | Image ICC Profile (see [ICC PROFILES](#icc_profiles))                                |
| R      | Print image structure recursively (only for the 'debug' build with jpg, png, tiff, webp, cr2 and jp2 types) |
| S      | Image structure information (jpg, png, tiff, webp, cr2 and jp2 types only)           |
| X      | "raw" XMP                                                                            |

**--print** *mod* can be combined with [--grep str](#grep_str) or 
[--key key](#key_key) to further filter the output.

For the order of the columns in the output, see 
[--Print flgs](#Print_flgs_order).

<div id="Print_flgs">

### **-P** *flgs*, **-Print** *flgs*
Print flags allows fine control of the tag list for the 
[print](#pr_print) action (see [DESCRIPTION](#file_types), for metadata 
support in a file type). The option allows control of the type of metadata 
as well as data columns included in the print output. Valid flags are:

| Option | Description                                                                 |
|:------ |:----                                                                        |
| E      | Exif tags                                                                   |
| I      | IPTC tags                                                                   |
| X      | XMP tags                                                                    |
| x      | Tag number for Exif or IPTC tags (in hexadecimal)                           |
| g      | Group name (e.g., for Exif.Photo.UserComment, outputs Photo)                |
| k      | Key (e.g., Exif.Photo.UserComment)                                          |
| l      | Tag label (human-readable tagname, e.g., for Exif.Photo.UserComment, outputs 'User comment') |
| n      | Tagname (e.g., for Exif.Photo.UserComment, outputs UserComment)             |
| y      | Type (for available types, see [Exif/IPTC/XMP types](#exiv2_types))         |
| c      | Number of components (for single entry types, the number of **sizeof('type')** in 'size'. For multi-entry types, the number of entries. See [Exif/IPTC/XMP types](#exiv2_types)) |
| s      | Size in bytes of vanilla output (see note in [Exif 'Comment' values](#exif_comment_values)). Some types include a *NULL* character in the size (see [Exif/IPTC/XMP types](#exiv2_types)) |
| v      | Plain data value (vanilla values, i.e., untranslated)                       |
| V      | Plain data value and the word 'set ' (see ['MODIFY' COMMANDS](#modify_cmds))|
| t      | Interpreted (translated) human-readable data values (includes plain vanilla values) |
| h      | Hex dump of the data                                                        |

<div id="Print_flgs_order">

The order of the values in *flgs* is not respected. For example, the order 
of the columns, using some tags from *Stonehenge.jpg*, is as follows:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Print xgknlycst Stonehenge.jpg
```

| Tag number<br>(x) | Plain 'set'<br>(V) | Group<br>(g) | Key<br>(k)                 | Tagname<br>(n) | Tagname label<br>(l) | Type<br>(y) | Comp<br>(c) | Size<br>(s) | Value<br>(E, I, X, v, t) | Translated<br>(t) |
|:------            |:----               |:------       |:------                     |:------         |:------               |:------      |:------      |:------      |:------                   |:------           |
| 0x0110            | set                | Image        | Exif.Image.Model           | Model          | Model                | Ascii       | 12          | 12          | NIKON D5300              | NIKON D5300      |
| 0x0006            | set                | NikonIi      | Exif.NikonIi.ISO2          | ISO2           | ISO 2                | Byte        | 1           | 1           | 72                       | 200              |
| 0x0000            | set                | xmp          | Xmp.xmp.Rating             | Rating         | Rating               | XmpText     | 1           | 1           | 0                        | 0                |
| 0x0000            | set                | dc           | Xmp.dc.Family              | Family         | Family               | XmpBag      | 1           | 5           | Robin                    | Robin            |

**--Print** *flgs* can be combined with [--grep str](#grep_str) or 
[--key key](#key_key) to further filter the output.

<div id="delete_tgt1">

### **-d** *tgt1**, **--delete** *tgt1*
Delete target(s) for the [delete](#rm_delete) action. Possible targets are:

| Option | Description                                                              |
|:------ |:----                                                                     |
| a      | All supported metadata (the default for the [delete](#rm_delete) action) |
| e      | Exif tags                                                                |
| t      | Exif thumbnail only (see [PREVIEW IMAGES AND THUMBNAILS](#preview_images)) |
| i      | IPTC tags                                                                |
| x      | XMP tags                                                                 |
| c      | JPEG comment (see [IMAGE COMMENTS](#image_comments))                     |
| C      | ICC Profile (see [ICC PROFILES](#icc_profiles))                          |
| I      | All IPTC data (removes broken metadata containing multiple IPTC blocks)  |
| -      | Insert from stdin. This option is intended for "filter" operations       |

<div id="insert_tgt2">

### **-i** *tgt2*, **--insert** *tgt2*
Insert target(s) for the [insert](#in_insert) action (see 
[DESCRIPTION](#file_types), for metadata support in a file type). Possible 
targets are:

| Option | Description                                                              |
|:------ |:----                                                                     |
| a      | All supported metadata (the default for the [insert](#in_insert) action) |
| e      | Exif tags                                                                |
| t      | Exif thumbnail only. Only JPEG thumbnails can be inserted from a file called *\<file\>-thumb.jpg* (see [PREVIEW IMAGES AND THUMBNAILS](#preview_images)) |
| i      | IPTC tags                                                                |
| x      | XMP tags                                                                 |
| c      | JPEG comment (see [IMAGE COMMENTS](#image_comments))                     |
| C      | ICC Profile, inserted from a file called *\<file\>.icc* (see [ICC PROFILES](#icc_profiles)) |
| X      | Insert metadata from an XMP sidecar file, *\<file\>.xmp*. The remaining targets determine which metadata to include, possible are Exif, IPTC and XMP (default is all of these). Inserted XMP tags include those converted to Exif and IPTC |
| XX     | Insert "raw" metadata from a sidecar file, *\<file\>.exv*                |
| -      | Insert from stdin. This option is intended for "filter" operations       |

To insert from a location other than the current directory, use [--location dir](#location_dir). 

Filter operations between [--extract tgt3](#extract_tgt3) and 
**--insert** *tgt2* are possible, using the following template; 
[xmllint(1)](https://linux.die.net/man/1/xmllint) can be optionally used 
when extracting XMP sidecar files:

```
$ exiv2 -e{tgt3}- filename | [ xmllint .... |] exiv2 -i{tgt2}- filename
```

<div id="extract_tgt3">

###  **-e** *tgt3*, **--extract** *tgt3*
Extract target(s) for the [extract](#ex_extract) action. Possible targets are:

| <div style="width:100px">Option</div> | Description |
|:------           |:----                                                                       |
| a                | All supported metadata (the default for the [extract](#ex_extract) action) |
| e                | Exif tags                                                                  |
| t                | Extract Exif thumbnail only to a file called *\<file\>-thumb.jpg* (see [PREVIEW IMAGES AND THUMBNAILS](#preview_images)) |
| i                | IPTC tags                                                                  |
| x                | XMP tags                                                                   |
| c                | JPEG comment (see [IMAGE COMMENTS](#image_comments))                       |
| p\[\<n\>\[,\<m\> ...\]\] | Extract preview images to files called *\<file\>-preview\<n \| m \| ...\>.\<ext\>* (see [PREVIEW IMAGES AND THUMBNAILS](#preview_images))  |
| C                | Extract ICC profile, to a file called *\<file\>.icc* (see [ICC PROFILES](#icc_profiles)) |
| X                | Extract metadata to an XMP sidecar file, \<file\>.xmp. Other targets cannot be used with this, as only XMP data is written. Extracted XMP tags include those converted from Exif and IPTC |
| XX               | Extract "raw" metadata to a sidecar file, \<file\>.exv. The remaining targets determine which metadata to include, possible are Exif and IPTC (XMP is always included) |
| -                | Output to stdout (see [--insert tgt2](#insert_tgt2) for an example of this feature). This argument ignores [--verbose](#verbose) |

To extract to a location other than the current directory, use [--location dir](#location_dir). 

When extracting, the [--Modify cmd](#Modify_cmd) and [--modify cmdfile](#modify_cmdfile) 
options can be used to filter the output (see ['MODIFY' COMMANDS](#modify_cmds)). 
For example, extracting Exif and IPTC "raw" metadata and adding an 
[Xmp.dc.subject](https://www.exiv2.org/tags-xmp-dc.html) value.
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --extract XXei --Modify "add Xmp.dc.subject Monument" Stonehenge.jpg
$ exiv2 --key Xmp.dc.subject Stonehenge.exv
Xmp.dc.subject                               XmpBag      1  Monument
```

<div id="comment_txt">

### **-c** *txt*, **--comment** *txt*
JPEG comment string to set in the image ([modify](#mo_modify) action) 
(see [IMAGE COMMENTS](#image_comments)).

<div id="modify_cmdfile">

### **-m** *cmdfile*, **--modify** *cmdfile*
Commands for the [modify](#mo_modify) action (see 
[Command file format](#Command_file_format)). In the format:

**--modify** ( *file* | *-* )

| Parameter | Description                               |
|:------    |:----                                      |
| *file*    | The file containing the 'modify' commands |
| *-*       | Stdin                                     |

This option can also be used with the [extract](#ex_extract) actions to 
modify metadata on-the-fly. See ['MODIFY' COMMANDS](#modify_cmds).

<div id="Modify_cmd">

### **-M** *cmd*, **--Modify** *cmd*
Command line for the [modify](#mo_modify) action. This option can also be 
used with the [extract](#ex_extract) action to modify metadata on-the-fly. 
See ['MODIFY' COMMANDS](#modify_cmds) and ['Modify' command file](#mod_cmd_file).

<div id="location_dir">

### **-l** *dir*, **--location** *dir*
Location (directory) for files to be inserted from or extracted to.

<div id="suffix_suf">

### **-S** *suf*, **--suffix** *suf*
Use suffix *suf* for source files when using [insert](#in_insert) action 
(e.g., '.txt').

[TOC](#TOC)

<div id="groups_types_values">

# 7 EXIV2 GROUPS, TYPES AND VALUES

Exiv2 supports Exif 2.32, Adobe XMP 4.4.0 by default, although can be 
built with newer versions of the SDK, and many manufacturer's makernote 
tags.

<div id="exiv2_key_syntax">

### 7.1 Exiv2 key syntax

The standard format for Exif and IPTC tags is:

*Family.Group.Tagname*

 Definition | Description                                                          |
|:------    |:----                                                                 |
| Family    | Either 'Exif' or 'Iptc'                                              |
| Group     | The group within the family. e.g., for Exif, 'Image'                 |
| Tagname   | The tagname within a family and group. e.g., For Exif.Image, 'Model' |

For example, Exif.Image.Model or Iptc.Application2.Subject .

<div id="exiv2_xmp_key_syntax">

#### XMP
In addition to the standard key syntax, XMP can also use an extended format:

*Family.Group.Tagname*[ *\<index\>*[ *\<value_types\>* ] ]+

| Definition        | Description                                                          |
|:------            |:----                                                                 |
| Family            | i.e., 'Xmp'                                                          |
| Group             | The group within the family. e.g., for Xmp, 'dc'                     |
| Tagname           | The tagname within a family and group. e.g., For Xmp.dc, 'subject'   |
| *\<index\>*       | An integer (1,...,n) surrounded by square brackets (e.g., [n], is the n'th item in the 'array') |
| *\<value_types\>* | This is  */name1* : *name2* (e.g., /dim:length). Within a *Family.Group.Tagname[n]* and nested arrays, only one type of */name1* identifier is allowed, but many *name2* values |

For example, the [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html) 
tag would use the extended key:
```  
Xmp.xmpMM.History[1]/stEvt:action
```
Nested structures are permitted and use multiple [ *\<index\>*[ *\<value_types\>* ] ] 
in the key. For example, if an [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html) 
tag defined an additional nested array struct, the extended key would be:
```
Xmp.xmpMM.History[1]/stEvt:myArray[1]/myName1:myName2
```

See [XMP structs](#xmp_structs) and [Setting an XMP struct value](#set_xmp_struct).

More information about XMP keys and conventions for *\<value_types\>* is 
available from: https://www.adobe.com/content/dam/acom/en/devnet/xmp/pdfs/XMPSDKReleasecc-2020/XMPSpecificationPart2.pdf#G4.1132618

[TOC](#TOC)

<div id="exiv2_tags">

### 7.2 Exiv2 tags
For an explanation of Exiv2 keys, see [Exiv2 key syntax](#exiv2_key_syntax).
The following groups are defined for the Exif family:

```
GPSInfo    Canon         Nikon1     NikonMe       OlympusFe8      SonyMisc2b
Image      CanonCf       Nikon2     NikonPc       OlympusFe9      SonyMisc3c
Image2     CanonCs       Nikon3     NikonPreview  OlympusFi       SonyMinolta
Image3     CanonFi       NikonAFT   NikonSi01xx   OlympusIp       SonySInfo1
Iop        CanonPa       NikonAf    NikonSi02xx   OlympusRd
MakerNote  CanonPi       NikonAf2   NikonSiD300a  OlympusRd2      Samsung2
MpfInfo    CanonPr       NikonAf22  NikonSiD300b  OlympusRi       SamsungPictureWizard
Photo      CanonSi       NikonCb1   NikonSiD40    SamsungPreview
SubImage1  CanonTi       NikonCb2   NikonSiD80    Sigma	
SubImage2                NikonCb2a  NikonVr
SubImage3  Casio         NikonCb2b  NikonWt       Sony1
SubImage4  Casio2        NikonCb3                 Sony1Cs
SubImage5                NikonCb4   Olympus       Sony1Cs2
SubImage6  Minolta       NikonFi    Olympus2      Sony1MltCs7D
SubImage7  MinoltaCs5D   NikonFl1   OlympusCs     Sony1MltCsA100
SubImage8  MinoltaCs7D   NikonFl2   OlympusEq     Sony1MltCsNew
SubImage9  MinoltaCsNew  NikonFl3   OlympusFe1    Sony1MltCsOld
SubThumb1  MinoltaCsOld  NikonFl7   OlympusFe2    Sony2
Thumbnail                NikonIi    OlympusFe3    Sony2Cs
           Panasonic     NikonLd1   OlympusFe4    Sony2Cs2
Pentax     PanasonicRaw  NikonLd2   OlympusFe5    Sony2010e
PentaxDng                NikonLd3   OlympusFe6    Sony2Fp
           Fujifilm      NikonLd4   OlympusFe7    SonyMisc1
```

A full list of built-in Exif, IPTC and XMP tags is available online at 
https://www.exiv2.org/metadata.html. To query Exiv2 Groups and Tagnames, 
a sample program called [taglist](https://github.com/Exiv2/exiv2/blob/main/README-SAMPLES.md#taglist) 
is provided, which can be built from source (see https://github.com/Exiv2/exiv2/blob/main/README.md) 
or downloaded from https://www.exiv2.org/download.html.

For the Exif, IPTC and XMP groups:

- The same tags that are available in Exif's Image group (e.g., Exif.Image.XResolution), 
are also available in Exif's Thumbnail, Image(2|3), SubImage(1-9) and 
SubThumb1 groups, (e.g., Exif.Thumbnail.XResolution).

- Many camera manufacturer's tags are available and are accessed as 
different Exif groups, using the manufacturer's name as a prefix (e.g., 
CanonCs is the Camera Settings for a Canon camera). Groups called the 
manufacturer name plus number, access the main tags in different file 
formats (e.g., Sony1 is for JPEGs, Sony2 is for others such as TIFF and 
ARW). Groups are available for use, depending upon which camera 
manufacturer and model captured the photo. Supporting the large number of 
tags for all the different manufacturers is an ongoing task, only partial 
support is available. The full unprocessed makernotes data is available 
in [Exif.Photo.MakerNote](https://www.exiv2.org/tags.html).

- Every Exif and IPTC tag has a tag number (16 bit, 2 byte integer), 
which is unique within a Group (to display, see [--Print x](#Print_flgs)).

- Some of the Exif and IPTC tags are mirrored in the XMP specification 
(see https://www.exiv2.org/metadata.html).

- The XMP specification is flexible and allows new custom 'Groups' and 
'Tagnames' to be added (see [Adding new XMP tags](#add_xmp_tags) and 
['Modify' command format](#mod_cmd_format)). For example, the 'cm2e' 
'Group' has been added, which has 'Father' and 'Family' Tagnames. 
Additionally, a 'Family' tag has been added to the built-in 'dc' Group:
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

- Further information on Exiv2 groups can be found on the Exiv2 wiki: https://github.com/Exiv2/exiv2/wiki

[TOC](#TOC)

<div id="exiv2_types">

### 7.3 Exif/IPTC/XMP types

Each tag uses a specific type to store the value, with all elements in 
any array having the same type. Available types for the different 
'Family' categories are:

<div id="exiv2_types_exif">

#### Exif

| Type      | Format                                       | Element | Comment                                        |
|:------    |:----                                         |:------  |:------                                         |
| Short     | A 16-bit (2-byte) unsigned integer           | Single  | -                                              |
| Long      | A 32-bit (4-byte) unsigned integer           | Single  | -                                              |
| Rational  | Two Long types. Numerator and denominator    | Single  | Use as X/Y (e.g., "31/1" is 31.0). See [Setting a Rational or SRational value](#set_Rational_SRational) |
| Byte      | A string of 8-bit unsigned integers          | Single  | Use as a string (e.g., "Value")|
| SShort    | A 16-bit (2-byte) signed integer             | Single  | -                                              |
| SLong     | A 32-bit (4-byte) signed integer             | Single  | -                                              |
| SRational | Two SLong types. Numerator and denominator   | Single  | Use as [+\|-]X/Y (e.g., "-31/1" is -31.0). See [Setting a Rational or SRational value](#set_Rational_SRational) |
| SByte     | A string of 8-bit signed integers            | Single  | Use as a string (e.g., "Value")                |
| Float     | Signed single precision (4-byte) IEEE format | Single  | Use as a signed decimal (e.g., "-0.5" is -0.5) |
| Double    | Signed double precision (8-byte) IEEE format | Single  | Use as a signed decimal (e.g., "-0.5" is -0.5) |
| Ascii     | A string of 7-bit ASCII. NULL terminated     | Single  | Use as a string (e.g., "Value")                |
| Comment   | A string of 7-bit ASCII. NULL terminated)    | Single  | Use as a string (e.g., "Value"). See [Exif 'Comment' values](#exif_comment_values) |
| Undefined | A string of 8-bit bytes                      | Single  | Use as a string (e.g., "Value")                |

<div id="exiv2_types_iptc">

#### IPTC

| Type      | Format                             | Element | Comment                                  |
|:------    |:----                               |:------  |:------                                   |
| String    | A string of ASCII                  | Single  | -                                        |
| Short     | A 16-bit (2-byte) unsigned integer | Single  | -                                        |
| Date      | A string of ASCII (10 bytes)       | Single  | See [Date/Time formats](#date_time_fmts) |
| Time      | A string of ASCII (11 bytes)       | Single  | See [Date/Time formats](#date_time_fmts) |
| Undefined | A string of 8-bit bytes            | Single  | Use as a string (e.g., "Value")          |

<div id="exiv2_types_xmp">

#### XMP

| Type    | Format           | Element | Comment                                      |
|:------  |:----             |:------  |:------                                       |
| LangAlt | A string of text | Multi   | Similar to XmpAlt. Alternative text for different languages. See [XMP LangAlt values](#langalt_values), [Multiple elements](#multi_elements) and ['Modify' command format](#mod_cmd_format) |
| XmpAlt  | A string of text | Multi   | An ordered array, any default value is first |
| XmpBag  | A string of text | Multi   | An unordered array (e.g., 3 values, "Red, Yellow, Green"). See [Multiple elements](#multi_elements) and ['Modify' command format](#mod_cmd_format) |
| XmpSeq  | A string of text | Multi   | An ordered array (e.g., 3 values, "Gold, Silver, Bronze"). See [Multiple elements](#multi_elements) and ['Modify' command format](#mod_cmd_format) |
| XmpText | A string of text | Single  | -                                            |

[TOC](#TOC)

<div id="multi_elements">

### 7.4 Multiple elements
Some IPTC and XMP tags allow multiple elements to be stored, although 
they both do this in different ways.

<div id="multi_elements_iptc">

#### IPTC

IPTC tags marked as repeatable, can have multiple tags with the same 
key (see [Exiv2 key syntax](#exiv2_xmp_key_syntax)) and type (for 
supported tags, see https://www.exiv2.org/iptc.html, column R).

For example, a file containing three [Iptc.Application2.Subject](https://www.exiv2.org/iptc.html) 
values:
```
$ exiv2 --grep Application2 Stonehenge_with_extra_data.jpg
Iptc.Application2.RecordVersion              Short       1  4
Iptc.Application2.Caption                    String     12  Monument
Iptc.Application2.Subject                    String     10  Stonehenge
Iptc.Application2.Subject                    String      6  Cloudy
Iptc.Application2.Subject                    String      5  Grass
```

<div id="multi_elements_xmp">

#### XMP
XMP multiple values are grouped together within the same tag, using `, ` 
as a separator (see [Exif/IPTC/XMP types](#exiv2_types) for supported 
types). If a default value exists for a LangAlt or XmpAlt tag, then this 
is the first item listed. For example, a file containing two 
[Xmp.dc.description](https://www.exiv2.org/tags-xmp-dc.html) values (for 
format, see ['Modify' command format](#Modify_command_format) and three 
[Xmp.dc.subject](https://www.exiv2.org/tags-xmp-dc.html) values:

```
$ exiv2 --grep description --grep subject Stonehenge_with_extra_data.jpg
Xmp.dc.description                           LangAlt     1  lang="x-default" Monument, lang="de-DE" das Monument
Xmp.dc.subject                               XmpBag      3  Stonehenge, Cloudy, Grass
```

[TOC](#TOC)

<div id="multi_tags">

### 7.5 Duplicate tags

#### Exif
The Exif specification allows for duplicate tags with the same key and 
even the same key but different type.

These tags are currently unused by any official tag definition.

#### IPTC
Only IPTC repeatable tags are allowed (see [Multiple elements](#multi_elements)).

#### XMP
No duplicate XMP tags are allowed.

[TOC](#TOC)

<div id="date_time_fmts">

### 7.6 Date/Time formats

The times and dates are stored using different formats in image metadata. 

<div id="date_time_fmts_exif">

#### Exif
An Exif DateTime string is stored as 20 ASCII bytes (including trailing 
NULL) in the format:
```
YYYY:MM:DD HH:mm:SS
``` 
where YYYY:MM:DD is the year, month and day and HH:mm:SS is the hour, 
minute and second (e.g., "2010:09:15 08:45:21" is 08:45:21 on 15-Aug-10).

The options [--timestamp](#timestamp) and [--Timestamp](#Timestamp) will 
accept files in which the Date part, has been incorrectly stored as 
YYYY-MM-DD.

<div id="date_time_iptc">

#### IPTC
An IPTC 'Date' type is stored in the format:
```
YYYY-MM-DD
```
where YYYY-MM-DD is the year, month and day (e.g., "2010-09-15" is 15-Aug-10).

An IPTC 'Time' type is stored in the format:
```
HH:MM:SScHH:MM
```
where HH:MM:SS refers to local time's hour, minutes and seconds and, HH:MM 
refers to hours and minutes with c being ahead (+) or behind (-) 
Universal Coordinated Time (UCT) as described in ISO 8601 (e.g., 
08:45:15+01:00 is 08:45:15, which is +1 hour different from UCT).

<div id="date_time_makernotes">

#### Makernote tags
There is no one single format. Different camera manufacturers use 
different formats, even within their own sets of tags. Some tags use 
encoded data that does not conform to any official format.

[TOC](#TOC)

<div id="exif_comment_values">

### 7.7 Exif 'Comment' values
Exif 'Comment' values can include an optional charset definition to 
explain how to interpret the rest of the value. This is used by tags such 
as [Exif.Photo.UserComment](https://www.exiv2.org/tags.html), [Exif.GPSInfo.GPSProcessingMethod](https://www.exiv2.org/tags.html) 
and [Exif.GPSInfo.GPSAreaInformation](https://www.exiv2.org/tags.html).

The format of the value is:

[ **charset=** ( *Ascii* | *Jis* | *Unicode* | *Undefined* ) ] comment

If the charset is not specified then the default is *Undefined*.

For example, if Ascii and Unicode Comments have been set:
```
$ exiv2 --grep UserComment --grep GPSProcessingMethod Stonehenge.jpg
Exif.Photo.UserComment                       Undefined  16  charset=Ascii Monument
Exif.GPSInfo.GPSProcessingMethod             Undefined  14  charset=Unicode GPS
```

The reported size of the 'value' is calculated as:
```
8 Bytes (for "charset=val ") + (the rest of the value) * (2 Bytes for Unicode, otherwise 1 Byte)
```

See [Setting an Exif Comment value](#set_exif_comment).

[TOC](#TOC)

<div id="xmp_namespaces">

### 7.8 XMP namespaces
Every XMP 'Group' (called a *prefix*) has an identifier (called a 
*namespace*) associated with it. Within an image's metadata, both 
namespaces and prefixes are unique. 

**exiv2** has standards defined prefixes and namespaces built-in, which 
can be displayed in the format: `xmlns=<prefix>:<namespace>`. For 
example, here, only a couple are shown:
```
$ exiv2 --verbose --version --grep xmlns
...
xmlns=xmp:http://ns.adobe.com/xap/1.0/ 
xmlns=dc:http://purl.org/dc/elements/1.1/
...
```

To list the XMP prefixes and namespaces used by an image's metadata (in 
the format `xmlns:<prefix>=<namespace>`), [extract a sidecar file to stdout](#extract_tgt3). 
For example, here, other data is not included:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --extract X- Stonehenge.jpg
<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
 <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <rdf:Description rdf:about=""
    xmlns:xmp="http://ns.adobe.com/xap/1.0/"
    xmlns:cm2e="http://clanmills.com/exiv2/"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:tiff="http://ns.adobe.com/tiff/1.0/"
    xmlns:exif="http://ns.adobe.com/exif/1.0/"
    xmlns:photoshop="http://ns.adobe.com/photoshop/1.0/"
   xmp:Rating="0"
   xmp:ModifyDate="2015-07-16T20:25:28+01:00"
   xmp:CreateDate="2015-07-16T15:38:54.00"
   ...
```

New XMP namespaces can be registered, allowing new 'Groups' and 
'Tagnames' to be created (see [Registering an XMP namespace](#reg_xmp_namespace)).

Within an image's metadata, built-in prefixes can be re-registered with 
different namespaces. In this case, **exiv2** will use any new 
definitions.

[TOC](#TOC)

<div id="langalt_values">

### 7.9 XMP LangAlt values
A 'LangAlt' string provides alternative text for different languages, 
with individual entries separated using a  `, `. Each entry is in the 
format:

[**lang**=( *x-default* | *\<lang\>* | (*\<lang\>-\<country\>*)] text

| Definition | Description                                 |
|:------     |:----                                        |
| *x-default*  | The default value. If defined, a LangAlt string always lists this entry first |
| *lang*       | A language code as defined in [IEFT RFC 3066](https://www.ietf.org/rfc/rfc3066.txt) |
| *country*    | A country code as defined in [IEFT RFC 3066](https://www.ietf.org/rfc/rfc3066.txt) |
| *text*       | The text to use for a language definition   |

The *\<lang\>-\<country\>* combination is useful where a country uses a 
different version of a common language (e.g., Swiss-French, would be 
"fr-CH"). The *\<lang\>* or *\<lang\>-\<country\>* can be optionally 
surrounded by double quotes (single quotes are not allowed). If no 
language is specified, the default value is *x-default*. Any *x-default* 
entry is always listed first.

For example, 3 language definitions including a default value, a 
British English value and a general French value:
```
lang="x-default" Elevator, lang="en-gb" Lift, lang="fr" ascenseur
```
[TOC](#TOC)

<div id="xmp_structs">

### 7.10 XMP structs

XMP structs group XmpText elements together in an array arrangement. The 
elements themselves can contain other structs, allowing complex nested 
array structures to be created. The tags in a struct use the extended XMP 
version of the [Exiv2 key syntax](#exiv2_xmp_key_syntax).

In a struct, the base key (e.g., [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html)) 
contains:

**type**=*\<array_type\>*

where *\<array_type\>* is 'Alt' for XmpAlt, 'Bag' for XmpBag or 'Seq' for 
XmpSeq.

Each numbered base key (e.g., Xmp.xmpMM.History[n]) contains the 
string: `type="Struct"` and each numbered base key plus value type 
(e.g., Xmp.xmpMM.History[n]/stEvt:action), contains the data.

For example, the [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html) 
tag recording that a file has been converted from tiff to JPEG, then 
saved.

```
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     7  derived
Xmp.xmpMM.History[1]/stEvt:converted         XmpText    35  tiff to jpeg, saved to new location
Xmp.xmpMM.History[2]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[2]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[2]/stEvt:instanceID        XmpText    40  xmp.iid:89A4EB97412C88632107D659
Xmp.xmpMM.History[2]/stEvt:when              XmpText    25  2021-08-13T10:12:09+01:00
Xmp.xmpMM.History[2]/stEvt:softwareAgent     XmpText    21  MySuperPhotoFixer 1.0
```
 
For an example of modifying a struct, see [Setting an XMP struct value](#set_xmp_struct).

More information about structs is available from: https://wwwimages2.adobe.com/content/dam/acom/en/devnet/xmp/pdfs/XMP%20SDK%20Release%20cc-2016-08/XMPSpecificationPart1.pdf#G5.873540

[TOC](#TOC)

<div id="preview_images">

# 8 PREVIEW IMAGES AND THUMBNAILS
Image metadata can contain several smaller versions of the main image, 
which can be extracted as separate images. If a thumbnail has been added, 
then this is also included in the list of available previews.

Currently, preview images are read only (apart from thumbnails of 
certain [FILE TYPES](#file_types)).

For example, to extract a preview image, first list which previews are 
available:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --print p Stonehenge.jpg
Preview 1: image/jpeg, 160x120 pixels, 10837 bytes
```
then extract the preview by number (in this case, preview 1 is also the 
thumbnail):
```
$ exiv2 --verbose --extract p1 Stonehenge.jpg
File 1/1: Stonehenge.jpg
Writing preview 1 (image/jpeg, 160x120 pixels, 10837 bytes) to file ./Stonehenge-preview1.jpg
```
If no image number is provided, then all available preview images will be extracted: 
```
$ exiv2 --verbose --extract p Stonehenge.jpg
File 1/1: Stonehenge.jpg
Writing preview 1 (image/jpeg, 160x120 pixels, 10837 bytes) to file ./Stonehenge-preview1.jpg
```

Each image type and camera manufacturer, stores preview images in 
different ways. Only partial support is available for the many different 
preview formats.

In addition to using preview images, another method is provided to access 
the thumbnail. For example, to extract the thumbnail to a *\<file\>-thumb.\<ext\>* 
file:
```
$ exiv2 --verbose --extract t Stonehenge.jpg
File 1/1: Stonehenge.jpg
Writing thumbnail (image/jpeg, 10837 Bytes) to file ./Stonehenge-thumb.jpg
```
Some [FILE TYPES](#file_types) allow writing a thumbnail to the image 
metadata. For example, to delete a thumbnail:
```
$ exiv2 --verbose --delete t Stonehenge.jpg
File 1/1: Stonehenge.jpg
Erasing thumbnail data
``` 

For example, if a file called *\<file\>-thumb.\<ext\> exists, it can be 
added as a thumbnail with:
```
$ exiv2 --verbose --insert t Stonehenge.jpg
File 1/1: Stonehenge.jpg
```

[TOC](#TOC)

<div id="icc_profiles">

# 9. ICC PROFILES

An ICC profile describes the color characteristics of the image (see 
https://en.wikipedia.org/wiki/ICC_profile). Depending upon [FILE TYPES](#file_types) 
support, the profiles can accessed as follows.

For example, to display the profile to stdout:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
exiv2 --print C Stonehenge.jpg
```

extract to a *\<file\>.icc* file with:
```
exiv2 --verbose --extract C Stonehenge.jpg
```
and insert from a *\<file\>.icc* with:
```
exiv2 --verbose --insert C Stonehenge.jpg
```

[TOC](#TOC)

<div id="image_comments">

# 10. IMAGE COMMENTS
An image comment is text stored separately in the image metadata, instead 
of a specific Exif, IPTC or XMP tag. See [FILE TYPES](#file_types) for 
supported image types.

[TOC](#TOC)

<div id="modify_cmds">

# 11 'MODIFY' COMMANDS
Tags can be altered using the 'modify' action and 
[--Modify cmd](#Modify_cmd) or [--modify cmdfile](#modify_cmdfile).

For example, adding an IPTC tag on the command line:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "add Iptc.Application2.Credit String Mr. Mills" Stonehenge.jpg
```
or applying a set of 'modify' commands from a file (see ['Modify' command File](#mod_cmd_file)):
```
$ exiv2 --modify cmd.txt Stonehenge.jpg
```
Multiple [--Modify cmd](#Modify_cmd) and [--modify cmdfile](#modify_cmdfile) 
options can be combined together, with the changes applied left to right 
on each file. For example, applying one command line 'modify', then a 
'modify' command file:
```
$ exiv2 --Modify "set Iptc.Application2.Caption Stonehenge" --modify cmdfile.txt Stonehenge.jpg
```

<div id="gen_modify_cmds">

A list of modify commands can be generated from an existing file using 
[--Print Vkyv](#Print_flgs). For example, only displaying the 'dc' 
'modify' commands:
```
$ exiv2 --Print Vkyv --grep dc Stonehenge.jpg
set Xmp.dc.description                           LangAlt    lang="x-default" Classic View
set Xmp.dc.Family                                XmpBag     Robin
```

<div id="modify_cmdfile_stdin">

Using [--modify -](#modify_cmdfile), a command file can be read from 
stdin instead of 
being a named file. For example: 
```
$ cat cmdfile.txt | exiv2 --modify - Stonehenge.jpg
```

When modifying, only a correct metadata structure is enforced and it is 
possible to write tags with types and values different from those 
specified in the standards. This can include incorrect types (e.g., 
change [Exif.Image.DateTime](https://www.exiv2.org/tags.html) to a 
Float), [Duplicate tags](#multi_tags) or incomplete metadata 
(e.g., deleting [Exif.GPSInfo.GPSLatitudeRef](https://www.exiv2.org/tags.html), 
which works together with [Exif.GPSInfo.GPSLatitude](https://www.exiv2.org/tags.html)). 
While **exiv2** is able to read all metadata that it can write, other 
programs may have difficulties with images that contain non 
standards-defined metadata.

<div id="quotes_modify">

### 9.1 Quotations with 'modify' commands
With [--Modify cmd](#Modify_cmd), quotation marks are use to surround the 
*cmd*. The Windows command line requires double quotes to be 
used, however generally, quotes inside that string, can be either single 
or double. Generally, UNIX based systems can use pairs of single or 
double quotes in any position. In both systems, inner string quotation 
marks may need to use the `\` escape sequence.

See note on quotations when using [XMP LangAlt values](#langalt_values).

[TOC](#TOC)

<div id="mod_cmd_format">

### 9.2 'Modify' command format
The format of a command is:

( **set** | **add** ) *key* [ [ *type* ] *value* ] | **del** *key* [ *type* ] | **reg** *prefix* *namespace*

Where:

| Option      | Description                                                              |
|:------      |:----                                                                     |
| **add**     | Adds a tag. For Exif and IPTC repeatable tags, this adds a new tag, otherwise this replaces any existing tag (see [Multiple elements](#multi_elements) and [Duplicate (Exif) tags](#multi_tags)) |
| **set**     | Set the *value* of an existing tag with a matching *key* or if undefined, **add** the tag. If [Duplicate (Exif) tags](#multi_tags) or IPTC repeatable tags already exist, this replaces them (see [Multiple elements](#multi_elements)) |
| **del**     | Delete all occurrences of a tag that matches *key*, including any duplicate tags |
| **reg**     | Registers an XMP *namespace* with an XMP *prefix* (see [XMP namespaces](#xmp_namespaces)) If registering an existing *prefix* with a different *namespace*, then the namespace is updated |

using the parameters:

| Parameter   | Description                                                              |
|:------      |:----                                                                     |
| *key*       | See [Exiv2 key syntax](#exiv2_key_syntax)                                |
| *type*      | The *type* for the tag, a default is used if none is given. For built-in tags, the default is defined in the standard and for registered XMP tags, the default is XmpText (see [Exif/IPTC/XMP types](#exiv2_types)) |
| *value*     | The remaining text on the line is the value, and can optionally be enclosed in quotes (see [Quotations with 'modify' commands](#quotes_modify)). For Ascii, XmpAlt, XmpBag, XmpSeq and XmpText, the value is optional which is equivalent to an empty value ("") |
| *prefix*    | This is the XMP 'Group' that is being registered. This is case sensitive and must be unique within an image's metadata. e.g., For an "Exiv2" *prefix*, the tags for that group are available as Xmp.Exiv2.Tagname |
| *namespace* | An identifier for the prefix. This is case sensitive and must be unique within an image's metadata. Built-in namespaces use a URL format, but they are not required to exist |

[TOC](#TOC)

<div id="mod_value">

### 9.5 Modifying a value
When modifying a value, the 'translated' version of the value should not 
be used (with the exception of [Setting an aperture value](#set_aperture)). 
Translated values are in a localized human-readable format, which may 
change over time. Instead, modify by using the 'vanilla' value.

[TOC](#TOC)

<div id="mod_examples">

### 9.4 'Modify' examples

<div id="set_Rational_SRational">

#### Setting a Rational or SRational value
For the 'Rational' and 'SRational' formats, see [Exif/IPTC/XMP types](#exiv2_types_exif). 

For example, modifying a 'Rational' number:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify"set Exif.Photo.MaxApertureValue 2485427/500000" Stonehenge.jpg
```

<div id="set_aperture">

#### Setting an aperture value
Exif aperture values are stored in an apex-value format and when using the 
translated print option, are output as an Fnumber value. The APEX Fnumber 
translations use the formulae (in base 2):

```
apex-value  = log(Fnumber) * 2.0 / log(2.0)
Fnumber     = exp(apex-value * log(2.0) / 2.0)
```
The apex-value is a Rational and can be modified as shown in [Setting a Rational or SRational value](#set_Rational_SRational), 
however a convenience method is also provided to set an Fnumber directly. 
This uses the format:

**F** *\<value\>*

Where *\<value\>* is either an integer or decimal number.

For example, the following 'modify' will produce the same value as 
[Setting a Rational or SRational value](#set_Rational_SRational). This is 
displayed in vanilla and translated output:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "set Exif.Photo.MaxApertureValue F5.6" Stonehenge.jpg
$ exiv2 --print e --grep MaxApertureValue Stonehenge.jpg
Exif.Photo.MaxApertureValue                  Rational    1  2485427/500000
$ exiv2 --print t --grep MaxApertureValue Stonehenge.jpg
Exif.Photo.MaxApertureValue                  Rational    1  F5.6
```

See [Modifying a value](#mod_value).

More information about APEX values is available from: http://en.wikipedia.org/wiki/APEX_system

<div id="set_exif_comment">

#### Setting an Exif Comment value
For the 'Comment' format, see [Exif 'Comment' values](#exif_comment_values).

For example, to set an Ascii comment:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "set Exif.Photo.UserComment charset=Ascii Photo" Stonehenge.jpg
$ exiv2 --grep UserComment Stonehenge.jpg
Exif.Photo.UserComment                       Undefined  13  charset=Ascii Photo
```
or a Unicode comment:
```
$ exiv2 --Modify "set Exif.GPSInfo.GPSProcessingMethod charset=Unicode \\u0047\\u0050\\u0053" Stonehenge.jpg
$ exiv2 --grep GPSProcessingMethod Stonehenge.jpg
Exif.GPSInfo.GPSProcessingMethod             Undefined  14  charset=Unicode GPS
```

<div id="reg_xmp_namespace">

#### Registering an XMP namespace
For help with namespaces, see [XMP namespaces](#xmp_namespaces).

Each XMP [namespace](#mod_cmd_format) has a [prefix](#mod_cmd_format) 
associated with it. For example, when modifying, this is declared using 
the string:
```
--Modify "reg myPrefix http://ns.myPrefix.me/"
```

When a non-standard group or tag belonging to that group is modified, the 
appropriate 'reg' 'modify' command must be included before that. For 
example, adding a *myPic* group with a *weather* tag and values:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "reg myPic http://ns.myPic.org/" --Modify "add Xmp.myPic.weather XmpBag Cloudy" Stonehenge.jpg
$ exiv2 --Modify "reg myPic http://ns.myPic.org/" --Modify "set Xmp.myPic.weather XmpBag Sunny" Stonehenge.jpg
$ exiv2 --Modify "reg myPic http://ns.myPic.org/" --Modify "set Xmp.myPic.weather XmpBag Hot" Stonehenge.jpg
$ exiv2 --grep myPic Stonehenge.jpg
Xmp.myPic.weather                            XmpBag      3  Cloudy, Sunny, Hot
```

<div id="add_xmp_tags">

#### Adding new XMP tags
Provided that an XMP's group is defined (see [Registering an XMP namespace](#reg_xmp_namespace)), 
new tags can be added by assigning values.

For example, to add a *RatingInPercent* tag to a built-in namespace and 
a *Surname* tag to a registered namespace:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmp.RatingInPercent XmpText 98" Stonehenge.jpg
$ exiv2 --Modify "reg myPic http://ns.myPic.org/" --Modify "add Xmp.myPic.Weather XmpBag Hot" Stonehenge.jpg
$ exiv2 --grep RatingInPercent --grep Weather Stonehenge.jpg
Xmp.xmp.RatingInPercent                      XmpText     2  98
Xmp.myPic.Weather                            XmpBag      1  Hot
```

If adding an empty non XmpText tag, include the empty string as the 
value. This prevents an XmpText tag being created with the value set to 
the 'type'.
   
<div id="set_langalt">

#### Setting an XMP LangAlt value
For the LangAlt format, see [XMP LangAlt values](#langalt_values). When 
setting a LangAlt tag, only one entry is allowed per modify.

For example, adding default and language/country entries:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "set Xmp.dc.description LangAlt Monument" Stonehenge.jpg
$ exiv2 --Modify "set Xmp.dc.description LangAlt lang=de-DE das Monument" Stonehenge.jpg
$ exiv2 --grep description Stonehenge.jpg
Xmp.dc.description                           LangAlt     2  lang="x-default" Monument, lang="de-DE" das Monument
```

and adding a language and language/country entries:

```
$ exiv2 --Modify "set Xmp.dc.title LangAlt lang=en-US My holiday" Stonehenge.jpg
$ exiv2 --Modify "set Xmp.dc.title LangAlt lang=fr Mes vacances" Stonehenge.jpg
$ exiv2 --grep title Stonehenge.jpg
Xmp.dc.title                                 LangAlt     2  lang="en-US" My holiday, lang="fr" Mes vacances
```

To remove a language specification, set the value to the empty string. 
For example:
```
$ exiv2 --Modify "set Xmp.dc.description lang=de-DE" Stonehenge.jpg
```
To remove all language specifications, delete the key. For example:
```
$ exiv2 --Modify "del Xmp.dc.description" Stonehenge.jpg
```

<div id="set_xmp_struct">

#### Setting an XMP struct value
For the format of XMP structs, see [Exiv2 key syntax](#exiv2_xmp_key_syntax) 
and [XMP structs](#xmp_structs).

To create a struct, first set the base key to an empty version of an 
array type. This can be achieved by either stating the array type and 
using the empty string, or by using XmpText and stating the array type.

For example, both of the following set the Xmp.xmpMM.History tag to an 
empty XmpSeq:
```
--Modify "add Xmp.xmpMM.History XmpSeq \"\""
--Modify "add Xmp.xmpMM.History XmpText type=Seq"
```
Numbered keys plus name and action can then be added.
 
For example, setting the [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html) 
tag to record the file being converted from tiff to JPEG, then saved.
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History XmpSeq \"\"" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:action derived" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:converted tiff to jpeg, saved to new location" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[2]/stEvt:action saved" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[2]/stEvt:instanceID xmp.iid:89A4EB97412C88632107D659" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[2]/stEvt:when 2021-08-13T10:12:09+01:00" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[2]/stEvt:softwareAgent MySuperPhotoFixer 1.0" Stonehenge.jpg
$ exiv2 --grep xmpMM Stonehenge.jpg
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     7  derived
Xmp.xmpMM.History[1]/stEvt:converted         XmpText    35  tiff to jpeg, saved to new location
Xmp.xmpMM.History[2]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[2]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[2]/stEvt:instanceID        XmpText    32  xmp.iid:89A4EB97412C88632107D659
Xmp.xmpMM.History[2]/stEvt:when              XmpText    25  2021-08-13T10:12:09+01:00
Xmp.xmpMM.History[2]/stEvt:softwareAgent     XmpText    21  MySuperPhotoFixer 1.0
```

Nested arrays are permitted. For example, to add an additional XmpSeq to 
the first struct in the [Xmp.xmpMM.History](https://www.exiv2.org/tags-xmp-xmpMM.html) 
tag:

```
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:mySeq XmpText type=Bag" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:mySeq[1]/stEvt:action open a file" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:mySeq[2]/stEvt:action convert" Stonehenge.jpg
$ exiv2 --Modify "add Xmp.xmpMM.History[1]/stEvt:mySeq[3]/stEvt:action save" Stonehenge.jpg
$ exiv2 --grep xmpMM Stonehenge.jpg
Xmp.xmpMM.History                            XmpText     0  type="Seq"
Xmp.xmpMM.History[1]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:action            XmpText     7  derived
Xmp.xmpMM.History[1]/stEvt:converted         XmpText    35  tiff to jpeg, saved to new location
Xmp.xmpMM.History[1]/stEvt:mySeq             XmpText     0  type="Bag"
Xmp.xmpMM.History[1]/stEvt:mySeq[1]          XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:mySeq[1]/stEvt:action XmpText    11  open a file
Xmp.xmpMM.History[1]/stEvt:mySeq[2]          XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:mySeq[2]/stEvt:action XmpText     7  convert
Xmp.xmpMM.History[1]/stEvt:mySeq[3]          XmpText     0  type="Struct"
Xmp.xmpMM.History[1]/stEvt:mySeq[3]/stEvt:action XmpText     4  save
Xmp.xmpMM.History[2]                         XmpText     0  type="Struct"
Xmp.xmpMM.History[2]/stEvt:action            XmpText     5  saved
Xmp.xmpMM.History[2]/stEvt:instanceID        XmpText    32  xmp.iid:89A4EB97412C88632107D659
Xmp.xmpMM.History[2]/stEvt:when              XmpText    25  2021-08-13T10:12:09+01:00
Xmp.xmpMM.History[2]/stEvt:softwareAgent     XmpText    21  MySuperPhotoFixer 1.0
```

[TOC](#TOC)

<div id="mod_cmd_file">

### 9.5 'Modify' command file
Commands for the 'modify' action can be read from a file. They are 
executed line-by-line, as though each command were passed to [--Modify cmd](#Modify_cmd) 
on a single **exiv2** line. In the file, any blank lines or additional 
white space is ignored and any lines beginning with a `#` are comments. 
When registering namespaces, place this in the file before the command to 
use it.

For help creating a command file from an existing file, see [extracting modify commands](#gen_modify_cmds).

For example, a *cmd.txt* file:
```
# Comments are ignored
add Xmp.dc.description LangAlt lang="x-default" Monument
set Xmp.dc.description LangAlt lang="de-DE" das Monument

# This command was generated from a file. Additional whitespace is ignored
set Xmp.xmp.Rating                               XmpText    0

# Additional whitespace is ignored
  del Iptc.Application2.Caption

# Use 'reg' before modifying any non-standard 'Group'
reg myPic https://ns.myPic.org/
set Xmp.myPic.weather Cloudy
```
would be run using:
```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --modify cmd.txt Stonehenge.jpg
```
For another example, see: https://www.exiv2.org/sample.html

[TOC](#TOC)

<div id="config_file">

# 12 CONFIGURATION FILE
**exiv2** can read an optional configuration file, which allows 
additional lens definitions to be added to translated output. On UNIX 
based systems, this file is called *.exiv2* and on Windows (including MinGW), 
*exiv2.ini*. The file is searched for first in the current directory, 
then in the home directory (on UNIX based systems, `~/` and on Windows, 
`%USERPROFILE%\\`).

You can determine the name of the file and where it is searched for, 
with the command:

```
$ exiv2 --verbose --version --grep config_path
exiv2 1.0.0.9
config_path=/Users/rmills/.exiv2
```

The configuration file uses the *Windows INI* format (see https://en.wikipedia.org/wiki/INI_file)
and has sections for each of the major camera manufactures: Canon, 
Minolta, Nikon, Olympus, Pentax and Sony. The configuration file 
definitions override any existing values.

For example, to add a new lens definition, first obtain the lensID for 
your camera:

```
$ curl --silent -O https://clanmills.com/Stonehenge.jpg
$ exiv2 --print v --grep lens/i Stonehenge.jpg
0x0083 Nikon3       LensType           Byte       1  14
0x0084 Nikon3       Lens               Rational   4  180/10 2500/10 35/10 63/10
0x008b Nikon3       LensFStops         Undefined  4  55 1 12 0
0x000c NikonLd3     LensIDNumber       Byte       1  146
0x000d NikonLd3     LensFStops         Byte       1  55
```

Then using the 'LensIDNumber' value (i.e., 146), add the following text to 
the configuration file:
```
[nikon]
146=Robin's Sigma Lens
```
The text will now appear when the translated print option is used:

```
$ exiv2 --print t --grep lens/i Stonehenge.jpg
Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  18-250mm F3.5-6.3
Exif.Nikon3.LensFStops                       Undefined   4  4.58333
Exif.NikonLd3.LensIDNumber                   Byte        1  Robin's Sigma Lens
Exif.NikonLd3.LensFStops                     Byte        1  F4.6
```

[TOC](#TOC)

<div id="examples">

# 13 EXAMPLES

- `exiv2 *.jpg`<br>
Prints a summary of the Exif information for all the JPEG files in the current 
directory (the same as `exiv2 print *.jpg`). The summary 
is brief and does not use the Family.Group.Tagname format. See ([--print mod](#print_mod)).

- `exiv2 --grep date/i https://clanmills.com/Stonehenge.jpg`<br>
Prints the tags in https://clanmills.com/Stonehenge.jpg, where the key 
(see [Exiv2 key syntax](#exiv2_key_syntax)) contains the string *date* 
(*/i* searches case insensitive, see [--grep str](#grep_str)). When not including [--print mod](#print_mod) 
or [--Print flgs](#Print_flgs), the default output becomes [--print a](#print_mod) 
(i.e., print all).

- `exiv2 --print i image.jpg`<br>
Prints the IPTC tags in *image.jpg* (see [--print mod](#print_mod)).

- `exiv2 --Print IkytEX image.jpg`<br>
Prints (with finer grained control) the Exif and XMP tags in *image.jpg*. 
The tag's key (see [Exiv2 key syntax](#exiv2_key_syntax)), 
type and translated value are displayed (see [--Print flgs](#Print_flgs)).

- `exiv2 rename image.jpg`<br>
Renames *image.jpg* (taken on 13-Nov-05 at 22:58:31) to *20051113_225831.jpg*. 
See [rename](#mv_rename).

- `exiv2 --rename ":basename:_%Y-%m" image.jpg`<br>
Renames *image.jpg* using the basename (i.e., *image*) and values 
defined in [iconv_open(3)](https://linux.die.net/man/3/iconv_open) 
to *image_2005-11.jpg*. The values for time and date are taken from the 
Exif tags. See [--rename fmt](#rename_fmt)).

- `exiv2 --extract t image1.jpg image2.jpg`<br>
Extracts (copies) the thumbnail from *image1.jpg* into 
*image1-thumb.jpg* and from *image2.jpg* into *image2-thumb.jpg*. See 
[--extract tgt3](#extract_tgt3).

- `exiv2 --insert t image1.jpg image2.jpg`<br>
Inserts (copies) thumbnails *image1-thumb.jpg* into *image1.jpg* and 
*image2-thumb.jpg* into *image2.jpg*. See [--insert tgt2](#insert_tgt2).

- `exiv2 --extract p1,2 image.jpg`<br>
Extracts (copies) previews 1 and 2 from *image.jpg*, into *image-preview1.jpg* 
and *image-preview2.jpg*. Use `exiv2 --print p image.jpg` to display a 
list of available previews for *image.jpg*. See [--extract tgt3](#extract_tgt3).

- `exiv2 --extract X image.jpg`<br>
Extracts (copies) metadata tags from *image.jpg*, into an XMP sidecar file, 
*image.xmp*. In the process, this converts selected Exif and IPTC tags 
to XMP tags. See [--extract tgt3](#extract_tgt3).

- `exiv2 --insert X image.jpg`<br>
Inserts (copies) metadata from an XMP sidecar file, *image.xmp*, into 
*image.jpg*. The resulting Exif and IPTC tags are converted from the 
equivalent XMP tags in the sidecar file. See [--insert tgt2](#insert_tgt2).

- `exiv2 --extract X --Modify "add Xmp.dc.subject Sunset" image.jpg`<br>
Extracts (copies) metadata tags from *image.jpg*, applies [--Modify cmd](#Modify_cmd) 
to those tags and then saves in an XMP sidecar file, *image.xmp*. While 
saving, selected Exif and IPTC tags are converted to XMP tags. Multiple 
[--Modify cmd](#Modify_cmd) and [--modify cmdfile](#modify_cmdfile) can 
be used. See [--extract tgt3](#extract_tgt3).

- `exiv2 --extract X- image1.jpg | exiv2 --insert X- image2.jpg`<br>
Extracts (copies) the *image1.jpg* metadata as XMP sidecar data and 
inserts it directly into *image2.jpg*. [--Modify cmd](#Modify_cmd) and 
[--modify cmdfile](#modify_cmdfile) can also be added when extracting 
from *image1.jpg*. See [--extract tgt3](#extract_tgt3) and [--insert tgt2](#insert_tgt2).

- `exiv2 delete image.jpg`<br>
Deletes all the metadata in *image.jpg*. See [delete](#rm_delete).

- `exiv2 --delete tC image.jpg`<br>
Deletes the thumbnail and ICC profile in *image.jpg*. See [--delete a](#delete_tgt1).

- `exiv2 --adjust 1:00:00 image.jpg`<br>
Adjusts Exif timestamps in *image.jpg*, adding 1 hour. See [--adjust time](#adjust_time).

- `exiv2 --Modify "set Exif.Photo.UserComment charset=Ascii New Exif comment" image.jpg`<br>
Sets the Exif comment in *image.jpg*, to an Ascii string with the value 
*New Exif comment*. See [--Modify cmd](#Modify_cmd).

- `exiv2 --Modify "set Exif.GPSInfo.GPSLatitude 4/1 15/1 33/1" image.jpg`<br>
`exiv2 --Modify "set Exif.GPSInfo.GPSLatitudeRef N" image.jpg`<br>
Sets the latitude to 4 degrees, 15 minutes and 33 seconds north in 
*image.jpg*. The Exif standard stipulates that the GPSLatitude 
tag consists of three Rational numbers for the degrees, minutes and 
seconds of the latitude and GPSLatitudeRef contains either 'N' or 'S' 
for north or south latitude respectively. See [--Modify cmd](#Modify_cmd).
Both [--Modify cmd](#Modify_cmd) options can be added to the same **exiv2** 
command.

- `exiv2 --Modify "reg myPic http://ns.myPic.org/" --Modify "add Xmp.myPic.Author Robin" Stonehenge.jpg`<br>
Registers a new XMP namespace called *http<nolink>://ns.myPic.org/* and a new 
XMP group called *myPic*. This new Group has a new *Author* tag added to it. 
See [--Modify cmd](#Modify_cmd).

- `exiv2 --location /tmp --suffix .CRW insert /data/*.JPG`<br>	
Copy all metadata from *CRW* files in the */tmp* directory to *JPG* files 
with corresponding basenames in the */data* directory. Note that this 
copies metadata as is, without any modifications to adapt it to the 
requirements of the target format. Some tags copied like this may not make 
sense in the target image. See [--location dir](#location_dir) and [--suffix suf](#suffix_suf) 
and [insert](#in_insert).

- `exiv2 fixiso image.jpg`<br>
Adds the Exif ISO metadata (if missing) to *image.jpg*. This is for 
Nikon and Canon cameras only and copies the camera maker's value into the Exif 
tags. See [fixiso](#fi_fixiso).

[TOC](#TOC)

<div id="return_value">

# 14 RETURN VALUE
`0` if successfull, otherwise a positive integer as error code.

[TOC](#TOC)

<div id="environment">

# 15 ENVIRONMENT

<div id="TZ">

### TZ
This is the timezone to use when adjusting for daylight savings time (DST) and 
affects the changing of the the time and date (e.g., when adjusting or 
renaming). See https://en.wikipedia.org/wiki/List_of_tz_database_time_zones 
for a list of valid values to use. The variable is not set by default for 
Windows and an attempt is made to obtain the timezone from the OS. If this 
fails, the DST rules for the United States are used. This can be corrected 
by defining *TZ*.

[TOC](#TOC)

<div id="notes">

# 16 NOTES
| Description          | Location                                                               |
|:----                 |:----                                                                   |
| Exiv2 wiki           | https://exiv2.org                                                      |
| Exiv2 book           | https://www.exiv2.org/book/index.html                                  |

[TOC](#TOC)

<div id="bugs">

# 17 BUGS
Report bugs to: https://github.com/Exiv2/exiv2/issues

[TOC](#TOC)

<div id="copyright">

# 18 COPYRIGHT
The Exiv2 project is released under the GNU GPLv2 license: https://github.com/Exiv2/exiv2/blob/main/COPYING

[TOC](#TOC)

<div id="authors">

# 19 AUTHORS
**exiv2** was written by Andreas Huggel and others.

Exiv2 github contributors: https://github.com/Exiv2/exiv2/graphs/contributors

[TOC](#TOC)

<div id="see_also">

# 20 SEE ALSO
[curl(1)](https://linux.die.net/man/1/curl), [xmllint(1)](https://linux.die.net/man/1/xmllint), [iconv_open(3)](https://linux.die.net/man/3/iconv_open), 
[strftime(3)](https://linux.die.net/man/3/strftime), 

| Description                    | Location                                                     |
|:----                           |:----                                                         |
| Exiv2 project website          | https://exiv2.org                                            |
| Exiv2 source code              | https://github.com/exiv2/exiv2                               |
| ICC Profiles                   | https://en.wikipedia.org/wiki/ICC_profile                    |
| Exiv2 'modify' action examples | https://exiv2.org/sample.html#modify                         |
| All Exiv2 tags                 | https://exiv2.org/metadata.html                              |
| Exiv2 README                   | https://github.com/Exiv2/exiv2/blob/main/README.md           |
| Exiv2 sample programs          | https://github.com/Exiv2/exiv2/blob/main/README-SAMPLES.md   |
| Exiv2 downloads                | https://www.exiv2.org/download.html                          |
| Exiv2 wiki                     | https://github.com/Exiv2/exiv2/wiki                          |
| Exiv2 book                     | https://www.exiv2.org/book/index.html                        |
| APEX Exposure values           | https://en.wikipedia.org/wiki/APEX_system                    |
| XMP keys                       | https://www.adobe.com/content/dam/acom/en/devnet/xmp/pdfs/XMPSDKReleasecc-2020/XMPSpecificationPart2.pdf#G4.1128133 |
| XMP structs                    | https://wwwimages2.adobe.com/content/dam/acom/en/devnet/xmp/pdfs/XMP%20SDK%20Release%20cc-2016-08/XMPSpecificationPart1.pdf |
| International Language Codes   | https://www.ietf.org/rfc/rfc3066.txt                         |
| Windows INI format             | https://en.wikipedia.org/wiki/INI_file                       |
| TZ Environment variable values | https://en.wikipedia.org/wiki/List_of_tz_database_time_zones |
| Exiv2 GitHub contributors      | https://github.com/Exiv2/exiv2/graphs/contributors           |

[TOC](#TOC)
