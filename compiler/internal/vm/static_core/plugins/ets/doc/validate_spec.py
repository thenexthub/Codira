#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
# Script for validate spec .rst docs before build

import argparse
import re


def get_files_for_validation():
    parser = argparse.ArgumentParser(description='Validation for Spec')
    parser.add_argument('spec_files_dir', type=str, help='Dir with Spec files for validation')
    args = parser.parse_args()

    toctree_str = ".. toctree::\n"
    re_empty_str = " *\n"
    re_service_line = " *:.*\n"
    on_file_list = False
    file_list = list()
    spec_dir = args.spec_files_dir
    file_format = ".rst"

    with open("".join([spec_dir, "/", "index.rst"])) as file:
        for line in file:
            if on_file_list and not re.fullmatch(re_empty_str, line) and not re.fullmatch(re_service_line, line):
                file_name = line.replace(" ", "").replace("\n", "")
                file_name = "".join([spec_dir, file_name, file_format])
                file_list.append(file_name)
            if(line == toctree_str):
                on_file_list = True
    return file_list


def validate_meta_in_file(spec_file, spec_file_name):
    re_start_meta_line = "\.\. meta:\n"
    re_base_meta_line = " +.*\n"
    re_status_meta_line = " +(?:frontend_status: (?:Done|Partly|None))\n"
    re_todo_meta_line = " +(?:todo: .*|)\n"

    on_meta_flag = False
    num_of_line = 1
    num_of_errors = 0
    meta_with_errors = False
    num_of_frontend_status_in_meta = 0
    for line in spec_file:
        if(on_meta_flag and re.fullmatch(re_base_meta_line, line)):
            correct_meta_line = False
            if re.fullmatch(re_status_meta_line, line):
                num_of_frontend_status_in_meta += 1
                correct_meta_line = True
            if re.fullmatch(re_todo_meta_line, line):
                correct_meta_line = True
            if not correct_meta_line or num_of_frontend_status_in_meta > 1:
                print("".join([str(spec_file_name), ":", str(num_of_line), ": error in meta"]))
                print("".join(["line: ", line]))
                meta_with_errors = True
                num_of_errors += 1
        else:
            on_meta_flag = False
        if(re.fullmatch(re_start_meta_line, line)):
            on_meta_flag = True
            num_of_frontend_status_in_meta = 0
        num_of_line += 1
    return meta_with_errors, num_of_errors


def validate_meta(files_for_validation):
    num_of_errors = 0
    for spec_file in files_for_validation:
        with open(spec_file) as file:
            meta_with_errors, errors_in_file = validate_meta_in_file(file, spec_file)
            num_of_errors += errors_in_file
            print("".join([spec_file, ": ", ("meta contains errors" if meta_with_errors else "meta is ok")]))
    return num_of_errors


def main():
    files = get_files_for_validation()
    errors = validate_meta(files)
    if errors != 0:
        exit(1)

if __name__ == "__main__":
    main()
