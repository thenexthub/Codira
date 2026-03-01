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

import builtins
import types
from typing import Any, List, Optional, Protocol, Tuple, Type, runtime_checkable

from ..logs import logger

LOG = logger(__name__)

DEFAULT_ARKTS_STR_DEPTH = 10


class MirrorMeta(type):
    def __new__(mcls, name: str, bases: Tuple[type], namespace: dict[str, Any], module: Any = None, **kwargs: Any):
        namespace = {
            **namespace,
            "__arkts_mirror_type__": True,
        }
        if module:
            namespace["__module__"] = module
        return super().__new__(mcls, name, bases, namespace, **kwargs)

    def __hash__(self) -> int:
        return id(self)


def is_mirror_type(t: Type) -> bool:
    return getattr(t, "__arkts_mirror_type__", False)


@runtime_checkable
class ArkPrintable(Protocol):
    def __arkts_str__(
        self,
        *,
        depth: Optional[int] = None,
    ) -> str:
        pass


def _arkts_str(obj: Any, *, depth: Optional[int] = None) -> str:
    if isinstance(obj, ArkPrintable):
        return obj.__arkts_str__(depth=depth)

    b = builtins
    t = types
    match type(obj):
        case b.bool:
            return "true" if obj else "false"
        case b.int | b.float:
            return str(obj)
        case b.str:
            return obj  # It should be changed to 'repr()' when the string representation in runtime is corrected.
        case t.NoneType:
            return "null"
        case b.list:
            if depth == 0:
                return "[...]"
            next_depth = depth - 1 if depth is not None else None
            return f"[{', '.join([_arkts_str(o, depth=next_depth) for o in obj])}]"
        case _:
            return repr(obj)


def arkts_str(*objs, depth: Optional[int] = DEFAULT_ARKTS_STR_DEPTH) -> str:
    match len(objs):
        case 0:
            return ""
        case 1:
            return _arkts_str(objs[0], depth=depth)
        case _:
            return ", ".join(arkts_str_list(list(objs), depth=depth))


def arkts_str_list(objs: List, depth: Optional[int] = DEFAULT_ARKTS_STR_DEPTH) -> List[str]:
    return [_arkts_str(o, depth=depth) for o in objs]
