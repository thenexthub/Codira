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

from __future__ import annotations

import json
import re
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import ClassVar

from runner.common_exceptions import RunnerException
from runner.logger import Log

_LOGGER = Log.get_logger(__file__)
AstCheckerError = tuple[str, str]
AstCheckerErrorsSet = set[AstCheckerError]
LocationType = tuple[int, int]


class UtilASTChecker:
    skip_options: ClassVar[dict[str, bool]] = {'SkipErrors': False, 'SkipWarnings': False}

    class AstCheckerException(RunnerException):
        pass

    class _TestType(Enum):
        NODE = 'Node'
        ERROR = 'Error'
        WARNING = 'Warning'

    @dataclass
    class _Pattern:
        pattern_type: UtilASTChecker._TestType
        pattern: str
        line: int
        col: int
        error_file: str = ''

    class _TestCase:
        """
        Class for storing test case parsed from test file
        """

        def __init__(self, name: str | None, pattern: UtilASTChecker._Pattern, checks: dict) -> None:
            self.name = name
            self.line = pattern.line
            self.col = pattern.col
            self.test_type = pattern.pattern_type
            self.checks = checks
            self.error_file = pattern.error_file

        def __repr__(self) -> str:
            return f'TestCase({self.name}, {self.line}:{self.col}, {self.test_type}, {self.checks}, {self.error_file})'

        def __eq__(self, other: object) -> bool:
            if not isinstance(other, UtilASTChecker._TestCase):
                return NotImplemented
            return bool(self.name == other.name and self.line == other.line and self.col == other.col
                        and self.test_type == other.test_type and self.checks == other.checks
                        and self.error_file == other.error_file)

        def __hash__(self) -> int:
            return hash(self.__repr__())

    class TestCasesList:
        """
        Class for storing test cases parsed from one test file
        """

        def __init__(self, tests_list: set[UtilASTChecker._TestCase]):
            self.tests_list = tests_list
            has_error_tests = False
            has_warning_tests = False
            for test in tests_list:
                if test.test_type == UtilASTChecker._TestType.ERROR:
                    has_error_tests = True
                if test.test_type == UtilASTChecker._TestType.WARNING:
                    has_warning_tests = True
            self.has_error_tests = has_error_tests
            self.has_warning_tests = has_warning_tests
            self.skip_errors = False
            self.skip_warnings = False

    def __init__(self) -> None:
        self.regex = re.compile(r'/\*\s*@@\s*(?P<pattern>.*?)\s*\*/', re.DOTALL)
        self.reset_skips()

    @staticmethod
    def create_test_case(name: str | None, pattern: UtilASTChecker._Pattern) -> UtilASTChecker._TestCase:
        pattern_parsed = {'error': pattern.pattern}
        if pattern.pattern_type == UtilASTChecker._TestType.NODE:
            try:
                pattern_parsed = json.loads(pattern.pattern)
            except json.JSONDecodeError as ex:
                raise UtilASTChecker.AstCheckerException(
                    f'TestCase: {name}.\nThrows JSON error: {ex}.\nJSON data: {pattern.pattern}') from ex
        return UtilASTChecker._TestCase(name, pattern, pattern_parsed)

    @staticmethod
    def get_match_location(match: re.Match, start: bool = False) -> LocationType:
        """
        Returns match location in file: line and column (counting from 1)
        """
        match_idx = match.start() if start else match.end()
        line_start_index = match.string[:match_idx].rfind('\n') + 1
        col = match_idx - line_start_index + 1
        line = match.string[:match_idx].count('\n') + 1
        return line, col

    @staticmethod
    def check_properties(node: dict, properties: dict) -> bool:
        """
        Checks if `node` contains all key:value pairs specified in `properties` dict argument
        """
        for key, value in properties.items():
            if node.get(key) != value:
                return False
        return True

    @staticmethod
    def run_error_test(test_file: Path, test: _TestCase, actual_errors: AstCheckerErrorsSet) -> bool:
        file_name: str = test_file.name if test.error_file == '' else test.error_file
        expected_error: AstCheckerError = f'{test.checks["error"]}', f'[{file_name}:{test.line}:{test.col}]'
        if expected_error in actual_errors:
            actual_errors.remove(expected_error)
            return True
        _LOGGER.all(f'No Expected error {expected_error}')
        return False

    @staticmethod
    def get_actual_errors(error: str) -> set:
        actual_errors = set()
        for error_str in error.splitlines():
            if error_str.strip():
                error_text, error_loc = error_str.rsplit(' ', 1)
                actual_errors.add((error_text.strip(), error_loc))
        return actual_errors

    def reset_skips(self) -> None:
        self.skip_options['SkipErrors'] = False
        self.skip_options['SkipWarnings'] = False

    def check_skip_error(self) -> bool:
        return self.skip_options["SkipErrors"]

    def check_skip_warning(self) -> bool:
        return self.skip_options["SkipWarnings"]

    def parse_define_statement(self, match: re.Match[str],
                               link_defs_map: dict[str, list[tuple[UtilASTChecker._TestType, str]]],
                               link_sources_map: dict[str, re.Match[str]]) -> UtilASTChecker._TestCase | None:
        """
        Parses `@<id> <pattern-type> <pattern>`
        """
        match_str = re.sub(r'\s+', ' ', match.group('pattern'))[1:].strip()
        sep1 = match_str.find(' ')
        sep2 = match_str.find(' ', sep1 + 1)
        if sep1 == -1 or sep2 == -1:
            raise UtilASTChecker.AstCheckerException(
                'Wrong definition format: expected '
                f'`/* @@@ <id> <pattern-type> <pattern> */`, got /* @@@ {match_str} */')
        name = match_str[:sep1]
        pattern_type = UtilASTChecker._TestType(match_str[sep1 + 1:sep2])
        pattern = match_str[sep2 + 1:]

        name = match_str[:sep1]
        link_def = link_defs_map.get(name, [])
        link_def.append((pattern_type, pattern))
        link_defs_map[name] = link_def

        if name in link_sources_map:
            match = link_sources_map[name]
            line, col = self.get_match_location(match)
            return self.create_test_case(name, UtilASTChecker._Pattern(pattern_type, pattern, line, col))
        return None

    def parse_match_statement(self, match: re.Match[str],
                              link_defs_map: dict[str, list[tuple[UtilASTChecker._TestType, str]]],
                              link_sources_map: dict[str, re.Match[str]]) -> list[UtilASTChecker._TestCase]:
        """
        Parses `<pattern-type> <pattern>` and `<id>`
        """
        str_match = match.group('pattern')
        sep = str_match.find(' ')
        if sep == -1:
            # parse `<id>`
            name = str_match
            if any(not char.isalnum() and char != '_' for char in name):
                raise UtilASTChecker.AstCheckerException(
                    f'Bad `<id>` value, expected value from `[a-zA-Z0-9_]+`, got {name}')

            if name in link_sources_map:
                line, col = self.get_match_location(match)
                raise UtilASTChecker.AstCheckerException(f'Link {name} (at location {line}:{col}) is already defined')

            if name in link_defs_map:
                result = []
                for (pattern_type, pattern) in link_defs_map[name]:
                    line, col = self.get_match_location(match)
                    result.append(
                        self.create_test_case(name, UtilASTChecker._Pattern(pattern_type, pattern, line, col)))
                return result
            link_sources_map[name] = match
            return []

        # parse `<pattern-type> <pattern>`
        pattern_type = UtilASTChecker._TestType(str_match[:sep])
        pattern = str_match[sep + 1:]
        line, col = self.get_match_location(match)
        return [self.create_test_case(None, UtilASTChecker._Pattern(pattern_type, pattern, line, col))]

    def parse_skip_statement(self, match: re.Match[str]) -> None:
        """
        Parses `# <skip-option>*`
        """
        match_str = re.sub(r'\s+', ' ', match.group('pattern'))[1:].strip()
        pattern = r"(\w+)\s*=\s*(\w+)"
        matches = re.findall(pattern, match_str)
        for opt, val in matches:
            value = val.lower()
            if opt not in self.skip_options or value not in ['true', 'false']:
                raise UtilASTChecker.AstCheckerException('Wrong match_at_location format: expected '
                                                         f'`/* @@# <skip-option> */`, got /* @@? {match_str} */')
            self.skip_options[opt] = value == 'true'

    def parse_match_at_loc_statement(self, match: re.Match[str]) -> UtilASTChecker._TestCase:
        """
        Parses `? <line>:<col> <pattern-type> <pattern>`
        and  `? <file-name>:<line>:<col> <pattern-type> <pattern>`
        """
        match_str = re.sub(r'\s+', ' ', match.group('pattern'))[1:].strip()
        sep1 = match_str.find(' ')
        sep2 = match_str.find(' ', sep1 + 1)
        if sep1 == -1 or sep2 == -1:
            raise UtilASTChecker.AstCheckerException('Wrong match_at_location format: expected '
                                                     '`/* @@? <line>:<col> <pattern-type> <pattern> */`, '
                                                     f'got /* @@? {match_str} */')
        location = match_str[:sep1]
        loc_sep1 = location.find(':')
        loc_sep2 = location.find(':', loc_sep1 + 1)
        if loc_sep1 == -1:
            raise UtilASTChecker.AstCheckerException('Wrong match_at_location format: expected '
                                                     '`/* @@? <line>:<col> <pattern-type> <pattern> */`, '
                                                     f'got /* @@? {match_str} */')
        line_str = location[:loc_sep1] if loc_sep2 == -1 else location[loc_sep1 + 1:loc_sep2]
        col_str = location[loc_sep1 + 1:] if loc_sep2 == -1 else location[loc_sep2 + 1:]
        error_file = '' if loc_sep2 == -1 else location[:loc_sep1]
        if loc_sep2 != -1:
            pass
        if not line_str.isdigit():
            raise UtilASTChecker.AstCheckerException(f'Expected line number, got {line_str}')
        if not col_str.isdigit():
            raise UtilASTChecker.AstCheckerException(f'Expected column number, got {col_str}')

        line, col = int(line_str), int(col_str)
        pattern_type = UtilASTChecker._TestType(match_str[sep1 + 1:sep2])
        pattern = match_str[sep2 + 1:]

        return self.create_test_case(
            name=None,
            pattern=UtilASTChecker._Pattern(pattern_type, pattern, line, col, error_file)
        )

    def parse_tests(self, test_text: str) -> UtilASTChecker.TestCasesList:
        """
        Takes .ets file with tests and parses them into a list of TestCases.
        """
        self.reset_skips()
        link_defs_map: dict[str, list[tuple[UtilASTChecker._TestType, str]]] = {}
        link_sources_map: dict[str, re.Match[str]] = {}
        test_cases = set()
        matches = list(re.finditer(self.regex, test_text))
        for match in matches:
            pattern = match.group('pattern')
            tests = []
            if pattern.startswith('@'):
                test = self.parse_define_statement(match, link_defs_map, link_sources_map)
                if test is not None:
                    tests.append(test)
            elif pattern.startswith('?'):
                test = self.parse_match_at_loc_statement(match)
                if test is not None:
                    tests.append(test)
            elif pattern.startswith('#'):
                self.parse_skip_statement(match)
            else:
                tests = self.parse_match_statement(match, link_defs_map, link_sources_map)
            for test in tests:
                test_cases.add(test)
        test_case_list = UtilASTChecker.TestCasesList(test_cases)
        test_case_list.skip_errors = self.check_skip_error()
        test_case_list.skip_warnings = self.check_skip_warning()
        return test_case_list

    def find_nodes_by_start_location(self, root: dict, line: int, col: int) -> list[dict]:
        """
        Finds all descendants of `root` with location starting at `loc`
        """
        nodes = []
        start = root.get('loc', {}).get('start', {})
        if start.get('line', None) == line and start.get('column', None) == col:
            nodes.append(root)

        for child in root.values():
            if isinstance(child, dict):
                nodes.extend(self.find_nodes_by_start_location(child, line, col))
            if isinstance(child, list):
                for item in child:
                    nodes.extend(self.find_nodes_by_start_location(item, line, col))
        return nodes

    def run_node_test(self, test: _TestCase, ast: dict) -> bool:
        if test.test_type != UtilASTChecker._TestType.NODE:
            return True
        nodes_by_loc = self.find_nodes_by_start_location(ast, test.line, test.col)
        test_passed = False
        for node in nodes_by_loc:
            if self.check_properties(node, test.checks):
                test_passed = True
                break
        return test_passed

    def run_tests(self, test_file: Path, test_cases: TestCasesList, ast: dict, error: str = '') -> bool:
        """
        Takes AST and runs tests on it, returns True if all tests passed
        """
        _LOGGER.all(f'Running {len(test_cases.tests_list)} tests...')
        failed_tests = 0
        actual_errors: AstCheckerErrorsSet = self.get_actual_errors(error)
        tests_set = set(test_cases.tests_list)
        node_test_passed = True
        error_test_passed = True
        warning_test_passed = True

        for i, test in enumerate(tests_set):
            if test.test_type == UtilASTChecker._TestType.NODE:
                node_test_passed = self.run_node_test(test, ast)
            elif test.test_type == UtilASTChecker._TestType.ERROR:
                error_test_passed = True if test_cases.skip_errors \
                    else self.run_error_test(test_file, test, actual_errors)
            elif test.test_type == UtilASTChecker._TestType.WARNING:
                warning_test_passed = True if test_cases.skip_warnings \
                    else self.run_error_test(test_file, test, actual_errors)
            test_name = f'Test {i + 1}' + ('' if test.name is None else f': {test.name}')
            if bool(node_test_passed and error_test_passed and warning_test_passed):
                _LOGGER.all(f'PASS: {test_name}')
            else:
                _LOGGER.all(f'FAIL: {test_name} in {test_file}')
                failed_tests += 1

        failed_tests = self.check_remaining_errors(actual_errors, failed_tests)

        if failed_tests == 0:
            _LOGGER.all('All tests passed')
            return True

        _LOGGER.all(f'Failed {failed_tests} tests')
        return False

    def check_remaining_errors(self, actual_errors: AstCheckerErrorsSet, failed_tests: int) -> int:
        for actual_error in actual_errors:
            if actual_error[0].split()[0] == "Warning:" and not self.check_skip_warning():
                _LOGGER.all(f'Unexpected warning {actual_error}')
                failed_tests += 1
            if (actual_error[0].split()[0] in ("TypeError:", "SyntaxError:", "Error:")
                    and not self.check_skip_error()):
                _LOGGER.all(f'Unexpected error {actual_error}')
                failed_tests += 1

        return failed_tests
