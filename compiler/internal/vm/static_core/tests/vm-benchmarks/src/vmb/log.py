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

import logging
import re
from pathlib import Path
from dataclasses import dataclass, field
from datetime import datetime
from typing import Any, Optional, Union, Iterable, List, Tuple
from vmb.helpers import die, load_file, create_file
from vmb.cli import Args

log = logging.getLogger('vmb')


class TimeStamp:
    ts_format = '%Y-%m-%dT%H:%M:%S.%fZ'

    def __init__(self, name: str, start: Union[str, datetime], end: Union[str, datetime] = ''):
        self.name: str = name
        self.start: Optional[datetime] = TimeStamp.parse_date(start)
        self.end: Optional[datetime] = TimeStamp.parse_date(end)

    def __str__(self):
        return f'{self.name}: {self.get_seconds()}'

    @staticmethod
    def parse_date(ts: Union[str, datetime]) -> Optional[datetime]:
        if not ts:
            return None
        try:
            return ts if isinstance(ts, datetime) else datetime.strptime(ts, TimeStamp.ts_format)
        except ValueError as e:
            log.debug('Error parsing [%s]: %s', str(ts), e)
            return None

    def set_end(self, end: Union[str, datetime]):
        self.end = end if isinstance(end, datetime) \
            else datetime.strptime(end, TimeStamp.ts_format)

    def get_seconds(self) -> int:
        if self.start is None or self.end is None:
            return 0
        return (self.end - self.start).seconds


@dataclass
class Test:
    ts: TimeStamp
    cmds: List[TimeStamp] = field(default_factory=list)

    def __str__(self):
        return f'{self.ts} [{",".join([str(c) for c in self.cmds])}]'

    def set_end(self, end: Union[str, datetime]):
        self.ts.set_end(end)


class LineConsumed(Exception):
    pass


class LogParser:
    re_timestamp = r'\[(?P<ts>\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}.\d{3}Z)]'
    re_start_esc = r'(?:\x1b\[[\d;]+m)?'
    re_end_esc = r'(?:\x1b\[0m)?'
    # batch mode will not match this
    re_start_test = r'Starting bench \(\d+/\d+\): (?P<test>\w+)'
    re_run_total = r'Run took (?P<total_h>\d+):(?P<total_m>\d+):(?P<total_s>\d+)(\.\d+)?'
    # only timed commands
    # f.e. rm -f /tmp/blah will not match
    re_host_cmd = r'\\time -v env (\w+=[\w\-\./]+ )?([\w\-\./]+/)?(?P<cmd>\w+) .*'
    re_device_cmd = r'([\w\-\./]+/)?(?P<cmd>hdc|ad' + \
                    r"b) (.*)?shell '\(\\time -v .*"
    re_file_transfer_cmd = r'([\w\-\./]+/)?(hdc|ad' + \
                           r"b) (.*)?(file )?(?P<cmd>recv|send|push|pull) .*"

    def __init__(self, text: str = ''):
        self._lines = text.splitlines() if text else []
        self._i = 0
        self._tests: List[Test] = []
        self._ts: str = ''
        self._test: Optional[Test] = None
        self._cmd: Optional[TimeStamp] = None
        # Pattern
        self._re_start_test = re.compile(f'^{LogParser.re_timestamp} '
                                         f'{LogParser.re_start_esc}'
                                         f'{LogParser.re_start_test}'
                                         f'{LogParser.re_end_esc}$')
        self._re_host_cmd = re.compile(f'^{LogParser.re_timestamp} '
                                       f'{LogParser.re_start_esc}'
                                       f'{LogParser.re_host_cmd}'
                                       f'{LogParser.re_end_esc}$')
        self._re_run_end = re.compile(f'^{LogParser.re_timestamp} '
                                      f'{LogParser.re_start_esc}'
                                      f'{LogParser.re_run_total}'
                                      f'{LogParser.re_end_esc}$')
        self._re_device_cmd = re.compile(f'^{LogParser.re_timestamp} '
                                         f'{LogParser.re_start_esc}'
                                         f'{LogParser.re_device_cmd}'
                                         f'{LogParser.re_end_esc}$')
        self._re_transfer_cmd = re.compile(f'^{LogParser.re_timestamp} '
                                           f'{LogParser.re_start_esc}'
                                           f'{LogParser.re_file_transfer_cmd}'
                                           f'{LogParser.re_end_esc}$')
        self._re_any_ts = re.compile(f'^{LogParser.re_timestamp} .*$')

    @staticmethod
    def get_header(seq: Iterable[str]) -> str:
        header = 'N,TestName,TotalTime'
        if not seq:
            return header
        name_index = {n: 1 for n in set(seq)}
        seq_indexed = []
        for x in seq:
            seq_indexed.append(f'{x}_{name_index[x]}')
            name_index[x] += 1
        return header + ',' + ','.join(seq_indexed)

    def reset(self):
        self._i = 0
        self._tests = []
        self._ts = ''
        self._test = None
        self._cmd = None

    def parse(self) -> None:
        self.reset()
        while self._i < len(self._lines):
            try:
                self._is_start_test()
                self._is_cmd(self._re_host_cmd)
                self._is_cmd(self._re_device_cmd)
                self._is_cmd(self._re_transfer_cmd)
                self._is_run_end()
                self._is_ts()
            except LineConsumed:
                pass
            finally:
                self._i += 1

    def analyze(self) -> Tuple[List[List[Any]], List[str]]:
        """Get list of N,TestName,TotalTime,Cmd1,Cmd2,..."""
        longest_cmds_sequence: List[str] = []
        for t in self._tests:
            cmd_names = [c.name for c in t.cmds]
            if len(cmd_names) > len(longest_cmds_sequence):
                longest_cmds_sequence = cmd_names
        report: List[List[Any]] = []
        for n, t in enumerate(self._tests):
            line = [n + 1, t.ts.name, t.ts.get_seconds()]
            maybe_idx = 0
            for name in longest_cmds_sequence:
                if len(t.cmds) > maybe_idx and t.cmds[maybe_idx].name == name:
                    line.append(t.cmds[maybe_idx].get_seconds())
                    maybe_idx += 1
                else:
                    line.append(0)
            report.append(line)
        return report, longest_cmds_sequence

    def save_csv(self, csv: Union[str, Path]) -> None:
        report, longest_cmds_sequence = self.analyze()
        # Test and commands report
        with create_file(csv) as f:
            f.write(LogParser.get_header(longest_cmds_sequence) + '\n')
            for test in report:
                f.write(','.join([str(x) for x in test]) + '\n')
        # Total time distribution report
        totals = [rec[2] for rec in report]
        distinct_totals = set(totals)
        distribution = {total: totals.count(total) for total in distinct_totals}
        with create_file(Path(csv).with_suffix('.dist.csv')) as f:
            f.write('Time,Count\n')
            for total in sorted(list(distinct_totals)):
                f.write(f'{total},{distribution[total]}\n')

    def _current(self) -> str:
        return self._lines[self._i]

    def _close_cmd(self) -> None:
        if self._cmd:
            self._cmd.set_end(self._ts)
            if self._test:
                self._test.cmds.append(self._cmd)
                self._cmd = None

    def _is_start_test(self) -> None:
        m = self._re_start_test.match(self._current())
        if not m:
            return
        self._ts = m.group('ts')
        if self._test:
            self._test.set_end(self._ts)
            self._close_cmd()
            self._tests.append(self._test)
        self._test = Test(ts=TimeStamp(name=m.group('test'), start=self._ts))
        raise LineConsumed

    def _is_cmd(self, regex: re.Pattern):
        if not self._test:
            return  # skip setup commands (those before tests)
        m = regex.match(self._current())
        if not m:
            return
        self._ts = m.group('ts')
        self._close_cmd()
        self._cmd = TimeStamp(name=m.group('cmd'), start=self._ts)
        raise LineConsumed

    def _is_run_end(self) -> None:
        m = self._re_run_end.match(self._current())
        if not m:
            return
        self._ts = m.group('ts')
        if self._test:  # close and add previous test
            self._test.set_end(self._ts)
            self._close_cmd()
            self._tests.append(self._test)
        raise LineConsumed

    def _is_ts(self) -> None:
        m = self._re_any_ts.match(self._current())
        if not m:
            return
        self._ts = m.group('ts')
        raise LineConsumed


def log_main(args: Args) -> None:
    die(len(args.paths) < 1, 'Log paths missed!')
    log_file = args.paths[0]
    die(not log_file.is_file(), f'Log file {log_file} does not exist!')
    text = load_file(log_file)
    parser = LogParser(text)
    parser.parse()
    csv_file = Path(args.out).with_suffix('.csv').resolve()
    parser.save_csv(csv_file)
    log.passed('Reports are saved to %s and %s',
               str(csv_file), str(csv_file.with_suffix('.dist.csv')))
