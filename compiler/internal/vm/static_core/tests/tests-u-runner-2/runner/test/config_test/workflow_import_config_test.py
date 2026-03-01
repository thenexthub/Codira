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

from runner.options.cli_options import get_args
from runner.options.options import IOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_test_suite import TestSuiteOptions
from runner.options.options_workflow import WorkflowOptions
from runner.options.step import StepKind


class WorkflowImportConfigTest(unittest.TestCase):
    data_folder: ClassVar[Path] = Path(__file__).with_name("data")
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': Path.cwd().as_posix()
    }
    sys_argv: ClassVar[list[str]] = ["runner.sh", "config-with-import", "test_suite1"]

    @staticmethod
    def prepare_test() -> tuple[TestSuiteOptions, WorkflowOptions]:
        args = get_args()
        general = GeneralOptions(args, IOptions())
        test_suite = TestSuiteOptions(args, general)
        workflow = WorkflowOptions(cfg_content=args, parent_test_suite=test_suite)
        return test_suite, workflow

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowImportConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowImportConfigTest.data_folder)
    @patch('sys.argv', sys_argv)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_loaded_steps(self) -> None:
        _, workflow = self.prepare_test()
        self.assertEqual(len(workflow.steps), 1)
        self.assertSetEqual(
            {step.name for step in workflow.steps},
            {'imported.echo'}
        )

    @patch('runner.utils.get_config_workflow_folder', lambda: WorkflowImportConfigTest.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: WorkflowImportConfigTest.data_folder)
    @patch('sys.argv', sys_argv)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_echo_step(self) -> None:
        _, workflow = self.prepare_test()
        echo = [step for step in workflow.steps if step.name == 'imported.echo']
        self.assertEqual(len(echo), 1, "Step 'es2panda' not found")
        step = echo[0]
        self.assertEqual(str(step.executable_path), "/usr/bin/echo")
        self.assertEqual(step.step_kind, StepKind.OTHER)
        self.assertEqual(step.timeout, 30)
        self.assertFalse(step.can_be_instrumented)
        self.assertSetEqual(set(step.args), {
            "--arktsconfig=world",
            "--thread=3",
            "--extension=ets",
            "${test-id}"
        })
