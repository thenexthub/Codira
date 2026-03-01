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
from pathlib import Path
from unittest import TestCase
from unittest.mock import MagicMock, patch

from runner.init_runner import InitRunner
from runner.test.test_utils import random_suffix


class InitRunnerTest01(TestCase):
    def setUp(self) -> None:
        self.global_env_name = f".urunner.{random_suffix()}"
        self.global_env_path = Path.cwd() / self.global_env_name

    def test_is_runner_initialized_exist(self) -> None:
        # preparation
        self.global_env_path.touch(exist_ok=True)
        # test
        init_runner = InitRunner(self.global_env_name)
        self.assertTrue(init_runner.is_runner_initialized())
        # clear up
        self.global_env_path.unlink(missing_ok=True)

    @patch('runner.init_runner.InitRunner.is_runner_initialized', return_value=False)
    def test_is_runner_initialized_not_exist(self, _: MagicMock) -> None:
        # preparation
        self.global_env_path.unlink(missing_ok=True)
        # test
        init_runner = InitRunner(self.global_env_name)
        self.assertFalse(init_runner.is_runner_initialized())

    def test_search_global_env_current(self) -> None:
        # preparation
        self.global_env_path.touch(exist_ok=True)
        # test
        init_runner = InitRunner(self.global_env_name)
        self.assertEqual(init_runner.urunner_env_path, self.global_env_path)
        # clear up
        self.global_env_path.unlink(missing_ok=True)

    def test_search_global_env_parent(self) -> None:
        # preparation
        self.global_env_path = self.global_env_path.parent.with_name(self.global_env_path.name)
        self.global_env_path.touch(exist_ok=True)
        # test
        init_runner = InitRunner(self.global_env_name)
        self.assertEqual(init_runner.urunner_env_path, self.global_env_path)
        # clear up
        self.global_env_path.unlink(missing_ok=True)

    def test_search_global_env_home(self) -> None:
        # preparation
        self.global_env_path.unlink(missing_ok=True)
        # test
        init_runner = InitRunner()
        global_env_path = init_runner.search_global_env(self.global_env_name)
        self.assertEqual(global_env_path, Path(init_runner.urunner_env_default))

    def test_should_runner_initialize_yes1(self) -> None:
        self.assertTrue(InitRunner.should_runner_initialize(["runner.sh", "init"]))

    def test_should_runner_initialize_yes2(self) -> None:
        cli_options = ["runner.sh", "es2panda", "ets-cts", "init", "--show-progress"]
        self.assertTrue(InitRunner.should_runner_initialize(cli_options))

    def test_should_runner_initialize_no(self) -> None:
        self.assertFalse(InitRunner.should_runner_initialize(["runner.sh", "only-es2panda", "ets-cts"]))
