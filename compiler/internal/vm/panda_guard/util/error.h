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

#ifndef PANDA_GUARD_UTIL_ERROR_H
#define PANDA_GUARD_UTIL_ERROR_H

#include <sstream>
#include <string_view>

namespace panda::guard {

enum class ErrorCode {
    CONFIG_FILE_FORMAT_ERROR = 11303001,
    CONFIG_FILE_CONTENT_EMPTY = 11303002,
    NOT_CONFIGURED_OBFUSCATION_RULES = 11303003,
    NOT_CONFIGURED_ABC_FILE_PATH = 11303004,
    NOT_CONFIGURED_OBF_ABC_FILE_PATH = 11303005,
    NOT_CONFIGURED_TARGET_API_VERSION = 11303006,
    NOT_CONFIGURED_DEFAULT_AND_PRINT_NAME_CACHE_PATH = 11303007,
    NOT_CONFIGURED_REQUIRED_FIELD = 11303008,
    NAME_CACHE_FILE_FORMAT_ERROR = 11303009,
    COMMAND_PARAMS_PARSE_ERROR = 11310010,

    GENERIC_ERROR = 11310001,
};

class Error {
public:
    Error(ErrorCode code, std::string_view tag) : errorCode_(code), tag_(tag) {}

    std::ostream &GetDescStream();

    std::ostream &GetCauseStream();

    std::ostream &GetSolutionsStream();

    void Print();

private:
    void PrintGenericError();

    ErrorCode errorCode_;

    std::string_view tag_;

    std::stringstream descStream_;

    std::stringstream causeStream_;

    std::stringstream solutionsStream_;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_UTIL_ERROR_H
