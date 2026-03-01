#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import difflib
import re
from typing import Iterable

from arkdb.compiler_verification import ExpressionEvaluationNames
from arkdb.runnable_module import ScriptFile


class BytecodeComparisonError(AssertionError):
    pass


class BytecodeComparator:
    EXPECTED_FUNC_DECL_PATTERN = (
        r"^\.function [a-zA-Z_\d\$\-\.]+ "
        f"{ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME}"
        r"\."
        f"{ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME}"
        r"\."
        f"{ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME}"
        r"\(\)"
    )

    def __init__(self, expression: ScriptFile, expected: ScriptFile, base_module_name: str, eval_method_name: str):
        self.expression = expression
        self.expected = expected
        self.base_module_name = base_module_name
        self.eval_method_name = eval_method_name
        self.patch_func_pattern = (
            r"^\.function [a-zA-Z_\d\$\-\.]+ "
            f"{self.eval_method_name}"
            r"\."
            f"{self.eval_method_name}"
            r"\."
            f"{self.eval_method_name}"
            r"\(\)"
        )

    def compare(self):
        """
        Compares two bytecode files according to disasm output.
        """
        with self.expression.disasm_file.open() as expression_file:
            with self.expected.disasm_file.open() as expected_file:
                error_report = ""

                expected_func_body = _fetch_bytecode_function(
                    expected_file.readlines(),
                    BytecodeComparator.EXPECTED_FUNC_DECL_PATTERN,
                    [f"{ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME}.", f"{self.base_module_name}."],
                )
                if not expected_func_body:
                    error_report = "Expected bytecode function was not found or empty."
                else:
                    # Restore fully qualified function name after prefixes removal.
                    method_name = ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME
                    expected_func_body[0] = expected_func_body[0].replace(
                        f" {method_name}()",
                        f" {method_name}.{method_name}.{method_name}()",
                    )

                patch_func_body = _fetch_bytecode_function(
                    expression_file.readlines(),
                    self.patch_func_pattern,
                    [f"{self.base_module_name}."],
                )
                if not patch_func_body:
                    error_report += "\tEvaluation patch bytecode function was not found"

                if error_report == "" and len(expected_func_body) != len(patch_func_body):
                    error_report = "Expected and patch bytecode differ in count"

                if error_report != "":
                    raise BytecodeComparisonError(error_report)

                patch_func_body[0] = patch_func_body[0].replace(
                    self.eval_method_name,
                    ExpressionEvaluationNames.EVAL_PATCH_FUNCTION_NAME,
                )
                diff = difflib.ndiff(expected_func_body, patch_func_body)
                error_list = [x for x in diff if x[0] not in ExpressionEvaluationNames.NON_DIFF_MARKS]
                if error_list:
                    raise BytecodeComparisonError("Bytecode comparison failed:\n" + "\n".join(error_list))


def _fetch_bytecode_function(
    bytecode: list[str],
    function_decl_pattern: str,
    prefixes: Iterable[str],
) -> list[str]:
    func_body: list[str] = []
    start_idx: int | None = None

    for idx, line in enumerate(bytecode):
        if re.match(function_decl_pattern, line):
            start_idx = idx
            break

    if start_idx is not None:
        for line in bytecode[start_idx:]:
            func_body.append(_remove_prefix(line, prefixes))
            if line == "}\n":
                return func_body

    return []


def _remove_prefix(line: str, prefixes: Iterable[str]) -> str:
    for p in prefixes:
        line = line.replace(p, "")
    return line
