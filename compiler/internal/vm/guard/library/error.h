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

#ifndef GUARD_ERROR_H
#define GUARD_ERROR_H

#include <sstream>

namespace ark::guard {

/**
 * @brief ErrorCode
 * Subsystem Code(3-digit number): indicating the error subsystem. 113: bytecode obfuscation
 * Error Type Code(2-digit number): indicating the error type. 1x: Each system defines its own specific error types,
 * starting from 10. 11: guard obfuscation failed.
 * Extension Code(3-digit number): indicating the scene subdivision of the error type code.
 */
enum class ErrorCode {
    ERR_SUCCESS = 0,
    INVALID_PARAMS = 11311001,
    CONFIGURATION_FILE_FORMAT_ERROR = 11311002,
    UNKNOWN_KEEP_OPTION = 11311003,
    CLASS_SPECIFICATION_FORMAT_ERROR = 11311004,
    NAME_CACHE_FORMAT_ERROR = 11311005,
    NAME_CACHE_MODULE_FORMAT_ERROR = 11311006,
    NAME_CACHE_FUNCTION_FORMAT_ERROR = 11311007,
    UNKNOWN_NAME_GENERATOR_TYPE = 11311008,

    GENERIC_ERROR = 11311900,
};

/**
 * @brief Error
 */
class Error {
public:
    /**
     * @brief constructor
     * @param code ErrorCode
     */
    explicit Error(ErrorCode code) : errorCode_(code) {}

    /**
     * @brief constructor
     * @return error description
     */
    std::ostream &GetDescStream();

    /**
     * @brief constructor
     * @return error cause
     */
    std::ostream &GetCauseStream();

    /**
     * @brief constructor
     * @return error solution
     */
    std::ostream &GetSolutionsStream();

    /**
     * @brief print error info
     */
    void Print();

private:
    ErrorCode errorCode_;

    std::stringstream descStream_;

    std::stringstream causeStream_;

    std::stringstream solutionsStream_;
};
}  // namespace ark::guard

#endif  // GUARD_ERROR_H
