#!/usr/bin/env python3
#
# Usage: PrintResponseFile.py [CodiraCommandLine]

import sys

# Grab language-frontend arguments. argv[0] is python script, argv[1] is
# language-frontend path, the remaining args are what needs to be used.
cmd = sys.argv[2:]

# Print quoted command-line as response file.
for c in cmd:
    print('"{}"'.format(c))
