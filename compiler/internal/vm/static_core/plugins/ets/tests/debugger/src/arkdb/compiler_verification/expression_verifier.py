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

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Protocol

import yaml
from pytest import fixture

from arkdb.compiler import AstParser
from arkdb.compiler_verification.ast import AstComparator
from arkdb.compiler_verification.bytecode import BytecodeComparator
from arkdb.disassembler import ScriptFileDisassembler
from arkdb.logs import RichLogger
from arkdb.runnable_module import ScriptFile


class MetadataLoader(Protocol):
    def __call__(
        self,
        path: Path,
    ) -> dict[str, Any]:
        pass


@fixture
def metadata_loader() -> MetadataLoader:
    def load(path: Path) -> dict[str, Any]:
        metadata_pattern = re.compile(r"(?<=\/\*---)(.*?)(?=---\*\/)", flags=re.DOTALL)
        data = path.read_text()
        yaml_text = "\n".join(re.findall(metadata_pattern, data))
        metadata = yaml.safe_load(yaml_text)
        if metadata is None:
            metadata = {}
        return metadata

    return load


@dataclass
class ExpressionComparatorOptions:
    disable_ast_comparison: bool = False
    disable_bytecode_comparison: bool = False
    expected_imports_base: bool = False


class OptionsLoader(Protocol):
    def __call__(
        self,
        path: Path,
    ) -> ExpressionComparatorOptions:
        pass


@fixture
def eval_options_loader(
    metadata_loader: MetadataLoader,
) -> OptionsLoader:
    def load(path: Path) -> ExpressionComparatorOptions:
        all_options = metadata_loader(path)
        eval_options = all_options.get("evaluation", {})
        if not isinstance(eval_options, dict):
            raise RuntimeError()
        return ExpressionComparatorOptions(**eval_options)

    return load


class ExpressionFileComparator(Protocol):
    def __call__(
        self,
        base: ScriptFile,
        expression: ScriptFile,
        expected: ScriptFile,
    ):
        pass


@fixture
def expression_file_comparator(
    script_disassembler: ScriptFileDisassembler,
    eval_options_loader: OptionsLoader,
    log: RichLogger,
) -> ExpressionFileComparator:
    def verify(
        base: ScriptFile,
        expression: ScriptFile,
        expected: ScriptFile,
    ):
        options = eval_options_loader(base.source_file)
        expression = script_disassembler.disassemble(expression)
        expected = script_disassembler.disassemble(expected)

        ast_comparator = AstComparator(expression, expected, options.expected_imports_base)
        if not options.disable_ast_comparison:
            try:
                ast_comparator.compare()
            except Exception as e:
                log.warning("Expression AST mismatch: %s", e)
        if not options.disable_bytecode_comparison:
            bytecode_comparator = BytecodeComparator(
                expression,
                expected,
                base.source_file.stem,
                ast_comparator.get_eval_method_name(),
            )
            bytecode_comparator.compare()

    return verify


class ExpressionVerifier(Protocol):

    def __call__(
        self,
        expression: ScriptFile,
    ) -> None:
        pass

    # Read-only field
    @property
    def ast_parser(self) -> AstParser:
        pass


def get_expression_verifier(
    verifier: ExpressionFileComparator,
    base: ScriptFile,
    expected: ScriptFile,
    ast_parser: AstParser,
) -> ExpressionVerifier:
    class WithExpectedComparator:
        def __init__(self, parser: AstParser):
            self._ast_parser = parser

        # CC-OFFNXT(G.CLS.07) followed required interface
        def __call__(self, expression: ScriptFile):
            verifier(base, expression, expected)

        @property
        def ast_parser(self):
            return self._ast_parser

    return WithExpectedComparator(ast_parser)
