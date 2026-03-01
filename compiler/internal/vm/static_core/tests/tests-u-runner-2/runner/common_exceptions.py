#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2026 Huawei Device Co., Ltd.
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

from pathlib import Path

ExceptionName = str
ExceptionMessage = str
ExceptionInfo = tuple[ExceptionName, ExceptionMessage]


def parse_exception_args(args: tuple) -> tuple[str, str]:
    first = str(args[0])
    colon_pos = first.find(":")
    name = first[:colon_pos].strip()
    error = first[colon_pos + 1:].strip()
    return name, error


class RunnerException(Exception):
    def __init__(self, message: str) -> None:
        self.message = message
        super().__init__(self.message)


class InvalidInitialization(RunnerException):
    pass


class DownloadException(RunnerException):
    pass


class UnzipException(RunnerException):
    pass


class InvalidConfiguration(RunnerException):
    pass


class ParameterNotFound(RunnerException):
    pass


class MacroNotExpanded(RunnerException):
    pass


class RunnerCustomSchemeException(RunnerException):
    pass


class TestNotExistException(RunnerException):
    pass


class InvalidGTestNameException(RunnerException):
    pass


class StepUtilsException(RunnerException):
    pass


class MalformedStepConfigurationException(RunnerException):
    def __init__(self, message: str) -> None:
        message = f"Malformed configuration file: {message}"
        super().__init__(message)


class IncorrectEnumValue(RunnerException):
    pass


class IncorrectFileFormatChapterException(RunnerException):
    def __init__(self, chapters_name: str | Path) -> None:
        message = f"Incorrect file format: {chapters_name}"
        super().__init__(message)


class CyclicDependencyChapterException(RunnerException):
    def __init__(self, item: str) -> None:
        message = f"Cyclic dependency: {item}"
        super().__init__(message)


class UnknownException(RunnerException):
    pass


class SetupException(RunnerException):
    pass


class FileNotFoundException(RunnerException):
    pass


class YamlException(RunnerException):
    pass


class TimeoutException(RunnerException):
    pass


class TestGenerationException(RunnerException):
    def __init__(self, args: tuple) -> None:
        if len(args) > 0:
            name, error = parse_exception_args(args)
            message = f"Error \"{error}\" during test \"{name}\" generation"
        else:
            message = ""
        super().__init__(message)
