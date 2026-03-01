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

import os
import logging
from pathlib import Path
from typing import List, Optional
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.gensettings import GenSettings
from vmb.tool import ToolBase
from vmb.plugins.platforms.interop_base import InteropPlatformBase

log = logging.getLogger('vmb')


class Platform(InteropPlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.establish_arkjs_module()
        here = os.path.realpath(os.path.dirname(__file__))
        runner_js = Path(here).parent.parent.joinpath('resources', 'interop', 'InteropRunner.js')
        self.interop_runner = ToolBase.libs.joinpath('InteropRunner.abc')
        self.es2abc_interop.run_es2abc(runner_js, self.interop_runner)

    @property
    def name(self) -> str:
        return 'Interop ArkJS/ArkTS (s2d) host'

    @property
    def target(self) -> Target:
        return Target.HOST

    @property
    def langs(self) -> List[str]:
        """Use this lang plugins."""
        return ['ets']

    @property
    def template(self) -> Optional[GenSettings]:
        return None

    # pylint: disable=duplicate-code
    def run_unit(self, bu: BenchUnit) -> None:
        libs = bu.libs('.ts', '.mjs', '.js')
        if not libs:
            raise RuntimeError('No dynamic imports in ets bench!')
        for lib in libs:
            # treat all src as ts to simplify arktsconfig generation
            lib = ToolBase.rename_suffix(lib, '.ts')
            self.es2abc_interop.run_es2abc(lib, lib.with_suffix('.abc'))
        self.es2panda(bu)
        zip_path = self.zip_classes(bu)
        self.arkjs_interop.run(bu,
                               abc=str(self.interop_runner),
                               entry_point='InteropRunner',
                               bench_name=bu.name,
                               zip_path=zip_path)
