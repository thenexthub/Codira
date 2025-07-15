#!/usr/bin/env python3
# check-filelist-abc.py - Fake build to test driver-produced -filelists.
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

import os
import sys

assert sys.argv[1] == '-frontend'

if '-primary-file' in sys.argv:
    primaryFile = sys.argv[sys.argv.index('-primary-file') + 1]
else:
    primaryFile = None

if primaryFile and primaryFile.endswith(".bc"):
    sys.exit()

filelistFile = sys.argv[sys.argv.index('-filelist') + 1]

with open(filelistFile, 'r') as f:
    lines = f.readlines()
    assert (lines[0].endswith("/a.code\n") or
            lines[0].endswith("/a.codemodule\n"))
    assert (lines[1].endswith("/b.code\n") or
            lines[1].endswith("/b.codemodule\n"))
    assert (lines[2].endswith("/c.code\n") or
            lines[2].endswith("/c.codemodule\n"))

if primaryFile:
    print("Command-line primary", os.path.basename(primaryFile))

if '-primary-filelist' in sys.argv:
    primaryFilelistFile = sys.argv[sys.argv.index('-primary-filelist') + 1]
    with open(primaryFilelistFile, 'r') as f:
        lines = f.readlines()
        assert len(lines) == 1
        print("Handled", os.path.basename(lines[0]).rstrip())
elif lines[0].endswith(".codemodule\n"):
    print("Handled modules")
else:
    print("Handled all")


if '-supplementary-output-file-map' in sys.argv:
    supplementaryOutputMapFile = \
        sys.argv[sys.argv.index('-supplementary-output-file-map') + 1]
    with open(supplementaryOutputMapFile, 'r') as f:
        # The output is in DenseMap order, which is unstable, therefore sort.
        lines = sorted(f.readlines())
        for line in lines:
            print("Supplementary", line.rstrip())

if '-num-threads' in sys.argv:
    outputListFile = sys.argv[sys.argv.index('-output-filelist') + 1]
    with open(outputListFile, 'r') as f:
        lines = f.readlines()
        assert lines[0].endswith("/a.o\n") or lines[0].endswith("/a.bc\n")
        assert lines[1].endswith("/b.o\n") or lines[1].endswith("/b.bc\n")
        assert lines[2].endswith("/c.o\n") or lines[2].endswith("/c.bc\n")
    print("...with output!")
