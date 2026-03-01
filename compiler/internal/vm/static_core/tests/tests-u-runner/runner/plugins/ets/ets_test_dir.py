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

from functools import cached_property
from pathlib import Path
from typing import Optional


class EtsTestDir:
    def __init__(self, static_core_root: str, root: Optional[str] = None) -> None:
        self.__static_core_root = Path(static_core_root)
        self.__root = root

    @property
    def tests(self) -> Path:
        return self.root / "tests"

    @property
    def ets_templates(self) -> Path:
        return self.tests / "ets-templates"

    @property
    def ets_func_tests(self) -> Path:
        return self.tests / "ets_func_tests"

    @property
    def stdlib_templates(self) -> Path:
        return self.tests / "stdlib-templates"

    @property
    def ets_func_tests_templates(self) -> Path:
        return self.tests / "ets-func-tests-templates"

    @property
    def gc_stress(self) -> Path:
        return (self.__static_core_root.parent.parent
                / "ets_frontend" / "ets2panda" / "test"/ "ark_tests" / "ets-tests" / "ets-gc-stress")

    @property
    def ets_es_checked(self) -> Path:
        return self.tests / "ets_es_checked"

    @property
    def ets_ts_subset(self) -> Path:
        return self.tests / "ets_ts_subset"

    @property
    def ets_sdk(self) -> Path:
        return self.tests / "ets_sdk"

    @property
    def declgen_sdk(self) -> Path:
        return self.__static_core_root / "plugins" / "ets" / "sdk"

    @cached_property
    def root(self) -> Path:
        return Path(self.__root) if self.__root else self.__static_core_root / "plugins" / "ets"
