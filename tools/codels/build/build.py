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

"""cangjie language server build entry"""

import argparse
import os
import platform
import shutil
import stat
import subprocess
from subprocess import PIPE
from pathlib import Path
from enum import Enum

HOME_DIR = os.path.dirname(Path(__file__).resolve().parent)
BUILD_DIR = os.path.join(HOME_DIR, "build-lsp")
THIRDPARTY_DIR = os.path.join(HOME_DIR, "third_party")
LIB_DIR = os.path.join(HOME_DIR, "src", "lib")
CODIRA_LIB_DIR = os.path.join(LIB_DIR, "cangjie", "lib", "codenative")
OUTPUT_DIR = os.path.join(HOME_DIR, "output", "bin")

BUILD_TYPE_MAP = {
    "release": "Release",
    "debug": "Debug",
    "relwithdebinfo": "RelWithDebInfo"
}

class TARGET_SYSTEM_TYPE(Enum):
    NATIVE = "native"
    WINDOWS_X86_64 = "windows-x86_64"
TARGET_SYSTEM_MAP = {
    "native": TARGET_SYSTEM_TYPE.NATIVE,
    "windows-x86_64": TARGET_SYSTEM_TYPE.WINDOWS_X86_64
}
TARGET_SYSTEM = TARGET_SYSTEM_TYPE.NATIVE

IS_WINDOWS = platform.system() == "Windows"
IS_MACOS = platform.system() == "Darwin"
IS_LINUX = platform.system() == "Linux"
JSON_GIT = "https://gitcode.com/openharmony/third_party_json.git"
FLATBUFFER_GIT = "https://gitcode.com/openharmony/third_party_flatbuffers.git"
SQLITE_GIT = "https://gitcode.com/openharmony/third_party_sqlite.git"

def resolve_path(path):
    if os.path.isabs(path):
        return path
    return os.path.abspath(path)
def download_json():
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", JSON_GIT, "json-v3.11.3"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)


def download_flatbuffers(args):
    cmd = ["git", "clone", "-b", "master", FLATBUFFER_GIT, "flatbuffers"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)
    checkout_cmd = ["git", "checkout", "8355307828c7a6bc6bae9d2dba48ad3ab0b5ff2d"]
    flatbuffers_dir = os.path.join(THIRDPARTY_DIR, "flatbuffers")
    subprocess.run(checkout_cmd, cwd=flatbuffers_dir, check=True)


def generate_flat_header():
    env = os.environ.copy()
    env["ZERO_AR_DATE"] = "1"
    index_file = os.path.join(HOME_DIR, "generate", "index.fbs")
    flatbuffers_dir = os.path.join(THIRDPARTY_DIR, "flatbuffers")
    new_index_path = os.path.join(flatbuffers_dir, "include", "index.fbs")
    shutil.copy(index_file, new_index_path)

    flatbuffers_build_dir = os.path.join(flatbuffers_dir, "build")
    if not os.path.exists(flatbuffers_build_dir):
        os.makedirs(flatbuffers_build_dir)
    compile_cmd = ["cmake", flatbuffers_dir, "-G", get_generator(), "-DFLATBUFFERS_BUILD_TESTS=OFF"]
    subprocess.run(compile_cmd, cwd=flatbuffers_build_dir, check=True, env=env)
    build_cmd = ["cmake", "--build", flatbuffers_build_dir, "-j8"]
    subprocess.run(build_cmd, cwd=flatbuffers_build_dir, check=True, env=env)
    flatc_binary = "flatc"
    if IS_WINDOWS:
        flatc_binary = "flatc.exe"
    flac_cmd = os.path.join(flatbuffers_build_dir, flatc_binary)
    generate_cmd = [flac_cmd, "--cpp", index_file]
    subprocess.run(generate_cmd, cwd=flatbuffers_build_dir, check=True, env=env)
    header_path = os.path.join(flatbuffers_build_dir, "index_generated.h")
    new_header_path = os.path.join(flatbuffers_dir, "include", "index_generated.h")
    shutil.copy(header_path, new_header_path)

def download_sqlite(args):
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", SQLITE_GIT, "sqlite3"]
    subprocess.run(cmd, cwd=THIRDPARTY_DIR, check=True)

def build_sqlite_amalgamation():
    sqlite_dir = os.path.join(THIRDPARTY_DIR, "sqlite3")
    amalgamation_dir = os.path.join(sqlite_dir, "amalgamation")
    if not os.path.exists(amalgamation_dir):
        os.makedirs(amalgamation_dir)
    shell_c_file = os.path.join(sqlite_dir, "src", "shell.c")
    sqlite3_c_file = os.path.join(sqlite_dir, "src", "sqlite3.c")
    sqlite3_h_file = os.path.join(sqlite_dir, "include", "sqlite3.h")
    sqlite3ext_h_file = os.path.join(sqlite_dir, "include", "sqlite3ext.h")
    new_shell_c_file = os.path.join(amalgamation_dir, "shell.c")
    nwe_sqlite3_c_file = os.path.join(amalgamation_dir, "sqlite3.c")
    nwe_sqlite3_h_file = os.path.join(amalgamation_dir, "sqlite3.h")
    nwe_sqlite3ext_h_file = os.path.join(amalgamation_dir, "sqlite3ext.h")
    shutil.copy(shell_c_file, new_shell_c_file)
    shutil.copy(sqlite3_c_file, nwe_sqlite3_c_file)
    shutil.copy(sqlite3_h_file, nwe_sqlite3_h_file)
    shutil.copy(sqlite3ext_h_file, nwe_sqlite3ext_h_file)

def prepare_cangjie(args):
    cangjie_sdk_path = resolve_path(os.getenv("CODIRA_HOME"))
    if not os.path.exists(LIB_DIR):
        os.makedirs(LIB_DIR)
    if not os.path.exists(CODIRA_LIB_DIR):
        os.makedirs(CODIRA_LIB_DIR)
    if os.path.exists(cangjie_sdk_path):
        # copy header files
        cangjie_header_dir = os.path.join(cangjie_sdk_path, "include")
        new_header_dir = os.path.join(LIB_DIR, "cangjie", "include")
        shutil.copytree(cangjie_header_dir, new_header_dir, dirs_exist_ok=True)
        # copy libraries
        tools_lib_dir = os.path.join(cangjie_sdk_path, "tools", "lib")
        tools_bin_dir = os.path.join(cangjie_sdk_path, "tools", "bin")
        file_extension = ""
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            file_extension = "dll"
        if IS_LINUX and not TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            file_extension = "so"
        if IS_MACOS:
            file_extension = "dylib"
        cangjie_lib_path = os.path.join(tools_lib_dir, "libcangjie-lsp." + file_extension)
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            cangjie_lib_path = os.path.join(tools_bin_dir, "libcangjie-lsp." + file_extension)
        new_lib_path = os.path.join(CODIRA_LIB_DIR, "libcangjie-lsp." + file_extension)
        shutil.copy(cangjie_lib_path, new_lib_path)
        if IS_WINDOWS or TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
            cangjie_lib_path = os.path.join(tools_lib_dir, "libcangjie-lsp.dll.a")
            new_lib_path = os.path.join(CODIRA_LIB_DIR, "libcangjie-lsp.dll.a")
            shutil.copy(cangjie_lib_path, new_lib_path)

def prepare_build(args):
    prepare_cangjie(args)
    if not os.path.exists(THIRDPARTY_DIR):
        os.makedirs(THIRDPARTY_DIR)
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "json-v3.11.3")):
        download_json()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "flatbuffers")):
        download_flatbuffers(args)
        generate_flat_header()
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "sqlite3")):
        download_sqlite(args)
        build_sqlite_amalgamation()

def get_generator():
    generator = "Unix Makefiles"
    if IS_WINDOWS:
        generator = "MinGW Makefiles"
    return generator

def get_compiler_type():
    compiler_type = "Ninja"
    if IS_WINDOWS:
        compiler_type = "MINGW"
    return compiler_type

def get_build_commands(args):
    result = [
        "cmake",
        "--build",
        BUILD_DIR
    ]
    if args.jobs > 0:
        result.extend(["-j", str(args.jobs)])
    return result

def generate_cmake_commands(args):
    # cmake .. -G "MinGW Makefiles" -DCOMPILER_TYPE=MINGW -DCMAKE_BUILD_TYPE=Release
    result = [
        "cmake", HOME_DIR,
        "-G", get_generator(),
        "-DCOMPILER_TYPE=" + get_compiler_type(),
        "-DCMAKE_BUILD_TYPE=" + BUILD_TYPE_MAP[args.build_type]
    ]
    if TARGET_SYSTEM == TARGET_SYSTEM_TYPE.WINDOWS_X86_64:
        result.extend([
            "-DCROSS_WINDOWS=ON",
            "-DCMAKE_SYSTEM_NAME=Windows",
            "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
            "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++"
        ])
    if args.test:
        result.extend([
            "-DENABLE_TEST=ON"
        ])
    return result

def build(args):
    env = os.environ.copy()
    env["ZERO_AR_DATE"] = "1"
    print("start build")
    if os.getenv("CODIRA_HOME") is None:
        print("build failed, 'CODIRA_HOME' environment variable not found, please config it first.")
        return
    if args.target_system is not None:
        global TARGET_SYSTEM
        TARGET_SYSTEM = TARGET_SYSTEM_MAP[args.target_system]
    prepare_build(args)
    cmake_command = generate_cmake_commands(args)
    build_command = get_build_commands(args)
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)
        subprocess.run(cmake_command, cwd=BUILD_DIR, check=True, env=env)
    print(build_command)
    subprocess.run(build_command, cwd=BUILD_DIR, check=True, env=env)
    print("end build")

def redo_with_write(redo_func, path, err):
    # Is the error an access error?
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        redo_func(path)
    else:
        raise

def delete_folder(folder_path):
    try:
        shutil.rmtree(folder_path, onerror=redo_with_write)
    except Exception as e:
        print(f"delete {folder_path} failed: {str(e)}")

def clean(args):
    """
    clean target
    output build-lsp
    third_party/json-v3.11.3
    third_party/flatbuffers
    third_party/sqlite3
    src/lib
    """
    print("start clean")
    delete_folders = ["output", "build-lsp"]
    for folder in delete_folders:
        folder = os.path.join(HOME_DIR, folder)
        if os.path.exists(folder):
            delete_folder(os.path.join(HOME_DIR, folder))
    if os.path.exists(THIRDPARTY_DIR):
        delete_folder(THIRDPARTY_DIR)
    third_party_folders = ["json-v3.11.3", "flatbuffers", "sqlite3"]
    for folder in third_party_folders:
        folder = os.path.join(THIRDPARTY_DIR, folder)
        if os.path.exists(folder):
            delete_folder(os.path.join(THIRDPARTY_DIR, folder))
    if os.path.exists(LIB_DIR):
        delete_folder(LIB_DIR)
    print("end clean")

def install(args):
    binary_path = os.path.join(HOME_DIR, "output", "bin")
    if not os.path.exists(binary_path):
        print("install failed, binary not found, please run build first.")
        return
    if args is None or args.install_path is None:
        print(f"no path is specified, the binary is in the '{binary_path}' path.")
        return
    install_path = os.path.abspath(args.install_path)
    if not os.path.exists(install_path):
        print("install failed, target install path does not exist.")
        return
    if not os.path.isdir(install_path):
        print("install failed, target install path is not a folder.")
        return
    shutil.copytree(binary_path, install_path, dirs_exist_ok=True)
    print(f"target path '{install_path}' installed successfully.")

def get_run_test_command(cangjie_sdk_path):
    env_file = "envsetup.sh"
    gtest_file = "gtest_LSPServer_test"
    if IS_WINDOWS:
        env_file = "envsetup.bat"
        gtest_file = "gtest_LSPServer_test.exe"
    env_path = os.path.join(resolve_path(cangjie_sdk_path), env_file)
    test_path = os.path.join(OUTPUT_DIR, gtest_file)
    result = []
    if not IS_WINDOWS:
        result.extend(["bash", "-c", "source " + env_path + " && " + test_path])
    else:
        result.extend([env_path, "&&", test_path])
    return result

def test(args):
    env = os.environ.copy()
    env["ZERO_AR_DATE"] = "1"
    print("start run test")
    cangjie_sdk_path = resolve_path(os.getenv("CODIRA_HOME"))
    if not os.path.exists(cangjie_sdk_path):
        print("set cangjie sdk path not exists")
        return
    if not os.path.exists(OUTPUT_DIR):
        print("no output/bin path")
        return
    commands = get_run_test_command(cangjie_sdk_path)
    output = subprocess.run(commands, cwd=OUTPUT_DIR, check=True, env=env)
    if output.returncode != 0:
        print("test failed with return code:", output.returncode)
        exit(1)
    print("end run test")

def main():
    """build entry"""
    parser = argparse.ArgumentParser(description="build lspserver project")
    subparsers = parser.add_subparsers(help="sub command help")
    parser_build = subparsers.add_parser("build", help=" build cangjie")
    parser_build.add_argument(
        "-t",
        "--build-type",
        choices=["release", "debug", "relwithdebinfo"],
        dest="build_type",
        type=str,
        default="release",
        help="select target build type"
    )
    parser_build.add_argument(
        "-j", "--jobs", dest="jobs", type=int, default=0, help="run N jobs in parallel"
    )
    parser_build.add_argument(
        "--target",
        choices=["native", "windows-x86_64"],
        dest="target_system",
        type=str,
        default="native",
        help="set build target system"
    )
    parser_build.add_argument(
        "--test",
        action="store_true",
        dest="test",
        help="set for test LSP"
    )
    parser_build.set_defaults(func=build)

    parser_clean = subparsers.add_parser("clean", help="clean build")
    parser_clean.set_defaults(func=clean)

    parser_install = subparsers.add_parser("install", help="install binary")
    parser_install.add_argument(
        "--prefix",
        dest="install_path",
        type=str,
        help="set installation path"
    )
    parser_install.set_defaults(func=install)

    parser_test = subparsers.add_parser("test", help="run test case")
    parser_test.set_defaults(func=test)

    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    os.environ["LANG"] = "C.UTF-8"
    main()