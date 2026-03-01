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

import json
import logging
from os import path
from pathlib import Path
from typing import Set, List

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.logger import Log
from runner.options.config import Config
from runner.plugins.ets.ets_suites import EtsSuites
from runner.plugins.ets.ets_test_suite import EtsTestSuite
from runner.plugins.ets.test_ets import TestETS
from runner.plugins.ets.test_ets_ts_subset import TestTSSubset
from runner.runner_base import get_test_id
from runner.runner_file_based import RunnerFileBased
from runner.enum_types.test_directory import TestDirectory


_LOGGER = logging.getLogger("runner.plugins.ets.runner_ets")


class RunnerETSException(Exception):
    pass


class RunnerETS(RunnerFileBased):
    def __init__(self, config: Config):
        self.__ets_suite_name = self.get_ets_suite_name(config.test_suites)
        RunnerFileBased.__init__(self, config, self.__ets_suite_name)
        self.stdlib_path = path.join(self.build_dir, "plugins/ets/etsstdlib.abc")
        self.sdk_path = path.join(self.build_dir, "plugins/ets/sdk/etssdk.abc")
        if not path.exists(self.stdlib_path):
            self.stdlib_path = path.join(self.build_dir, "gen", "plugins/ets/etsstdlib.abc")  # for GN build

        self._check_binary_artifacts()

        self.test_env.es2panda_args.extend([
            f"--arktsconfig={self.arktsconfig}",
            "--gen-stdlib=false",
            "--extension=ets",
            f"--opt-level={self.config.es2panda.opt_level}"
        ])
        if self.config.es2panda.debug_info:
            self.test_env.es2panda_args.append("--debug-info")
        self.test_env.es2panda_args.extend(self.config.es2panda.es2panda_args)

        if self.__ets_suite_name == EtsSuites.SDK.value:
            boot_files = f"--boot-panda-files={self.stdlib_path}:{self.sdk_path}"
        else:
            boot_files = f"--boot-panda-files={self.stdlib_path}"
        load_runtime_ets = [boot_files, "--load-runtimes=ets"]
        self.test_env.runtime_args.extend(load_runtime_ets)
        self.test_env.runtime_args.append("--verification-mode=ahead-of-time")
        if self.__ets_suite_name == EtsSuites.SDK.value:
            self.test_env.runtime_args.append(f"--panda-files={self.sdk_path}")
            self.test_env.verifier_args.append(f"--panda-files={self.sdk_path}")
        self.test_env.verifier_args.extend(load_runtime_ets)
        if self.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.aot_args.extend(load_runtime_ets)

        test_suite_class = EtsTestSuite.get_class(self.__ets_suite_name)

        if self.__ets_suite_name == EtsSuites.RUNTIME.value:
            self.default_list_root = self._get_frontend_test_lists()

        test_suite = test_suite_class(self.config, self.work_dir, self.default_list_root)
        test_suite.process(self.config.general.generate_only or self.config.ets.force_generate)
        self.test_root, self.list_root = test_suite.test_root, test_suite.list_root

        self.explicit_list = self.recalculate_explicit_list(config.test_lists.explicit_list)

        Log.summary(_LOGGER, f"TEST_ROOT set to {self.test_root}")
        Log.summary(_LOGGER, f"LIST_ROOT set to {self.list_root}")

        suite_name = self.__ets_suite_name if self.__ets_suite_name != "ets-custom" else self.config.custom.suite_name
        self.collect_excluded_test_lists(test_name=suite_name)
        self.collect_ignored_test_lists(test_name=suite_name)

        self.add_directories([TestDirectory(self.test_root, "ets", [])])

    @property
    def default_work_dir_root(self) -> Path:
        return Path("/tmp") / "ets" / self.__ets_suite_name

    def create_test(self, test_file: str, flags: List[str], is_ignored: bool) -> TestETS:
        if self.__ets_suite_name != EtsSuites.TS_SUBSET.value:
            test = TestETS(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        else:
            test = TestTSSubset(self.test_env, test_file, flags, get_test_id(test_file, self.test_root))
        test.ignored = is_ignored
        return test

    def get_ets_suite_name(self, test_suites: Set[str]) -> str:
        name = ""
        if "ets_func_tests" in test_suites:
            name = EtsSuites.FUNC.value
        elif 'ets_cts' in test_suites:
            name = EtsSuites.CTS.value
        elif 'ets_runtime' in test_suites:
            name = EtsSuites.RUNTIME.value
        elif 'ets_gc_stress' in test_suites:
            name = EtsSuites.GCSTRESS.value
        elif 'ets_es_checked' in test_suites:
            name = EtsSuites.ESCHECKED.value
        elif 'ets_custom' in test_suites:
            name = EtsSuites.CUSTOM.value
        elif 'ets_ts_subset' in test_suites:
            name = EtsSuites.TS_SUBSET.value
        elif 'ets_sdk' in test_suites:
            name = EtsSuites.SDK.value
        else:
            Log.exception_and_raise(_LOGGER, f"Unsupported test suite: {self.config.test_suites}")
        return name

    def _check_binary_artifacts(self) -> None:
        stdlib_path_obj = Path(self.stdlib_path)

        if not stdlib_path_obj.is_file():
            Log.exception_and_raise(
                _LOGGER,
                f"Standard library at {self.stdlib_path} was not found",
                FileNotFoundError)

    # We use Log.exception_and_raise which throws exception. no need in return
    # pylint: disable=inconsistent-return-statements
    def _get_std_from_arktsconfig(self) -> str:
        with open(self.arktsconfig, encoding="utf-8") as file:
            arkconfig = json.load(file)
            try:
                r = arkconfig.get("compilerOptions").get("paths").get("std")[0]
                assert isinstance(r, str)
                return r
            except (AttributeError, KeyError, IndexError):
                Log.exception_and_raise(
                    _LOGGER,
                    "Incomplete arktsconfig.json file, path to std should look like: "
                    "{'compilerOptions': {'paths': {'std': ['/path/to/std']}}}")
