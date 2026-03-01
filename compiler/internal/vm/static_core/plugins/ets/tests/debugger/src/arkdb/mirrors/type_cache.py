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

from dataclasses import dataclass, field
from functools import _CacheInfo as CacheInfo
from functools import cache as func_cach
from typing import Callable, Iterator, List, Literal, Protocol, Tuple

from ..logs import logger

LOG = logger(__name__)


class CacheWrapper(Protocol):
    def cache_info(self) -> CacheInfo:
        pass

    def cache_clear(self) -> None:
        pass


@dataclass
class HitStat:
    hits: int = 0
    misses: int = 0
    count: int = 0

    def add(self, info: CacheInfo):
        self.hits += info.hits
        self.misses += info.misses
        self.count += info.currsize


@dataclass
class Cache:
    cache: CacheWrapper
    scope: str
    name: str = ""
    stats: HitStat = field(default_factory=HitStat)

    def clear(self) -> None:
        self._accum_info()
        self.cache.cache_clear()

    def info(self) -> CacheInfo:
        return self.cache.cache_info()

    def empty(self) -> bool:
        return self.cache.cache_info().hits == 0

    def _accum_info(self):
        self.stats.add(self.cache.cache_info())


CacheScope = Literal["test", "session"]


class MirrorTypeCaches:
    _caches: List[Cache]

    def __init__(self) -> None:
        self._caches = []

    def add(self, cache: Cache):
        self._caches.append(cache)

    def find(self, *scopes: CacheScope) -> Iterator[Cache]:
        for c in self._caches:
            if len(scopes) == 0 or c.scope in scopes:
                yield c

    def stats(self) -> Iterator[Tuple[str, HitStat]]:
        for c in self._caches:
            yield c.name, c.stats


_CACHE_WRAPPERS = MirrorTypeCaches()


def type_cache(*, scope: CacheScope):
    def wrapper(user_function: Callable, /):
        w = func_cach(user_function)
        _CACHE_WRAPPERS.add(Cache(cache=w, scope=scope, name=user_function.__name__))
        return w

    return wrapper


def clear_cache(*scopes: CacheScope) -> None:
    for c in _CACHE_WRAPPERS.find(*scopes):
        if not c.empty():
            LOG.debug("Clear cache '%s': %s", c.name, c.info())
            c.clear()


def log_stats():
    lines = [f"'{name}': {stat}" for name, stat in _CACHE_WRAPPERS.stats() if stat.hits]
    LOG.info("Cache statisticts:\n%s", "\n".join(lines))
