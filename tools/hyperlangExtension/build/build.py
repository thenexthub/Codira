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

# Check CODIRA_HOME
if not os.environ.get("CODIRA_HOME"):
    print("error: cannot find CODIRA_HOME, please make sure cangjie sdk is configured", file=sys.stderr)
    sys.exit(1)

if not os.environ.get("CODIRA_STDX_PATH"):
    print("error: cannot find CODIRA_STDX_PATH, please make sure stdx lib is configured", file=sys.stderr)
    sys.exit(1)

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))

IS_WINDOWS = platform.system() == "Windows"
IS_LINUX = platform.system() == "Linux"
IS_MACOS = platform.system() == "Darwin"
IS_CROSS_WINDOWS = False
SUPPORTED_BUILD_TYPE = ["debug", "release"]
SUPPORTED_TARGET = ["native", "windows-x86_64"]

# Check command
def check_call(command):
    try:
        env = os.environ.copy()
        env["ZERO_AR_DATE"] = "1"
        return subprocess.check_call(command, env=env)
    except subprocess.CalledProcessError as e:
        print(f"Command '{e.cmd}' returned non-zero exit status {e.returncode}.")
        sys.exit(e.returncode) 

# Build HLE
def build(args):
    target = args.target
    build_type = args.build_type

    global IS_LINUX
    global IS_CROSS_WINDOWS
    if target != "native" and not IS_LINUX:
        print("error: cross compile is only supported from Linux to windows-x86_64.", file=sys.stderr)
        return 1
    if target == "windows-x86_64" and IS_LINUX:
        IS_CROSS_WINDOWS = True
        IS_LINUX = False

    # Set common compile option
    DEBUG_MODE = ""
    if build_type == "debug":
        DEBUG_MODE = "-g"

    if IS_WINDOWS: 
        COMMON_OPTION = ['codec.exe', f"--trimpath={CURRENT_DIR} {DEBUG_MODE}", '--import-path', f"{os.path.join(CURRENT_DIR, '..', 'target')}"]
    else:
        COMMON_OPTION = ['codec', '-j1', f"--trimpath={CURRENT_DIR} {DEBUG_MODE}", '--import-path', f"{os.path.join(CURRENT_DIR, '..', 'target')}"]
    
    # Create output directories
    os.makedirs(os.path.join(CURRENT_DIR, '..', 'target', 'bin'), exist_ok=True)
    os.makedirs(os.path.join(CURRENT_DIR, '..', 'target', 'hle'), exist_ok=True)

    src_dirs = ['dtsparser', 'tool', 'entry']
    for src in src_dirs:
        if IS_LINUX or IS_MACOS:
            returncode = check_call(COMMON_OPTION + ['-O2', '--strip-all', '--no-sub-pkg', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src', src)}", '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '--output-type=staticlib', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-o', f"libhle.{src}.a"])
        if IS_WINDOWS:
            returncode = check_call(COMMON_OPTION + ['-O2', '--strip-all', '--no-sub-pkg', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src', src)}", '--import-path', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '--output-type=staticlib', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-o', f"libhle.{src}.a"])
        if IS_CROSS_WINDOWS:
            returncode = check_call(COMMON_OPTION + ['--target=x86_64-windows-gnu', '-O2', '--strip-all', '--no-sub-pkg', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src', src)}", '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '--output-type=staticlib', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-o', f"libhle.{src}.a"])
        if returncode != 0: 
            print(f"build {src} failed")
            return returncode
        
    if IS_LINUX:
        returncode = check_call(COMMON_OPTION + ['--link-options=-z noexecstack -z relro -z now -s', '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '-L', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-lhle.dtsparser', '-lhle.tool', '-lhle.entry', '-L', f"{os.environ['CODIRA_STDX_PATH']}", '-lstdx.logger', '-lstdx.log', '-lstdx.encoding.json.stream', '-lstdx.serialization.serialization', '-lstdx.encoding.json', '-lstdx.encoding.url', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src')}", '-O2', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'bin')}", '-o', 'main'])
    if IS_MACOS:
        returncode = check_call(COMMON_OPTION + ['--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '-L', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-lhle.dtsparser', '-lhle.tool', '-lhle.entry', '-L', f"{os.environ['CODIRA_STDX_PATH']}", '-lstdx.logger', '-lstdx.log', '-lstdx.encoding.json.stream', '-lstdx.serialization.serialization', '-lstdx.encoding.json', '-lstdx.encoding.url', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src')}", '-O2', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'bin')}", '-o', 'main'])
    if IS_CROSS_WINDOWS:
        returncode = check_call(COMMON_OPTION + ['--target=x86_64-windows-gnu', '--import-path', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '--link-options=--no-insert-timestamp', '-L', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-lhle.dtsparser', '-lhle.tool', '-lhle.entry', '-L', f"{os.environ['CODIRA_STDX_PATH']}", '-lstdx.logger', '-lstdx.log', '-lstdx.encoding.json.stream', '-lstdx.serialization.serialization', '-lstdx.encoding.json', '-lstdx.encoding.url', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src')}", '-O2', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'bin')}", '-o', 'main.exe'])
    if IS_WINDOWS:
        returncode = check_call(COMMON_OPTION + ['--import-path', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '--import-path', f"{os.environ['CODIRA_STDX_PATH']}", '--link-options=--no-insert-timestamp', '-L', f"{os.path.join(CURRENT_DIR, '..', 'target', 'hle')}", '-lhle.dtsparser', '-lhle.tool', '-lhle.entry', '-L', f"{os.environ['CODIRA_STDX_PATH']}", '-lstdx.logger', '-lstdx.log', '-lstdx.encoding.json.stream', '-lstdx.serialization.serialization', '-lstdx.encoding.json', '-lstdx.encoding.url', '-p', f"{os.path.join(CURRENT_DIR, '..', 'src')}", '-O2', '--output-dir', f"{os.path.join(CURRENT_DIR, '..', 'target', 'bin')}", '-o', 'main.exe'])
    if returncode != 0: 
        return returncode

    print("Successfully build HLE!")

# Install HLE
def install(args):
    prefix = args.prefix
    if not os.path.exists(os.path.join(CURRENT_DIR, '..', 'target')):
        print("error: no HLE output, please run command 'build' first.")
        return 1

    if prefix:
        os.makedirs(os.path.abspath(prefix), exist_ok=True)
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'target', 'bin', 'main')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'target', 'bin', 'main'), os.path.join(os.path.abspath(prefix), 'hle'))
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'target', 'bin', 'main.exe')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'target', 'bin', 'main.exe'), os.path.join(os.path.abspath(prefix), 'hle.exe'))
    print("Successfully install HLE!")

# Clean output of build
def clean():
    if os.path.exists(os.path.join(CURRENT_DIR, '..', 'target')):
        shutil.rmtree(os.path.join(CURRENT_DIR, '..', 'target'))
    print("Successfully clean HLE!")

def main():
    # set options
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Build command
    build_parser = subparsers.add_parser('build', help='Build HLE')
    build_parser.add_argument('-t', '--build-type', default='release', choices=list(SUPPORTED_BUILD_TYPE), help='Select target build type')
    build_parser.add_argument('--target', default='native', choices=list(SUPPORTED_TARGET), help='Specify the target platform for cross compilation')

    # Install command
    install_parser = subparsers.add_parser('install', help='Install HLE')
    install_parser.add_argument('--prefix', help='Specify installation prefix')

    # Clean command
    subparsers.add_parser('clean', help='Clean build files')

    args = parser.parse_args()
    if args.command == 'build':
        return build(args)
    elif args.command == 'install':
        return install(args)
    elif args.command == 'clean':
        return clean()
    else:
        parser.print_help()
        return 0

if __name__ == '__main__':
    main()