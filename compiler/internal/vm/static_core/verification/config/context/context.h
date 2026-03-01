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

#ifndef PANDA_VERIFIER_DEBUG_CONTEXT_H_
#define PANDA_VERIFIER_DEBUG_CONTEXT_H_

#include "verification/config/config.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification/public.h"
#include "verification/util/callable.h"
#include "verification/util/synchronized.h"

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ark::verifier::debug {

enum class WhitelistKind : uint8_t { METHOD, METHOD_CALL, CLASS, LAST };

struct DebugConfig {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    PandaUnorderedMap<PandaString, ark::verifier::callable<bool(Config *cfg, const config::Section &)>> sectionHandlers;

#ifndef NDEBUG
    // note: this is assumed to be small so stored as a vector (not even sorted, so we use a linear search)
    // if changed, a sorted vector or a PandaMap would likely be better than PandaUnorderedMap for faster comparison
    PandaVector<std::pair<PandaString, Offsets>> managedBreakpoints;
    void AddBreakpointConfig(const PandaString &methodName, Offset offset);
#else
    void AddBreakpointConfig([[maybe_unused]] const PandaString &methodName, [[maybe_unused]] Offset offset) {}
#endif

    std::array<PandaVector<PandaString>, static_cast<size_t>(WhitelistKind::LAST)> whitelistNames;
    bool whitelistNotEmpty = false;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    void AddWhitelistMethodConfig(WhitelistKind kind, const PandaString &name);
};

struct DebugContext {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    DebugConfig const *config = nullptr;

#ifndef NDEBUG
    Synchronized<PandaUnorderedMap<Method::UniqId, PandaUnorderedSet<uint32_t>>> breakpoint;
#endif

    struct {
        // similar to ManagedBreakpoints.Config
        std::array<PandaVector<PandaString>, static_cast<size_t>(WhitelistKind::LAST)> names;
        std::array<Synchronized<PandaUnorderedSet<Method::UniqId>>, static_cast<size_t>(WhitelistKind::LAST)> id;
        bool isNotEmpty = false;
    } whitelist;

    // NOLINTEND(misc-non-private-member-variables-in-classes)

public:
    void SetConfig(DebugConfig const *cfg)
    {
        config = cfg;
    }

    void AddMethod(const Method &method, bool isDebug);

    bool SkipVerification(Method::UniqId id) const
    {
        return InWhitelist(WhitelistKind::METHOD, id) || InWhitelist(WhitelistKind::CLASS, id);
    }

    bool SkipVerificationOfCall(Method::UniqId id) const
    {
        return InWhitelist(WhitelistKind::METHOD_CALL, id) || InWhitelist(WhitelistKind::CLASS, id);
    }

private:
#ifndef NDEBUG
    void InsertBreakpoints(PandaString const &methodName, Method::UniqId id);
#else
    void InsertBreakpoints([[maybe_unused]] PandaString const &methodName, [[maybe_unused]] Method::UniqId id) {}
#endif

    void InsertIntoWhitelist(const PandaString &name, bool isClassName, Method::UniqId id);
    bool InWhitelist(WhitelistKind kind, uint64_t id) const;
};
}  // namespace ark::verifier::debug

#endif  // !PANDA_VERIFIER_DEBUG_CONTEXT_H_
