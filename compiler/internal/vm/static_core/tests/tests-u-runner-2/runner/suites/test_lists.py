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

import os
import re
from fnmatch import fnmatch
from pathlib import Path
from typing import cast

from dotenv import load_dotenv

from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.configuration_kind import (
    BuildSystem,
    BuildTypeKind,
    ConfigurationKind,
    SanitizerKind,
)
from runner.enum_types.params import TestEnv
from runner.logger import Log
from runner.options.step import Step, StepKind
from runner.utils import correct_path, detect_architecture, detect_operating_system

_LOGGER = Log.get_logger(__file__)

_GN_BUILD_PROPERTIES_FILE_NAME = "args.gn"
_CMAKE_BUILD_PROPERTIES_FILE_NAME = "CMakeCache.txt"


class TestLists:
    def __init__(self, list_root: Path, test_env: TestEnv, jit_repeats: int | None):
        load_dotenv()
        self.list_root = list_root
        self.config = test_env.config
        self.explicit_list: Path | None = (
            correct_path(self.list_root, self.config.test_suite.test_lists.explicit_list)
            if self.config.test_suite.test_lists.explicit_list is not None and self.list_root is not None
            else None
        )
        self.explicit_test: Path | None = None
        self.excluded_lists: list[Path] = []
        self.ignored_lists: list[Path] = []
        self.build_properties: list[str] = []
        self.build_system: BuildSystem = BuildSystem.CMAKE
        self.is_jit_with_repeats = jit_repeats is not None and jit_repeats > 0

        _LOGGER.default(f"Initialize TestLists: gn_build = {self.config.general.gn_build}")
        if self.config.general.gn_build:
            self.build_properties = self.gn_build_properties()
            self.build_system = BuildSystem.GN
        else:
            self.build_properties = self.cmake_build_properties()
        self.sanitizer = self.search_sanitizer()
        _LOGGER.default(f"Initialize TestLists: sanitizer = {self.sanitizer}")
        self.architecture = detect_architecture(self.config.general.qemu)
        _LOGGER.default(f"Initialize TestLists: architecture = {self.architecture}")
        self.operating_system = detect_operating_system()
        _LOGGER.default(f"Initialize TestLists: operating_system = {self.operating_system}")
        self.build_type = self.search_build_type()
        _LOGGER.default(f"Initialize TestLists: build_type = {self.build_type}")
        self.conf_kind = self.detect_conf()
        _LOGGER.default(f"Initialize TestLists: conf_kind = {self.conf_kind}")

    @staticmethod
    def matched_test_lists(test_lists: list[Path], extra_lists: list[Path]) -> list[Path]:
        found_test_lists = set()
        if not extra_lists:
            return []
        patterns = [pattern.as_posix() for pattern in extra_lists]
        for test_list in test_lists:
            test_list_name = test_list.name

            for pattern in patterns:
                if fnmatch(test_list_name, pattern) or fnmatch(test_list.as_posix(), pattern):
                    found_test_lists.add(test_list)
                    break

        return list(found_test_lists)

    @staticmethod
    def __search_option_in_list(option: str, arg_list: list[str] | None) -> list[str]:
        if arg_list is None:
            return []
        return [arg for arg in arg_list if arg.startswith(option)]

    @staticmethod
    def __to_bool(value: str | None) -> bool | None:
        if not value:
            return None
        bool_map = {
            "true": True, "on": True, "1": True,
            "false": False, "off": False, "0": False
        }
        result_value = value.strip().lower()
        if result_value not in bool_map:
            return None
        return bool_map[result_value]

    def collect_excluded_test_lists(self, extra_list: list[str] | None = None,
                                    test_name: str | None = None) -> None:
        self.excluded_lists.extend(self.collect_test_lists("excluded", extra_list, test_name))

    def collect_ignored_test_lists(self, extra_list: list[str] | None = None,
                                   test_name: str | None = None) -> None:
        exclude_test_lists = self.config.test_suite.test_lists.exclude_ignored_test_lists
        exclude_all_ignored_lists = self.config.test_suite.test_lists.exclude_ignored_tests

        if exclude_all_ignored_lists:
            self.excluded_lists.extend(self.collect_test_lists("ignored", extra_list, test_name))
        elif exclude_test_lists:

            self.excluded_lists.extend(self.collect_test_lists("ignored", list(exclude_test_lists),
                                                               test_name, filter_list=True))
            self.ignored_lists.extend(self.collect_test_lists("ignored", list(exclude_test_lists),
                                                              test_name, filter_list=False))
        else:
            self.ignored_lists.extend(self.collect_test_lists("ignored", extra_list, test_name))

    def collect_test_lists(
            self,
            kind: str, extra_lists: list[str] | None = None,
            test_name: str | None = None, filter_list: bool | None = None
    ) -> list[Path]:

        test_lists: list[Path] = []
        test_name = test_name if test_name else self.config.test_suite.suite_name

        short_template_name = f"{test_name}*-{kind}*.txt"
        conf_kind = self.conf_kind.value.upper()
        interpreter = self.get_interpreter().upper()
        full_template_name = f"{test_name}.*-{kind}" + \
                             f"(-{self.operating_system.value})?" \
                             f"(-{self.architecture.value})?" \
                             f"(-{conf_kind})?" \
                             f"(-{interpreter})?"
        if self.sanitizer != SanitizerKind.NONE:
            full_template_name += f"(-{self.sanitizer.value})?"
        if self.opt_level() is not None:
            full_template_name += f"(-OL{self.opt_level()})?"
        if self.debug_info():
            full_template_name += "(-DI)?"
        if self.is_full_ast_verifier():
            full_template_name += "(-FULLASTV)?"
        if self.is_simultaneous():
            full_template_name += "(-SIMULTANEOUS)?"
        if self.conf_kind == ConfigurationKind.JIT and self.is_jit_with_repeats:
            full_template_name += "(-(repeats|REPEATS))?"
        gc_type = cast(str, self.config.workflow.get_parameter('gc-type', 'g1-gc')).upper()
        full_template_name += f"(-({gc_type}))?"
        full_template_name += f"(-{self.build_type.value})?"
        full_template_name += ".txt"
        _LOGGER.default(f"Test lists searching: template = {full_template_name}")
        full_pattern = re.compile(full_template_name)

        def is_matched(file: Path) -> bool:
            file_name = file.name
            match = full_pattern.match(file_name)
            return match is not None

        glob_expression = f"**/{short_template_name}"
        test_lists.extend(filter(
            is_matched,
            self.list_root.glob(glob_expression)
        ))

        _LOGGER.all(f"Loading {kind} test lists: {test_lists}")

        if filter_list is not None and extra_lists:
            found_test_lists = TestLists.matched_test_lists(test_lists,
                                                            [Path(extra_list) for extra_list in extra_lists])
            if filter_list:
                return found_test_lists

            return list(set(test_lists) - set(found_test_lists))

        return test_lists

    def search_build_type(self) -> BuildTypeKind:
        if self.config.general.gn_build:
            is_debug = self.__search_in_build_properties('is_debug')
            is_fastverify = self.__search_in_build_properties('is_fastverify')
            if is_debug is not None:
                build_type = 'debug' if is_debug.lower() == 'true' else 'release'
                return BuildTypeKind.is_value(build_type, option_name=f"from gn 'is_debug={is_debug}'")
            if is_fastverify is not None and bool(is_fastverify):
                return BuildTypeKind.is_value('fast-verify', option_name="from gn 'is_fastverify=true'")
        cmake_build_type = cast(str, self.__search_in_build_properties('CMAKE_BUILD_TYPE'))
        if cmake_build_type == "fastverify":
            cmake_build_type = "fast-verify"
        return BuildTypeKind.is_value(cmake_build_type, option_name="from cmake CMAKE_BUILD_TYPE")

    def search_sanitizer(self) -> SanitizerKind:
        props = self.get_sanitizers_properties_names()

        is_ubsan = self.__to_bool(self.__search_in_build_properties(props["ubsan"]))
        is_asan = self.__to_bool(self.__search_in_build_properties(props["asan"]))
        is_tsan = self.__to_bool(self.__search_in_build_properties(props["tsan"]))

        if is_asan or is_ubsan:
            return SanitizerKind.ASAN
        if is_tsan:
            return SanitizerKind.TSAN
        return SanitizerKind.NONE

    def get_sanitizers_properties_names(self) -> dict[str, str]:
        sanitizer_depending_on_build_system = {
            BuildSystem.GN.value: {
                "ubsan": "is_ubsan",
                "asan": "is_asan",
                "tsan": "is_tsan",
            },
            BuildSystem.CMAKE.value: {
                "ubsan": "PANDA_ENABLE_UNDEFINED_BEHAVIOR_SANITIZER",
                "asan": "PANDA_ENABLE_ADDRESS_SANITIZER",
                "tsan": "PANDA_ENABLE_THREAD_SANITIZER",
            }
        }
        build_sys = self.build_system.value
        if build_sys not in sanitizer_depending_on_build_system:
            raise InvalidConfiguration(f"Unknown build system: {build_sys}")
        return sanitizer_depending_on_build_system[build_sys]

    def is_aot(self) -> Step | None:
        return next((step for step in self.config.workflow.steps if step.step_kind == StepKind.AOT), None)

    def is_aot_full(self, step: Step) -> bool:
        return len(self.__search_option_in_list("--compiler-inline-full-intrinsics=true", step.args)) > 0

    def is_aot_pgo(self, step: Step) -> bool:
        return len(self.__search_option_in_list("--paoc-use-profile:path=", step.args)) > 0

    def is_jit(self) -> bool:
        jit = str(self.config.workflow.get_parameter("compiler-enable-jit"))
        return jit.lower() == "true"

    def get_interpreter(self) -> str:
        result: list[str] | str | None = self.config.workflow.get_parameter("ark-args")
        ark_args: list[str] = [] if result is None else result if isinstance(result, list) else [result]
        is_int = self.__search_option_in_list("--interpreter-type", ark_args)
        if is_int:
            return is_int[0].split('=')[-1]
        return "default"

    def detect_conf(self) -> ConfigurationKind:
        if (step := self.is_aot()) is not None:
            if self.is_aot_full(step):
                return ConfigurationKind.AOT_FULL
            if self.is_aot_pgo(step):
                return ConfigurationKind.AOT_PGO
            return ConfigurationKind.AOT

        if self.is_jit():
            return ConfigurationKind.JIT

        return ConfigurationKind.INT

    def opt_level(self) -> int | None:
        level = self.config.workflow.get_parameter("opt-level")
        return int(level) if level is not None else None

    def debug_info(self) -> bool:
        debug_info_opt = "--debug-info"
        args = self.get_es2panda_args()
        if isinstance(args, str):
            return args == debug_info_opt
        return len(self.__search_option_in_list(debug_info_opt, args)) > 0

    def is_full_ast_verifier(self) -> bool:
        option = "--verifier-all-checks"
        args = self.get_es2panda_args()
        if isinstance(args, str):
            return args == option
        return len(self.__search_option_in_list(option, args)) > 0

    def is_simultaneous(self) -> bool:
        return "--simultaneous=true" in self.get_es2panda_args()

    def get_es2panda_args(self) -> list[str] | str:
        return args if (args := self.config.workflow.get_parameter("es2panda-args")) else []

    def gn_build_properties(self) -> list[str]:
        file_name = os.getenv('GN_BUILD_PROPERTIES_FILE_NAME', _GN_BUILD_PROPERTIES_FILE_NAME)
        return self.__get_build_properties_from_file(file_name)

    def cmake_build_properties(self) -> list[str]:
        file_name = os.getenv('CMAKE_BUILD_PROPERTIES_FILE_NAME', _CMAKE_BUILD_PROPERTIES_FILE_NAME)
        return self.__get_build_properties_from_file(file_name)

    def __search_in_build_properties(self, property_name: str) -> str | None:
        for prop in self.build_properties:
            if property_name in prop:
                match = re.match(fr'^({property_name})(?:[: \t=].*?[= \t:]| *= *)[ \t]?(.*)$', prop)
                if match:
                    return str(match.group(2).strip().lower())
        return None

    def __get_build_properties_from_file(self, file_name: str) -> list[str]:
        cache_file: Path = self.config.general.build / file_name
        if not cache_file.exists():
            raise InvalidConfiguration(
                f"Incorrect build folder {self.config.general.build}. Cannot find '{file_name}' file")
        with open(cache_file, encoding="utf-8") as file_handler:
            cache = [line.strip()
                     for line in file_handler.readlines()
                     if line.strip() and not line.strip().startswith("#") and not line.strip().startswith("//")]
            return sorted(cache)
