#!/usr/bin/env python3

# Utility for generating a fuzzing dictionary for Exiv2.
# See README.md (in this directory) for more information.

import sys
import json

# If `c` is not a printable ASCII character then escape it.
# For example, a zero byte is converted to '\x00'.
def escapeChar(c):
    if c == '\\':
        return '\\\\'
    if c == '"':
        return '\\"'
    if c.isascii() and c.isprintable():
        return c
    return f'\\x{ord(c):02X}'

def escapeString(str):
    return ''.join(map(escapeChar, bytes(str, 'utf-8')))

if len(sys.argv) < 2:
    print("usage: mkdict.py dict.json")
    sys.exit(1)

with open(sys.argv[1], 'r', encoding=None) as f:
    dict_json = json.loads(f.read())
    tuples = dict_json["#select"]["tuples"]
    for r in tuples:
        print(f"\"{escapeString(r[0])}\"")
