#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

"""CodiraCodeCheck build entry"""
import argparse
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
from enum import Enum
from logging.handlers import TimedRotatingFileHandler
from subprocess import PIPE

THREADS_NUM = multiprocessing.cpu_count() // 2
FAIL_CASE_NUM = 0
PASS_CASE_NUM = 0

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
EXECUTE_DIR = os.path.join(BUILD_DIR, "build", "bin")
THIRDPARTY_DIR = os.path.join(HOME_DIR, "third_party")
CMAKE_BUILD_DIR = os.path.join(BUILD_DIR, "build")
CMAKE_OUTPUT_DIR = os.path.join(HOME_DIR, "dist")
OUTPUT_BIN_DIR = os.path.join(CMAKE_OUTPUT_DIR, "bin")
LOG_DIR = os.path.join(BUILD_DIR, "logs")
LOG_FILE = os.path.join(LOG_DIR, "codelint.log")

IS_WINDOWS = platform.system() == "Windows"
IS_MACOS = platform.system() == "Darwin"
ARCH_NAME = (
    platform.machine().replace("AMD64", "x86_64").replace("arm64", "aarch64").lower()
)
CAN_RUN_MAPLE = platform.system() == "Linux" and platform.machine() == "x86_64"

JSON_GIT = "https://gitcode.com/openharmony/third_party_json.git"

def download_json():
    cmd = ["git", "clone", "-b", "OpenHarmony-v6.0-Release", "--depth=1", JSON_GIT, "json-v3.11.3"]
    output = subprocess.Popen(cmd, cwd=THIRDPARTY_DIR, stdout=PIPE)
    for line in output.stdout:
        print(line.decode("ascii", "ignore").rstrip())

def prepare_build():
    if not os.path.exists(THIRDPARTY_DIR):
        os.makedirs(THIRDPARTY_DIR)
    if not os.path.exists(os.path.join(THIRDPARTY_DIR, "json-v3.11.3")):
        download_json()

# use llvm toolchain on linux
if not IS_WINDOWS:
    clang_path = shutil.which("clang")
    clang_pp_path = shutil.which("clang++")
    if not clang_path:
        LOG.error("clang is required to build cangjie compiler")
    if not clang_pp_path:
        LOG.error("clang++ is requreed to build cangjie compiler")

    os.environ["CC"] = clang_path
    os.environ["CXX"] = clang_pp_path
else:
    c_compiler = shutil.which("gcc")
    cxx_compiler = shutil.which("g++")
    if not c_compiler:
        LOG.error("gcc is required to build cangjie compiler")
    if not cxx_compiler:
        LOG.error("g++ is requreed to build cangjie compiler")
    os.environ["CC"] = c_compiler
    os.environ["CXX"] = cxx_compiler


def log_output(output):
    """log command output"""
    while True:
        line = output.stdout.readline()
        if not line:
            output.communicate()
            returncode = output.returncode
            if returncode != 0:
                LOG.error("build error: %d!\n", returncode)
                sys.exit(1)
            break
        try:
            LOG.info(line.decode("ascii", "ignore").rstrip())
        except UnicodeEncodeError:
            LOG.info(line.decode("utf-8", "ignore").rstrip())


def init_log(name):
    """init log config"""
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)

    log = logging.getLogger(name)
    log.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
        "[%(asctime)s:%(module)s:%(lineno)s:%(levelname)s] %(message)s"
    )
    streamhandler = logging.StreamHandler(sys.stdout)
    streamhandler.setLevel(logging.DEBUG)
    streamhandler.setFormatter(formatter)
    log.addHandler(streamhandler)
    filehandler = TimedRotatingFileHandler(
        LOG_FILE, when="W6", interval=1, backupCount=60
    )
    filehandler.setLevel(logging.DEBUG)
    filehandler.setFormatter(formatter)
    log.addHandler(filehandler)
    return log


def generate_cmake_defs(args):
    """convert args to cmake defs"""

    def bool_to_opt(value):
        return "ON" if value else "OFF"

    if args.target == "windows-x86_64":
        return [
            "-DCMAKE_BUILD_TYPE=" + args.build_type.value,
            "-DCOVERAGE_FLAG=" + bool_to_opt(args.code_coverage),
            "-DCMAKE_SYSTEM_NAME=" + "Windows",
            "-DCMAKE_C_COMPILER=" + "x86_64-w64-mingw32-gcc",
            "-DCMAKE_CXX_COMPILER=" + "x86_64-w64-mingw32-g++",
            "-DCODIRA_HOME=" + os.environ["CODIRA_HOME"],
        ] + [arg for arg in args.cmake_args if arg != "--"]

    return [
        "-DCMAKE_BUILD_TYPE=" + args.build_type.value,
        "-DCOVERAGE_FLAG=" + bool_to_opt(args.code_coverage),
    ] + [arg for arg in args.cmake_args if arg != "--"]


def build(args):
    if args.build_type is None:
        LOG.error(
            "please specify the build type. Supported options are: release and debug."
        )
        return

    prepare_build()
    """codelint build"""
    LOG.info("begin build...\n")

    """build windows executables from Linux"""
    if args.target == "windows-x86_64":
        os.makedirs(CMAKE_BUILD_DIR)
        output = subprocess.Popen(
            ["cmake", HOME_DIR] + generate_cmake_defs(args),
            cwd=CMAKE_BUILD_DIR,
            stdout=PIPE,
        )
        log_output(output)
        output = subprocess.Popen(
            ["make", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE
        )
        log_output(output)
        LOG.info("end build\n")
        return

    if os.path.exists(CMAKE_BUILD_DIR):
        if IS_WINDOWS:
            output = subprocess.Popen(
                ["mingw32-make", "-j" + str(args.jobs)],
                cwd=CMAKE_BUILD_DIR,
                stdout=PIPE,
            )
        else:
            output = subprocess.Popen(["ninja", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE)
        log_output(output)
    else:
        os.makedirs(CMAKE_BUILD_DIR)
        if IS_WINDOWS:
            generator = "MinGW Makefiles"
        else:
            generator = "Ninja"
        output = subprocess.Popen(
            ["cmake", HOME_DIR, "-G", generator] + generate_cmake_defs(args),
            cwd=CMAKE_BUILD_DIR,
            stdout=PIPE,
        )
        log_output(output)
        if IS_WINDOWS:
            output = subprocess.Popen(
                ["mingw32-make", "-j" + str(args.jobs)],
                cwd=CMAKE_BUILD_DIR,
                stdout=PIPE,
            )
        else:
            output = subprocess.Popen(["ninja", "-j" + str(args.jobs)], cwd=CMAKE_BUILD_DIR, stdout=PIPE)
        log_output(output)
    LOG.info("end build\n")


def clean(args):
    """clean build outputs and logs"""
    LOG.info("begin clean...\n")
    output_dirs = [
        "build/build",
    ]
    for file_path in output_dirs:
        abs_file_path = os.path.join(HOME_DIR, file_path)
        if os.path.isdir(abs_file_path):
            shutil.rmtree(abs_file_path, ignore_errors=True)
        if os.path.isfile(abs_file_path):
            os.remove(abs_file_path)
    LOG.info("end clean\n")


def install(args):
    """install targets"""
    LOG.info("begin install targets...")
    install_path = args.install_prefix if args.install_prefix else CMAKE_OUTPUT_DIR
    output = subprocess.Popen(
        ["cmake", "--install", "build", "--prefix", install_path],
        cwd=BUILD_DIR,
        stdout=PIPE,
    )
    log_output(output)
    if output.returncode != 0:
        LOG.fatal("install failed")

    LOG.info("end install targets...")


class BuildType(Enum):
    """CMAKE_BUILD_TYPE options"""

    debug = "Debug"
    release = "Release"

    def __str__(self):
        return self.name

    def __repr__(self):
        return str(self)

    @staticmethod
    def argparse(s):
        try:
            return BuildType[s]
        except KeyError:
            return s


def main():
    """build entry"""
    parser = argparse.ArgumentParser(description="build codelint project")
    subparsers = parser.add_subparsers(help="sub command help")
    parser_build = subparsers.add_parser("build", help="build codelint")
    parser_build.add_argument(
        "-t",
        "--build-type",
        type=BuildType.argparse,
        dest="build_type",
        default=None,
        choices=list(BuildType),
        help="select target build type",
    )
    parser_build.add_argument(
        "--code-coverage", action="store_true", help="do code coverage"
    )
    parser_build.add_argument(
        "--target",
        type=str,
        dest="target",
        default="native",
        choices=["native", "windows-x86_64"],
        help="Specify the target platform (default: native).",
    )
    parser_build.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=THREADS_NUM,
        help="number of parallel jobs to run")
    parser_build.add_argument(
        "cmake_args",
        nargs=argparse.REMAINDER,
        help="other arguments directly passed to CMake, " "please pass it after a '--'",
    )
    parser_build.set_defaults(func=build)

    parser_install = subparsers.add_parser("install", help="install targets")
    parser_install.add_argument(
        "--prefix", dest="install_prefix", help="target install prefix"
    )
    parser_install.set_defaults(func=install)

    parser_clean = subparsers.add_parser("clean", help="clean build")
    parser_clean.set_defaults(func=clean)

    args = parser.parse_args()
    if not hasattr(args, "func"):
        args = parser.parse_args(["build"] + sys.argv[1:])

    args.func(args)
    if FAIL_CASE_NUM != 0:
        exit(1)


if __name__ == "__main__":
    LOG = init_log("root")
    os.environ["LANG"] = "C.UTF-8"
    main()
