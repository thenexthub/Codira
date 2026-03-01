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

from runner.options.decorator_value import value, _to_bool, _to_int, _to_str, _to_path


class VerifierOptions:
    __DEFAULT_TIMEOUT = 60

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "enable": self.enable,
            "timeout": self.timeout,
            "config": self.config,
        }

    @cached_property
    @value(yaml_path="verifier.enable", cli_name="enable_verifier", cast_to_type=_to_bool)
    def enable(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="verifier.timeout", cli_name="verifier_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return VerifierOptions.__DEFAULT_TIMEOUT

    @cached_property
    @value(yaml_path="verifier.config", cli_name="verifier_config", cast_to_type=_to_path)
    def config(self) -> Optional[str]:
        return None

    def get_command_line(self) -> str:
        options = [
            '--disable-verifier' if not self.enable else '',
            f'--verifier-timeout={self.timeout}' if self.timeout != VerifierOptions.__DEFAULT_TIMEOUT else '',
            f'--verifier-config="{self.config}"' if self.config is not None else ''
        ]
        return ' '.join(options)
