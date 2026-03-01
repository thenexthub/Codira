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

import operator
from functools import reduce
from typing import Any, Iterable, Optional, Tuple, Type

import rich.repr

from .base import MirrorMeta, _arkts_str
from .type_cache import type_cache


class ObjectMeta(MirrorMeta):

    def __new__(mcls, name: str, bases: Tuple[Type], namespace: dict[str, Any], **kwargs: Any):
        namespace = {
            **namespace,
            "__eq__": ObjectMeta.__eq__,
            "__repr__": ObjectMeta.__repr__,
            "__rich_repr__": ObjectMeta.__rich_repr__,
            "__arkts_repr__": ObjectMeta.__arkts_repr__,
            "__arkts_str__": ObjectMeta.__arkts_str__,
            "__hash__": ObjectMeta.__hash__,
        }
        cls = super().__new__(mcls, name, bases, namespace, **kwargs)
        return rich.repr.auto(cls)

    # CC-OFFNXT(G.NAM.05) Standard function from rich package
    def __rich_repr__(self) -> Iterable[Tuple[str, Any]]:
        return [(name, attr) for name, attr in vars(self).items() if not callable(attr)]

    # CC-OFFNXT(G.NAM.05) internal function name
    def __arkts_repr__(self) -> Iterable[Tuple[str, Any]]:
        return [
            (name, attr)
            for name, attr in [(name, getattr(self, name)) for name in dir(self) if not name.startswith("_")]
            if not callable(attr)
        ]

    def __eq__(self, other: Any) -> bool:
        if type(self).__name__ != type(other).__name__:
            return False
        if vars(self).keys() != vars(other).keys():
            return False
        for k, v in vars(self).items():
            if v != vars(other).get(k, None):
                return False
        return True

    # CC-OFFNXT(G.NAM.05) internal function name
    def __arkts_str__(self, *, depth: Optional[int] = None) -> str:
        if depth == 0:
            return f"{self.__class__.__name__} {{...}}"
        next_depth = depth - 1 if depth is not None else None
        params = ", ".join([f"{name}: {_arkts_str(param, depth=next_depth)}" for name, param in self.__arkts_repr__()])
        return f"{self.__class__.__name__} {{{params}}}"

    def __hash__(self) -> int:
        return reduce(operator.xor, [hash(k) for k in vars(self).keys()]) ^ hash(type(self).__name__)


class Object(metaclass=ObjectMeta):
    """Base class for ArkTS mirror classes."""

    pass


@type_cache(scope="test")
def mirror_object_type(cls_name: str, /, **kwargs) -> Type:
    t = ObjectMeta(cls_name, (Object,), kwargs, module=__package__)
    return t


def mirror_object(cls_name: str, /, **kwargs):
    t = mirror_object_type(cls_name, **dict.fromkeys(kwargs.keys()))
    obj = t()
    for k, v in kwargs.items():
        setattr(obj, k, v)
    return obj
