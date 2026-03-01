#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

import subprocess
import sys
from glob import glob
from os import path
from pathlib import Path

from runner.common_exceptions import SetupException
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class EsCheckedGenerator(IGenerator):
    def generate(self) -> list[str]:
        confs = list(glob(path.join(self._source, "**/*.yaml"), recursive=True))
        generator_root = Path(__file__).parent / "generate-es-checked"
        generator_executable = generator_root / "main.rb"
        res = subprocess.run(
            [
                generator_executable,
                '--proc',
                str(self._config.general.processes),
                '--out',
                self._target,
                '--tmp',
                self._target / 'tmp',
                '--ts-node',
                f'npx:--prefix:{generator_root}:ts-node:-P:{generator_root / "tsconfig.json"}',
                *confs
            ],
            capture_output=True,
            encoding=sys.stdout.encoding,
            check=False,
            errors='replace',
        )
        if res.returncode != 0:
            _LOGGER.default(
                'Failed to run es cross-validator, please, make sure that '
                'all required tools are installed (see tests-u-runner/readme.md#ets-es-checked-dependencies)')
            raise SetupException(f"invalid return code {res.returncode}\n" + res.stdout + res.stderr)
        extension = self._config.test_suite.extension()
        glob_expression = path.join(self._target, f"**/*.{extension}")
        return list(glob(glob_expression, recursive=True))
