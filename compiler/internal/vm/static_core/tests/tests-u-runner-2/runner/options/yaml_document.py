#!/usr/bin/env python3
# -- coding: utf-8 --
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

from pathlib import Path
from typing import Any

import yaml

from runner import utils
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class YamlDocument:

    def __init__(self) -> None:
        super().__init__()
        self._document: dict[str, Any] | None = None  # type: ignore[explicit-any]
        self._warnings: list[str] = []

    @staticmethod
    def load(config_path: Path) -> dict[str, Any]:  # type: ignore[explicit-any]
        with open(config_path, encoding="utf-8") as stream:
            data: dict[str, Any] = yaml.safe_load(stream)   # type: ignore[explicit-any]
            return data

    @staticmethod
    def save(config_path: str, data: dict[str, Any]) -> None:   # type: ignore[explicit-any]
        data_to_save = yaml.dump(data, indent=4)
        utils.write_2_file(config_path, data_to_save)
