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

from signal import SIGTERM
from typing import Any, Dict, List, TypedDict

import trio
from pytest import mark

from arkdb import layouts
from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebuggerClient, DebugLocator
from arkdb.debug_types import paused_locator
from arkdb.logs import RichLogger
from arkdb.runtime import Runtime
from arkdb.source_meta import Breakpoint, parse_source_meta


class Scope(TypedDict):
    variables: Dict[str, Any]


class Frame(TypedDict):
    id: str
    function_name: str
    scopes: List[Scope]


class Data(TypedDict):
    label: str
    frames: List[Frame]


LOG_LINE_RANGE = 5

TEST_BREAKPOINTS_JUMPS_AND_CHECK_PROPERTIES_CODE = """\
class A {
    public i: int;
    public d: double;
    public s: string;
    public ss: String;
    constructor() {
        this.i = 1;
        this.d = 2;                 // #BP{3}
        this.s = 's';
        this.ss = 'ss';             // #BP{4}
    }
}

function foo(arg: int): int {
    let c: int = 2;
    let result = arg * c;
    return result;                  // #BP{1}
}

const d: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];

function main(): int {
    const dd: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];
    let a: int = 100;               // #BP{0}
    let b: int = a * foo(a);
    let obj = new A();              // #BP{2}
    console.log(a / b)
    let result = a / b;             // #BP{5}
    return result                   // #BP{6}
}\
"""


@mark.timeout(seconds=60)
async def test_breakpoints_jumps_and_check_properties(
    log: RichLogger,
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
):
    code = TEST_BREAKPOINTS_JUMPS_AND_CHECK_PROPERTIES_CODE

    script_file = code_compiler.compile(source_code=code)
    meta = parse_source_meta(code)
    breakpoints = sorted(
        meta.breakpoints,
        key=lambda br: (br.label if br.label is not None else ""),
    )
    script_file.log(log, highlight_lines=[b.line_number for b in breakpoints])
    log.info("Parsed breakpoints %s", breakpoints)

    async def _check():
        await trio.lowlevel.checkpoint()
        for br in breakpoints:
            with trio.fail_after(2):
                paused = await client.resume_and_wait_for_paused()
                log.info(f"{paused}")
                log.info(
                    script_file.source_file,
                    extra=dict(
                        rich=await layouts.paused_layout(
                            paused=paused_locator(paused=paused, client=client),
                            url=script_file.source_file,
                        ),
                    ),
                )
                assert paused.call_frames[0].location.line_number == br.line_number

    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            _ = await client.run_if_waiting_for_debugger()
            for br in breakpoints:
                (bp_id, _) = await client.set_breakpoint_by_url(
                    url=script_file.source_file.name,
                    line_number=br.line_number,
                )
                assert str(bp_id) == br.label, "Breakpoints order"

            await _check()
        process.terminate()
    assert process.returncode == -SIGTERM


TEST_ARRAY_CODE = """\
const gd: Number[] = [10, -5];
function main(): int {
    const n1: int = 1;
    const n2: Number = 2;
    const nn = [n1, n2];
    console.log("array");      // #BP{0}
    return 0;
}\
"""


async def test_array(
    log: RichLogger,
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
):
    code = TEST_ARRAY_CODE
    script_file = code_compiler.compile(source_code=code)
    meta = parse_source_meta(code)
    breakpoints = sorted(
        meta.breakpoints,
        key=lambda br: (br.label if br.label is not None else ""),
    )
    script_file.log(log, highlight_lines=[b.line_number for b in breakpoints])
    log.info("Parsed, debug_info=False, breakpoints %s", breakpoints)

    async def _check(client: DebuggerClient, brekpoint: Breakpoint):
        with trio.fail_after(2):
            paused = await client.resume_and_wait_for_paused()
            paused_ln = paused.call_frames[0].location.line_number
            log.info(
                script_file.source_file,
                rich=await layouts.paused_layout(
                    paused=paused_locator(paused=paused, client=client),
                    url=script_file.source_file,
                ),
            )
            assert paused_ln == brekpoint.line_number

    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            _ = await client.run_if_waiting_for_debugger()
            for br in breakpoints:
                (bp_id, _) = await client.set_breakpoint_by_url(
                    url=script_file.source_file.name,
                    line_number=br.line_number,
                )
                assert str(bp_id) == br.label, "Breakpoints order"
            for br in breakpoints:
                await _check(client, br)
        process.terminate()
    assert process.returncode == -SIGTERM
