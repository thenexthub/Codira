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

import warnings
from typing import Any, OrderedDict

import pytest

from arkdb.debug_types import mirror_undefined
from arkdb.expect import Expect
from arkdb.walker import BreakpointWalkerType

code = """\
class A {
    public i: int
    public d: double
    public s: string
    public ss: String
    constructor() {
        this.i = 1
        this.d = 2
        this.s = 's'
        this.ss = 'ss'
        undefined                       // #BP{ctor}
        console.log("<ctor>", this)
    }

    foo1(n: int): int {
        let func = (a: int): int => {
            let x = a * n
            undefined                   // #BP{lambda}
            console.log("lambda", x, n, this)
            return x
        }
        let x = n * func(2)
        undefined                       // #BP{foo1}
        console.log("foo1", n, x)
        return x
    }

    foo(n: int): int {
        let x = n + this.foo1(n)
        undefined                       // #BP{foo}
        console.log("foo", n, x)
        return x
    }
}

function main(): int {
    let n: int = 2
    let obj: A = new A()                // #BP{unknown}
    let x = obj.foo(n)
    let func = (a: int): int => {       // #BP{lambda_decl}
        let z = a * x
        undefined                       // #BP{main_lambda}
        console.log("lambda", z, a, x)
        return z
    }
    let z = func(10)
    undefined                           // #BP{main}
    console.log("main", n, obj, x)
    return x                            // #BP{end}
}\
"""


async def test_this_in_lambda_and_class(
    breakpoint_walker: BreakpointWalkerType,
    expect: Expect,
):

    def _check(label: str, this: Any, scope_vars: OrderedDict[str, Any]):
        match label:
            case "ctor" | "foo1" | "foo" | "lambda":
                with expect.error():
                    assert this is not None
                    # Fully-qualified name of class
                    assert type(this).__name__ == "test_string.A"
            case "main":
                with expect.error():
                    assert this == mirror_undefined()
            case "end":
                walker.stop()
            case "main_lambda":
                with expect.error():
                    assert this == mirror_undefined()
            case "lambda_decl":
                # NOTE(dslynko, #19869): check debug-info is correct for lambdas
                assert all(
                    scope_vars.get(x) is not None
                    for x in (
                        "n",
                        "obj",
                        "x",
                    )
                )
                assert scope_vars.get("func") is None
            case _:
                warnings.warn(f"Unknown breakpoint on line {paused.frame().data.location.line_number}")

    async with breakpoint_walker(code) as walker:
        unk_bp = next(filter(lambda b: b.label == "unknown", walker.meta.breakpoints))
        with pytest.warns(UserWarning, match=f"Unknown breakpoint on line {unk_bp.line_number}"):
            async for paused in walker:
                bp = paused.hit_breakpoints().pop()
                label = str(bp.label)
                await walker.log_layout("%s %s", label, bp)
                this_obj = paused.frame().this()
                this = await this_obj.mirror_value() if this_obj is not None else None
                scope_vars = await paused.frame().scope().mirror_variables()
                _check(label, this, scope_vars)
