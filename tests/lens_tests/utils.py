import re
import os
import logging
from itertools import groupby

log = logging.getLogger(__name__)


LENS_ENTRY_DEFAULT_RE = re.compile('^\{\s*(?P<lens_id>[0-9]+),\s*"(?P<lens_description>.*)"')

LENS_META_DEFAULT_RE = re.compile(
    (
        # anything at the start
        ".*?"
        # maybe min focal length and hyhpen, surely max focal length e.g.: 24-70mm
        "(?:(?P<focal_length_min>[0-9]+)-)?(?P<focal_length_max>[0-9]+)mm"
        # anything inbetween
        ".*?"
        # maybe short focal length max aperture and hyhpen, surely at least single max aperture e.g.: f/4.5-5.6
        # short and tele indicate apertures at the short (focal_length_min) and tele (focal_length_max) position of the lens
        "(?:(?:f\/)|T)(?:(?P<aperture_max_short>[0-9]+(?:\.[0-9]+)?)-)?(?P<aperture_max_tele>[0-9]+(?:\.[0-9])?)"
        # check if there is a teleconverter pattern e.g. + 1.4x
        "(?:.*?\+.*?(?P<tc>[0-9.]+)x)?"
    )
)


def parse_lens_entry(text, pattern=LENS_ENTRY_DEFAULT_RE):
    """get the ID, and description from a lens entry field
    Expexted input format:
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


# FIXME explain somwhere that lens_is_match(l1,l2) does not imply lens_is_match(l2,l1)
# becuse we don't have short and tele aperture values in exif
def lens_is_match(l1, l2):
    """
    Test if lens l2 is compatible with lens l1,
    assuming we write l1's metadata and apeture_max_short into exif
    """
    return (
        all([l1[k] == l2[k] for k in ["tc", "focal_length_min", "focal_length_max"]])
        and l2["aperture_max_short"] <= l1["aperture_max_short"] <= l2["aperture_max_tele"]
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
    Extract lens information from the lens descritpions array in a maker note cpp file
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
                    log.error(f"Failure extracing metadata from lens description:  {lens_entry[0]}: {lens_entry[1]}.")
                    continue

                lenses.append({"id": lens_entry[0], "desc": lens_entry[1], "meta": meta})
        return lenses
