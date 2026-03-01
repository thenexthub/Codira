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

from dataclasses import replace
from typing import Any, List, Tuple

from pytest import fixture

from arkdb import runtime
from arkdb.expect import Expect
from arkdb.logs import RichLogger
from arkdb.mirrors import arkts_str
from arkdb.runnable_module import syntax
from arkdb.walker import BreakpointWalker, BreakpointWalkerType, PausedStep


async def test_class_ark_str(breakpoint_walker: BreakpointWalkerType, expect: Expect):

    code = """\
class A {
    public base: int = -1
}

class B extends A {
    public i: int
    public d: double
    public s: string
    public ss: String
    constructor() {
        this.i = 1
        this.d = 2
        this.s = 's'
        this.ss = 'ss'
    }
}

function main(): int {
    let a = new B()
    console.log(a)
    undefined           // #BP
    return 0
}
"""

    async with breakpoint_walker(code) as walker:
        cap = walker.capture
        async for paused in walker:
            if len(paused.hit_breakpoints()) == 0:
                continue
            vars = await paused.frame().scope().mirror_variables()
            a = arkts_str(vars["a"])
            await walker.log_layout("%s", a)

            assert len(cap.stdout) > 0
            with expect.warning():
                assert a == cap.stdout[-1]
            walker.stop()


@fixture
def ark_runtime_options(
    ark_runtime_default_options: runtime.Options,
) -> runtime.Options:
    return replace(
        ark_runtime_default_options,
    )


def _code_generator(objects: List[Tuple[str, Any]]) -> tuple[str, list[str]]:
    lines: List[str] = []
    expected: List[str] = []
    for i in range(len(objects)):
        t, v = objects[i]
        if isinstance(v, tuple):
            v_exp = v[1]
            v = v[0]
        else:
            v_exp = v
        var = f"x_{i}"
        lines += [
            f"let {var}: {t} = {v};" if t else f"let {var} = {v};",
            f'console.print("{var}: ");',
            f"console.log({var});",
            # >>> f"undefined;     // #BP{{log{i}}}",
        ]
        expected += [f"{var}: {arkts_str(v_exp)}"]

    gen_code = "\n    ".join(lines)
    code = f"""\
function main(): int {{
    console.println("begin");
    undefined;     // #BP{{begin}}
    {gen_code}
    undefined;     // #BP{{end}}
    return 0
}}\
"""
    return code, expected


async def _check(  # noqa: ASYNC910
    label: str | None,
    paused: PausedStep,
    walker: BreakpointWalker,
    log: RichLogger,
    expect: Expect,
    expected: list[str],
):
    match label:
        case "begin":
            assert paused.capture.stdout[-1] == "begin"
        case "end":
            vars = await paused.local_scope().mirror_variables()
            log.info("Locals: %s", vars)
            with expect.error():
                assert paused.capture.stdout == expected
            walker.stop()
        case None:
            assert False
        case _:
            # >>> assert lbl == f"log{len(printed_variables)}"
            # >>> printed_variables.append(paused.capture.stdout[-1])
            pass


async def test_arkts_primitive(
    breakpoint_walker: BreakpointWalkerType,
    log: RichLogger,
    expect: Expect,
) -> None:

    objects: List[Tuple[str, Any]] = [
        ("number", 1),
        ("Number", 1),
        ("byte", 1),
        ("Byte", 1),
        ("short", 1),
        ("Short", 1),
        ("int", 1),
        ("Int", 1),
        ("long", 1),
        ("Long", 1),
        ("float", 1),
        ("Float", 1),
        ("double", 1),
        ("Double", 1),
        ("char", ("c'A'", "A")),
        ("string", ("'AA'", "AA")),
        ("Char", ("c'A'", "A")),  # Char object will be output as String
        ("Char", ("new Char(c'A')", "A")),  # Char object will be output as String
        ("boolean", ("true", True)),
        ("Boolean", ("true", True)),
        ("", ("null", None)),
    ]

    code, expected = _code_generator(objects)
    log.info("Source code", rich=syntax(code))

    async with breakpoint_walker(code) as walker:
        async for paused in walker:
            if len(paused.hit_breakpoints()) == 0:
                continue
            label = paused.hit_breakpoints()[0].label
            await walker.log_layout("Breakpoint: '%s'", label)
            log.info("Capture:\n%s", "\n".join(paused.capture.stdout))

            await _check(label, paused, walker, log, expect, expected)
