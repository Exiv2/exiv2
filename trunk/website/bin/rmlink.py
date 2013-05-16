#! /usr/bin/env python
# Remove links from HTML code
# 05-Jun-05, -ahu.

import re
import sys

# Check command line arguments
if len(sys.argv) != 2:
   print """Usage: rmlink.py <file>
   Remove links from HTML code
   """
   sys.exit()

e = re.compile('<a.*?>(.*?)</a>', re.VERBOSE)
print e.sub(r'\1', open(sys.argv[1]).read())
