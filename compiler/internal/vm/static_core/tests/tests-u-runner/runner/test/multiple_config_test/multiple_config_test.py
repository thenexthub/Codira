#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

import os
import unittest
from typing import Dict, Any, Optional

from runner.options.yaml_document import YamlDocument


class MultipleConfigTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)

    def compare_dicts(self, dict1: Dict[str, Any], dict2: Dict[str, Any]) -> None:
        self.assertEqual(len(dict1), len(dict2), f"Dict1: {dict1.keys()}, dict2: {dict2.keys()}")
        for key in dict1:
            self.assertTrue(key in dict2)
            value1 = dict1[key]
            value2 = dict2[key]
            self.assertEqual(type(value1), type(value2),
                             f"Key '{key}': type of value1 = {type(value1)}, "
                             f"type of value2 = {type(value2)}. Expected to be equal")

            if value1 and value2 and isinstance(value1, dict):
                self.compare_dicts(value1, value2)
            elif isinstance(value1, list):
                self.assertListEqual(sorted(value1), sorted(value2))
            else:
                self.assertEqual(value1, value2,
                                 f"Key '{key}': value1 = {value1}, value2 = {value2}. Expected to be equal")

    def test_load_one_config(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [os.path.join(self.current_folder, "config-1.yaml")]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True
            },
            'es2panda': {
                'timeout': 60,
                'opt-level': 4
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    'world',
                    "-a=5"]
            }
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        self.assertListEqual(yaml.warnings(), [])

    def test_load_independent_configs(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-1.yaml"),
                os.path.join(self.current_folder, "config-2.yaml")
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 60,
                'opt-level': 4,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    'world',
                    "-a=5"],
            },
            'verifier': {
                'enable': True,
                'timeout': 30
            }
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        self.assertListEqual(yaml.warnings(), [])

    def test_load_configs_repeated_values(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-1.yaml"),
                os.path.join(self.current_folder, "config-3.yaml")
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 30,
                'opt-level': 2,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    'world',
                    "-a=5"],
            },
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        for warning in yaml.warnings():
            self.assertTrue(warning.find("config-3.yaml' replaces value") > 0)
        self.assertEqual(len(yaml.warnings()), 2)

    def test_load_configs_repeated_values_inverse(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-3.yaml"),
                os.path.join(self.current_folder, "config-1.yaml"),
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 60,
                'opt-level': 4,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    'world',
                    "-a=5"],
            },
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        for warning in yaml.warnings():
            self.assertTrue(warning.find("config-1.yaml' replaces value") > 0)
        self.assertEqual(len(yaml.warnings()), 2)

    def test_load_configs_different_types(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-1.yaml"),
                os.path.join(self.current_folder, "config-4.yaml")
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 60,
                'opt-level': 4,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    'world',
                    "-a=5"],
            },
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        for warning in yaml.warnings():
            self.assertTrue(warning.find("config-4.yaml") > 0 and warning.find("provides different type") > 0,
                            f"Actual warning: {warning}. Expected: provides different type")
        self.assertEqual(len(yaml.warnings()), 2)

    def test_load_configs_merging_lists(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-1.yaml"),
                os.path.join(self.current_folder, "config-5.yaml")
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 60,
                'opt-level': 4,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    "-a=5",
                    'world',
                    'hello',
                ],
            },
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        for warning in yaml.warnings():
            self.assertTrue(warning.find("config-5.yaml") > 0 and warning.find("merges value") > 0,
                            f"Actual warning: {warning}. Expected: merges value")
        self.assertEqual(len(yaml.warnings()), 1)

    def test_load_configs_many(self) -> None:
        yaml = YamlDocument()
        yaml.load_configs(
            [
                os.path.join(self.current_folder, "config-1.yaml"),
                os.path.join(self.current_folder, "config-2.yaml"),
                os.path.join(self.current_folder, "config-3.yaml"),
                os.path.join(self.current_folder, "config-4.yaml"),
                os.path.join(self.current_folder, "config-5.yaml")
            ]
        )
        actual: Optional[Dict[str, Any]] = yaml.document()
        expected: Dict[str, Any] = {
            'test-suites': ['ets_func_tests'],
            'general': {
                'build': '~/build',
                'verbose': 'NONE',
                'show-progress': True,
            },
            'es2panda': {
                'timeout': 30,
                'opt-level': 2,
            },
            'ark': {
                'timeout': 180,
                'heap-verifier': 'fail_on_verification',
                'ark_args': [
                    "-a=5",
                    'world',
                    'hello',
                ],
            },
            'verifier': {
                'enable': True,
                'timeout': 30
            }
        }
        self.assertIsNotNone(actual)
        if actual:
            self.compare_dicts(actual, expected)
        self.assertEqual(len(yaml.warnings()), 5)

    def test_non_exiting_config(self) -> None:
        with self.assertRaises(FileNotFoundError):
            yaml = YamlDocument()
            yaml.load_configs(
                [
                    os.path.join(self.current_folder, "config-non-exist.yaml"),
                ]
            )
            self.assertIsNone(yaml.document())
