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

import logging
from os import makedirs, path
from pathlib import Path

from runner.enum_types.verbose_format import VerboseKind

SUMMARY_LOG_LEVEL = 21
NONE_LOG_LEVEL = 22


class Log:
    _is_init = False

    def __init__(self, file_name: str):
        self.logger = logging.getLogger(file_name)

    @staticmethod
    def setup(verbose: VerboseKind, report_root: Path) -> 'Log':
        root_logger = Log("runner")

        log_path = report_root if report_root is not None else \
            path.join(path.sep, "tmp")
        makedirs(log_path, exist_ok=True)

        file_handler = logging.FileHandler(path.join(log_path, "runner.log"))
        console_handler = logging.StreamHandler()

        root_logger.logger.setLevel(logging.DEBUG)
        file_handler.setLevel(logging.DEBUG)

        if verbose == VerboseKind.ALL:
            console_handler.setLevel(logging.DEBUG)
        elif verbose == VerboseKind.SHORT:
            console_handler.setLevel(logging.INFO)
        else:
            console_handler.setLevel(NONE_LOG_LEVEL)

        file_formatter = logging.Formatter('%(asctime)s - %(name)s - %(message)s')
        file_handler.setFormatter(file_formatter)
        console_formatter = logging.Formatter('%(message)s')
        console_handler.setFormatter(console_formatter)

        root_logger.logger.addHandler(file_handler)
        root_logger.logger.addHandler(console_handler)

        Log._is_init = True

        return root_logger

    @staticmethod
    def get_logger(file_name: str) -> 'Log':
        pos = len(str(Path(__file__).parent.parent)) + 1
        name = file_name[pos:-3].replace(path.sep, ".")
        return Log(name)

    def all(self, message: str) -> None:
        """
        Logs on the level verbose=ALL
        """
        self.logger.debug(message)

    def short(self, message: str) -> None:
        """
        Logs on the level verbose=SHORT
        """
        self.logger.info(message)

    def summary(self, message: str) -> None:
        """
        Logs on the level verbose=SUMMARY (sum)
        """
        self.logger.log(SUMMARY_LOG_LEVEL, message)

    def default(self, message: str) -> None:
        """
        Logs on the level verbose=None
        """
        self.logger.log(NONE_LOG_LEVEL, message)
