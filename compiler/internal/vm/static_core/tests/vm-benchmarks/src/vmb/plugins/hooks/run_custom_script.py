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
from typing import Optional
from pathlib import Path
from vmb.cli import Args
from vmb.unit import BenchUnit
from vmb.hook import HookBase
from vmb.shell import ShellBase
from vmb.platform import PlatformBase
from vmb.tool import ToolBase
from vmb.target import Target
from vmb.helpers import copy_file

log = logging.getLogger('vmb')


class Hook(HookBase):
    """Run shell script after each test."""

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.x_sh: Optional[ShellBase] = None
        script = args.get('custom_script', 'unexisting')
        # Need to peek into args and set script accordingly ...
        self.script: Path = Path(script).resolve()
        if not self.script.is_file():
            raise RuntimeError(f'Custom script not found! ({script})')

    @property
    def name(self) -> str:
        return f'Custom script: {self.script}'

    def before_suite(self, platform: PlatformBase) -> None:
        self.x_sh = platform.x_sh
        if Target.HOST == platform.target:
            sh = ToolBase.libs.parent.joinpath(self.script.name)
            copy_file(self.script, sh)
        else:
            sh = platform.dev_dir.joinpath(self.script.name)
            self.x_sh.push(self.script, sh)
        self.x_sh.run(f'chmod a+x {sh}')
        self.script = sh

    def after_unit(self, bu: BenchUnit) -> None:
        if self.x_sh is None:
            raise RuntimeError('Remote shell has not been set!')
        self.x_sh.run(f'VMB_BU_PATH={bu.path} '
                      f'VMB_BU_NAME={bu.name} '
                      f'{self.script}')
