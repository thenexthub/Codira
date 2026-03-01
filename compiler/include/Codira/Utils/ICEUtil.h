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

/**
 * @file
 *
 * This file declares ICE related variables and functions.
 */

#ifndef CODIRA_UTILS_ICE_H
#define CODIRA_UTILS_ICE_H

#include "Codira/Basic/Color.h"
#include "Codira/Utils/CheckUtils.h"

#include <string>
#include <unistd.h>

namespace Codira {
enum class CompileStage;
namespace ICE {
const int EXIT_CODE = 2; // Internal compiler error
const std::string MSG_PART_ONE = ANSI_COLOR_RED + "Internal Compiler Error: " + ANSI_COLOR_RESET;
const std::string MSG_PART_TWO = "\nPlease report this to Codira team and include the project. Error Code: ";
constexpr int64_t FRONTEND_TP = -1;
constexpr int64_t UNITTEST_TP = -2;
constexpr int64_t LSP_TP = -3;
void PrintVersionFromError();
void RemoveTempFile();
int64_t GetTriggerPoint();

class TriggerPointSetter {
public:
    TriggerPointSetter(CompileStage cs)
    {
        SetICETriggerPoint(cs);
    }

    TriggerPointSetter(int64_t tp)
    {
        SetICETriggerPoint(tp);
    }

    ~TriggerPointSetter()
    {
        SetICETriggerPoint();
    }

    friend int64_t GetTriggerPoint();

    static int64_t interpreterTP;  // Module code of the interpreter stage, which is equal to frontend code plus 1.
    static int64_t writeCahedTP;  // Module code of the write cahed stage, which is equal to frontend code plus 2.

private:
    // Save global ICE trigger point
    static int64_t triggerPoint;
    static void SetICETriggerPoint(CompileStage cs);
    static void SetICETriggerPoint(int64_t tp = ICE::FRONTEND_TP);
};

bool CanWriteOnceICEMessage();
} // namespace ICE

template <typename... Args> inline void InternalError(Args&&... args)
{
    if (ICE::CanWriteOnceICEMessage()) {
        ICE::PrintVersionFromError();
        std::cerr << ICE::MSG_PART_ONE;
        ((std::cerr << args), ...);
        int64_t tp = ICE::GetTriggerPoint();
        std::cerr << ICE::MSG_PART_TWO << std::to_string(tp) << std::endl;
        // When ut and lsp cases are compiled, do not exit after ICE,
        // because some ut cases are designed to go to the wrong branch.
        if (tp == ICE::LSP_TP || tp == ICE::UNITTEST_TP) {
            return;
        }
        ICE::RemoveTempFile();

#ifdef NDEBUG
        _exit(ICE::EXIT_CODE);
#else
        CODEC_ASSERT(false);
#endif
    }
}

template <typename... Args> inline void InternalError(bool pred, Args&&... args)
{
    if (!pred) {
        InternalError(std::forward<Args>(args)...);
    }
}

} // namespace Codira

#endif // CODIRA_UTILS_ICE_H
