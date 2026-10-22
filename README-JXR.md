# JPEG XR metadata

`ImageFactory::open()` recognizes the version-1 JPEG XR still-image signature,
independently of the filename extension. It reports `ImageType::jxr` and
`image/jxr`, using a private adapter built on the TIFF reader and serializer.
Earlier versions reported TIFF identity. Native width and height describe the
stored primary image before its spatial transformation.

The adapter reads and writes Exif, IPTC, XMP and ICC metadata without decoding or
recompressing image data. It preserves the ordered image/preview chain and both
interleaved and separate alpha streams. The exposed metadata belongs to the first
non-preview image; other images retain their metadata during ordinary edits.
TIFF's existing metadata precedence applies independently within each image.

Explicit category-clearing APIs and empty category setters remove that category
from every image and preview. `clearMetadata()` removes all supported metadata
categories throughout the file. Clearing a mutable metadata container affects
only the exposed primary image. Native decoding, layout and image-association
fields survive category clears. Supplied changes to those protected fields
(`0xbc00`–`0xbcff` and PageNumber) are rejected, including partial removal and
duplicates. A metadata-only replacement may omit the native view; it is retained.
`setMetadata()` copies writable metadata, excluding source-native fields.

Raw XMP packets remain writable without the XMP toolkit. Structured XMP edits
require the toolkit. An image-byte count of zero is read through EOF and rewritten
as an explicit length. BYTE/USHORT stream offsets and lengths are promoted to
ULONG when rewritten. Output directories and external values are word-aligned;
bounded unaligned input values are accepted for compatibility with existing files.

Writing requires complete, nonoverlapping ownership of directory/value ranges and
compressed streams, valid image/alpha pairs, and a result within the 32-bit
container limit. Unsupported TIFF strip/tile/SubIFD-array/embedded-JPEG pointer
layouts and opaque maker notes are rejected. Resource limits are 1,024 images,
4,096 total directories, 65,536 entries and 64 MiB of typed values per file.
The replacement is serialized to temporary I/O, reparsed, and checked against
the intended per-image metadata and every original compressed stream before
source transfer. Original compressed streams are borrowed rather than stored in
Exif values.

JPEG XR creation, pixel encoding and Motion JPEG XR are unsupported. Direct
`TiffImage`/`TiffParser` writing rejects JPEG XR; use `ImageFactory` for editing.

The container follows Annex A of
[ITU-T T.832 (01/2012)](https://www.itu.int/rec/dologin_pub.asp?id=T-REC-T.832-201201-S%21%21PDF-E&lang=e&type=items).
No JPEG XR codec library is required. Focused unit tests run with
`unit_tests --gtest_filter='Jxr*'`; the CLI regression is
`tests/bugfixes/github/test_issue_3162.py`.
