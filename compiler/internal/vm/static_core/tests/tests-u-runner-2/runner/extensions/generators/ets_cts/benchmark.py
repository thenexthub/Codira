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

import re
from pathlib import Path

from runner import common_exceptions
from runner.extensions.generators.ets_cts.params import Params
from runner.extensions.generators.ets_cts.template import Template
from runner.logger import Log
from runner.utils import write_2_file

_LOGGER = Log.get_logger(__file__)

FILE_NAME = "file_name"
FILE_SUFFIX = "test_suffix"


class Benchmark:
    def __init__(self, test_path: Path, output: Path, test_full_name: str, ets_templates_path: Path,
                 out_extension: str) -> None:
        self.__input = test_path
        self.__output = output.parent
        self.__name = test_path.name
        self.__out_extension = out_extension
        self.__full_name = Path(test_full_name).with_suffix('').name
        self.__ets_templates_path = ets_templates_path

    @staticmethod
    def get_name_suffix(test_content: str) -> tuple[str | None, str | None]:
        name_pattern = rf'{FILE_NAME}:\s*([\'"])(.*?)\1'
        suffix_pattern = rf'{FILE_SUFFIX}:\s*([\'"])(.*?)\1'
        test_lines = test_content.split('\n')
        file_name = None
        suffix = None
        for line in test_lines:
            suffix_pattern_match = re.search(suffix_pattern, line)
            name_pattern_match = re.search(name_pattern, line)
            if suffix_pattern_match:
                suffix = suffix_pattern_match.group(2)
            if name_pattern_match:
                file_name = name_pattern_match.group(2)
        return file_name if file_name else None, suffix if suffix else None

    def generate(self) -> list[str]:
        _LOGGER.all(f"Generating test: {self.__name}")
        name_without_ext = self.__input.stem
        params = Params(self.__input, name_without_ext).generate()
        template = Template(self.__input, params, self.__ets_templates_path)
        _LOGGER.all(f"Starting generate test template: {self.__name}")
        rendered_tests = template.render_template()
        if len(rendered_tests) <= 0:
            raise common_exceptions.TestNotExistException(f"{self.__name}: Incorrect test template")

        tests = []
        for i, test in enumerate(rendered_tests):
            file_name, suffix = Benchmark.get_name_suffix(test)
            test_suffix = suffix if suffix is not None else i
            file_name = file_name if file_name else name_without_ext
            name = f"{file_name}_{test_suffix}" if len(rendered_tests) > 1 else file_name
            full_name = f"{self.__full_name}_{test_suffix}" if len(rendered_tests) > 1 else self.__full_name
            test_content = template.generate_test(test, full_name)
            file_path = self.__output / f"{name}{self.__out_extension}"
            write_2_file(file_path=file_path, content=test_content, disallow_overwrite=True)
            tests.append(str(file_path))

        _LOGGER.all(f"Finish generating test template for: {self.__name}")
        return tests
