#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

import os
import subprocess
import sys
import shutil
import platform
import argparse

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))

# Check command
def check_call(command):
    try:
        env = os.environ.copy()
        env["ZERO_AR_DATE"] = "1"
        return subprocess.check_call(command, shell=True, env=env)
    except subprocess.CalledProcessError as e:
        print(f"Command '{e.cmd}' returned non-zero exit status {e.returncode}.")
        return e.returncode

# Build codepm
def build(build_type, target, rpath=None):
    if not build_type:
        build_type = ""
    if not target:
        target = "native"

    # Check CODIRA_HOME
    if not os.environ.get("CODIRA_HOME"):
        print("error: cannot find CODIRA_HOME, please make sure cangjie sdk is configured.", file=sys.stderr)
        return 1

    # Check stdx lib
    if not os.environ.get("CODIRA_STDX_PATH"):
        print("error: cannot find CODIRA_STDX_PATH, please make sure stdx lib is configured.", file=sys.stderr)
        return 1

    # Check if cross compile is supported
    is_windows = platform.system() == "Windows"
    is_linux = platform.system() == "Linux"
    is_macos = platform.system() == "Darwin"
    is_cross_windows = False

    if target != "native" and not is_linux:
        print("error: cross compile is only supported from Linux to windows-x86_64.", file=sys.stderr)
        return 1
    if target == "windows-x86_64" and is_linux:
        is_cross_windows = True
        is_linux = False

    # Set rpath
    rpath_set_option = ""
    if rpath:
        if is_macos:
            rpath_set_option = f"--link-options=\"-rpath {rpath}\""
        elif is_linux:
            rpath_set_option = f"--link-options=\"--disable-new-dtags -rpath={rpath}\""

    # Set common compile option
    debug_mode = ""
    if build_type == "debug":
        debug_mode = "-g"
    elif build_type != "release":
        print("error: codepm only support 'release' and 'debug' mode of compiling.")
        return 1

    if is_windows:
        common_option = f"--trimpath={CURRENT_DIR} {debug_mode} --import-path {os.path.join(CURRENT_DIR,'bin')}"
    else:
        common_option = f"-j1 --trimpath={CURRENT_DIR} {debug_mode} --import-path {os.path.join(CURRENT_DIR,'bin')}"

    # Get codec executable file
    if is_windows:
        codec = "codec.exe"
    else:
        codec = "codec"

    # Create output directories
    os.makedirs(os.path.join(CURRENT_DIR, 'bin', 'codepm'), exist_ok=True)
    os.makedirs(os.path.join(CURRENT_DIR, '..', 'dist'), exist_ok=True)

    # Compile static libs of sub-packages
    src_dirs = ['toml', 'util', 'config', 'implement', 'command']
    for src in src_dirs:
        if is_linux or is_macos:
            returncode = check_call(f"{codec} {common_option} -p {os.path.join(CURRENT_DIR, '..', 'src', src)} --import-path {os.environ['CODIRA_STDX_PATH']} --output-type=staticlib --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o libcodepm.{src}.a")
        if is_windows:
            returncode = check_call(f"{codec} {common_option} -p {os.path.join(CURRENT_DIR, '..', 'src', src)} --import-path {os.path.join(CURRENT_DIR, 'bin')} --import-path {os.environ['CODIRA_STDX_PATH']} --output-type=staticlib --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o libcodepm.{src}.a")
        if is_cross_windows:
            returncode = check_call(f"{codec} --target=x86_64-windows-gnu {common_option} -p {os.path.join(CURRENT_DIR, '..', 'src', src)} --import-path {os.environ['CODIRA_STDX_PATH']} --output-type=staticlib --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o libcodepm.{src}.a")
        if returncode != 0:
            return returncode

    # Compile codepm executable file
    if is_linux:
        returncode = check_call(f"{codec} {common_option} {rpath_set_option} \"--link-options=-z noexecstack -z relro -z now -s\" --import-path {os.environ['CODIRA_STDX_PATH']} -L {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -lcodepm.command -lcodepm.implement -lcodepm.config -lcodepm.util -lcodepm.toml -L {os.environ['CODIRA_STDX_PATH']} -lstdx.logger -lstdx.log -lstdx.encoding.json.stream -lstdx.serialization.serialization -lstdx.encoding.json -lstdx.encoding.url -p {os.path.join(CURRENT_DIR, '..', 'src')} -O2 --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o codepm")
    if is_macos:
        returncode = check_call(f"{codec} {common_option} {rpath_set_option} --import-path {os.environ['CODIRA_STDX_PATH']} -L {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -lcodepm.command -lcodepm.implement -lcodepm.config -lcodepm.util -lcodepm.toml -L {os.environ['CODIRA_STDX_PATH']} -lstdx.logger -lstdx.log -lstdx.encoding.json.stream -lstdx.serialization.serialization -lstdx.encoding.json -lstdx.encoding.url -p {os.path.join(CURRENT_DIR, '..', 'src')} -O2 --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o codepm")
    if is_cross_windows:
        returncode = check_call(f"{codec} --target=x86_64-windows-gnu {common_option} --import-path {os.path.join(CURRENT_DIR, 'bin')} --import-path {os.environ['CODIRA_STDX_PATH']} --link-options=--no-insert-timestamp -L {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -lcodepm.command -lcodepm.implement -lcodepm.config -lcodepm.util -lcodepm.toml -L {os.environ['CODIRA_STDX_PATH']} -lstdx.logger -lstdx.log -lstdx.encoding.json.stream -lstdx.serialization.serialization -lstdx.encoding.json -lstdx.encoding.url -p {os.path.join(CURRENT_DIR, '..', 'src')} -O2 --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o codepm.exe")
    if is_windows:
        returncode = check_call(f"{codec} {common_option} --import-path {os.path.join(CURRENT_DIR, 'bin')} --import-path {os.environ['CODIRA_STDX_PATH']} --link-options=--no-insert-timestamp -L {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -lcodepm.command -lcodepm.implement -lcodepm.config -lcodepm.util -lcodepm.toml -L {os.environ['CODIRA_STDX_PATH']} -lstdx.logger -lstdx.log -lstdx.encoding.json.stream -lstdx.serialization.serialization -lstdx.encoding.json -lstdx.encoding.url -p {os.path.join(CURRENT_DIR, '..', 'src')} -O2 --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codepm')} -o codepm.exe")

    if returncode != 0:
        return returncode

    if is_windows or is_cross_windows:
        shutil.copy(os.path.join(CURRENT_DIR, 'bin', 'codepm', 'codepm.exe'), os.path.join(CURRENT_DIR, '..', 'dist'))
    else:
        shutil.copy(os.path.join(CURRENT_DIR, 'bin', 'codepm', 'codepm'), os.path.join(CURRENT_DIR, '..', 'dist'))

    print("Successfully build codepm!")
    return 0

# Install codepm
def install(prefix):
    if not os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm')) \
        and not os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm.exe')):
        print("error: no codepm output, please run command 'build' first.")
        return 1

    if prefix:
        os.makedirs(os.path.abspath(prefix), exist_ok=True)
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm'), os.path.abspath(prefix))
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm.exe')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'dist', 'codepm.exe'), os.path.abspath(prefix))

    print("Successfully install codepm!")
    return 0

# Clean output of build
def clean():
    if os.path.exists(os.path.join(CURRENT_DIR, 'bin')):
        shutil.rmtree(os.path.join(CURRENT_DIR, 'bin'))

    if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist')):
        shutil.rmtree(os.path.join(CURRENT_DIR, '..', 'dist'))

    print("Successfully clean codepm!")
    return 0

def main():
    parser = argparse.ArgumentParser(description='Build system')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Build command
    build_parser = subparsers.add_parser('build', help='Build codepm')
    build_parser.add_argument('-t', '--build-type', type=str, dest='build_type', help='Specify build type', required=True)
    build_parser.add_argument('--target', type=str, dest='target', help='Specify build target')
    build_parser.add_argument('--set-rpath', type=str, dest='rpath', help='Set rpath value')

    # Install command
    install_parser = subparsers.add_parser('install', help='Install codepm')
    install_parser.add_argument('--prefix', help='Specify installation prefix')

    # Clean command
    subparsers.add_parser('clean', help='Clean build files')

    args = parser.parse_args()

    if args.command == 'build':
        return build(build_type=args.build_type, target=args.target, rpath=args.rpath)
    elif args.command == 'install':
        return install(prefix=args.prefix)
    elif args.command == 'clean':
        return clean()
    else:
        parser.print_help()
        return 0

if __name__ == '__main__':
    main()