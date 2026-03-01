#!/usr/bin/env python3
# -- coding: utf-8 --
#
# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
import copy
import os
import re
from collections.abc import Sequence
from copy import deepcopy
from fnmatch import translate
from pathlib import Path
from typing import TYPE_CHECKING

from typing_extensions import Self

from runner import utils
from runner.common_exceptions import InvalidConfiguration
from runner.enum_types.params import BinaryParams, TestEnv, TestReport
from runner.enum_types.validation_result import ValidationResult, ValidatorFailKind
from runner.extensions.flows.test_flow_registry import ITestFlow
from runner.extensions.validators.base_validator import BaseValidator
from runner.extensions.validators.ivalidator import IValidator
from runner.logger import Log
from runner.options.macros import Macros, ParameterNotFound
from runner.options.options import IOptions
from runner.options.step import Step, StepKind
from runner.suites.one_test_runner import OneTestRunner
from runner.suites.test_metadata import TestMetadata
from runner.test_base import Test
from runner.utils import get_class_by_name, to_bool

if TYPE_CHECKING:
    pass

_LOGGER = Log.get_logger(__file__)

IS_PANDA = "is-panda"


class ComparisonUtils:
    @staticmethod
    def compare_line_sets(expected: str, actual: str, expected_path: Path) -> bool:
        expected_lines = set(filter(None, expected.splitlines()))
        actual_lines = set(filter(None, actual.splitlines()))

        if not actual_lines and not expected_lines:
            return True

        if not expected_lines:
            _LOGGER.all(f"{expected_path} is empty after normalization")
            return False
        if not actual_lines:
            _LOGGER.all(f"{expected_path} is not empty, but actual output is")
            return False

        is_subset = expected_lines.issubset(actual_lines)
        return is_subset

    @staticmethod
    def normalize_output_lines(output: str, expected: str) -> tuple[str, str]:
        norm_output = ''.join(ComparisonUtils.normalize_line(line) + '\n' for line in output.splitlines())
        norm_output_err = ''.join(ComparisonUtils.normalize_line(line) + '\n' for line in expected.splitlines())
        return norm_output, norm_output_err

    @staticmethod
    def normalize_line(line: str) -> str:
        return ComparisonUtils._remove_file_info_from_error(ComparisonUtils._normalize_error_report(line))

    @staticmethod
    def log_comparison_difference(actual_output: str | None, expected_output: str | None) -> list[str]:
        def find_difference(expected: str | None, actual: str | None) -> set[str]:
            if expected is None or actual is None:
                return set()
            expected_lines = set(filter(None, expected.splitlines()))
            actual_lines = set(filter(None, actual.splitlines()))
            return actual_lines.difference(expected_lines)

        differences = []

        dif_exp_output = find_difference(actual_output, expected_output)
        if dif_exp_output:
            differences.append(f"Actual output doesn't contain expected lines: \n{dif_exp_output}\n")

        return differences

    @staticmethod
    def _normalize_error_report(report: str) -> str:
        pattern = r"\[TID [0-9a-fA-F]{6,}\]\s*"
        result = re.sub(pattern, "", report).strip()
        return ComparisonUtils._remove_tabs_and_spaces_from_begin(result)

    @staticmethod
    def _remove_tabs_and_spaces_from_begin(report: str) -> str:
        pattern = r"^\s+"
        return re.sub(pattern, "", report, flags=re.MULTILINE)

    @staticmethod
    def _remove_file_info_from_error(error_message: str) -> str:
        pattern = r'\s*[\[\(]\s*[^]\()]+\.ets:\d+:\d+\s*[\]\)]\s*|\s*[\[\(]\s*[^]\()]+\.abc\s*[\]\)]\s*'
        return re.sub(pattern, '', error_message)


class StandardFlowUtils:

    def __init__(self, test_env: TestEnv):
        self.test_env = test_env

    def get_step_env(self, step: Step) -> dict[str, str]:
        cmd_env = self.test_env.cmd_env
        if step.env:
            cmd_env = deepcopy(self.test_env.cmd_env)
            for env_item in step.env:
                env_value: str | list[str] = step.env[env_item]
                if isinstance(env_value, list):
                    cmd_env[env_item] = "".join(env_value)
                else:
                    cmd_env[env_item] = env_value
        return cmd_env


class TestStandardFlow(ITestFlow, Test):
    __DEFAULT_ENTRY_POINT = "ETSGLOBAL::main"
    CTE_RETURN_CODE = 1
    metadata: TestMetadata
    flow_utils: StandardFlowUtils
    comparison_utils: ComparisonUtils = ComparisonUtils()

    def __init__(self, test_env: TestEnv, test_path: Path, *,
                 params: IOptions, test_id: str, is_dependent: bool = False,
                 parent_test_id: str = "") -> None:

        Test.__init__(self, test_env, test_path, params, test_id)
        self.metadata = TestMetadata.get_metadata(test_path) if test_env.config.test_suite.use_metadata \
            else TestMetadata.create_empty_metadata(test_path)
        self._init_bytecode_paths(test_env)

        self.test_cli: list[str] = self.metadata.test_cli or []
        self.main_entry_point: str = (
            f"ETSGLOBAL::{self.metadata.entry_point}"
            if self.metadata.entry_point else self.__DEFAULT_ENTRY_POINT
        )
        if self.metadata.package is not None:
            self.main_entry_point = f"{self.metadata.package}.{self.main_entry_point}"
        else:
            self.main_entry_point = f"{test_path.stem}.{self.main_entry_point}"

        # Defines if in dependent packages there is at least one file compile-only and negative
        self.dependent_packages: dict[str, bool] = {
            'package': self.is_negative_compile
        }

        self.parent_test_id = parent_test_id

        self.is_panda = to_bool(test_env.config.workflow.parameters.get(IS_PANDA, False))

        # If test fails it contains reason of first failed step
        # It's supposed if the first step is failed then no step is executed further
        self.fail_kind: str | None = None

        self.expected_err_log = ""
        self.flow_utils = StandardFlowUtils(test_env)
        self.validator_utils = ValidatorUtils(self, test_env)
        self.runtime_steps = [step for step in self.test_env.config.workflow.steps
                                if step.step_kind not in (StepKind.COMPILER, StepKind.VERIFIER)]
        self._dependent_tests: list[TestStandardFlow] = []
        self.__is_dependent = is_dependent
        self.__boot_panda_files: str = ""
        self.validator_utils.add_additional_validator()

    @property
    def direct_dependent_tests(self) -> list['TestStandardFlow']:
        if not self.metadata.files:
            return []
        for file in self.metadata.files:
            if file.endswith(".d.ets"):
                raise InvalidConfiguration(f"Test {self.test_id} has a dependent .d.ets file")
        id_pos: int = str(self.path).find(self.test_id)
        test_root: str = str(self.path)[:id_pos]

        dependent_tests: list[TestStandardFlow] = []

        for file in self.metadata.files:
            new_test_file_path: Path = self.path.parent.joinpath(file).resolve()
            new_test_id: Path = new_test_file_path.relative_to(test_root)
            if new_test_id in dependent_tests:
                continue
            test = self.__class__(self.test_env, new_test_file_path,
                                  params=self.test_extra_params,
                                  test_id=str(new_test_id),
                                  is_dependent=True,
                                  parent_test_id=self.test_id)
            dependent_tests.append(test)

        return dependent_tests

    @property
    def is_negative_runtime(self) -> bool:
        """ True if a test is expected to fail on ark """
        negative_runtime_metadata = self.metadata.tags.negative and not self.metadata.tags.compile_only
        return negative_runtime_metadata or self.path.stem.startswith("n.")

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
    def dependent_tests(self) -> Sequence['TestStandardFlow']:
        if not self.metadata.files:
            return []

        if self._dependent_tests:
            return self._dependent_tests

        self._dependent_tests.extend(self.direct_dependent_tests)
        current_test_id: Path = Path(self.test_id)
        for test in self._dependent_tests:
            prefix = Path(self.parent_test_id).stem if self.parent_test_id else current_test_id.stem

            test_abc_name = f'{prefix}_{Path(test.test_abc).name}'
            test_an_name = f'{prefix}_{Path(test.test_an).name}'

            test.test_abc = Path(test.test_abc).parent / Path(test_abc_name)
            test.test_an = Path(test.test_abc).parent / Path(test_an_name)

            os.makedirs(str(test.test_abc.parent), exist_ok=True)

            package = test.metadata.get_package_name()
            self.dependent_packages[package] = self.dependent_packages.get(package, False) or test.is_negative_compile
            if test.dependent_packages:
                for dep_key, dep_item in test.dependent_packages.items():
                    self.dependent_packages[dep_key] = self.dependent_packages.get(dep_key, False) or dep_item
        return self._dependent_tests

    @property
    def dependent_abc_files(self) -> list[str]:
        abc_files_lists = [[*df.dependent_abc_files, df.test_abc.as_posix()] for df in self.dependent_tests]
        result = []
        for abc_files_list in abc_files_lists:
            result += abc_files_list
        return list(result)

    @property
    def invalid_tags(self) -> list:
        return self.metadata.tags.invalid_tags

    @property
    def has_expected(self) -> bool:
        """ True if test.expected file exists or expected_out is in metadata """
        return super().has_expected or self.metadata.expected_out is not None

    @property
    def has_expected_err(self) -> bool:
        """ True if test.expected.err file exists or expected_error is in metadata """
        return super().has_expected_err or self.metadata.expected_error is not None

    @staticmethod
    def __add_options(options: list[str]) -> list[str]:
        for index, option in enumerate(options):
            if not option.startswith("--"):
                options[index] = f"--{option}"
        return options

    @staticmethod
    def _get_return_code_from_device(output: str, actual_return_code: int) -> int:
        if output.find('TypeError:') > -1 or output.find('FatalOutOfMemoryError:') > -1:
            return actual_return_code if actual_return_code else -1
        match = re.search(r'Exit code:\s*(-?\d+)', output)
        if match:
            return_code_from_device = int(match.group(1))
            return return_code_from_device
        return actual_return_code

    @staticmethod
    def _remove_main_decl(lines: str) -> str:
        expected = lines.split("\n")
        index_to_delete = len(expected)
        for i, item in enumerate(expected):
            if '.main' in item or ':main' in item:
                index_to_delete = i
                break

        return '\n'.join(expected[:index_to_delete])

    def do_run(self) -> Self:
        if not self._continue_after_process_dependent_files():
            return self

        self._log_invalid_tags_if_any()
        compile_only_test = self._is_compile_only_test()
        allowed_steps = [StepKind.COMPILER]  # steps to run for compile only or not-a-test tests
        steps = [step for step in self.test_env.config.workflow.steps
                 if step.executable_path is not None and
                 ((compile_only_test and step.step_kind in allowed_steps) or not compile_only_test)]
        for step in steps:
            pattern = re.compile(translate(step.step_filter))
            if not pattern.search(str(self.path)):
                continue
            self.passed, self.report, self.fail_kind = self._do_run_one_step(step)
            if step.step_kind in allowed_steps:
                allowed_steps.remove(step.step_kind)
            if not self.passed or (compile_only_test and not allowed_steps):
                return self

        if not steps:
            # no step runs, so nothing bad occurs, and we consider the test is passed
            self.passed = True
            self.fail_kind = 'PASSED'

        return self

    def prepare_compiler_step(self, step: Step) -> Step:
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        if self.__is_dependent:
            new_step.args = self.__change_output_arg(step.args, new_step.args)
            new_step.args = self.__change_arktsconfig_arg(step.args, new_step.args)
            return new_step
        new_step.args = self.__change_arktsconfig_arg(step.args, new_step.args)
        return new_step

    def prepare_verifier_step(self, step: Step) -> Step:
        if self.dependent_tests and self.is_panda:
            new_step = copy.copy(step)
            new_step.args = step.args[:]
            new_step.args = self.__add_boot_panda_files(new_step.args)
            return new_step
        return step

    def prepare_aot_step(self, step: Step) -> Step:
        if not self.is_panda:
            return step
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        if self.dependent_tests:
            for abc_file in list(self.dependent_abc_files):
                new_step.args.extend([f'--paoc-panda-files={abc_file}'])
        new_step.args.extend([f'--paoc-panda-files={self.test_abc}'])
        return new_step

    def prepare_runtime_step(self, step: Step) -> Step:
        if not self.is_panda:
            return step
        new_step = copy.copy(step)
        new_step.args = step.args[:]
        new_step.args.insert(-2, "--verification-mode=ahead-of-time")

        new_step.args.insert(-2, self.__add_panda_files())
        if self.metadata.test_cli:
            new_step.args.append("--")
            new_step.args.extend(self.metadata.test_cli)

        return new_step

    def compare_with_stdout(self, output: str) -> bool:
        """
        Compare test stdout with expected output
        """
        try:
            self._get_expected_data()
            passed = self._determine_test_status(output, self.expected)

        except OSError:
            _LOGGER.all("Failed to read expected data")
            passed = False

        return passed

    def compare_with_stderr(self, error: str) -> bool:
        """
        Compare test stderr with expected output
        """
        try:
            self._get_expected_data()
            passed = self._determine_test_status(error, self.expected_err)

        except OSError:
            _LOGGER.all("Failed to read expected data")
            passed = False

        return passed

    def _continue_after_process_dependent_files(self) -> bool:
        """
        Processes dependent files
        Returns True if to continue test run
        False - break test run
        """
        for test in self.dependent_tests:
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
                self.last_failure_check_passed = dependent_result.last_failure_check_passed
                return False
        return True

    def _log_invalid_tags_if_any(self) -> None:
        if len(self.invalid_tags) > 0:
            Log.default(
                _LOGGER,
                f"\n{utils.FontColor.RED_BOLD.value}Invalid tags:{utils.FontColor.RESET.value} `"
                f"{', '.join(self.invalid_tags)}` in test file: {self.test_id}")

    def _is_compile_only_test(self) -> bool:
        return self.is_compile_only or self.metadata.tags.not_a_test or self.parent_test_id != ""

    def _get_expected_data(self) -> None:
        if self.has_expected:
            self.expected = self.metadata.expected_out or utils.read_expected_file(self.path_to_expected)

        if self.has_expected_err:
            self.expected_err = self.metadata.expected_error or utils.read_expected_file(self.path_to_expected_err)

    def _refactor_expected_str_for_jit(self) -> None:
        if self.expected:
            self.expected = TestStandardFlow._remove_main_decl(self.expected)

        if self.expected_err:
            self.expected_err = TestStandardFlow._remove_main_decl(self.expected_err)

    def _determine_test_status(self, actual_output: str, expected_output: str) -> bool:
        actual_output, expected_output = ComparisonUtils.normalize_output_lines(actual_output, expected_output)
        passed = ComparisonUtils.compare_line_sets(expected_output, actual_output, self.path_to_expected_err)

        if passed:
            self.expected_err_log = "Comparison with expected output/error has passed\n"
        else:
            self.expected_err_log = "Comparison with expected output/error has failed\n"
            differences = ComparisonUtils.log_comparison_difference(actual_output, expected_output)
            self.expected_err_log += "\n".join(differences)

        return passed

    def _check_expected_err(self, error: str | None) -> bool:
        # to cover case: there is no expected_err file, but stderr is not empty
        # or there is expected err file, but it's empty
        passed = True
        if (not self.has_expected_err and error) or (not self.expected_err and error):
            passed = False

        return passed

    def _do_run_one_step(self, step: Step) -> tuple[bool, TestReport | None, str | None]:
        if not step.enabled:
            passed, report, fail_kind = True, None, None
        elif step.step_kind == StepKind.COMPILER:
            passed, report, fail_kind = self.__run_step(self.prepare_compiler_step(step))
        elif step.step_kind == StepKind.VERIFIER:
            passed, report, fail_kind = self.__run_step(self.prepare_verifier_step(step))
        elif step.step_kind == StepKind.AOT:
            passed, report, fail_kind = self.__run_step(self.prepare_aot_step(step))
        elif step.step_kind == StepKind.RUNTIME:
            passed, report, fail_kind = self.__run_step(self.prepare_runtime_step(step))
        else:
            passed, report, fail_kind = self.__run_step(step)
        return passed, report, fail_kind

    def _make_params(self, step: Step, cmd_env: dict[str, str]) -> BinaryParams:
        assert step.executable_path is not None
        params = BinaryParams(
            executor=step.executable_path,
            flags=self.__expand_last_call_macros(step),
            env=cmd_env,
            timeout=step.timeout,
            use_qemu=not step.skip_qemu
        )
        return params

    def _init_bytecode_paths(self, test_env: TestEnv) -> None:
        self.bytecode_path: Path = test_env.work_dir.intermediate
        self.test_abc: Path = self.bytecode_path / f"{self.test_id}.abc"
        self.test_an: Path = self.bytecode_path / f"{self.test_id}.an"
        self.test_abc.parent.mkdir(parents=True, exist_ok=True)

    def __fix_entry_point(self, args: list[str]) -> list[str]:
        result: list[str] = args[:]
        for index, arg in enumerate(result):
            if self.__DEFAULT_ENTRY_POINT in str(arg):
                result[index] = arg.replace(self.__DEFAULT_ENTRY_POINT, self.main_entry_point).strip()
        return [res for res in result if res]

    def __change_output_arg(self, source_args: list[str], new_args: list[str]) -> list[str]:
        for index, arg in enumerate(source_args):
            if arg.startswith("--output="):
                new_args[index] = f"--output={self.test_abc}"
                break
        return new_args

    def __change_arktsconfig_arg(self, source_args: list[str], new_args: list[str]) -> list[str]:
        for index, arg in enumerate(source_args):
            if arg.startswith("--arktsconfig=") and self.metadata.arktsconfig is not None:
                stdlib_path = self.test_env.config.general.static_core_root / 'plugins' / 'ets' / 'stdlib'
                new_args[index] = f"--arktsconfig={self.metadata.arktsconfig}"
                new_args.insert(0, f"--stdlib={stdlib_path.as_posix()}")
                break
        return new_args

    def __run_step(self, step: Step) -> tuple[bool, TestReport, str | None]:
        cmd_env = self.flow_utils.get_step_env(step)

        params = self._make_params(step, cmd_env)

        test_runner = OneTestRunner(self.test_env)
        passed, report, fail_kind = test_runner.run_with_coverage(
            name=step.name,
            step_kind=step.step_kind,
            params=params,
            result_validator=lambda out, err, return_code: self.validator_utils.step_validator(step, out, err,
                                                                                               return_code),
            return_code_interpreter=lambda out, err, return_code: self._get_return_code_from_device(out, return_code)
        )
        self.reproduce += test_runner.reproduce
        self.reproduce += self.expected_err_log
        result = "PASSED" if passed else f"FAILED with {fail_kind}"
        self.reproduce += f"Step '{step.name}' result: {result}\n"
        return passed, report, fail_kind

    def __expand_last_call_in_args(self, args: list[str]) -> list[str]:
        flags: list[str] = []
        for arg in self.__fix_entry_point(args):
            flag = utils.replace_macro(str(arg), "test-id", self.test_id)
            if utils.has_macro(flag):
                flag_expanded: str | list[str] = ""
                try:
                    flag_expanded = Macros.correct_macro(flag, self.test_extra_params)
                except ParameterNotFound:
                    flag_expanded = Macros.correct_macro(flag, self.test_env.config.workflow)
                if isinstance(flag_expanded, list):
                    flags.extend(flag_expanded)
                elif isinstance(flag_expanded, str):
                    flags.extend(flag_expanded.split())
            else:
                flags.extend(flag.split())
        return flags

    def __expand_last_call_macros(self, step: Step) -> list[str]:
        flags = step.args[:]
        while utils.list_has_macros(flags):
            flags = self.__expand_last_call_in_args(flags)
        if step.step_kind == StepKind.COMPILER and self.metadata.es2panda_options:
            if 'dynamic-ast' in self.metadata.es2panda_options:
                index = flags.index("--dump-ast")
                flags[index] = "--dump-dynamic-ast"
            if 'module' in self.metadata.es2panda_options:
                flags.insert(0, "--module")
        if step.step_kind == StepKind.RUNTIME and self.metadata.ark_options:
            prepend_options = self.__add_options(self.metadata.ark_options)
            flags = utils.prepend_list(prepend_options, flags)
        return flags

    def __add_panda_files(self) -> str:
        opt_name = '--panda-files'
        if self.dependent_abc_files:
            return f'{opt_name}={":".join(self.dependent_abc_files)}'

        return f'{opt_name}={self.test_abc!s}'

    def __add_boot_panda_files(self, args: list[str]) -> list[str]:
        dep_files_args: list[str] = []
        for arg in args:
            name = '--boot-panda-files'
            if name in arg:
                if not self.__boot_panda_files:
                    _, value = arg.split('=')
                    boot_panda_files = [value, *self.dependent_abc_files, self.test_abc.as_posix()]
                    self.__boot_panda_files = f'{name}={":".join(boot_panda_files)}'
                dep_files_args.append(self.__boot_panda_files)
            else:
                dep_files_args.append(arg)
        return dep_files_args


class ValidatorUtils:

    def __init__(self, flow_type: TestStandardFlow, test_env: TestEnv):
        self.test_env = test_env
        self.validator: IValidator = self._init_validator()
        self._flow_type: TestStandardFlow = flow_type

    @staticmethod
    def __get_validator_class(clazz: str) -> type[IValidator]:
        class_obj = get_class_by_name(clazz)
        if not issubclass(class_obj, IValidator):
            raise InvalidConfiguration(
                f"Validator class '{clazz}' not found. "
                f"Check value of 'validator' parameter")
        return class_obj

    def add_additional_validator(self) -> None:
        runtime_steps = [step.step_kind.value for step in self._flow_type.runtime_steps]
        if self._flow_type.has_expected:
            if self._flow_type.is_compile_only:
                self.validator.add(StepKind.COMPILER.value, BaseValidator.check_output)
            else:
                for step in runtime_steps:
                    self.validator.add(step, BaseValidator.check_output)

    def step_validator(self, step: Step, output: str, error: str, return_code: int) -> ValidationResult:
        validator_name = step.name if step.name in self.validator.validators else step.step_kind.value
        validator = self.validator.get_validator(validator_name)
        if validator is not None:
            for validator_func in validator:
                step_validator_result = validator_func(self._flow_type, step.step_kind.value,
                                                       output, error, return_code)
                if not step_validator_result.passed:
                    return ValidationResult(step_validator_result.passed,
                                            step_validator_result.kind,
                                            step_validator_result.description)
                if step_validator_result.stop_runtime_validation:
                    # we have a comparison result for runtime step - no need in output comparision with expected for
                    # further steps
                    self._delete_runtime_validator(step.step_kind.value)
            return ValidationResult(True, ValidatorFailKind.NONE, "")
        return ValidationResult(True, ValidatorFailKind.NONE, "")

    def _init_validator(self) -> IValidator:
        validator_class_name = self.test_env.config.test_suite.get_parameter("validator")
        if validator_class_name is None:
            return BaseValidator()
        validator_class = self.__get_validator_class(validator_class_name)
        return validator_class()

    def _delete_runtime_validator(self, step_kind: str) -> None:
        runtime_steps = [step.step_kind.value for step in self._flow_type.runtime_steps]
        runtime_steps.remove(step_kind)

        for step in runtime_steps:
            self.validator.delete(step, BaseValidator.check_output)
