#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

import shutil
import subprocess
import sys
from pathlib import Path
from subprocess import CompletedProcess
from typing import cast

from runner.common_exceptions import SetupException
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class EtsArkJsGenerator(IGenerator):
    @staticmethod
    def on_error(res: CompletedProcess[str], path: Path, filename: str) -> None:
        _LOGGER.default(f'Failed to compile {path}')
        raise SetupException(f'Error on {filename} compilation, retL {res.returncode}\n' + res.stdout + res.stderr)
    
    def generate(self) -> list[str]:
        shutil.copytree(self._source, self._target, dirs_exist_ok=True, ignore=lambda directory, files: 
                    [f for f in files if Path.is_file(Path(directory) / f) and f[-3:] != '.ts' and f[-4:] != '.ets'])
        ts_common_path = self._target / 'gc_test_common.ts'
        ets_common_path = self._target / 'ets' / 'gc_test_sts_common.ets'
        ts_common_abc_path = Path(self._config.test_suite.work_dir) / 'intermediate' / 'gc_test_common.abc'
        ets_common_abc_path = Path(self._config.test_suite.work_dir) / 'intermediate' / 'gc_test_sts_common.abc'
        args_ts: list[str] = [
            str(self._config.general.build / 'arkcompiler' / 'ets_frontend' / 'es2abc'),
            '--merge-abc',
            '--extension=ts',
            '--module',
            f'--output={ts_common_abc_path}',
            str(ts_common_path)
        ]
        res = subprocess.run(
            args_ts,
            capture_output=True,
            encoding=sys.stdout.encoding,
            check=False,
            errors='replace'
        )
        if res.returncode != 0:
            EtsArkJsGenerator.on_error(res, ts_common_path, 'gc_test_common.ts')
        arktsconfig_path = self._config.general.build / "arkcompiler" / "ets_frontend" / "arktsconfig.json"
        args_ets: list[str] = [
            str(self._config.general.build / 'arkcompiler' / 'ets_frontend' / 'es2panda'),
            '--extension=ets',
            f'--arktsconfig={arktsconfig_path}',
            '--gen-stdlib=false',
            '--opt-level=2',
            f'--output={ets_common_abc_path}',
            str(ets_common_path)
        ]
        res = subprocess.run(
            args_ets,
            capture_output=True,
            encoding=sys.stdout.encoding,
            check=False,
            errors='replace'
        )
        if res.returncode != 0:
            EtsArkJsGenerator.on_error(res, ets_common_path, 'gc_test_sts_common.ets')
        extension = self._config.test_suite.extension()
        return cast(list[str], (self._target.rglob(f"**/*.{extension}")))
