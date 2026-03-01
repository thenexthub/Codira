#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2023 Huawei Device Co., Ltd.
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
import os
import platform
import re
import subprocess
import signal
import sys


DEFAULT_TIMEOUT = 60000


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--testcase-list', help='the file path of testcase list', required=True)
    parser.add_argument(
        '--build-dir', help='the build dir of es2abc', required=True)
    parser.add_argument(
        '--output-dir', help='the output path', required=True)
    return parser.parse_args()


def create_dir(out_dir, mode=0o775):
    if os.path.isabs(out_dir) and not os.path.exists(out_dir):
        os.makedirs(out_dir, mode)


def output(retcode, msg):
    if retcode == 0:
        if msg != '':
            print(str(msg))
    elif retcode == -6:
        sys.stderr.write("Aborted (core dumped)")
    elif retcode == -4:
        sys.stderr.write("Aborted (core dumped)")
    elif retcode == -11:
        sys.stderr.write("Segmentation fault (core dumped)")
    elif msg != '':
        sys.stderr.write(str(msg))
    else:
        sys.stderr.write("Unknown Error: " + str(retcode))


def run_command(cmd_args, timeout=DEFAULT_TIMEOUT):
    proc = subprocess.Popen(cmd_args,
                            stderr=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            close_fds=True,
                            start_new_session=True)
    cmd_string = " ".join(cmd_args)
    code_format = 'utf-8'
    if platform.system() == "Windows":
        code_format = 'gbk'

    try:
        (output_res, errs) = proc.communicate(timeout=timeout)
        ret_code = proc.poll()

        if errs.decode(code_format, 'ignore') != '':
            output(1, errs.decode(code_format, 'ignore'))
            return 1

        if ret_code and ret_code != 1:
            code = ret_code
            msg = f"Command {cmd_string}: \n"
            msg += f"error: {str(errs.decode(code_format,'ignore'))}"
        else:
            code = 0
            msg = str(output_res.decode(code_format, 'ignore'))

    except subprocess.TimeoutExpired:
        proc.kill()
        proc.terminate()
        os.kill(proc.pid, signal.SIGTERM)
        code = 1
        msg = f"Timeout:'{cmd_string}' timed out after' {str(timeout)} seconds"
    except Exception as err:
        code = 1
        msg = f"{cmd_string}: unknown error: {str(err)}"
    output(code, msg)
    return code


def search_dependency(file, directory):
    for root, dirs, files in os.walk(directory, topdown=True):
        for f in files:
            if f == file:
                return os.path.join(root, f)
    return "FILE_NOT_FOUND"


def collect_dependencies(tc_file, search_dir, traversed_dependencies):
    dependencies = []
    traversed_dependencies.append(tc_file)
    with open(tc_file, 'r', encoding='utf-8') as f:
        content = f.read()
        module_import_list = re.findall(r'(import|from)(?:\s*)\(?(\'(\.\/.*)\'|"(\.\/.*)")\)?', content)

        for result in list(set(module_import_list)):
            specifier = (result[2] if len(result[2]) != 0 else result[3]).lstrip('./')
            if os.path.basename(tc_file) is not specifier:
                dependency = search_dependency(specifier, search_dir)
                if dependency == "FILE_NOT_FOUND":
                    continue

                if dependency not in traversed_dependencies:
                    dependencies.extend(collect_dependencies(dependency, search_dir, 
                                                             list(set(traversed_dependencies))))
                dependencies.append(dependency)

    return dependencies


def check_compile_mode(file):
    with open(file, 'r', encoding='utf-8') as check_file:
        content_file = check_file.read()
        module_pattern = '((?:export|import)\s+(?:{[\s\S]+}|\*))|'
        module_pattern += '(export\s+(?:let|const|var|function|class|default))|'
        module_pattern += '(import\s+[\'\"].+[\'\"])'
        module_mode_list = re.findall(module_pattern, content_file)

        for module_mode in list(set(module_mode_list)):
            if len(module_mode[0]) != 0 or len(module_mode[1]) != 0 or \
                len(module_mode[2]) != 0:
                return True

    if "flags: [module]" in content_file or "/module/" in file:
            return True

    return False


def gen_merged_abc(tc_name_pre, build_dir, output_dir, dependencies):
    cmd = []
    retcode = 0
    proto_bin_suffix = "protoBin"
    merge_abc_binary = os.path.join(build_dir, 'merge_abc')
    out_merged_abc_name = f"{tc_name_pre}.abc"

    for dependency in list(set(dependencies)):
        out_dependency_dir = []
        out_merged_dependency_proto_dir = []
        if "/module/" in dependency:
            out_dependency_dir = os.path.join(output_dir, 'disassember_tests', 'module')

        out_merged_dependency_proto_dir = os.path.join(out_dependency_dir, tc_name_pre)
        if not os.path.exists(out_merged_dependency_proto_dir):
            create_dir(out_merged_dependency_proto_dir)

    cmd = [merge_abc_binary, '--input', out_merged_dependency_proto_dir, '--suffix', proto_bin_suffix,
           '--outputFilePath', out_dependency_dir, '--output', out_merged_abc_name]

    retcode = run_command(cmd)
    return retcode


def gen_module_and_dynamicimport_abc(args, tc, tc_dir, out_dir):
    cmd = []
    search_dir = tc_dir
    retcode = 0
    frontend_binary = os.path.join(args.build_dir, 'es2abc')

    testcase, module_flag = tc.split(' ')
    tc_name = os.path.basename(testcase)
    tc_name_pre = tc_name[0: tc_name.rfind('.')]
    tc_file = os.path.join(tc_dir, testcase)
    out_file = os.path.join(out_dir, tc_name_pre + '.abc')

    dependencies = collect_dependencies(tc_file, search_dir, [])
    if list(set(dependencies)):
        for dependency in list(set(dependencies)):
            cmd = []
            out_dependency_abc_dir = []
            dependency_name = os.path.basename(dependency)
            dependency_name_pre = os.path.splitext(dependency_name)[0]

            # dependent output abc directory
            if "/dynamic-import/" in dependency:
                out_dependency_abc_dir = os.path.join(args.output_dir, 'disassember_tests', 'dynamic-import')
            elif "/module/" in dependency:
                out_dependency_abc_dir = os.path.join(args.output_dir, 'disassember_tests', 'module')

            # generate output dependent-abc directory or output dependent-protobin directory
            if not os.path.exists(out_dependency_abc_dir):
                create_dir(out_dependency_abc_dir)
            out_dependency_proto_dir = os.path.join(out_dependency_abc_dir, tc_name_pre)
            if not os.path.exists(out_dependency_proto_dir):
                create_dir(out_dependency_proto_dir)

            # dependent output-protobin for module, dependent output-abc for dynamic-import
            out_dependency_proto_file = os.path.join(out_dependency_abc_dir, tc_name_pre,
                                                     dependency_name_pre + '.protoBin')
            out_dependency_aparted_abc_file = os.path.join(out_dependency_abc_dir, dependency_name_pre + '.abc')

            if "/dynamic-import/" in dependency:
                cmd = [frontend_binary, dependency, '--output', out_dependency_aparted_abc_file]
            elif "/module/" in dependency:
                cmd = [frontend_binary, dependency, '--outputProto', out_dependency_proto_file, '--merge-abc']

            if check_compile_mode(dependency):
                cmd.append('--module')

            retcode = run_command(cmd)

    # generate self-protobin for module, generate self-abc for dynamic-import
    if module_flag == '1' and "/module/" in tc_file:
        if len(dependencies) != 0:
            out_self_proto_file = os.path.join(out_dir, tc_name_pre, tc_name_pre + '.protoBin')
            cmd = [frontend_binary, tc_file, '--outputProto', out_self_proto_file, '--merge-abc']
        else:
            cmd = [frontend_binary, tc_file, '--output', out_file, '--merge-abc']
    elif module_flag == '0' and "/dynamic-import/" in tc_file:
        cmd = [frontend_binary, tc_file, '--output', out_file]

    if module_flag == '1':
        cmd.append('--module')

    retcode = run_command(cmd)

    # generate merged-abc for module
    if module_flag == '1' and "/module/" in tc_file and len(dependencies) != 0:
        retcode = gen_merged_abc(tc_name_pre, args.build_dir, args.output_dir, dependencies)

    return retcode


def gen_abc(args):
    retcode = 0
    frontend_binary = os.path.join(args.build_dir, 'es2abc')
    tc_dir = os.path.dirname(args.testcase_list)
    out_dir_disassember_test = os.path.join(args.output_dir, 'disassember_tests')

    if os.path.exists(out_dir_disassember_test):
        subprocess.run(['rm', '-rf', out_dir_disassember_test])

    with open(args.testcase_list) as testcase_file:
        for tc in testcase_file.readlines():
            tc = tc.strip()
            annotation_index = tc.find('#')
            if len(tc) == 0 or annotation_index == 0:
                continue

            cmd = []
            testcase, module_flag = tc.split(' ')
            tc_name = os.path.basename(testcase)
            tc_name_pre = tc_name[0: tc_name.rfind('.')]
            tc_file = os.path.join(tc_dir, testcase)
            out_dir = os.path.join(args.output_dir, 'disassember_tests', testcase[0: testcase.rfind('/')])
            if not os.path.exists(out_dir):
                create_dir(out_dir)
            out_file = os.path.join(out_dir, tc_name_pre + '.abc')

            if ("/module/" in tc_file or "/dynamic-import/" in tc_file):
                retcode = gen_module_and_dynamicimport_abc(args, tc, tc_dir, out_dir)
            else:
                cmd = [frontend_binary, tc_file, '--output', out_file]
                if module_flag == '1':
                    cmd.append('--module')

                if not os.path.exists(out_file):
                    retcode = run_command(cmd)

    return retcode


def get_pa_command(testcase, disasm_tool, output_dir):
    tc_name = os.path.basename(testcase)
    tc_name_pre = tc_name[0: tc_name.rfind('.')]
    out_tc_abc_dir = os.path.join(output_dir, 'disassember_tests', testcase[0: testcase.rfind('/')])
    out_tc_abc_file = os.path.join(out_tc_abc_dir, tc_name_pre + '.abc')
    out_tc_pa_file = os.path.join(out_tc_abc_dir, tc_name_pre + '.pa')

    cmd = [disasm_tool, out_tc_abc_file, out_tc_pa_file]
    return cmd


def gen_pa(args):
    retcode = 0
    disasm_tool = os.path.join(args.output_dir, 'ark_disasm')

    with open(args.testcase_list) as testcase_file:
        for tc in testcase_file.readlines():
            tc = tc.strip()
            annotation_index = tc.find('#')
            if len(tc) == 0 or annotation_index == 0:
                continue

            tc_dir = os.path.dirname(args.testcase_list)
            testcase, module_flag = tc.split(' ')
            tc_file = os.path.join(tc_dir, testcase)
            if not os.path.exists(tc_file):
                retcode = 1

            if "dynamic-import" in tc:
                # generate self-pa for dynamic-import
                cmd = get_pa_command(testcase, disasm_tool, args.output_dir)

                # generate dependent-pa for dynamic-import
                search_dir = tc_dir
                dependencies = collect_dependencies(tc_file, search_dir, [])
                if list(set(dependencies)):
                    for dependency in list(set(dependencies)):
                        dependency_testcase_name = os.path.basename(dependency)
                        dependency_testcase = os.path.join('dynamic-import', dependency_testcase_name)
                        cmd = get_pa_command(dependency_testcase, disasm_tool, args.output_dir)
                        retcode = run_command(cmd)

            cmd = get_pa_command(testcase, disasm_tool, args.output_dir)
            retcode = run_command(cmd)

    return retcode


def cmp_pa_file(testcase, tc_dir, output_dir):
    retcode = 0
    tc_name = os.path.basename(testcase)
    tc_name = tc_name[0: tc_name.rfind('.')]

    out_abc_dir = os.path.join(output_dir, 'disassember_tests', testcase[0: testcase.rfind('/')])
    out_tc_pa_file = os.path.join(out_abc_dir, tc_name + '.pa')
    is_out_tc_pa_file = os.path.exists(out_tc_pa_file)

    target_pa_dir = os.path.join(tc_dir, 'expected')
    target_pa_file = os.path.join(target_pa_dir, tc_name + '.pa')
    is_target_pa_file = os.path.exists(target_pa_file)

    if not is_out_tc_pa_file or not is_target_pa_file:
        retcode = 1
        print(" --- pa not exist ---")

    print(" --- Running disassember testcase: " + testcase + " ---")
    with open(out_tc_pa_file, 'r') as tc_file, open(target_pa_file, 'r') as target_file:
        for tc_line, target_line in zip(tc_file, target_file):
            if tc_line != target_line:
                print(" error --- out_pa_line != target_pa_line  ---")
                retcode = 1
                return retcode

    print(" --- Pass ---")
    return retcode


def check_pa(args):
    retcode = 0
    tc_dir = os.path.dirname(args.testcase_list)

    with open(args.testcase_list) as testcase_file:
        for tc in testcase_file.readlines():
            tc = tc.strip()
            annotation_index = tc.find('#')
            if len(tc) == 0 or annotation_index == 0:
                continue

            testcase, module_flag = tc.split(' ')
            tc_file = os.path.join(tc_dir, testcase)

            if "dynamic-import" in tc:
                retcode = cmp_pa_file(testcase, tc_dir, args.output_dir)
                search_dir = tc_dir
                dependencies = collect_dependencies(tc_file, search_dir, [])
                if list(set(dependencies)):
                    for dependency in list(set(dependencies)):
                        dependency_testcase_name = os.path.basename(dependency)
                        dependency_testcase = os.path.join('dynamic-import', dependency_testcase_name)
                        retcode = cmp_pa_file(dependency_testcase, tc_dir, args.output_dir)
            else:
                retcode = cmp_pa_file(testcase, tc_dir, args.output_dir)

    return retcode


def main():
    input_args = parse_args()

    if gen_abc(input_args):
        print("[0/0] --- gen abc fail ---")
        return

    if gen_pa(input_args):
        print("[0/0] --- gen pa fail ---")
        return

    if check_pa(input_args):
        print("[0/0] --- check pa fail ---")
        return


if __name__ == '__main__':
    main()
