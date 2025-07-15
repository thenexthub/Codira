#!/usr/bin/env python3
# adb_push_build_products.py - Push libraries to Android device -*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import sys

from adb_push_built_products.main import main


if __name__ == '__main__':
    sys.exit(main())
