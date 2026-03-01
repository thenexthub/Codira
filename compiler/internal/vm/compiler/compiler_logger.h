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

#ifndef COMPILER_COMPILER_LOGGER_H
#define COMPILER_COMPILER_LOGGER_H

#include <bitset>
#include <vector>
#include "utils/logger.h"

namespace panda::compiler {
#include "compiler_logger_components.inc"

enum CompilerLoggerComponents : uint8_t {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF(COMPONENT, ...) COMPONENT,
    COMPILER_LOG_COMPONENTS(DEF)
#undef DEF
        LOG_COMPONENTS_NUM
};

class CompilerLogger {
    NO_COPY_SEMANTIC(CompilerLogger);
    NO_MOVE_SEMANTIC(CompilerLogger);

public:
    static void SetComponents(const std::vector<std::string> &args);

    static void Init([[maybe_unused]] const std::vector<std::string> &args)
    {
#ifndef NDEBUG
        SetComponents(args);
#endif
    }

    static inline bool IsComponentEnabled(CompilerLoggerComponents comp)
    {
        return components.test(comp);
    }

    static void EnableComponent(CompilerLoggerComponents comp)
    {
        components.set(comp);
    }

private:
    CompilerLogger() = default;
    ~CompilerLogger() = default;

private:
    static std::bitset<CompilerLoggerComponents::LOG_COMPONENTS_NUM> components;
};

#ifndef NDEBUG

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMPILER_LOG(level, comp) \
    CompilerLogger::IsComponentEnabled(CompilerLoggerComponents::comp) && LOG(level, COMPILER) << "[" #comp "] "

#else

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMPILER_LOG(level, comp) \
    CompilerLogger::IsComponentEnabled(CompilerLoggerComponents::comp) && LOG(level, COMPILER) << "[" #comp "] "

#endif
}  // namespace panda::compiler

#endif  // COMPILER_COMPILER_LOGGER_H
