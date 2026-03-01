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

import argparse
from os import makedirs, path
from typing import Optional, Sequence, Type, Union

from runner.enum_types.verbose_format import VerboseFilter, VerboseKind
from runner.path_utils import is_sudo_user
from runner.reports.report_format import ReportFormat
from runner.utils import EnumT, enum_from_str


def is_directory(arg: str) -> str:
    if not path.isdir(path.expanduser(arg)):
        raise argparse.ArgumentTypeError(f"The directory {arg} does not exist")

    return str(path.abspath(path.expanduser(arg)))


def make_dir_if_not_exist(arg: str) -> str:
    arg = path.abspath(path.expanduser(arg))
    if not path.isdir(arg):
        makedirs(arg)

    return str(arg)


def is_file(arg: str) -> str:
    if not path.isfile(path.expanduser(arg)):
        raise argparse.ArgumentTypeError(f"The file {arg} does not exist")

    return str(path.abspath(path.expanduser(arg)))


def check_int(value: str, value_name: str, *, is_zero_allowed: bool = False) -> int:
    ivalue = int(value)
    if ivalue < 0 or (not is_zero_allowed and ivalue == 0):
        raise argparse.ArgumentTypeError(f"{value} is an invalid {value_name} value")

    return ivalue


def check_timeout(value: str) -> int:
    return check_int(value, "timeout")


def is_enum_value(value: str, enum_class: Type[EnumT], option_name: str) -> EnumT:
    enum_value = enum_from_str(value, enum_class)
    if enum_value is None:
        raise argparse.ArgumentTypeError(f"{value} is an invalid for parameter {option_name}")
    return enum_value


def add_js_test_suite_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--test262', '-t', action='store_true', dest='test262',
        default=None,
        help='run test262 tests')
    parser.add_argument(
        '--hermes', action='store_true', dest='hermes',
        default=None,
        help='run Hermes tests')


class HandleTimeoutAction(argparse.Action):
    def __call__(self,
                 _: argparse.ArgumentParser,
                 namespace: argparse.Namespace,
                 _2: Optional[Union[str, Sequence]],
                 option_string: Optional[str] = None) -> None:
        if is_sudo_user():
            setattr(namespace, self.dest, True)
        else:
            raise argparse.ArgumentTypeError(f"Run under 'sudo' if you use {option_string} option")


def add_sts_test_suite_args(parser: argparse.ArgumentParser) -> None:
    ets_mutex_group = parser.add_mutually_exclusive_group(required=False)
    ets_mutex_group.add_argument(
        '--ets-system-tests', action='store_true', dest='system',
        default=None,
        help='run ets-system-tests')
    ets_mutex_group.add_argument(
        '--ets-func-tests', action='store_true', dest='ets_func_tests',
        default=None, help='run test against ETS STDLIB TEMPLATES and manual written ETS tests')
    ets_mutex_group.add_argument(
        '--ets-runtime', action='store_true', dest='ets_runtime',
        default=None,
        help='run ETS runtime tests')
    ets_mutex_group.add_argument(
        '--ets-gc-stress', action='store_true', dest='ets_gc_stress',
        default=None,
        help='run ETS GCStress tests')
    ets_mutex_group.add_argument(
        '--ets-cts', action='store_true', dest='ets_cts',
        default=None, help='run ets-templates tests')
    ets_mutex_group.add_argument(
        '--ets-es-checked', action='store_true', dest='ets_es_checked',
        default=None, help='run es checked templates tests')
    ets_mutex_group.add_argument(
        '--ets-custom', action='store_true', dest='ets_custom',
        default=None, help='run custom ETS suite test')
    ets_mutex_group.add_argument(
        '--ets-ts-subset', action='store_true', dest='ets_ts_subset',
        default=None, help='run comparing static VM vs dynamic one on TS subset suite test')
    ets_mutex_group.add_argument(
        '--ets-sdk', action='store_true', dest='ets_sdk',
        default=None, help='run sdk tests')



def add_other_test_suite_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--parser', action='store_true', dest='parser',
        default=None,
        help='run parser tests (ex --regression')
    parser.add_argument(
        '--declgenparser', action='store_true', dest='declgenparser',
        default=None,
        help="run declgen parser tests")
    parser.add_argument(
        '--astchecker', action='store_true', dest='astchecker',
        default=None,
        help='run ast checker tests')
    parser.add_argument(
        '--srcdumper', action='store_true', dest='srcdumper',
        default=None,
        help='run src dumper tests')
    parser.add_argument(
        '--recheck', action='store_true', dest='recheck',
        default=None,
        help='run recheck tests')


def add_test_suite_args(parser: argparse.ArgumentParser) -> None:
    # Test suite options
    parser.add_argument(
        '--test-suite', action='append', dest='test_suites',
        default=None,
        help='names of test suites to run')
    add_js_test_suite_args(parser)
    add_sts_test_suite_args(parser)
    add_other_test_suite_args(parser)


def add_ets_args(parser: argparse.ArgumentParser) -> None:
    generate_mutex_group = parser.add_mutually_exclusive_group(required=False)
    generate_mutex_group.add_argument(
        '--force-generate', action='store_true', dest='is_force_generate', default=None,
        help='mandatory generate the ETS test cases from templates')
    parser.add_argument(
        '--compare-files', action='store_true', dest='compare_files', default=None,
        help='only compile into .abc and compare whether abc files are equal')
    parser.add_argument(
        '--compare-files-iterations', action='store', dest='compare_files_iterations',
        type=lambda arg: check_int(arg, "--compare-files-iterations", is_zero_allowed=False),
        default=None,
        help='compile into .abc file the specified number of times; works only with the option compare-files')


def add_test_group_args(parser: argparse.ArgumentParser) -> None:
    # Test groups options
    test_groups = parser.add_argument_group(
        "Test groups",
        "allowing to divide tests into groups and run groups separately")
    test_groups.add_argument(
        '--groups', action='store', dest='groups',
        type=lambda arg: check_int(arg, "--groups", is_zero_allowed=False),
        default=None,
        help='Quantity of groups used for automatic division. '
             'By default 1 - all tests in one group')
    test_groups.add_argument(
        '--group-number', action='store', dest='group_number',
        type=lambda arg: check_int(arg, "--group-number", is_zero_allowed=False),
        default=None,
        help='run tests only of specified group number. Used only if --groups is set. '
             'Default value 1 - the first available group. '
             'If the value is more than the total number of groups the latest group is taken.')
    test_groups.add_argument(
        '--chapter', action='append', dest='chapters',
        type=str,
        default=None,
        help='run tests only of specified chapter. Can be specified several times.')
    test_groups.add_argument(
        '--chapters-file', action='store', dest='chapters_file',
        type=is_file,
        default=None,
        help='Full name to the file with chapter description. '
             'By default chapters.yaml file located along with test lists.')
    test_groups.add_argument(
        '--filter', '-f', action='store', dest='filter', default=None,
        help='test filter wildcard')


def add_config_args(parser: argparse.ArgumentParser) -> None:
    # Configuration
    parser.add_argument(
        '--config', action='append', dest='configs',
        default=None,
        type=is_file,
        help='config with runner options. Can be specified several config files, they will be merged. '
             'Values from the latest config file override values of the '
             'similar options from earlier config files.')
    parser.add_argument(
        '--generate-config', action='store', dest='generate_config',
        default=None,
        help='generate config file with all options')


def add_custom_suite_args(parser: argparse.ArgumentParser) -> None:
    # Custom suite
    parser.add_argument(
        '--custom-suite', dest='custom_suite_name', default=None,
        help='name of the custom suite')
    parser.add_argument(
        '--custom-test-root', dest='custom_test_root', default=None, type=is_directory,
        help='directory with test files for the custom suite. '
             'It cannot be set along with --test-root. '
             'It must be set if --custom-suite is set')
    parser.add_argument(
        '--custom-list-root', dest='custom_list_root', default=None, type=is_directory,
        help='directory with files what are lists of excluded/ignored tests for the custom suite. '
             'It cannot be set along with --list-root. '
             'It must be set if --custom-suite is set')
    parser.add_argument(
        '--custom-generator', dest='custom_generator', default=None, type=is_file,
        help='executable file generating tests. If this option is set, then --test-root is considered as a path '
             'where template files are located. '
             'It must be set if --custom-suite is set')
    parser.add_argument(
        '--custom-generator-option', action="append", dest='custom_generator_option', default=None,
        help='Options for the custom generator. It cannot be used without --custom-generator.')


def add_general_args(parser: argparse.ArgumentParser) -> None:
    # General - path options
    parser.add_argument(
        "--build-dir", action="store", dest='build_dir',
        type=is_directory,
        help='build directory. Obligatory parameter.')
    parser.add_argument(
        '--test-root', dest='test_root', default=None, type=is_directory,
        help='directory with test file. If not set the module directory is used')
    parser.add_argument(
        '--list-root', dest='list_root', default=None, type=is_directory,
        help='directory with files what are lists of excluded/ignored tests. If not set the module directory is used')
    parser.add_argument(
        '--ets-stdlib-root', dest='ets_stdlib_root', default=None, type=is_directory,
        help='directory of eTS standard library source code. '
             'Applied only to ets-func-tests and ets-runtime test suites.')
    parser.add_argument(
        '--work-dir', action='store', dest='work_dir', default=None, type=make_dir_if_not_exist,
        help='path to the working temp folder with gen, intermediate and report folders')
    # General - other options
    parser.add_argument('--processes', '-j', dest='processes', default=None,
                        help='Number of processes to use in parallel. By default 1. '
                             'Special value `all` - means to use all available processes')
    parser.add_argument(
        '--gc-type', dest='gc_type',
        default=None, help='Type of garbage collector')
    parser.add_argument(
        '--full-gc-bombing-frequency', dest='full_gc_bombing_frequency', default=None, help='Full GC bombing frequency')
    parser.add_argument(
        '--no-run-gc-in-place', action='store_true', dest='run_gc_in_place',
        default=None,
        help='enable --run-gc-in-place mode')
    parser.add_argument(
        '--generate-only', action='store_true', dest='generate_only', default=None,
        help='only generate tests without running')

    qemu_group = parser.add_mutually_exclusive_group(required=False)
    qemu_group.add_argument(
        '--arm64-qemu', action='store_true', dest='arm64_qemu', default=None,
        help='launch all binaries in qemu aarch64')
    qemu_group.add_argument(
        '--arm32-qemu', action='store_true', dest='arm32_qemu', default=None,
        help='launch all binaries in qemu arm')


def add_general_report_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--report-format', action='store', dest='report_formats', default=None,
        type=lambda arg: is_enum_value(arg, ReportFormat, "--report-format"),
        help='formats of report files. Supported values: log, md and html. '
             'Several values can be set - in such case formats of all specified values are generated. '
             'Default value: log')
    parser.add_argument(
        '--detailed-report', action='store_true', dest='detailed_report', default=None,
        help='creates additional detailed report with counting tests for each folder. '
             'By default the report is $WorkDir/report/<suite name>_detailed-report.md '
             'where $WorkDir is the folder specified by the option --work-dir')
    parser.add_argument(
        '--detailed-report-file', action='store', dest='detailed_report_file', default=None,
        help='specifies custom file name for the detailed report. '
             'By default the report is $WorkDir/report/<suite name>_detailed-report.md '
             'where $WorkDir is the folder specified by the option --work-dir')
    parser.add_argument(
        '--spec-report', action='store_true', dest='spec_report', default=None,
        help='creates specification coverage report in two formats: markdown and YAML. '
             'By default the report files are written into $WorkDir/report/ directory, '
             'where $WorkDir is the folder specified by the option --work-dir')
    parser.add_argument(
        '--spec-report-file', action='store', dest='spec_report_file', default=None,
        help='specifies custom file name for the specification coverage report in markdown format. '
             'By default the report is $WorkDir/report/<suite name>_spec-report.md '
             'where $WorkDir is the folder specified by the option --work-dir')
    parser.add_argument(
        '--spec-report-yaml', action='store', dest='spec_report_yaml', default=None,
        help='specifies custom file name for the specification coverage report in YAML format. '
             'By default the report is $WorkDir/report/<suite name>_spec-report.yaml '
             'where $WorkDir is the folder specified by the option --work-dir')
    parser.add_argument(
        '--spec-file', action='store', dest='spec_file', default=None, type=is_file,
        help='specifies file/path of specification PDF file.')


def add_general_other_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--show-progress', action='store_true', dest='progress', default=None,
        help='don\'t show progress bar')
    parser.add_argument(
        '--verbose', '-v', action='store', dest='verbose', default=None,
        type=lambda arg: is_enum_value(arg, VerboseKind, "--verbose"),
        help='Enable verbose output. Possible values one of: all - the most detailed output, '
             'short - test status and output. By default: only test status for new failed tests')
    parser.add_argument(
        '--verbose-filter', action='store', dest='verbose_filter',
        default=None,
        type=lambda arg: is_enum_value(arg, VerboseFilter, "--verbose-filter"),
        help='Filter for what kind of tests to output stdout and stderr. Works only when --verbose option is set.'
             'Supported values: all - for all executed tests, '
             'ignored - for new failures and tests from ignored test lists both passed and failed. '
             'new - only for new failures. Default value.')
    parser.add_argument(
        '--no-bco', action='store_false', dest='bco', default=None,
        help='disable bytecodeopt. Applied only to test262')
    parser.add_argument(
        '--force-download', action='store_true', dest='force_download',
        default=None, help='force download and prepare test suites')
    parser.add_argument(
        '--no-js', action='store_false', dest='with_js',
        default=None,
        help='disable JS-related tests')
    parser.add_argument(
        '--gdb-timeout', type=check_timeout, default=None,
        dest='gdb_timeout', help='Additional timeout for collecting stack information by gdb. By default 10 sec')
    parser.add_argument(
        '--handle-timeout', action=HandleTimeoutAction, default=None, nargs=0,
        dest='handle_timeout',
        help='Specify to handle failures by timeout for collecting stack information by gdb. Requires run under sudo')


def add_es2panda_args(parser: argparse.ArgumentParser) -> None:
    # Es2panda options
    parser.add_argument(
        '--custom-es2panda', action='store', dest='custom_es2panda_path',
        default=None,
        type=is_file,
        help='custom path to es2panda binary file')
    parser.add_argument(
        '--es2panda-timeout', type=check_timeout, default=None,
        dest='es2panda_timeout', help='es2panda translator timeout')
    parser.add_argument(
        '--es2panda-opt-level',
        default=None, type=lambda arg: check_int(arg, "es2panda-opt-level", is_zero_allowed=True),
        dest='es2panda_opt_level', help='es2panda opt-level. By default 2')
    parser.add_argument(
        '--es2panda-debug-info', action='store_true',
        dest='es2panda_debug_info', help='Compile with debug-info, false by default')
    parser.add_argument(
        '--arktsconfig', action='store', dest='arktsconfig',
        type=is_file, default=None,
        help='path to arktsconfig file')
    parser.add_argument(
        '--es2panda-args', action='append', dest='es2panda_args', default=None,
        help='Additional arguments that will be passed to es2panda')


def add_verifier_args(parser: argparse.ArgumentParser) -> None:
    # Verifier options
    parser.add_argument(
        '--disable-verifier', action="store_false", default=None,
        dest='enable_verifier', help='bytecode verifier timeout')
    parser.add_argument(
        '--verifier-timeout', type=check_timeout, default=None,
        dest='verifier_timeout', help='bytecode verifier timeout')
    parser.add_argument(
        '--verifier-config', action='store', dest='verifier_config',
        type=is_file,
        default=None,
        help='verification config file')


def add_ark_aot_quick_args(parser: argparse.ArgumentParser) -> None:
    # Ark_aot options
    parser.add_argument(
        '--aot', action='store_true', dest='aot', default=None,
        help='use AOT compilation')
    parser.add_argument(
        '--paoc-timeout', type=check_timeout, default=None,
        dest='paoc_timeout', help='AOT compiler timeout')
    parser.add_argument(
        '--aot-args', action='append', dest='aot_args', default=None,
        help='Additional arguments that will passed to ark_aot')

    # Quick options
    parser.add_argument(
        '--quick', '-q', action='store_true', dest='quick', default=None,
        help='use bytecode quickener')


def add_ark_args(parser: argparse.ArgumentParser) -> None:
    # Ark options
    parser.add_argument(
        '--ark-args', action='append', dest='ark_args', default=None,
        help='Additional arguments that will passed to ark as is.')
    parser.add_argument(
        '--timeout', type=check_timeout,
        dest='timeout', default=None, help='JS runtime timeout')
    parser.add_argument(
        '--heap-verifier', dest='heap_verifier', default=None,
        help='Heap verifier options')

    interpreter_group = parser.add_mutually_exclusive_group(required=False)
    interpreter_group.add_argument(
        '--irtoc', action='store_const', const="irtoc", dest='interpreter_type', default=None,
        help='use irtoc in interpreter')
    interpreter_group.add_argument(
        '--llvmirtoc', action='store_const', const="llvm", dest='interpreter_type', default=None,
        help='use llvmirtoc in interpreter')
    interpreter_group.add_argument(
        '--interpreter-type', action='store', dest='interpreter_type', default=None,
        help='use explicitly set interpreter')

    # Ark: Jit options
    parser.add_argument(
        '--jit', action='store_true', dest='jit', default=None,
        help='use JIT in interpreter')
    parser.add_argument(
        '--jit-preheat-repeats', action='store', dest='jit_preheat_repeats',
        default=None,
        help='Sets additional parameters for JIT preheat actions. '
             'Possible values: "num_repeats=30,compiler_threshold=20". '
             'Works only with --jit option.')


def add_time_report_args(parser: argparse.ArgumentParser) -> None:
    # Time report options
    parser.add_argument(
        '--time-report', action='store_true', dest='time_report', default=None,
        help='Log execution test time')
    parser.add_argument(
        '--time-edges', action='store', dest='time_edges', default=None,
        help='Time edges in the format `1,5,10` where numbers are seconds')


def add_test_lists_args(parser: argparse.ArgumentParser) -> None:
    # Test lists options
    parser.add_argument(
        '--test-list', dest='test_list', default=None,
        help='run only the tests listed in this file')
    parser.add_argument(
        '--test-file', dest='test_file', default=None,
        help='run only one test specified here')
    parser.add_argument(
        '--test-list-arch', action='store', dest='test_list_arch', default=None,
        help='load specified architecture specific test lists. One of: arm32, arm64, amd32, amd64')
    parser.add_argument(
        '--test-list-san', action='store', dest='test_list_san', default=None,
        help='load specified sanitizer specific test lists. '
             'One of asan (on running against build with ASAN and UBSAN sanitizers), '
             'tsan (on running against build with TSAN sanitizers).')
    parser.add_argument(
        '--test-list-os', action='store', dest='test_list_os', default=None,
        help='load specified operating system specific test lists. '
             'One of LIN, WIN or MAC')
    parser.add_argument(
        '--test-list-build', action='store', dest='test_list_build', default=None,
        help='load specified build type specific test lists. '
             'One of FAST-VERIFY, DEBUG or RELEASE')
    parser.add_argument(
        '--skip-test-lists', action='store_true', dest='skip_test_lists', default=None,
        help='do not use ignored or excluded lists, run all available tests, report all found failures')
    parser.add_argument(
        '--update-excluded', action='store_true', dest='update_excluded', default=None,
        help='update list of excluded tests')
    parser.add_argument(
        '--update-expected', action='store_true', dest='update_expected', default=None,
        help='update files with expected results')


def add_coverage_args(parser: argparse.ArgumentParser) -> None:
    # Coverage settings
    parser.add_argument(
        '--use-llvm-cov', action='store_true', dest='use_llvm_cov', default=None)
    parser.add_argument(
        '--use-lcov', action='store_true', dest='use_lcov', default=None)

    parser.add_argument(
        '--llvm-cov-exclude', dest='llvm_cov_exclude', default=None,
        help='')
    parser.add_argument(
        '--lcov-exclude', dest='lcov_exclude', default=None,
        help='')

    parser.add_argument(
        '--profdata-files-dir', dest='profdata_files_dir', default=None,
        type=make_dir_if_not_exist,
        help='Directory where coverage intermediate files (*.profdata) are created.')
    parser.add_argument(
        '--coverage-html-report-dir', dest='coverage_html_report_dir', default=None,
        type=make_dir_if_not_exist,
        help='Stacks files in the specified directory')
    parser.add_argument(
        '--coverage-per-binary', action='store_true', dest='coverage_per_binary',
        default=None, help='Creates a separate coverage report for each binary running')

    parser.add_argument(
        '--gcov-tool', dest='gcov_tool', default=None,
        help='Specify gcov binary name (Needed for lcov --gcov-tool)')


def add_declgen_ets2ts_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--declgen-ets2ts-cts', action='store_true', dest='declgen_ets2ts_cts',
        default=None,
        help="run declgen_ets2ts ets-templates tests")
    parser.add_argument(
        '--declgen-ets2ts-func-tests', action='store_true', dest='declgen_ets2ts_func_tests',
        default=None,
        help="run declgen_ets2ts test against ETS STDLIB TEMPLATES and manual written ETS tests")
    parser.add_argument(
        '--declgen-ets2ts-runtime', action='store_true', dest='declgen_ets2ts_runtime',
        default=None,
        help="run declgen_ets2ts ETS runtime tests")
    parser.add_argument(
        '--declgen-ets2ts-sdk', action='store_true', dest='declgen_ets2ts_sdk',
        default=None,
        help="run declgen_ets2ts ETS sdk")


def add_declgen_ts2ets_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--declgen-ts2ets-cts', action='store_true', dest='declgen_ts2ets_cts',
        default=None,
        help="run declgen_ts2ets ts-templates tests")


def add_declgen_ets2ets_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        '--declgen-ets2ets', action='store_true', dest='declgen_ets2ets',
        default=None,
        help="run declgen_ets2ets tests")


def get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Regression test runner")

    add_test_suite_args(parser)
    add_ets_args(parser)
    add_test_group_args(parser)
    add_config_args(parser)
    add_custom_suite_args(parser)
    add_general_args(parser)
    add_general_report_args(parser)
    add_general_other_args(parser)
    add_es2panda_args(parser)
    add_verifier_args(parser)
    add_ark_aot_quick_args(parser)
    add_ark_args(parser)
    add_time_report_args(parser)
    add_test_lists_args(parser)
    add_coverage_args(parser)
    add_declgen_ets2ts_args(parser)
    add_declgen_ts2ets_args(parser)
    add_declgen_ets2ets_args(parser)

    return parser.parse_args()
