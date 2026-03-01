#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
from datetime import date
from os import path
from pathlib import Path
from typing import List, Optional, Dict, Any
import yaml

from runner.test_base import Test
from runner.reports.spec_node import SpecNode
from runner.reports.pdf_loader import PdfLoader
from runner.utils import write_2_file
from runner.logger import Log

_LOGGER = logging.getLogger("runner.reports.report_by_folders")
_DIR_PATTERN = re.compile(r"([0-9][0-9])\.(.*)")


class SpecReport:
    REPORT_DATE = "${Date}"
    REPORT_TEST_SUITE = "${TestSuite}"
    REPORT_EXCLUDED_HEADER = "${ExcludedHeader}"
    REPORT_EXCLUDED_DIVIDER = "${ExcludedDivider}"
    REPORT_RESULT = "${Result}"
    REPORT_SPEC_FILE = "${SpecFile}"
    REPORT_SPEC_CREATION_DATE = "${SpecCreationDate}"

    TEMPLATE = "spec_report_template.md"
    SPEC_CONFIG_FILE = "arkts-spec-config.yaml"
    EXCLUDED_HEADER = " Excluded after |"
    EXCLUDED_DIVIDER = "-------|"

    def __init__(
            self,
            results: List[Test],
            test_suite: str,
            report_path: Path,
            md_report: Optional[Path],
            yaml_report: Optional[Path],
            spec_file: Path,
    ):
        self.tests = results
        self.test_suite = test_suite
        self.report_file_md = (
            str(md_report) if md_report else path.join(report_path, f"{test_suite}_spec-report.md")
        )
        self.report_file_yaml = (
            str(yaml_report) if yaml_report else path.join(report_path, f"{test_suite}_spec-report.yaml")
        )
        self.spec_file = spec_file
        self.has_excluded = False

        # get config data
        fpath = Path(path.dirname(path.abspath(__file__)), self.SPEC_CONFIG_FILE)
        data = Path.read_text(fpath)
        self.config: Dict = yaml.safe_load(data)

        # get spec data
        pdf_loader = PdfLoader(spec_file, self.config).parse()
        self.spec: SpecNode = pdf_loader.get_root_node()
        self.spec_creation_date: str = pdf_loader.get_creation_date()

        # process test results
        self.__calculate()

    @staticmethod
    def obj_to_dict(dumper: yaml.SafeDumper, data: SpecNode) -> Any:
        return dumper.represent_dict(data.to_dict())

    @staticmethod
    def __fmt_str(src: str) -> str:
        """Escape some characters for Markdown document"""
        res = src.replace("\\", "\\\\")
        res = res.replace("|", "\\|")
        res = res.replace("_", "\\_")
        res = res.replace("<", "\\<")
        res = res.replace(">", "\\>")
        res = res.replace("*", "\\*")
        res = res.replace("`", "\\`")
        res = res.replace("{", "\\{")
        res = res.replace("}", "\\}")
        res = res.replace("[", "\\[")
        res = res.replace("]", "\\]")
        res = res.replace("(", "\\(")
        res = res.replace(")", "\\)")
        res = res.replace("#", "\\#")
        res = res.replace("+", "\\+")
        res = res.replace("-", "\\-")
        res = res.replace(".", "\\.")
        res = res.replace("!", "\\!")
        return res

    @staticmethod
    def __fmt_num(src: int) -> str:
        """Replace 0 with space"""
        return str(src) if src > 0 else ""

    def populate_report(self) -> None:
        self.populate_report_md()
        self.populate_report_yaml()

    def populate_report_md(self) -> None:
        template_path = path.join(path.dirname(path.abspath(__file__)), self.TEMPLATE)
        with open(template_path, "r", encoding="utf-8") as file:
            report = file.read()
        report = report.replace(self.REPORT_DATE, str(date.today()))
        report = report.replace(self.REPORT_TEST_SUITE, self.test_suite)
        report = report.replace(self.REPORT_SPEC_FILE, self.__fmt_str(str(self.spec_file)))
        report = report.replace(self.REPORT_SPEC_CREATION_DATE, self.spec_creation_date)

        if not self.has_excluded:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, "")
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, "")
        else:
            report = report.replace(self.REPORT_EXCLUDED_HEADER, self.EXCLUDED_HEADER)
            report = report.replace(self.REPORT_EXCLUDED_DIVIDER, self.EXCLUDED_DIVIDER)

        lines: List[str] = []
        self.__report_nodes(self.spec.children, lines)
        lines.append(self.__report_node(self.spec))  # root node - summary line at the bottom
        result = "\n".join(lines)
        report = report.replace(self.REPORT_RESULT, result)
        Log.default(_LOGGER, f"Spec coverage report is saved to '{self.report_file_md}'")
        write_2_file(self.report_file_md, report)

    def populate_report_yaml(self) -> None:
        res: dict = {
            'report_title': 'ArkTS Specification Coverage Report',
            'report_date': str(date.today()),
            'spec_file': str(self.spec_file),
            'spec_date': self.spec_creation_date,
            'test_suite': self.test_suite,
            'data': self.spec.to_dict()
        }
        yaml.SafeDumper.add_representer(SpecNode, SpecReport.obj_to_dict)
        with open(self.report_file_yaml, mode="wt", encoding="utf-8") as file:
            yaml.safe_dump(res, file, default_flow_style=False, sort_keys=False)

    def __calculate_one_test(self, test: Test) -> None:
        node = self.spec
        self.__update_counters(test, node)

        dir_parts = test.test_id.split("/")
        out_dir = ""
        last_node = node
        for dir_part in dir_parts:
            matches = _DIR_PATTERN.match(dir_part)
            if matches is None or len(matches.groups()) != 2:
                break
            num = int(matches.group(1))
            out_dir = dir_part if out_dir == "" else out_dir + "/" + dir_part
            for _ in range(len(node.children), num):
                SpecNode("?", "", "", node)
            node = node.children[num - 1]
            node.dir = out_dir
            self.__update_counters(test, node)
            last_node = node
        self.__update_last_node_counters(test, last_node)

    def __update_counters(self, test: Test, node: SpecNode) -> None:
        node.total_acc += 1
        if test.excluded:
            node.excluded_after_acc += 1
            self.has_excluded = True
        elif test.passed and not test.ignored:
            node.passed_acc += 1
        elif not test.passed and not test.ignored:
            node.failed_acc += 1
        elif test.passed and test.ignored:
            node.ignored_but_passed_acc += 1
        elif not test.passed and test.ignored:
            node.ignored_acc += 1

    def __update_last_node_counters(self, test: Test, node: SpecNode) -> None:
        node.total += 1
        if test.excluded:
            node.excluded_after += 1
            self.has_excluded = True
        elif test.passed and not test.ignored:
            node.passed += 1
        elif not test.passed and not test.ignored:
            node.failed += 1
        elif test.passed and test.ignored:
            node.ignored_but_passed += 1
        elif not test.passed and test.ignored:
            node.ignored += 1

    def __calculate(self) -> None:
        for test in self.tests:
            self.__calculate_one_test(test)

    def __report_node(self, node: SpecNode) -> str:
        title = self.__fmt_str(node.prefix + " " + node.title)
        dirs = self.__fmt_str(node.dir.replace("/", "\u200B/\u200B"))
        status = self.__fmt_str(node.status)
        total = self.__fmt_num(node.total_acc)
        passed = self.__fmt_num(node.passed_acc)
        failed = self.__fmt_num(node.failed_acc)
        ign_pas = self.__fmt_num(node.ignored_but_passed_acc)
        ignored = self.__fmt_num(node.ignored_acc)
        excluded = (" " + self.__fmt_num(node.excluded_after_acc) + " |") if self.has_excluded else ""
        return f"| {title} | {dirs} | {status} | {total} | {passed} | {failed} | {ign_pas} | {ignored} |{excluded}"

    def __report_nodes(self, nodes: List[SpecNode], lines: List[str]) -> None:
        for node in nodes:
            lines.append(self.__report_node(node))
            self.__report_nodes(node.children, lines)
