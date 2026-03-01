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

# pylint: disable=duplicate-code
import logging
from typing import List
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.swiftc = self.tools_get('swiftc')
        self.exe = self.tools_get('exe')

    @property
    def name(self) -> str:
        return 'Swift on host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def required_tools(self) -> List[str]:
        return ['swiftc', 'exe']

    @property
    def langs(self) -> List[str]:
        return ['swift']

    def run_unit(self, bu: BenchUnit) -> None:
        self.swiftc(bu)
        if self.dry_run_stop(bu):
            return
        self.exe(bu)
