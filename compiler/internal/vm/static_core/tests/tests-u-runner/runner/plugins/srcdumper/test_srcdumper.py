#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import json
import os
from typing import List, Any
import traceback

from runner.test_file_based import TestFileBased
from runner.enum_types.params import TestEnv
from runner.enum_types.fail_kind import FailKind
from runner.plugins.srcdumper.util_srcdumper import AstComparator, PathConverter
from runner.enum_types.params import TestReport
from runner.parser import START_ERRORS


class TestSRCDumper(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)
        self.util = self.test_env.util
        self.passed = True
        self.original_ast: str = ""
        self.dumped_ast: str = ""

        self.path_conv = PathConverter(test_id, test_env.work_dir.root.as_posix())


    def do_run(self) -> TestFileBased:
        if not self.expects_error():
            self.path_conv.init_artefact_dirs()
            self.compile_original_with_dump_src()
            self.compile_original_with_dump_ast()
            self.compile_dumped_with_dump_ast()
            self.compare_ast()

        return self


    def expects_error(self) -> bool:
        with os.fdopen(os.open(self.path, os.O_RDONLY, mode=511), "rb") as file:
            return START_ERRORS.search(file.read()) is not None


    def compare_ast(self) -> None:
        if not self.passed:
            return

        try:
            loading_json = self.original_ast
            original_ast = json.loads(loading_json)
            loading_json = self.dumped_ast
            dumped_ast = json.loads(loading_json)

            self.passed, self.report, self.fail_kind = AstComparator(original_ast, dumped_ast).run()

        except json.JSONDecodeError:
            self.passed = False
            self.report = TestReport(str(loading_json), traceback.format_exc(), -1)
            self.fail_kind = FailKind.INVALID_JSON


    def compile_original_with_dump_src(self) -> None:
        if not self.passed:
            return

        es2panda_flags = self.flags + ['--dump-ets-src-after-phases', 'plugins-after-parse']

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc='',
            result_validator=self.es2panda_result_validator
        )

        with os.fdopen(
            os.open(
                self.path_conv.dumped_src_path(),
                os.O_WRONLY | os.O_CREAT | os.O_TRUNC, mode=511),
                "w",
                encoding="utf-8"
        ) as file:
            file.writelines(self.report.output.splitlines(keepends=True)[1:])


    def compile_original_with_dump_ast(self) -> None:
        if not self.passed:
            return

        es2panda_flags = self.flags + ['--dump-after-phases', 'plugins-after-parse']

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc='',
            result_validator=self.es2panda_result_validator,
            no_log=True
        )

        self.original_ast = self.report.output[self.report.output.find("\n") + 1:]


    def compile_dumped_with_dump_ast(self) -> None:
        if not self.passed:
            return

        old_path = self.path
        es2panda_flags = self.flags + ['--dump-after-phases', 'plugins-after-parse']
        self.path = self.path_conv.dumped_src_path()

        self.passed, self.report, self.fail_kind = self.run_es2panda(
            flags=es2panda_flags,
            test_abc='',
            result_validator=self.es2panda_result_validator,
            no_log=True
        )

        if not self.passed:
            self.fail_kind = FailKind.SRC_DUMPER_FAIL

        self.path = old_path
        self.dumped_ast = self.report.output[self.report.output.find("\n") + 1:]


    def es2panda_result_validator(self, _: str, __: Any, return_code: int) -> bool:
        return not return_code
