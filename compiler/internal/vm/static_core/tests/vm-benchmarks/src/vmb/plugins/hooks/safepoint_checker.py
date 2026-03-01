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
from pathlib import Path
from typing import Optional, Dict
from vmb.cli import Args
from vmb.unit import BenchUnit, BENCH_PREFIX
from vmb.hook import HookBase
from vmb.shell import ShellBase
from vmb.platform import PlatformBase
from vmb.target import Target

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Load safepoint checker intrinsic timings data."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.target: Target = Target.HOST
        self.x_sh: Optional[ShellBase] = None

    @property
    def name(self) -> str:
        return 'Load safepoint checker intrinsic timings data'

    def before_suite(self, platform: PlatformBase) -> None:
        self.x_sh = platform.x_sh
        self.target = platform.target

    def after_unit(self, bu: BenchUnit) -> None:
        file_name = f'{BENCH_PREFIX}{bu.name}.abc.spcr.json'
        file_path = bu.path.joinpath(file_name)
        if self.target != Target.HOST:
            if bu.device_path is None:
                raise RuntimeError(f'Safepoint checker: no device path for "{bu.name}"')
            if self.x_sh is None:
                raise RuntimeError('Remote shell has not been set!')
            dev_path = bu.device_path.joinpath(file_name)
            file_size = self.x_sh.get_filesize(dev_path)
            if file_size > 0:
                self.x_sh.pull(dev_path, file_path)
        if file_path.is_file():
            bu.result.safepoint_checker = self.parse_file_to_dict(file_path)
        else:
            log.warning('Safepoint checker log is missed "%s"', file_name)

    def parse_file_to_dict(self, file_path: Path) -> Dict[str, int]:
        result = {}
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                line = line.strip()
                if not line:  # ignore empty lines
                    continue
                key, value = line.split(':', 1)
                result[key.strip()] = int(value.strip())
        return result
