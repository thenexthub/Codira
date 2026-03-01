#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

from pathlib import Path
from dataclasses import dataclass

from pytest import fixture, mark, param

from arkdb.compiler import AstParser, Compiler, CompilerArguments, ark_compile
from arkdb.compiler_verification.expression_verifier import ExpressionFileComparator, get_expression_verifier
from arkdb.mirrors.base import arkts_str
from arkdb.runnable_module import ScriptFile
from arkdb.walker import BreakpointWalkerType

pytestmark = mark.evaluate


@fixture
def arkts_file_extension() -> str:
    return "ets"


def replace_suffixes(ets_file_path: Path, new_suffixes: str):
    expression_file_name = ets_file_path.name
    expression_file_name = expression_file_name[: expression_file_name.find(".")] + new_suffixes
    return ets_file_path.parent / expression_file_name


def get_expression_path(ets_file_path: Path, lang_extension: str):
    return replace_suffixes(ets_file_path, f".patch.{lang_extension}")


def get_expected_path(ets_file_path: Path, lang_extension: str):
    return ets_file_path.parent / "expected" / replace_suffixes(ets_file_path, f".expected.{lang_extension}").name


@dataclass
class GetVerifierArgs:
    base: ScriptFile
    tmp_path: Path
    ark_compiler: Compiler
    expression_file_comparator: ExpressionFileComparator
    ast_parser: AstParser
    lang_extension: str


def get_verifier(args: GetVerifierArgs):
    expected: ScriptFile = ark_compile(
        get_expected_path(args.base.source_file, args.lang_extension),
        args.tmp_path,
        args.ark_compiler,
        arguments=CompilerArguments(ets_module=True, dump_dynamic_ast=True),
        ast_parser=args.ast_parser,
    )
    return get_expression_verifier(args.expression_file_comparator, args.base, expected, args.ast_parser)


@mark.parametrize(
    "base_ets_file",
    [
        param(file, id=str(file.relative_to(Path(__file__).parent)))
        for test_dir in (Path(__file__).parent / Path(__file__).stem).iterdir()
        if test_dir.is_dir()
        for file in test_dir.iterdir()
        if file.is_file() and file.name.endswith(".base.ets")
    ],
)
# CC-OFFNXT(G.FNM.03) temporary: reduce parameters list
async def test_evaluate_from_file(
    breakpoint_walker: BreakpointWalkerType,
    tmp_path: Path,
    ark_compiler: Compiler,
    expression_file_comparator: ExpressionFileComparator,
    ast_parser: AstParser,
    base_ets_file: Path,
    arkts_file_extension: str,
):
    # `base_ets_file` is passed as string for better log output.
    base_file: ScriptFile = ark_compile(
        base_ets_file,
        tmp_path,
        ark_compiler,
    )
    async with breakpoint_walker(base_file) as walker:
        stop_point = await anext(walker)

        verifier = get_verifier(
            GetVerifierArgs(
                base_file, tmp_path, ark_compiler, expression_file_comparator, ast_parser, arkts_file_extension
            )
        )
        result = await stop_point.evaluate(
            get_expression_path(base_file.source_file, arkts_file_extension),
            [walker.script_file.panda_file],
            verifier=verifier,
        )
        assert len(result) == 2
        # Must execute without exceptions.
        assert result[1] is None

        breakpoints = stop_point.hit_breakpoints()
        assert len(breakpoints) == 1
        result_type = breakpoints[0].label if breakpoints[0].label else "undefined"
        assert result[0].type_ == result_type

        # Run after evaluation point - execution must be finished correctly.
        stop_point = await anext(walker)
        breakpoints = stop_point.hit_breakpoints()
        assert len(breakpoints) == 1
        if breakpoints[0].label is not None:
            assert walker.capture.stdout[-1] == arkts_str(breakpoints[0].label)
