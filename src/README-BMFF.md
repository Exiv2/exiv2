# BMFF internal layers {#bmff_internals}

The BMFF implementation uses private, composable layers. Sharing box mechanics
does not make every BMFF format writable: factory routing is unchanged, HEIF
edits still require layout and metadata checks, and AVIF, CR3 and JPEG XL remain
read-only. QuickTime and JPEG 2000 use their existing readers.

## Responsibilities

| Files | Responsibility |
| --- | --- |
| `bmffbox_int.hpp/.cpp` | Checked box headers, source intervals, bounded field cursors, sibling traversal and explicit resource limits. |
| `bmffitem_int.hpp/.cpp` | Standard item descriptions, locations, references, properties and extent resolution. |
| `bmfflayout_int.hpp/.cpp` | Output measurement, placement, approved source relocation maps, bounded streaming and byte verification. |
| `bmffitemwrite_int.hpp/.cpp` | Item-location encoding, file/idat coordinates, offset-width promotion and layout convergence. |
| `heifstructure_int.hpp/.cpp` | HEIF admission, primary metadata selection, brands, Canon extensions, media ownership and relocation allowlists. |
| `heifwrite_int.hpp/.cpp` | HEIF edit selection, shared-data rejection, retained/discarded ranges, compaction and structural verification. |
| `heifimage_int.hpp/.cpp` | Private image implementation, TIFF/XMP interpretation and serialization, metadata precedence, fallback, no-op detection, semantic verification and final transfer. |
| `bmffimage.cpp` | Legacy metadata interpretation and structure printing through the private `BmffLegacyReader` adapter. |

The shared headers are implementation details in `src`, not installed APIs.
`BmffImage` retains its public class layout and existing entry points; its private
friend adapter borrows state without adding members. The existing `boxHandler`
entry point delegates to the adapter for ABI compatibility.

## Reading and ownership

`BmffReader` borrows an open, seekable `BasicIo` and snapshots its size. The input
must remain unchanged and outlive the reader and its cursors. Cursors share the
reader's counters; reading fields can reposition the stream. `BmffSpan` uses
absolute input coordinates and owns no payload bytes. Parsed boxes retain both
the original encoded size field and the physical header length, including a
UUID user type, so structure printing can preserve its original representation.

The box layer requires neither `ftyp` nor `meta`. An adapter recognizes a
container, consumes any format-specific prefix, and visits the remaining child
range. Unknown boxes stay opaque. Size-zero boxes extend to file end, so their
containing range must also end there. Limits are supplied by each caller:
native HEIF uses `HeifLimits`, while the legacy adapter retains its per-root
visit and recursion limits rather than adopting the HEIF budgets.

The native HEIF path calls `parseHeif`, which composes `BmffReader` with
`parseBmffItems` and a HEIF policy. The standard item model needs no brand,
picture handler or primary image. It supports local file-relative and
idat-relative extents, explicit nonzero lengths and the implemented table
versions; it is not a decoder for every item-storage mode. `resolveBmffItems`
checks links and computes absolute spans after source ranges are known.
`readBmffItem` gathers only the selected payload under a caller-supplied limit.

Legacy `BmffImage` reads, structure printing and HEIF fallback all use
`BmffReader` framing and traversal. Legacy field interpretation remains in its
adapter because item-table heuristics, traversal order, Canon UUID/CMT handling,
JPEG XL boxes and optional Brotli decoding have compatibility rules different
from the strict item model. FullBox bit decoding is shared; field admission and
metadata precedence remain with the respective adapters.

## Preparing writes

HEIF selects metadata and retained ranges before constructing output. It rejects
unsupported relocation and edits to shared metadata, and excludes obsolete
metadata bytes from compacted payloads. Moving an opaque box always requires
adapter approval: the generic engine cannot know whether it contains offsets.

`BmffOutputBox` owns its prefix and child nodes. Its segments borrow unchanged
source ranges or immutable replacement vectors; those inputs must outlive
measurement, emission and verification. Input spans and output positions use
separate absolute coordinate systems. `BmffRelocations` translates only ranges
explicitly approved by the adapter.

`layoutBmff` measures nodes, promotes box headers and places them. The item
layer's `layoutBmffItems` regenerates `iloc` fields, converts absolute positions
back to file/idat coordinates and converges after any offset-width promotion.
The generic engine knows no item IDs, brands, codecs or metadata semantics.

`writeBmffLayout` emits to a distinct, empty staging stream with requests bounded
to 64 KiB. `verifyBmffLayout` compares all generated and retained bytes. HEIF
then reparses the prepared output and checks its intended metadata before the
image implementation performs the existing final transfer. The engine does not
transfer or replace the source. A true unchanged write bypasses rewriting in
the image implementation.

## Maintaining the boundaries

Add format recognition, payload interpretation and write eligibility to an
adapter. Add common field mechanics to the item layer only when their semantics
agree; preserve legacy compatibility rules explicitly. Do not infer relocation
safety from successful parsing or silently apply HEIF limits to another reader.

The focused unit suites cover box mechanics, item tables, output layout, legacy
compatibility and HEIF integration. For commands, see the HEIF metadata writing
section of `tests/README-TESTS.md` and the instructions in `fuzz/README.md`.
The legacy hardening tests separately cover malformed container/UUID boundaries
and underlying I/O failures.
