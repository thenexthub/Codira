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

# flake8: noqa
# pylint: skip-file

import pytest  # type: ignore
from unittest import TestCase
from vmb.log import LogParser

LOG = '''
[2025-06-26T10:43:16.000Z] Starting RUN phase...
[2025-06-26T10:43:16.000Z] Starting bench (1/4): ArraySort_sort
[2025-06-26T10:43:16.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/es2panda --foo=bar
[2025-06-26T10:43:20.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/es2panda --foo=bar
[2025-06-26T10:43:27.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/ark --foo=bar blah
[2025-06-26T10:43:36.000Z] 1750923807597 Startup execution started: 1750923807597
1750923807600 Starting ArraySort_sort @ distribution="sorted";size=100...
1750923810922 Tuning: 131072 ops, 7865.90576171875 ns/op => 127130 reps
1750923812301 Warmup 1:381390 ops, 3615.7214399958048 ns/op
1750923814472 Iter 1:381390 ops, 2808.149138676945 ns/op
1750923816539 Benchmark result: ArraySort_sort 2740.851446900373

[2025-06-26T10:43:36.000Z] ArraySort_sort: 2740.851447
[2025-06-26T10:43:36.000Z] ArraySort_sort total time: 19.801197
[2025-06-26T10:43:36.000Z] Starting bench (2/4): ArraySort_sort_1
[2025-06-26T10:43:36.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/es2panda --foo=bar blah
[2025-06-26T10:43:57.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/ark --foo=bar blah
[2025-06-26T10:43:57.000Z] 1750923827323 Startup execution started: 1750923827323
1750923827327 Starting ArraySort_sort_1 @ distribution="sorted_reversed";size=100...
1750923832013 Tuning: 131072 ops, 12321.47216796875 ns/op => 81159 reps
1750923833178 Warmup 1:162318 ops, 7171.108564669353 ns/op
1750923834269 Warmup 2:162318 ops, 6715.213346640545 ns/op
1750923835355 Iter 1:162318 ops, 6684.409615692653 ns/op
1750923836410 Iter 2:162318 ops, 6499.587230005298 ns/op
1750923837412 Iter 3:162318 ops, 6173.067681957638 ns/op
1750923837413 Benchmark result: ArraySort_sort_1 6452.354842551863

[2025-06-26T10:43:57.000Z] ArraySort_sort_1: 6452.354843
[2025-06-26T10:43:57.000Z] ArraySort_sort_1 total time: 20.872486
[2025-06-26T10:43:57.000Z] Starting bench (3/4): ArraySort_baseline
[2025-06-26T10:43:57.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/es2panda --foo=bar
[2025-06-26T10:44:08.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/ark --foo=bar
[2025-06-26T10:44:13.000Z] 1750923849063 Startup execution started: 1750923849062
1750923849069 Starting ArraySort_baseline @ distribution="sorted";size=100...
1750923851851 Tuning: 1048576 ops, 1080.5130004882813 ns/op => 925486 reps
1750923853004 Iter 1:925486 ops, 1245.8319196616696 ns/op
1750923853005 Benchmark result: ArraySort_baseline 1245.8319196616696

[2025-06-26T10:44:13.000Z] ArraySort_baseline: 1245.831920
[2025-06-26T10:44:13.000Z] ArraySort_baseline total time: 15.591799
[2025-06-26T10:44:13.000Z] Starting bench (4/4): ArraySort_baseline_1
[2025-06-26T10:44:13.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/es2panda --foo=bar
[2025-06-26T10:44:24.000Z] \\time -v env AB=/foo/bar/lib /foo/bar/bin/ark --foo=bar blah
[2025-06-26T10:44:28.000Z] 1750923864469 Startup execution started: 1750923864469
1750923864472 Starting ArraySort_baseline_1 @ distribution="sorted_reversed";size=100...
1750923867253 Tuning: 1048576 ops, 1119.6136474609375 ns/op => 893165 reps
1750923868308 Iter 1:893165 ops, 1180.0731107913991 ns/op
1750923868309 Benchmark result: ArraySort_baseline_1 1180.0731107913991

[2025-06-26T10:44:28.000Z] ArraySort_baseline_1: 1180.073111
[2025-06-26T10:44:28.000Z] ArraySort_baseline_1 total time: 15.304288
[2025-06-26T10:44:28.000Z] Run took 0:01:11.570169
'''


def test_log():
    test = TestCase()
    parser = LogParser(LOG)
    parser.parse()
    report, cmds = parser.analyze()
    test.assertEqual(cmds, ['es2panda', 'es2panda', 'ark'])
    test.assertEqual(len(report), 4)
    test.assertEqual(report[0], [1, 'ArraySort_sort', 20, 4, 7, 9])
    test.assertEqual(report[1], [2, 'ArraySort_sort_1', 21, 21, 0, 0])
    test.assertEqual(report[2], [3, 'ArraySort_baseline', 16, 11, 0, 5])
    test.assertEqual(report[3], [4, 'ArraySort_baseline_1', 15, 11, 0, 4])

