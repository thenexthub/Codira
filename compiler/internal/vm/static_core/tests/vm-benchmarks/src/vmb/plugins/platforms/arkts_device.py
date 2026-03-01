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
from typing import List, Optional, Type  # noqa
from pathlib import Path
from vmb.platform import PlatformBase
from vmb.target import Target
from vmb.unit import BenchUnit
from vmb.cli import Args, OptFlags
from vmb.result import AOTStatsLib
from vmb.gclog.ark.ark_gclog_parser import ArkGcLogParser

log = logging.getLogger('vmb')


class Platform(PlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.es2panda = self.tools_get('es2panda')
        self.paoc = self.tools_get('paoc')
        self.ark = self.tools_get('ark')
        self.nark = self.tools_get('nark')
        self.aot_lib_opts = ' '.join(args.get('aot_lib_compiler_options', ''))

    @property
    def name(self) -> str:
        return 'Ark on HOS Device'

    @property
    def target(self) -> Target:
        return Target.DEVICE

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'ark', 'paoc', 'nark']

    @property
    def langs(self) -> List[str]:
        return ['ets']

    @property
    def gc_parcer(self) -> Optional[Type]:
        return ArkGcLogParser

    def push_unit(self, bu: BenchUnit, *ext) -> None:
        """Override PlatformBase::Push bench unit to device."""
        super().push_unit(bu, *ext)
        native_dir = bu.path.joinpath('native')
        if not native_dir.exists():
            return
        if not bu.device_path:
            return  # make linter happy
        for f in native_dir.glob('*.so'):
            p = f.resolve()
            self.x_sh.push(p, bu.device_path.joinpath(f.name))

    def run_unit(self, bu: BenchUnit) -> None:
        self.es2panda(bu)
        self.nark(bu)
        if self.dry_run_stop(bu):
            return
        self.push_unit(bu, '.abc')
        if OptFlags.AOTPGO in self.flags:
            self.ark.do_exec(bu, profile=True)
            self.paoc.do_exec(bu, profdata=True)
        if OptFlags.AOT in self.flags:
            self.paoc(bu)
        self.ark(bu)

    def lazy_setup(self):
        if OptFlags.AOT_SKIP_LIBS in self.flags:
            log.info('Skipping aot compilation of libs')
        else:
            an = Path(self.ark.etsstdlib).with_suffix('.an')
            log.info('AOT-Compiling %s. This may take a long time...',
                     self.ark.etsstdlib)
            res = self.paoc.run_paoc(self.ark.etsstdlib, an,
                                     opts=self.aot_lib_opts, timeout=self.DEFAULT_TIMEOUT)
            if not self.ext_info.get('etsstdlib', {}):
                self.ext_info['etsstdlib'] = {}
            self.ext_info['etsstdlib']['etsstdlib.an'] = \
                AOTStatsLib(time=res.tm,
                            size=self.x_sh.get_filesize(an),
                            aot_stats=self.paoc.get_aot_stats(
                                an, self.paoc.libs))
        self.push_libs()
