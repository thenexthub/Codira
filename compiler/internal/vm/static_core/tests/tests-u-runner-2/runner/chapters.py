#!/usr/bin/env python3
# -- coding: utf-8 --
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
#

import re
from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from runner.common_exceptions import CyclicDependencyChapterException, IncorrectFileFormatChapterException
from runner.logger import Log
from runner.options.yaml_document import YamlDocument


@dataclass
class Chapter:
    name: str
    includes: list[str]
    excludes: list[str]


_LOGGER = Log.get_logger(__file__)


class Chapters:
    def __init__(self, chapters_file: Path):
        self.chapters = self.__parse(chapters_file)
        self.__validate_cycles()

    @staticmethod
    def __parse(chapters_file: Path) -> dict[str, Chapter]:
        result: dict[str, Chapter] = {}
        yaml_header: dict[str, list] = YamlDocument.load(chapters_file)
        if not yaml_header or not isinstance(yaml_header, dict):
            raise IncorrectFileFormatChapterException(chapters_file)
        yaml_chapters: list[dict] | None = yaml_header.get('chapters')
        if not yaml_chapters or not isinstance(yaml_chapters, list):
            raise IncorrectFileFormatChapterException(chapters_file)
        for yaml_chapter in yaml_chapters:
            if not isinstance(yaml_chapter, dict):
                raise IncorrectFileFormatChapterException(chapters_file)
            for yaml_name, yaml_items in yaml_chapter.items():
                chapter = Chapters.__parse_chapter(yaml_name, yaml_items, chapters_file)
                if chapter.name in result:
                    raise IncorrectFileFormatChapterException(f"Chapter '{chapter.name}' already used")
                result[chapter.name] = chapter
        return result

    @staticmethod
    def __parse_item(includes: list[str], excludes: list[str], yaml_item: str | dict) -> None:
        if isinstance(yaml_item, str):
            includes.append(yaml_item.strip())
        elif isinstance(yaml_item, dict):
            for sub_name, sub_items in yaml_item.items():
                if sub_name == 'exclude' and isinstance(sub_items, list):
                    excludes.extend(sub_items)
                else:
                    raise IncorrectFileFormatChapterException(
                        f"Only 'exclude' is allowed as a nested dictionary: {sub_name}")

    @staticmethod
    def __parse_chapter(name: str, yaml_items: Sequence[str | dict[str, str]], chapters_file: Path) -> Chapter:
        if not isinstance(yaml_items, list):
            raise IncorrectFileFormatChapterException(f"Incorrect file format: {chapters_file}")
        includes: list[str] = []
        excludes: list[str] = []
        for yaml_item in yaml_items:
            Chapters.__parse_item(includes, excludes, yaml_item)

        return Chapter(name, includes, excludes)

    @staticmethod
    def __filter_by_mask(mask: str, files: list[Path]) -> list[Path]:
        filtered: list[Path] = []
        mask = Chapters.__escape_mask(mask)
        mask_folder = f'{mask}/.*' if '*' not in mask else mask
        mask_pattern = re.compile(mask)
        mask_folder_pattern = re.compile(mask_folder)
        for file in files:
            match_file = mask_pattern.search(str(file))
            match_folder = mask_folder_pattern.search(str(file))
            if match_file is not None:
                filtered.append(file)
            elif match_folder is not None:
                filtered.append(file)
        return filtered

    @staticmethod
    def __escape_mask(mask: str) -> str:
        return (mask.replace('\\', r'\\')
                .replace('.', r'\.')
                .replace('*', '.*'))

    def filter_by_chapter(self, chapter_name: str, base_folder: Path, files: list[Path]) -> set[Path]:
        if chapter_name not in self.chapters:
            return set()
        chapter = self.chapters[chapter_name]
        filtered: set[Path] = set()
        for inc in chapter.includes:
            if inc in self.chapters:
                filtered.update(self.filter_by_chapter(inc, base_folder, files))
            else:
                mask = str(base_folder.joinpath(inc))
                filtered.update(Chapters.__filter_by_mask(mask, files))
        excluded: set[Path] = set()
        for exc in chapter.excludes:
            if exc in self.chapters:
                excluded.update(self.filter_by_chapter(exc, base_folder, files))
            else:
                mask = str(base_folder.joinpath(exc))
                excluded.update(Chapters.__filter_by_mask(mask, files))
        return filtered - excluded

    def __validate_cycles(self) -> None:
        """
        :raise: CyclicDependencyChapterException if a cyclic dependency found
            Normal finish means that no cycle found
        """
        seen_chapters: list[str] = []
        for name, chapter in self.chapters.items():
            seen_chapters.append(name)
            self.__check_cycle(chapter, seen_chapters)
            seen_chapters.pop()

    def __check_cycle(self, chapter: Chapter, seen_chapters: list[str]) -> None:
        """
        Checks if items contains any name from seen
        :param chapter: investigated chapter
        :param seen_chapters: array of seen items' names
        :raise: CyclicDependencyChapterException if a name of nested chapter is in seen names
            Normal finish means that no cycle found
        """
        for item in chapter.includes + chapter.excludes:
            if item not in self.chapters:
                continue
            if item in seen_chapters:
                raise CyclicDependencyChapterException(item)
            seen_chapters.append(item)
            self.__check_cycle(self.chapters[item], seen_chapters)
            seen_chapters.pop()
