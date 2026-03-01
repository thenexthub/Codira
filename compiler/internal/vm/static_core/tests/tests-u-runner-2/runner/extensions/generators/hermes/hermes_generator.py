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
#

import os
import re
import subprocess
from glob import glob
from os import makedirs, path
from pathlib import Path
from typing import cast

from runner.common_exceptions import InvalidConfiguration, RunnerException
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.utils import download_and_generate

_LOGGER = Log.get_logger(__file__)


class HermesGeneratorException(RunnerException):
    pass


class HermesGenerator(IGenerator):
    __URL = "url"
    __REVISION = "revision"
    __FORCE_DOWNLOAD = "force-download"

    def __init__(self, source: Path, target: Path, config: Config) -> None:
        super().__init__(source, target, config)

        self.check_expr = re.compile(r"^\s*//\s?(?:CHECK-NEXT|CHECK-LABEL|CHECK):(.+)", re.MULTILINE)

        self.show_progress = self._config.general.show_progress
        self.force_download = cast(bool, self._config.test_suite.get_parameter(self.__FORCE_DOWNLOAD, False))

        url: str | None = self._config.test_suite.get_parameter(self.__URL)
        if url is None:
            raise InvalidConfiguration(f"No {self.__URL} parameter set in "
                                       "`hermes.yaml` config file")
        self.url: str = url

        revision: str | None = self._config.test_suite.get_parameter(self.__REVISION)
        if revision is None:
            raise InvalidConfiguration(f"No {self.__REVISION} parameter set in "
                                       "`hermes.yaml` config file")
        self.revision: str = revision

    @staticmethod
    def create_file(src_file: str, dest_file: str) -> None:
        with os.fdopen(os.open(src_file, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_pointer:
            input_str = file_pointer.read()

        out_str = input_str

        with os.fdopen(os.open(dest_file, os.O_RDWR | os.O_CREAT, 0o755), 'w', encoding="utf-8") as output:
            output.write(out_str)

    def generate(self) -> list[str]:
        stamp_name = f"hermes-{self.revision}"
        test_root = download_and_generate(
            name="hermes",
            stamp_name=stamp_name,
            url=self.url,
            revision=self.revision,
            download_root=self._source,
            generated_root=self._target,
            test_subdir="test/hermes",
            show_progress=self.show_progress,
            process_copy=self.process_copy,
            force_download=self.force_download
        )
        glob_expression = test_root / "**/*.js"
        return glob(str(glob_expression), recursive=True)

    def process_copy(self, src_path: Path, dst_path: Path) -> None:
        _LOGGER.all("Generating tests")

        glob_expression = path.join(src_path, "**/*.js")
        files = glob(glob_expression, recursive=True)

        for src_file in files:
            dest_file = src_file.replace(str(src_path), str(dst_path))
            makedirs(path.dirname(dest_file), exist_ok=True)
            self.create_file(src_file, dest_file)

    def run_filecheck(self, test_file: str, actual_output: str) -> bool:
        with open(test_file, encoding="utf-8") as file_pointer:
            input_str = file_pointer.read()
        if not re.match(self.check_expr, input_str):
            return True

        if actual_output is None:
            raise HermesGeneratorException("Expected some output to check")
        cmd = ['FileCheck-14', test_file]
        with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE) as process:
            try:
                if process.stdin is None:
                    raise HermesGeneratorException(f"Unexpected error on checking {test_file}")
                process.stdin.write(actual_output.encode('utf-8'))
                process.communicate(timeout=10)
                return_code = process.returncode
            except subprocess.TimeoutExpired as ex:
                _LOGGER.default(f"{' '.join(cmd)} failed with {ex}")
                process.kill()
                return_code = -1
            except subprocess.SubprocessError as ex:
                _LOGGER.default(f"{' '.join(cmd)} failed with {ex}")
                return_code = -1

        return return_code == 0
