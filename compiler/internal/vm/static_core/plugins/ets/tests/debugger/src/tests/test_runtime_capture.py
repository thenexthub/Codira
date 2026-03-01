#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

from re import search

from arkdb.debug import CompileAndRunType
from arkdb.expect import Expect
from arkdb.logs import RichLogger
from arkdb.mirrors import arkts_str, arkts_str_list
from arkdb.walker import BreakpointWalkerType, PausedStep


async def test_assert_in_capture(
    log: RichLogger,
    compile_and_run: CompileAndRunType,
    expect: Expect,
) -> None:
    code = """\
function main(): int {
    console.log("START")
    arktest.assertTrue(false)
    console.log("TEST_END")
    return 0
}\
"""

    async with compile_and_run(code) as process:
        await process.wait()
    runtime_capture = process.capture
    log.info("STDOUT %s", runtime_capture.stdout)
    log.info("STDERR %s", runtime_capture.stderr)
    with expect.error():
        assert arkts_str_list(["START"]) == runtime_capture.stdout
        assertion_message = "expected true but was false"
        error_log = "\n".join(runtime_capture.stderr)
        assert search(assertion_message, error_log)
        assert search(r"Unhandled exception: std.core.AssertionError", error_log)


async def test_walker_capture_steps(breakpoint_walker: BreakpointWalkerType, expect: Expect):
    code = """\
function main(): int {
console.log("TEST_START")
undefined                           // #BP{TEST_START}
for (let i = 0; i < 20; ++i) {
    console.log(i)
    undefined                       // #BP{LOOP}
}
console.log("TEST_END")
undefined                           // #BP{TEST_END}
return 0
}\
"""
    counter = -1
    async with breakpoint_walker(code) as walker:
        async for paused in walker:
            bp = paused.hit_breakpoints().pop()
            counter, stop = _check(bp.label, expect, paused, counter)
            if stop:
                walker.stop()


def _check(bp_label: str | None, expect: Expect, paused: PausedStep, counter: int) -> tuple[int, bool]:
    match bp_label:
        case "TEST_START":
            counter = 0
            with expect.error():
                assert arkts_str(bp_label) == paused.capture.stdout[-1]
        case "LOOP":
            with expect.error():
                assert [arkts_str(counter)] == paused.capture.stdout
            counter += 1
        case "TEST_END":
            with expect.error():
                assert arkts_str_list([bp_label]) == paused.capture.stdout
            return counter, True
    return counter, False
