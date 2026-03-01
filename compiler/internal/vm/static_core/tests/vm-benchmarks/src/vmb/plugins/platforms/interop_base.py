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
from typing import List, Optional
from abc import ABC
import json
from vmb.platform import PlatformBase
from vmb.unit import BenchUnit
from vmb.cli import Args
from vmb.tool import ToolBase
from vmb.helpers import force_link

log = logging.getLogger('vmb')


class InteropPlatformBase(PlatformBase, ABC):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.panda_root = ToolBase.ensure_dir_env('PANDA_BUILD')
        self.es2abc_interop = self.tools_get('es2abc_interop')
        self.es2panda = self.tools_get('es2panda')
        self.arkjs_interop = self.tools_get('arkjs_interop')
        self.ark = self.tools_get('ark')
        # transfer serialized ark vm custom options for using as env var
        self.arkjs_interop.ets_vm_opts = json.dumps(self.ark.custom_opts_obj())

    @property
    def required_tools(self) -> List[str]:
        # For d2d only es2abc and arkjs required
        # but since we use them from Panda build, it is safe to init all the tools
        return ['es2abc_interop', 'es2panda', 'arkjs_interop', 'ark']

    def establish_arkjs_module(self) -> None:
        # ideally this should be inside `generated/libs` and cd on exec
        module = Path.cwd().joinpath('module')
        module.mkdir(parents=True, exist_ok=True)
        for p in (Path(self.panda_root).joinpath('lib/module/ets_interop_js_napi.so'),
                  Path(self.panda_root).joinpath('lib/interop_js/libinterop_test_helper.so')):
            if not p.exists():
                raise RuntimeError(f'Lib `{str(p)}` not found!')
            force_link(module.joinpath(p.name), p)

    def zip_classes(self, bu: BenchUnit,
                    abc: Optional[Path] = None, zip_path: Optional[Path] = None) -> Path:
        a: Path = abc if abc else bu.src('.abc')
        a.rename(a.parent.joinpath('classes.abc'))
        z: Path = zip_path if zip_path else a.with_suffix('.zip')
        log.trace('Zipping %s -> %s', str(a), str(z))
        self.sh.run(f'zip -r -0 {z.name} classes.abc',
                    cwd=str(bu.path))
        return z
