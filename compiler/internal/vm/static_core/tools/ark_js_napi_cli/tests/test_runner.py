#!/usr/bin/env python3
#coding: utf-8

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#     http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Script-runner for XGC tests"""

import argparse
import logging
import os
import shutil
import subprocess
import sys
from typing import Optional, List

logger = logging.getLogger(__name__)


class PathConfig:
    """Necessary file paths config for test class"""
    build_root: str
    test_dir: str

    def __init__(self, build_dir: str, test_dir: str):
        self.build_root = build_dir
        self.test_dir = test_dir

    @staticmethod
    def get_ts_entry_point(file: str) -> str:
        """Get ts entry point name by path to ts file"""
        # path/to/ts_test.ts -> ts_test
        return file.split('/')[-1][:-3]

    def get_abc(self, file: str, suffix: str) -> str:
        """Get .abc filename by file path"""
        file_to_abc = file.replace('/', '_')[:-len(suffix)]
        return os.path.join(self.get_testspace(), f'{file_to_abc}.abc')

    def get_ld_libs(self) -> str:
        """Get libraries list for LD_LIBRARY_PATH"""
        return ':'.join([
            f'{self.build_root}/arkcompiler/runtime_core',
            f'{self.build_root}/arkcompiler/ets_runtime',
            f'{self.build_root}/arkui/napi',
            f'{self.build_root}/thirdparty/icu',
            f'{self.build_root}/thirdparty/libuv',
            f'{self.build_root}/thirdparty/bounds_checking_function',
        ])

    def get_testspace(self) -> str:
        """Get full path to test space directory"""
        return os.path.join(self.build_root, 'hybrid_tests')


def parse_args() -> argparse.Namespace:
    """parse arguments."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--tests-list', required=True, help='path to the tests list file')
    parser.add_argument('--build-dir', required=True, help='path to build dir')
    parser.add_argument('--tests-dir', required=True, help='path to tests dir')
    parser.add_argument('--verbose', action='store_true', default=False,
                        required=False, help='print detailed info during script execution')
    return parser.parse_args()


def check_command_result(command: List[str], change_dir: Optional[str] = None) -> bool:
    command_str = ' '.join(command)
    ld_library_path = os.environ.get('LD_LIBRARY_PATH')
    if ld_library_path is not None:
        command_str = f'LD_LIBRARY_PATH={ld_library_path} {command_str}'
    if change_dir is not None:
        command_str = f'cd {change_dir}\n{command_str}'
    logger.debug('Run command:\n%s', command_str)
    # Run command
    try:
        subprocess.check_output(command, stderr=subprocess.STDOUT, cwd=change_dir)
    except subprocess.CalledProcessError as e:
        if not e.stdout:
            logger.warning('Note: missed output for %s', command_str)
            return True
        logger.warning(e.stdout.decode())
        if e.stderr:
            logger.warning(e.stderr.decode())
        logger.warning('Failed:\n%s', command_str)
        return False
    return True


def compile_ts_cmd(config: PathConfig, file: str) -> bool:
    """Compile ts file via es2abc"""
    return check_command_result([
        f'{config.build_root}/arkcompiler/ets_frontend/es2abc',
        '--merge-abc',
        '--extension=ts',
        '--module',
        '--output', config.get_abc(file, '.ts'),
        os.path.join(config.test_dir, file)
    ])


def compile_sts_cmd(config: PathConfig, file: str) -> bool:
    """Compile ets file via es2panda"""
    abc_file: str = config.get_abc(file, '.ets')
    return check_command_result([
        f'{config.build_root}/arkcompiler/ets_frontend/es2panda',
        '--extension=ets',
        '--opt-level=2',
        f'--arktsconfig={config.build_root}/arkcompiler/ets_frontend/arktsconfig.json',
        f'--output={abc_file}',
        os.path.join(config.test_dir, 'ets', file)
    ])


def prepare_test_files(path_config: PathConfig) -> bool:
    """Create test space and compile necessary common part for tests"""
    test_space: str = path_config.get_testspace()
    try:
        os.makedirs(f'{test_space}/module', exist_ok=True)
        # Copy ETS specific files
        shutil.copy(f'{path_config.build_root}/arkcompiler/runtime_core/libets_interop_js_napi.so',
                    f'{test_space}/module')
        shutil.copy(f'{path_config.build_root}/gen/arkcompiler/runtime_core/static_core/plugins/ets/etsstdlib.abc',
                    test_space)
    except OSError as error:
        logger.warning(error)
        return False
    # Compile necessary .ts files
    return compile_ts_cmd(path_config, 'gc_test_common.ts')


def run_test(config: PathConfig, ts_test_path: str) -> bool:
    """Run hybrid test in test space directory"""
    return check_command_result([
        f'{config.build_root}/arkcompiler/runtime_core/ark_js_napi_cli',
        '--stub-file', f'{config.build_root}/gen/arkcompiler/ets_runtime/stub.an',
        '--enable-force-gc=false',
        '--log-info=gc',
        '--open-ark-tools=true',
        '--entry-point', PathConfig.get_ts_entry_point(ts_test_path),
        config.get_abc(ts_test_path, '.ts')
    ], config.get_testspace())


def judge_output(arguments: argparse.Namespace) -> bool:
    path_config = PathConfig(arguments.build_dir, arguments.tests_dir)
    if not prepare_test_files(path_config):
        return False
    try:
        with open(arguments.tests_list, 'r') as f:
            lines = f.read().splitlines()
    except OSError:
        logger.warning('Could not open/read tests file: %s', arguments.tests_list)
        return False
    result = True
    os.environ['LD_LIBRARY_PATH'] = path_config.get_ld_libs()
    for line in lines:
        test_names = line.strip()
        if test_names and test_names[0] != '#':
            file_names = list(map(lambda x: x.strip(), test_names.split(',')))
            assert len(file_names) == 2, "Expected 2 files: .ts and .ets"
            # Run test
            logger.debug('---- Run: %s, %s ---', file_names[0], file_names[1])
            if (compile_ts_cmd(path_config, file_names[0]) and
                compile_sts_cmd(path_config, file_names[1]) and
                run_test(path_config, file_names[0])):
                logger.info('Passed: %s, %s', file_names[0], file_names[1])
            else:
                result = False
    return result

if __name__ == '__main__':
    input_args = parse_args()
    logging.basicConfig(format='%(message)s')
    if input_args.verbose:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)
    if not judge_output(input_args):
        sys.exit("Failed: hybrid tests runner got errors")
    logger.info("Hybrid tests runner was passed successfully!")
