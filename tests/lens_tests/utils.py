import re
import os
import logging
import math
from itertools import groupby

log = logging.getLogger(__name__)


LENS_ENTRY_DEFAULT_RE = re.compile('^\{\s*(?P<lens_id>[0-9]+),\s*"(?P<lens_description>.*)"')

LENS_META_DEFAULT_RE = re.compile(
    (
        # anything at the start
        ".*?"
        # maybe min focal length and hyphen, surely max focal length e.g.: 24-70mm
        "(?:(?P<focal_length_min>[0-9]+)-)?(?P<focal_length_max>[0-9]+)mm"
        # anything in-between
        ".*?"
        # maybe short focal length max aperture and hyphen, surely at least single max aperture e.g.: f/4.5-5.6
        # short and tele indicate apertures at the short (focal_length_min) and tele (focal_length_max) position of the lens
        "(?:(?:f\/)|T|F)(?:(?P<aperture_max_short>[0-9]+(?:\.[0-9]+)?)-)?(?P<aperture_max_tele>[0-9]+(?:\.[0-9])?)"
        # check if there is a teleconverter pattern e.g. + 1.4x
        "(?:.*?\+.*?(?P<tc>[0-9.]+)x)?"
    )
)


def aperture_to_raw_exif(aperture):
    # see https://github.com/exiftool/exiftool/blob/master/lib/Image/ExifTool/Canon.pm#L9678
    """Transform aperture value to Canon maker note style hex format."""
    # for apertures < 1 the below is negative
    num = math.log(aperture) * 2 / math.log(2)

    # temporarily make the number positive
    if num < 0:
        num = -num
        sign = -1
    else:
        sign = 1

    val = int(num)
    frac = num - val

    if abs(frac - 0.33) < 0.05:
        frac = 0x0C
    elif abs(frac - 0.67) < 0.05:
        frac = 0x14
    else:
        frac = int(frac * 0x20 + 0.5)

    return sign * (val * 0x20 + frac)


def raw_exif_to_aperture(raw):
    """The inverse operation of aperture_to_raw_exif"""
    val = raw
    if val < 0:
        val = -val
        sign = -1
    else:
        sign = 1

    frac = val & 0x1F
    val -= frac
    # Convert 1/3 and 2/3 codes
    if frac == 0x0C:
        frac = 0x20 / 3
    elif frac == 0x14:
        frac = 0x40 / 3

    ev = sign * (val + frac) / 0x20
    return math.exp(ev * math.log(2) / 2)


def parse_lens_entry(text, pattern=LENS_ENTRY_DEFAULT_RE):
    """
    get the ID, and description from a lens entry field

    Expected input format:
    { 748, "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x" }
    We return a dict of:
    lens_id = 748
    lens_description = "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"
    """
    result = pattern.match(text)
    return result.groups() if result else None


def extract_meta(text, pattern=LENS_META_DEFAULT_RE):
    """
    Extract metadata from lens description.

    Input expected in the form of e.g. "Canon EF 100-400mm f/4.5-5.6L IS II USM + 1.4x"
    We return a dict of:
    focal_length_min = 100
    focal_length_max = 400
    aperture_max_short = 4.5
    aperture_max_tele = 5.6
    tc = 1.4
    """
    result = pattern.match(text)

    if not result:
        # didn't match
        return None

    ret = result.groupdict()
    # set min to max value if we didn't get a range but a single value
    ret["focal_length_min"] = int(ret["focal_length_min"] or ret["focal_length_max"])
    ret["focal_length_max"] = int(ret["focal_length_max"])
    ret["aperture_max_short"] = float(ret["aperture_max_short"] or ret["aperture_max_tele"])
    ret["aperture_max_tele"] = float(ret["aperture_max_tele"])
    ret["tc"] = float(ret["tc"] or 1)
    return ret


def lens_is_match(l1, l2):
    """
    Test if lens l2 is compatible with lens l1

    This assumes we write l1's metadata and pick its 'aperture_max_short' value
    as the maximum aperture value to write into exif.
    Normally the canon maker note holds the max aperture of the lens at the focal length
    the picture was taken at. Thus for a f/4-6.3 lens, this value could be anywhere in that range.
    """
    # the problem is that the round trip transformation isn't exact
    # so we need to account for this here as well to not define a target
    # which isn't achievable for exiv2
    reconstructed_aperture = raw_exif_to_aperture(aperture_to_raw_exif(l1["aperture_max_short"] * l1["tc"]))
    return all(
        [
            l1["focal_length_min"] * l1["tc"] == l2["focal_length_min"] * l2["tc"],
            l1["focal_length_max"] * l1["tc"] == l2["focal_length_max"] * l2["tc"],
            (l2["aperture_max_short"] * l2["tc"]) - 0.1
            <= reconstructed_aperture
            <= (l2["aperture_max_tele"] * l2["tc"]) + 0.1,
        ]
    )


def make_test_cases(lenses):
    """
    Creates a test case for each lens
    Main job of this function is to collect all ambiguous lenses and define a test target
    as the " *OR* " joined string of all ambiguous lens descriptions
    """
    test_cases = []
    for lens_id, group in groupby(lenses, lambda x: x["id"]):
        lens_group = list(group)
        test_cases += [
            {
                **lens["meta"],
                "id": lens["id"],
                "desc": lens["desc"],
                "target": " *OR* ".join([l["desc"] for l in lens_group if lens_is_match(lens["meta"], l["meta"])]),
            }
            for lens in lens_group
        ]
    return test_cases


def extract_lenses_from_cpp(filename, start_pattern):
    """
    Extract lens information from the lens descriptions array in a maker note cpp file
    filename: path to cpp file
    start_pattern: start_pattern == line.strip() should return True for
                   the starting line of the array containing the lenses.
    returns: a list of lens entries containing a tuple of the form:
             (lens ID, lens description, metadata dictionary)
             for content of metadata see extract_meta() function.
    """
    lenses = []
    with open(filename, "r") as f:
        in_lens_array = False

        for line in f.readlines():
            stripped = line.strip()

            if stripped == start_pattern:
                in_lens_array = True
                continue

            if stripped == "};":
                in_lens_array = False
                continue

            if in_lens_array:
                lens_entry = parse_lens_entry(stripped)
                if not lens_entry:
                    log.error(f"Failure parsing lens entry: {stripped}.")
                    continue

                if lens_entry[1] == "n/a":
                    continue

                meta = extract_meta(lens_entry[1])
                if not meta:
                    log.error(f"Failure extracting metadata from lens description:  {lens_entry[0]}: {lens_entry[1]}.")
                    continue

                lenses.append({"id": lens_entry[0], "desc": lens_entry[1], "meta": meta})
        return lenses
