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

#ifndef NDEBUG

#include "verification/config/context/context.h"
#include "verification/util/optional_ref.h"
#include "verification/util/str.h"

#include "verifier_messages.h"

#include "libarkbase/utils/logger.h"

namespace ark::verifier::debug {

using BreakpointConfigT = PandaVector<std::pair<PandaString, Offsets>>;

OptionalRef<Offsets> BreakpointsForName(const BreakpointConfigT &breakpointConfig, const PandaString &methodName)
{
    auto iter = std::find_if(breakpointConfig.begin(), breakpointConfig.end(),
                             [&methodName](const auto &pair) { return pair.first == methodName; });
    if (iter == breakpointConfig.end()) {
        return {};
    }
    return iter->second;
}

void DebugConfig::AddBreakpointConfig(const PandaString &methodName, Offset offset)
{
    auto optBreakpoints = BreakpointsForName(managedBreakpoints, methodName);
    if (optBreakpoints.HasRef()) {
        optBreakpoints->push_back(offset);
    } else {
        managedBreakpoints.push_back({methodName, Offsets {offset}});
    }
    LOG_VERIFIER_DEBUG_BREAKPOINT_ADDED_INFO(methodName, offset);
}

void DebugContext::InsertBreakpoints(PandaString const &methodName, Method::UniqId id)
{
    auto optBreakpoints = BreakpointsForName(config->managedBreakpoints, methodName);
    if (optBreakpoints.HasRef()) {
        for (const auto offset : optBreakpoints.Get()) {
            LOG_VERIFIER_DEBUG_BREAKPOINT_SET_INFO(methodName, id, offset);
            breakpoint.Apply([&](auto &breakpointMap) { breakpointMap[id].insert(offset); });
        }
    }
}

bool CheckManagedBreakpoint(DebugContext const *ctx, Method::UniqId id, Offset offset)
{
    return ctx->breakpoint.Apply([&](const auto &breakpointMap) {
        auto iter = breakpointMap.find(id);
        if (iter == breakpointMap.end()) {
            return false;
        }
        return iter->second.count(offset) > 0;
    });
}

bool ManagedBreakpointPresent(DebugContext const *ctx, Method::UniqId id)
{
    return ctx->breakpoint->count(id) > 0;
}

}  // namespace ark::verifier::debug

#endif
