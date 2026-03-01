#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

# flake8: noqa
# pylint: skip-file

import json
import inspect
from unittest import TestCase
from typing import Optional
from pathlib import Path
from dataclasses import dataclass, field
from collections import namedtuple
from vmb.helpers import Singleton, remove_prefix, Jsonable, get_plugin
from vmb.helpers_numbers import auto_str, nano_str
from vmb.plugins.hooks.cpumask import parse_bitmask

NameVal = namedtuple("NameVal", "name value")


def test_singleton() -> None:
    class S(metaclass=Singleton):
        pass

    a = S()
    b = S()
    TestCase().assertTrue(a is b)


def test_prefix() -> None:
    test = TestCase()
    test.assertTrue("There" == remove_prefix("HelloThere", "Hello"))
    test.assertTrue("NotMatched" == remove_prefix("NotMatched", "nOT"))


def test_jsonable() -> None:

    @dataclass
    class A(Jsonable):
        x: str = 'XXX'
        y: int = 123
        z: NameVal = NameVal('N', 'V')

        @classmethod
        def from_obj(cls, **kwargs):
            kwargs = {
                k: v for k, v in kwargs.items()
                if k in inspect.signature(cls).parameters
            }
            for k, v in kwargs.items():
                if 'z' == k:
                    kwargs[k] = NameVal(*v)
            return cls(**kwargs)

    @dataclass
    class B(Jsonable):
        a: A = field(default_factory=A)
        b: str = 'BBB'

    test = TestCase()
    a = A()
    test.assertTrue(a.z.name == 'N')
    test.assertTrue(a.z.value == 'V')

    test.assertTrue('{"a": {"x": "XXX", "y": 123, "z": ["N", "V"]}, "b": "BBB"}' == \
        B().js(indent=None))

    obj = json.loads('{"x": "ok", "y": 5, "z": [3, 4]}')
    a = A.from_obj(**obj)
    test.assertTrue(a.z.name == 3)
    test.assertTrue(a.z.value == 4)


@dataclass
class C(Jsonable):
    m: int
    n: Optional[int]


def test_json_loads() -> None:
    test = TestCase()
    obj = json.loads('{"n": 1, "m":2}')
    test.assertTrue(C(**obj).n == 1)
    obj = json.loads('{"n": null, "m":2}')
    test.assertTrue(C(**obj).n is None)


def test_bitmask():
    test = TestCase()
    test.assertTrue(parse_bitmask('0x03')[1] == [True] * 2 + [False] * 6)
    test.assertTrue(parse_bitmask('0xF0')[1] == [False] * 4 + [True] * 4)
    test.assertTrue(
        parse_bitmask('0x100')[1] == [False] * 8 + [True] + [False] * 3)


def test_numbers():
    test = TestCase()
    test.assertEqual(auto_str(1.40e-09), f'{1:>7}n')
    test.assertEqual(auto_str(0.123), f'{123:>7}m')
    test.assertEqual(auto_str(123456), f'{123:>7}K')
    test.assertEqual(nano_str(1.40e-09), f'{1:>11}n')
    test.assertEqual(nano_str(1.0), f'{1000000000:>11}n')
    test.assertEqual(nano_str(1.50e+07), '15000000000000000n')


def check_plugin(kind: str, plugin_name: str):
    plugin = get_plugin(kind, plugin_name)
    if 'langs' == kind:
        plugin.Lang()


def test_load_plugins():
    # try load all the plugins
    plug_root = Path(__file__).parent.parent.joinpath('src', 'vmb', 'plugins').resolve()
    for kind in ('hooks', 'tools', 'langs', 'platforms'):
        for p in [f.with_suffix('').name for f in plug_root.joinpath(kind).glob('*.py')]:
            try:
                check_plugin(kind, p)
            except SystemExit:
                TestCase().assertTrue(False, 'sys.exit called')
