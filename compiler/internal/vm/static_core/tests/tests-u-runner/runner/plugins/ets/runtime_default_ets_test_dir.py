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
from pathlib import Path
from typing import Optional

_LOGGER = logging.getLogger("runner.plugins.ets.es2panda_test_dir")


class RuntimeDefaultEtsTestDir:
    def __init__(self, static_core_root: str, root: Optional[str] = None) -> None:
        self.__static_core_root = Path(static_core_root)
        self.__root = root

    @property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.runtime_ets

    @property
    def es2panda_test(self) -> Path:
        symlink_es2panda_test = self.__static_core_root / "tools" / "es2panda" / "test"
        if symlink_es2panda_test.exists():
            return symlink_es2panda_test
        return self.__static_core_root.parent.parent / 'ets_frontend' / 'ets2panda' / 'test'

    @property
    def runtime_ets(self) -> Path:
        return self.es2panda_test / "runtime" / "ets"

    @property
    def list_root(self) -> Path:
        return self.es2panda_test / 'test-lists'
