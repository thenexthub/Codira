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

import shutil
import logging
from pathlib import Path
from typing import Set, List

from runner.logger import Log
from runner.plugins.ets.utils.exceptions import InvalidFileFormatException, InvalidFileStructureException, \
    UnknownTemplateException
from runner.plugins.ets.ets_templates.benchmark import Benchmark, TEMPLATE_EXTENSION, TEMPLATE_EXTENSION_TS

_LOGGER = logging.getLogger("runner.plugins.ets.ets_templates.ets_templates_generator")

EXPECTED_FILE_EXTENSION = ".expected"


class EtsTemplatesGenerator:
    def __init__(self, root_path: Path, gen_path: Path) -> None:
        self.__root_path = root_path
        self.__output_path = gen_path
        self.generated_tests: List[str] = []

        if not self.__root_path.is_dir():
            Log.exception_and_raise(_LOGGER, f"{str(self.__root_path.absolute())} must be a directory")

    def dfs(self, path: Path, seen: Set) -> None:
        if not path.exists() or path in seen:
            return
        seen.add(path)

        if path.is_dir():
            for i in sorted(path.iterdir()):
                self.dfs(i, seen)
        elif path.suffix == TEMPLATE_EXTENSION:
            self.__generate_test(path)
        elif path.suffix == TEMPLATE_EXTENSION_TS:
            self.__generate_test(path)
        elif EXPECTED_FILE_EXTENSION in path.suffixes:
            test_full_name = path.relative_to(self.__root_path)
            output = self.__output_path / test_full_name
            shutil.copy(path, output)


    def generate(self) -> List[str]:
        Log.all(_LOGGER, "Starting generate test")
        if self.__output_path.exists():
            shutil.rmtree(self.__output_path)
        try:
            self.dfs(self.__root_path, set())
        except InvalidFileFormatException as inv_format_exp:
            Log.exception_and_raise(_LOGGER, inv_format_exp.message)
        except InvalidFileStructureException as inv_fs_exp:
            Log.exception_and_raise(_LOGGER, inv_fs_exp.message)
        except UnknownTemplateException as unknown_template_exp:
            Log.default(_LOGGER, f"\t {repr(unknown_template_exp.exception)}")
            Log.exception_and_raise(_LOGGER, f"{unknown_template_exp.filepath}: exception while processing template:")
        Log.all(_LOGGER, "Generation finished!")
        return self.generated_tests

    def __generate_test(self, path: Path) -> None:
        test_full_name = str(path.relative_to(self.__root_path))
        output = self.__output_path / test_full_name
        bench = Benchmark(path, output, test_full_name, self.__root_path)
        self.generated_tests.extend(bench.generate())
