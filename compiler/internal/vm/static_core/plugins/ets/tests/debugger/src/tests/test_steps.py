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

import logging
import warnings
from signal import SIGTERM

import trio
from cdp import runtime
from pytest import fixture

from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebuggerClient, DebugLocator
from arkdb.logs import RichLogger
from arkdb.runnable_module import ScriptFile
from arkdb.runtime import Runtime, RuntimeProcess
from arkdb.source_meta import parse_source_meta


class RuntimeExitStatusWarning(UserWarning):
    pass


@fixture
def script_file(
    code_compiler: StringCodeCompiler,
) -> ScriptFile:
    code = """\
class A {
    public i: int;
    public d: double;
    public s: string;
    public ss: String;
    constructor() {
        this.i = 1;
        this.d = 2;
        this.s = 's';
        this.ss = 'ss';
    }
}

function foo(arg: int): int {
    let c: int = 2;
    return arg * c
}

function main(): int {
    let a: int = 100;
    let b: int = a * foo(a);
    let obj = new A();
    console.log(a / b)
    return a / b;
}
"""
    return code_compiler.compile(code)


def check_exit_status(process: RuntimeProcess, log: RichLogger):
    status = process.returncode
    if status != 0:
        msg = f"Runtime exit status is {status}"
        log.warning(msg)
        warnings.warn(msg, RuntimeExitStatusWarning)


async def test_run(
    nursery: trio.Nursery,
    ark_runtime: Runtime,
    script_file: ScriptFile,
    log: RichLogger,
):
    script_file.log(log, logging.INFO)
    async with ark_runtime.run(nursery, module=script_file, debug=False) as process:
        await process.wait()
    check_exit_status(process, log)


async def test_debug(
    nursery: trio.Nursery,
    ark_runtime: Runtime,
    script_file: ScriptFile,
    log: RichLogger,
    debug_locator: DebugLocator,
):
    script_file.log(log, logging.INFO)
    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            paused = await client.run_if_waiting_for_debugger()
            log.info("%s", paused)
            await client.resume()
    check_exit_status(process, log)


async def _pause_and_get_vars(client: DebuggerClient, log: RichLogger, script_file: ScriptFile, line_number: int):
    paused = await client.continue_to_location(script_id=runtime.ScriptId("0"), line_number=line_number)
    script_file.log(log, highlight_lines=[paused.call_frames[0].location.line_number + 1])
    log.info("paused: %s", paused)
    # NOTE(dslynko, #22497): do not omit global objects when tests' applications are loaded into application context
    object_ids = [
        frame.scope_chain[0].object_.object_id
        for frame in paused.call_frames
        if len(frame.scope_chain) > 0 and frame.scope_chain[0].object_.object_id is not None
    ]
    # fmt: off
    props = [
        prop
        for props in [(await client.get_properties(obj_id))[0]
                      for obj_id in object_ids]
        for prop in props
    ]
    # fmt: on
    variables = {prop.name: prop.value.value if prop.value is not None else None for prop in props}
    log.info("Properties: \n%s", repr(props))
    log.info("All variables: %r", variables)
    return variables


async def test_code_compiler(
    nursery: trio.Nursery,
    ark_runtime: Runtime,
    code_compiler: StringCodeCompiler,
    debug_locator: DebugLocator,
    log: RichLogger,
):
    code = """
    function main(): int {
        let a: int = 100;
        let b: int = a + 10;
        console.log(a + 1)
        return a;
    }
    """

    script_file = code_compiler.compile(code)
    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:

            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()

            variables = await _pause_and_get_vars(client, log, script_file, 3)
            assert variables == {"a": 100}

            variables = await _pause_and_get_vars(client, log, script_file, 4)
            assert variables == {"a": 100, "b": 110}

            await client.resume()
    check_exit_status(process, log)


async def test_inline_breakpoints(
    nursery: trio.Nursery,
    ark_runtime: Runtime,
    code_compiler: StringCodeCompiler,
    debug_locator: DebugLocator,
    log: RichLogger,
):
    code = """\
    function main(): int {
        let a: int = 100;    // # BP {}
        let b: int = a + 10; //  #BP{1}
        console.log(a + 1)  // # BP
        return a;
    }"""

    script_file = code_compiler.compile(code)
    meta = parse_source_meta(code)  # noqa F841
    log.info("Parsed breakpoints %s", meta.breakpoints)
    script_file.log(log, highlight_lines=[b.line_number for b in meta.breakpoints])

    async def _check_breakpoints():
        await trio.lowlevel.checkpoint()
        for br in meta.breakpoints:
            with trio.fail_after(2):
                paused = await client.resume_and_wait_for_paused()
                paused_ln = paused.call_frames[0].location.line_number
                script_file.log(log, highlight_lines=[paused_ln])
                assert paused_ln == br.line_number

    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            _ = await client.run_if_waiting_for_debugger()
            for br in meta.breakpoints:
                await client.set_breakpoint_by_url(
                    url=script_file.source_file.name,
                    line_number=br.line_number,
                )

            await _check_breakpoints()
            process.terminate()
    assert process.returncode == -SIGTERM
