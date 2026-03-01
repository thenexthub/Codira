#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
from pathlib import Path
from vmb.target import Target
from vmb.tool import ToolBase, OptFlags
from vmb.unit import BenchUnit, BUStatus
from vmb.result import JITStat

log = logging.getLogger('vmb')


class Tool(ToolBase):

    def __init__(self, *args) -> None:
        super().__init__(*args)
        if Target.HOST == self.target:
            panda_root = self.ensure_dir_env('PANDA_BUILD')
            self.ark = self.ensure_file(self.custom_path) if self.custom_path \
                else self.ensure_file(panda_root, 'bin', 'ark')
            self.ark_lib = self.ensure_dir(panda_root, 'lib')
            self.etsstdlib = self.ensure_file(
                panda_root, 'plugins', 'ets', 'etsstdlib.abc')
        elif self.target in (Target.DEVICE, Target.OHOS):
            self.ark = self.custom_path if self.custom_path \
                else f'{self.dev_dir.as_posix()}/ark'
            self.ark_lib = f'{self.dev_dir.as_posix()}/lib'
            self.etsstdlib = f'{self.dev_dir.as_posix()}/etsstdlib.abc'
        else:
            raise NotImplementedError(f'Wrong target: {self.target}!')
        opts = '--gc-trigger-type=heap-trigger '
        self.an_files = []
        if OptFlags.AOT_SKIP_LIBS not in self.flags:
            stdlib = f'{ToolBase.libs.as_posix()}/etsstdlib.an' \
                if Target.HOST == self.target \
                else f'{self.dev_dir.as_posix()}/etsstdlib.an'
            self.an_files.append(stdlib)
        if OptFlags.INT in self.flags:
            opts += '--compiler-enable-jit=false ' \
                    '--profiler-enabled=false '
        if OptFlags.INT_CPP in self.flags:
            opts += '--interpreter-type=cpp '
        if OptFlags.INT_IRTOC in self.flags:
            opts += '--interpreter-type=irtoc '
        if OptFlags.INT_LLVM in self.flags:
            opts += '--interpreter-type=llvm '
        if OptFlags.GC_STATS in self.flags:
            opts += '--print-gc-statistics --log-components=gc ' \
                    '--log-level=info --log-stream=file ' \
                    '--log-file={gclog} '
        if OptFlags.JIT_STATS in self.flags:
            opts += '--compiler-dump-jit-stats-csv={abc}.dump.csv '
        if OptFlags.SAFEPOINT_CHECKER in self.flags:
            opts += '--safepoint-checkers-report-filepath={abc}.spcr.json '
        self.cmd = f'LD_LIBRARY_PATH={self.ark_lib} {self.ark} ' \
                   f'--boot-panda-files={self.etsstdlib} ' \
                   f'--load-runtimes=ets {opts} {{aot_opts}} {self.custom} ' \
                   '{options} {abc} {name}.VmbLauncher::main'

    @property
    def name(self) -> str:
        return 'Ark VM'

    def get_cmd(self, name: str, abc: str, options: str, gclog: str, an: str) -> str:
        an_files = self.an_files + [an] \
            if an and (OptFlags.AOT in self.flags or OptFlags.AOTPGO in self.flags) \
            else self.an_files
        aot_opts = ''
        if an_files:
            enable_an = '' if Target.HOST == self.target else '--enable-an:force'
            aot_opts = f'{enable_an} --aot-files={":".join([x for x in an_files if x])}'
        if OptFlags.AOTPGO in self.flags:
            options += '--compiler-enable-jit=false '
        return self.cmd.format(
            name=name, abc=abc, options=options, gclog=gclog, aot_opts=aot_opts)

    def do_exec(self, bu: BenchUnit, profile: bool = False) -> None:
        bu_flags, _ = self.get_bu_opts(bu)
        gclog = ''
        libs = ':'.join([str(f) for f in self.x_libs(bu, '.abc')])
        options = f'--panda-files={libs} ' if libs else ''
        native_dir = bu.path.joinpath('native')
        if native_dir.exists():
            natives = native_dir if Target.HOST == self.target \
                else bu.device_path
            options += f'--ets.native-library-path={natives} '
        abc = self.x_src(bu, '.abc')
        an_files = [str(f.as_posix()) for f in self.x_libs(bu, '.an')]
        if not profile:
            an_files.append(str(abc.with_suffix('.an').as_posix()))
        an = ':'.join(an_files) if an_files else ''
        if OptFlags.DISABLE_INLINING in bu_flags:
            options += '--compiler-inlining=false '
        if OptFlags.GC_STATS in bu_flags:
            gclog = str(abc.with_suffix('.gclog.txt').as_posix())
        if profile:
            options += ('--compiler-profiling-threshold=0 '
                        '--profilesaver-enabled=true '
                        '--compiler-enable-jit=false '
                        f'--profile-output={abc.as_posix()}.profdata ')
        arkts_cmd = self.get_cmd(
            name=bu.name, abc=str(abc.as_posix()), options=options, gclog=gclog, an=an)
        res = self.x_run(arkts_cmd)
        if self.no_run:
            bu.status = BUStatus.NOT_RUN
            return
        if profile:
            return  # profiling run should not affect bu.result
        bu.parse_run_output(res)
        if OptFlags.JIT_STATS in bu_flags:
            csv = Path(f'{abc}.dump.csv')
            if self.target != Target.HOST:
                self.x_sh.pull(csv, bu.path)
                csv = bu.path.joinpath(csv.name)
            if csv.exists():
                bu.result.jit_stats = JITStat.from_csv(csv)
            else:
                log.error('JIT stats dump missed: %s', str(csv))

    def exec(self, bu: BenchUnit) -> None:
        self.do_exec(bu)

    def kill(self) -> None:
        self.x_sh.run('pkill ark')
