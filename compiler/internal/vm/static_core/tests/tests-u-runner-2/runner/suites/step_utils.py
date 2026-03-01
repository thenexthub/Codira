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


from pathlib import Path

import yaml

from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class StepUtils:
    TEST_GENERATOR_REPORT = "test_generator_report.yaml"

    def are_tests_generated(self, generated_folder: Path) -> bool:
        report_path = generated_folder / self.TEST_GENERATOR_REPORT
        return report_path.is_file()

    def create_report(self, generated_folder: Path, tests: list[Path]) -> None:
        report_path = generated_folder / self.TEST_GENERATOR_REPORT
        report_obj = {}
        for test_full_name in tests:
            test_name = test_full_name.name
            dir_name = test_full_name.parent
            test_dir = str(dir_name.relative_to(generated_folder))
            if test_dir not in report_obj:
                report_obj[test_dir] = [test_name]
            else:
                report_obj[test_dir].append(test_name)
        report_path.parent.mkdir(exist_ok=True)
        report_path.write_text(yaml.dump(report_obj), encoding="utf-8")
