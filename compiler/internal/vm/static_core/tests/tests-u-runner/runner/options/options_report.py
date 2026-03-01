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

from functools import cached_property
from typing import Dict, Optional

from runner.options.decorator_value import value, _to_bool, _to_enum, _to_str, _to_path
from runner.reports.report_format import ReportFormat


class ReportOptions:
    __DEFAULT_REPORT_FORMAT = ReportFormat.LOG

    def __str__(self) -> str:
        return _to_str(self, 1)

    def to_dict(self) -> Dict[str, object]:
        return {
            "report-format": self.report_format.value.upper(),
            "detailed-report": self.detailed_report,
            "detailed-report-file": self.detailed_report_file,
            "spec-report": self.spec_report,
            "spec-report-file": self.spec_report_file,
            "spec-report-yaml": self.spec_report_yaml,
            "spec-file": self.spec_file
        }

    @cached_property
    @value(
        yaml_path="report.report-format",
        cli_name="report_formats",
        cast_to_type=lambda x: _to_enum(x, ReportFormat)
    )
    def report_format(self) -> ReportFormat:
        return ReportOptions.__DEFAULT_REPORT_FORMAT

    @cached_property
    @value(
        yaml_path="report.detailed-report",
        cli_name="detailed_report",
        cast_to_type=_to_bool
    )
    def detailed_report(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="report.detailed-report-file",
        cli_name="detailed_report_file",
        cast_to_type=_to_path
    )
    def detailed_report_file(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="report.spec-report",
        cli_name="spec_report",
        cast_to_type=_to_bool
    )
    def spec_report(self) -> bool:
        return False

    @cached_property
    @value(
        yaml_path="report.spec-report-file",
        cli_name="spec_report_file",
        cast_to_type=_to_path
    )
    def spec_report_file(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="report.spec-report-yaml",
        cli_name="spec_report_yaml",
        cast_to_type=_to_path
    )
    def spec_report_yaml(self) -> Optional[str]:
        return None

    @cached_property
    @value(
        yaml_path="report.spec-file",
        cli_name="spec_file",
        cast_to_type=_to_path
    )
    def spec_file(self) -> Optional[str]:
        return None

    def get_command_line(self) -> str:
        options = [
            f'--report-format={self.report_format.value}'
            if self.report_format != ReportOptions.__DEFAULT_REPORT_FORMAT else '',
            '--detailed-report' if self.detailed_report else '',
            f'--detailed-report-file={self.detailed_report_file}'
            if self.detailed_report_file else '',
            '--spec-report' if self.spec_report else '',
            f'--spec-report-file={self.spec_report_file}'
            if self.spec_report_file else '',
            f'--spec-report-yaml={self.spec_report_yaml}'
            if self.spec_report_yaml else '',
            f'--spec-file={self.spec_file}'
            if self.spec_file else ''
        ]
        return ' '.join(options)
