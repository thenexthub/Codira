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

from pathlib import Path
from typing import Tuple

from runner.plugins.ets.utils.constants import NEGATIVE_PREFIX, NEGATIVE_EXECUTION_PREFIX, SKIP_PREFIX


def is_negative(path: Path) -> bool:
    return path.name.startswith(NEGATIVE_PREFIX) or path.name.startswith(NEGATIVE_EXECUTION_PREFIX)


def should_be_skipped(path: Path) -> bool:
    return path.name.startswith(SKIP_PREFIX)


def strip_template(path: Path) -> Tuple[str, int]:
    stem = path.stem
    i = path.stem.rfind("_")
    if i == -1:
        return stem, 0
    template_name = stem[:i]
    test_index = stem[i + 1:]
    if not test_index.isdigit():
        return stem, 0
    return template_name, int(test_index)
