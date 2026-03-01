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

import logging
from typing import Dict, Optional, List, Union, Any

import yaml
from runner import utils
from runner.logger import Log

_LOGGER = logging.getLogger("runner.options.yaml_document")


class YamlDocument:

    def __init__(self) -> None:
        super().__init__()
        self._document: Optional[Dict[str, Any]] = None
        self._warnings: List[str] = []

    @staticmethod
    def load(config_path: str) -> Dict[str, Any]:
        with open(config_path, "r", encoding="utf-8") as stream:
            try:
                data: Dict[str, Any] = yaml.safe_load(stream)
                return data
            except yaml.YAMLError as exc:
                Log.exception_and_raise(_LOGGER, str(exc), yaml.YAMLError)

    @staticmethod
    def save(config_path: str, data: Dict[str, Any]) -> None:
        data_to_save = yaml.dump(data, indent=4)
        utils.write_2_file(config_path, data_to_save)

    def load_configs(self, config_paths: Optional[List[str]]) -> None:
        if config_paths is None:
            return
        for config_path in config_paths:
            self.merge(config_path, self.load(config_path))

    def document(self) -> Optional[Dict[str, Any]]:
        return self._document

    def warnings(self) -> List[str]:
        return self._warnings

    # We use Log.exception_and_raise which throws exception. no need in return
    # pylint: disable=inconsistent-return-statements
    def get_value_by_path(self, yaml_path: str) -> Optional[Union[int, bool, str, List[str]]]:
        yaml_parts = yaml_path.split(".")
        current: Any = self._document
        for part in yaml_parts:
            if current and isinstance(current, dict) and part in current.keys():
                current = current.get(part)
            else:
                return None
        if current is None or isinstance(current, (bool, int, list, str)):
            return current

        Log.exception_and_raise(_LOGGER, f"Unsupported value type '{type(current)}' for '{yaml_path}'")

    def merge(self, config_path: str, data: Dict[str, Any]) -> None:
        if self._document is None:
            self._document = data
            return
        self.__merge_level(config_path, "", data, self._document)

    def __merge_level(self, config_path: str, parent_key: str, current_data: Dict[str, Any],
                      parent_data: Dict[str, Any]) -> \
            None:
        for key in current_data:
            if key not in parent_data:
                parent_data[key] = current_data[key]
                continue
            current_value = current_data[key]
            parent_value = parent_data[key]
            new_parent_key = f"{parent_key}.{key}" if parent_key else key
            # CC-OFFNXT(G.CTL.03) temporary suppress
            if current_value and isinstance(current_value, dict) and parent_value and isinstance(parent_value, dict):
                self.__merge_level(config_path, new_parent_key, current_value, parent_value)
                continue
            # CC-OFFNXT(G.CTL.03) temporary suppress
            if current_value and isinstance(current_value, list) and parent_value and isinstance(parent_value, list):
                self._warnings.append(f"Attention: config file '{config_path}' merges value "
                                      f"`{new_parent_key}:{current_value}` with `{parent_value}` ")
                parent_value.extend(current_value)
                parent_data[key] = list(set(parent_value))
                continue
            current_type = type(current_value)
            parent_type = type(parent_value)
            if current_type != parent_type:
                self._warnings.append(
                    f"Attention: config file '{config_path}' for key '{new_parent_key}' provides "
                    f"different type {current_type}. Before: {parent_type}")
                continue
            if current_value == parent_value:
                continue
            self._warnings.append(
                f"Attention: config file '{config_path}' replaces value '{new_parent_key}:{parent_value}' "
                f"with '{current_value}'")
            parent_data[key] = current_value
