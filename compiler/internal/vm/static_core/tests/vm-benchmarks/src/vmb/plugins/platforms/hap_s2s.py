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
from typing import List, Optional, Union, Callable
from vmb.helpers import copy_file, copy_files, create_file_from_template
from vmb.gensettings import GenSettings
from vmb.tool import ToolBase
from vmb.unit import BenchUnit
from vmb.cli import Args, OptFlags
from vmb.plugins.platforms.hap_base import HapPlatformBase, ohos_dynamic_paths
from vmb.plugins.tools.es2panda import make_arktsconfig, fix_arktsconfig

log = logging.getLogger('vmb')


class Platform(HapPlatformBase):

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.mode = args.mode
        self.es2panda = self.tools_get('es2panda')
        self.panda_sdk = ToolBase.ensure_dir_env('PANDA_SDK')
        self.etsstdlib = Path(self.panda_sdk).joinpath('ets', 'etsstdlib.abc')
        self.panda_root = self.es2panda.panda_root
        self.lib_dir = self.hap_dir.joinpath(self.mod_name, 'libs', 'arm64-v8a')
        arktsconfig_json = self.es2panda.default_arktsconfig
        panda_src = ToolBase.ensure_dir_env('PANDA_SRC')
        make_arktsconfig(arktsconfig_json, panda_src, [])
        fix_arktsconfig(ohos_dynamic_paths)

    @property
    def name(self) -> str:
        return 'Application (hap) interop s2s on OHOS'

    @property
    def template(self) -> Optional[GenSettings]:
        return GenSettings(src={'.ets'},
                           template='ets-hap.j2',
                           out='.ets',
                           link_to_src=False)

    @property
    def required_tools(self) -> List[str]:
        return ['es2panda', 'hvigor']

    @property
    def langs(self) -> List[str]:
        return ['ets']

    def run_ark_aot(self, abc: List[Union[str, Path]], an: Union[str, Path]) -> None:
        # not using Tool::paoc because of special case on Target.OHOS
        cmd = f'LD_LIBRARY_PATH={self.panda_root}/lib {self.panda_root}/bin/ark_aot ' \
              f'--boot-panda-files={self.etsstdlib} --paoc-mode=aot ' \
              '--load-runtimes=ets --compiler-cross-arch=arm64 ' \
              f'--paoc-panda-files={",".join([str(f) for f in abc])} --paoc-output={an}'
        ret = self.sh.run(cmd)
        if ret.ret != 0:
            raise RuntimeError(f"Paoc failed!\n{ret.out}\n{ret.err}")

    def copy_panda_libs(self, dst: Union[str, Path]) -> None:
        src = Path(self.panda_sdk).joinpath('ohos_arm64', 'lib')
        copy_files(src, dst, "*.so")
        copy_file(self.etsstdlib, Path(dst).joinpath('etsstdlib.abc.so'))

    def run_batch_impl(self, bus: List[BenchUnit],
                       before_compile: Optional[Callable[[BenchUnit], None]] = None) -> None:
        hap_unit = self.make_hap_unit()
        module_config_copy = self.make_module_config()
        self.copy_panda_libs(self.lib_dir)
        # EtsRuntime
        runtime_ts = self.resource_dir.joinpath('EtsRuntime.ts')
        copy_file(runtime_ts, self.ability_dir.joinpath(runtime_ts.name))
        ability_template = self.resource_dir.joinpath('EntryAbility.ts.tpl')
        abc_for_aot: List[Union[str, Path]] = []
        for bu in bus:
            if before_compile:
                before_compile(bu)
            self.es2panda(bu)  # compile src and libs
            abc = bu.src('.abc', die_on_zero_matches=True)
            so = ToolBase.rename_suffix(abc, '.abc.so')
            copy_file(so, self.lib_dir)
            for lib in bu.libs('.abc'):
                copy_file(ToolBase.rename_suffix(lib, '.abc.so'), self.lib_dir)
            if OptFlags.AOT in self.flags:
                abc_for_aot.append(so)
            # Create bench ability file
            ability_ts = self.ability_dir.joinpath(f'{bu.name}_Ability.ts')
            create_file_from_template(ability_template,
                                      ability_ts,
                                      BENCH_NAME=bu.name,
                                      BENCH_FILE_SO=f'bench_{bu.name}.abc.so',
                                      BENCH_MODE=self.mode)
            # Add bench ability to module.json5
            module_config_copy['module']['abilities'].append(HapPlatformBase.make_ability(bu))
        if OptFlags.AOT in self.flags:
            self.run_ark_aot(abc=abc_for_aot, an=self.lib_dir.joinpath('aot_file.an.so'))
        self.flush_module_config(module_config_copy)
        self.hvigor(hap_unit)

    def run_batch(self, bus: List[BenchUnit]) -> None:
        self.run_batch_impl(bus)
