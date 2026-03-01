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

from __future__ import annotations
import re
from unittest import TestCase
from os import path, makedirs
from pathlib import Path
from typing import Tuple, Optional, Sequence, List, Dict

from runner.utils import compare_files, unlines
from runner.plugins.ets.ets_templates.test_metadata import get_metadata, TestMetadata
from runner.enum_types.configuration_kind import ConfigurationKind
from runner.enum_types.fail_kind import FailKind
from runner.enum_types.params import TestEnv, TestReport, Params
from runner.test_file_based import TestFileBased



class TestETS(TestFileBased):
    def __init__(self, test_env: TestEnv, test_path: str, flags: List[str], test_id: str,
                 parent_test_id: str = "") -> None:
        TestFileBased.__init__(self, test_env, test_path, flags, test_id)

        self.metadata: TestMetadata = get_metadata(Path(test_path))
        self.main_entry_point = f"{self.metadata.module}.ETSGLOBAL::{self.metadata.entry_point or 'main'}"
        self.test_cli: List[str] = self.metadata.test_cli or []
        package = self.metadata.get_package_name()
        # Defines if in dependent packages there is at least one file compile-only and negative
        self.dependent_packages: Dict[str, bool] = {
            package: self.is_negative_compile
        }

        # If test fails it contains reason (of FailKind enum) of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind = None

        # parent test id
        self.parent_test_id = parent_test_id

        self.bytecode_path = test_env.work_dir.intermediate
        makedirs(self.bytecode_path, exist_ok=True)
        self.test_abc = path.join(self.bytecode_path, f"{self.test_id}.abc")
        self.test_an = path.join(self.bytecode_path, f"{self.test_id}.an")
        self.test_expected = Path(test_env.work_dir.gen) / f"{self.test_id}.expected"
        self.test_expected_err = Path(test_env.work_dir.gen) / f"{self.test_id}.expected.err"
        makedirs(path.dirname(self.test_abc), exist_ok=True)

    @property
    def is_negative_runtime(self) -> bool:
        """ True if a test is expected to fail on ark """
        negative_runtime_metadata = self.metadata.tags.negative and not self.metadata.tags.compile_only
        return negative_runtime_metadata or path.basename(self.path).startswith("n.")

    @property
    def is_negative_compile(self) -> bool:
        """ True if a test is expected to fail on es2panda """
        return self.metadata.tags.negative and self.metadata.tags.compile_only

    @property
    def is_compile_only(self) -> bool:
        """ True if a test should be run only on es2panda """
        return self.metadata.tags.compile_only

    @property
    def is_valid_test(self) -> bool:
        """ True if a test is valid """
        return not self.metadata.tags.not_a_test

    @property
    def has_expected(self) -> bool:
        """ True if test.expected file exists or expected_out is in metadata """
        return self.metadata.expected_out is not None or path.isfile(self.test_expected)

    @property
    def has_expected_err(self) -> bool:
        """ True if test.expected.err file exists or expected_error is in metadata """
        return self.metadata.expected_error is not None or path.isfile(self.test_expected_err)

    @property
    def ark_extra_options(self) -> List[str]:
        return self.metadata.ark_options

    @property
    def ark_timeout(self) -> int:
        return self.metadata.timeout if self.metadata.timeout else super().ark_timeout

    @property
    def dependent_files(self) -> Sequence[TestETS]:
        if not self.metadata.files:
            return []

        tests = []
        for file in self.metadata.files:
            test_path = Path(self.path).parent / Path(file)
            current_test_id = Path(self.test_id)
            test = self.__class__(
                test_env=self.test_env,
                test_path=str(test_path),
                flags=self.flags,
                test_id=str(current_test_id.parent / Path(file)),
                parent_test_id=self.test_id)

            prefix = Path(self.parent_test_id).stem if self.parent_test_id else current_test_id.stem

            test_abc_name = f'{prefix}_{Path(test.test_abc).name}'
            test_an_name = f'{prefix}_{Path(test.test_an).name}'

            test.test_abc = str(Path(test.test_abc).parent / Path(test_abc_name))
            test.test_an = str(Path(test.test_abc).parent / Path(test_an_name))

            package = test.metadata.get_package_name()
            self.dependent_packages[package] = self.dependent_packages.get(package, False) or test.is_negative_compile
            if test.dependent_packages:
                for dep_key, dep_item in test.dependent_packages.items():
                    self.dependent_packages[dep_key] = self.dependent_packages.get(dep_key, False) or dep_item

            tests.append(test)
        return tests

    @property
    def runtime_args(self) -> List[str]:
        if not self.dependent_files:
            return super().runtime_args
        return self.add_panda_files(super().runtime_args)

    @property
    def verifier_args(self) -> List[str]:
        if not self.dependent_files:
            return super().verifier_args
        return self.add_panda_files(super().verifier_args)

    @staticmethod
    def _read_file(path_to_file: Path) -> str:
        with open(path_to_file, 'r', encoding='utf-8') as f:
            return ''.join(line for line in f if not line.startswith('#')).strip()

    @staticmethod
    def _normalize_error_report(report: str) -> str:
        pattern = r"\[TID \w{6,}\]\s*"
        result = re.sub(pattern, "", report)
        return TestETS._remove_tabs_and_spaces_from_begin(result)

    @staticmethod
    def _remove_tabs_and_spaces_from_begin(report: str) -> str:
        pattern = r"^\s+"
        return re.sub(pattern, "", report, flags=re.MULTILINE)

    @staticmethod
    def _remove_file_info_from_error(error_message: str) -> str:
        pattern = r'\s*[\[\(]\s*[^]\()]+\.ets:\d+:\d+\s*[\]\)]\s*|\s*[\[\(]\s*[^]\()]+\.abc\s*[\]\)]'
        return re.sub(pattern, '', error_message)

    def get_all_abc_dependent_files(self) -> List[str]:
        result: List[str] = []
        for dep_file in self.dependent_files:
            result.extend(dep_file.get_all_abc_dependent_files())
            result.append(dep_file.test_abc)

        return result

    def add_panda_files(self, args: List[str]) -> List[str]:
        opt_name = '--panda-files'
        met_panda_files_opt = False
        dep_files_args = []
        for arg in args:
            if opt_name in arg:
                met_panda_files_opt = True
                _, value = arg.split('=')
                opt_value = ":".join([value] + self.get_all_abc_dependent_files())
                arg = f'{opt_name}={opt_value}'
            dep_files_args.append(arg)
        # Add the option only in case of non-empty dependency files list
        if not met_panda_files_opt and len(self.dependent_files) > 0:
            opt_value = ":".join(self.get_all_abc_dependent_files())
            dep_files_args.append(f'{opt_name}={opt_value}')
        return dep_files_args

    def continue_after_process_dependent_files(self) -> bool:
        """
        Processes dependent files
        Returns True if to continue test run
        False - break test run
        """
        for test in self.dependent_files:
            dependent_result = test.do_run()
            self.reproduce += dependent_result.reproduce
            simple_failed = not dependent_result.passed
            negative_compile = dependent_result.passed and dependent_result.is_negative_compile
            dep_package = dependent_result.metadata.get_package_name()
            package_neg_compile = self.dependent_packages.get(dep_package, False)
            if simple_failed or negative_compile or package_neg_compile:
                self.passed = dependent_result.passed if not package_neg_compile else True
                self.report = dependent_result.report
                self.fail_kind = dependent_result.fail_kind
                return False
        return True

    # pylint: disable=too-many-return-statements
    def do_run(self) -> TestETS:
        if not self.continue_after_process_dependent_files():
            return self

        if not self.is_valid_test and self.is_compile_only:
            self.passed, self.report, self.fail_kind = self._run_compiler(self.test_abc)
            return self

        if self.test_env.config.ets.compare_files:
            return self._run_compare_mode()
        self.passed, self.report, self.fail_kind = self._run_compiler(self.test_abc)

        if not self.passed or (self.passed and self.is_compile_only):
            return self

        # Run verifier if required
        if self.test_env.config.verifier.enable:
            self.passed, self.report, self.fail_kind = self._run_verifier(self.test_abc)
            if not self.passed:
                return self

        # Run aot if required
        if self.test_env.conf_kind in [ConfigurationKind.AOT, ConfigurationKind.AOT_FULL]:
            self.passed, self.report, self.fail_kind = self.run_aot(
                self.test_an,
                self.get_all_abc_dependent_files() + [self.test_abc],
                lambda o, e, rc: rc == 0 and path.exists(self.test_an) and path.getsize(self.test_an) > 0
            )

            if not self.passed:
                return self

        self.passed, self.report, self.fail_kind = self.run_runtime(
            self.test_an,
            self.test_abc,
            self._runtime_result_validator)

        return self


    def compare_output_with_expected(self, output: str, err_output: str) -> Tuple[bool, Optional[FailKind]]:
        """Compares test output with .expected or .expected.err files"""
        fail_kind = None
        try:
            self._read_expected_file()
            if not self.test_env.config.ark.jit.enable or self.metadata.tags.no_warmup:
                passed = self._determine_test_status(output, err_output)
            else:
                self._refactor_expected_str_for_jit()
                passed = self._determine_test_status(output, err_output)

        except OSError:
            passed = False

        if not passed:
            fail_kind = FailKind.COMPARE_OUTPUT_FAIL
        return bool(passed), fail_kind

    def _determine_test_status(self, output: str, err_output: str) -> bool:

        output = output.strip()
        err_output = err_output.strip()

        def compare_line_sets(expected: str, actual: str, expected_path: Path) -> bool:
            expected_lines = set(filter(None, expected.splitlines()))
            actual_lines = set(filter(None, actual.splitlines()))

            if not actual_lines and not expected_lines:
                return True

            if not expected_lines:
                self.log_cmd(f"{expected_path} is empty after normalization")
                return False
            if not actual_lines:
                self.log_cmd(f"{expected_path} is not empty, but actual output is")
                return False

            is_subset = expected_lines.issubset(actual_lines)
            if not is_subset:
                self.log_cmd(f"{expected_path} is not a subset of actual, the missing lines (after normalization) are:")
                for line in expected_lines.difference(actual_lines):
                    self.log_cmd(line)
                self.log_cmd("(end of missing lines)")
                self.log_cmd("actual lines:")
                for line in actual_lines:
                    self.log_cmd(line)
                self.log_cmd("(end of actual lines)")

            return is_subset

        normalized_out = self.normalize_output(output)
        normalized_err = self.normalize_output(err_output)
        normalized_expected_out = self.normalize_output(self.expected)
        normalized_expected_err = self.normalize_output(self.expected_err)

        if self.expected and not self.expected_err and output:
            # Compare with output from std.OUT
            return compare_line_sets(normalized_expected_out,
                             normalized_out, self.test_expected) and not err_output
        if self.expected_err and not self.expected and err_output:
            # Compare with output from std.ERR
            return compare_line_sets(normalized_expected_err,
                             normalized_err, self.test_expected_err)
        if self.expected and self.expected_err and output and err_output:
            # Compare .expected file with std.OUT and .expected.err with std.ERR
            passed_stdout = compare_line_sets(normalized_expected_out,
                                    normalized_out, self.test_expected)
            passed_stderr = compare_line_sets(normalized_expected_err,
                                    normalized_err, self.test_expected_err)
            return passed_stdout and passed_stderr
        return True

    def normalize_output(self, output: str)-> str:
        return unlines(self.normalize_line(line) for line in output.splitlines())

    def normalize_line(self, line: str)-> str:
        return self._remove_file_info_from_error(self._normalize_error_report(line))

    def _read_expected_file(self) -> None:
        if self.has_expected:
            # __post_init__ converts list to str, isinstance narrows type for MyPy
            if (exp_out := self.metadata.expected_out) and isinstance(exp_out, str):
                self.expected = exp_out
            else:
                self.expected = self._read_file(self.test_expected)

        if self.has_expected_err:
            # __post_init__ converts list to str, isinstance narrows type for MyPy
            if (exp_err := self.metadata.expected_error) and isinstance(exp_err, str):
                self.expected_err = exp_err
            else:
                self.expected_err = self._read_file(self.test_expected_err)

    def _refactor_expected_str_for_jit(self) -> None:
        def _remove_after_main(s: str) -> str:
            index_to_delete = len(s.split("\n"))
            if index_to_delete > 1:
                expected = s.split("\n")
                for i, item in enumerate(expected):
                    if '.main' in item or ':main' in item:
                        index_to_delete = i
                        break
                s = '\n'.join(expected[:index_to_delete])
            else:
                s = s.replace('main', 'main_run')

            return s

        if self.expected:
            self.expected = _remove_after_main(self.expected)

        if self.expected_err:
            self.expected_err = _remove_after_main(self.expected_err)

    def _run_compare_mode(self) -> TestETS:
        files = []
        iterations = self.test_env.config.ets.compare_files_iterations
        for i in range(iterations):
            if i != 0:
                test_ets = self.bytecode_path / self.test_id
                stem, suffix_ets = test_ets.stem, test_ets.suffix
                suffix_abc = ".abc"
                test_abc = test_ets.parent / Path(f'{stem}_{str(i + 1)}{suffix_ets}{suffix_abc}')
            else:
                test_abc = Path(self.test_abc)

            files.append(test_abc)
            self.passed, self.report, self.fail_kind = self._run_compiler(str(test_abc))
            if not self.passed:
                return self

        self.passed = compare_files(files)
        if not self.passed:
            self.fail_kind = FailKind.COMPARE_FAIL
        return self

    def _runtime_result_validator(self, output: str, err: str, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """

        if (not self.is_negative_runtime and return_code == 0) or (self.is_negative_runtime and return_code != 0):
            if self.has_expected or self.has_expected_err:
                comparison_res, self.fail_kind = self.compare_output_with_expected(output, err)

                if not comparison_res:
                    self.log_cmd("The comparison with .expected or .expected.err file failed.")
                    return False

                return True

            if not err:
                # for passed tests stderr should be empty or there should be .expected.err file to compare with
                return True

            self.log_cmd("The test has passed but stdError is not empty. "
                         "There is no .expected.err file to compare with stdError output.")
            return False

        return False

    def _ts_node_result_validator(self, return_code: int) -> bool:
        """
        :return: True if test is successful, False if failed
        """
        if self.is_negative_runtime:
            return return_code != 0

        return return_code == 0


    def _run_compiler(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        es2panda_flags = []
        es2panda_flags.extend(self.test_env.es2panda_args)
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

        passed, report, fail_kind = self.run_one_step(
            name="es2panda",
            params=params,
            result_validator=lambda output, error, rc: self._validate_compiler(output, error, rc, test_abc)
        )
        if fail_kind == FailKind.ES2PANDA_FAIL and report.return_code == 0:
            fail_kind = FailKind.ES2PANDA_NEG_FAIL
        return passed, report, fail_kind


    def update_expected(self, expected_path: Path, contents: str) -> None:
        self.log_cmd(f"Updating expected file: {expected_path}")
        with open(expected_path, "w", encoding="utf-8") as expected_file:
            expected_file.write(contents)


    def _validate_compiler(self, output: str, err: str, return_code: int, output_path: str) -> bool:
        dep_package = self.metadata.get_package_name()
        package_compile = self.dependent_packages.get(dep_package, False)
        negative_test_passed = False
        test_passed = False
        if self.is_negative_compile or package_compile:
            negative_test_passed = return_code == self.CTE_RETURN_CODE
        else:
            test_passed = return_code == 0 and path.exists(output_path) and path.getsize(output_path) > 0

        if self.should_update_expected:
            if self.has_expected:
                self.update_expected(self.test_expected, output)
            # NOTE(pronai) #29808 might affect this
            # positive tests can have warnings!
            if self.is_negative_compile:
                # negative tests must have an .expected.err
                self.update_expected(self.test_expected_err, err)

        if not (negative_test_passed or test_passed):
            return False

        if self.has_expected or self.has_expected_err:
            comparison_res, self.fail_kind = self.compare_output_with_expected(output, err)

            if not comparison_res:
                self.log_cmd("The comparison with .expected or .expected.err file failed.")
                return False
            return True

        # NOTE(pronai) #29808 might affect this
        if not err:
            # for passed tests stderr should be empty or there should be .expected.err file to compare with
            return True
        return False


    def _run_verifier(self, test_abc: str) -> Tuple[bool, TestReport, Optional[FailKind]]:
        TestCase().assertTrue(
            path.exists(self.test_env.verifier),
            f"Verifier binary '{self.test_env.verifier}' is absent or not set"
        )
        config_path = self.test_env.config.verifier.config
        if config_path is None:
            config_path = path.join(path.dirname(__file__), 'ets-verifier.config')

        verifier_flags = list(self.verifier_args)
        verifier_flags.append(f"--config-file={config_path}")
        verifier_flags.append(test_abc)

        params = Params(
            executor=self.test_env.verifier,
            flags=verifier_flags,
            env=self.test_env.cmd_env,
            timeout=self.test_env.config.verifier.timeout,
            gdb_timeout=self.test_env.config.general.gdb_timeout,
            fail_kind_fail=FailKind.VERIFIER_FAIL,
            fail_kind_timeout=FailKind.VERIFIER_TIMEOUT,
            fail_kind_other=FailKind.VERIFIER_OTHER,
        )
        return self.run_one_step(
            name="verifier",
            params=params,
            result_validator=lambda _, _2, rc: rc == 0,
        )
