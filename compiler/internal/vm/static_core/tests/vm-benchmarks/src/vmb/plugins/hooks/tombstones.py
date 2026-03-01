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
from typing import Optional
from vmb.cli import Args
from vmb.unit import BenchUnit
from vmb.hook import HookBase
from vmb.shell import ShellBase
from vmb.platform import PlatformBase

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Save crash reports info to failure log."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.x_sh: Optional[ShellBase] = None

    @property
    def name(self) -> str:
        return 'Save crash info'

    @classmethod
    def skipme(cls, args: Args) -> bool:
        """Do not register me on host."""
        return args.platform.endswith('host')

    def before_suite(self, platform: PlatformBase) -> None:
        self.x_sh = platform.x_sh

    # pylint: disable-next=unused-argument
    def before_unit(self, bu: BenchUnit) -> None:
        if self.x_sh is None:
            raise RuntimeError('Remote shell has not been set!')
        self.x_sh.run(f'rm -f /data/tomb{"stones"}/*')

    def after_unit(self, bu: BenchUnit) -> None:
        if self.x_sh is None:
            raise RuntimeError('Remote shell has not been set!')
        dump = self.x_sh.grep(f'ls -t -1 /data/tomb{"stones"}',
                              f'^tomb{"stone"}_[0-9][0-9]$')
        if dump:
            res = self.x_sh.run(f'head -40 /data/tombstones/{dump}')
            self.x_sh.run(f'rm -f /data/tombstones/{dump}*')
            log.error("Crash Dump detected:\n%s", res.out)
            bu.crash_info = res.out
