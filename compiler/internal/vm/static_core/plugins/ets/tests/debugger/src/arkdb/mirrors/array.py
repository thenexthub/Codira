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

from collections.abc import Iterator
from typing import Any, Generic, Iterable, List, Optional, TypeVar

from rich.repr import rich_repr

from .base import arkts_str_list
from .primitive import PrimitiveMeta

T = TypeVar("T")


@rich_repr
class Array(Generic[T], metaclass=PrimitiveMeta):
    """Base class for ArkTS mirror primitives."""

    __slots__ = ("_cls_name", "_items")

    def __init__(self, cls_name: str, items: List[T] | None = None) -> None:
        self._cls_name = cls_name
        self._items = [*items] if items is not None else []

    def __eq__(self, other: Any) -> bool:
        if self is other:
            return True
        if not isinstance(other, Array):
            return False
        if self._cls_name != other._cls_name:
            return False
        return self._items == other._items

    # CC-OFFNXT(G.NAM.05) Standard function from rich package
    def __rich_repr__(self):
        yield self._cls_name
        yield self._items

    # CC-OFFNXT(G.NAM.05) internal function name
    def __arkts_str__(self, *, depth: Optional[int] = None) -> str:
        if depth == 0:
            return "[...]"
        next_depth = depth - 1 if depth is not None else None
        childs = ", ".join(arkts_str_list(self._items, depth=next_depth))
        return f"[{childs}]"

    def __iter__(self) -> Iterator[T]:
        return self._items.__iter__()

    def __getitem__(self, *args, **kwargs) -> T:
        return self._items.__getitem__(*args, **kwargs)


def mirror_array(cls_name: str, items: Iterable[Any]) -> Array:
    return Array(cls_name=cls_name, items=list(items))
