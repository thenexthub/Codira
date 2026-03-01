#!/usr/bin/env python3
# -- coding: utf-8 --
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
from os import path
from pathlib import Path
from typing import List, Any

from runner.enum_types.params import TestEnv
from runner.test_file_based import TestFileBased

_LOGGER = logging.getLogger("runner.plugins.declgen_ets2ets.test_declgenets2ets")


class TestDeclgen(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        super().__init__(test_env, test_path, flags, test_id)
        base_path = path.splitext(self.path)[0]
        self.source_path = f"{base_path}.ets"
        self.expected_path = f"{base_path}-expected.txt"

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "declgen_ets2ets"

    @property
    def generated_abc_from_decl_path(self) -> Path:
        abc_filename = f"{Path(self.source_path).stem}.abc"
        return self.default_work_dir_root / "from_decl" / abc_filename

    def do_run(self) -> TestFileBased:
        self._prepare_work_dirs()

        decl_filename = f"{Path(self.source_path).stem}.d.ets"
        abc_filename = f"{Path(self.source_path).stem}.abc"

        generated_decl_path = self.default_work_dir_root / decl_filename
        generated_decl_abc_path = self.default_work_dir_root / "from_src" / abc_filename
        generated_abc_path = self.generated_abc_from_decl_path

        self._ensure_parent_dirs([generated_decl_abc_path, generated_abc_path])

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=[
                "--generate-decl:enabled",
                f"--generate-decl:path={generated_decl_path}",
            ],
            test_abc=str(generated_decl_abc_path),
            result_validator=self.compare_decl_output
        )

        if not self.passed:
            return self

        if not self._validate_decl_to_abc_generation(generated_decl_path, generated_abc_path):
            self.passed = False

        return self

    def compare_decl_output(self, _: str, __: Any, ___: int) -> bool:
        decl_path = self.default_work_dir_root / f"{Path(self.source_path).stem}.d.ets"
        try:
            with open(self.expected_path, encoding="utf-8") as expected_file:
                expected = expected_file.read()

            with open(decl_path, encoding="utf-8") as actual_file:
                actual = actual_file.read()

            expected_cleaned = self._strip_leading_comments(expected).rstrip()
            actual_cleaned = self._strip_leading_comments(actual).rstrip()
            return expected_cleaned == actual_cleaned
        except (FileNotFoundError, PermissionError) as error:
            _LOGGER.error(f"Declgen compare failed: {error}")
            return False

    def check_abc_generated(self, _: str, __: Any, ___: int) -> bool:
        return path.exists(self.generated_abc_from_decl_path)

    def _strip_leading_comments(self, text: str) -> str:
        pattern = r"""
            ^\s*(
                (/\*\*[^*]*\*+(?:[^/*][^*]*\*+)*/\s*) |
                (/\*.*?\*/\s*) |
                ((//[^\n]*\n)+)
            )+
        """

        cleaned = re.sub(pattern, '', text, flags=re.DOTALL | re.VERBOSE)
        return cleaned.strip()

    def _prepare_work_dirs(self) -> None:
        self.default_work_dir_root.mkdir(parents=True, exist_ok=True)

    def _ensure_parent_dirs(self, paths: List[Path]) -> None:
        for file_path in paths:
            file_path.parent.mkdir(parents=True, exist_ok=True)

    def _validate_decl_to_abc_generation(
        self, generated_decl_path: Path, generated_abc_path: Path
    ) -> bool:
        try:

            original_path = self.path
            self.path = str(generated_decl_path)

            passed, _, _ = self.run_es2panda(
                flags=[],
                test_abc=str(generated_abc_path),
                result_validator=self.check_abc_generated
            )

            self.path = original_path
            return passed
        except (FileNotFoundError, PermissionError) as error:
            _LOGGER.error(f"Failed to process temp decl file: {error}")
            return False
