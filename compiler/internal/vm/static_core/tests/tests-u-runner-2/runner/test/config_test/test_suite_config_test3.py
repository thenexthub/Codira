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
from typing import ClassVar, cast
from unittest.mock import patch

from runner.options.cli_options import CliOptions
from runner.test.config_test.data import data_collections
from runner.test.test_utils import compare_dicts


class TestSuiteConfigTest3(unittest.TestCase):
    workflow_name = "config-1"
    test_suite_name = "test_suite3"
    current_path = Path(__file__).parent
    data_folder = current_path / "data"
    collections = data_collections.collections
    test_environ: ClassVar[dict[str, str]] = {
        'ARKCOMPILER_RUNTIME_CORE_PATH': Path.cwd().as_posix(),
        'ARKCOMPILER_ETS_FRONTEND_PATH': Path.cwd().as_posix(),
        'PANDA_BUILD': Path.cwd().as_posix(),
        'WORK_DIR': Path.cwd().as_posix()
    }

    @patch('runner.utils.get_config_workflow_folder', lambda: TestSuiteConfigTest3.data_folder)
    @patch('runner.utils.get_config_test_suite_folder', lambda: TestSuiteConfigTest3.data_folder)
    @patch.dict(os.environ, test_environ, clear=True)
    def test_collections1(self) -> None:
        args = [self.workflow_name, self.test_suite_name]
        actual = CliOptions(args)
        actual_data = cast(dict, actual.data.get("test_suite3.data"))
        self.assertIn("collections", actual_data)
        actual_collections = cast(dict, actual_data["collections"])
        compare_dicts(self, actual_collections, self.collections)
