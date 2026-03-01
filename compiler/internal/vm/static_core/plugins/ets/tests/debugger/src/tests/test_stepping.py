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

from typing import Final

from pytest import mark

from arkdb.debug import StopOnPausedType
from arkdb.logs import RichLogger


async def test_for_steps_in_functions(
    run_and_stop_on_breakpoint: StopOnPausedType,
) -> None:
    line_number_after_step_into: Final[int] = 6
    line_number_after_step_over: Final[int] = 7
    line_number_after_step_out: Final[int] = 12
    code = """\
function baz(a: int): int {
    a = a + 1;
    return a;
}

function bar(a: int): int {
    a = baz(a);
    a = a + 1;
    return a;
}

function foo(a: int): int {
    a = bar(a);             // #BP
    a = a + 1;
    return a;
}

function main(): int {
    let a: int = 1;
    a = foo(a);
    return 0;
}
"""
    async with run_and_stop_on_breakpoint(code) as paused_step:
        client = paused_step.client

        paused = await client.step_into()
        assert paused.call_frames[0].location.line_number == line_number_after_step_into
        paused = await client.step_over()
        assert paused.call_frames[0].location.line_number == line_number_after_step_over
        paused = await client.step_out()
        assert paused.call_frames[0].location.line_number == line_number_after_step_out


@mark.parametrize(
    "start,end_condition,update_statement",
    [
        (0, "i < 3", "i++"),
        (0, "i < 3", "i += 1"),
        (3, "i > 0", "i--"),
        (3, "i > 0", "i -= 1"),
        (1, "i < 8", "i *= 2"),
        (8, "i > 1", "i /= 2"),
    ],
)
async def test_for_loop_iterator_update(
    run_and_stop_on_breakpoint: StopOnPausedType,
    log: RichLogger,
    start: int,
    end_condition: str,
    update_statement: str,
) -> None:
    total_iterations: Final[int] = 3
    for_loop_line_number: Final[int] = 2
    code = f"""\
function main(): int {{
    let acc: int = 0; // #BP
    for (let i = {start}; {end_condition}; {update_statement}) {{
        acc += i;
    }}
    console.log(acc);
    return 0;
}}\
"""
    async with run_and_stop_on_breakpoint(code) as paused_step:
        client = paused_step.client

        for i in range(total_iterations):
            log.info(f"Iteration #{i}")
            paused = await client.step_into()
            assert paused.call_frames[0].location.line_number == for_loop_line_number
            paused = await client.step_into()
            assert paused.call_frames[0].location.line_number == for_loop_line_number + 1
