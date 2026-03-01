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
#

from arkdb.compiler import EvaluateCompileExpressionArgs
from arkdb.debug_types import compile_expression
from arkdb.walker import BreakpointWalkerType
import cdp.debugger as debugger


async def test_evaluate_on_call_frame(
    breakpoint_walker: BreakpointWalkerType,
) -> None:
    code = """\
function foo() {
    let x: int = 10;
    let y: int = 20;
    x += 1;
    y += 1; // #BP{1}
    return;
}
function main(): int {
    let x: int = 1;
    let y: int = 2;
    x += 1;
    y += 1;
    foo();
    return 0;
}
"""
    async with breakpoint_walker(code) as walker:
        paused_step = await anext(walker)
        bps = paused_step.hit_breakpoints()

        compiled_expression_bytecode = compile_expression(
            paused_step.client.code_compiler,
            EvaluateCompileExpressionArgs(
                ets_expression="x+y",
                eval_panda_files=[walker.script_file.panda_file],
                eval_source=walker.script_file.source_file,
                eval_line=bps[0].line_number + 1,
            ),
        )
        result, _ = await paused_step.client.evaluate_on_call_frame(
            debugger.CallFrameId("0"),
            compiled_expression_bytecode,
            return_by_value=True,
            silent=True,
            object_group="console",
            include_command_line_api=True,
        )
        assert result.value == 31
