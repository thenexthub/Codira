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
import re
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Optional, Any, List, Union

import yaml

METADATA_PATTERN = re.compile(r"(?<=/\*---)(.*?)(?=---\*/)", flags=re.DOTALL)
DOTS_WHITESPACES_PATTERN = r"(\.\w+)*"
PACKAGE_PATTERN = re.compile(f"\\n\\s*package[\\t\\f\\v  ]+(?P<package_name>\\w+{DOTS_WHITESPACES_PATTERN})\\b")
SPEC_CHAPTER_PATTERN = re.compile(r"^\d{1,2}\.{0,1}\d{0,3}\.{0,1}\d{0,3}\.{0,1}\d{0,3}\.{0,1}\d{0,3}$")
_LOGGER = logging.getLogger("runner.plugins.ets.ets_templates.test_metadata")


class Tags:
    class EtsTag(Enum):
        COMPILE_ONLY = "compile-only"
        NO_WARMUP = "no-warmup"
        NOT_A_TEST = "not-a-test"
        NEGATIVE = "negative"

    def __init__(self, tags: Optional[List[str]] = None) -> None:
        self.__compile_only = Tags.__contains(Tags.EtsTag.COMPILE_ONLY.value, tags)
        self.__negative = Tags.__contains(Tags.EtsTag.NEGATIVE.value, tags)
        self.__not_a_test = Tags.__contains(Tags.EtsTag.NOT_A_TEST.value, tags)
        self.__no_warmup = Tags.__contains(Tags.EtsTag.NO_WARMUP.value, tags)

    @property
    def compile_only(self) -> bool:
        return self.__compile_only

    @property
    def negative(self) -> bool:
        return self.__negative

    @property
    def not_a_test(self) -> bool:
        return self.__not_a_test

    @property
    def no_warmup(self) -> bool:
        return self.__no_warmup

    @staticmethod
    def __contains(tag: str, tags: Optional[List[str]]) -> bool:
        return tag in tags if tags is not None else False


@dataclass
class TestMetadata:
    tags: Tags
    desc: Optional[str] = None
    files: Optional[List[str]] = None
    assertion: Optional[str] = None
    params: Optional[Any] = None
    name: Optional[str] = None
    entry_point: Optional[str] = None
    test_cli: Optional[List[str]] = None
    package: Optional[str] = None
    module: Optional[str] = None
    ark_options: List[str] = field(default_factory=list)
    timeout: Optional[int] = None
    spec: Optional[str] = None
    expected_out: Union[str, List[str], None] = None
    expected_error: Union[str, List[str], None] = None

    def __post_init__(self) -> None:
        if isinstance(self.expected_out, list):
            self.expected_out = '\n'.join(self.expected_out)
        if isinstance(self.expected_error, list):
            self.expected_error = '\n'.join(self.expected_error)

        if self.spec is None:
            return
        self.spec = str(self.spec)
        if not re.match(SPEC_CHAPTER_PATTERN, self.spec):
            error_message = f"Incorrect format of specification chapter number : {self.spec}"
            _LOGGER.error(error_message)

    def get_package_name(self) -> str:
        return self.package if self.package is not None else ""


def get_metadata(path: Path) -> TestMetadata:
    data = Path.read_text(path)
    yaml_text = "\n".join(re.findall(METADATA_PATTERN, data))
    metadata = yaml.safe_load(yaml_text)
    if metadata is None:
        metadata = {}
    metadata['tags'] = Tags(metadata.get('tags'))
    metadata['assertion'] = metadata.get('assert')
    metadata['entry_point'] = metadata.get('entry_point')
    metadata['test_cli'] = metadata.get('test_cli')
    if 'assert' in metadata:
        del metadata['assert']
    if isinstance(type(metadata.get('ark_options')), str):
        metadata['ark_options'] = [metadata['ark_options']]
    metadata['package'] = metadata.get("package")
    if metadata['package'] is None:
        metadata['package'] = get_package_statement(path)
    metadata['module'] = metadata['package']
    if metadata['module'] is None:
        metadata['module'] = path.stem
    return TestMetadata(**metadata)


def get_package_statement(path: Path) -> Optional[str]:
    data = Path.read_text(path)
    match = PACKAGE_PATTERN.search(data)
    if match is None:
        return None
    return stmt if (stmt := match.group("package_name")) is not None else None
