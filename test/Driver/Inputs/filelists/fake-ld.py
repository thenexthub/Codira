#!/usr/bin/env python3
# fake-ld.py - Fake Darwin linker to test driver-produced -filelists.
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# ----------------------------------------------------------------------------

import sys

filelistFile = sys.argv[sys.argv.index('-filelist') + 1]

with open(filelistFile, 'r') as f:
    lines = f.readlines()
    assert lines[0].endswith("/a.o\n")
    assert lines[1].endswith("/b.o\n")
    assert lines[2].endswith("/c.o\n")

print("Handled link")
