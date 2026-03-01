#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
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

import argparse
import json
import multiprocessing
import sys
import os
import subprocess as spp
import tempfile
from collections import OrderedDict
from typing import List
import logging

NPROC = multiprocessing.cpu_count()
TMP_DIR = tempfile.gettempdir()

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
OPTIONS_LIST_PATH = os.path.join(SCRIPT_DIR, 'options_list.json')


def get_args():
    parser = argparse.ArgumentParser(description="Abckit stress test")
    parser.add_argument('--build-dir',
                        type=str,
                        required=True,
                        help=f'Path to build dir')
    parser.add_argument('--update-fail-list',
                        action='store_true',
                        default=False,
                        help=f'Update fail list')
    parser.add_argument("--ark-path",
                        type=str,
                        default=None,
                        help=f'ARK runtime path')
    parser.add_argument("--repeats",
                        type=int,
                        default=None,
                        help=f'VM test retry attempts')
    parser.add_argument("--with-runtime",
                        type=bool,
                        default=False,
                        help=f'Run stress tests and compare their outputs')
    return parser.parse_args()


def collect_from(rd, func) -> List[str]:
    tmp: List[str] = []
    for root, _, file_names in os.walk(rd):
        for file_name in file_names:
            tmp.append(os.path.join(root, file_name))
    return filter(func, tmp)


class ExecRes:

    def __init__(self, returncode, stdout, stderr):
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr


def stress_exec(cmd, **kwargs):
    logger = create_logger()
    default_kwargs = {
        'cwd': os.getcwd(),
        'allow_error': False,
        'timeout': 900,
        'print_output': False,
        'print_command': True,
        'env': {},
        'repeats': 1,
    }
    kwargs = {**default_kwargs, **kwargs}

    if kwargs['print_command']:
        logger.debug(
            '$ %s> %s %s', kwargs.get('cwd'), " ".join(
                list(
                    map(lambda x: f'{x}={kwargs.get("env").get(x)}',
                        list(kwargs.get('env'))))), " ".join(cmd))

    ct = kwargs['timeout']
    sub_env = {**os.environ.copy(), **kwargs['env']}
    for _ in range(kwargs['repeats']):
        return_code, stdout, stderr = __exec_impl(
            cmd,
            cwd=kwargs.get('cwd'),
            timeout=ct,
            print_output=kwargs.get('print_output'),
            env=sub_env)
        if return_code == 0:
            break
        ct = ct + 60

    if return_code != 0 and not kwargs['allow_error']:
        raise Exception(
            f"Error: Non-zero return code\nstdout: {stdout}\nstderr: {stderr}")
    return ExecRes(return_code, stdout, stderr)


def __exec_impl(cmd,
                cwd=os.getcwd(),
                timeout=900,
                print_output=False,
                env=None):
    logger = create_logger()
    proc = spp.Popen(cmd,
                     cwd=cwd,
                     stdout=spp.PIPE,
                     stderr=spp.STDOUT,
                     encoding='ISO-8859-1',
                     env=env)
    while True and print_output:
        line = proc.stdout.readline()
        logger.debug(line.strip())
        if not line:
            break
    try:
        stdout, stderr = proc.communicate(timeout=timeout)
        return_code = proc.wait()
    except spp.TimeoutExpired:
        stdout, stderr = "timeout", "timeout"
        return_code = -1

    return return_code, stdout, stderr


def parse_stdout(error: str, stdout):
    if stdout is not None and 'ASSERTION FAILED:' in stdout:
        for line in stdout.split('\n'):
            if 'ASSERTION FAILED:' in line:
                error = line
    if stdout is not None and 'ERROR: plugin returned non-zero' in stdout:
        for line in stdout.split('\n'):
            if 'failed!' in line:
                error = line.split(' ')[-2] + " " + line.split(' ')[-1]
    return error


def get_fail_list(result):
    fail_list = OrderedDict()
    for src_path in result:
        if result[src_path]['error'] != '0':
            fail_list[src_path] = result[src_path]['error']
    return fail_list


def check_regression_errors(kfl_path: str, fail_list):
    logger = create_logger()
    with open(kfl_path, 'r') as f:
        old_fail_list = json.load(f)
    regression_errors = []
    for fail in fail_list:
        if fail not in old_fail_list:
            regression_errors.append(f'REGRESSION ERROR: new fail "{fail}"')
    if regression_errors:
        logger.debug('\n'.join(regression_errors))
        return False
    return True


def update_fail_list(kfl_path: str, fail_list):
    with os.fdopen(
            os.open(kfl_path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o755),
            'w') as f:
        json.dump(fail_list, f, indent=2, sort_keys=True)


def check_fail_list(kfl_path: str, fail_list):
    logger = create_logger()

    with open(kfl_path, 'r') as f:
        old_fail_list = json.load(f)

    errors = []
    for old_fail in old_fail_list:
        if old_fail not in fail_list:
            errors.append(f'ERROR: no file in new fail list "{old_fail}"')
    if errors:
        logger.debug('\n'.join(errors))
        logger.debug(
            'Please update fail list, rerun this script with "--update-fail-list"'
        )
        return False
    return True


def create_logger():
    logger = multiprocessing.get_logger()
    logger.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
        '[%(asctime)s| %(levelname)s| %(processName)s] %(message)s')
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(formatter)

    # this bit will make sure you won't have
    # duplicated messages in the output
    if not len(logger.handlers):
        logger.addHandler(handler)
    return logger


def run_abckit(build_dir, source, input_abc, output_abc):
    abckit = os.path.join(build_dir, 'arkcompiler', 'runtime_core', 'abckit')
    stress_plugin = os.path.join(build_dir, 'arkcompiler', 'runtime_core',
                                 'libabckit_stress_plugin.so')
    ld_library_path = []
    ld_library_path_old = os.environ.get('LD_LIBRARY_PATH')
    if ld_library_path_old:
        ld_library_path.append(ld_library_path_old)
    ld_library_path = [
        os.path.join(build_dir, 'arkcompiler', 'runtime_core'),
        os.path.join(build_dir, 'arkcompiler', 'ets_runtime'),
        os.path.join(build_dir, 'arkcompiler', 'ets_frontend'),
        os.path.join(build_dir, 'thirdparty', 'icu'),
        os.path.join(build_dir, 'thirdparty', 'zlib')
    ]
    cmd = [
        abckit, '--plugin-path', stress_plugin, '--input-file', input_abc,
        '--output-file', output_abc
    ]
    options_list = OrderedDict()
    with open(OPTIONS_LIST_PATH, 'r') as f:
        options_list = json.load(f)
    if source in options_list:
        cmd += [options_list[source][0], options_list[source][1]]
    result = stress_exec(cmd,
                         allow_error=True,
                         print_command=True,
                         env={"LD_LIBRARY_PATH": ':'.join(ld_library_path)})
    return result
