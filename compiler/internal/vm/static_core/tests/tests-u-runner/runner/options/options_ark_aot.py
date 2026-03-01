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
from typing import Dict, List

from runner.options.decorator_value import value, _to_bool, _to_int, _to_str


class ArkAotOptions:
    __DEFAULT_TIMEOUT = 600

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "timeout": self.timeout,
            "aot-args": self.aot_args,
        }

    @cached_property
    @value(yaml_path="ark_aot.enable", cli_name="aot", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="ark_aot.timeout", cli_name="paoc_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return ArkAotOptions.__DEFAULT_TIMEOUT

    @cached_property
    @value(yaml_path="ark_aot.aot-args", cli_name="aot_args")
    def aot_args(self) -> List[str]:
        return []

    def get_command_line(self) -> str:
        _aot_args = [f'--aot-args="{arg}"' for arg in self.aot_args]
        options = [
            '--aot' if self.enable else '',
            f'--paoc-timeout={self.timeout}' if self.timeout != ArkAotOptions.__DEFAULT_TIMEOUT else '',
            ' '.join(_aot_args)
        ]
        return ' '.join(options)
