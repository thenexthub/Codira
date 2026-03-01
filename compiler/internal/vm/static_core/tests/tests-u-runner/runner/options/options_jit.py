#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_jit_preheats, _to_str


class JitOptions:
    __DEFAULT_REPEATS = 0
    __DEFAULT_REPEATS_IF_EMPTY = 30
    __DEFAULT_THRESHOLD_IF_EMPTY = 20

    def __str__(self) -> str:
        return _to_str(self, 2)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "num_repeats": self.num_repeats,
            "compiler_threshold": self.compiler_threshold,
        }

    @cached_property
    @value(yaml_path="ark.jit.enable", cli_name="jit", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="ark.jit.num_repeats",
        cli_name="jit_preheat_repeats",
        cast_to_type=lambda x: _to_jit_preheats(
            cli_value=x, prop="num_repeats",
            default_if_empty=JitOptions.__DEFAULT_REPEATS_IF_EMPTY)
    )
    def num_repeats(self) -> int:
        return JitOptions.__DEFAULT_REPEATS

    @cached_property
    @value(
        yaml_path="ark.jit.compiler_threshold",
        cli_name="jit_preheat_repeats",
        cast_to_type=lambda x: _to_jit_preheats(
            cli_value=x, prop="compiler_threshold",
            default_if_empty=JitOptions.__DEFAULT_THRESHOLD_IF_EMPTY)
    )
    def compiler_threshold(self) -> Optional[int]:
        return None

    def get_command_line(self) -> str:
        options = '--jit' if self.enable else ''
        jit_options = [
            f'num_repeats={self.num_repeats}' if self.num_repeats != JitOptions.__DEFAULT_REPEATS else '',
            f'compiler_threshold={self.compiler_threshold}' if self.compiler_threshold is not None else '',
        ]
        jit_options = [option for option in jit_options if option]

        if jit_options:
            options += f' --jit-preheat-repeats="{",".join(jit_options)}"'
        return options
