#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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


import logging
import os
from pathlib import Path
from typing import List
import yaml


_LOGGER = logging.getLogger("runner.plugins.ets.ets_utils")


class ETSUtilsException(Exception):
    pass


# NOTE(pronai) this could just be a namespace
class ETSUtils:
    TEST_GENERATOR_REPORT = "test_generator_report.yaml"

    def are_tests_generated(self, generated_folder: Path) -> bool:
        report_path = generated_folder / self.TEST_GENERATOR_REPORT
        return os.path.isfile(report_path)

    def create_report(self, generated_folder: Path, tests: List[str]) -> None:
        report_path = os.path.join(generated_folder, self.TEST_GENERATOR_REPORT)
        report_obj = {}
        for test_full_name in tests:
            test_name = os.path.basename(test_full_name)
            dir_name = os.path.dirname(test_full_name)
            package_name = os.path.relpath(dir_name, generated_folder)
            package_name = package_name.replace("/", ".")
            if package_name not in report_obj:
                report_obj[package_name] = [test_name]
            else:
                report_obj[package_name].append(test_name)
        with os.fdopen(os.open(report_path, os.O_WRONLY | os.O_CREAT, 0o755), "w", encoding="utf-8") as file_handle:
            yaml.dump(report_obj, file_handle)
