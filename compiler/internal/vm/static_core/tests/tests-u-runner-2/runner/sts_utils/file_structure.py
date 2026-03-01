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
# This file defines the CTS file structure
# The entrypoint is the function 'walk_test_subdirs'
from __future__ import annotations

import os
from collections.abc import Iterator
from dataclasses import dataclass
from pathlib import Path

from runner.logger import Log

__LOGGER = Log.get_logger(__file__)


@dataclass
class TestDirectory:
    path: Path
    name: str

    parent: TestDirectory | None
    subdirs: list[TestDirectory]

    def __init__(self, path: Path, *,
                 test_id: int = 0,
                 name: str = "",
                 parent: TestDirectory | None = None,
                 subdirs: list[TestDirectory] | None = None) -> None:

        self.path = path

        if test_id == 0 or name == "":
            self.name = str(path)
        else:
            self.test_id = test_id
            self.name = name

        self.parent = parent
        self.subdirs = subdirs if subdirs is not None else []

    def full_index(self) -> list[int]:
        cur: TestDirectory | None = self
        result = []
        while cur is not None:
            result.append(cur.test_id)
            cur = cur.parent
        return list(reversed(result))

    def iter_files(self, allowed_ext: list[str] | None = None) -> Iterator[Path]:
        for filename in os.listdir(str(self.path)):
            filepath: Path = self.path / filename
            if allowed_ext and filepath.suffix not in allowed_ext:
                continue
            yield filepath

    def add_subdir(self, test_dir: TestDirectory) -> None:
        test_dir.parent = self
        self.subdirs.append(test_dir)

    def find_subdir_by_name(self, name: str) -> TestDirectory | None:
        # decrease complexity
        for sub_dir in self.subdirs:
            if sub_dir.name == name:
                return sub_dir
        return None

    def is_empty(self) -> bool:
        return len(os.listdir(str(self.path))) == 0


def walk_test_subdirs(path: Path, parent: TestDirectory | None = None) -> Iterator[TestDirectory]:
    """
    Walks the file system from the CTS root, yielding TestDirectories, in correct order:
    For example, if only directories 1, 1/1, 1/1/1, 1/1/2, 1/2 exist, they will be yielded in that order.
    """
    subdirs = []
    for name in os.listdir(str(path)):
        if (path / name).is_dir():
            subdirs.append(TestDirectory(parent=parent, path=path / name))

    for subdir in subdirs:
        yield subdir
        yield from (walk_test_subdirs(subdir.path, subdir))


def build_directory_tree(test_dir: TestDirectory) -> None:
    subdirs = []
    for name in os.listdir(str(test_dir.path)):
        if (test_dir.path / name).is_dir():
            subdirs.append(TestDirectory(
                parent=test_dir, path=test_dir.path / name))
    subdirs = sorted(subdirs, key=lambda dir: dir.test_id)

    for sub_dir in subdirs:
        test_dir.add_subdir(sub_dir)
        build_directory_tree(sub_dir)


def print_tree(test_dir: TestDirectory) -> None:
    for sub_dir in test_dir.subdirs:
        left_space = " " * 2 * len(sub_dir.full_index())
        section_index = str(sub_dir.test_id)
        section_name = sub_dir.name.replace("_", " ").title()
        right_space = 90 - len(left_space) - len(section_index) - len(section_name)

        __LOGGER.all(", ".join([left_space, section_index, section_name, "." * right_space, "\n"]))
        print_tree(sub_dir)
