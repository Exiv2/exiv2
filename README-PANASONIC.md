# Panasonic metadata support

This work extends interpretation of Panasonic JPEG and RW2 metadata through the
existing TIFF and MakerNote implementation. Existing named keys and write
capabilities remain stable. Leica-specific formats, video atoms and additional
RW2 previews are separate work; these changes do not resolve #3282 or #2856.

## Reference and initial audit

The comparison uses [ExifTool Panasonic definitions](https://github.com/exiftool/exiftool/blob/2200871d9cef988051d2a99d67df3bda6cbb30a8/lib/Image/ExifTool/Panasonic.pm)
and its generated tag documentation at the same revision. ExifTool is a reference
for tag meanings, not a runtime dependency or a Panasonic-published specification.

At Exiv2 081bb55f0, 39 documented main-table identifiers are absent and one has
an unnamed placeholder. Unknown TIFF values can already be retained; absence
from this table does not imply that their bytes are discarded.

| ID | Reference name | Encoded type / reference representation | Handling |
| --- | --- | --- | --- |
| `0x0027` | VideoFrameRate | `int16u` | Named field; documented scalar interpretation where available |
| `0x0038` | BatteryLevel | `int16u` | Named field; documented scalar interpretation where available |
| `0x0043` | JPEGQuality | `int16u` | Named field; documented scalar interpretation where available |
| `0x0063` | RecognizedFaceFlags? | `undef[4]` | Uncertain; retain numeric identity |
| `0x0076` | MergedImages | `int16u` | Named field; documented scalar interpretation where available |
| `0x008b` | WBShiftIntelligentAuto | `int16u` | Named field; documented scalar interpretation where available |
| `0x0092` | WBShiftCreativeControl | `int8u` | Named field; documented scalar interpretation where available |
| `0x00a1` | FilterEffect | `rational64u[0.5]` | Bounded interpretation; encoded parent retained |
| `0x00a7` | OutputLUT | `yes` | Expose raw table; no color processing |
| `0x00ac` | MonochromeFilterEffect | `int16u` | Named field; documented scalar interpretation where available |
| `0x00ad` | HighlightShadow | `int16u[2]` | Bounded interpretation; encoded parent retained |
| `0x00af` | TimeStamp | `string` | Named field; documented scalar interpretation where available |
| `0x00b3` | VideoBurstResolution | `int16u` | Named field; documented scalar interpretation where available |
| `0x00b4` | MultiExposure | `int16u` | Named field; documented scalar interpretation where available |
| `0x00b9` | RedEyeRemoval | `int16u` | Named field; documented scalar interpretation where available |
| `0x00bb` | VideoBurstMode | `int32u` | Named field; documented scalar interpretation where available |
| `0x00bc` | DiffractionCorrection | `int16u` | Named field; documented scalar interpretation where available |
| `0x00bd` | FocusBracket | `int16u` | Named field; documented scalar interpretation where available |
| `0x00be` | LongExposureNRUsed | `int16u` | Named field; documented scalar interpretation where available |
| `0x00bf` | PostFocusMerging | `int32u[2]` | Bounded interpretation; encoded parent retained |
| `0x00c1` | VideoPreburst | `int16u` | Named field; documented scalar interpretation where available |
| `0x00c4` | LensTypeMake | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00c5` | LensTypeModel | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00ca` | SensorType | `int16u` | Named field; documented scalar interpretation where available |
| `0x00d1` | ISO | `int32u` | Named field; documented scalar interpretation where available |
| `0x00d2` | MonochromeGrainEffect | `int16u` | Named field; documented scalar interpretation where available |
| `0x00d4` | HybridLogGamma | `int16u` | Named field; documented scalar interpretation where available |
| `0x00d6` | NoiseReductionStrength | `rational64s` | Named field; documented scalar interpretation where available |
| `0x00de` | AFAreaSize | `rational64u[2]` | Named field; documented scalar interpretation where available |
| `0x00e4` | LensTypeModel | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00e8` | MinimumISO | `int32u` | Named field; documented scalar interpretation where available |
| `0x00e9` | AFSubjectDetection | `int16u` | Named field; documented scalar interpretation where available |
| `0x00ee` | DynamicRangeBoost | `int16u` | Named field; documented scalar interpretation where available |
| `0x00f1` | LUT1Name | `string` | Named field; documented scalar interpretation where available |
| `0x00f3` | LUT1Opacity | `int8u` | Named field; documented scalar interpretation where available |
| `0x00f4` | LUT2Name | `string` | Named field; documented scalar interpretation where available |
| `0x00f5` | LUT2Opacity | `int8u` | Named field; documented scalar interpretation where available |
| `0x2003` | TimeInfo | `-` | Bounded interpretation; encoded parent retained |
| `0x8002` | HighlightWarning | `int16u` | Named field; documented scalar interpretation where available |
| `0x8003` | DarkFocusEnvironment | `int16u` | Named field; documented scalar interpretation where available |

Types above describe the reference, including its overridden display formats;
implementation must distinguish encoded TIFF types from signed/packed display
interpretation. In particular, FilterEffect contains two integers rather than a
meaningful mathematical fraction. Do not infer a new writer from this inventory.

## Compatibility baseline

Tests retain established Quality, AFMode, LensType and PanasonicRaw keys, unknown
numeric access, existing JPEG MakerNote reading and TIFF metadata round trips.
Model-dependent enumerations and structured blocks require additional checks.

Earlier work: #1106 was closed after its changes landed through other branches;
#2997 exposes RAW distortion data, while the broader #2077 left uncertain
interpretation unfinished. Neither should be reapplied wholesale.

## Interpretation and preservation

`Exifdatum::print(&image.exifData())` provides the interpreted view. Passing the
complete metadata is required for model-specific AF meanings and binary-block
byte order. `Exifdatum::value()`, `copy()` and the CLI's `-pv`/`-Pv` retain the
encoded values. Add `-u` to include unnamed tags in CLI output. New definitions do not change TIFF field types. Signed shifts
and HighlightShadow are displayed as signed while retaining unsigned encoding.
The two lens-model identifiers have distinct keys: LensTypeModel (0x00c5) and
LensTypeModel2 (0x00e4). They are identifiers, not inferred lens names.

FaceDetInfo displays up to five stored rectangles. FaceRecInfo displays up to
three names, rectangles and age strings; fixed-width strings need no terminating
NUL. Rectangle values are center X/Y and width/height, relative to twice the
thumbnail size (or a 320-pixel-wide image), not necessarily full-image pixels.
Stored positions may be fewer than FacesDetected. Invalid types, short records,
excessive counts or missing MakerNote byte order retain the raw display.

TimeInfo displays a validated packed-decimal date/time and an optional shot
counter. It infers no time zone and creates no standard Exif date tags. Unknown
bytes, including the gap before the counter, remain in the original parent.
FilterEffect is a packed selector in a rational slot, not a fraction; even 0/0
is meaningful. Unrecognized packed settings remain raw.

Structured interpretation uses the existing TIFF-decoded parent values. There
are no derived child tags to merge or serialize, no new IFD groups, and no new
editing API. Reading or printing never replaces the parent bytes. Unrelated
metadata edits retain complete blocks, including uninterpreted suffix bytes.

## Model compatibility and deliberate limits

The FZ10 spot modes are distinguished from later AF-area encodings. New zone
interpretations are limited to documented models; in particular the uncertain
GH6 zone pair remains raw. Without camera identity the earlier ambiguous AF
labels remain available. Existing ambiguous stabilization labels and BurstMode
values 1/2 are retained until model-specific evidence supports changing them.

RecognizedFaceFlags (0x0063) remains unknown. OutputLUT is exposed as raw data
without applying a color transform. Composite lens-name resolution, advanced
scene composites and Leica-specific layouts remain separate investigations.
RW2 preview precedence/filtering and its read-only capability are unchanged.

## Focused checks

Build options/dependencies are described in [README](README.md). With unit tests
enabled, run `build/bin/unit_tests --gtest_filter='Panasonic*'` from the checkout.
Run the CLI regression from the tests directory, pointing at the intended build:

```sh
cd tests
EXIV2_BINDIR="$(pwd)/../build/bin" python3 runner.py --config_file suite.conf \
  bugfixes/github/test_panasonic_metadata.py
```

Tests cover both TIFF byte orders, raw preservation, existing JPEG reads, model
selection, malformed blocks, bounded strings, optional time fields and metadata
growth. Small upstream Panasonic JPEG/RW2 samples provide additional local
comparison evidence; they are not downloaded by the test suite.
