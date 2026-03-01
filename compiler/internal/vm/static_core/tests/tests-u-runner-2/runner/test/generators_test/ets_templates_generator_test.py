#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
import shutil
from collections.abc import Callable
from pathlib import Path
from typing import ClassVar
from unittest import TestCase
from unittest.mock import patch

from runner.common_exceptions import InvalidConfiguration
from runner.extensions.generators.ets_cts.ets_templates_generator import EtsTemplatesGenerator
from runner.options.config import Config
from runner.test import test_utils
from runner.test.generators_test.ets_data import data_test_suite0


class EtsGeneratorTest(TestCase):
    args = data_test_suite0.args
    get_instance_id: ClassVar[Callable[[], str]] = lambda: test_utils.create_runner_test_id(__file__)
    config: Config

    def load_config(self) -> None:
        self.config = Config(self.args)

    def generate_tests(self, test_source_path: Path, test_gen_path: Path) -> list[str]:
        ets_test_generator = EtsTemplatesGenerator(test_source_path, test_gen_path, self.config)
        ets_test_generator.generate()
        generated_tests = sorted(test.stem for test in test_gen_path.rglob("*") if test.is_file())
        return generated_tests

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_disable_overwrite(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data_tests")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.load_config()

        ets_test_generator = EtsTemplatesGenerator(test_source_path, test_gen_path, self.config)
        self.assertRaises(InvalidConfiguration, ets_test_generator.generate)

        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_test_names = ['types_declaration_0',
                               'types_declaration_1',
                               'types_declaration_2',
                               'simple_test',
                               'simple_types',
                               'types_example']
        self.assertEqual(sorted(expected_test_names), sorted(generated_tests))
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_with_custom_suffixes(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data_suffixes")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_test_names = ['different_file_name_a',
                               'different_file_name_b',
                               'different_file_name_c']
        self.assertEqual(expected_test_names, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_one_test_file(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.args['test_suite.parameters.test-file'] = 'simple_test.ets'
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_tests = ['simple_test']
        self.assertEqual(expected_tests, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_tests_by_filter(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.args['test_suite.parameters.filter'] = '*declaration*'
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_tests = ['types_declaration_0',
                            'types_declaration_1',
                            'types_declaration_2']
        self.assertEqual(expected_tests, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_tests_by_filter_somwhere(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.args['test_suite.parameters.filter'] = '**/*types*'
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_tests = ['simple_types',
                          'types_declaration_0',
                          'types_declaration_1',
                          'types_declaration_2',
                          'types_example']
        self.assertEqual(expected_tests, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_tests_by_filter_startswith(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.args['test_suite.parameters.filter'] = 'types*'
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_tests = ['types_declaration_0',
                          'types_declaration_1',
                          'types_declaration_2',
                         ]
        self.assertEqual(expected_tests, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)

    @patch.dict(os.environ, {
        'ARKCOMPILER_RUNTIME_CORE_PATH': ".",
        'ARKCOMPILER_ETS_FRONTEND_PATH': ".",
        'PANDA_BUILD': ".",
        'WORK_DIR': f"work-{test_utils.random_suffix()}"
    }, clear=True)
    @patch('runner.options.local_env.LocalEnv.get_instance_id', get_instance_id)
    def test_generate_tests_by_filter_mix(self) -> None:
        test_source_path: Path = Path(__file__).with_name("ets_data")
        test_gen_path: Path = Path(__file__).with_name("gen")
        shutil.rmtree(test_gen_path, ignore_errors=True)
        self.args['test_suite.parameters.filter'] = 'inner*/test*/*types*'
        self.load_config()

        generated_tests = self.generate_tests(test_source_path, test_gen_path)
        self.assertTrue(test_gen_path.exists())

        expected_tests = ['simple_types']
        self.assertEqual(expected_tests, generated_tests)
        work_dir = Path(os.environ["WORK_DIR"])
        shutil.rmtree(work_dir, ignore_errors=True)
