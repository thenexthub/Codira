/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_VERIFIER_DEBUG_BREAKPOINT_H_
#define PANDA_VERIFIER_DEBUG_BREAKPOINT_H_

#include "runtime/include/method.h"
#include "runtime/include/mem/panda_string.h"

#include <cstdint>
#include <cstddef>

namespace ark::verifier::debug {

using Offset = uint32_t;

using Offsets = PandaVector<Offset>;

struct DebugContext;

#ifndef NDEBUG
bool CheckManagedBreakpoint(DebugContext const *ctx, Method::UniqId id, Offset offset);
bool ManagedBreakpointPresent(DebugContext const *ctx, Method::UniqId id);

// this is a macro so it doesn't show up in the backtrace
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DBG_MANAGED_BRK(ctx, method_id, method_offset)                                 \
    if (ark::verifier::debug::CheckManagedBreakpoint(ctx, method_id, method_offset)) { \
        __builtin_trap();                                                              \
    }
#else
inline bool CheckManagedBreakpoint([[maybe_unused]] DebugContext const *ctx, [[maybe_unused]] Method::UniqId id,
                                   [[maybe_unused]] Offset offset)
{
    return false;
}

inline bool ManagedBreakpointPresent([[maybe_unused]] DebugContext const *ctx, [[maybe_unused]] Method::UniqId id)
{
    return false;
}

#define DBG_MANAGED_BRK(ctx, method_id, method_offset)
#endif
}  // namespace ark::verifier::debug

#endif  // PANDA_VERIFIER_DEBUG_BREAKPOINT_H_
