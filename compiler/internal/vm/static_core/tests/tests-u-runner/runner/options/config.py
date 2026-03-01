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

import argparse
import logging
import re
from functools import cached_property
from typing import Set, Dict

from runner.logger import Log
from runner.options.cli_args_wrapper import CliArgsWrapper
from runner.options.config_keeper import ConfigKeeper
from runner.options.decorator_value import value, _to_test_suites, _to_str
from runner.options.options_ark import ArkOptions
from runner.options.options_ark_aot import ArkAotOptions
from runner.options.options_custom import CustomSuiteOptions
from runner.options.options_es2panda import Es2PandaOptions
from runner.options.options_ets import ETSOptions
from runner.options.options_general import GeneralOptions
from runner.options.options_report import ReportOptions
from runner.options.options_quick import QuickOptions
from runner.options.options_test_lists import TestListsOptions
from runner.options.options_time_report import TimeReportOptions
from runner.options.options_verifier import VerifierOptions

_LOGGER = logging.getLogger("runner.options.config")


class Config:
    def __init__(self, args: argparse.Namespace):
        CliArgsWrapper.setup(args)
        ConfigKeeper.get().load_configs(args.configs)

    def __str__(self) -> str:
        return _to_str(self, 0)

    @staticmethod
    def log_warnings() -> None:
        for warning in ConfigKeeper.get().warnings():
            Log.summary(_LOGGER, warning)

    @cached_property
    @value(
        yaml_path="test-suites",
        cli_name=[
            "test_suites",
            "test262",
            "parser",
            "declgenparser",
            "hermes",
            "system",
            "astchecker",
            "srcdumper",
            "ets_func_tests",
            "ets_runtime",
            "ets_cts",
            "ets_gc_stress",
            "ets_es_checked",
            "ets_custom",
            "ets_ts_subset",
            "declgen_ets2ts_cts",
            "declgen_ets2ts_func_tests",
            "declgen_ets2ts_runtime",
            "declgen_ets2ts_sdk",
            "ets_sdk",
            "declgen_ts2ets_cts",
            "recheck",
            "declgen_ets2ets",
        ],
        cast_to_type=_to_test_suites,
        required=True,
    )
    def test_suites(self) -> Set[str]:
        return set([])

    general = GeneralOptions()
    report = ReportOptions()
    custom = CustomSuiteOptions()
    es2panda = Es2PandaOptions()
    verifier = VerifierOptions()
    quick = QuickOptions()
    ark_aot = ArkAotOptions()
    ark = ArkOptions()
    time_report = TimeReportOptions()
    test_lists = TestListsOptions()
    ets = ETSOptions()

    def generate_config(self) -> None:
        if self.general.generate_config is None:
            return
        data = self._to_dict()
        ConfigKeeper.get().save(self.general.generate_config, data)

    def get_command_line(self) -> str:
        _test_suites = ["--" + suite.replace("_", "-") for suite in self.test_suites]
        options = " ".join(
            [
                " ".join(_test_suites),
                self.general.get_command_line(),
                self.report.get_command_line(),
                self.es2panda.get_command_line(),
                self.verifier.get_command_line(),
                self.quick.get_command_line(),
                self.ark_aot.get_command_line(),
                self.ark.get_command_line(),
                self.time_report.get_command_line(),
                self.test_lists.get_command_line(),
                self.ets.get_command_line(),
            ]
        )
        options_str = re.sub(r"\s+", " ", options, re.IGNORECASE | re.DOTALL)

        return options_str

    def _to_dict(self) -> Dict[str, object]:
        return {
            "test-suites": list(self.test_suites),
            "custom": self.custom.to_dict(),
            "general": self.general.to_dict(),
            "report": self.report.to_dict(),
            "es2panda": self.es2panda.to_dict(),
            "verifier": self.verifier.to_dict(),
            "quick": self.quick.to_dict(),
            "ark_aot": self.ark_aot.to_dict(),
            "ark": self.ark.to_dict(),
            "time-report": self.time_report.to_dict(),
            "test-lists": self.test_lists.to_dict(),
            "ets": self.ets.to_dict(),
        }
