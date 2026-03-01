#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from pathlib import Path
from typing import Any
import yaml
from jinja2 import Environment
from jinja2.ext import Extension
from jinja2.nodes import Assign
from jinja2.parser import Parser
from runner.utils import read_file
from runner.plugins.ets.utils.exceptions import InvalidFileFormatException


class YamlExtension(Extension):

    def __init__(self, environment: Environment):
        super().__init__(environment)
        self.tags.add("yaml")

    def parse(self, parser: Parser) -> Assign:
        lineno = next(parser.stream).lineno
        template = parser.parse_expression()
        parser.stream.expect("name:as")
        assign_node = Assign(lineno = lineno)
        assign_node.target = parser.parse_assign_target(name_only=True)
        assign_node.node = self.call_method("call_read_yaml", [template], lineno=lineno)
        return assign_node

    def call_read_yaml(self, name: str) -> Any:  # type: ignore[explicit-any, unused-ignore]
        yaml_dir = self.environment.getattr(self.environment, 'current_test_path_dir')
        yaml_file = Path(yaml_dir, name)
        try:
            text = read_file(yaml_file)
            result = yaml.safe_load(text)
        except Exception as exc:
            raise InvalidFileFormatException(message=f"Could not load YAML in test params: {str(exc)}",
                                             filepath=str(yaml_file)) from exc
        return result
