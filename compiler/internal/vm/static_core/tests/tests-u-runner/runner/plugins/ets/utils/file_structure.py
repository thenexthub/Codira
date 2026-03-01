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
# This file defines the CTS file structure
# The entrypoint is the function 'walk_test_subdirs'
from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Iterator


@dataclass
class TestDirectory:
    path: Path
    name: str

    parent: Optional[TestDirectory]
    subdirs: List[TestDirectory]

    def __init__(self, path: Path,
                 test_id: int = 0,
                 name: str = "",
                 parent: Optional[TestDirectory] = None,
                 subdirs: Optional[List[TestDirectory]] = None) -> None:

        self.path = path

        if test_id == 0 or name == "":
            self.name = str(path)
        else:
            self.test_id = test_id
            self.name = name

        self.parent = parent
        self.subdirs = subdirs if subdirs is not None else []

    def full_index(self) -> List[int]:
        cur: Optional[TestDirectory] = self
        result = []
        while cur is not None:
            result.append(cur.test_id)
            cur = cur.parent
        return list(reversed(result))

    def iter_files(self, allowed_ext: Optional[List[str]] = None) -> Iterator[Path]:
        for filename in os.listdir(str(self.path)):
            filepath: Path = self.path / filename
            if allowed_ext and filepath.suffix not in allowed_ext:
                continue
            yield filepath

    def add_subdir(self, test_dir: TestDirectory) -> None:
        test_dir.parent = self
        self.subdirs.append(test_dir)

    def find_subdir_by_name(self, name: str) -> Optional[TestDirectory]:
        # decrease complexity
        for sub_dir in self.subdirs:
            if sub_dir.name == name:
                return sub_dir
        return None

    def is_empty(self) -> bool:
        return len(os.listdir(str(self.path))) == 0


def walk_test_subdirs(path: Path, parent: Optional[TestDirectory] = None) -> Iterator[TestDirectory]:
    """
    Walks the file system from the CTS root, yielding TestDirectories, in correct order:
    For example, if only directories 1, 1/1, 1/1/1, 1/1/2, 1/2 exist, they will be yielded in that order.
    """
    subdirs = []
    for name in os.listdir(str(path)):
        if (path / name).is_dir():
            subdirs.append(TestDirectory(parent=parent, path=(path / name)))

    for subdir in subdirs:
        yield subdir
        # walk recursively
        for subsubdir in walk_test_subdirs(subdir.path, subdir):
            yield subsubdir
