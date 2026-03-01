#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#
#

import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch
import pytest
from vmb.cli import Args
from vmb.plugins.hooks.safepoint_checker import Hook


def cmdline(line):
    return patch.object(sys, 'argv', ['vmb'] + line.split())


@pytest.mark.parametrize('file_name, expected', [
    ('safepoint_test0.txt', {}),
    ('safepoint_test1.txt', {'key': 123}),
    ('safepoint_test2.txt', {'key': 123, 'id': 0, 'count': 3}),
])
def test_some(file_name, expected):
    test = TestCase()
    with cmdline(f'all --lang=blah --platform=xxx --safepoint-checker'):
        args = Args()
        hk = Hook(args)
    path = Path(__file__).parent /'resources' / file_name
    test.assertDictEqual(expected, hk.parse_file_to_dict(path))
