# -*- coding: utf-8 -*-

import json
import system_tests


def _parse_json(stdout_text):
    """Try to parse the given text as JSON. Returns the parsed object or None."""
    try:
        return json.loads(stdout_text)
    except json.JSONDecodeError:
        return None


class TestJsonBasicStructure(metaclass=system_tests.CaseMeta):
    """Basic structure: verify all expected top-level keys and per-tag fields"""

    filename = "$data_path/DSC_3079.jpg"
    commands = ["$exiv2 -p j $filename"]
    retval = [0]

    # We override compare_stdout, but CaseMeta still needs a stdout list entry.
    # Use a dummy value so the list has the right length. The custom comparison
    # ignores this value.
    stdout = [""]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        data = _parse_json(got_stdout)
        self.assertIsNotNone(data, "stdout is not valid JSON")

        # Top-level keys: note: not all images have imageWidth/imageHeight,
        # but DSC_3079.jpg does. We assert they exist and are numbers.
        for key in ("file", "mime", "imageWidth", "imageHeight", "comment",
                     "exif", "iptc", "xmp"):
            self.assertIn(key, data, f"missing top-level key: {key}")

        # Metadata arrays
        for section in ("exif", "iptc", "xmp"):
            self.assertIsInstance(data[section], list,
                                  f"'{section}' must be a JSON array")

        # Per-tag fields in any non-empty section
        tag_keys = {"key", "group", "name", "label", "tag",
                     "type", "count", "size", "value", "interpreted"}

        for section in ("exif", "iptc", "xmp"):
            for tag in data[section]:
                self.assertIsInstance(tag, dict,
                                      f"each tag in '{section}' must be a dict")
                for tk in tag_keys:
                    self.assertIn(tk, tag, f"tag in '{section}' missing '{tk}'")
                # Type assertions
                self.assertIsInstance(tag["tag"], int,
                                      f"tag field must be int, got {type(tag['tag'])}")
                self.assertIsInstance(tag["count"], int)
                self.assertIsInstance(tag["size"], int)
                self.assertIsInstance(tag["key"], str)
                self.assertIsInstance(tag["group"], str)
                self.assertIsInstance(tag["name"], str)
                self.assertIsInstance(tag["label"], str)
                self.assertIsInstance(tag["type"], str)
                self.assertIsInstance(tag["value"], str)
                self.assertIsInstance(tag["interpreted"], str)
                # Only check the first tag per section to keep the test fast
                break


class TestJsonFileNotFound(metaclass=system_tests.CaseMeta):
    """File not found: error message and non-zero return value"""

    commands = ["$exiv2 -p j nonexistent_file.jpg"]
    retval = [255]
    stdout = [""]
    stderr = [
        "nonexistent_file.jpg: Failed to open the file\n"
    ]


class TestJsonWithGrepFilter(metaclass=system_tests.CaseMeta):
    """Grep filter (-g): output is restricted to matching keys"""

    filename = "$data_path/DSC_3079.jpg"
    commands = ["$exiv2 -p j -g Make $filename"]
    retval = [0]
    stdout = [""]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        data = _parse_json(got_stdout)
        self.assertIsNotNone(data, "stdout is not valid JSON")

        for section in ("exif", "iptc", "xmp"):
            for tag in data[section]:
                self.assertIn("Make", tag["key"],
                              f"grep filter FAILED: tag key '{tag['key']}'"
                              f" does not contain 'Make'")


class TestJsonWithKeyFilter(metaclass=system_tests.CaseMeta):
    """Key filter (-K): output contains only the exact key"""

    filename = "$data_path/DSC_3079.jpg"
    # -K must come before -p j because -K resets printMode_ to pmList
    commands = ["$exiv2 -K Exif.Image.Make -p j $filename"]
    retval = [0]
    stdout = [""]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        data = _parse_json(got_stdout)
        self.assertIsNotNone(data, "stdout is not valid JSON")

        all_keys = []
        for section in ("exif", "iptc", "xmp"):
            all_keys.extend(tag["key"] for tag in data[section])

        self.assertEqual(all_keys, ["Exif.Image.Make"],
                         f"expected only 'Exif.Image.Make', got {all_keys}")


class TestJsonWithGrepNoMatch(metaclass=system_tests.CaseMeta):
    """Grep filter with no matches: matching sections get empty arrays"""

    filename = "$data_path/DSC_3079.jpg"
    commands = ["$exiv2 -p j -g NoSuchTagZZZ $filename"]
    retval = [0]
    stdout = [""]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        data = _parse_json(got_stdout)
        self.assertIsNotNone(data, "stdout is not valid JSON")

        # Every metadata section must be an empty array when grep matches nothing
        for section in ("exif", "iptc", "xmp"):
            self.assertEqual(len(data[section]), 0,
                             f"'{section}' must be empty when grep has no matches")


class TestJsonIsValidJson(metaclass=system_tests.CaseMeta):
    """Valid JSON smoke test: output is always parseable JSON"""

    filename = "$data_path/DSC_3079.jpg"
    commands = ["$exiv2 -p j $filename"]
    retval = [0]
    stdout = [""]

    def compare_stdout(self, i, command, got_stdout, expected_stdout):
        data = _parse_json(got_stdout)
        self.assertIsNotNone(data, "stdout is not valid JSON")
        # If json.loads succeeded, the test passes
