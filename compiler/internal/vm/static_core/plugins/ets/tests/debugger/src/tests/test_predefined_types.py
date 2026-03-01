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

from arkdb.debug import StopOnPausedType
from arkdb.expect import Expect
from arkdb.mirrors import mirror_object as cls
from arkdb.mirrors import mirror_primitive as meta_primitive


def b(value: bool):
    return meta_primitive("boolean", value)


def n(value):
    return meta_primitive("number", value)


def ref_type(type_name: str):
    return cls(type_name, isValue=b(False))


def val_type(type_name: str):
    return cls(type_name, isValue=b(True))


def _code_generator(variables: list[tuple[str, Any, Any, Any]]) -> str:
    lines = []
    test_vars = {}
    ignore_vars = []
    i = 0
    for source_type, source_value, debug_type, debug_value in variables:
        var_name = f"x_{i:02d}_value"
        type_name = f"x_{i:02d}_type"
        lines.append(f"    let {var_name}: {source_type} = {source_value}")
        lines.append(f"    let {type_name}: Type = Type.of({var_name})")
        if debug_type is not None:
            test_vars[type_name] = debug_type
        else:
            ignore_vars.append(type_name)
        if debug_value is not None:
            test_vars[var_name] = debug_value
        else:
            ignore_vars.append(var_name)
        i += 1

    gen_code = "\n".join(lines)

    return f"""\
function main(): int {{
{gen_code}
    console.log(1)           // #BP
    return 0
}}\
"""


async def test_primitive_types(
    run_and_stop_on_breakpoint: StopOnPausedType,
    expect: Expect,
):

    variables = [
        ("boolean", "true", val_type("std.core.BooleanType"), b(True)),
        ("byte", 1, val_type("std.core.ByteType"), n(1)),
        ("short", 1, val_type("std.core.ShortType"), n(1)),
        ("int", 1, val_type("std.core.IntType"), n(1)),
        ("long", 1, val_type("std.core.LongType"), n(1)),
        ("float", 1, val_type("std.core.FloatType"), n(1)),
        ("double", 1, val_type("std.core.DoubleType"), n(1)),
        ("number", 1, val_type("std.core.DoubleType"), n(1)),
    ]
    code = _code_generator(variables)
    async with run_and_stop_on_breakpoint(code) as paused:
        local_vars = await paused.frame().scope().mirror_variables()
        for i, (source_type, source_value, debug_type, debug_value) in enumerate(variables):
            var_name = f"x_{i:02d}_value"
            with expect.error():
                assert debug_value == local_vars[var_name]
