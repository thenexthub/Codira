#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import json
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.base_enum import BaseEnum
from runner.logger import Log

METADATA_PATTERN = re.compile(r"(?<=/\*---)(.*?)(?=---\*/)", flags=re.DOTALL)
DOTS_WHITESPACES_PATTERN = r"(\.\w+)*"
PACKAGE_PATTERN = re.compile(f"\\n\\s*package[\\t\\f\\v  ]+(?P<package_name>\\w+{DOTS_WHITESPACES_PATTERN})\\b")
SPEC_CHAPTER_PATTERN = re.compile(r"^\d{1,2}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}\.?\d{0,3}$")

__LOGGER = Log.get_logger(__file__)


class Tags:
    class EtsTag(BaseEnum):
        COMPILE_ONLY = "compile-only"
        NO_WARMUP = "no-warmup"
        NOT_A_TEST = "not-a-test"
        NEGATIVE = "negative"

    def __init__(self, tags: list[str] | None = None) -> None:
        self.__values: dict[Tags.EtsTag, bool] = {
            Tags.EtsTag.COMPILE_ONLY: Tags.__contains(Tags.EtsTag.COMPILE_ONLY.value, tags),
            Tags.EtsTag.NEGATIVE: Tags.__contains(Tags.EtsTag.NEGATIVE.value, tags),
            Tags.EtsTag.NOT_A_TEST: Tags.__contains(Tags.EtsTag.NOT_A_TEST.value, tags),
            Tags.EtsTag.NO_WARMUP: Tags.__contains(Tags.EtsTag.NO_WARMUP.value, tags),
        }
        self.invalid_tags = Tags.get_invalid_tags(tags)

    def __repr__(self) -> str:
        values = [tag.name for (tag, value) in self.__values.items() if value]
        return str(values)

    @property
    def compile_only(self) -> bool:
        return self.__values.get(Tags.EtsTag.COMPILE_ONLY, False)

    @property
    def negative(self) -> bool:
        return self.__values.get(Tags.EtsTag.NEGATIVE, False)

    @property
    def not_a_test(self) -> bool:
        return self.__values.get(Tags.EtsTag.NOT_A_TEST, False)

    @property
    def no_warmup(self) -> bool:
        return self.__values.get(Tags.EtsTag.NO_WARMUP, False)

    @staticmethod
    def get_invalid_tags(tags: str | list[str] | None) -> list[str]:
        if isinstance(tags, str):
            tags = [tag.strip() for tag in tags.split(",")]

        if tags:
            return [tag for tag in tags if tag not in Tags.EtsTag.values()]
        return []

    @staticmethod
    def __contains(tag: str, tags: list[str] | None) -> bool:
        return tag in tags if tags is not None else False


@dataclass
class TestMetadata:     # type: ignore[explicit-any]
    tags: Tags = field(default_factory=Tags)
    desc: str | None = None
    test_suffix: str | None = None
    file_name: str | None = None
    files: list[str] | None = None
    assertion: str | None = None
    params: Any | None = None   # type: ignore[explicit-any]
    name: str | None = None
    entry_point: str | None = None
    test_cli: list[str] | None = None
    package: str | None = None
    ark_options: list[str] = field(default_factory=list)
    timeout: int | None = None
    es2panda_options: list[str] = field(default_factory=list)
    spec: str | None = None
    arktsconfig: Path | None = None
    expected_out: str | None = None
    expected_error: str | None = None
    # Test262 specific metadata keys
    description: str | None = None
    defines: str | None = None
    includes: str | None = None
    features: str | None = None
    esid: str | None = None
    es5id: str | None = None
    es6id: str | None = None
    author: str | None = None
    info: str | None = None
    locale: str | None = None
    # ark_tests/parser
    issues: str | None = None

    @classmethod
    def get_metadata(cls, path: Path) -> 'TestMetadata':
        data = Path.read_text(path)
        yaml_text = "\n".join(re.findall(METADATA_PATTERN, data))
        try:
            metadata = yaml.safe_load(yaml_text)
            if metadata is None or isinstance(metadata, str):
                return cls.create_empty_metadata(path)
            return cls.create_filled_metadata(metadata, path)
        except (FileNotFoundError, TypeError, AttributeError) as error:
            logger = globals().get('__LOGGER')
            if isinstance(logger, Log):
                raise InvalidConfiguration(f"Yaml parsing: '{path}' - error: '{error}'") from error
            return cls.create_empty_metadata(path)

    @classmethod
    def create_filled_metadata(cls, metadata: dict[str, Any],       # type: ignore[explicit-any]
                               path: Path) -> 'TestMetadata':
        metadata['tags'] = Tags(metadata.get('tags'))
        metadata['entry_point'] = metadata.get('entry_point')
        metadata['test_cli'] = metadata.get('test_cli')
        if 'assert' in metadata:
            metadata['assertion'] = metadata.get('assert')
            del metadata['assert']
        if 'description' in metadata:
            metadata['desc'] = metadata['description']
        if 'flags' in metadata:
            flags_list = metadata['flags']
            if flags_list and isinstance(flags_list, list):
                metadata.update({'es2panda_options': flags_list})
            del metadata['flags']
        if isinstance(type(metadata.get('ark_options')), str):
            metadata['ark_options'] = [metadata['ark_options']]
        if isinstance(metadata.get('expected_out'), list):
            metadata['expected_out'] = '\n'.join(metadata['expected_out'])
        if isinstance(metadata.get('expected_error'), list):
            metadata['expected_error'] = '\n'.join(metadata['expected_error'])
        arktsconfig = metadata.get('arktsconfig')
        package = metadata.get("package")
        if arktsconfig is not None:
            arktsconfig_path = (path.parent / arktsconfig).resolve()
            metadata['arktsconfig'] = arktsconfig_path if arktsconfig_path.exists() else None
            package = cls.get_package_statement_from_arktsconfig(path, arktsconfig_path)
        metadata['package'] = cls.get_package_statement_from_source(path) if package is None else package
        return TestMetadata(**metadata)

    @classmethod
    def create_empty_metadata(cls, path: Path, find_package: bool = True) -> 'TestMetadata':
        metadata = cls()
        if metadata.package is None and find_package:
            metadata.package = cls.get_package_statement_from_source(path)
        return metadata

    @classmethod
    def get_package_statement_from_source(cls, path: Path) -> str | None:
        data = Path.read_text(path)
        match = PACKAGE_PATTERN.search(data)
        if match is None:
            return None
        if (stmt := match.group("package_name")) is not None:
            return stmt
        return path.stem

    @classmethod
    def get_package_statement_from_arktsconfig(cls, path: Path, arktsconfig_path: Path) -> str | None:
        with open(arktsconfig_path, encoding="utf-8") as json_handler:
            data = json.load(json_handler)
        package = data.get("compilerOptions", {}).get('package', None)
        return f"{package}.{path.stem}" if package is not None else None

    def get_package_name(self) -> str:
        return self.package if self.package is not None else ""
