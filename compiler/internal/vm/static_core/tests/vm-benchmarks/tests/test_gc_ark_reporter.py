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

from vmb.gclog.ark.ark_gclog_reporter import ArkGcLogReporter
from vmb.gclog.ark.ark_pause_event import ArkPauseEvent


class ReporterTests(unittest.TestCase):
    def test_generate_report(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=100.0, pause_time_sum=200.0,
                raw_message='', timestamp=1000000
            ),
            ArkPauseEvent(
                gc_name='gc2',
                gc_counter='0123456789',
                gc_cause='cause2',
                gc_collection_type='TENURED',
                gc_time=50.0,
                freed_object_mem=50,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=50.0, pause_time_sum=200.0,
                raw_message='', timestamp=2000000
            )
        ]
        try:
            report = ArkGcLogReporter().generate_report(events)
            self.assertEqual(events[1].timestamp - events[0].timestamp + events[1].gc_time,
                             report['gc_vm_time'])
            self.assertEqual(len(events), report['gc_pause_count'])
            self.assertEqual(sum(map(lambda x: x.pause_time, events)), report['gc_total_time'])
            self.assertEqual(len(events), len(report['gc_pauses']))
            for i, p in enumerate(report['gc_pauses']):
                self.assertEqual(events[i].timestamp, p[0])
                self.assertEqual(events[i].gc_name, p[1])
                self.assertEqual(events[i].pause_time, p[2])
                self.assertEqual(events[i].mem_after + events[i].freed_object_mem +
                                 events[i].freed_large_object_mem, p[3])
                self.assertEqual(events[i].mem_after, p[4])
                self.assertEqual(events[i].mem_total, p[5])
        except KeyError as e:
            unittest.TestCase.fail(self, f'Bad report property: {e}')

    def test_generate_report_no_events(self):
        expected_props = [
            'gc_vm_time', 'gc_name', 'gc_pause_count', 'gc_total_time', 'gc_memory_total_heap',
            'gc_total_bytes_reclaimed', 'gc_avg_bytes_reclaimed', 'gc_avg_time', 'gc_median_time',
            'gc_std_time', 'gc_max_time', 'gc_min_time', 'gc_pct99_time', 'gc_pct95_time',
            'gc_pct50_time', 'gc_pauses', 'gc_avg_interval_time', 'time_unit'
        ]
        expected_props.sort()
        report = ArkGcLogReporter().generate_report([])
        actual_props = [*report.keys()]
        actual_props.sort()
        self.assertListEqual(expected_props, actual_props)

    def test_generate_report_with_adjust_time(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=100
            ),
            ArkPauseEvent(
                gc_name='gc2',
                gc_counter='0123456789',
                gc_cause='cause2',
                gc_collection_type='TENURED',
                gc_time=50.0,
                freed_object_mem=50,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=50.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=200
            )
        ]
        adjust_time = {'vm_start_time': 50, 'fw_start_time': 1000,
                       'fw_end_time': 1500}
        # Ark uses VM timestamps in the log messages, therefore:
        # gc_vm_time equals fw_end_time minus fw_start_time
        gc_vm_time = adjust_time['fw_end_time'] - adjust_time['fw_start_time']
        delta = events[0].timestamp
        report = ArkGcLogReporter().generate_report(events, adjust_time=adjust_time)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))
        for i, p in enumerate(report.get('gc_pauses')):
            self.assertEqual(events[i].timestamp - delta, p[0])

    def test_generate_report_with_adjust_time_in_future(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=100
            )
        ]
        adjust_time = {
            'vm_start_time': 150,
            'fw_start_time': 1000,
            'fw_end_time': 1500
        }
        # A rare case when GC has been triggered before VM prints timestamp
        gc_vm_time = adjust_time['fw_end_time'] - adjust_time['fw_start_time']
        report = ArkGcLogReporter().generate_report(events, adjust_time=adjust_time)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))
        for i, p in enumerate(report.get('gc_pauses')):
            self.assertEqual(0, p[0])

    def test_generate_report_with_late_gc_pause(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=100
            ),
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=2000
            )
        ]
        # A rare case when GC triggered after VM prints timestamp
        # gc_vm_time = first gc ts - last gc ts + last gc duration
        gc_vm_time = events[1].timestamp - events[0].timestamp + events[1].gc_time
        report = ArkGcLogReporter().generate_report(events)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))

    def test_generate_report_with_only_late_gc_pause(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=2000
            )
        ]
        # A rare case when GC  triggered after VM prints timestamp
        # gc_vm_time equals last gc duration
        gc_vm_time = 100
        report = ArkGcLogReporter().generate_report(events)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))

    def test_generate_report_with_late_gc_pause_and_adjust(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=1100
            ),
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=2000
            )
        ]
        adjust_time = {
            'vm_start_time': 50,
            'fw_start_time': 1000,
            'fw_end_time': 1500
        }
        # A rare case when GC  triggered after VM prints timestamp
        # gc_vm_time equals late gc ts plus late gc duration
        gc_vm_time = events[1].timestamp - events[0].timestamp + events[1].gc_time
        report = ArkGcLogReporter().generate_report(events, adjust_time=adjust_time)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))

    def test_generate_report_with_late_gc_pause_and_adjust_in_future(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='', timestamp=100
            ),
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200, mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='', timestamp=2000
            )
        ]
        adjust_time = {'vm_start_time': 250, 'fw_start_time': 1000,
                       'fw_end_time': 1500}
        # A rare case when GC  triggered after VM prints timestamp
        # gc_vm_time equals late gc ts plus late gc duration
        gc_vm_time = events[1].timestamp - events[0].timestamp + 100
        report = ArkGcLogReporter().generate_report(events, adjust_time=adjust_time)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))

    def test_generate_report_with_gc_pause_ending_too_late(self):
        events = [
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=100.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=1100
            ),
            ArkPauseEvent(
                gc_name='gc1',
                gc_counter='0123456789',
                gc_cause='cause1',
                gc_collection_type='TENURED',
                gc_time=100.0,
                freed_object_mem=100,
                freed_large_object_mem=0,
                mem_after=200,
                mem_total=500,
                pause_time=300.0,
                pause_time_sum=200.0,
                raw_message='',
                timestamp=1400
            )
        ]
        adjust_time = {
            'vm_start_time': 250,
            'fw_start_time': 1000,
            'fw_end_time': 1500
        }
        # A rare case when GC triggered after VM prints timestamp
        # gc_vm_time equals late gc ts plus late gc duration
        gc_vm_time = events[1].timestamp - adjust_time['fw_start_time'] + events[1].gc_time
        report = ArkGcLogReporter().generate_report(events, adjust_time=adjust_time)
        self.assertEqual(gc_vm_time, report.get('gc_vm_time'))


if __name__ == '__main__':
    unittest.main()
