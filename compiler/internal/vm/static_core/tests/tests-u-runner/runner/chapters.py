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
#

import logging
import re
from dataclasses import dataclass
from os import path
from typing import Optional, Dict, Any, Union, List, Set, Sequence

from runner.logger import Log
from runner.options.yaml_document import YamlDocument


@dataclass
class Chapter:
    name: str
    includes: List[str]
    excludes: List[str]


class IncorrectFileFormatChapterException(Exception):
    def __init__(self, chapters_name: str) -> None:
        message = f"Incorrect file format: {chapters_name}"
        Exception.__init__(self, message)


class CyclicDependencyChapterException(Exception):
    def __init__(self, item: str) -> None:
        message = f"Cyclic dependency: {item}"
        Exception.__init__(self, message)


_LOGGER = logging.getLogger("runner.chapters")


class Chapters:
    def __init__(self, chapters_file: str):
        self.chapters = self.__parse(chapters_file)
        self.__validate_cycles()

    @staticmethod
    def __parse(chapters_file: str) -> Dict[str, Chapter]:
        result: Dict[str, Chapter] = {}
        yaml_header: Dict[str, Any] = YamlDocument.load(chapters_file)
        if not yaml_header or not isinstance(yaml_header, dict):
            Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
        yaml_chapters: Optional[List[Any]] = yaml_header.get('chapters')
        if not yaml_chapters or not isinstance(yaml_chapters, list):
            Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
        for yaml_chapter in yaml_chapters:
            if not isinstance(yaml_chapter, dict):
                Log.exception_and_raise(_LOGGER, chapters_file, IncorrectFileFormatChapterException)
            for yaml_name, yaml_items in yaml_chapter.items():
                chapter = Chapters.__parse_chapter(yaml_name, yaml_items, chapters_file)
                if chapter.name in result:
                    Log.exception_and_raise(
                        _LOGGER,
                        f"Chapter '{chapter.name}' already used",
                        IncorrectFileFormatChapterException)
                result[chapter.name] = chapter
        return result

    @staticmethod
    def __parse_item(includes: List[str], excludes: List[str], yaml_item: Union[str, dict]) -> None:
        if isinstance(yaml_item, str):
            includes.append(yaml_item.strip())
        elif isinstance(yaml_item, dict):
            for sub_name, sub_items in yaml_item.items():
                if sub_name == 'exclude' and isinstance(sub_items, list):
                    excludes.extend(sub_items)
                else:
                    Log.exception_and_raise(
                        _LOGGER,
                        f"Only 'exclude' is allowed as a nested dictionary: {sub_name}",
                        IncorrectFileFormatChapterException)

    @staticmethod
    def __parse_chapter(name: str, yaml_items: Sequence[Union[str, Dict[str, str]]], chapters_file: str) -> Chapter:
        if not isinstance(yaml_items, list):
            Log.exception_and_raise(
                _LOGGER,
                f"Incorrect file format: {chapters_file}",
                IncorrectFileFormatChapterException
            )
        includes: List[str] = []
        excludes: List[str] = []
        for yaml_item in yaml_items:
            Chapters.__parse_item(includes, excludes, yaml_item)

        return Chapter(name, includes, excludes)

    @staticmethod
    def __filter_by_mask(mask: str, files: List[str], extension: str) -> List[str]:
        filtered: List[str] = []
        if '*' not in mask and not mask.endswith(f".{extension}"):
            # mask is a folder
            mask = f'{mask}/*'
        mask = mask.replace('\\', r'\\')
        mask = mask.replace('.', r'\.')
        mask = mask.replace('*', '.*')
        for file in files:
            match = re.search(mask, file)
            if match is not None:
                filtered.append(file)
        return filtered

    def filter_by_chapter(self, chapter_name: str, base_folder: str, files: List[str], extension: str) -> Set[str]:
        if chapter_name not in self.chapters:
            return set()
        chapter = self.chapters[chapter_name]
        filtered: Set[str] = set()
        for inc in chapter.includes:
            if inc in self.chapters:
                filtered.update(self.filter_by_chapter(inc, base_folder, files, extension))
            else:
                mask = path.join(base_folder, inc)
                filtered.update(Chapters.__filter_by_mask(mask, files, extension))
        excluded: Set[str] = set()
        for exc in chapter.excludes:
            if exc in self.chapters:
                excluded.update(self.filter_by_chapter(exc, base_folder, files, extension))
            else:
                mask = path.join(base_folder, exc)
                excluded.update(Chapters.__filter_by_mask(mask, files, extension))
        return filtered - excluded

    def __validate_cycles(self) -> None:
        """
        :raise: CyclicDependencyChapterException if a cyclic dependency found
            Normal finish means that no cycle found
        """
        seen_chapters: List[str] = []
        for name, chapter in self.chapters.items():
            seen_chapters.append(name)
            self.__check_cycle(chapter, seen_chapters)
            seen_chapters.pop()

    def __check_cycle(self, chapter: Chapter, seen_chapters: List[str]) -> None:
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
                Log.exception_and_raise(_LOGGER, item, CyclicDependencyChapterException)
            seen_chapters.append(item)
            self.__check_cycle(self.chapters[item], seen_chapters)
            seen_chapters.pop()
