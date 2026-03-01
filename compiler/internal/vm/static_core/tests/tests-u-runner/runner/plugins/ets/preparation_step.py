#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

import argparse
import logging
import multiprocessing
import os
import re
import shutil
import subprocess
import sys
from abc import ABC, abstractmethod
from glob import glob
from pathlib import Path
from typing import List

from runner import utils
from runner.logger import Log
from runner.options.config import Config
from runner.plugins.ets.ets_suites import EtsSuites
from runner.plugins.ets.ets_templates.ets_templates_generator import EtsTemplatesGenerator
from runner.plugins.ets.ets_templates.test_metadata import get_metadata
from runner.plugins.ets.stdlib_templates.stdlib_templates_generator import StdlibTemplatesGenerator
from runner.generators.ets_func_tests.ets_func_test_template_generator import EtsFuncTestsCodeGenerator
from runner.plugins.ets.utils.exceptions import InvalidFileFormatException, InvalidFileStructureException, \
    UnknownTemplateException
from runner.plugins.ets.utils.file_structure import walk_test_subdirs
from runner.utils import read_file, write_2_file

_LOGGER = logging.getLogger("runner.plugins.ets.generate")


class TestPreparationStep(ABC):
    def __init__(self, test_source_path: Path, test_gen_path: Path, config: Config) -> None:
        self.__test_source_path = test_source_path
        self.__test_gen_path = test_gen_path
        self.config = config

    @property
    def test_source_path(self) -> Path:
        return self.__test_source_path

    @property
    def test_gen_path(self) -> Path:
        return self.__test_gen_path

    @abstractmethod
    def transform(self, force_generated: bool) -> List[str]:
        pass


class CtsTestPreparationStep(TestPreparationStep):
    def __str__(self) -> str:
        return f"Test Generator for '{EtsSuites.CTS.value}' test suite"

    def transform(self, force_generated: bool) -> List[str]:
        ets_templates_generator = EtsTemplatesGenerator(self.test_source_path, self.test_gen_path)
        return ets_templates_generator.generate()


class CustomGeneratorTestPreparationStep(TestPreparationStep):
    def __init__(self, test_source_path: Path, test_gen_path: Path, config: Config, extension: str) -> None:
        super().__init__(test_source_path, test_gen_path, config)
        self.extension = extension

    def __str__(self) -> str:
        return f"Test Generator for '{EtsSuites.CUSTOM.value} - {self.config.custom.suite_name}' test suite"

    def transform(self, force_generated: bool) -> List[str]:
        # call of the custom generator
        if self.test_gen_path.exists() and force_generated:
            shutil.rmtree(self.test_gen_path)
        self.test_gen_path.mkdir(exist_ok=True)
        cmd = [
            self.config.custom.generator,
            "--source", self.test_source_path,
            "--target", self.test_gen_path
        ]
        cmd.extend(self.config.custom.generator_options)
        timeout = 300
        result: List[str] = []
        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            try:
                process.communicate(timeout=timeout)
            except subprocess.TimeoutExpired:
                process.kill()
                Log.exception_and_raise(_LOGGER, f"Generation failed by timeout after {timeout} sec")
            except Exception as ex:  # pylint: disable=broad-except
                Log.exception_and_raise(_LOGGER, f"Generation failed by unknown reason: {ex}")
            finally:
                glob_expression = os.path.join(self.test_gen_path, f"**/*.{self.extension}")
                result.extend(glob(glob_expression, recursive=True))

        return result


class FuncTestPreparationStep(TestPreparationStep):
    def __str__(self) -> str:
        return f"Test Generator for '{EtsSuites.FUNC.value}' test suite"

    @staticmethod
    def generate_template_tests(template_root_path: Path, test_gen_path: Path) -> List[str]:
        """
        Renders all templates and saves them.
        """
        if not template_root_path.is_dir():
            Log.all(_LOGGER, f"ERROR: {str(template_root_path.absolute())} must be a directory")
            return []

        ets_render = StdlibTemplatesGenerator(template_root_path)
        generated_tests = []
        for dir_name in walk_test_subdirs(template_root_path):
            dir_outpath = test_gen_path / dir_name.path.relative_to(template_root_path)
            generated_tests_tmp = ets_render.render_and_write_templates(template_root_path, dir_name.path, dir_outpath)
            generated_tests.extend(generated_tests_tmp)

        return generated_tests

    def transform(self, force_generated: bool) -> List[str]:
        return self.generate_template_tests(self.test_source_path, self.test_gen_path)


class FrontendFuncTestPreparationStep(TestPreparationStep):
    def __str__(self) -> str:
        return f"Test Generator for '{EtsSuites.FUNC.value}' test suite"

    @staticmethod
    def generate_template_tests(template_root_path: Path, test_gen_path: Path) -> List[str]:
        """
        Renders all templates and saves them.
        """
        if not template_root_path.is_dir():
            Log.all(_LOGGER, f"ERROR: {str(template_root_path.absolute())} must be a directory")
            return []

        generated_tests = []
        ets_func_test_render = EtsFuncTestsCodeGenerator(template_root_path)
        for dir_name in walk_test_subdirs(template_root_path):
            dir_outpath = test_gen_path / dir_name.path.relative_to(template_root_path)
            generated_tests_tmp = ets_func_test_render.render_and_write_templates(template_root_path,
                                                                                  dir_name.path,
                                                                                  dir_outpath)
            generated_tests.extend(generated_tests_tmp)

        return generated_tests

    def transform(self, force_generated: bool) -> List[str]:
        return self.generate_template_tests(self.test_source_path, self.test_gen_path)


class ESCheckedTestPreparationStep(TestPreparationStep):
    def __str__(self) -> str:
        return f"Test Generator for '{EtsSuites.ESCHECKED.value}' test suite"

    def transform(self, force_generated: bool) -> List[str]:
        confs = list(glob(os.path.join(self.test_source_path, "**/*.yaml"), recursive=True))
        generator_root = (Path(self.config.general.static_core_root) /
                          "tests" /
                          "tests-u-runner" /
                          "tools" /
                          "generate-es-checked")
        generator_executable = generator_root / "main.rb"
        res = subprocess.run(
            [
                generator_executable,
                '--proc',
                str(self.config.general.processes),
                '--out',
                self.test_gen_path,
                '--tmp',
                self.test_gen_path / 'tmp',
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
            Log.default(_LOGGER,
                        'Failed to run es cross-validator, please, make sure that '
                        'all required tools are installed (see tests-u-runner/readme.md#ets-es-checked-dependencies)')
            Log.exception_and_raise(_LOGGER, f"invalid return code {res.returncode}\n" + res.stdout + res.stderr)
        glob_expression = os.path.join(self.test_gen_path, "**/*.ets")
        return list(glob(glob_expression, recursive=True))


class CopyStep(TestPreparationStep):
    def __str__(self) -> str:
        return "Test preparation step for any ets test suite: copying"

    def transform(self, force_generated: bool) -> List[str]:
        utils.copy(self.test_source_path, self.test_gen_path, remove_if_exist=False)
        glob_expression = os.path.join(self.test_gen_path, "**/*.ets")
        return list(glob(glob_expression, recursive=True))


class JitStep(TestPreparationStep):
    __main_pattern = r"\bfunction\s+(?P<main>main)\b"
    __param_pattern = r"\s*\(\s*(?P<param>(?P<param_name>\w+)(\s*:\s*\w+))?\s*\)"
    __return_pattern = r"\s*(:\s*(?P<return_type>\w+)\b)?"
    __throws_pattern = r"\s*(?P<throws>throws)?"
    __indent = "    "

    def __init__(self, test_source_path: Path, test_gen_path: Path, config: Config, num_repeats: int = 0):
        super().__init__(test_source_path, test_gen_path, config)
        self.num_repeats = num_repeats
        self.main_regexp = re.compile(
            f"{self.__main_pattern}{self.__param_pattern}{self.__return_pattern}{self.__throws_pattern}",
            re.MULTILINE
        )

    def __str__(self) -> str:
        return "Test preparation step for any ets test suite: transforming for JIT testing"

    def transform(self, force_generated: bool) -> List[str]:
        glob_expression = os.path.join(self.test_gen_path, "**/*.ets")
        tests = list(glob(glob_expression, recursive=True))
        with multiprocessing.Pool(processes=self.config.general.processes) as pool:
            run_tests = pool.imap_unordered(self.jit_transform_one_test, tests, chunksize=self.config.general.chunksize)
            pool.close()
            pool.join()

        return list(run_tests)

    def jit_transform_one_test(self, test_path: str) -> str:
        metadata = get_metadata(Path(test_path))
        is_convert = (not metadata.tags.not_a_test and
                      not metadata.tags.compile_only and
                      not metadata.tags.no_warmup)
        if not is_convert:
            return test_path

        original = read_file(test_path)
        match = self.main_regexp.search(original)
        if match is None:
            return test_path

        is_int_main = match.group("return_type") == "int"
        return_type = ": int" if is_int_main else ""
        throws = "throws " if match.group("throws") else ""
        param_line = param if (param := match.group("param")) is not None else ""
        param_name = param if (param := match.group("param_name")) is not None else ""

        tail = [f"\nfunction main({param_line}){return_type} {throws}{{"]
        if is_int_main:
            tail.append(f"{self.__indent}let result = 0")
        tail.append(f"{self.__indent}for(let i = 0; i < {self.num_repeats}; i++) {{")
        if is_int_main:
            tail.append(f"{self.__indent * 2}result += main_run({param_name})")
        else:
            tail.append(f"{self.__indent * 2}main_run({param_name})")
        tail.append(f"{self.__indent}}}")
        if is_int_main:
            tail.append(f"{self.__indent}return result;")
        tail.append("}")

        result = self.main_regexp.sub(lambda arg: arg.group(0).replace("main", "main_run"), original)
        result += "\n".join(tail)
        write_2_file(test_path, result)
        return test_path


def command_line_parser() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generator test runner")
    parser.add_argument("-t", "--templates-dir", type=Path, dest='templates_dir',
                        help="Path to a root directory that contains test templates and parameters",
                        required=True)
    parser.add_argument("-o", "--output-dir", type=Path, dest='output_dir',
                        help="Path to output directory. Output directory and all" +
                             " subdirectories are created automatically",
                        required=True)
    return parser.parse_args()


def main() -> None:
    args = command_line_parser()

    try:
        if not FuncTestPreparationStep.generate_template_tests(args.templates_dir, args.output_dir):
            sys.exit(1)
    except InvalidFileFormatException as inv_format_exp:
        Log.all(_LOGGER, f'Error:  {inv_format_exp.message}')
        sys.exit(1)
    except InvalidFileStructureException as inv_fs_exp:
        Log.all(_LOGGER, f'Error:  {inv_fs_exp.message}')
        sys.exit(1)
    except UnknownTemplateException as unknown_template_exp:
        Log.all(_LOGGER, f"{unknown_template_exp.filepath}: exception while processing template:")
        Log.all(_LOGGER, f"\t {repr(unknown_template_exp.exception)}")
        sys.exit(1)
    Log.all(_LOGGER, "Finished")


if __name__ == "__main__":
    main()
