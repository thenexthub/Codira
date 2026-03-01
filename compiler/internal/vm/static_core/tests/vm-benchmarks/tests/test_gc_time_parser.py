#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import unittest

from vmb.gclog.fw_time_parser import FwTimeParser


class FwTimeParserTests(unittest.TestCase):

    def test_parse_text(self):
        fw_start_time = 1624469473444
        fw_end_time = 1624469503456
        vm_start_time = 116359999212857
        format1 = f'''
1624469503548 INFO [Host] - VM 1/1 output
{fw_start_time} INFO - Startup execution started: {vm_start_time}
1624469476451 INFO - Warmup 1: 8387 ops, 357863.083105 ns/op
1624469479452 INFO - Warmup 2: 9280 ops, 323313.791272 ns/op
1624469488454 INFO - Iter 1: 8982 ops, 333967.421176 ns/op
1624469491454 INFO - Iter 2: 9100 ops, 329618.112747 ns/op
1624469494454 INFO - Iter 3: 9112 ops, 329214.669008 ns/op
{fw_end_time} INFO - Benchmark result: TestBench1 327044.073024'''
        format2 = f'''
1624469503548 [Host] - VM 1/1 output
{fw_start_time} Startup execution started: {vm_start_time}
1624469476451 Warmup 1: 8387 ops, 357863.083105 ns/op
1624469488454 Iter 1: 8982 ops, 333967.421176 ns/op
{fw_end_time} Benchmark result: TestBench2 327044.073024'''

        for text in (format1, format2):
            result = FwTimeParser().parse_text(text)
            self.assertIsNotNone(result, 'Parsing fw time failed')
            self.assertEqual(fw_start_time * 1000.0 * 1000.0,
                             result.get('fw_start_time'))
            self.assertEqual(fw_end_time * 1000.0 * 1000.0,
                             result.get('fw_end_time'))
            self.assertEqual(vm_start_time,
                             result.get('vm_start_time'))


if __name__ == '__main__':
    unittest.main()
