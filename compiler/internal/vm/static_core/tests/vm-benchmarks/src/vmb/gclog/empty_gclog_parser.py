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

from typing import Any, List, Dict, Optional

from vmb.gclog.abstract_gclog_parser import AbstractLogParser
from vmb.gclog.event_parser import LogEntry
from vmb.gclog.gclog_reporter import LogReporter


class EmptyGcLogParser(AbstractLogParser):
    class EmptyGcReporter(LogReporter):

        def generate_report(self, events: Optional[List[LogEntry]] = None,
                            **options: Any) -> Dict[str, Any]:
            return {
                "gc_vm_time": 0,
                "gc_name": "",
                "gc_pause_count": 0,
                "gc_total_time": 0,
                "gc_avg_time": 0,
                "gc_avg_interval_time": 0,
                "gc_std_time": 0,
                "gc_min_time": 0,
                "gc_max_time": 0,
                "gc_median_time": 0,
                "gc_pct50_time": 0,
                "gc_pct95_time": 0,
                "gc_pct99_time": 0,
                "gc_memory_total_heap": 0,
                "gc_total_bytes_reclaimed": 0,
                "gc_avg_bytes_reclaimed": 0,
                "gc_pauses": [],
                "time_unit": "ns"
            }

    def parse_log_file(self, logfile: str) -> List[LogEntry]:
        return []

    def parse_log_text(self, source: str) -> List[LogEntry]:
        return []

    def reporter(self) -> Optional[LogReporter]:
        return EmptyGcLogParser.EmptyGcReporter()
