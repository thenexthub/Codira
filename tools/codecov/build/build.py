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

# Build codecov
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
    IS_WINDOWS = platform.system() == "Windows"
    IS_LINUX = platform.system() == "Linux"
    IS_MACOS = platform.system() == "Darwin"
    IS_CROSS_WINDOWS = False

    if target != "native" and not IS_LINUX:
        print("error: cross compile is only supported from Linux to windows-x86_64.", file=sys.stderr)
        return 1
    if target == "windows-x86_64" and IS_LINUX:
        IS_CROSS_WINDOWS = True
        IS_LINUX = False

    # Set rpath
    RPATH_SET_OPTION = ""#= "\$ORIGIN/../../runtime/lib/linux_$(arch)_codenative"
    if rpath:
        if IS_MACOS:
            RPATH_SET_OPTION = f"--link-options=\"-rpath {rpath}\""
        elif IS_LINUX:
            RPATH_SET_OPTION = f"--link-options=\"--disable-new-dtags -rpath={rpath}\""

    # Set common compile option
    DEBUG_MODE = ""
    if build_type == "debug":
        DEBUG_MODE = "-g"
    elif build_type != "release":
        print("error: codecov only support 'release' and 'debug' mode of compiling.")
        return 1
    # Get codec executable file
    if IS_WINDOWS:
        CODEC = "codec.exe"
    else:
        CODEC = "codec"

    # Create output directories
    os.makedirs(os.path.join(CURRENT_DIR, 'bin', 'codecov'), exist_ok=True)
    os.makedirs(os.path.join(CURRENT_DIR, '..', 'dist'), exist_ok=True)

    # Compile static libs of sub-packages
    if IS_WINDOWS:
        COMMON_OPTION = f"-j1 --trimpath={CURRENT_DIR} {DEBUG_MODE} --import-path {os.path.join(CURRENT_DIR,'bin')} --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codecov')} --output-type=staticlib"
    else:
        COMMON_OPTION = f"-j1 --trimpath={CURRENT_DIR} {DEBUG_MODE} --import-path {os.path.join(CURRENT_DIR,'bin')} --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codecov')} --output-type=staticlib"

    if IS_LINUX or IS_MACOS:
        print(f"{CODEC} {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/util')} -o libcodecov.util.a")
        returncode1 = check_call(f"{CODEC} {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/util')} -o libcodecov.util.a")
        returncode2 = check_call(f"{CODEC} {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/core')} --import-path {os.environ['CODIRA_STDX_PATH']} -o libcodecov.core.a")
    if IS_WINDOWS:
        returncode1 = check_call(f"{CODEC} {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/util')} -o libcodecov.util.a")
        returncode2 = check_call(f"{CODEC} {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/core')} --import-path {os.environ['CODIRA_STDX_PATH']} -o libcodecov.core.a")
    if IS_CROSS_WINDOWS:
        returncode1 = check_call(f"{CODEC} --target=x86_64-windows-gnu {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/util')} -o libcodecov.util.a")
        returncode2 = check_call(f"{CODEC} --target=x86_64-windows-gnu {COMMON_OPTION} -p {os.path.join(CURRENT_DIR, '..', 'src/core')} --import-path {os.environ['CODIRA_STDX_PATH']} -o libcodecov.core.a")
    if returncode1 != 0:
        return returncode1
    if returncode2 != 0:
        return returncode2

    # Compile codecov executable file
    COMMON_OPTION2 = f"-j1 --trimpath={CURRENT_DIR} {DEBUG_MODE} --import-path {os.path.join(CURRENT_DIR,'bin')} --import-path {os.environ['CODIRA_STDX_PATH']} --output-dir {os.path.join(CURRENT_DIR, 'bin', 'codecov')}"
    if IS_WINDOWS:
        STDX_LINKS=f"-L %CODIRA_STDX_PATH% -l:libstdx.logger.a -l:libstdx.log.a -l:libstdx.encoding.json.stream.a -l:libstdx.serialization.serialization.a -l:libstdx.encoding.json.a"
    if IS_MACOS:
        STDX_LINKS=f"$CODIRA_STDX_PATH/libstdx.logger.a $CODIRA_STDX_PATH/libstdx.log.a $CODIRA_STDX_PATH/libstdx.encoding.json.stream.a $CODIRA_STDX_PATH/libstdx.serialization.serialization.a $CODIRA_STDX_PATH/libstdx.encoding.json.a"
    else:
        STDX_LINKS=f"-L $CODIRA_STDX_PATH -l:libstdx.logger.a -l:libstdx.log.a -l:libstdx.encoding.json.stream.a -l:libstdx.serialization.serialization.a -l:libstdx.encoding.json.a"
    
    if IS_LINUX:
        returncode = check_call(f"{CODEC} {COMMON_OPTION2} {RPATH_SET_OPTION} \"--link-options=-z noexecstack -z relro -z now -s\" -L {os.path.join(CURRENT_DIR, 'bin', 'codecov')} -L bin/codecov -lcodecov.util -lcodecov.core {STDX_LINKS} -p {os.path.join(CURRENT_DIR, '..', 'src')}  -o codecov")
    if IS_MACOS:
        returncode = check_call(f"{CODEC} {COMMON_OPTION2} {RPATH_SET_OPTION} --import-path {os.environ['CODIRA_STDX_PATH']} -L {os.path.join(CURRENT_DIR, 'bin', 'codecov')} -L bin/codecov -lcodecov.util -lcodecov.core  {STDX_LINKS} -p {os.path.join(CURRENT_DIR, '..', 'src')} -O2 -o codecov")
    if IS_CROSS_WINDOWS:
        returncode = check_call(f"{CODEC} --target=x86_64-windows-gnu {COMMON_OPTION2} --link-options=--no-insert-timestamp -L {os.path.join(CURRENT_DIR, 'bin', 'codecov')} -L bin/codecov -lcodecov.util -lcodecov.core {STDX_LINKS} -p {os.path.join(CURRENT_DIR, '..', 'src')} -o codecov.exe")
    if IS_WINDOWS:
        returncode = check_call(f"{CODEC} {COMMON_OPTION2}  --link-options=--no-insert-timestamp -L {os.path.join(CURRENT_DIR, 'bin', 'codecov')} -lcodecov.util -lcodecov.core {STDX_LINKS} -p {os.path.join(CURRENT_DIR, '..', 'src')} -o codecov.exe")

    if returncode != 0:
        return returncode

    if IS_WINDOWS or IS_CROSS_WINDOWS:
        shutil.copy(os.path.join(CURRENT_DIR, 'bin', 'codecov', 'codecov.exe'), os.path.join(CURRENT_DIR, '..', 'dist'))
    else:
        shutil.copy(os.path.join(CURRENT_DIR, 'bin', 'codecov', 'codecov'), os.path.join(CURRENT_DIR, '..', 'dist'))

    print("Successfully build codecov!")
    return 0

# Install codecov
def install(prefix):
    if not os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov')) \
        and not os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov.exe')):
        print("error: no codecov output, please run command 'build' first.")
        return 1

    if prefix:
        os.makedirs(os.path.abspath(prefix), exist_ok=True)
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov'), os.path.abspath(prefix))
        if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov.exe')):
            shutil.copy(os.path.join(CURRENT_DIR, '..', 'dist', 'codecov.exe'), os.path.abspath(prefix))

    print("Successfully install codecov!")
    return 0

# Clean output of build
def clean():
    if os.path.exists(os.path.join(CURRENT_DIR, 'bin')):
        shutil.rmtree(os.path.join(CURRENT_DIR, 'bin'))

    if os.path.exists(os.path.join(CURRENT_DIR, '..', 'dist')):
        shutil.rmtree(os.path.join(CURRENT_DIR, '..', 'dist'))

    print("Successfully clean codecov!")
    return 0

def main():
    parser = argparse.ArgumentParser(description='Build system')
    subparsers = parser.add_subparsers(dest='command', help='Available commands')

    # Build command
    build_parser = subparsers.add_parser('build', help='Build codecov')
    build_parser.add_argument('-t', '--build-type', type=str, dest='build_type', help='Specify build type', required=True)
    build_parser.add_argument('--target', type=str, dest='target', help='Specify build target')
    build_parser.add_argument('--set-rpath', type=str, dest='rpath', help='Set rpath value')

    # Install command
    install_parser = subparsers.add_parser('install', help='Install codecov')
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