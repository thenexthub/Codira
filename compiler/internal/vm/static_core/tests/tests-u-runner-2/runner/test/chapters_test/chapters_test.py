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

from runner.chapters import Chapters


class ChapterTest(unittest.TestCase):
    current_folder = Path(__file__).parent
    chapters: Chapters
    base_folder: Path
    files: list[Path]

    @classmethod
    def setUpClass(cls) -> None:
        cls.base_folder = Path()
        files_str = [
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
        cls.files = [Path(file_str) for file_str in files_str]
        cls.chapters = Chapters(cls.current_folder / 'chapters_test.yaml')

    def test_ch1(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch1',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch2(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch2',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder2/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch3(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch3',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file2.ets',
            'folder1/file3.ets',
            'folder3/test2.ets',
            'folder4/file1.ets',
            'fol-der 1/fi_le 1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch4_1(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch4_1',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch4(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch4',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder1/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch5(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch5',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder3/file1.ets',
            'folder3/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch6(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch6',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
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
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch7',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch8(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch8',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder2/file2.ets',
            'folder2/file3.ets',
            'folder3/file1.ets',
            'folder3/file2.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch9(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch9',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
            'folder4/file1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch10(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch10',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder4/file1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_ch11(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch11',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'folder1/file1.ets',
            'folder3/file1.ets',
            'folder3/file2.ets',
        }
        self.assertSetEqual(actual, expected)

    def test_ch12(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch12',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch13(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch13',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch14(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch14',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch15(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch15',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)

    def test_ch16(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='ch16',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = {
            'fol-der 1/fi_le 1.ets'
        }
        self.assertSetEqual(actual, expected)

    def test_non_exist_chapter(self) -> None:
        raw = ChapterTest.chapters.filter_by_chapter(
            chapter_name='non-exist',
            base_folder=ChapterTest.base_folder,
            files=ChapterTest.files
        )
        actual = {str(act) for act in raw}
        expected: set[str] = set()
        self.assertSetEqual(actual, expected)
