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
import os
import re
from glob import glob
from os import makedirs, path
from pathlib import Path
from typing import Any

from runner.common_exceptions import InvalidConfiguration
from runner.descriptor import Descriptor
from runner.extensions.generators.igenerator import IGenerator
from runner.logger import Log
from runner.options.config import Config
from runner.utils import download_and_generate

_LOGGER = Log.get_logger(__file__)


class Test262Generator(IGenerator):
    __URL = "url"
    __REVISION = "revision"
    __FORCE_DOWNLOAD = "force-download"

    def __init__(self, source: Path, target: Path, config: Config):
        super().__init__(source, target, config)

        self.async_ok = re.compile(r"Test262:AsyncTestComplete")
        self.show_progress: bool = self._config.general.show_progress
        self.force_download: bool = bool(self._config.test_suite.get_parameter(self.__FORCE_DOWNLOAD, False))
        url: str | None = self._config.test_suite.get_parameter(self.__URL)
        if url is None:
            raise InvalidConfiguration(f"No {self.__URL} parameter set in "
                                       "`test262.yaml` config file")
        self.url: str = url

        revision: str | None = self._config.test_suite.get_parameter(self.__REVISION)
        if revision is None:
            raise InvalidConfiguration(f"No {self.__REVISION} parameter set in "
                                       "`test262.yaml` config file")
        self.revision: str = revision

    @staticmethod
    def create_file(input_file: str, output_file: str, harness: str, test262_dir: Path) -> None:
        descriptor = Descriptor(input_file)
        desc = Test262Generator.process_descriptor(descriptor)

        out_str = descriptor.get_header()
        out_str += "\n"
        out_str += harness

        includes = desc.get('includes', [])
        for include in includes:
            out_str += f"//------------ {include} start ------------\n"
            include_file = test262_dir / 'harness' / include
            with os.fdopen(os.open(include_file, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
                harness_str = file_handler.read()
            out_str += harness_str
            out_str += f"//------------ {include} end ------------\n"
            out_str += "\n"

        out_str += descriptor.get_content()

        with os.fdopen(os.open(output_file, os.O_WRONLY | os.O_CREAT, 0o755), 'w', encoding="utf-8") as output:
            output.write(out_str)

    @staticmethod
    def process_descriptor(descriptor: Descriptor) -> dict[str, str | list]:
        desc = descriptor.parse_descriptor()

        includes = []

        if "includes" in desc:
            includes.extend(desc["includes"])
            includes.extend(['assert.js', 'sta.js'])

        if 'flags' in desc and 'async' in desc["flags"]:
            includes.extend(['doneprintHandle.js'])

        negative_phase = desc["negative_phase"] if 'negative_phase' in desc else 'pass'
        negative_type = desc["negative_type"] if 'negative_type' in desc else ''

        # negative_phase: pass, parse, resolution, runtime
        return {
            'flags': desc["flags"] if 'flags' in desc else [],
            'negative_phase': negative_phase,
            'negative_type': negative_type,
            'includes': includes,
        }

    @staticmethod
    def validate_parse_result(return_code: int, _: str, desc: dict[str, Any],   # type: ignore[explicit-any]
                              out: str) -> tuple[bool, bool]:
        is_negative = desc['negative_phase'] == 'parse'

        if return_code == 0:  # passed
            if is_negative:
                return False, False  # negative test passed

            return True, True  # positive test passed

        if return_code == 1:  # failed
            return is_negative and (desc['negative_type'] in out), False

        return False, False  # abnormal

    def generate(self) -> list[str]:
        harness_path = Path(__file__).parent / "test262harness.js"
        stamp_name = f"test262-{self.revision}"
        test_root = download_and_generate(
            name="test262",
            stamp_name=stamp_name,
            url=self.url,
            revision=self.revision,
            download_root=self._source,
            generated_root=self._target,
            test_subdir="test",
            show_progress=self.show_progress,
            process_copy=lambda src, dst: self.prepare_tests(src, dst, harness_path, src.parent),
            force_download=self.force_download
        )
        glob_expression = test_root / "**/*.js"
        return glob(str(glob_expression), recursive=True)

    def prepare_tests(self, src_path: Path, dest_path: Path, harness_path: Path, test262_path: Path) -> None:
        glob_expression = src_path / "**/*.js"
        files = glob(str(glob_expression), recursive=True)
        files = [file for file in files if not file.endswith("FIXTURE.js")]

        with open(harness_path, encoding="utf-8") as file_handler:
            harness = file_handler.read()

        harness = harness.replace('$SOURCE', f'`{harness}`')

        for src_file in files:
            dest_file = src_file.replace(str(src_path), str(dest_path))
            makedirs(path.dirname(dest_file), exist_ok=True)
            self.create_file(src_file, dest_file, harness, test262_path)

    def validate_runtime_result(self, return_code: int, std_err: str,   # type: ignore[explicit-any]
                                desc: dict[str, Any], out: str) -> bool:
        is_negative = (desc['negative_phase'] == 'runtime') or \
                      (desc['negative_phase'] == 'resolution')

        if return_code == 0:  # passed
            if is_negative:
                return False  # negative test passed

            passed = len(std_err) == 0
            if 'async' in desc['flags']:
                passed = passed and bool(self.async_ok.match(out))
            return passed  # positive test passed?

        if return_code == 1:  # failed
            return is_negative and bool(desc['negative_type'] in std_err)

        return False  # abnormal
