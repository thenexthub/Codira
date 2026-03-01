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

# pylint: disable=duplicate-code
import logging
from typing import List, Optional
from vmb.platform import PlatformBase
from vmb.gensettings import GenSettings
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.tsc = self.tools_get('tsc') \
            if 'tsc' in self.required_tools else None
        self.node = self.tools_get('node')

    @property
    def name(self) -> str:
        return 'Node on host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def required_tools(self) -> List[str]:
        return ['tsc', 'node'] if 'ts' in self.langs else ['node']

    @property
    def required_hooks(self) -> List[str]:
        return ['import_esm_without_extention']

    @property
    def langs(self) -> List[str]:
        return list(self.args_langs) if self.args_langs else ['ts', 'js']

    @property
    def template(self) -> Optional[GenSettings]:
        """Special template because of print  method."""
        return GenSettings(src=set(), template='', out='',
                           print_func='console')

    def run_unit(self, bu: BenchUnit) -> None:
        if self.tsc:
            self.tsc(bu)
        if self.dry_run_stop(bu):
            return
        self.node(bu)
