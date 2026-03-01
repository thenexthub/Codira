#!/usr/bin/env python3
# -- coding: utf-8 --
# Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

import argparse
import json
import os
import multiprocessing
import sys
import subprocess
import re
from typing import List


def get_args():
    parser = argparse.ArgumentParser(
        description="Runner for clang-tidy for panda project.")
    parser.add_argument(
        'panda_dir', help='panda sources directory.', type=str)
    parser.add_argument(
        'build_dir', help='panda build directory.', type=str)
    parser.add_argument(
        '--filename-filter', type=str, action='store', dest='filename_filter',
        required=False, default=None,
        help='Regexp for filename with path to it. If missed all source files will be checked.')
    parser.add_argument(
        '--changed-only', action="store_true", help='Check changed files only.'
    )
    parser.add_argument(
        '--full', action="store_true", help='Check all files with all compile keys.')
    parser.add_argument(
        '--check-libabckit', action="store_true", help='Check runtime_core/libabckit folder')
    parser.add_argument(
        '--header-filter', type=str, action='store', dest='header_filter',
        required=False, default=".*",
        help='Ignore header filter from .clang-tidy file.')
    parser.add_argument(
        '--proc-count', type=int, action='store', dest='proc_count',
        required=False, default="-1",
        help='Paralell process count of clang-tidy')

    return parser.parse_args()


default_disabled_checks = [
    # aliases for other checks(here full list: https://clang.llvm.org/extra/clang-tidy/checks/list.html):
    "-bugprone-narrowing-conversions",
    "-cert-con36-c",
    "-cert-con54-cpp",
    "-cert-dcl03-c",
    "-cert-dcl16-c",
    "-cert-dcl37-c",
    "-cert-dcl51-cpp",
    "-cert-dcl54-cpp",
    "-cert-dcl59-cpp",
    "-cert-err09-cpp",
    "-cert-err61-cpp",
    "-cert-exp42-c",
    "-cert-fio38-c",
    "-cert-flp37-c",
    "-cert-msc30-c",
    "-cert-msc32-c",
    "-cert-oop11-cpp",
    "-cert-oop54-cpp",
    "-cert-pos44-c",
    "-cert-pos47-c",
    "-cert-sig30-c",
    "-cert-str34-c",
    "-cppcoreguidelines-avoid-c-arrays",
    "-cppcoreguidelines-avoid-magic-numbers",
    "-cppcoreguidelines-c-copy-assignment-signature",
    "-cppcoreguidelines-explicit-virtual-functions",
    "-cppcoreguidelines-macro-to-enum",
    "-cppcoreguidelines-non-private-member-variables-in-classes",
    "-fuchsia-header-anon-namespaces",
    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-readability-braces-around-statements",
    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-readability-function-size",
    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-readability-namespace-comments",
    "-hicpp-avoid-c-arrays",
    "-hicpp-avoid-goto",
    "-hicpp-braces-around-statements",
    "-hicpp-deprecated-headers",
    "-hicpp-explicit-conversions",
    "-hicpp-function-size",
    "-hicpp-invalid-access-moved",
    "-hicpp-member-init",
    "-hicpp-move-const-arg",
    "-hicpp-named-parameter",
    "-hicpp-new-delete-operators",
    "-hicpp-no-array-decay",
    "-hicpp-no-malloc",
    "-hicpp-noexcept-move",
    "-hicpp-special-member-functions",
    "-hicpp-static-assert",
    "-hicpp-undelegated-constructor",
    "-hicpp-uppercase-literal-suffix",
    "-hicpp-use-auto",
    "-hicpp-use-emplace",
    "-hicpp-use-equals-default",
    "-hicpp-use-equals-delete",
    "-hicpp-use-noexcept",
    "-hicpp-use-nullptr",
    "-hicpp-use-override",
    "-hicpp-vararg",
    "-llvm-else-after-return",
    "-llvm-qualified-auto",
    # explicitly disabled checks
    # disabled because it is hard to write macros with types with it
    "-bugprone-macro-parentheses",
    # disabled because of incorrect root prefix
    "-llvm-header-guard",
    # disabled because conflicts with the clang-format
    "-llvm-include-order",
    # disabled to use non-const references
    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-runtime-references",
    # disabled because we have a lot of false positives and it is stylistic check
    "-fuchsia-trailing-return",
    # disabled because we use functions with default arguments a lot
    "-fuchsia-default-arguments-calls",
    # disabled because we use functions with default arguments a lot
    "-fuchsia-default-arguments-declarations",
    # disabled as a stylistic check
    "-modernize-use-trailing-return-type",
    # Fix all occurences
    "-readability-static-accessed-through-instance",
    # Fix all occurences
    "-bugprone-sizeof-expression",
    # Fix all occurences
    "-readability-convert-member-functions-to-static",
    # Fix all occurences
    "-bugprone-branch-clone",
    # disabled llvm-libc specific checks
    "-llvmlibc-*",
    # disabled FGPA specific checks
    "-altera-*",
    # disable all suggestions to use abseil library
    "-abseil-*",
    # disabled as a stylistic check
    "-readability-identifier-length",
    # Look if we want to use GSL or gsl-lite
    "-cppcoreguidelines-owning-memory",
    # Look into issue with ASSERT
    "-cppcoreguidelines-pro-bounds-array-to-pointer-decay",
    # Look if we want to use GSL or gsl-lite
    "-cppcoreguidelines-pro-bounds-constant-array-index",
    # Consider to remove from global list
    "-cppcoreguidelines-pro-type-const-cast",
    # Consider to remove from global list
    "-cppcoreguidelines-pro-type-reinterpret-cast",
    # Look into it
    "-cppcoreguidelines-pro-type-static-cast-downcast",
    # Look into it
    "-fuchsia-default-arguments",
    # Consider to remove from global list
    "-fuchsia-overloaded-operator",
    # Look into it
    "-modernize-use-nodiscard",
    "-cert-dcl50-cpp",
    # candidates for removal:
    # For some reason become failed in DEFAULT_MOVE_SEMANTIC
    "-performance-noexcept-move-constructor",

    # clang-14 temporary exceptions
    "-bugprone-easily-swappable-parameters",
    "-bugprone-reserved-identifier",
    "-bugprone-signed-char-misuse",
    "-bugprone-implicit-widening-of-multiplication-result",
    "-bugprone-suspicious-include",
    "-bugprone-dynamic-static-initializers",

    "-cppcoreguidelines-avoid-non-const-global-variables",
    "-cppcoreguidelines-virtual-class-destructor",
    "-cppcoreguidelines-prefer-member-initializer",
    "-cppcoreguidelines-init-variables",
    "-cppcoreguidelines-narrowing-conversions",

    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-upgrade-googletest-case",

    "-readability-redundant-access-specifiers",
    "-readability-qualified-auto",
    "-readability-make-member-function-const",
    "-readability-container-data-pointer",
    "-readability-function-cognitive-complexity",
    "-readability-use-anyofallof",
    "-readability-suspicious-call-argument",

    "-modernize-return-braced-init-list",

    "-cert-err33-c",

    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-readability-casting",

    "-concurrency-mt-unsafe",
    "-performance-no-int-to-ptr",
    "-misc-no-recursion",

    # CHECKER_IGNORE_NEXTLINE(AF0010)
    "-google-readability-avoid-underscore-in-googletest-name",
    "-readability-avoid-const-params-in-decls"
]


def run_clang_tidy(src_path: str, config_file_path: str, build_dir: str, header_filter: str, compile_args: str) -> bool:
    # Used by ctcache to provide a wrapper for real clang-tidy that will check the cache
    # before launching clang-tidy and save the result to ctcache server
    cmd_path = os.getenv('CLANG_TIDY_PATH')
    if not cmd_path:
        cmd_path = 'clang-tidy-14'
    cmd = [cmd_path]
    cmd += ['-checks=*,' + ','.join(default_disabled_checks)]
    cmd += ['--header-filter=' + header_filter]
    cmd += ['--config-file=' + os.path.join(config_file_path, '.clang-tidy')]
    cmd += [src_path]
    cmd += ['--']
    cmd += compile_args.split()

    try:
        subprocess.check_output(cmd, cwd=build_dir, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        # Skip error for some invalid release configurations.
        if not e.stdout:
            print("Note: missed output for ", src_path)
            return True

        out_msg = e.stdout.decode()
        if ',-warnings-as-errors]' not in out_msg:
            print("Note: bad output for ", src_path)
            return True

        print('Failed: ' + ' '.join(cmd) + '\n' + out_msg)

        if e.stderr:
            print(e.stderr.decode())

        return False

    return True


def get_changed_files(src_path: str) -> List[str]:
    cmd = ['git', 'diff', 'HEAD^', '--name-status']
    ret = subprocess.check_output(cmd, cwd=src_path)
    files = [col.split('\t')[1] for col in ret.decode().split('\n') if col]
    return files


def get_full_path(relative_path: str, location_base: str, panda_dir: str, build_dir: str) -> str:
    full_path = panda_dir if location_base == "PANDA_DIR" else build_dir
    full_path += "/" + relative_path
    full_path = str(os.path.realpath(full_path))
    return full_path


def check_file_list(file_list: list, panda_dir: str, build_dir: str, header_filter: str, proc_count: int) -> bool:
    pool = multiprocessing.Pool(proc_count)
    jobs = []
    for src, args in file_list:

        msg = "Done clang-tidy: %s" % (src)

        proc = pool.apply_async(func=run_clang_tidy, args=(
            src, panda_dir, build_dir, header_filter, args))
        jobs.append((proc, msg))

    # Wait for jobs to complete before exiting
    total_count = str(len(jobs))
    main_ret_val = True
    idx = 0
    while jobs:
        upd_job = []
        for proc, msg in jobs:
            if not proc.ready():
                upd_job.append((proc, msg))
                continue

            idx += 1
            print("[%s/%s] %s" % (str(idx), total_count, msg))
            if main_ret_val and not proc.get():
                main_ret_val = False

        jobs = upd_job

    # Safely terminate the pool
    pool.close()
    pool.join()

    return main_ret_val


def need_to_ignore_file(file_path: str, panda_dir: str, build_dir: str) -> bool:
    src_exts = (".c", '.cc', ".cp", ".cxx", ".cpp", ".CPP", ".c++", ".C")
    if not file_path.endswith(src_exts):
        return True

    # Skip third_party.
    regexp = re.compile(".*/third_party/.*")
    if regexp.search(file_path):
        return True

    return False


def get_file_list(panda_dir: str, build_dir: str, filename_filter: str) -> list:
    json_cmds_path = os.path.join(build_dir, 'compile_commands.json')
    cmds_json = []
    with open(json_cmds_path, 'r') as f:
        cmds_json = json.load(f)

    if not cmds_json:
        return []

    regexp = None
    if filename_filter:
        regexp = re.compile(filename_filter)

    file_list = []
    for cmd in cmds_json:
        # this check is needed to exclude symlinks in third_party
        file_path_raw = cmd["file"]
        if need_to_ignore_file(file_path_raw, panda_dir, build_dir):
            continue

        file_path = str(os.path.realpath(cmd["file"]))
        if need_to_ignore_file(file_path, panda_dir, build_dir):
            continue

        if regexp and not regexp.search(file_path):
            continue

        compile_args = cmd["command"]
        # strip unnecessary escape sequences
        compile_args = compile_args.replace('\\', '')
        # Removing sysroot for correct clang-tidy work on cross-compiled arm
        sysroot = re.search(r' --sysroot[\w\d\S]+', compile_args)
        if sysroot:
            compile_args = compile_args.replace(sysroot.group(0), '')

        file_list.append((file_path, compile_args))

    file_list.sort(key=lambda tup: tup[0])
    return file_list


# Remove noisy test files duplicates.
def filter_test_file_duplicates(file_list: list) -> list:
    filtered = []
    regexp = re.compile(".*/tests/.*")
    for file_path, keys in file_list:
        if not filtered:
            filtered.append((file_path, keys))
            continue

        if filtered[-1][0] != file_path:
            filtered.append((file_path, keys))
            continue

        if not regexp.search(file_path):
            filtered.append((file_path, keys))

    filtered.sort(key=lambda tup: tup[0])
    return filtered;


def check_file_list_duplicate(dup_path : str, file_list: list) -> bool:
    count = 0
    for file_path, keys in file_list:
        if dup_path == file_path:
            count += 1

        if count > 1:
            return True

    return False


# Remove same files if their compile keys has minor differents.
def filter_file_duplicated_options(file_list: list) -> list:
    filtered = []
    for file_path, keys in file_list:
        if not check_file_list_duplicate(file_path, file_list):
            filtered.append((file_path, keys))
            continue

    filtered.sort(key=lambda tup: tup[0])
    return filtered;


def filter_file_list(file_list: list) -> list:
    print('Files before filter:', len(file_list))

    filtered = filter_file_duplicated_options(file_list)
    filtered = filter_test_file_duplicates(filtered)

    print('Filtered files:', len(filtered))
    return filtered


def verify_uniq_element_list(uniq_element_list: list) -> bool:
    return len(uniq_element_list) == len(set(uniq_element_list))


def verify_args(panda_dir: str, build_dir: str) -> str:
    if not verify_uniq_element_list(default_disabled_checks):
        return "Error: Dupclicated defauls disabled checks"

    return ""


def check_headers_in_es2panda_sources(panda_dir):
    es2panda_dir = os.path.join(panda_dir, "tools/es2panda")
    result = []
    for root, dirs, files in os.walk(es2panda_dir):
        for file in files:
            file_path = os.path.join(root, file)
            _, extension = os.path.splitext(file_path)
            if extension != ".h" and extension != ".cpp":
                continue
            with open(file_path, "r") as source_file:
                for line in source_file.readlines():
                    line = line.replace(" ", "")
                    if (line.startswith("#include\"tools/es2panda")):
                        result.append(f"Error: use of header starting with tools/es2panda in {file_path}")
                        continue
    if len(result) > 0:
        for file in result:
            print(file)
        sys.exit(1)


def check_file_list_for_system_headers_includes(file_list: list):
    system_headers_ = []
    regexp = re.compile("-I[^ ]*/third_party[^ ]*")
    for path, compile_args in file_list:
        match = regexp.search(compile_args)
        if match:
            system_headers_.append((path, match.group()))

    return system_headers_


def get_proc_count(cmd_ard : int) -> int:
    if cmd_ard > 0:
        return cmd_ard

    min_proc_str = os.getenv('NPROC_PER_JOB')
    if min_proc_str:
        return int(min_proc_str)

    return multiprocessing.cpu_count()

if __name__ == "__main__":
    arguments = get_args()
    files_list = []

    if not os.path.exists(os.path.join(arguments.build_dir, 'compile_commands.json')):
        sys.exit("Error: Missing file `compile_commands.json` in build directory")

    # A lot of false positive checks on GTESTS in libabckit
    if arguments.check_libabckit:
        default_disabled_checks.append("-fuchsia-statically-constructed-objects")
        default_disabled_checks.append("-cert-err58-cpp")

    err_msg = verify_args(arguments.panda_dir, arguments.build_dir)
    if err_msg:
        sys.exit(err_msg)

    if arguments.changed_only:
        if arguments.filename_filter:
            raise Exception("--changed-only cannot be used with --filename-filer")
        changed_files = get_changed_files(arguments.panda_dir)
        arguments.filename_filter = '(' + '|'.join([re.escape(file) for file in changed_files]) + ')'

    files_list = get_file_list(
        arguments.panda_dir, arguments.build_dir, arguments.filename_filter)

    if not files_list:
        sys.exit("Can't prepare source list."
                 "Please check if 'compile_commands.json` file exists in the build dir "
                 "and check `--filename-filter` parameter correctness if you use it.")

    if not arguments.check_libabckit:
        check_headers_in_es2panda_sources(arguments.panda_dir)
        print('Checked for system headers: Starting')
        system_headers = check_file_list_for_system_headers_includes(files_list)
        if system_headers:
            err_msg = "Error: third_party includes should be marked as system\n"
            for e_path, e_system_header in system_headers:
                err_msg += e_path + " error: " + e_system_header + "\n"
            sys.exit(err_msg)
        print('Checked for system headers: Done')

    if not arguments.full:
        files_list = filter_file_list(files_list)
    else:
        # Disable ctcache in full mode to handle cases when caching works incorrectly
        os.environ['CTCACHE_DISABLE'] = '1'

    process_count = get_proc_count(arguments.proc_count)
    print('clang-tidy proc_count: ' + str(process_count))
    conf_file_path = arguments.panda_dir
    if arguments.check_libabckit:
        conf_file_path = os.path.join(conf_file_path, "libabckit")
    if not check_file_list(files_list, conf_file_path, arguments.build_dir, arguments.header_filter, process_count):
        sys.exit("Failed: Clang-tidy get errors")

    print("Clang-tidy was passed successfully!")
