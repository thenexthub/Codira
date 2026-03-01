#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import os
import unittest
from pathlib import Path

from runner.suites.test_metadata import TestMetadata


class MetadataTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)

    def test_expected_out_string(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': 'output line 1\noutput line 2',
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, 'output line 1\noutput line 2')
        self.assertIsInstance(metadata.expected_out, str)

    def test_expected_out_list(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': ['output line 1', 'output line 2', 'output line 3'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, 'output line 1\noutput line 2\noutput line 3')
        self.assertIsInstance(metadata.expected_out, str)

    def test_expected_out_empty_string(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': '',
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, '')
        self.assertIsInstance(metadata.expected_out, str)

    def test_expected_error_string(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_error': 'error line 1\nerror line 2',
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_error, 'error line 1\nerror line 2')
        self.assertIsInstance(metadata.expected_error, str)

    def test_expected_error_list(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_error': ['error line 1', 'error line 2', 'error line 3'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_error, 'error line 1\nerror line 2\nerror line 3')
        self.assertIsInstance(metadata.expected_error, str)

    def test_expected_error_empty_string(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_error': '',
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_error, '')
        self.assertIsInstance(metadata.expected_error, str)

    def test_expected_out_and_error_both_strings(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': 'output text',
            'expected_error': 'error text',
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, 'output text')
        self.assertEqual(metadata.expected_error, 'error text')
        self.assertIsInstance(metadata.expected_out, str)
        self.assertIsInstance(metadata.expected_error, str)

    def test_expected_out_and_error_both_lists(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': ['out1', 'out2'],
            'expected_error': ['err1', 'err2', 'err3'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, 'out1\nout2')
        self.assertEqual(metadata.expected_error, 'err1\nerr2\nerr3')
        self.assertIsInstance(metadata.expected_out, str)
        self.assertIsInstance(metadata.expected_error, str)

    def test_expected_out_list_single_element(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_out': ['single line'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_out, 'single line')
        self.assertIsInstance(metadata.expected_out, str)

    def test_expected_error_list_single_element(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'expected_error': ['single error'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(metadata.expected_error, 'single error')
        self.assertIsInstance(metadata.expected_error, str)
