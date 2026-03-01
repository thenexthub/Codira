#!/usr/bin/env python3
# -- coding: utf-8 --
#
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

import sys
import os
import uuid
from pathlib import Path
from typing import Optional, List
from multiprocessing import current_process


BINARY_NAMES: set[str] = {'es2panda', 'ark', 'verifier', 'ark_aot', 'ark_asm'}
SCRIPT_FILE_EXTENSIONS: set[str] = {'.rb', '.py', '.sh'}
COVERAGE_ROOT_DIR: Path = Path('/tmp/coverage_intermediate')
LLVM_PROFDATA_BINARY: str = 'llvm-profdata-14'

COVERAGE_TOOLS = {'llvm-cov', 'lcov'}

ADD_CUSTOM_TARGET_FUNCTION_ARGS: set[str] = {
    'ALL', 'DEPENDS', 'BYPRODUCTS', 'WORKING_DIRECTORY',
    'COMMENT', 'VERBATIM', 'USES_TERMINAL',
    'COMMAND_EXPAND_LISTS', 'JOB_SERVER_AWARE', 'COMMAND'
}


def is_script_file(file_path: str) -> bool:
    return any(file_path.endswith(ext) for ext in SCRIPT_FILE_EXTENSIONS)


def is_script_call(command_part: str) -> bool:
    return (is_script_file(command_part) or
            (command_part.startswith('./') and is_script_file(command_part)))


def is_cmake_copy_command(command_parts: List[str]) -> bool:
    return (len(command_parts) >= 4 and
            command_parts[0].endswith('cmake') and
            command_parts[1] == '-E' and
            command_parts[2] in {'copy', 'copy_directory'})


def is_target_binary(command_part: str) -> bool:
    """Check if the command part refers to one of our target binaries."""
    command_part = command_part.lower()

    # Direct binary name match
    if command_part in {cmd.lower() for cmd in BINARY_NAMES}:
        return True

    # Check for CMake generator expressions
    if '$<' in command_part and '>' in command_part:
        return any(cmd.lower() in command_part for cmd in BINARY_NAMES)

    # Check for binary in file path
    if '/' in command_part or '\\' in command_part:
        filename = os.path.basename(command_part).lower()
        return filename in {cmd.lower() for cmd in BINARY_NAMES}

    return False


def generate_profdata_merge_command(profraw_path: Path) -> str:
    """Generate command to merge LLVM profraw files into profdata format."""
    return (f"COMMAND;{LLVM_PROFDATA_BINARY};merge;"
            f"-output={profraw_path.with_suffix('.profdata')};{profraw_path}")


def extract_target_binary_name(command_part: str) -> Optional[str]:
    """Extract the binary name from a command part if it's one of our targets."""

    # Handle CMake generator expressions
    if command_part.startswith('$<TARGET_FILE:') and command_part.endswith('>'):
        binary_name = command_part[len('$<TARGET_FILE:'):-1]
        if binary_name in BINARY_NAMES:
            return binary_name

    # Direct binary name match
    if command_part in BINARY_NAMES:
        return command_part

    # Extract from file path
    if '/' in command_part or '\\' in command_part:
        filename = os.path.basename(command_part)
        return next((cmd for cmd in BINARY_NAMES 
                    if cmd.lower() == filename.lower()), filename)

    return command_part


def find_binary_in_command(command_parts: List[str]) -> Optional[str]:
    for part in command_parts:
        binary_name = extract_target_binary_name(part)
        if binary_name is not None and binary_name in BINARY_NAMES:
            return binary_name
    return None


def should_skip_command_processing(command_parts: List[str]) -> bool:
    return is_cmake_copy_command(command_parts) or any(
        is_script_call(part) for part in command_parts)


def instrument_for_lcov_coverage(command_parts: List[str]) -> List[str]:
    if should_skip_command_processing(command_parts):
        return command_parts

    binary_name = find_binary_in_command(command_parts)
    if binary_name is None:
        return command_parts

    gcov_dir_path = COVERAGE_ROOT_DIR / Path(binary_name)
    modified_parts = command_parts.copy()
    for i, part in enumerate(modified_parts):
        if part.startswith('LD_LIBRARY_PATH=') or is_target_binary(part):
            modified_parts.insert(i, f"GCOV_PREFIX={gcov_dir_path}")
            break
    return modified_parts


def instrument_for_llvm_coverage(command_parts: List[str]) -> List[str]:
    if should_skip_command_processing(command_parts):
        return command_parts

    binary_name = find_binary_in_command(command_parts)
    if binary_name is None:
        return command_parts

    pid = current_process().pid
    unique_id = uuid.uuid4()
    file_path = COVERAGE_ROOT_DIR / Path(f"{binary_name}-{pid}-{unique_id}")
    profraw_path = file_path.with_suffix('.profraw')
    
    modified_parts = command_parts.copy()
    for i, part in enumerate(modified_parts):
        if part.startswith('LD_LIBRARY_PATH=') or is_target_binary(part):
            modified_parts.insert(i, f"LLVM_PROFILE_FILE={profraw_path}")
            modified_parts.extend([
                generate_profdata_merge_command(profraw_path),
                f"COMMAND;rm;{profraw_path}"
            ])
            break
    return modified_parts


def process_command_section(command_parts: List[str], tool: str) -> List[str]:
    """Process COMMAND section based on the selected coverage tool."""
    processors = {
        'llvm-cov': instrument_for_llvm_coverage,
        'lcov': instrument_for_lcov_coverage
    }
    processor = processors.get(tool)
    if processor is None:
        raise ValueError(f"Unsupported coverage tool: {tool}. "
                        f"Supported tools are {list(processors.keys())}")
    return processor(command_parts)


def process_input_sections(input_string: str, tool: str) -> List[str]:
    sections = input_string.strip().split(';')
    processed_sections = []
    i = 0
    while i < len(sections):
        section = sections[i]
        if section not in ADD_CUSTOM_TARGET_FUNCTION_ARGS:
            processed_sections.append(section)
            i += 1
            continue
        if section == 'COMMAND':
            command_parts = []
            i += 1
            while i < len(sections) and sections[i] not in ADD_CUSTOM_TARGET_FUNCTION_ARGS:
                command_parts.append(sections[i])
                i += 1

            processed_command = process_command_section(command_parts, tool)
            processed_sections.extend(['COMMAND'] + processed_command)
        else:
            end = i + 1
            while end < len(sections) and sections[end] not in ADD_CUSTOM_TARGET_FUNCTION_ARGS:
                end += 1
            processed_sections.extend(sections[i:end])
            i = end
    return processed_sections


def main(tool: str, input_string: str) -> None:
    if tool not in COVERAGE_TOOLS:
        raise ValueError("Tool must be either 'llvm-cov' or 'lcov'")
    processed_sections = process_input_sections(input_string, tool)
    print(';'.join(processed_sections))


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python script.py 'tool'(llvm-cov/lcov) 'input_string'")
        sys.exit(1)
    try:
        main(sys.argv[1], sys.argv[2])
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
