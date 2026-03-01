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

from typing import Any, Iterable, List, Dict, Optional
from statistics import mean, median, stdev

from vmb.gclog.empty_gclog_parser import EmptyGcLogParser
from vmb.gclog.gclog_reporter import LogReporter
from vmb.gclog.event_parser import LogEntry
from vmb.gclog.ark.ark_pause_event import ArkPauseEvent
from vmb.gclog.utils import percentile


def filter_pause_events(events: Iterable[LogEntry]) -> List[ArkPauseEvent]:
    pause_events = [e for e in events if isinstance(e, ArkPauseEvent)]
    return [ArkPauseEvent.from_instance(e) for e in pause_events]


def filter_timestamped_events(
        events: Iterable[LogEntry]) -> Iterable[LogEntry]:
    return filter(lambda x: x.timestamp > -1, events)


def pause_time(ev: ArkPauseEvent) -> float:
    return ev.pause_time


def pause_time_sum(ev: ArkPauseEvent) -> float:
    return ev.pause_time_sum


def pause_mem_delta(ev: ArkPauseEvent) -> int:
    return ev.freed_object_mem + ev.freed_large_object_mem


def event_timestamp(ev: ArkPauseEvent) -> int:
    return ev.timestamp


def pause_log(ev: ArkPauseEvent) -> List[Any]:
    return [
        int(ev.timestamp),
        ev.gc_name,
        ev.pause_time,
        ev.mem_after + pause_mem_delta(ev),
        ev.mem_after,
        ev.mem_total
    ]


class ArkGcLogReporter(LogReporter):
    def generate_report(self, events: Optional[List[LogEntry]] = None,
                        **options: Any) -> Dict[str, Any]:
        pauses = filter_pause_events(events) if events else []

        if len(pauses) < 1:
            return EmptyGcLogParser.EmptyGcReporter().generate_report()

        sorted_pause_times = sorted(map(pause_time, pauses))
        gc_total_time = sum(sorted_pause_times)
        gc_total_time_sum = sum(map(pause_time_sum, pauses))

        sorted_by_ts = sorted(pauses, key=event_timestamp)

        gc_vm_time = 0.0
        adjust_time = options.get('adjust_time')
        if adjust_time is not None:
            gc_vm_time = \
                adjust_time['fw_end_time'] - adjust_time['fw_start_time']
            ts_delta = sorted_by_ts[0].timestamp
            for p in sorted_by_ts:
                p.timestamp -= ts_delta

        gc_vm_time = max(sorted_by_ts[-1].timestamp - sorted_by_ts[0].timestamp
                         + sorted_by_ts[-1].gc_time, gc_vm_time)

        ts_diffs = [j.timestamp - i.timestamp
                    for i, j
                    in zip(sorted_by_ts[:-1], sorted_by_ts[1:])]
        avg_interval_time = mean(ts_diffs) if len(ts_diffs) > 0 else 0

        return {
            'gc_vm_time': gc_vm_time,
            'gc_name': pauses[0].gc_name,
            'gc_pause_count': len(pauses),
            'gc_total_time': gc_total_time,
            'gc_total_time_sum': gc_total_time_sum,
            'gc_memory_total_heap': pauses[0].mem_total,
            'gc_total_bytes_reclaimed': sum(map(pause_mem_delta, pauses)),
            'gc_avg_bytes_reclaimed': int(mean(map(pause_mem_delta, pauses))),
            'gc_avg_time': mean(sorted_pause_times),
            'gc_median_time': median(sorted_pause_times),
            'gc_std_time': stdev(sorted_pause_times) if len(pauses) > 1 else 0,
            'gc_max_time': max(sorted_pause_times),
            'gc_min_time': min(sorted_pause_times),
            'gc_pct99_time': percentile(sorted_pause_times, 99),
            'gc_pct95_time': percentile(sorted_pause_times, 95),
            'gc_pct50_time': percentile(sorted_pause_times, 50),
            'gc_pauses': list(map(pause_log, pauses)),
            'gc_avg_interval_time': avg_interval_time,
            'time_unit': 'ns'
        }
