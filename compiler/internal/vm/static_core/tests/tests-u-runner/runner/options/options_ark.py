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
from typing import Dict, List, Optional

from runner.options.decorator_value import value, _to_int, _to_str
from runner.options.options_jit import JitOptions


class ArkOptions:
    __DEFAULT_TIMEOUT = 10

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "timeout": self.timeout,
            "interpreter-type": self.interpreter_type,
            "jit": self.jit.to_dict(),
            "ark-args": self.ark_args
        }

    @cached_property
    @value(yaml_path="ark.timeout", cli_name="timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return ArkOptions.__DEFAULT_TIMEOUT

    @cached_property
    @value(yaml_path="ark.interpreter-type", cli_name="interpreter_type")
    def interpreter_type(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="ark.ark-args", cli_name="ark_args")
    def ark_args(self) -> List[str]:
        return []

    jit = JitOptions()

    def get_command_line(self) -> str:
        _ark_args = [f'--ark-args="{arg}"' for arg in self.ark_args]
        options = [
            f'--timeout={self.timeout}' if self.timeout != ArkOptions.__DEFAULT_TIMEOUT else '',
            f'--interpreter-type={self.interpreter_type}' if self.interpreter_type is not None else '',
            ' '.join(_ark_args),
            self.jit.get_command_line()
        ]
        return ' '.join(options)
