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
from pathlib import Path

from vmb.gclog.event_parser import LogEntry
from vmb.gclog.gclog_reporter import LogReporter


class AbstractLogParser:

    @staticmethod
    def parse_lines(source: Iterable[str]) -> Iterable[str]:
        for line in source:
            yield line.replace('\r', '').replace('\n', '')

    # pylint: disable-next=unused-argument
    def parse_log_entries(self, source: Iterable[str]) -> Iterable[LogEntry]:
        yield from ()

    def reporter(self) -> Optional[LogReporter]:
        return None

    def parse_log_file(self, logfile: str) -> List[LogEntry]:
        with Path(logfile).open('r', encoding="utf-8") as log_records:
            data_reader = self.parse_log_entries(self.parse_lines(log_records))
            return [*data_reader]

    def parse_log_text(self, source: str) -> List[LogEntry]:
        data_reader = self.parse_log_entries(
            self.parse_lines(source.splitlines(True)))
        return [*data_reader]
