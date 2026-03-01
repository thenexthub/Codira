#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
import logging
import re
import os
from unittest import TestCase
from glob import glob
from typing import Optional, Tuple, Dict, Any

from runner.descriptor import Descriptor
from runner.logger import Log
from runner.options.config import Config
from runner.utils import generate, wrap_with_function
from runner.plugins.work_dir import WorkDir

TEST262_URL = "TEST262_URL"
TEST262_REVISION = "TEST262_REVISION"

_LOGGER = logging.getLogger("runner.plugins.test262.util_test262")


class UtilTest262:
    def __init__(self, config: Config, work_dir: WorkDir):
        self.async_ok = re.compile(r"Test262:AsyncTestComplete")
        self.force_download = config.general.force_download
        self.jit = config.ark.jit.enable
        self.jit_preheat_repeats = config.ark.jit.num_repeats
        self.show_progress = config.general.show_progress
        self.work_dir = work_dir
        self.test262_url: Optional[str] = os.getenv(TEST262_URL)
        self.test262_revision: Optional[str] = os.getenv(TEST262_REVISION)
        if self.test262_url is None:
            Log.exception_and_raise(_LOGGER, f"No {TEST262_URL} environment variable set", EnvironmentError)
        if self.test262_revision is None:
            Log.exception_and_raise(_LOGGER, f"No {TEST262_REVISION} environment variable set", EnvironmentError)

    @staticmethod
    def process_descriptor(descriptor: Descriptor) -> Dict[str, Any]:
        desc = descriptor.parse_descriptor()

        includes = []

        if "includes" in desc:
            includes.extend(desc["includes"])
            includes.extend(['assert.js', 'sta.js'])

        if 'flags' in desc and 'async' in desc["flags"]:
            includes.extend(['doneprintHandle.js'])

        negative_phase = desc["negative_phase"] if 'negative_phase' in desc else 'pass'
        negative_type = desc["negative_type"] if 'negative_type' in desc else ''

        # negative_phase: pass, parse, resolution, runtime
        return {
            'flags': desc["flags"] if 'flags' in desc else [],
            'negative_phase': negative_phase,
            'negative_type': negative_type,
            'includes': includes,
        }

    @staticmethod
    def validate_parse_result(return_code: int, cerr: str, desc: Dict[str, Any], cout: str) -> Tuple[bool, bool]:
        is_negative = (desc['negative_phase'] == 'parse')

        if return_code == 0:  # passed
            if is_negative:
                return False, False  # negative test failed

            return True, True  # positive test passed

        if return_code == 1:  # failed
            # NOTE(pronai) use runner.parser #30090
            real_name_error = "Syntax error" if desc['negative_type'] == "SyntaxError" else desc['negative_type']
            # NOTE(pronai) re-examine during #29808
            return is_negative and (real_name_error in cerr or real_name_error in cout), False

        return False, False  # abnormal

    def generate(self, harness_path: str) -> str:
        stamp_name = f"test262-{self.test262_revision}"
        if self.jit and self.jit_preheat_repeats > 1:
            stamp_name += f"-jit-{self.jit_preheat_repeats}"
        test = TestCase()
        if self.test262_url is not None and self.test262_revision is not None:
            return generate(
                name="test262",
                stamp_name=stamp_name,
                url=self.test262_url,
                revision=self.test262_revision,
                generated_root=self.work_dir.gen,
                test_subdir="test",
                show_progress=self.show_progress,
                process_copy=lambda src, dst: self.prepare_tests(src, dst, harness_path, os.path.dirname(src)),
                force_download=self.force_download
            )
        test.assertFalse(self.test262_url is None)
        test.assertFalse(self.test262_revision is None)
        return ""

    def prepare_tests(self, src_path: str, dest_path: str, harness_path: str, test262_path: str) -> None:
        glob_expression = os.path.join(src_path, "**/*.js")
        files = glob(glob_expression, recursive=True)
        files = [file for file in files if not file.endswith("FIXTURE.js")]

        with os.fdopen(os.open(harness_path, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
            harness = file_handler.read()

        harness = harness.replace('$SOURCE', f'`{harness}`')

        for src_file in files:
            dest_file = src_file.replace(src_path, dest_path)
            os.makedirs(os.path.dirname(dest_file), exist_ok=True)
            self.create_file(src_file, dest_file, harness, test262_path)

    def create_file(self, input_file: str, output_file: str, harness: str, test262_dir: str) -> None:
        descriptor = Descriptor(input_file)
        desc = UtilTest262.process_descriptor(descriptor)

        out_str = descriptor.get_header()
        out_str += "\n"
        out_str += harness

        if 'includes' in desc:
            for include in desc['includes']:
                out_str += f"//------------ {include} start ------------\n"
                include_file = os.path.join(test262_dir, 'harness', include)
                with os.fdopen(os.open(include_file, os.O_RDONLY, 0o755), 'r', encoding="utf-8") as file_handler:
                    harness_str = file_handler.read()
                out_str += harness_str
                out_str += f"//------------ {include} end ------------\n"
                out_str += "\n"

        if self.jit and self.jit_preheat_repeats > 1:
            out_str += wrap_with_function(descriptor.get_content(), self.jit_preheat_repeats)
        else:
            out_str += descriptor.get_content()

        with os.fdopen(os.open(output_file, os.O_WRONLY | os.O_CREAT, 0o755), 'w', encoding="utf-8") as output:
            output.write(out_str)

    def validate_runtime_result(self, return_code: int, std_err: str, desc: Dict[str, Any], out: str) -> bool:
        is_negative = (desc['negative_phase'] == 'runtime') or \
                      (desc['negative_phase'] == 'resolution')

        if return_code == 0:  # passed
            if is_negative:
                return False  # negative test failed

            # NOTE(pronai) re-examine below comment for #29808
            # We don't check std_err string is empty because in case of eval code,
            # errors are wrote in stderr and it is correct
            passed = True
            if 'async' in desc['flags']:
                passed = passed and bool(self.async_ok.search(out))
            return passed  # positive test passed?

        if return_code == 1:  # failed
            return is_negative and bool(desc['negative_type'] in std_err)

        return False  # abnormal
