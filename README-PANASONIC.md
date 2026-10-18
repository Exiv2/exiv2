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

| ID | Reference name | Encoded type / reference representation | Initial scope |
| --- | --- | --- | --- |
| `0x0027` | VideoFrameRate | `int16u` | Confirm definition and interpretation |
| `0x0038` | BatteryLevel | `int16u` | Confirm definition and interpretation |
| `0x0043` | JPEGQuality | `int16u` | Confirm definition and interpretation |
| `0x0063` | RecognizedFaceFlags? | `undef[4]` | Uncertain; retain numeric identity |
| `0x0076` | MergedImages | `int16u` | Confirm definition and interpretation |
| `0x008b` | WBShiftIntelligentAuto | `int16u` | Confirm definition and interpretation |
| `0x0092` | WBShiftCreativeControl | `int8u` | Confirm definition and interpretation |
| `0x00a1` | FilterEffect | `rational64u[0.5]` | Preserve raw; bounded interpretation in structured stage |
| `0x00a7` | OutputLUT | `yes` | Expose raw table; no color processing |
| `0x00ac` | MonochromeFilterEffect | `int16u` | Confirm definition and interpretation |
| `0x00ad` | HighlightShadow | `int16u[2]` | Preserve raw; bounded interpretation in structured stage |
| `0x00af` | TimeStamp | `string` | Confirm definition and interpretation |
| `0x00b3` | VideoBurstResolution | `int16u` | Confirm definition and interpretation |
| `0x00b4` | MultiExposure | `int16u` | Confirm definition and interpretation |
| `0x00b9` | RedEyeRemoval | `int16u` | Confirm definition and interpretation |
| `0x00bb` | VideoBurstMode | `int32u` | Confirm definition and interpretation |
| `0x00bc` | DiffractionCorrection | `int16u` | Confirm definition and interpretation |
| `0x00bd` | FocusBracket | `int16u` | Confirm definition and interpretation |
| `0x00be` | LongExposureNRUsed | `int16u` | Confirm definition and interpretation |
| `0x00bf` | PostFocusMerging | `int32u[2]` | Preserve raw; bounded interpretation in structured stage |
| `0x00c1` | VideoPreburst | `int16u` | Confirm definition and interpretation |
| `0x00c4` | LensTypeMake | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00c5` | LensTypeModel | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00ca` | SensorType | `int16u` | Confirm definition and interpretation |
| `0x00d1` | ISO | `int32u` | Confirm definition and interpretation |
| `0x00d2` | MonochromeGrainEffect | `int16u` | Confirm definition and interpretation |
| `0x00d4` | HybridLogGamma | `int16u` | Confirm definition and interpretation |
| `0x00d6` | NoiseReductionStrength | `rational64s` | Confirm definition and interpretation |
| `0x00de` | AFAreaSize | `rational64u[2]` | Confirm definition and interpretation |
| `0x00e4` | LensTypeModel | `int16u` | Expose identifiers; defer composite lens-name mapping |
| `0x00e8` | MinimumISO | `int32u` | Confirm definition and interpretation |
| `0x00e9` | AFSubjectDetection | `int16u` | Confirm definition and interpretation |
| `0x00ee` | DynamicRangeBoost | `int16u` | Confirm definition and interpretation |
| `0x00f1` | LUT1Name | `string` | Confirm definition and interpretation |
| `0x00f3` | LUT1Opacity | `int8u` | Confirm definition and interpretation |
| `0x00f4` | LUT2Name | `string` | Confirm definition and interpretation |
| `0x00f5` | LUT2Opacity | `int8u` | Confirm definition and interpretation |
| `0x2003` | TimeInfo | `-` | Preserve raw; bounded interpretation in structured stage |
| `0x8002` | HighlightWarning | `int16u` | Confirm definition and interpretation |
| `0x8003` | DarkFocusEnvironment | `int16u` | Confirm definition and interpretation |

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
