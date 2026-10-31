# Nikon AFInfo2 geometry

Nikon stores AFInfo2 in tag `0x00b7` of its type 3 MakerNote. The value is
an unencrypted byte array beginning with a four-character ASCII version.
Multi-byte fields use the MakerNote's embedded TIFF byte order, which can
differ from the enclosing image's byte order.

Exiv2 already decodes versions `0100` (`NikonAf2`) and `0101` (`NikonAf22`).
This change adds `0300`/`0301` (`NikonAf23`, Expeed 6 cameras such as the Z6/Z7
and D780) and `0400`/`0401`/`0402` (`NikonAf24`, Expeed 7 cameras such as the
Z8/Z9, Zf and Z6III). Unknown versions remain opaque.

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
when the flag is 0; ExifTool conditionally omits those coordinates. The AF
coordinate system need not match the output image dimensions, particularly
when cropping. No orientation transformation is applied.

For example, from the repository root:

```sh
build/bin/exiv2 -pa --grep NikonAf test/data/NikonZ6.exv
build/bin/exiv2 -K Exif.NikonAf23.AFAreaXPosition -K Exif.NikonAf23.AFAreaYPosition -Pv image.nef
```

The Z6 fixture yields image dimensions 6048 × 4024, center (4590, 2580), and
area dimensions 285 × 308, matching ExifTool.

The change uses Exiv2's existing binary-array reader/writer. Uninterpreted
ranges are retained, including the camera-dependent AF point bitmaps. Decoding
those bitmaps into point names and ExifTool's derived horizontal/vertical grid
labels is outside this change. Nikon 1 versions `0200`/`0201` have no geometry
fields in ExifTool's layout and remain opaque.

## Sources

The sources were retrieved from GitHub source archives because Git clone
failed on DNS in this environment. Git archive headers record these revisions:

- [ExifTool Nikon.pm](https://github.com/exiftool/exiftool/blob/2200871d9cef988051d2a99d67df3bda6cbb30a8/lib/Image/ExifTool/Nikon.pm):
  `AFInfo2V0300`, `AFInfo2V0400`, and the `0x00b7` dispatch table.
- [Exiv2 source](https://github.com/Exiv2/exiv2/tree/081bb55f06c931bf61662749fd2e410e82c10cfa):
  `src/makernote_int.cpp`, `src/tiffimage_int.cpp`, `src/nikonmn_int.cpp`.

The local `upstream-snapshot` branch retains the source import. The squashed
implementation on `feature/nikon-afinfo2` is based directly on the original
upstream commit above, whose Git tree matches that import exactly.

## Validation

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DEXIV2_ENABLE_NLS=OFF -DEXIV2_BUILD_SAMPLES=ON
cmake --build build -j 8
cd tests
python3 runner.py bugfixes/github/test_nikon_afinfo2.py
python3 runner.py bugfixes/github/test_issue_646.py
python3 runner.py regression_tests/test_regression_allfiles.py
```

The new system tests cover seven versions with both outer and embedded TIFF
byte orders, the real Z6 fixture, coordinate availability, unknown versions,
truncated blocks, and writes that preserve uninterpreted bytes. ExifTool also independently agrees on all six geometry fields across 14 generated
version/byte-order fixtures. The existing
D850 test checks the `0101` layout remains unchanged. The all-files regression
suite covers 329 fixtures; the two Z6 reference outputs are updated to
show the newly decoded tags.
