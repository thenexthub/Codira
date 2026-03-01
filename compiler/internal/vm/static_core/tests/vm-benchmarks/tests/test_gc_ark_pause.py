#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
from datetime import datetime

from vmb.gclog.ark.ark_pause_parser import ArkPauseParser
from vmb.gclog.ark.ark_pause_event import ArkPauseEvent


class PauseTests(unittest.TestCase):
    def test_pause_stw(self):
        current_ts = datetime(datetime.today().year, 3, 10, 12, 5, 20, 514000)
        dt = current_ts.strftime('%b %d %H:%M:%S')
        ms = int(current_ts.microsecond / 1000)
        line = f'[TID 000a4c] I/gc: [14] [TENURED (Threshold)] {dt}.{ms} ' \
            'G1 GC freed 4MB, 0B LOS objects, 82% free, 46MB/256MB,  ' \
            'phase: COMMON_PAUSE paused: 335.100us phase: REMARK_PAUSE paused: 28.200us ' \
            'total: 786.200us'
        ev = ArkPauseParser().parse(line)
        self.assertTrue(isinstance(ev, ArkPauseEvent), 'PauseEvent should be emitted')
        pev: ArkPauseEvent = ArkPauseEvent.from_instance(ev)
        self.assertEqual(line, pev.raw_message, 'GC message is correct')
        self.assertEqual('G1', pev.gc_name, 'GC name is correct')
        self.assertEqual((int(current_ts.timestamp()) * 1000 + ms) * 1000 * 1000, pev.timestamp,
                         'timestamp is correct')
        self.assertEqual(4 * 1024 * 1024, pev.freed_object_mem, 'freed objects memory is correct')
        self.assertEqual(0, pev.freed_large_object_mem, 'freed large objects memory is correct')
        self.assertEqual(46 * 1024 * 1024, pev.mem_after, 'memory after gc is correct')
        self.assertEqual(256 * 1024 * 1024, pev.mem_total, 'memory total is correct')
        self.assertEqual(335100.0, pev.pause_time, 'pause time is correct')
        self.assertEqual(363300.0, pev.pause_time_sum, 'pause sum time is correct')
        self.assertEqual(786200, pev.gc_time, 'gc time is correct')
        self.assertEqual('Threshold', pev.gc_cause)
        self.assertEqual('TENURED', pev.gc_collection_type)
        self.assertEqual('14', pev.gc_counter)


if __name__ == '__main__':
    unittest.main()
