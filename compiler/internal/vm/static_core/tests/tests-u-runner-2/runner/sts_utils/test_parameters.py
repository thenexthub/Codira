#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

# This file provides functions thar are responsible for loading test parameters
# that are used in templates (yaml lists).

import os.path as ospath
from pathlib import Path
from typing import Any

import yaml

from runner.sts_utils.constants import LIST_PREFIX, YAML_EXTENSIONS
from runner.sts_utils.exceptions import InvalidFileFormatException
from runner.utils import iter_files, read_file

Params = dict


def load_params(dirpath: Path) -> Params:
    """
    Loads all parameters for a directory
    """
    result = {}

    for filename, filepath in iter_files(dirpath, allowed_ext=YAML_EXTENSIONS):
        name_without_ext, _ = ospath.splitext(filename)
        if not name_without_ext.startswith(LIST_PREFIX):
            raise InvalidFileFormatException(message="Lists of parameters must start with 'list.'", filepath=filepath)
        listname = name_without_ext[len(LIST_PREFIX):]
        params: list[dict[str, Any]] = parse_yaml(filepath)  # type: ignore[explicit-any]
        if not isinstance(params, list):
            raise InvalidFileFormatException(message="Parameters list must be YAML array", filepath=filepath)
        result[listname] = params
    return result


def parse_yaml(path: str) -> Any:       # type: ignore[explicit-any]
    """
    Parses a single YAML list of parameters
    """
    text = read_file(path)
    try:
        params = yaml.safe_load(text)
    # NOTE(pronai): can we make it more precise?
    except Exception as common_exp:
        raise InvalidFileFormatException(
            message=f"Could not load YAML: {common_exp!s}",
            filepath=path) from common_exp
    return params
