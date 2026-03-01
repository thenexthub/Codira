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

#ifndef GUARD_UTIL_EXECUTE_UTIL_H
#define GUARD_UTIL_EXECUTE_UTIL_H

#ifndef PANDA_TARGET_WINDOWS

#include <iostream>
#include <string>

namespace ark::guard {
/**
 * @brief CoutRedirect
 */
class CoutRedirect {
public:
    explicit CoutRedirect(std::streambuf *newBuffer)
        : oldBuffer_(std::cout.rdbuf(newBuffer))
    {
    }

    ~CoutRedirect()
    {
        std::cout.rdbuf(oldBuffer_);
    }

private:
    std::streambuf *oldBuffer_;
};

/**
 * @brief ExecuteUtil
 */
class ExecuteUtil {
public:
    /**
     * @brief execute static abc file function
     * @param abcPath abc file path
     * @param moduleName module name
     * @param fnName function name
     * @return execution result
     */
    static std::string ExecuteStaticAbc(const std::string &abcPath, const std::string &moduleName,
                                        const std::string &fnName);

private:
    static void InitEnvironment();

    static bool isInitialized_;
};
} // namespace ark::guard

#endif
#endif  // GUARD_UTIL_EXECUTE_UTIL_H