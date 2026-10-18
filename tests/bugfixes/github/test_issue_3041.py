# -*- coding: utf-8 -*-

from base64 import b64decode
from pathlib import Path
import struct

from system_tests import CaseMeta, FileDecoratorBase, path


# A locally generated 8x8 JPEG keeps fixture creation independent of codecs.
JPEG_PREVIEW = b64decode(
    "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkS"
    "Ew8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJ"
    "CQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIy"
    "MjIyMjIyMjIyMjIyMjL/wAARCAAIAAgDASIAAhEBAxEB/8QAFQABAQAAAAAAAAAA"
    "AAAAAAAAAAf/xAAUEAEAAAAAAAAAAAAAAAAAAAAA/8QAFAEBAAAAAAAAAAAAAAAA"
    "AAAABP/EABQRAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AI6AaG//2Q=="
)


def make_dng(byte_order, preview, thumbnail, extra_preview=False, malformed=None):
    """Build a reduced IFD0/primary SubIFD layout without JPEG XL compression."""

    def field(tag, kind, values):
        """Encode the TIFF field's value before assigning external offsets."""
        if kind == 2:
            data = values.encode("ascii") + b"\0"
            count = len(data)
        elif kind == 1:
            data = bytes(values)
            count = len(values)
        else:
            data = struct.pack(
                byte_order + str(len(values)) + {3: "H", 4: "I"}[kind], *values
            )
            count = len(values)
        return tag, kind, count, data

    def raster(width, height, compression, photometric, bits, count, reduced):
        """Describe one three-channel image stored in a single TIFF strip."""
        return [
            field(0x00FE, 4, [reduced]),
            field(0x0100, 4, [width]),
            field(0x0101, 4, [height]),
            field(0x0102, 3, [bits] * 3),
            field(0x0103, 3, [compression]),
            field(0x0106, 3, [photometric]),
            field(0x0111, 4, [0]),
            field(0x0115, 3, [3]),
            field(0x0116, 4, [height]),
            field(0x0117, 4, [count]),
            field(0x011C, 3, [1]),
        ]

    # Match the reported directory arrangement; primary compression is immaterial.
    raw_size = 16 * 16 * 3 * 2
    primary = raster(16, 16, 1, 34892, 16, raw_size, 0)
    root = raster(8, 8, 7, 6, 8, len(JPEG_PREVIEW), 1) if preview else primary
    root += [
        field(0x010F, 2, "Exiv2"),
        field(0x0110, 2, "Issue 3041 fixture"),
        field(0xC612, 1, [1, 4, 0, 0]),
        field(0xC613, 1, [1, 1, 0, 0]),
        field(0xC614, 2, "Exiv2 issue 3041 synthetic"),
    ]
    tables = [root]
    if preview:
        root.append(field(0x014A, 4, [0]))
        tables.append(primary)

    # A smaller SubIFD preview tests API ordering independently of directory order.
    if extra_preview:
        smaller = raster(4, 4, 1, 2, 8, 4 * 4 * 3, 1)
        tables.append(smaller)

    if thumbnail:
        tables.append(
            [
                field(0x0100, 4, [8]),
                field(0x0101, 4, [8]),
                field(0x0103, 3, [6]),
                field(0x0201, 4, [0]),
                field(0x0202, 4, [0 if thumbnail == "empty" else len(JPEG_PREVIEW)]),
            ]
        )

    # Reserve all directories before appending payloads and external field values.
    offsets = []
    end = 8
    for table in tables:
        offsets.append(end)
        end += 2 + 12 * len(table) + 4

    output = bytearray(end)
    marker = b"MM" if byte_order == ">" else b"II"
    output[:8] = marker + struct.pack(byte_order + "HI", 42, 8)

    def store(data):
        """Append a word-aligned value and return its file-relative offset."""
        if len(output) % 2:
            output.append(0)
        offset = len(output)
        output.extend(data)
        return offset

    def replace(table, tag, values):
        """Resolve reserved fields after placing their targets."""
        table[:] = [
            field(entry[0], entry[1], values) if entry[0] == tag else entry
            for entry in table
        ]

    jpeg_offset = store(JPEG_PREVIEW) if preview or thumbnail else 0
    raw_offset = store(bytes(raw_size))
    replace(root, 0x0111, [jpeg_offset if preview else raw_offset])
    if preview:
        replace(primary, 0x0111, [raw_offset])
        replace(root, 0x014A, offsets[1:3] if extra_preview else [offsets[1]])
    if extra_preview:
        replace(smaller, 0x0111, [store(bytes(4 * 4 * 3))])
    if thumbnail:
        replace(tables[-1], 0x0201, [0 if thumbnail == "empty" else jpeg_offset])

    # Metadata remains readable while preview reconstruction encounters bad ranges.
    if malformed == "overflow":
        replace(root, 0x0111, [0xFFFFFFF0])
    elif malformed == "oversized":
        replace(root, 0x0111, [jpeg_offset, jpeg_offset])
        replace(root, 0x0117, [10 * len(JPEG_PREVIEW)] * 2)

    # Only IFD0's next link leads to the optional conventional IFD1 thumbnail.
    for index, table in enumerate(tables):
        offset = offsets[index]
        struct.pack_into(byte_order + "H", output, offset, len(table))
        for number, (tag, kind, count, data) in enumerate(sorted(table)):
            encoded = (
                data.ljust(4, b"\0")
                if len(data) <= 4
                else struct.pack(byte_order + "I", store(data))
            )
            struct.pack_into(
                byte_order + "HHI4s",
                output,
                offset + 2 + 12 * number,
                tag,
                kind,
                count,
                encoded,
            )
        next_offset = offsets[-1] if index == 0 and thumbnail else 0
        struct.pack_into(byte_order + "I", output, offset + 2 + 12 * len(table), next_offset)

    return bytes(output)


class CreateDngFixture(FileDecoratorBase):
    """Create a temporary structural DNG fixture, removed by the base decorator."""

    def __init__(self, filename, byte_order=">", preview=True, thumbnail=False,
                 extra_preview=False, malformed=None):
        """Select the directory arrangement used by the decorated test."""
        super().__init__(filename)
        self.byte_order = byte_order
        self.preview = preview
        self.thumbnail = thumbnail
        self.extra_preview = extra_preview
        self.malformed = malformed

    def setUp_file_action(self, expanded_file_name):
        """Write a standalone fixture without network or codec dependencies."""
        Path(expanded_file_name).write_bytes(
            make_dng(self.byte_order, self.preview, self.thumbnail,
                     self.extra_preview, self.malformed)
        )
        return expanded_file_name


def compare_preview_summary(self, i, command, got_stdout, expected_stdout):
    """Require a listed TIFF preview to appear separately in the summary."""
    if i == 0:
        self.assertEqual(got_stdout, expected_stdout)
        return

    self.assertRegex(got_stdout, r"(?m)^MIME type\s+: image/tiff$")
    self.assertRegex(got_stdout, r"(?m)^Image size\s+: 16 x 16$")
    self.assertRegex(got_stdout, r"(?m)^Thumbnail\s+: None$")
    self.assertRegex(got_stdout, r"(?m)^Preview\s+: image/tiff, 8x8 pixels, 426 Bytes$")


@CreateDngFixture("$tmp_path/issue_3041_be.dng")
class DngBigEndianPreviewSummary(metaclass=CaseMeta):
    """Expose the reported reduced-IFD0 preview in the default summary."""

    url = "https://github.com/Exiv2/exiv2/issues/3041"
    filename = path("$tmp_path/issue_3041_be.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    stdout = ["Preview 1: image/tiff, 8x8 pixels, 426 bytes\n", ""]
    compare_stdout = compare_preview_summary


@CreateDngFixture("$tmp_path/issue_3041_le.dng", byte_order="<")
class DngLittleEndianPreviewSummary(metaclass=CaseMeta):
    """Require the same summary behavior for the opposite TIFF byte order."""

    url = "https://github.com/Exiv2/exiv2/issues/3041"
    filename = path("$tmp_path/issue_3041_le.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    stdout = ["Preview 1: image/tiff, 8x8 pixels, 426 bytes\n", ""]
    compare_stdout = compare_preview_summary


@CreateDngFixture("$tmp_path/issue_3041_no_preview.dng", preview=False)
class DngWithoutPreviewSummary(metaclass=CaseMeta):
    """Keep summaries without a thumbnail or another preview unchanged."""

    filename = path("$tmp_path/issue_3041_no_preview.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Check that neither enumeration nor summary invents a preview."""
        if i == 0:
            self.assertEqual(got_stdout, "")
        else:
            self.assertRegex(got_stdout, r"(?m)^Thumbnail\s+: None$")
            self.assertNotRegex(got_stdout, r"(?m)^Preview\s+:")


@CreateDngFixture("$tmp_path/issue_3041_thumbnail.dng", thumbnail=True)
class DngWithConventionalThumbnailSummary(metaclass=CaseMeta):
    """Retain the existing conventional-thumbnail summary even with a preview."""

    filename = path("$tmp_path/issue_3041_thumbnail.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    stdout = [
        "Preview 1: image/jpeg, 8x8 pixels, 286 bytes\n"
        "Preview 2: image/tiff, 8x8 pixels, 426 bytes\n",
        "",
    ]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Keep the JPEG thumbnail line and omit the optional fallback line."""
        if i == 0:
            self.assertEqual(got_stdout, expected_stdout)
        else:
            self.assertRegex(got_stdout, r"(?m)^Thumbnail\s+: image/jpeg, 286 Bytes$")
            self.assertNotRegex(got_stdout, r"(?m)^Preview\s+:")


@CreateDngFixture("$tmp_path/issue_3041_empty_thumbnail.dng", thumbnail="empty")
class DngWithEmptyThumbnailSummary(metaclass=CaseMeta):
    """Use the other preview when conventional thumbnail tags have no payload."""

    filename = path("$tmp_path/issue_3041_empty_thumbnail.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    stdout = ["Preview 1: image/tiff, 8x8 pixels, 426 bytes\n", ""]
    compare_stdout = compare_preview_summary


@CreateDngFixture("$tmp_path/issue_3041_multiple_previews.dng", extra_preview=True)
class DngWithMultiplePreviewsSummary(metaclass=CaseMeta):
    """Select Preview 1 by pixel area, rather than choosing the root IFD."""

    filename = path("$tmp_path/issue_3041_multiple_previews.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    stdout = [
        "Preview 1: image/tiff, 4x4 pixels, 188 bytes\n"
        "Preview 2: image/tiff, 8x8 pixels, 426 bytes\n",
        "",
    ]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        """Require the summary's optional preview to agree with the preview list."""
        if i == 0:
            self.assertEqual(got_stdout, expected_stdout)
        else:
            self.assertRegex(got_stdout, r"(?m)^Thumbnail\s+: None$")
            self.assertRegex(got_stdout, r"(?m)^Preview\s+: image/tiff, 4x4 pixels, 188 Bytes$")


def compare_malformed_summary(self, i, command, got_stdout, expected_stdout):
    """Keep readable summary metadata even when preview enumeration fails."""
    if i == 0:
        self.assertEqual(got_stdout, "")
    else:
        self.assertRegex(got_stdout, r"(?m)^Thumbnail\s+: None$")
        self.assertNotRegex(got_stdout, r"(?m)^Preview\s+:")
        self.assertRegex(got_stdout, r"(?m)^Camera model\s+: Issue 3041 fixture$")
        self.assertRegex(got_stdout, r"(?m)^Exif comment\s+: $")


@CreateDngFixture("$tmp_path/issue_3041_overflow_preview.dng", malformed="overflow")
class DngWithOverflowingPreviewSummary(metaclass=CaseMeta):
    """An overflowing preview range must not turn a readable summary into failure."""

    filename = path("$tmp_path/issue_3041_overflow_preview.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    warning = "Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.\n"
    stderr = [
        warning + "std::overflow_error exception in print action for file $filename:\n"
        "Overflow in addition\n",
        warning,
    ]
    retval = [1, 0]
    compare_stdout = compare_malformed_summary


@CreateDngFixture("$tmp_path/issue_3041_oversized_preview.dng", malformed="oversized")
class DngWithOversizedPreviewSummary(metaclass=CaseMeta):
    """An Exiv2 preview-read error must not suppress the rest of a valid summary."""

    filename = path("$tmp_path/issue_3041_oversized_preview.dng")
    commands = ['$exiv2 -pp "$filename"', '$exiv2 "$filename"']
    warnings = (
        "Warning: Directory Image, entry 0x0111: Strip 0 is outside of the data area; ignored.\n"
        "Warning: Directory Image, entry 0x0111: Strip 1 is outside of the data area; ignored.\n"
    )
    stderr = [
        warnings + "Exiv2 exception in print action for file $filename:\n$kerCorruptedMetadata\n",
        warnings,
    ]
    retval = [1, 0]
    compare_stdout = compare_malformed_summary
