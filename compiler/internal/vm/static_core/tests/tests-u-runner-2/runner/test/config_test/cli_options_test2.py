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

from runner.common_exceptions import InvalidConfiguration
from runner.options.cli_options import CliOptions
from runner.test.test_utils import MethodType
from runner.test.test_utils import get_method as gm


class CliOptionsTest2(unittest.TestCase):
    @staticmethod
    def get_method(name: str) -> MethodType:
        return gm(CliOptions, name)

    def test_get_option_value1(self) -> None:
        method = self.get_method("__get_option_value")
        value = method("option", "0")
        self.assertIsInstance(value, int)
        self.assertEqual(0, value)

    def test_get_option_value2(self) -> None:
        method = self.get_method("__get_option_value")
        value = method("option", "FalSe")
        self.assertIsInstance(value, bool)
        self.assertFalse(value)

    def test_get_option_value3(self) -> None:
        method = self.get_method("__get_option_value")
        value = method("option", "abc")
        self.assertIsInstance(value, str)
        self.assertEqual("abc", value)

    def test_get_option_value4(self) -> None:
        method = self.get_method("__get_option_value")
        with self.assertRaises(InvalidConfiguration):
            method("option", None)

    def test_parse_option_with_space1(self) -> None:
        method = self.get_method("__parse_option_with_space")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter=new"
        ]
        expected_args = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "--processes", "all",
            "--force-generate", "--verbose-filter=new"
        ]
        actual_value, refreshed_args = method(arg_list, "--verbose")
        self.assertEqual("silent", actual_value)
        self.assertIsInstance(actual_value, str)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option_with_space2(self) -> None:
        method = self.get_method("__parse_option_with_space")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter=new"
        ]
        expected_args = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--verbose", "silent", "--processes", "all",
            "--force-generate", "--verbose-filter=new"
        ]
        actual_value, refreshed_args = method(arg_list, "--show-progress")
        self.assertTrue(actual_value)
        self.assertIsInstance(actual_value, bool)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option_with_space3(self) -> None:
        method = self.get_method("__parse_option_with_space")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter=new"
        ]
        expected_args = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent", "--processes", "all",
            "--force-generate", "--verbose-filter=new"
        ]
        actual_value, refreshed_args = method(arg_list, "--groups")
        self.assertEqual(1, actual_value)
        self.assertIsInstance(actual_value, int)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option_with_equal1(self) -> None:
        method = self.get_method("__parse_option_with_equal")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "--verbose", "silent",
            "--processes", "all", "--force-generate", "--verbose-filter=new"
        ]
        expected_args = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "--verbose", "silent",
            "--processes", "all", "--force-generate"
        ]
        actual_value, refreshed_args = method(arg_list, "--verbose-filter")
        self.assertEqual("new", actual_value)
        self.assertIsInstance(actual_value, str)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option_with_equal2(self) -> None:
        method = self.get_method("__parse_option_with_equal")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate=FALSE", "--verbose-filter=new"
        ]
        expected_args = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--verbose-filter=new"
        ]
        actual_value, refreshed_args = method(arg_list, "--force-generate")
        self.assertFalse(actual_value)
        self.assertIsInstance(actual_value, bool)
        self.assertListEqual(expected_args, refreshed_args)

    def test_parse_option_with_equal3(self) -> None:
        method = self.get_method("__parse_option_with_equal")
        arg_list = [
            "--es2panda-timeout=95", "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate=FALSE", "--verbose-filter=new"
        ]
        expected_args = [
            "--ark-timeout", "210", "--gc-type", "stw",
            "--heap-verifier", "fail_on_verification:pre:into:post", "--groups", "1",
            "--group-number=1", "--show-progress", "true", "--verbose", "silent",
            "--processes", "all", "--force-generate=FALSE", "--verbose-filter=new"
        ]
        actual_value, refreshed_args = method(arg_list, "--es2panda-timeout")
        self.assertEqual(95, actual_value)
        self.assertIsInstance(actual_value, int)
        self.assertListEqual(expected_args, refreshed_args)
