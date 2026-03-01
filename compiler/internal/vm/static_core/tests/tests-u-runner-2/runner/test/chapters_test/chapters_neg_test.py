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

import unittest
from pathlib import Path

from runner.chapters import Chapters, IncorrectFileFormatChapterException


class ChapterNegativeCases(unittest.TestCase):
    current_folder = Path(__file__).parent

    def test_unknown(self) -> None:
        with self.assertRaises(FileNotFoundError):
            Chapters(Path('does-not-exist.yaml'))

    def test_no_chapters(self) -> None:
        with self.assertRaises(IncorrectFileFormatChapterException):
            Chapters(self.current_folder / 'chapters_neg_1_test.yaml')

    def test_no_semicolon(self) -> None:
        with self.assertRaises(IncorrectFileFormatChapterException):
            Chapters(self.current_folder / 'chapters_neg_2_test.yaml')

    def test_incorr_exclude_01(self) -> None:
        with self.assertRaises(IncorrectFileFormatChapterException):
            Chapters(self.current_folder / 'chapters_neg_3_test.yaml')

    def test_incorr_exclude_02(self) -> None:
        # Missed ':' results in parsing of entire exclude list as one include item.
        # This is YAML parser behaviour
        test_file = self.current_folder / 'chapters_neg_4_test.yaml'
        test_chapter = 'ch1'
        chapters = Chapters(test_file)
        ch1 = chapters.chapters.get(test_chapter)
        if ch1:
            len_includes = len(ch1.includes)
            len_excludes = len(ch1.excludes)
            self.assertEqual(len_includes, 2)
            self.assertEqual(len_excludes, 0)
        else:
            self.assertIsNotNone(ch1, f"Chapter '{test_chapter}' is absent in '{test_file}'")
