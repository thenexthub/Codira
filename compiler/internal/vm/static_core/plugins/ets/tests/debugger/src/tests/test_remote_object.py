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

from typing import Any

import rich
import rich.pretty
from pytest import mark

from arkdb.debug import StopOnPausedType
from arkdb.logs import RichLogger
from arkdb.mirrors import mirror_primitive


@mark.parametrize(
    "line,meta",
    [
        ("Number = 1", mirror_primitive("number", 1)),
        ("int = 1", 1),
        ("float = 1.0f", 1),
    ],
)
async def test_const_var(
    log: RichLogger,
    run_and_stop_on_breakpoint: StopOnPausedType,
    line: str,
    meta: Any,
) -> None:
    code = f"""\
function main(): int {{
    const x: {line};
    console.log(x);           // #BP
    return 0;
}}\
"""
    async with run_and_stop_on_breakpoint(code) as paused:
        scope_vars = await paused.frame().scope().mirror_variables()
        log.info("Variables: %s", rich.pretty.pretty_repr(scope_vars))
        assert scope_vars["x"] == meta


async def test_let_vars(
    log: RichLogger,
    run_and_stop_on_breakpoint: StopOnPausedType,
) -> None:
    code = """\
    function foo(n: Number, i: int, f: float, b: boolean, s: String): int {
        console.log(s + " " + n + " " + i + " " + f + " " + b); // #BP
        return 1
    }
    function main(): int {
        let n: Number = 1.3;
        let i0: int = 1;
        let i: int = 30 + i0;
        let f: float = 1.2f;
        let b: boolean = true;
        let s: String = "str";
        foo(n, i, f, b, s);
        return i;
    }\
    """
    value = mirror_primitive
    async with run_and_stop_on_breakpoint(code) as paused:
        scope_vars = await paused.frame().scope().mirror_variables()
        expect = {
            "b": value("boolean", True),
            "f": value("number", 1.2),
            "i": value("number", 31),
            "n": value("number", 1.3),
            "s": value("string", "str"),
        }
        log.info("Variables: %s", rich.pretty.pretty_repr(scope_vars))
        assert scope_vars == expect
