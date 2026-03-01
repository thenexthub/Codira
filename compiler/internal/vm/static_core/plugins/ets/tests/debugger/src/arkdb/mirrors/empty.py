#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

from typing import Any, Optional

import rich.repr

from .primitive import PrimitiveMeta
from .type_cache import type_cache


class EmptyBase(metaclass=PrimitiveMeta):
    """Base class for ArkTS mirror primitives."""

    __slots__ = ()

    def __eq__(self, other: Any) -> bool:
        return type(self) is type(other) or type(self).__name__ == type(other).__name__

    # CC-OFFNXT(G.NAM.05) Standard function from rich package
    # CC-OFFNXT(G.CLS.07) followed required interface
    def __rich_repr__(self):
        return []

    # CC-OFFNXT(G.NAM.05) internal function name
    def __arkts_str__(self, *, depth: Optional[int] = None) -> str:
        return type(self).__name__


@type_cache(scope="session")
def mirror_empty_type(cls_name: str):
    return rich.repr.auto(PrimitiveMeta(cls_name, (EmptyBase,), {}, module=__package__))


def mirror_undefined():
    return mirror_empty_type("undefined")()
