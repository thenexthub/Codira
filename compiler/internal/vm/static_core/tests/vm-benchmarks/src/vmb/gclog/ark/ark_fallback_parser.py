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

import re
from typing import Optional

from vmb.gclog.ark.ark_gc_patterns import ArkGcPatterns
from vmb.gclog.event_parser import EventParser, UnmatchedEntry, LogEntry
from vmb.gclog.utils import ark_datetime_to_nanos as datetime_to_nanos


class ArkFallbackParser(EventParser):

    PATTERN_TEXT = (
        rf'{ArkGcPatterns.THREAD_ID}\s+'
        r'I\/gc:\s+'
        rf'{ArkGcPatterns.TIMESTAMP}\s+.*'
    )

    PATTERN = re.compile(PATTERN_TEXT)

    def parse(self, line: str) -> Optional[LogEntry]:
        m = self.PATTERN.search(line)
        return UnmatchedEntry(
            raw_message=line,
            timestamp=datetime_to_nanos(m.group('timestamp')) if m else -1
        )


class ArkSyslogFallbackParser(ArkFallbackParser):
    def parse(self, line: str) -> Optional[LogEntry]:
        return UnmatchedEntry(
            raw_message=line,
            timestamp=-1
        )
