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

#ifndef ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H
#define ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H

#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <string_view>

namespace panda::abc2program {
class Abc2ProgramTestUtils {
public:
    static bool ValidateProgramStrings(const std::set<std::string> &program_strings);
    static bool ValidateRecordNames(const std::vector<std::string> &record_names);
    static bool ValidateLiteralArrayKeys(const std::vector<std::string> &literal_array_keys);
    static bool ValidateLiteralsSizes(const std::set<size_t> &literals_sizes);
    static bool ValidateDumpResult(const std::string &result_file_name, const std::string &expected_file_name);
    static void RemoveDumpResultFile(const std::string &file_name);
private:
    template <typename T>
    static bool ValidateStrings(const T &strings, const T &expected_strings);
    static bool ValidateLine(const std::string &result_line, const std::string &expected_line);
    static bool ValidateLiteralArrayName(const std::string &result, const std::string &expected);
    static std::set<std::string> helloworld_expected_program_strings_;
    static std::vector<std::string> helloworld_expected_record_names_;
    static std::vector<std::string> helloworld_expected_literal_array_keys_;
    static std::set<size_t> helloworld_expected_literals_sizes_;
};  // class Abc2ProgramTestUtils

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_TEST_CPP_SOURCES_ABC2PROGRAM_TEST_UTILS_H