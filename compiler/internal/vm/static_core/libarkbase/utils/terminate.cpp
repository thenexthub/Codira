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

#include <array>
#include <string>

#include "securec.h"

namespace ark::terminate {
#ifndef FUZZING_EXIT_ON_FAILED_ASSERT_FOR
auto constexpr FUZZING_EXIT_ON_FAILED_ASSERT_FOR = "";
#endif

[[noreturn]] void Terminate(const char *file)
{
    auto filepath = std::string(file);
    std::string libsTmp;

    char *replace = std::getenv("FUZZING_EXIT_ON_FAILED_ASSERT");
    if ((replace != nullptr) && (std::string(replace) == "false")) {
        std::abort();
    }

    char *libs = std::getenv("FUZZING_EXIT_ON_FAILED_ASSERT_FOR");
    if (libs == nullptr) {
        libsTmp = FUZZING_EXIT_ON_FAILED_ASSERT_FOR;
        if (libsTmp.empty()) {
            std::abort();
        }
        libs = libsTmp.data();
    }

    char *context;
    char *lib = strtok_s(libs, ",", &context);
    while (lib != nullptr) {
        if (filepath.find(lib) != std::string::npos) {
            std::exit(1);
        }
        lib = strtok_s(nullptr, ",", &context);
    }
    std::abort();
}

}  // namespace ark::terminate
