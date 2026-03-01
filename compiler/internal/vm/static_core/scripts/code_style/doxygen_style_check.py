#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
import re
import sys


def get_args():
    parser = argparse.ArgumentParser(
        description="Doxygen style checker for panda project.")
    parser.add_argument(
        'panda_dir', help='panda sources directory.', type=str)

    return parser.parse_args()


def get_file_list(panda_dir) -> list:
    src_exts = (".c", '.cc', ".cp", ".cxx", ".cpp", ".CPP", ".c++", ".C", ".h",
                ".hh", ".H", ".hp", ".hxx", ".hpp", ".HPP", ".h++", ".tcc")
    skip_dirs = ["third_party", "artifacts", "\..*", "build.*"]
    file_list = []
    for dirpath, dirnames, filenames in os.walk(panda_dir):
        dirnames[:] = [d for d in dirnames if not re.match(f"({')|('.join(skip_dirs)})", d)]
        for fname in filenames:
            if (fname.endswith(src_exts)):
                full_path = os.path.join(panda_dir, dirpath, fname)
                full_path = str(os.path.realpath(full_path))
                file_list.append(full_path)

    return file_list


# Additional check because regexps sometimes find correct comments
def is_doxygen_comment(s: str) -> bool:
    # Helps not to raise warnings for fine comments
    # Examples of fine comments: /************/, /* ///////TEXT */
    fine_comments = [re.compile(r'///[^\n]*\*/[^\n]*'), re.compile(r'/\*\*[^\n]*\*/')]
    for comm in fine_comments:
        if comm.search(s):
            return False
    return True


def print_correct_style() -> None:
    lines = ["\nPlease, for single-line doxygen comments use the following formats:",\
             "'/// TEXT' - used for commenting on an empty line", "or",\
             "'///< TEXT' - used for commenting after declared/defined variables in the same line",\
             "\nand for multi-line doxygen comments use the following Javadoc format:"
             "\n/**\n * TEXT\n * TEXT\n * TEXT\n */"]
    for line in lines:
        print(line)


def check_keywords(src_path: str, splitted_lines: list, line_num: int) -> bool:
    is_right_style = True
    keywords_to_check = ["brief", "param", "tparam", "return", "class", "see", "code", "endcode"]
    for line in splitted_lines:
        for keyword in keywords_to_check:
            ind = line.find(keyword)
            if ind != -1 and line[ind - 1] == '\\':
                err_msg = "%s:%s" % (src_path, line_num)
                print(err_msg)
                print("Please, use '@' instead of '\\' before '%s':\n%s\n" % (keyword, line))
                is_right_style = False
        line_num += 1
    return is_right_style


def check_javadoc(src_path: str, strings: list) -> bool:
    text = []
    with open(src_path, 'r') as f:
        text = f.read()
    found_wrong_comment = False
    found_wrong_keyword_sign = False
    for string in strings:
        line_num = text[:text.find(string)].count('\n') + 1
        if string.find("Copyright") != -1 or string.count('\n') <= 1:
            continue
        pattern_to_check = re.search(r' */\*\* *\n( *\* *[^\n]*\n)+ *\*/', string)
        if not pattern_to_check or pattern_to_check.group(0) != string:
            err_msg = "%s:%s" % (src_path, line_num)
            print(err_msg)
            print("Found doxygen comment with a wrong Javadoc style:\n%s\n" % string)
            found_wrong_comment = True
            continue
        if string.count('\n') == 2:
            err_msg = "%s:%s" % (src_path, line_num)
            print("%s\n%s\n" % (err_msg, string))
            found_wrong_comment = True
            continue
        found_wrong_keyword_sign |= not check_keywords(src_path, string.splitlines(), line_num)
    if found_wrong_comment:
        print_correct_style()
    return not (found_wrong_comment or found_wrong_keyword_sign)


def check_additional_slashes(src_path: str, strings: list) -> bool:
    text = []
    with open(src_path, 'r') as f:
        text = f.read()
    lines = text.splitlines()
    found_wrong_comment = False
    found_wrong_keyword_sign = False
    fine_comments_lines = []
    strings = list(set(strings)) # Only unique strings left
    for string in strings:
        # Next line is used to find all occurencies of a given string
        str_indexes = [s.start() for s in re.finditer(re.escape(string), text)]
        for str_index in str_indexes:
            line_num = text[:str_index].count('\n') + 1
            pattern_to_check = re.search(r' */// [^ ]+?[^\n]*', lines[line_num - 1])
            if not pattern_to_check or pattern_to_check.group(0) != lines[line_num - 1]:
                err_msg = "%s:%s" % (src_path, line_num)
                print(err_msg)
                print("Found doxygen comment with a wrong style:\n%s\n" % string)
                found_wrong_comment = True
                continue
            fine_comments_lines.append(line_num)
            found_wrong_keyword_sign |= not check_keywords(src_path, string.splitlines(), line_num)

    fine_comments_lines.sort()
    for i in range(0, len(fine_comments_lines) - 1):
        if fine_comments_lines[i] + 1 == fine_comments_lines[i + 1]:
            err_msg = "%s:%s" % (src_path, fine_comments_lines[i])
            print(err_msg)
            print("Please, use '///' only for single-line comments:\n%s\n%s\n" % (
                lines[fine_comments_lines[i] - 1],
                lines[fine_comments_lines[i + 1] - 1]))
            found_wrong_comment = True
            break
    if found_wrong_comment:
        print_correct_style()
    return not (found_wrong_comment or found_wrong_keyword_sign)


def check_less_than_slashes(src_path: str, strings: list) -> bool:
    text = []
    with open(src_path, 'r') as f:
        text = f.read()
    lines = text.splitlines()
    found_wrong_comment = False
    found_wrong_keyword_sign = False
    for string in strings:
        line_num = text[:text.find(string)].count('\n') + 1
        pattern_to_check = re.search(r' *[^ \n]+[^\n]* +///< [^\n]+', lines[line_num - 1])
        if not pattern_to_check or pattern_to_check.group(0) != lines[line_num - 1]:
            err_msg = "%s:%s" % (src_path, line_num)
            print(err_msg)
            print("Found doxygen comment with a wrong style:\n%s\n" % string)
            found_wrong_comment = True
            continue
        found_wrong_keyword_sign |= not check_keywords(src_path, string.splitlines(), line_num)
    if found_wrong_comment:
        print_correct_style()
    return not (found_wrong_comment or found_wrong_keyword_sign)


def check_all(src_path: str, fine_patterns_found: list, wrong_patterns_number: int) -> bool:
    passed = wrong_patterns_number == 0
    passed &= check_javadoc(src_path, fine_patterns_found[0])
    passed &= check_additional_slashes(src_path, fine_patterns_found[1])
    passed &= check_less_than_slashes(src_path, fine_patterns_found[2])
    return passed


def run_doxygen_check(src_path: str, msg: str) -> bool:
    print(msg)
    # Forbidden styles
    qt_style = re.compile(r'/\*![^\n]*')
    slashes_with_exclamation_style = re.compile(r'//![^\n]*')
    # Allowed styles
    # Allowed if number of lines in a comment is >= 2
    javadoc_style = re.compile(r' */\*\*[\w\W]*?\*/')
    # Allowed to comment only one line. Otherwise javadoc style should be used
    additional_slashes_style = re.compile(r'/// *[^< ][^\n]*')
    # Allowed to comment declared/defined variables in the same line
    less_than_slashes_style = re.compile(r'/// *< *[^\n]*')

    regexps_for_fine_styles = [javadoc_style, additional_slashes_style, less_than_slashes_style]
    regexps_for_wrong_styles = [qt_style, slashes_with_exclamation_style]
    fine_patterns_found = [[] for i in range(len(regexps_for_fine_styles))]
    wrong_patterns_found = []
    # Looking for comments with wrong style
    for regexp in regexps_for_wrong_styles:
        strings = []
        with open(src_path, 'r') as f:
            strings = regexp.findall(f.read())
        for s in strings:
            if is_doxygen_comment(s):
                wrong_patterns_found.append(s)
    lines = []
    with open(src_path, 'r') as f:
        lines = f.readlines()
    line_num = 1
    for line in lines:
        for pattern in wrong_patterns_found:
            if pattern in line:
                err_msg = "%s:%s" % (src_path, line_num)
                print(err_msg)
                print("Found wrong doxygen style:\n", line)
                break
        line_num += 1

    # Getting comments with possibly allowed styles
    ind = 0
    for regexp in regexps_for_fine_styles:
        strings = []
        with open(src_path, 'r') as f:
            strings = regexp.findall(f.read())
        for s in strings:
            if is_doxygen_comment(s):
                fine_patterns_found[ind].append(s)
        ind += 1

    return check_all(src_path, fine_patterns_found, len(wrong_patterns_found))


def check_file_list(file_list: list) -> bool:
    jobs = []
    main_ret_val = True
    total_count = str(len(file_list))
    idx = 0
    for src in file_list:
        idx += 1
        msg = "[%s/%s] Running doxygen style checker: %s" % (str(idx), total_count, src)
        proc = run_doxygen_check(src, msg)
        jobs.append(proc)

    for job in jobs:
        if not job:
            main_ret_val = False
            break

    return main_ret_val

if __name__ == "__main__":
    args = get_args()
    files_list = get_file_list(args.panda_dir)
    if not files_list:
        sys.exit(
            "Source list can't be prepared. Please check panda_dir variable: " + args.panda_dir)

    if not check_file_list(files_list):
        sys.exit("Failed: doxygen style checker got errors")
    print("Doxygen style checker was passed successfully!")
