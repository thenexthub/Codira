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
from unittest import TestCase
from pathlib import Path
from typing import List

from runner.logger import Log
from runner.utils import write_2_file
from runner.plugins.ets.ets_templates.params import Params
from runner.plugins.ets.ets_templates.template import Template


TEMPLATE_EXTENSION = ".ets"
OUT_EXTENSION = ".ets"
TEMPLATE_EXTENSION_TS = ".ts"
OUT_EXTENSION_TS = ".ts"

_LOGGER = logging.getLogger("runner.plugins.ets.ets_templates.benchmark")


class Benchmark:
    def __init__(self, test_path: Path, output: Path, test_full_name: str, ets_templates_path: Path) -> None:
        self.__input = test_path
        self.__output = output.parent
        self.__name = test_path.name
        self.__full_name = Path(test_full_name).with_suffix('').name
        self.__ets_templates_path = ets_templates_path

    def generate(self) -> List[str]:
        Log.all(_LOGGER, f"Generating test: {self.__name}")
        name_without_ext = self.__input.stem
        params = Params(self.__input, name_without_ext).generate()

        template = Template(self.__input, params, self.__ets_templates_path)
        Log.all(_LOGGER, f"Starting generate test template: {self.__name}")
        rendered_tests = template.render_template()
        TestCase().assertTrue(len(rendered_tests) > 0, f"Internal error: there should be tests in {self.__name}")

        tests = []
        for i, test in enumerate(rendered_tests):
            name = f"{name_without_ext}_{i}" if len(rendered_tests) > 1 else name_without_ext
            full_name = f"{self.__full_name}_{i}" if len(rendered_tests) > 1 else self.__full_name
            test_content = template.generate_test(test, full_name)
            suffix = Path(self.__name).suffix
            file_path = self.__output / f"{name}{suffix}"
            write_2_file(file_path=file_path, content=test_content)
            tests.append(str(file_path))

        Log.all(_LOGGER, f"Finish generating test template for: {self.__name}")
        return tests
