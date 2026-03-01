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

"""codira compiler build entry"""

import argparse
import logging
import multiprocessing
import os
import platform
import re
import shutil
import stat
import subprocess
import sys
from enum import Enum
from logging.handlers import TimedRotatingFileHandler
from pathlib import Path
from subprocess import DEVNULL, PIPE

# build.py lives at the repo root.
HOME_DIR        = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT       = HOME_DIR

BUILD_DIR        = os.path.join(HOME_DIR, "build")
CMAKE_BUILD_DIR  = os.path.join(BUILD_DIR, "build")
CMAKE_OUTPUT_DIR = os.path.join(HOME_DIR, "output")
OUTPUT_BIN_DIR   = os.path.join(CMAKE_OUTPUT_DIR, "bin")
LOG_DIR          = os.path.join(BUILD_DIR, "logs")
LOG_FILE         = os.path.join(LOG_DIR, "codira.log")

# ---------------------------------------------------------------------------
# Repo-root directory layout
#
#   vm/core        - Backend project (compiler backend)
#   vm/boundscheck - bounds-checking function library
#   vm/flatbuffers - FlatBuffers serialisation library
#   vm/libxml2     - libxml2
#   vm/gtest       - GoogleTest
#   stdx/          - standard library extensions
#   runtime/       - codira runtime
#   compiler/      - this build script + compiler sources  (HOME_DIR)
#
# build.py only does pre-flight presence checks here; CMake FetchContent
# handles the actual cloning when a vm/* directory is absent.
# ---------------------------------------------------------------------------
COMPILER_DIR       = os.path.join(HOME_DIR, "compiler")         # ./compiler
INTERNAL_DIR       = os.path.join(COMPILER_DIR, "internal")     # ./compiler/internal
VM_DIR             = os.path.join(INTERNAL_DIR, "vm")           # ./compiler/internal/vm
VM_CORE_DIR        = os.path.join(VM_DIR, "core")               # ./compiler/internal/vm/core
VM_BOUNDSCHECK_DIR = os.path.join(INTERNAL_DIR, "boundscheck")  # ./compiler/internal/boundscheck
VM_FLATBUFFERS_DIR = os.path.join(VM_DIR, "flatbuffers")        # ./compiler/internal/vm
VM_LIBXML2_DIR     = os.path.join(VM_DIR, "libxml2")            # ./compiler/internal/vm/libxml2
VM_GTEST_DIR       = os.path.join(VM_DIR, "gtest")              # ./compiler/internal/vm/gtest

STDX_DIR    = os.path.join(HOME_DIR, "stdx")
RUNTIME_DIR = os.path.join(HOME_DIR, "runtime")

IS_WINDOWS = platform.system() == "Windows"
IS_MACOS   = platform.system() == "Darwin"
IS_ARM     = platform.uname().processor in ["aarch64", "arm", "arm64"]

CODE_OS_NAME   = platform.system().lower()
CODE_ARCH_NAME = platform.machine().replace("AMD64", "x86_64").lower()

CAN_RUN_CODENATIVE = (platform.system(), platform.machine()) in [
    ("Linux",   "x86_64"),
    ("Linux",   "aarch64"),
    ("Windows", "AMD64"),
    ("Darwin",  "x86_64"),
    ("Darwin",  "arm64"),
]

LD_LIBRARY_PATH   = "Path" if IS_WINDOWS else "DYLD_LIBRARY_PATH" if IS_MACOS else "LD_LIBRARY_PATH"
LIBRARY_PATH      = "Path" if IS_WINDOWS else "LIBRARY_PATH"
PATH_ENV_SEPERATOR = ";" if IS_WINDOWS else ":"

DELTA_JOBS = 2
MAKE_JOBS  = multiprocessing.cpu_count() + DELTA_JOBS

PYTHON_EXECUTABLE = sys.executable

# Supported cross-compilation targets (name -> triple).
# OHOS entries have been removed; linux-* entries cover generic cross builds.
TARGET_DICTIONARY = {
    "native":                  None,
    "windows-x86_64":          "x86_64-w64-mingw32",
    "linux-x86_64":            "x86_64-linux-gnu",
    "linux-aarch64":           "aarch64-linux-gnu",
    "ios-simulator-aarch64":   "arm64-apple-ios11-simulator",
    "ios-simulator-x86_64":    "x86_64-apple-ios11-simulator",
    "ios-aarch64":             "arm64-apple-ios11",
    "android-aarch64":         "aarch64-linux-android31",
    "android31-aarch64":       "aarch64-linux-android31",
    "android26-aarch64":       "aarch64-linux-android26",
    "android-x86_64":          "x86_64-linux-android",
    "android31-x86_64":        "x86_64-linux-android31",
    "android26-x86_64":        "x86_64-linux-android26",
}


def resolve_path(path):
    return path if os.path.isabs(path) else os.path.abspath(path)


def log_output(output, checker=None, args=[]):
    """Stream subprocess stdout to LOG line-by-line; exit on non-zero return."""
    while True:
        line = output.stdout.readline()
        if not line:
            output.communicate()
            if output.returncode != 0:
                LOG.error("build error: %d!\n", output.returncode)
                sys.exit(1)
            break
        try:
            LOG.info(line.decode("ascii", "ignore").rstrip())
        except UnicodeEncodeError:
            LOG.info(line.decode("utf-8", "ignore").rstrip())


def ensure_vm_dirs():
    """
    Create the vm/ root if absent and print a summary of what is / is not
    present.  Actual cloning is delegated to CMake's FetchContent; this
    function is only informational so the developer sees what will be fetched
    before the build starts.
    """
    os.makedirs(VM_DIR, exist_ok=True)
    deps = {
        "core  (LLVM)":  VM_CORE_DIR,
        "boundscheck":   VM_BOUNDSCHECK_DIR,
        "flatbuffers":   VM_FLATBUFFERS_DIR,
        "libxml2":       VM_LIBXML2_DIR,
        "gtest":         VM_GTEST_DIR,
    }
    for label, path in deps.items():
        status = "present" if os.path.isdir(path) else "absent - will be fetched by CMake"
        LOG.info("  vm/%-20s %s", label, status)


def generate_version_tail(target):
    """Generate the version suffix string embedded in the codec binary."""
    res = " (codenative)"
    arch_name   = platform.machine().replace("AMD64", "x86_64").replace("arm64", "aarch64").lower()
    vendor_name = "apple" if IS_MACOS else "unknown"
    if IS_WINDOWS:
        os_name = "windows-gnu"
    elif IS_MACOS:
        os_name = "darwin"
    else:
        os_name = "linux-gnu"
    res += "\\nTarget: {}".format(target) if target else \
           "\\nTarget: {}-{}-{}".format(arch_name, vendor_name, os_name)
    return str(res)


def generate_cmake_defs(args):
    """Translate parsed CLI arguments into -D flags for CMake."""

    def bool_to_opt(value):
        return "ON" if value else "OFF"

    # Select the appropriate toolchain file.
    # For native (no --target) builds we rely on CMake's auto-detection via
    # CC/CXX and do NOT pass a toolchain file - this avoids errors when the
    # project has not yet created platform-specific toolchain files.
    # A toolchain file is only used when (a) a cross target is given AND
    # (b) the file actually exists under cmake/.
    toolchain_abs = None
    if args.target:
        toolchain_map = {
            "x86_64-w64-mingw32":           "mingw_x86_64_toolchain.cmake",
            "x86_64-linux-gnu":             "linux_x86_64_toolchain.cmake",
            "aarch64-linux-gnu":            "linux_aarch64_toolchain.cmake",
            "arm64-apple-ios11-simulator":  "ios_simulator_arm64_toolchain.cmake",
            "x86_64-apple-ios11-simulator": "ios_simulator_x86_64_toolchain.cmake",
            "arm64-apple-ios11":            "ios_arm64_toolchain.cmake",
        }
        if args.target in toolchain_map:
            toolchain_file = toolchain_map[args.target]
        elif "aarch64-linux-android" in args.target:
            toolchain_file = "android_aarch64_toolchain.cmake"
        elif "x86_64-linux-android" in args.target:
            toolchain_file = "android_x86_64_toolchain.cmake"
        else:
            LOG.error("No toolchain file mapping for target: %s", args.target)
            sys.exit(1)
        candidate = os.path.join(HOME_DIR, "cmake", toolchain_file)
        if os.path.exists(candidate):
            toolchain_abs = candidate
        else:
            LOG.warning("Toolchain file absent, building without it: %s", candidate)

    install_prefix = CMAKE_OUTPUT_DIR + (
        "-{}".format(args.target) if (args.target and args.product == "codec") else "")

    result = [
        "-DCMAKE_BUILD_TYPE="             + args.build_type.value,
        "-DCMAKE_ENABLE_ASSERT="          + bool_to_opt(args.enable_assert),
        "-DCMAKE_INSTALL_PREFIX="         + install_prefix,
        *(["-DCMAKE_TOOLCHAIN_FILE=" + toolchain_abs] if toolchain_abs else []),
        "-DCMAKE_PREFIX_PATH="            + (
            args.target_toolchain + "/../x86_64-w64-mingw32"
            if args.target_toolchain else ""),
        "-DCODIRA_REPO_ROOT="             + REPO_ROOT,
        "-DCODIRA_VM_DIR="                + VM_DIR,
        "-DCODIRA_BUILD_TESTS="           + bool_to_opt(
            (not args.no_tests) and args.product in ("all", "libs")),
        "-DCODIRA_BUILD_CODEC="           + bool_to_opt(args.product in ("all", "codec")),
        "-DCODIRA_BUILD_STD_SUPPORT="     + bool_to_opt(args.product in ("all", "libs")),
        "-DCODIRA_BUILD_CODEDB="          + bool_to_opt(args.build_codedb),
        "-DCODIRA_BUILD_CODEDB_DISABLE_PYTHON=" + bool_to_opt(args.codedb_disable_python),
        "-DCODIRA_INSTALL_CODEDB_SCRIPT=" + bool_to_opt(args.install_codedb_script),
        "-DCODIRA_ENABLE_HWASAN="         + bool_to_opt(args.hwasan),
        "-DCODIRA_VERSION="               + generate_version_tail(args.target),
        "-DBUILD_GCC_TOOLCHAIN="          + (
            args.gcc_toolchain if args.gcc_toolchain and args.target is None else ""),
        "-DCODIRA_TARGET_LIB="            + (
            ";".join(args.target_lib) if args.target_lib else ""),
        "-DCODIRA_TARGET_TOOLCHAIN="      + (args.target_toolchain or ""),
        "-DCODIRA_INCLUDE="               + (
            ";".join(args.include) if args.include else ""),
        "-DCODIRA_TARGET_SYSROOT="        + (args.target_sysroot or ""),
        "-DCODIRA_LINK_JOB_POOL="         + (
            str(args.link_jobs) if args.link_jobs != 0 else ""),
        "-DCODIRA_DISABLE_STACK_GROW_FEATURE=" + bool_to_opt(
            args.disable_stack_grow_feature),
        "-DCODIRA_ENABLE_SANITIZE_OPTION=" + bool_to_opt(args.enable_sanitize_option),
        "-DCODIRA_DOWNLOAD_FLATBUFFERS="  + bool_to_opt(not args.no_download_flatbuffers),
    ]

    if args.version:
        result.append("-DCODE_SDK_VERSION=" + args.version)

    if args.target and "aarch64-linux-android" in args.target:
        android_api_level = re.match(
            r'aarch64-linux-android(\d{2})?', args.target).group(1)
        result.append("-DCMAKE_ANDROID_NDK="  + (args.android_ndk or ""))
        result.append("-DCMAKE_ANDROID_API="  + (android_api_level or ""))

    if args.sanitizer_support:
        result.append("-DCODIRA_SANITIZER_SUPPORT=" + args.sanitizer_support)

    return result


def build(args):
    """Configure and build the codira project."""
    if args.target:
        args.target = TARGET_DICTIONARY[args.target]

    if args.gcc_toolchain and args.target and args.product != "codec":
        LOG.warning(
            "--gcc-toolchain has no effect here because no host-platform "
            "product is being built in this configuration")

    check_compiler(args)

    LOG.info("begin build...")
    LOG.info("vm/ dependency layout:")
    ensure_vm_dirs()

    set_codenative_backend_env(args)

    if args.product is None:
        args.product = "all" if args.target is None else "libs"

    if args.android_ndk:
        os.environ["ANDROID_NDK_ROOT"] = args.android_ndk

    # Generator / build-tool selection.
    # Probe for ninja on every platform; fall back to cmake --build which
    # works with any generator (Unix Makefiles, MinGW Makefiles, etc.).
    def _ninja_available():
        try:
            proc = subprocess.Popen(
                ["ninja", "--version"], stdout=DEVNULL, stderr=DEVNULL)
            proc.communicate()
            return proc.returncode == 0
        except Exception:
            return False

    if _ninja_available():
        generator = "Ninja"
        build_cmd = ["ninja"]
        if args.jobs > 0:
            build_cmd += ["-j", str(args.jobs)]
    elif IS_WINDOWS:
        generator = "MinGW Makefiles"
        build_cmd = ["mingw32-make", "-j" + str(MAKE_JOBS)]
    else:
        # Unix Makefiles is always available; cmake --build delegates to it.
        generator = "Unix Makefiles"
        build_cmd = ["cmake", "--build", ".", "--parallel",
                     str(args.jobs if args.jobs > 0 else MAKE_JOBS)]

    cmake_command = ["cmake", HOME_DIR, "-G", generator] + generate_cmake_defs(args)

    if args.print_cmd:
        print(" ".join(cmake_command))
        sys.exit(0)

    if args.target == "x86_64-w64-mingw32":
        package_mingw_dependencies(args)

    os.makedirs(BUILD_DIR, exist_ok=True)

    if args.sanitizer_support:
        cmake_build_dir = os.path.join(
            BUILD_DIR, "build-libs-{}".format(args.sanitizer_support))
        if args.target:
            cmake_build_dir += "-{}".format(args.target)
    else:
        cmake_build_dir = (
            os.path.join(
                BUILD_DIR, "build-{}-{}".format(args.product, args.target))
            if args.target else CMAKE_BUILD_DIR)

    if not os.path.exists(os.path.join(cmake_build_dir, "CMakeCache.txt")):
        os.makedirs(cmake_build_dir, exist_ok=True)
        output = subprocess.Popen(cmake_command, cwd=cmake_build_dir, stdout=PIPE)
        log_output(output)

    output = subprocess.Popen(build_cmd, cwd=cmake_build_dir, stdout=PIPE)
    log_output(output)

    if output.returncode != 0:
        LOG.fatal("build failed")

    LOG.info("end build")


def set_codira_env(args):
    os.environ["HOME_DIR"]     = HOME_DIR
    os.environ["REPO_ROOT"]    = REPO_ROOT
    os.environ[LD_LIBRARY_PATH] = PATH_ENV_SEPERATOR.join([
        os.path.join(REPO_ROOT, "output/lib"),
        os.path.join(REPO_ROOT, "output/runtime/lib/{os}_{arch}_codenative".format(
            os=CODE_OS_NAME, arch=CODE_ARCH_NAME)),
        os.environ.get(LD_LIBRARY_PATH, ""),
    ])
    LOG.info("set codira env: %s=%s", LD_LIBRARY_PATH, os.environ[LD_LIBRARY_PATH])


def set_codenative_backend_env(args):
    os.environ[LD_LIBRARY_PATH] = PATH_ENV_SEPERATOR.join([
        os.path.join(REPO_ROOT, "output/lib"),
        os.path.join(REPO_ROOT, "output/lib/{os}_{arch}_codenative".format(
            os=CODE_OS_NAME, arch=CODE_ARCH_NAME)),
        os.path.join(REPO_ROOT, "build/compiler/lib"),
        os.path.join(REPO_ROOT, "build/compiler/lib/{os}_{arch}_codenative".format(
            os=CODE_OS_NAME, arch=CODE_ARCH_NAME)),
        os.environ.get(LD_LIBRARY_PATH, ""),
    ] + (args.target_lib if hasattr(args, "target_lib") else []))
    LOG.info("set codenative env: %s=%s", LD_LIBRARY_PATH, os.environ[LD_LIBRARY_PATH])

    os.environ[LIBRARY_PATH] = PATH_ENV_SEPERATOR.join([
        os.path.join(REPO_ROOT, "build/compiler/lib"),
        os.environ.get(LIBRARY_PATH, ""),
    ] + (args.target_lib if hasattr(args, "target_lib") else []))
    LOG.info("set codenative env: %s=%s", LIBRARY_PATH, os.environ[LIBRARY_PATH])

    os.environ["PATH"] = PATH_ENV_SEPERATOR.join([
        os.path.join(REPO_ROOT, "output/tools/bin"),
        os.environ.get("PATH", ""),
    ])

    if IS_MACOS:
        os.environ["ZERO_AR_DATE"] = "1"


def test(args):
    """Run the test suite."""
    LOG.info("begin test...")
    if not IS_WINDOWS:
        set_codira_env(args)
    unit_test(args)
    LOG.info("end test...\n")


def unit_test(args):
    """Run CTest inside the native build directory."""
    LOG.info("begin unit test...\n")
    output = subprocess.Popen(
        ["ctest", "--output-on-failure"], cwd=CMAKE_BUILD_DIR, stdout=PIPE)
    log_output(output)
    LOG.info("end unit test...\n")


def install(args):
    """Install all previously built targets."""
    if args.host:
        args.host = TARGET_DICTIONARY[args.host]

    LOG.info("begin install targets...")
    targets = []

    if not args.host:
        if os.path.exists(CMAKE_BUILD_DIR):
            targets.append(("native", CMAKE_BUILD_DIR))
    else:
        suffix = "codec-{}".format(args.host)
        cross_build_path = os.path.join(BUILD_DIR, "build-{}".format(suffix))
        if os.path.exists(cross_build_path):
            targets.append((suffix, cross_build_path))

    for directory in os.listdir(BUILD_DIR):
        if directory.startswith("build-libs-"):
            targets.append((
                "libs-{}".format(directory[len("build-libs-"):]),
                os.path.join(BUILD_DIR, directory)))

    if not targets:
        LOG.fatal("Nothing is built yet.")
        sys.exit(1)

    for label, build_path in targets:
        LOG.info("installing %s build...", label)
        cmake_cmd = ["cmake", "--install", "."]
        if args.prefix:
            cmake_cmd += ["--prefix", os.path.abspath(args.prefix)]
        elif args.host:
            cmake_cmd += ["--prefix",
                          os.path.join(REPO_ROOT, "output-{}".format(args.host))]

        output = subprocess.Popen(cmake_cmd, cwd=build_path, stdout=PIPE)
        log_output(output)
        if output.returncode != 0:
            LOG.fatal("install %s build failed", label)
            sys.exit(1)

    if args.host == "x86_64-w64-mingw32":
        mingw_bin_path = os.path.join(
            REPO_ROOT, "output-x86_64-w64-mingw32/vm/mingw/bin")
        if os.path.exists(mingw_bin_path):
            for entry in os.listdir(mingw_bin_path):
                p = os.path.join(mingw_bin_path, entry)
                if os.path.isfile(p) and not entry.endswith(".exe") \
                        and entry != "libssp-0.dll":
                    os.remove(p)

    LOG.info("end install targets...")


def redo_with_write(redo_func, path, err):
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        redo_func(path)
    else:
        raise


def clean(args):
    """Remove all build outputs and logs."""
    LOG.info("begin clean...\n")

    # Close file handlers before wiping the log directory
    for handler in list(LOG.handlers):
        if isinstance(handler, logging.FileHandler):
            handler.close()
            LOG.removeHandler(handler)

    output_dirs = ["build", "output", "vm/.fetch-stamps"]

    if os.path.isdir(BUILD_DIR):
        for directory in os.listdir(BUILD_DIR):
            if directory.startswith("build-codec-"):
                suffix = directory[len("build-codec-"):]
                cross_out = os.path.join(REPO_ROOT, "output-{}".format(suffix))
                if os.path.isdir(cross_out):
                    output_dirs.append(cross_out)

    for rel in output_dirs:
        abs_path = os.path.join(REPO_ROOT, rel)
        if os.path.isdir(abs_path):
            shutil.rmtree(abs_path, ignore_errors=False, onerror=redo_with_write)
        elif os.path.isfile(abs_path):
            os.remove(abs_path)

    LOG.info("end clean\n")


def package_mingw_dependencies(args):
    archive = os.path.join(REPO_ROOT, "vm/mingw/windows-x86_64-mingw.tar.gz")
    if os.path.exists(archive):
        return

    LOG.info("Packaging MinGW dependencies...")

    def copy_files_to(from_path, filenames, to_dir):
        os.makedirs(to_dir, exist_ok=True)
        for name in filenames:
            src = os.path.join(from_path, name)
            if os.path.exists(src):
                shutil.copy(src, to_dir + "/")
            else:
                LOG.info("%s not found - skipping", src)

    search = args.target_toolchain or None
    mingw_root = str(os.path.dirname(os.path.dirname(
        os.path.abspath(shutil.which("x86_64-w64-mingw32-gcc", path=search)))))
    pkg_path = os.path.join(REPO_ROOT, "vm/mingw")

    copy_files_to(
        os.path.join(mingw_root, "x86_64-w64-mingw32"),
        ["bin/libc++.dll", "bin/libunwind.dll", "bin/libwinpthread-1.dll"],
        os.path.join(pkg_path, "dll"))
    copy_files_to(
        os.path.join(mingw_root, "x86_64-w64-mingw32/lib"),
        ["crt2.o", "dllcrt2.o", "libadvapi32.a", "libkernel32.a", "libm.a",
         "libmingw32.a", "libmingwex.a", "libmoldname.a", "libmsvcrt.a",
         "libpthread.a", "libshell32.a", "libuser32.a", "libws2_32.a",
         "libcrypt32.a", "crtbegin.o", "crtend.o"],
        os.path.join(pkg_path, "lib"))

    subprocess.run(
        ["tar", "-C", "mingw", "-zcf", "windows-x86_64-mingw.tar.gz", "dll", "lib"],
        cwd=os.path.join(REPO_ROOT, "vm"),
        check=True)


def init_log(name):
    """Initialise logging to stdout and a rotating file."""
    os.makedirs(LOG_DIR, exist_ok=True)
    log = logging.getLogger(name)
    log.setLevel(logging.DEBUG)
    fmt = logging.Formatter(
        "[%(asctime)s] [%(module)s] [%(levelname)s] %(message)s",
        "%Y-%m-%d %H:%M:%S")

    sh = logging.StreamHandler(sys.stdout)
    sh.setLevel(logging.DEBUG)
    sh.setFormatter(fmt)
    log.addHandler(sh)

    fh = TimedRotatingFileHandler(LOG_FILE, when="W6", interval=1, backupCount=60)
    fh.setLevel(logging.DEBUG)
    fh.setFormatter(fmt)
    log.addHandler(fh)

    return log


class BuildType(Enum):
    """Mirrors CMAKE_BUILD_TYPE values."""
    debug          = "Debug"
    release        = "Release"
    relwithdebinfo = "RelWithDebInfo"

    def __str__(self):  return self.name
    def __repr__(self): return str(self)

    @staticmethod
    def argparse(s):
        try:
            return BuildType[s]
        except KeyError:
            return s.build_type


def main():
    parser = argparse.ArgumentParser(description="build codira project")
    subparsers = parser.add_subparsers(help="sub-command help")

    # ---- clean -------------------------------------------------------------
    parser_clean = subparsers.add_parser("clean", help="clean build", add_help=False)
    parser_clean.set_defaults(func=clean)

    # ---- build -------------------------------------------------------------
    pb = subparsers.add_parser("build", help="build codira")
    pb.add_argument("-t", "--build-type",
        type=BuildType.argparse, dest="build_type",
        choices=list(BuildType), required=True,
        help="select target build type")
    pb.add_argument("--print-cmd", action="store_true",
        help="print the cmake command without running it")
    pb.add_argument("-j", "--jobs",
        dest="jobs", type=int, default=0,
        help="run N jobs in parallel (0 = default)")
    pb.add_argument("--link-jobs",
        dest="link_jobs", type=int, default=0,
        help="max N link jobs in parallel (0 = default)")
    pb.add_argument("--enable-assert", action="store_true",
        help="build with assert and self-checking (release mode only)")
    pb.add_argument("--no-tests", action="store_true",
        help="build without unit tests")
    pb.add_argument("--disable-stack-grow-feature", action="store_true",
        help="disable the default stack grow feature for the codenative BE")
    pb.add_argument("--hwasan", action="store_true",
        help="build with hardware AddressSanitizer")
    pb.add_argument("--gcc-toolchain", dest="gcc_toolchain",
        help="specify a GCC toolchain for codec, stdlib, and BE")
    pb.add_argument("--target", dest="target",
        type=str, choices=TARGET_DICTIONARY.keys(),
        help="cross-compile stdlib for the given target triple")
    pb.add_argument("-L", "--target-lib",
        dest="target_lib", type=str, action="append", default=[],
        help="add a library search path for all products")
    pb.add_argument("--target-toolchain", dest="target_toolchain", type=str,
        help="path to the cross-compilation toolchain")
    pb.add_argument("-I", "--include",
        dest="include", type=str, action="append", default=[],
        help="additional header search paths")
    pb.add_argument("--target-sysroot", dest="target_sysroot", type=str,
        help="pass this path to the C/CXX compiler as --sysroot")
    pb.add_argument("--android-ndk", dest="android_ndk", type=str,
        help="path to the Android NDK")
    pb.add_argument("--product",
        dest="product", choices=["all", "codec", "libs"],
        help="select which part of Codira to build")
    pb.add_argument("--build-codedb", action="store_true",
        help="build codec with codedb")
    pb.add_argument("--codedb-disable-python", action="store_true",
        help="build codedb without the Python extension")
    pb.add_argument("--install-codedb-script", action="store_true",
        help="install codedb with its Python script")
    pb.add_argument("--no-download-flatbuffers", action="store_true",
        help="skip automatic download of FlatBuffers into vm/flatbuffers")
    pb.add_argument("--codelib-sanitizer-support",
        type=str, choices=["asan", "tsan", "hwasan"],
        dest="sanitizer_support",
        help="enable sanitizer support for codira libraries")
    pb.add_argument("--enable-sanitize-option", action="store_true",
        help="enable the --sanitize option in codec")
    pb.add_argument("-v", "--version",
        dest="version", default="0.0.1",
        help="SDK version string (e.g. 1.2.3)")
    pb.add_argument("--vm-dir", dest="vm_dir", type=str, default=None,
        help="override the vm/ dependency root directory")
    pb.set_defaults(func=build)

    # ---- install -----------------------------------------------------------
    pi = subparsers.add_parser("install", help="install built targets")
    pi.add_argument("--host",
        dest="host", type=str, choices=TARGET_DICTIONARY.keys(),
        help="generate an installation package for this host triple "
             "(ignored when --prefix is set)")
    pi.add_argument("--prefix", dest="prefix",
        help="installation prefix directory")
    pi.set_defaults(func=install)

    # ---- test --------------------------------------------------------------
    pt = subparsers.add_parser("test", help="run unit tests", add_help=False)
    pt.set_defaults(func=test)

    args = parser.parse_args()

    # Allow overriding VM_DIR from the CLI
    if hasattr(args, "vm_dir") and args.vm_dir:
        global VM_DIR, VM_CORE_DIR, VM_BOUNDSCHECK_DIR
        global VM_FLATBUFFERS_DIR, VM_LIBXML2_DIR, VM_GTEST_DIR
        VM_DIR             = os.path.abspath(args.vm_dir)
        VM_CORE_DIR        = os.path.join(VM_DIR, "core")
        VM_BOUNDSCHECK_DIR = os.path.join(VM_DIR, "boundscheck")
        VM_FLATBUFFERS_DIR = os.path.join(VM_DIR, "flatbuffers")
        VM_LIBXML2_DIR     = os.path.join(VM_DIR, "libxml2")
        VM_GTEST_DIR       = os.path.join(VM_DIR, "gtest")

    args.func(args)


def check_compiler(args):
    """Locate a suitable C/C++ compiler and export CC/CXX."""
    toolchain_path = args.target_toolchain or None
    if toolchain_path and not os.path.exists(toolchain_path):
        LOG.error("toolchain path does not exist: %s", toolchain_path)

    if IS_WINDOWS and args.target is None:
        c_compiler   = shutil.which("x86_64-w64-mingw32-gcc.exe", path=toolchain_path)
        cxx_compiler = shutil.which("x86_64-w64-mingw32-g++.exe", path=toolchain_path)
    else:
        c_compiler   = shutil.which("clang",   path=toolchain_path)
        cxx_compiler = shutil.which("clang++", path=toolchain_path)

    # Fall back to a triple-prefixed GCC when cross-compiling without clang
    if (c_compiler is None or cxx_compiler is None) and args.target:
        c_compiler   = shutil.which(args.target + "-gcc", path=toolchain_path)
        cxx_compiler = shutil.which(args.target + "-g++", path=toolchain_path)

    # Last resort: generic GCC
    if c_compiler is None or cxx_compiler is None:
        c_compiler   = shutil.which("gcc", path=toolchain_path)
        cxx_compiler = shutil.which("g++", path=toolchain_path)

    if c_compiler is None or cxx_compiler is None:
        where = "the given toolchain path" if toolchain_path else "$PATH"
        LOG.error("cannot find a usable compiler in %s", where)
        LOG.error("clang/clang++ or gcc/g++ is required to build codira")
        sys.exit(1)

    # When cross-compiling codec a *native* compiler must also be available so
    # that LLVM's code-generation tools (e.g. llvm-tblgen) can be built for
    # the host.  CMake reads CC/CXX for the native build.
    if args.target and args.product == "codec":
        native_clang   = shutil.which("clang")
        native_clangxx = shutil.which("clang++")
        if native_clang is None or native_clangxx is None:
            LOG.error("cross-compiling codec requires a native clang/clang++ in $PATH")
            sys.exit(1)
        tc_suffix = (
            " --gcc-toolchain={}".format(args.gcc_toolchain)
            if args.gcc_toolchain else "")
        os.environ["CC"]  = native_clang  + tc_suffix
        os.environ["CXX"] = native_clangxx + tc_suffix
    else:
        os.environ["CC"]  = c_compiler
        os.environ["CXX"] = cxx_compiler


if __name__ == "__main__":
    LOG = init_log("root")
    os.environ["LANG"] = "C.UTF-8"
    main()
