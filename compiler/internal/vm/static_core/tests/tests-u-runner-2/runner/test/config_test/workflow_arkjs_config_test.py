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

import os
import unittest
from pathlib import Path
from typing import ClassVar
from unittest.mock import patch

from runner.options.options import IOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.options_workflow import WorkflowOptions
from runner.options.step import StepKind
from runner.test.config_test.data import data_3


class WorkflowArkJsConfigTest(unittest.TestCase):
    current_path = Path(__file__).parent
    data_folder = current_path / "data"
    args = data_3.args
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': Path.cwd().as_posix()
    }

    def prepare_test(self) -> tuple[TestSuiteOptions, WorkflowOptions]:
        general = GeneralOptions(self.args, IOptions())
        test_suite = TestSuiteOptions(self.args, general)
        workflow = WorkflowOptions(cfg_content=self.args, parent_test_suite=test_suite)
        return test_suite, workflow

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_loaded_steps(self) -> None:
        _, workflow = self.prepare_test()
        self.assertEqual(len(workflow.steps), 2)
        self.assertSetEqual(
            {step.name for step in workflow.steps},
            {'ohos_es2abc', 'ark_js_napi_cli'}
        )

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_ohos_es2abc_step(self) -> None:
        _, workflow = self.prepare_test()
        echo = [step for step in workflow.steps if step.name == 'ohos_es2abc']
        self.assertEqual(len(echo), 1, "Step 'es2panda' not found")
        step = echo[0]
        self.assertEqual(str(step.executable_path), "/usr/bin/echo")
        self.assertEqual(step.step_kind, StepKind.OTHER)
        self.assertEqual(step.timeout, 30)
        self.assertSetEqual(set(step.args), {
            "--merge-abc",
            "--extension=ts",
            "--module",
            "--output=./intermediate/${test-id}.abc",
            "./gen/${test-id}"
        })

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_ark_js_napi_cli_step(self) -> None:
        _, workflow = self.prepare_test()
        echo = [step for step in workflow.steps if step.name == 'ark_js_napi_cli']
        self.assertEqual(len(echo), 1, "Step 'es2panda' not found")
        step = echo[0]
        self.assertEqual(step.executable_path,
                         Path(__file__).parent.parent.parent /
                         "extensions/generators/ets_arkjs_xgc/ark_js_napi_cli_runner.sh")
        self.assertEqual(step.step_kind, StepKind.OTHER)
        self.assertEqual(step.timeout, 60)
        self.assertSetEqual(set(step.args), {
            "--work-dir",
            "./intermediate",
            "--build-dir .",
            "--stub-file ./gen/arkcompiler/ets_runtime/stub.an",
            "--enable-force-gc=false",
            "--open-ark-tools=true",
            "--entry-point",
            "${test-id}",
            "./intermediate/${test-id}.abc"
        })
        self.assertSetEqual(set(step.env), {
            "LD_LIBRARY_PATH"
        })
        self.assertEqual(step.env['LD_LIBRARY_PATH'], [
            "./arkcompiler/runtime_core:"]
                         )

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowArkJsConfigTest.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_test_suite(self) -> None:
        test_suite, _ = self.prepare_test()
        self.assertEqual(test_suite.suite_name, "test_suite1")
        self.assertEqual(test_suite.test_root, Path.cwd().resolve())
        self.assertEqual(test_suite.list_root, Path.cwd().resolve())
        self.assertEqual(test_suite.extension(), "sts")
        self.assertEqual(test_suite.load_runtimes(), "ets")
        self.assertEqual(test_suite.work_dir, ".")
