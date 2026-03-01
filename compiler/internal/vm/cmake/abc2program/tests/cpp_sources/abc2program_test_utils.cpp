/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "abc2program_test_utils.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <vector>

#include "common/abc_file_utils.h"
#include "dump_utils.h"

namespace panda::abc2program {

std::set<std::string> Abc2ProgramTestUtils::helloworld_expected_program_strings_ = {"",
                                                                                    ".#*#add",
                                                                                    ".#*#asyncArrowFunc",
                                                                                    ".#*#asyncGenerateFunc",
                                                                                    ".#*#asyncSendableFunction",
                                                                                    ".#*#foo",
                                                                                    ".#*#generateFunc",
                                                                                    ".#*#goo",
                                                                                    ".#*#hoo",
                                                                                    ".#*#sendableFunction",
                                                                                    ".#~@0=#HelloWorld",
                                                                                    ".#~@1=#Lit",
                                                                                    ".#~@2=#NestedLiteralArray",
                                                                                    "HelloWorld",
                                                                                    "error",
                                                                                    "hello",
                                                                                    "inner catch",
                                                                                    "masg",
                                                                                    "max",
                                                                                    "min",
                                                                                    "msg",
                                                                                    "null",
                                                                                    "num",
                                                                                    "outter catch",
                                                                                    "print",
                                                                                    "prototype",
                                                                                    "str",
                                                                                    "string",
                                                                                    "toString",
                                                                                    "varA",
                                                                                    "x"};
std::vector<std::string> Abc2ProgramTestUtils::helloworld_expected_record_names_ = {
    "_ESExpectedPropertyCountAnnotation",
    "_ESModuleRecord",
    "_ESSlotNumberAnnotation",
    "_ESScopeNamesRecord",
    "_GLOBAL"
};
std::vector<std::string> Abc2ProgramTestUtils::helloworld_expected_literal_array_keys_ = {
    "_ESModuleRecord",
    "_ESScopeNamesRecord",
    "_GLOBAL",
    "_GLOBAL",
    "_GLOBAL",
    "_GLOBAL",
    "_GLOBAL"
};

std::set<size_t> Abc2ProgramTestUtils::helloworld_expected_literals_sizes_ = {2, 6, 8, 10, 21};

template <typename T>
bool Abc2ProgramTestUtils::ValidateStrings(const T &strings, const T &expected_strings)
{
    if (strings.size() != expected_strings.size()) {
        return false;
    }
    for (const std::string &expected_string : expected_strings) {
        const auto string_iter = std::find(strings.begin(), strings.end(), expected_string);
        if (string_iter == strings.end()) {
            return false;
        }
    }
    return true;
}

bool Abc2ProgramTestUtils::ValidateProgramStrings(const std::set<std::string> &program_strings)
{
    return ValidateStrings(program_strings, helloworld_expected_program_strings_);
}

bool Abc2ProgramTestUtils::ValidateRecordNames(const std::vector<std::string> &record_names)
{
    return ValidateStrings(record_names, helloworld_expected_record_names_);
}

bool Abc2ProgramTestUtils::ValidateLiteralArrayKeys(const std::vector<std::string> &literal_array_keys)
{
    if (literal_array_keys.size() != helloworld_expected_literal_array_keys_.size()) {
        return false;
    }
    for (size_t i = 0; i < literal_array_keys.size(); ++i) {
        if (literal_array_keys[i].find(helloworld_expected_literal_array_keys_[i]) != 0) {
            return false;
        }
    }
    return true;
}

bool Abc2ProgramTestUtils::ValidateLiteralsSizes(const std::set<size_t> &literal_array_sizes)
{
    return (literal_array_sizes == helloworld_expected_literals_sizes_);
}

bool Abc2ProgramTestUtils::ValidateDumpResult(
    const std::string &result_file_name, const std::string &expected_file_name)
{
    std::ifstream result_file(result_file_name);
    if (!result_file.is_open()) {
        return false;
    }
    std::ifstream expected_file(expected_file_name);
    if (!expected_file.is_open()) {
        return false;
    }
    bool is_same = true;
    std::string result_line;
    std::string expected_line;
    // Validate each line of dump results file, including empty lines.
    while (!result_file.eof() && !expected_file.eof()) {
        std::getline(result_file, result_line);
        std::getline(expected_file, expected_line);
        if (result_line != expected_line && !ValidateLine(result_line, expected_line)) {
            is_same = false;
            break;
        }
    }
    bool is_eof = result_file.eof() && expected_file.eof();
    result_file.close();
    expected_file.close();
    return is_same && is_eof;
}

bool Abc2ProgramTestUtils::ValidateLine(const std::string &result_line, const std::string &expected_line)
{
    // Split input strings by white spaces.
    std::regex white_space("\\s+");
    std::vector<std::string> result_substrs(
        std::sregex_token_iterator(result_line.begin(), result_line.end(), white_space, -1),
        std::sregex_token_iterator());
    std::vector<std::string> expected_substrs(
        std::sregex_token_iterator(expected_line.begin(), expected_line.end(), white_space, -1),
        std::sregex_token_iterator());
    // Compare all divived sub-strings.
    if (result_substrs.size() != expected_substrs.size()) {
        return false;
    }
    for (size_t i = 0; i != result_substrs.size(); ++i) {
        /* It's acceptable if:
         * 0. the sub-strings are same.
         * 1. the sub-string is a regular file path, for example, path of the input abc file, which
         *    could be different on different computers.
         * 2. the sub-strings are literal array names with offset, offset could be modified when
         *    abc file version changed.
         */
        if (result_substrs[i] != expected_substrs[i] &&
            !std::filesystem::is_regular_file(std::filesystem::path(result_substrs[i])) &&
            !ValidateLiteralArrayName(result_substrs[i], expected_substrs[i])) {
            return false;
        }
    }
    return true;
}

bool Abc2ProgramTestUtils::ValidateLiteralArrayName(const std::string &result, const std::string &expected)
{
    // Extract the record name from a literal array name, for example, extract "_GLOBAL" from "_GLOBAL_1466".
    // The record name of input string 'result' should be same as that of 'expected'.
    auto res_end_pos = result.rfind(UNDERLINE);
    auto exp_end_pos = expected.rfind(UNDERLINE);
    if (res_end_pos == std::string::npos || exp_end_pos == std::string::npos || res_end_pos != exp_end_pos) {
        return false;
    }
    auto res_start_pos = result.rfind(DUMP_CONTENT_COLON, res_end_pos) + 1;
    auto exp_start_pos = expected.rfind(DUMP_CONTENT_COLON, exp_end_pos) + 1;
    if (res_start_pos != exp_start_pos) {
        return false;
    }
    auto res_name = result.substr(res_start_pos, res_end_pos - res_start_pos);
    if (std::find(helloworld_expected_literal_array_keys_.begin(),
                  helloworld_expected_literal_array_keys_.end(),
                  res_name) == helloworld_expected_literal_array_keys_.end()) {
        return false;
    }
    auto exp_name = expected.substr(exp_start_pos, exp_end_pos - exp_start_pos);
    return res_name == exp_name;
}

void Abc2ProgramTestUtils::RemoveDumpResultFile(const std::string &file_name)
{
    std::filesystem::remove(std::filesystem::path(file_name));
}
}  // namespace panda::abc2program
