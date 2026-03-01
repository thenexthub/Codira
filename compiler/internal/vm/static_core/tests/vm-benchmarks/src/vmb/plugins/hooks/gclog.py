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

import logging
from typing import Optional, Type  # noqa
from vmb.cli import Args
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.hook import HookBase
from vmb.result import GCStats
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.shell import ShellBase
from vmb.gclog.ark.ark_gclog_parser import AbstractLogParser
from vmb.gclog.fw_time_parser import FwTimeParser

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Parse GC logs."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.target: Target = Target.HOST
        self.gc_parser_cls: Optional[Type] = None
        self.x_sh: Optional[ShellBase] = None

    @property
    def name(self) -> str:
        return 'Parse GC logs'

    def before_suite(self, platform: PlatformBase) -> None:
        self.target = platform.target
        self.gc_parser_cls = platform.gc_parcer
        self.x_sh = platform.x_sh

    def after_unit(self, bu: BenchUnit) -> None:
        gclog_name = f'{BENCH_PREFIX}{bu.name}.gclog.txt'
        gclog = bu.path.joinpath(gclog_name)
        if self.target != Target.HOST:
            if bu.device_path is None:
                raise RuntimeError(f'GC: no device path for "{bu.name}"')
            if self.x_sh is None:
                raise RuntimeError('Remote shell has not been set!')
            self.x_sh.pull(bu.device_path.joinpath(gclog_name), gclog)
        if not gclog.is_file():
            raise RuntimeError(f'GC: log missed "{gclog}"')
        if not bu.run_out:
            raise RuntimeError(f'GC: stdout is empty for "{bu.name}"')
        if self.gc_parser_cls is None:
            raise RuntimeError('GC: gc_parcer not set for platform')
        gc_parser: AbstractLogParser = self.gc_parser_cls()
        time_adjustments = FwTimeParser().parse_text(bu.run_out)
        events = gc_parser.parse_log_file(str(gclog))
        reporter = gc_parser.reporter()
        if not reporter:
            raise RuntimeError(f'GC: parser failure for "{bu.name}"')
        rep = reporter.generate_report(events, adjust_time=time_adjustments)
        bu.result.gc_stats = GCStats(**rep)
