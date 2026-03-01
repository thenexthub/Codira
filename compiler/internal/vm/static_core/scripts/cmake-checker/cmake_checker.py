#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

# cmake-checker.py checks that panda wrappers are used instead of standard
# cmake functions.
# cmake-checker.py <DIRECTORY> [TEST]
# - <DIRECTORY>
#       Root directory for checking
# - [TEST]
#       Test that cmake-checker work properly

import os
import re
import sys
import tempfile
import shutil
import subprocess

# List of functions to search for
function_list = [("add_library", "panda_add_library"),
                 ("add_executable", "panda_add_executable"),
                 ("target_link_libraries", "panda_target_link_libraries"),
                 ("target_include_directories", "panda_target_include_directories"),
                 ("target_compile_options", "panda_target_compile_options"),
                 ("target_compile_definitions", "panda_target_compile_definitions"),
                 ("target_sources", "panda_target_sources")]

ERROR_MESSAGE = "Found restricted native CMake function usage:"


def run_cmake_checker(directory):
    # don't search in following files and folders
    ignore_files = ["cmake/PandaCmakeFunctions.cmake", "cmake/Sanitizers.cmake"]
    ignore_files = [os.path.join(directory, s) for s in ignore_files]
    ignore_paths = ["third_party"]

    # search in following third_party folders
    white_list_paths = ["cmake/third_party"]

    cmake_files = []

    # Fetch all cmake files in directory
    for root, dirs, files in os.walk(directory, followlinks=True):
        ignore = False
        for white_path in white_list_paths:
            # Check if root directory matches any white list path
            if root.startswith(os.path.join(directory, white_path)):
                continue
            else:
                # Check if root directory matches any ignore path
                for ignore_path in ignore_paths:
                    if root.startswith(os.path.join(directory, ignore_path)) or "third_party" in root:
                        ignore = True
                        dirs.clear()
                        break

        if ignore:
            continue

        for file in files:
            if file == "CMakeLists.txt" or file.endswith(".cmake") and os.path.join(root, file) not in ignore_files:
                cmake_files.append(os.path.join(root, file))

    error = False
    # Grep for function in each cmake file
    for file in cmake_files:
        with open(file, 'r') as f:
            lines = f.readlines()
            line_number = 1
            for line in lines:
                for function_name in function_list:
                    if re.search(r'\b{}\b'.format(function_name[0]), line.split('#')[0]):
                        if not error:
                            print(ERROR_MESSAGE)
                        error = True
                        print("  {} instead of {} at {}:{}".format(function_name[0], function_name[1],
                            os.path.relpath(file, directory), line_number), file=sys.stderr)
                line_number += 1

    if error:
        exit(1)

    print("cmake-checker passed successfully!")


# create file that uses standard cmake functions and check that cmake_checker will find them
def test_cmake_checker(directory):
    source_file = os.path.join(directory, "CMakeLists.txt")

    temp_dir = tempfile.mkdtemp(dir=directory)
    temp_file = os.path.join(temp_dir, "CMakeLists.txt")

    try:
        shutil.copy(source_file, temp_file)

        with open(temp_file, 'r') as file:
            content = file.read()

        # replace wrappers with standard cmake functions and add standard functions in the end
        # in case we don't have them in main CMakeLists.txt
        for function in function_list:
            content = content.replace(function[1], function[0])
            content = f"{content}{function[0]}\n"

        with os.fdopen(os.open(temp_file, os.O_RDWR | os.O_CREAT, 0o755), 'w') as file:
            file.write(content)

        args = [sys.argv[0], directory]
        process = subprocess.run(args, capture_output=True)

        if process.returncode == 1 and ERROR_MESSAGE in process.stdout.decode():
            print("test-cmake-checker passed successfully!")
        else:
            sys.exit("Failed: cmake-checker doesn't work properly.")

    finally:
        shutil.rmtree(temp_dir)

if __name__ == "__main__":
    arg_directory = sys.argv[1]
    if len(sys.argv) == 3:
        test_cmake_checker(arg_directory)
    else:
        run_cmake_checker(arg_directory)
