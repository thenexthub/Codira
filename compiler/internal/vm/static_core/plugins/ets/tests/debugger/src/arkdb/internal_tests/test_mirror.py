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

from typing import Any, List, Tuple

import pytest

from arkdb.mirrors import mirror_array as arr
from arkdb.mirrors import mirror_object as cls
from arkdb.mirrors import mirror_primitive as value

from arkdb.mirrors import Object, arkts_str


def param(a, b, op: str):
    return pytest.param(
        a,
        b,
        id=f" {a!r} {op} {b!r} ",
    )


def param_eq(args: List[Tuple[Any, Any]]):
    return [param(a, b, "==") for a, b in args]


def param_neq(args: List[Tuple[Any, Any]]):
    return [param(a, b, "!=") for a, b in args]


@pytest.mark.parametrize(
    "x,y",
    param_eq(
        [
            (
                cls("foo", a=1, b=2.3, c="ddd"),
                cls("foo", a=1, b=2.3, c="ddd"),
            ),
            (
                cls("foo2", a=1, b=2.3, c="ddd"),
                cls("foo2", b=2.3, a=1, c="ddd"),
            ),
            (
                cls("parent", a=cls("child", b=12)),
                cls("parent", a=cls("child", b=12)),
            ),
            (
                arr("arr", [1, 2, 3]),
                arr("arr", [1, 2, 3]),
            ),
            (
                arr("arr", [cls("foo", a=1), cls("foo", a=2), cls("foo", a=3)]),
                arr("arr", [cls("foo", a=1), cls("foo", a=2), cls("foo", a=3)]),
            ),
            (
                cls("Null", a=None),
                cls("Null", a=None),
            ),
            (
                arr("arr", []),
                arr("arr", []),
            ),
            (
                value("int", 1),
                value("int", 1),
            ),
            (
                value("int", 2),
                2,
            ),
            (
                3,
                value("int", 3),
            ),
            (
                cls("std.core.Double", value=1.1),
                cls("std.core.Double", value=value("number", 1.1)),
            ),
            (
                cls("std.core.Double", value=value("number", 2.1)),
                cls("std.core.Double", value=2.1),
            ),
        ]
    ),
)
def test_mirror_primitive_eq(x, y) -> None:
    assert x == y


@pytest.mark.parametrize(
    "x,y",
    param_neq(
        [
            (
                cls("foo.1", a=1, b=2.3, c="ddd"),
                cls("foo.2", a=1, b=2.3, c="ddd"),
            ),
            (
                cls("parent", a=cls("child", b=12)),
                cls("parent", a=cls("child", b=13)),
            ),
            (
                arr("arr", [1, 2, 3]),
                arr("arr", [1, 2, 3, 4]),
            ),
            (
                arr("arr", [1, 2, 3, 4]),
                arr("arr", [1, 2, 3]),
            ),
            (
                arr("arr", [cls("foo.1", a=1), cls("foo", a=2), cls("foo", a=3)]),
                arr("arr", [cls("foo", a=1), cls("foo", a=2), cls("foo", a=3)]),
            ),
            (
                arr("arr", [cls("foo", a=1), cls("foo", a=2, b=1), cls("foo", a=3)]),
                arr("arr", [cls("foo", a=1), cls("foo", a=2), cls("foo", a=3)]),
            ),
            (
                arr("arr", [cls("foo", a=1), cls("foo", a=2), cls("foo", a=3)]),
                arr("arr", [cls("foo", a=1), cls("foo", a=2)]),
            ),
            (
                cls("Null"),
                cls("Null", a=None),
            ),
            (
                value("int", 1),
                value("int", 2),
            ),
            (
                value("int", 1),
                value("float", 1),
            ),
            (
                value("int", 1),
                2,
            ),
            (
                2,
                value("int", 1),
            ),
            (
                cls("std.core.Double", value=1.1),
                cls("std.core.Double", value=value("number", 1.2)),
            ),
            (
                cls("std.core.Double", value=value("number", 1.1)),
                cls("std.core.Double", value=1.2),
            ),
        ]
    ),
)
def test_mirror_primitive_neq(x, y) -> None:
    assert x != y


def test_mirror_dollar_field() -> None:
    data = cls("This", **{"$this": 2, "this": 3})
    assert data.this == 3
    assert getattr(data, "$this") == 2


# @pytest.mark.skip
def test_mirror_inline_classes() -> None:
    class ClassA(Object):
        ca = 1

        def __init__(self) -> None:
            self.a = 2

    class ClassB(ClassA):
        cb = 3

        def __init__(self) -> None:
            super().__init__()
            self.b = 4

    assert arkts_str(ClassB()) == "ClassB {a: 2, b: 4, ca: 1, cb: 3}"
