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

import logging
from functools import cached_property
from typing import Dict, Optional, List

from runner.options.decorator_value import value, _to_str, _to_path

from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.options_custom")


class CustomSuiteOptions:
    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "suite": self.suite_name,
            "test-root": self.test_root,
            "list-root": self.list_root,
            "generator": self.generator,
            "generator-options": self.generator_options,
        }

    @cached_property
    @value(yaml_path="custom.suite-name", cli_name="custom_suite_name")
    def suite_name(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="custom.test-root", cli_name="custom_test_root", cast_to_type=_to_path)
    def test_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="custom.list-root", cli_name="custom_list_root", cast_to_type=_to_path)
    def list_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="custom.generator", cli_name="custom_generator", cast_to_type=_to_path)
    def generator(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="custom.generator-options", cli_name="custom_generator_option")
    def generator_options(self) -> List[str]:
        return []

    def validate(self) -> None:
        if not self.__validate():
            Log.exception_and_raise(
                _LOGGER,
                "Invalid set of options for the custom suite.\n"
                "Expected min configuration: --custom-suite XXX --custom-test-root TTT --custom-list-root LLL\n"
                "With generator: --custom-suite XXX --custom-test-root TTT --custom-list-root LLL "
                "--custom-generator GGG --custom-generator-option CGO1 --custom-generator-option CG02")

    def get_command_line(self) -> str:
        generator_options = [f'--custom-generator-option="{arg}"' for arg in self.generator_options]
        options = [
            f'--custom-suite={self.suite_name}' if self.suite_name is not None else '',
            f'--custom-test-root={self.test_root}' if self.test_root is not None else '',
            f'--custom-list-root={self.list_root}' if self.list_root is not None else '',
            f'--custom-generator={self.generator}' if self.generator is not None else '',
            ' '.join(generator_options),
        ]
        return ' '.join(options)

    def __validate(self) -> bool:
        if self.suite_name is not None:
            if self.test_root is None or self.list_root is None:
                return False
            if self.generator_options:
                if self.generator is None:
                    return False
            return True
        return (self.test_root is None and
                self.list_root is None and
                self.generator is None and
                len(self.generator_options) == 0)
