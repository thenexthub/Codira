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
from typing import Dict, Optional, List

from runner.options.decorator_value import value, _to_int, _to_str, _to_path, _to_bool


class Es2PandaOptions:
    __DEFAULT_TIMEOUT = 60
    __DEFAULT_OPT_LEVEL = 2

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "timeout": self.timeout,
            "opt-level": self.opt_level,
            "debug-info": self.debug_info,
            "custom-path": self.custom_path,
            "arktsconfig": self.arktsconfig,
            "es2panda-args": self.es2panda_args,
            "system": self.system
        }

    @cached_property
    @value(yaml_path="es2panda.timeout", cli_name="es2panda_timeout", cast_to_type=_to_int)
    def timeout(self) -> int:
        return Es2PandaOptions.__DEFAULT_TIMEOUT

    @cached_property
    @value(yaml_path="es2panda.opt-level", cli_name="es2panda_opt_level", cast_to_type=_to_int)
    def opt_level(self) -> int:
        return Es2PandaOptions.__DEFAULT_OPT_LEVEL

    @cached_property
    @value(yaml_path="es2panda.debug-info", cli_name="es2panda_debug_info", cast_to_type=_to_bool)
    def debug_info(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="es2panda.custom-path", cli_name="custom_es2panda_path", cast_to_type=_to_path)
    def custom_path(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="es2panda.arktsconfig", cli_name="arktsconfig", cast_to_type=_to_path)
    def arktsconfig(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="es2panda.es2panda-args", cli_name="es2panda_args")
    def es2panda_args(self) -> List[str]:
        return []

    @cached_property
    @value(yaml_path="es2panda.system", cli_name="system", cast_to_type=_to_bool)
    def system(self) -> bool:
        return False

    def is_fullastverifier(self) -> bool:
        return '--ast-verifier:phases=each' in self.es2panda_args

    def is_simultaneous(self) -> bool:
        return "--simultaneous=true" in self.es2panda_args

    def get_command_line(self) -> str:
        options = [
            f'--es2panda-timeout={self.timeout}' if self.timeout != Es2PandaOptions.__DEFAULT_TIMEOUT else '',
            f'--es2panda-opt-level={self.opt_level}' if self.opt_level != Es2PandaOptions.__DEFAULT_OPT_LEVEL else '',
            f'--custom-es2panda="{self.custom_path}"' if self.custom_path is not None else '',
            f'--arktsconfig="{self.arktsconfig}"' if self.arktsconfig is not None else '',
            '--es2panda-debug-info' if self.debug_info else '',
        ]
        return ' '.join(options)
