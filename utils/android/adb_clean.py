#!/usr/bin/env python3
# adb_reboot.py - Reboots and cleans an Android device. -*- python -*-
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import os

from adb.commands import DEVICE_TEMP_DIR, reboot, rmdir


if __name__ == '__main__':
    if 'SKIP_ANDROID_CLEAN' not in os.environ:
        reboot()
        rmdir(DEVICE_TEMP_DIR)
