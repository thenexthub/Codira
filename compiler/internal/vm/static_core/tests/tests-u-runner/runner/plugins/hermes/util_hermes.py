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
import re
import subprocess
import os
from unittest import TestCase
from glob import glob
from typing import Optional

from runner.logger import Log
from runner.options.config import Config
from runner.utils import generate, wrap_with_function
from runner.plugins.work_dir import WorkDir

HERMES_URL = "HERMES_URL"
HERMES_REVISION = "HERMES_REVISION"

_LOGGER = logging.getLogger("runner.hermes.util_hermes")


class UtilHermes:
    def __init__(self, config: Config, work_dir: WorkDir) -> None:
        self.check_expr = re.compile(r"^\s*//\s?(?:CHECK-NEXT|CHECK-LABEL|CHECK):(.+)", re.MULTILINE)

        self.show_progress = config.general.show_progress
        self.force_download = config.general.force_download
        self.jit = config.ark.jit.enable
        self.jit_preheat_repeats = config.ark.jit.num_repeats

        self.work_dir = work_dir

        self.hermes_url: Optional[str] = os.getenv(HERMES_URL)
        self.hermes_revision: Optional[str] = os.getenv(HERMES_REVISION)
        if self.hermes_url is None:
            Log.exception_and_raise(_LOGGER, f"No {HERMES_URL} environment variable set", EnvironmentError)
        if self.hermes_revision is None:
            Log.exception_and_raise(_LOGGER, f"No {HERMES_REVISION} environment variable set", EnvironmentError)

    def generate(self) -> str:
        stamp_name = f"hermes-{self.hermes_revision}"
        if self.jit and self.jit_preheat_repeats > 1:
            stamp_name += f"-jit-{self.jit_preheat_repeats}"
        test = TestCase()
        if self.hermes_url is not None and self.hermes_revision is not None:
            return generate(
                name="hermes",
                stamp_name=stamp_name,
                url=self.hermes_url,
                revision=self.hermes_revision,
                generated_root=self.work_dir.gen,
                test_subdir="test/hermes",
                show_progress=self.show_progress,
                process_copy=self.process_copy,
                force_download=self.force_download
            )
        test.assertFalse(self.hermes_url is None)
        test.assertFalse(self.hermes_revision is None)
        return ""

    def process_copy(self, src_path: str, dst_path: str) -> None:
        Log.all(_LOGGER, "Generating tests")

        glob_expression = os.path.join(src_path, "**/*.js")
        files = glob(glob_expression, recursive=True)

        for src_file in files:
            dest_file = src_file.replace(src_path, dst_path)
            os.makedirs(os.path.dirname(dest_file), exist_ok=True)
            self.create_file(src_file, dest_file)

    def create_file(self, src_file: str, dest_file: str) -> None:
        with os.fdopen(os.open(src_file, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_pointer:
            input_str = file_pointer.read()

        if self.jit and self.jit_preheat_repeats > 1:
            out_str = wrap_with_function(input_str, self.jit_preheat_repeats)
        else:
            out_str = input_str

        with os.fdopen(os.open(dest_file, os.O_RDWR | os.O_CREAT, 0o755), 'w', encoding="utf-8") as output:
            output.write(out_str)

    def run_filecheck(self, test_file: str, actual_output: str) -> bool:
        with open(test_file, 'r', encoding="utf-8") as file_pointer:
            input_str = file_pointer.read()
        if not re.match(self.check_expr, input_str):
            return True

        TestCase().assertTrue(actual_output is not None, "Expected some output to check")
        cmd = ['FileCheck-14', test_file]
        with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE) as process:
            try:
                if process.stdin is None:
                    Log.exception_and_raise(_LOGGER, f"Unexpected error on checking {test_file}")
                process.stdin.write(actual_output.encode('utf-8'))
                process.communicate(timeout=10)
                return_code = process.returncode
            except subprocess.TimeoutExpired as ex:
                error_message = f"Timeout: {' '.join(cmd)} failed with {ex}"
                _LOGGER.error(error_message)
                process.kill()
                return_code = -1
            except Exception as ex:  # pylint: disable=broad-except
                error_message = f"Exception{' '.join(cmd)} failed with {ex}"
                _LOGGER.error(error_message)
                process.kill()
                return_code = -1

        return return_code == 0
