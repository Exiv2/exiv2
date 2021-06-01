import re
import os
import logging

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
        # maybe short focal length max aperture and hyhpen, surely maxaperture e.g.: f/4.5-5.6
        "f\/(?:(?P<aperture_min>[0-9]+(?:\.[0-9]+)?)-)?(?P<aperture_max>[0-9]+(?:\.[0-9])?)"
        # check if there is a teleconverter pattern e.g. + 1.4x
        "(?:.*?\+.*?(?P<tc>[0-9.]+x))?"
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
    aperture_min = 4.5
    aperture_max = 5.6
    tc = 1.4
    """
    result = pattern.match(text)

    return result.groupdict() if result else None


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
                    log.error(f"Failure extracing metadata from lens description: {lens_entry[1]}.")
                    continue

                lenses.append((*lens_entry, meta))
        return lenses

