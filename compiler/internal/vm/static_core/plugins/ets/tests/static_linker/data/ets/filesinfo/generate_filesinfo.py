#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import logging
import os
import sys


def find_files_with_extensions(directory='.', extensions=None):
    if extensions is None:
        extensions = ['.abc']
    files_list = sorted([
        os.path.abspath(os.path.join(root, file))
        for root, _, files in os.walk(directory)
        for file in files
        if any(file.endswith(ext) for ext in extensions)
    ])
    return files_list


def write_abc_files_to_filesinfo(abc_files, output_file):
    try:
        fd = os.open(output_file, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o644)
        with os.fdopen(fd, 'w', encoding='utf-8') as f:
            for file_path in abc_files:
                f.write(file_path + '\n')
    except Exception as e:
        raise Exception(f'failed to write to {output_file}') from e


def find_abc_files(directory='.', output_file='filesinfo.txt'):
    try:
        abc_files = find_files_with_extensions(directory, ['.abc'])
        write_abc_files_to_filesinfo(abc_files, output_file)
    except Exception as e:
        logging.exception('generate filesinfo failed')
        sys.exit(1)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, format='%(message)s')
    parser = argparse.ArgumentParser(description='generate filesinfo')
    parser.add_argument(
        '--dir',
        type=str,
        default='.',
        help='working directory (default: .)',
        required=False
    )
    parser.add_argument(
        '--output',
        type=str,
        default='filesinfo.txt',
        help='output path (default: filesinfo.txt)',
        required=False
    )

    args = parser.parse_args()

    logging.info('working directory: %s', args.dir)
    logging.info('output path: %s', args.output)

    find_abc_files(args.dir, args.output)
