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
from typing import Dict, Union

from runner.options.decorator_value import value, _to_str, _to_bool, _to_int


class ETSOptions:
    __DEFAULT_COMPARE_FILES_ITERATIONS = 2

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, Union[bool, int]]:
        return {
            "force-generate": self.force_generate,
            "compare-files": self.compare_files,
            "compare-files-iterations": self.compare_files_iterations,
        }

    @cached_property
    @value(yaml_path="ets.force-generate", cli_name="is_force_generate", cast_to_type=_to_bool)
    def force_generate(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="ets.compare-files", cli_name="compare_files", cast_to_type=_to_bool)
    def compare_files(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="ets.compare-files-iterations", cli_name="compare_files_iterations", cast_to_type=_to_int)
    def compare_files_iterations(self) -> int:
        return self.__DEFAULT_COMPARE_FILES_ITERATIONS

    def get_command_line(self) -> str:
        options = [
            '--force-generate' if self.force_generate else '',
            '---compare-files' if self.compare_files else '',
            f'--compare-files-iterations={self.compare_files_iterations}',
        ]
        return ' '.join(options)
