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

import logging
import subprocess
from copy import deepcopy
from os import path, remove, makedirs
from traceback import format_exc
from typing import List, Callable, Tuple, Optional
from unittest import TestCase
from pathlib import Path

from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, Params, TestReport
from runner.options.options_jit import JitOptions
from runner.test_base import Test

_LOGGER = logging.getLogger("runner.test_file_based")

ResultValidator = Callable[[str, str, int], bool]


class TestFileBased(Test):
    SEGFAULT_RETURN_CODE = [139, -11]
    ABORT_RETURN_CODE = [134, -6]
    IRTOC_ASSERT_RETURN_CODE = [133, -5]
    CTE_RETURN_CODE = 1

    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str) -> None:
        Test.__init__(self, test_env, test_path, flags, test_id)
        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind: Optional[FailKind] = None
        self.main_entry_point = "_GLOBAL::func_main_0"
        self.test_cli: List[str] = []

    @property
    def ark_extra_options(self) -> List[str]:
        return []

    @property
    def ark_timeout(self) -> int:
        return int(self.test_env.config.ark.timeout)

    @property
    def runtime_args(self) -> List[str]:
        return self.test_env.runtime_args

    @property
    def verifier_args(self) -> List[str]:
        return self.test_env.verifier_args

    def get_processes(self, pid: int, gdb_timeout: int) -> str:
        if not self.test_env.config.general.handle_timeout:
            return "There is a timeout failure. If you want to investigate threads, rerun with option " \
                   "--handle-timeout and under 'sudo'"

        cmd = [
            "gdb", "--batch", "-p", str(pid),
            "-ex", 'info threads',
            "-ex", 'thread apply all bt',
        ]
        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            try:
                output, error = process.communicate(timeout=gdb_timeout)
                return f"output: '{output}'\nerror: '{error}'"
            except subprocess.TimeoutExpired:
                process.kill()
                return "<no stacktrace information>"

    def detect_segfault(self, return_code: int, default_fail_kind: FailKind) -> FailKind:
        if return_code in self.SEGFAULT_RETURN_CODE:
            return FailKind.SEGFAULT_FAIL
        if return_code in self.ABORT_RETURN_CODE:
            return FailKind.ABORT_FAIL
        if return_code in self.IRTOC_ASSERT_RETURN_CODE:
            return FailKind.IRTOC_ASSERT_FAIL
        return default_fail_kind

    # pylint: disable=too-many-locals
    def run_one_step(self, name: str, params: Params, result_validator: ResultValidator, no_log: bool = False) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        coverage_per_binary = self.test_env.config.general.coverage.coverage_per_binary
        profraw_file, profdata_file, params = self.__get_prof_files(name, params)

        if self.test_env.config.general.coverage.use_lcov and coverage_per_binary:
            gcov_prefix, gcov_prefix_strip = self.coverage.lcov_tool.get_gcov_prefix(name)
            params = deepcopy(params)
            params.env['GCOV_PREFIX'] = gcov_prefix
            params.env['GCOV_PREFIX_STRIP'] = gcov_prefix_strip

        cmd = self.test_env.cmd_prefix + [params.executor]
        cmd.extend(params.flags)

        self.log_cmd(f"Run {name}: " + ' '.join(cmd))

        passed, output, fail_kind = False, "", None

        with subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=params.env,
                encoding='utf-8',
                errors='ignore',
        ) as process:
            try:
                output, error = process.communicate(timeout=params.timeout)
                return_code = process.returncode
                passed = result_validator(output, error, return_code)
                if not passed:
                    fail_kind = (self.detect_segfault(return_code, params.fail_kind_fail)
                                 if self.fail_kind != FailKind.COMPARE_OUTPUT_FAIL
                                 else self.fail_kind)
                    error = error.strip()
            except subprocess.TimeoutExpired:
                timeout_info = self.get_processes(process.pid, params.gdb_timeout)
                self.log_cmd(f"Failed by timeout after {params.timeout} sec\n{timeout_info}")
                fail_kind = params.fail_kind_timeout
                error = fail_kind.name
                return_code = process.returncode
                process.kill()
            except Exception:  # pylint: disable=broad-except
                self.log_cmd(f"Failed with {format_exc()}")
                fail_kind = params.fail_kind_other
                error = fail_kind.name
                return_code = -1

        if self.test_env.config.general.coverage.use_llvm_cov and profdata_file and profraw_file:
            self.coverage.llvm_cov_tool.merge_and_delete_prowraw_files(profraw_file, profdata_file)

        report = TestReport(output.strip(), error.strip(), return_code)

        if not no_log or report.error or report.return_code != 0:
            self.log_cmd(f"Output: '{report.output}'\nError: '{report.error}'\nReturn code: {report.return_code}")

        return passed, report, fail_kind

    def run_es2panda(self, flags: List[str], test_abc: str, result_validator: ResultValidator, no_log: bool = False) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = flags[:]
        es2panda_flags.append("--thread=0")
        if len(test_abc) > 0:
            es2panda_flags.append(f"--output={test_abc}")

        es2panda_flags.append(self.path)

        params = Params(
            executor=self.test_env.es2panda,
            flags=es2panda_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.es2panda.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.ES2PANDA_FAIL,
            fail_kind_timeout=FailKind.ES2PANDA_TIMEOUT,
            fail_kind_other=FailKind.ES2PANDA_OTHER,
        )

        return self.run_one_step("es2panda", params, result_validator, no_log)

    def run_runtime(self, test_an: str, test_abc: str, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        ark_flags: List[str] = []
        ark_flags.extend(self.ark_extra_options)
        ark_flags.extend(self.runtime_args)
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            ark_flags.extend(["--aot-files", test_an])

        if self.test_env.conf_kind == ConfigurationKind.JIT:
            ark_flags.extend([
                '--compiler-enable-jit=true',
                '--compiler-check-final=true'])
            jit_options: JitOptions = self.test_env.config.ark.jit
            if jit_options.compiler_threshold is not None:
                ark_flags.append(
                    f'--compiler-hotness-threshold={jit_options.compiler_threshold}'
                )
        else:
            ark_flags.extend(['--compiler-enable-jit=false'])

        if self.test_env.config.ark.interpreter_type is not None:
            ark_flags.extend([f'--interpreter-type={self.test_env.config.ark.interpreter_type}'])

        # Runtime treats the entry panda file as boot file if the option `--panda-files` was not specify.
        # Here we prevent this by manually adding `--panda-file` option with entry file
        panda_files_opt_name = '--panda-files'
        if not any(opt.startswith(panda_files_opt_name) for opt in ark_flags):
            ark_flags.append(f'{panda_files_opt_name}={test_abc}')

        ark_flags.extend([test_abc, self.main_entry_point])
        if self.test_cli:
            ark_flags.append('--')
            ark_flags.extend(self.test_cli)

        params = Params(
            timeout=self.ark_timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            executor=self.test_env.runtime,
            flags=ark_flags,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.RUNTIME_FAIL,
            fail_kind_timeout=FailKind.RUNTIME_TIMEOUT,
            fail_kind_other=FailKind.RUNTIME_OTHER,
        )

        return self.run_one_step("ark", params, result_validator)

    def run_aot(self, test_an: str, test_abcs: List[str], result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind]]:
        aot_flags = []
        aot_flags.extend(self.test_env.aot_args)
        aot_flags = [flag.strip("'\"") for flag in aot_flags]
        for test_abc in test_abcs:
            aot_flags.extend(['--paoc-panda-files', test_abc])
        aot_flags.extend(['--paoc-output', test_an])

        if path.isfile(test_an):
            remove(test_an)

        if self.test_env.ark_aot is not None:
            params = Params(
                timeout=self.test_env.config.ark_aot.timeout,
                gdb_timeout=self.test_env.config.general.gdb_timeout,
                executor=self.test_env.ark_aot,
                flags=aot_flags,
                env=self.test_env.cmd_env,
                fail_kind_fail=FailKind.AOT_FAIL,
                fail_kind_timeout=FailKind.AOT_TIMEOUT,
                fail_kind_other=FailKind.AOT_OTHER,
            )
            return self.run_one_step("ark_aot", params, result_validator)
        TestCase().assertFalse(self.test_env.ark_aot is None)
        return False, TestReport("", "", 0), FailKind.AOT_OTHER

    def run_ark_quick(self, flags: List[str], test_abc: str, result_validator: ResultValidator) \
            -> Tuple[bool, TestReport, Optional[FailKind], str]:
        quick_flags = flags[:]
        quick_flags.extend(self.test_env.quick_args)

        src_abc = test_abc
        root, ext = path.splitext(src_abc)
        dst_abc = f'{root}.quick{ext}'
        quick_flags.extend([src_abc, dst_abc])

        params = Params(
            timeout=self.ark_timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            executor=self.test_env.ark_quick,
            flags=quick_flags,
            env=self.test_env.cmd_env,
            fail_kind_fail=FailKind.QUICK_FAIL,
            fail_kind_timeout=FailKind.QUICK_TIMEOUT,
            fail_kind_other=FailKind.QUICK_OTHER,
        )

        return *(self.run_one_step("ark_quick", params, result_validator)), dst_abc

    def get_tests_abc(self) -> str:
        bytecode_path = self.test_env.work_dir.intermediate
        test_abc = path.join(bytecode_path, f"{self.test_id}.abc")
        makedirs(path.dirname(test_abc), exist_ok=True)
        return test_abc

    def __get_prof_files(
            self,
            name: str,
            params: Params
        ) -> Tuple[Optional[Path], Optional[Path], Params]:
        profraw_file, profdata_file = None, None
        if not self.test_env.config.general.coverage.use_llvm_cov:
            return profraw_file, profdata_file, params

        if self.test_env.config.general.coverage.coverage_per_binary:
            profraw_file, profdata_file = self.coverage.llvm_cov_tool.get_uniq_profraw_profdata_file_paths(name)
        else:
            profraw_file, profdata_file = self.coverage.llvm_cov_tool.get_uniq_profraw_profdata_file_paths()

        params = deepcopy(params)
        params.env['LLVM_PROFILE_FILE'] = str(profraw_file)

        return profraw_file, profdata_file, params
