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

import re
from typing import Optional

from vmb.gclog.event_parser import EventParser, LogEntry
from vmb.gclog.ark.ark_gc_patterns import ArkGcPatterns
from vmb.gclog.ark.ark_pause_event import ArkPauseEvent
from vmb.gclog.utils import mem_to_bytes, time_to_ns
from vmb.gclog.utils import ark_datetime_to_nanos as datetime_to_nanos


class ArkPauseParser(EventParser):
    PATTERN_TEXT = (
        rf'{ArkGcPatterns.THREAD_ID}\s+'
        r'I\/gc:\s+'
        rf'{ArkGcPatterns.GC_COUNTER}\s+'
        rf'{ArkGcPatterns.GC_CAUSE}\s+'
        rf'{ArkGcPatterns.TIMESTAMP}\s+'
        rf'{ArkGcPatterns.GC_NAME}\s+'
        r'freed\s+'
        rf'{ArkGcPatterns.OBJS_FREED}\s+'
        rf'{ArkGcPatterns.LOBJS_FREED}\s+'
        rf'{ArkGcPatterns.HEAP_FREE}\s+'
        rf'{ArkGcPatterns.MEM_AFTER_TOTAL}\s+'
        rf'{ArkGcPatterns.PAUSES}'
        rf'{ArkGcPatterns.GC_TOTAL_TIME}'
    )
    PATTERN = re.compile(PATTERN_TEXT)
    PAUSE_PATTERN = re.compile(ArkGcPatterns.PAUSE_TIME)

    def parse(self, line: str) -> Optional[LogEntry]:
        m = self.PATTERN.search(line)
        if m:
            pause_time_max = 0.0
            pause_time_sum = 0.0
            for pause in self.PAUSE_PATTERN.findall(m.group('pauses')):
                if pause:
                    t = time_to_ns(float(pause[0]), pause[1])
                    pause_time_sum += t
                    pause_time_max = max(pause_time_max, t)

            ev = ArkPauseEvent(
                raw_message=line,
                gc_name=m.group('gc_name'),
                timestamp=datetime_to_nanos(m.group('timestamp')),
                freed_object_mem=mem_to_bytes(
                    int(m.group('obj_mem_freed')),
                    m.group('obj_mem_units')
                ),
                freed_large_object_mem=mem_to_bytes(
                    int(m.group('large_obj_mem_freed')),
                    m.group('large_obj_mem_units')
                ),
                mem_after=mem_to_bytes(
                    int(m.group('mem_after')),
                    m.group('mem_after_units')
                ),
                mem_total=mem_to_bytes(
                    int(m.group('mem_total')),
                    m.group('mem_total_units')
                ),
                pause_time=pause_time_max,
                pause_time_sum=pause_time_sum,
                gc_time=time_to_ns(
                    float(m.group('gc_time')),
                    m.group('gc_time_units')
                ),
                gc_cause=m.group('gc_cause'),
                gc_collection_type=m.group('gc_collection_type'),
                gc_counter=m.group('gc_counter')
            )
            return ev
        return None


class ArkSyslogPauseParser(ArkPauseParser):
    PATTERN_TEXT = (
        rf'{ArkGcPatterns.SYSLOG_GC_ENTRY}'
        r'(?P<gc_counter>\d+)?\s*'
        rf'{ArkGcPatterns.GC_CAUSE}\s+'
        rf'{ArkGcPatterns.TIMESTAMP}\s+'
        rf'{ArkGcPatterns.GC_NAME}\s+'
        r'freed\s+'
        rf'{ArkGcPatterns.OBJS_FREED}\s+'
        rf'{ArkGcPatterns.LOBJS_FREED}\s+'
        rf'{ArkGcPatterns.HEAP_FREE}\s+'
        rf'{ArkGcPatterns.MEM_AFTER_TOTAL}\s+'
        rf'{ArkGcPatterns.PAUSE_TIME}\s+'
        rf'{ArkGcPatterns.GC_TOTAL_TIME}'
    )
    PATTERN = re.compile(PATTERN_TEXT)
