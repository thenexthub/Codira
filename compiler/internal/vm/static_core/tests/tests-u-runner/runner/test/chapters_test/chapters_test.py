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
from typing import List, Set

from runner.chapters import Chapters


class ChapterTest(unittest.TestCase):
    current_folder = os.path.dirname(__file__)
    chapters: Chapters
    base_folder: str
    files: List[str]

    @classmethod
    def setUpClass(cls) -> None:
        cls.base_folder = ''
        cls.files = [
            'folder1/file1.ets',
            'folder1/file2.ets',
            'folder1/file3.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
            'folder3/test2.ets',
            'folder4/file1.ets',
            'fol-der 1/fi_le 1.ets',
        ]
        cls.chapters = Chapters(os.path.join(cls.current_folder, 'chapters_test.yaml'))

    def test_ch1(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch1',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch2(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch2',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch3(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch3',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file2.ets',
            'folder1/file3.ets',
            'folder3/test2.ets',
            'folder4/file1.ets',
            'fol-der 1/fi_le 1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch4_1(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch4_1',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch4(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch4',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder1/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch5(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch5',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder3/file1.ets',
            'folder3/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch6(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch6',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder1/file2.ets',
            'folder1/file3.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
            'folder3/test2.ets',
            'folder4/file1.ets',
            'fol-der 1/fi_le 1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch7(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch7',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch8(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch8',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch9(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch9',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
            'folder4/file1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch10(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch10',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder4/file1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch11(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch11',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'folder1/file1.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch12(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch12',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch13(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch13',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch14(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch14',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch15(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch15',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch16(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch16',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = {
            'fol-der 1/fi_le 1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_non_exist_chapter(self) -> None:
        actual = ChapterTest.chapters.filter_by_chapter(
            chapter_name='non-exist',
            base_folder=ChapterTest.base_folder,
            extension="ets",
            files=ChapterTest.files
        )
        expected: Set[str] = set()
        self.assertSetEqual(actual, expected)
