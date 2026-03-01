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

from typing import Iterable, List, Optional

from vmb.gclog.abstract_gclog_parser import AbstractLogParser
from vmb.gclog.event_parser import EventParser, LogEntry
from vmb.gclog.gclog_reporter import LogReporter
from vmb.gclog.ark.ark_gclog_reporter import ArkGcLogReporter
from vmb.gclog.ark.ark_pause_parser import ArkPauseParser, ArkSyslogPauseParser
from vmb.gclog.ark.ark_fallback_parser import \
    ArkFallbackParser, ArkSyslogFallbackParser


class ArkGcLogParser(AbstractLogParser):

    parsers: List[EventParser] = [ArkPauseParser()]
    fallback = ArkFallbackParser()

    def parse_log_entries(self, source: Iterable[str]) -> Iterable[LogEntry]:
        for line in source:
            for p in self.parsers:
                e = p.parse(line)
                if e:
                    yield e
                    break
            else:
                le = self.fallback.parse(line)
                if le is None:
                    raise ValueError(f'Invalid GC log line: {line}')
                yield le

    def reporter(self) -> Optional[LogReporter]:
        return ArkGcLogReporter()


class ArkSyslogGcParser(ArkGcLogParser):

    parsers: List[EventParser] = [ArkSyslogPauseParser()]
    fallback = ArkSyslogFallbackParser()
