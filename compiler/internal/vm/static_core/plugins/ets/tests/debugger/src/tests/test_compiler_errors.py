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

import pytest
import trio

from arkdb import compiler


async def test_compile_type_error(
    code_compiler: compiler.StringCodeCompiler,
):
    code = """\
function main(): int {
    error
}
"""
    with pytest.raises(compiler.CompileError, match=r"Semantic error ESE0143: Unresolved reference error"):
        code_compiler.compile(code)
    await trio.lowlevel.checkpoint()
