#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

import unittest
from pathlib import Path

from runner.options.cli_options import CliOptions
from runner.test.test_utils import MethodType
from runner.test.test_utils import get_method as gm


class CliOptionsTest1(unittest.TestCase):
    @staticmethod
    def get_method(name: str) -> MethodType:
        return gm(CliOptions, name)

    def test_parse_option01(self) -> None:
        method = self.get_method("__parse_option")
        arg_list = [
            "panda-jit", "ets-func-tests", "--es2panda-timeout", "95",
            "--ark-timeout", "210", "--gc-type", "stw", "--heap-verifier",
            "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number", "1", "--show-progress", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter", "new"
        ]
        option_name = "--verbose"
        default_value = "silent"
        actual_value, refreshed_args = method(arg_list, option_name, default_value)
        self.assertEqual("silent", actual_value)
        expected_args = [
            'panda-jit', 'ets-func-tests', '--es2panda-timeout', '95',
            '--ark-timeout', '210', '--gc-type', 'stw', '--heap-verifier',
            'fail_on_verification:pre:into:post', '--groups', '1',
            '--group-number', '1', '--show-progress', '--processes', 'all',
            '--force-generate', '--verbose-filter', 'new'
        ]
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option02(self) -> None:
        method = self.get_method("__parse_option")
        arg_list = [
            "panda-jit", "ets-func-tests", "--es2panda-timeout", "95",
            "--ark-timeout", "210", "--gc-type", "stw", "--heap-verifier",
            "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number", "1", "--show-progress", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter", "new"
        ]
        expected_args = [
            'panda-jit', 'ets-func-tests', '--es2panda-timeout', '95',
            '--ark-timeout', '210', '--gc-type', 'stw', '--heap-verifier',
            'fail_on_verification:pre:into:post', '--group-number', '1',
            '--show-progress', '--verbose', 'silent', '--processes', 'all',
            '--force-generate', '--verbose-filter', 'new'
        ]
        option_name = "--groups"
        default_value = 1
        actual_value, refreshed_args = method(arg_list, option_name, default_value)
        self.assertEqual(1, actual_value)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option03(self) -> None:
        method = self.get_method("__parse_option")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate=FALSE", "--verbose-filter=new"
        ]
        expected_args = [
            '--es2panda-timeout=95', '--ark-timeout', '210', '--gc-type', 'stw',
            '--heap-verifier', 'fail_on_verification:pre:into:post', '--groups',
            '1', '--show-progress', 'true', '--verbose', 'silent', '--processes',
            'all', '--force-generate=FALSE', '--verbose-filter=new'
        ]
        option_name = "--group-number"
        default_value = 1
        actual_value, refreshed_args = method(arg_list, option_name, default_value)
        self.assertEqual(1, actual_value)
        self.assertListEqual(expected_args, refreshed_args)

    def test_is_option_value_int1(self) -> None:
        method = self.get_method("__is_option_value_int")
        is_int, value = method(1)
        self.assertTrue(is_int)
        self.assertIsInstance(value, int)
        self.assertEqual(1, value)

    def test_is_option_value_int2(self) -> None:
        method = self.get_method("__is_option_value_int")
        is_int, value = method('1')
        self.assertTrue(is_int)
        self.assertIsInstance(value, int)
        self.assertEqual(1, value)

    def test_is_option_value_int3(self) -> None:
        method = self.get_method("__is_option_value_int")
        is_int, value = method('abc')
        self.assertFalse(is_int)
        self.assertNotIsInstance(value, int)
        self.assertIsNone(value)

    def test_is_option_value_int4(self) -> None:
        method = self.get_method("__is_option_value_int")
        is_int, value = method(Path(""))
        self.assertFalse(is_int)
        self.assertNotIsInstance(value, int)
        self.assertIsNone(value)

    def test_is_option_value_int5(self) -> None:
        method = self.get_method("__is_option_value_int")
        is_int, value = method(None)
        self.assertFalse(is_int)
        self.assertNotIsInstance(value, int)
        self.assertIsNone(value)

    def test_is_option_value_str1(self) -> None:
        method = self.get_method("__is_option_value_str")
        is_str, value = method("1")
        self.assertTrue(is_str)
        self.assertIsInstance(value, str)
        self.assertEqual("1", value)

    def test_is_option_value_str2(self) -> None:
        method = self.get_method("__is_option_value_str")
        is_str, value = method(1)
        self.assertFalse(is_str)
        self.assertNotIsInstance(value, str)
        self.assertIsNone(value)

    def test_is_option_value_bool1(self) -> None:
        method = self.get_method("__is_option_value_bool")
        is_bool, value = method(False)
        self.assertTrue(is_bool)
        self.assertIsInstance(value, bool)
        self.assertFalse(value)

    def test_is_option_value_bool2(self) -> None:
        method = self.get_method("__is_option_value_bool")
        is_bool, value = method("False")
        self.assertTrue(is_bool)
        self.assertIsInstance(value, bool)
        self.assertFalse(value)

    def test_is_option_value_bool3(self) -> None:
        method = self.get_method("__is_option_value_bool")
        is_bool, value = method("TrUe")
        self.assertTrue(is_bool)
        self.assertIsInstance(value, bool)
        self.assertTrue(value)

    def test_is_option_value_bool4(self) -> None:
        method = self.get_method("__is_option_value_bool")
        is_bool, value = method("yes")
        self.assertFalse(is_bool)
        self.assertNotIsInstance(value, bool)
        self.assertIsNone(value)
