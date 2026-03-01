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

from typing import Any, Generic, Optional, Tuple, Type, TypeVar

import rich.repr

from .base import MirrorMeta, arkts_str
from .type_cache import type_cache

T = TypeVar("T")


class PrimitiveMeta(MirrorMeta):

    def __new__(mcls, name: str, bases: Tuple[Type], namespace: dict[str, Any], **kwargs: Any):
        cls = super().__new__(mcls, name, bases, namespace, **kwargs)
        t: Type = rich.repr.auto(cls)
        return t


class Primitive(Generic[T], metaclass=PrimitiveMeta):
    """Base class for ArkTS mirror primitives."""

    value: T

    __slots__ = ("value",)

    def __init__(self, value: T | None = None) -> None:
        if value is not None:
            self.value = value

    def __eq__(self, other: Any) -> bool:
        if type(self) is type(other) or type(self).__name__ == type(other).__name__:
            value = other.value if hasattr(other, "value") else other
            return self.value == value
        if not hasattr(other, "value"):
            return self.value == other
        return False

    # CC-OFFNXT(G.NAM.05) Standard function from rich package
    def __rich_repr__(self):
        return [self.value]

    # CC-OFFNXT(G.NAM.05) internal function name
    def __arkts_str__(self, *, depth: Optional[int] = None) -> str:
        return arkts_str(self.value)

    def __hash__(self) -> int:
        return hash(type(self).__name__) ^ hash(self.value)


@type_cache(scope="session")
def mirror_primitive_type(cls_name: str) -> Type:
    return PrimitiveMeta(cls_name, (Primitive,), {}, module=__package__)


def mirror_primitive(cls_name: str, value: T) -> Primitive[T]:
    t = mirror_primitive_type(cls_name)
    return t(value)
