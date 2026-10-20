# Nikon AFInfo2

Nikon stores AFInfo2 in tag `0x00b7` of its type 3 MakerNote. The value is
an unencrypted byte array beginning with a four-character ASCII version.
Multi-byte fields use the MakerNote's embedded TIFF byte order, which can
differ from the enclosing image's byte order.

Exiv2 decodes the following versions:

| Versions | Exiv2 group |
| --- | --- |
| `0100` | `NikonAf2` |
| `0101` | `NikonAf22` |
| `0300`, `0301` | `NikonAf23` |
| `0400`, `0401`, `0402` | `NikonAf24` |

Versions `0200`/`0201` (Nikon 1) and unknown versions remain opaque.

## Field layout

The following offsets are measured from the start of the AFInfo2 byte array.

| Field | 03xx byte offset | 04xx byte offset | Type |
| --- | ---: | ---: | --- |
| Version | 0 | 0 | Four bytes |
| AFDetectionMethod | 4 | 4 | Unsigned byte |
| AFAreaMode | 5 | 5 | Unsigned byte |
| FocusPointSchema | 6 | — | Unsigned byte |
| AFCoordinatesAvailable | 7 | 7 | Unsigned byte |
| AFImageWidth | 42 | 62 | Unsigned 16-bit |
| AFImageHeight | 44 | 64 | Unsigned 16-bit |
| AFAreaXPosition | 46 | 66 | Unsigned 16-bit |
| AFAreaYPosition | 48 | 68 | Unsigned 16-bit |
| AFAreaWidth | 50 | 70 | Unsigned 16-bit |
| AFAreaHeight | 52 | 72 | Unsigned 16-bit |
| PrimaryAFPoint | 56 | — | Unsigned byte |
| FocusResult | — | 74 | Unsigned byte |

X/Y identify the center of the AF area, measured from the top-left corner in
`AFImageWidth` × `AFImageHeight` coordinates. Only use X/Y when
`AFCoordinatesAvailable` is 1 (`Yes`). Exiv2 exposes stored numeric fields even
when the flag is 0. The AF coordinate system need not match the output image
dimensions, particularly when cropping. No orientation transformation is applied.

AF point bitmaps are preserved but are not decoded into point names or grid
labels.

## Usage

```sh
exiv2 -pa --grep NikonAf image.nef
exiv2 -K Exif.NikonAf23.AFAreaXPosition -K Exif.NikonAf23.AFAreaYPosition -Pv image.nef
```

## Reference

See [ExifTool's Nikon.pm](https://github.com/exiftool/exiftool/blob/2200871d9cef988051d2a99d67df3bda6cbb30a8/lib/Image/ExifTool/Nikon.pm)
for the `AFInfo2V0300` and `AFInfo2V0400` layouts and the `0x00b7` version
dispatch table.
