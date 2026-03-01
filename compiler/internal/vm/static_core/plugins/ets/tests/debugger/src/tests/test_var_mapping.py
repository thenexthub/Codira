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

from arkdb.debug import StopOnPausedType
from arkdb.mirrors import mirror_primitive as meta_primitive


def n(value):
    return meta_primitive("number", value)


def _ets_code() -> str:
    # BP in lambda: guard lambda frame locals (d1/a/b/c/d)
    return """\
function add1(a: int, b: int): int {
  let c = 1;
  let d = 2;
  let e = 3;
  let f = 4;
  return a + b;
}

function main(): int {
  (() => {
    let d1 = add1(1, 2);
    let a = 1;
    let b = 2;
    let c = 3;
    let d = 4;
    console.log(1)           // #BP
  })();
  return 0;
}\
"""


def _ets_code_add1_bp() -> str:
    # BP in add1: guard callee frame locals (a/b/c/d/e/f)
    # Keep main as well (do not delete); main still calls add1 from a lambda.
    return """\
function add1(a: int, b: int): int {
  let c = 1;
  let d = 2;
  let e = 3;
  let f = 4;
  console.log(1)           // #BP
  return a + b;
}

function main(): int {
  (() => {
    let d1 = add1(1, 2);
    let a = 1;
    let b = 2;
    let c = 3;
    let d = 4;
    console.log(2)
  })();
  return 0;
}\
"""


async def test_debugger_local_values_lambda_spill_window(
    run_and_stop_on_breakpoint: StopOnPausedType,
):
    code = _ets_code()
    async with run_and_stop_on_breakpoint(code) as paused:
        local_vars = await paused.frame().scope().mirror_variables()

        for name in ("d1", "a", "b", "c", "d"):
            assert name in local_vars, f"missing local var: {name}, got keys={list(local_vars.keys())}"

        assert local_vars["d1"] == n(3)  # add1(1,2) => 3
        assert local_vars["a"] == n(1)
        assert local_vars["b"] == n(2)
        assert local_vars["c"] == n(3)
        assert local_vars["d"] == n(4)


async def test_debugger_local_values_add1_spill_window(
    run_and_stop_on_breakpoint: StopOnPausedType,
):
    code = _ets_code_add1_bp()
    async with run_and_stop_on_breakpoint(code) as paused:
        local_vars = await paused.frame().scope().mirror_variables()

        # This breakpoint is inside add1(), so we expect to see add1 locals/params.
        for name in ("a", "b", "c", "d", "e", "f"):
            assert name in local_vars, f"missing local var: {name}, got keys={list(local_vars.keys())}"

        assert local_vars["a"] == n(1)
        assert local_vars["b"] == n(2)
        assert local_vars["c"] == n(1)
        assert local_vars["d"] == n(2)
        assert local_vars["e"] == n(3)
        assert local_vars["f"] == n(4)
