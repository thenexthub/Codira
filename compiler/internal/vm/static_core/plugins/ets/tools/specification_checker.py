#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

import argparse

import re
from pathlib import Path

import tempfile
import subprocess
import os

"""

A tool to verify the correctness of code examples in ArkTs documentation
which is built using reStructuredText files in the specified folder.
A folder containing reStructuredText files must be specified as an
argument to the script in order to check the included code examples for correctness.

To skip verification for a specific code example add :donotcompile: option to
the code-block directive in reStructuredText file. Also code example language
can be specified(default is ArkTS) by adding :language-to-compile: option to
the directive, but for now only ArkTS language is supported.

"""


def ensure_file(*args, err: str = "") -> str:
    f = os.path.join(*args)
    if not os.path.isfile(f):
        raise RuntimeError(f'File "{f}" not found! {err}')
    return str(f)


def extract_code_blocks(text, default_language="ArkTS"):
    def parse_options(start_index):
        lang = default_language
        skip = False
        i = start_index

        while i < len(lines) and lines[i].lstrip().startswith(":"):
            option_line = lines[i].strip()
            if option_line.lower() == ":donotcompile:":
                skip = True
            match_lang = re.match(
                r":language-to-compile:\s*(\S+)", option_line, re.IGNORECASE
            )
            if match_lang:
                lang = match_lang.group(1)
            i += 1

        return lang, skip, i

    def extract_code(start_index):
        code = []
        i = start_index
        while i < len(lines):
            line = lines[i]
            if line.strip() == "":
                code.append("")
                i += 1
            elif re.match(r"^[ \t]{2,}", line):
                code.append(line)
                i += 1
            else:
                break
        return code, i

    lines = text.splitlines()
    blocks = []
    i = 0

    while i < len(lines):
        if not re.match(r"\.\. code-block::", lines[i]):
            i += 1
            continue

        i += 1
        lang, skip_block, i = parse_options(i)

        if skip_block or i >= len(lines) or lines[i].strip() != "":
            continue

        i += 1
        code_lines, i = extract_code(i)

        if code_lines:
            blocks.append((lang, "\n".join(code_lines)))

    return blocks


def check_code(panda_root, language, code):
    languages = {"ArkTS"}

    if language not in languages:
        return 0

    if language == "ArkTS":
        abc_file = compile_ets_code(panda_root, code)

        if abc_file is None:
            return 1

        return run_ets_code(panda_root, abc_file)

    return 0


def compile_ets_code(panda_root, code):
    es2panda = ensure_file(panda_root, "bin", "es2panda")

    with tempfile.NamedTemporaryFile(mode="w", suffix=".ets", delete=False) as source:
        source.write(code)
    source_path = source.name
    output = str(Path(source_path).with_suffix(".abc"))

    env = os.environ.copy()
    env["LD_LIBRARY_PATH"] = os.path.join(panda_root, "lib")

    command = [
        es2panda,
        "--extension=ets",
        "--output=" + output,
        "--opt-level=2",
        source_path,
    ]

    result = subprocess.run(command, capture_output=True, text=True, env=env)

    os.remove(source_path)
    if result.returncode != 0:
        print("Compilation failed:")
        print(result.stdout)
        return None
    else:
        return output


def run_ets_code(panda_root, abc_file):
    ark = ensure_file(panda_root, "bin", "ark")

    etsstdlib = ensure_file(panda_root, "plugins", "ets", "etsstdlib.abc")

    env = os.environ.copy()
    env["LD_LIBRARY_PATH"] = os.path.join(panda_root, "lib")

    command = [
        ark,
        "--compiler-enable-jit=false",
        "--boot-panda-files=" + etsstdlib,
        "--load-runtimes=ets",
        abc_file,
        Path(abc_file).stem + ".ETSGLOBAL::main",
    ]

    result = subprocess.run(command, capture_output=True, text=True, env=env)
    if result.returncode != 0:
        print("Ark runtime failed:")
        print(result.stdout)

    os.remove(abc_file)
    return result.returncode


def process_file(panda_root, file_path):
    with file_path.open("r", encoding="utf-8") as file:
        content = file.read()
        extracted_code = extract_code_blocks(content)
        for lang, code in extracted_code:
            result = check_code(panda_root, lang, code)
            if result != 0:
                print(f"Check failed in {file_path}")
                raise RuntimeError(f"Check failed in {file_path}")


def process_folder(panda_root, folder_path):
    if not folder_path.is_dir():
        print("Folder " + str(folder_path) + " does not exist")
        raise RuntimeError("Folder " + str(folder_path) + " does not exist")
    for file_path in folder_path.rglob("*.rst"):
        process_file(panda_root, file_path)


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--spec-folder", help="Path to ArkTS specification")
    args = parser.parse_args()

    return args


def main():
    panda_root = os.environ.get("PANDA_ROOT")

    if panda_root is None:
        raise RuntimeError(f"PANDA_ROOT is not set")

    args = get_args()

    folder_path = Path(args.spec_folder)

    if folder_path is None:
        print("Documentation folder is not specified")

    process_folder(panda_root, folder_path)


if __name__ == "__main__":
    main()
