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
from os import path
from typing import Dict, Optional, Union, Any

from runner.enum_types.qemu import QemuKind
from runner.enum_types.verbose_format import VerboseKind, VerboseFilter
from runner.options.decorator_value import value, _to_qemu, _to_bool, _to_enum, _to_str, _to_path, _to_int, \
    _to_processes
from runner.options.options_coverage import CoverageOptions


# pylint: disable=too-many-public-methods
class GeneralOptions:
    __DEFAULT_PROCESSES = 1
    __DEFAULT_CHUNKSIZE = 32
    __DEFAULT_GC_TYPE = "g1-gc"
    __DEFAULT_GC_BOMBING_FREQUENCY = 0
    __DEFAULT_HEAP_VERIFIER = "fail_on_verification"
    __DEFAULT_VERBOSE = VerboseKind.NONE
    __DEFAULT_VERBOSE_FILTER = VerboseFilter.NEW_FAILURES
    __DEFAULT_QEMU = QemuKind.NONE
    __DEFAULT_GDB_TIMEOUT = 10
    __DEFAULT_HANDLE_TIMEOUT = False

    def __str__(self) -> str:
        return _to_str(self, 1)

    @property
    def qemu_cmd_line(self) -> str:
        qemu = ''
        if self.qemu == QemuKind.ARM64:
            qemu = '--arm64-qemu'
        elif self.qemu == QemuKind.ARM32:
            qemu = '--arm32-qemu'
        return qemu

    @staticmethod
    def __to_cli_with_default(
            option_name: str,
            option_value: Optional[Any],
            default_value: Optional[Any] = None) -> str:
        return f"{option_name}={option_value}" if option_value != default_value else ""

    @staticmethod
    def __to_cli(
            option_name: str,
            option_value: Optional[Union[str, bool]]) -> str:
        return f"{option_name}={option_value}" if option_value else ""

    @cached_property
    def chunksize(self) -> int:
        return GeneralOptions.__DEFAULT_CHUNKSIZE

    @cached_property
    def static_core_root(self) -> str:
        # This file is expected to be located at path:
        # $PANDA_SOURCE/tests/tests-u-runner/runner/options/options_general.py
        path_parts = __file__.split(path.sep)[:-5]
        return path.sep.join(path_parts)

    @cached_property
    @value(yaml_path="general.generate-config", cli_name="generate_config", cast_to_type=_to_path)
    def generate_config(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.processes", cli_name="processes", cast_to_type=_to_processes)
    def processes(self) -> int:
        return GeneralOptions.__DEFAULT_PROCESSES

    @cached_property
    @value(yaml_path="general.build", cli_name="build_dir", cast_to_type=_to_path, required=True)
    def build(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.test-root", cli_name="test_root", cast_to_type=_to_path)
    def test_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.list-root", cli_name="list_root", cast_to_type=_to_path)
    def list_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.work-dir", cli_name="work_dir", cast_to_type=_to_path)
    def work_dir(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.ets-stdlib-root", cli_name="ets_stdlib_root", cast_to_type=_to_path)
    def ets_stdlib_root(self) -> Optional[str]:
        return None

    @cached_property
    @value(yaml_path="general.show-progress", cli_name="progress", cast_to_type=_to_bool)
    def show_progress(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.gc_type", cli_name="gc_type")
    def gc_type(self) -> str:
        return GeneralOptions.__DEFAULT_GC_TYPE

    @cached_property
    @value(yaml_path="general.full-gc-bombing-frequency", cli_name="full_gc_bombing_frequency", cast_to_type=_to_int)
    def full_gc_bombing_frequency(self) -> int:
        return GeneralOptions.__DEFAULT_GC_BOMBING_FREQUENCY

    @cached_property
    @value(yaml_path="general.run_gc_in_place", cli_name="run_gc_in_place", cast_to_type=_to_bool)
    def run_gc_in_place(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.heap-verifier", cli_name="heap_verifier")
    def heap_verifier(self) -> str:
        return GeneralOptions.__DEFAULT_HEAP_VERIFIER

    @cached_property
    @value(
        yaml_path="general.verbose",
        cli_name="verbose",
        cast_to_type=lambda x: _to_enum(x, VerboseKind)
    )
    def verbose(self) -> VerboseKind:
        return GeneralOptions.__DEFAULT_VERBOSE

    @cached_property
    @value(
        yaml_path="general.verbose-filter",
        cli_name="verbose_filter",
        cast_to_type=lambda x: _to_enum(x, VerboseFilter)
    )
    def verbose_filter(self) -> VerboseFilter:
        return GeneralOptions.__DEFAULT_VERBOSE_FILTER

    @cached_property
    @value(yaml_path="general.force-download", cli_name="force_download", cast_to_type=_to_bool)
    def force_download(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.bco", cli_name="bco", cast_to_type=_to_bool)
    def bco(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="general.with_js", cli_name="with_js", cast_to_type=_to_bool)
    def with_js(self) -> bool:
        return True

    @cached_property
    @value(yaml_path="general.qemu", cli_name=["arm64_qemu", "arm32_qemu"], cast_to_type=_to_qemu)
    def qemu(self) -> QemuKind:
        return GeneralOptions.__DEFAULT_QEMU

    @cached_property
    @value(yaml_path="general.generate-only", cli_name="generate_only", cast_to_type=_to_bool)
    def generate_only(self) -> bool:
        return False

    @cached_property
    @value(yaml_path="general.gdb-timeout", cli_name="gdb_timeout", cast_to_type=_to_int)
    def gdb_timeout(self) -> int:
        return self.__DEFAULT_GDB_TIMEOUT

    @cached_property
    @value(yaml_path="general.handle-timeout", cli_name="handle_timeout", cast_to_type=_to_bool)
    def handle_timeout(self) -> bool:
        return self.__DEFAULT_HANDLE_TIMEOUT

    coverage = CoverageOptions()

    def get_command_line(self) -> str:
        options = [
            f'--build-dir="{self.build}"',
            self.__to_cli('--generate-config', self.generate_config),
            self.__to_cli_with_default('--processes', self.processes, GeneralOptions.__DEFAULT_PROCESSES),
            self.__to_cli_with_default('--chunksize', self.chunksize, GeneralOptions.__DEFAULT_CHUNKSIZE),
            self.__to_cli('--test-root', self.test_root),
            self.__to_cli('--list-root', self.list_root),
            self.__to_cli('--work-dir', self.work_dir),
            self.__to_cli('--ets-stdlib-root', self.ets_stdlib_root),
            self.__to_cli('--show-progress', self.show_progress),
            self.__to_cli_with_default('--gc-type', self.gc_type, GeneralOptions.__DEFAULT_GC_TYPE),
            self.__to_cli_with_default('--full-gc-bombing-frequency', self.full_gc_bombing_frequency,
                                       GeneralOptions.__DEFAULT_GC_BOMBING_FREQUENCY),
            self.__to_cli('--no-run-gc-in-place', self.run_gc_in_place),
            self.__to_cli_with_default('--heap-verifier', self.heap_verifier,
                                       GeneralOptions.__DEFAULT_HEAP_VERIFIER),
            self.__to_cli_with_default('--verbose', self.verbose, GeneralOptions.__DEFAULT_VERBOSE),
            self.__to_cli_with_default('--verbose-filter', self.verbose_filter,
                                       GeneralOptions.__DEFAULT_VERBOSE_FILTER),
            self.__to_cli('--force-download', self.force_download),
            self.__to_cli('--no-bco', not self.bco),
            self.__to_cli('--no-js', not self.with_js),
            self.__to_cli('--generate-only', self.generate_only),
            self.qemu_cmd_line,
            self.__to_cli_with_default('--gdb-timeout', self.gdb_timeout,
                                       GeneralOptions.__DEFAULT_GDB_TIMEOUT),
            self.__to_cli_with_default('--handle-timeout', self.handle_timeout,
                                       GeneralOptions.__DEFAULT_HANDLE_TIMEOUT),
            self.coverage.get_command_line(),
        ]
        return ' '.join(options)

    def to_dict(self) -> Dict[str, object]:
        return {
            "build": self.build,
            "processes": self.processes,
            "test-root": self.test_root,
            "list-root": self.list_root,
            "work-dir": self.work_dir,
            "ets-stdlib-root": self.ets_stdlib_root,
            "show-progress": self.show_progress,
            "gc_type": self.gc_type,
            "full-gc-bombing-frequency": self.full_gc_bombing_frequency,
            "run_gc_in_place": self.run_gc_in_place,
            "heap-verifier": self.heap_verifier,
            "verbose": self.verbose.value.upper(),
            "verbose-filter": self.verbose_filter.value.upper(),
            "coverage": self.coverage.to_dict(),
            "force-download": self.force_download,
            "bco": self.bco,
            "qemu": self.qemu.value.upper(),
            "with-js": self.with_js,
            "generate_only": self.generate_only,
            "gdb-timeout": self.gdb_timeout,
            "handle-timeout": self.handle_timeout
        }
