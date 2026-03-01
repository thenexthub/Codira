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
from typing import cast

from runner.suites.test_metadata import TestMetadata


class MetadataTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)

    def test_empty_metadata(self) -> None:
        metadata = TestMetadata.create_filled_metadata({}, Path(__file__))
        self.assertIsNotNone(metadata)
        self.assertEqual(str(metadata.tags), '[]')
        self.assertIsNone(metadata.desc)
        self.assertIsNone(metadata.files)
        self.assertIsNone(metadata.expected_out)
        self.assertIsNone(metadata.expected_error)

    def test_metadata_main_items(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'desc': "abc",
            'tags': [],
            'files': [],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)
        self.assertEqual(metadata.desc, "abc")
        self.assertIsNotNone(metadata.files)
        self.assertListEqual(cast(list[str], metadata.files), [])

    def test_metadata_tags_compile_only(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertTrue(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_negative(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['negative'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertTrue(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_not_a_test(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['not-a-test'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertTrue(metadata.tags.not_a_test)
        self.assertFalse(metadata.tags.no_warmup)

    def test_metadata_tags_no_warmup(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertFalse(metadata.tags.compile_only)
        self.assertFalse(metadata.tags.negative)
        self.assertFalse(metadata.tags.not_a_test)
        self.assertTrue(metadata.tags.no_warmup)

    def test_metadata_tags_all(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertTrue(metadata.tags.compile_only)
        self.assertTrue(metadata.tags.negative)
        self.assertTrue(metadata.tags.not_a_test)
        self.assertTrue(metadata.tags.no_warmup)

    def test_metadata_files(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'files': ['test1.sts', 'test2.sts'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(cast(list[str], metadata.files)), 2)
        self.assertListEqual(cast(list[str], metadata.files), ['test1.sts', 'test2.sts'])

    def test_extra_key(self) -> None:
        with self.assertRaises(TypeError):
            metadata = TestMetadata.create_filled_metadata({
                'abc': 'def',
            }, Path(__file__))
            self.assertIsNone(metadata)

    def test_non_existing_path(self) -> None:
        with self.assertRaises(FileNotFoundError):
            metadata = TestMetadata.create_filled_metadata({}, Path("path"))
            self.assertIsNone(metadata)

    def test_one_wrong_tag(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compily-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 1)
        self.assertEqual(metadata.tags.invalid_tags, ['compily-only'])

    def test_wrong_tags(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compily-only', 'negatives', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 2)
        self.assertEqual(metadata.tags.invalid_tags, ['compily-only', 'negatives'])

    def test_all_valid_tags(self) -> None:
        metadata = TestMetadata.create_filled_metadata({
            'tags': ['compile-only', 'negative', 'not-a-test', 'no-warmup'],
        }, Path(__file__))
        self.assertIsInstance(metadata, TestMetadata)
        self.assertIsNotNone(metadata)
        self.assertEqual(len(metadata.tags.invalid_tags), 0)
