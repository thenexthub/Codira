#!/usr/bin/env python3
#
# make-old.py - /bin/touch that writes a constant "old" timestamp.
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
# Like /bin/touch, but writes a constant "old" timestamp.
#
# ----------------------------------------------------------------------------

import os
import sys

OLD = 1390550700  # 2014-01-24T08:05:00+00:00
for f in sys.argv[1:]:
    os.utime(f, (OLD, OLD))
