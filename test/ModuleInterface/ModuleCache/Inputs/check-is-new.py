#!/usr/bin/env python3
#
# check-is-new.py - a more-legible way to read a timestamp test than test(1)
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2018 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
# ----------------------------------------------------------------------------
#
# Compares the mtime of the provided files to a constant "old" reference time.
# Fails if any file is as-old-or-older than old.
#
# ----------------------------------------------------------------------------

import os
import sys

OLD = 1390550700  # 2014-01-24T08:05:00+00:00
for f in sys.argv[1:]:
    if os.stat(f).st_mtime <= OLD:
        print("%s is not new!" % f)
        exit(1)
