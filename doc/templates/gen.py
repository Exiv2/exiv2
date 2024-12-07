#! /usr/bin/env python
import sys
import os
import re
import time

# ----------------------------------------------------------------------
# Settings
vardir = "."
date_format = "%d-%b-%Y"


# ----------------------------------------------------------------------
# functions
def usage():
    print("""Usage: gen.py file.in [...]
    Substitute placeholders in input files with content
    """)


def gen_html(file):
    """Replace variables in the file with their content"""
    text = open(file, encoding=None).read()
    for var in vars:
        vartext = open(f"{vardir}/{var}", encoding=None).read()
        text = text.replace(var, vartext)
    text = last_modified(text)
    return text


def last_modified(text):
    """Substitute variable __last_modified__ with the current date"""
    date = time.strftime(date_format, time.localtime())
    text = text.replace("__last_modified__", date)
    return text


# ----------------------------------------------------------------------
# main
# Check command line arguments
if len(sys.argv) == 1:
    usage()
    sys.exit()

# The input files from the command line
input = sys.argv[1:]

# Get a list of all variables (files in the form __*__) from vardir
vars = os.listdir(vardir)
for i in range(len(vars) - 1, -1, -1):
    if re.match("^__.*__$", vars[i]):
        continue
    del vars[i]
vars.sort()

# Substitute variables in all input files
print(f"Substituting variables {vars}")
for file in input:
    print(f"Processing {file}...")
    text = gen_html(file)
    file = file.replace(".in", "")
    open(file, "w", encoding=None).write(text)
