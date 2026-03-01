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
import re
import shutil
from io import StringIO
from pathlib import Path
from typing import cast
from unittest import TestCase
from unittest.mock import patch

from runner.init_runner import InitRunner, MandatoryProp, MandatoryProps
from runner.test.test_utils import random_suffix


class InitCLIRunnerTest(TestCase):
    current_folder = Path.cwd()
    local_home = current_folder / "local_home"

    def setUp(self) -> None:
        self.cfg_name = f".urunner.{random_suffix()}"

    def check_cfg(self, expected_cfg: Path) -> None:
        # before
        expected_cfg.unlink(missing_ok=True)
        mandatory_props = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': MandatoryProp("aaa", is_path=True, require_exist=True),
            'ARKCOMPILER_ETS_FRONTEND_PATH': MandatoryProp("bbb", is_path=True, require_exist=True),
            'PANDA_BUILD': MandatoryProp("ccc", is_path=True, require_exist=True),
            'WORK_DIR': MandatoryProp("ddd", is_path=True, require_exist=False)
        }
        for _, (prop_path, _2, require_exist) in mandatory_props.items():
            folder = self.current_folder / cast(str, prop_path)
            if require_exist:
                folder.mkdir(parents=True, exist_ok=True)

        # test
        runner = InitRunner(self.cfg_name)
        runner.initialize(mandatory_props)
        self.assertEqual(runner.urunner_env_path, expected_cfg)
        self.assertTrue(runner.urunner_env_path.exists())
        lines = runner.urunner_env_path.read_text(encoding="utf-8").split("\n")
        self.assertEqual(len(lines), len(mandatory_props))
        for name, prop in mandatory_props.items():
            prop_mask = re.compile(f"{name}=.*{prop.value}$")
            self.assertTrue(any(prop_mask.match(line) for line in lines))

        # clear up
        expected_cfg.unlink(missing_ok=True)
        for _, (prop_path, _, require_exist) in mandatory_props.items():
            folder = self.current_folder / cast(str, prop_path)
            if require_exist:
                shutil.rmtree(folder, ignore_errors=True)

    @patch('sys.argv', ["runner.sh", "init", "--home"])
    @patch.dict(os.environ, {}, clear=True)
    @patch('pathlib.Path.home', lambda: InitCLIRunnerTest.local_home)
    def test_initialize_home(self) -> None:
        """
        Test that config is created at the user's home directory
        """
        expected_cfg = Path.home() / self.cfg_name
        self.check_cfg(expected_cfg)

    @patch('sys.argv', ["runner.sh", "init", "--local", ".."])
    def test_initialize_local_explicit(self) -> None:
        """
        Test that config is created at the specified folder
        """
        expected_cfg = Path.cwd().parent / self.cfg_name
        self.check_cfg(expected_cfg)

    @patch('sys.argv', ["runner.sh", "init"])
    def test_initialize_only_init(self) -> None:
        """
        Test that if nothing is specified the config is created at the current folder
        """
        expected_cfg = Path.cwd() / self.cfg_name
        self.check_cfg(expected_cfg)

    def check_help(self, expected_props: MandatoryProps, expected_help: dict[str, int], handle: StringIO) -> None:
        runner = InitRunner()
        try:
            runner.initialize(expected_props)
        except SystemExit:
            std_out = str(handle.getvalue())
            for name, expected_quant in expected_help.items():
                actual_quant = len(re.findall(name, std_out))
                self.assertEqual(actual_quant, expected_quant)

    @patch('sys.argv', ["runner.sh", "init", "--help"])
    @patch("sys.stdout", new_callable=StringIO)
    def test_initialize_help_min(self, handle: StringIO) -> None:
        """
        Test that info only about base options is output if no mandatory props are set
        """
        expected = {
            "--home": 2,
            "--local": 2,
            "--arkcompiler_runtime_core_path": 0,
            "--arkcompiler_ets_frontend_path": 0,
            "--panda_build": 0,
            "--work_dir": 0,
        }
        self.check_help(expected_props={}, expected_help=expected, handle=handle)

    @patch('sys.argv', ["runner.sh", "init", "--help"])
    @patch("sys.stdout", new_callable=StringIO)
    def test_initialize_help_max(self, handle: StringIO) -> None:
        """
        Test that info only about mandatory options is output as well if they are set
        """
        expected_content = {
            'ARKCOMPILER_RUNTIME_CORE_PATH': MandatoryProp("aaa", is_path=True, require_exist=True),
            'ARKCOMPILER_ETS_FRONTEND_PATH': MandatoryProp("bbb", is_path=True, require_exist=True),
            'PANDA_BUILD': MandatoryProp("ccc", is_path=True, require_exist=True),
            'WORK_DIR': MandatoryProp("ddd", is_path=True, require_exist=False)
        }
        expected = {
            "--home": 2,
            "--local": 2,
            "--arkcompiler_runtime_core_path": 2,
            "--arkcompiler_ets_frontend_path": 2,
            "--panda_build": 2,
            "--work_dir": 2,
        }
        self.check_help(expected_props=expected_content, expected_help=expected, handle=handle)
