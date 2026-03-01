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

#include "verification/config/context/context.h"

#include "libarkbase/utils/logger.h"

namespace ark::verifier::debug {

void DebugConfig::AddWhitelistMethodConfig(WhitelistKind kind, const PandaString &name)
{
    whitelistNames[static_cast<size_t>(kind)].push_back(name);
    whitelistNotEmpty = true;
}

bool DebugContext::InWhitelist(WhitelistKind kind, uint64_t id) const
{
    return config->whitelistNotEmpty && whitelist.id[static_cast<size_t>(kind)]->count(id) > 0;
}

void DebugContext::InsertIntoWhitelist(const PandaString &name, bool isClassName, Method::UniqId id)
{
    auto kindsToAdd = isClassName
                          ? std::initializer_list<WhitelistKind> {WhitelistKind::CLASS}
                          : std::initializer_list<WhitelistKind> {WhitelistKind::METHOD, WhitelistKind::METHOD_CALL};

    for (auto kind : kindsToAdd) {
        auto k = static_cast<size_t>(kind);
        auto &ids = whitelist.id[k];
        auto &names = config->whitelistNames[k];
        if (std::find(names.begin(), names.end(), name) != names.end()) {
            ids->insert(id);
            LOG(DEBUG, VERIFIER) << "Method with " << (isClassName ? "class " : "") << "name " << name << ", id 0x"
                                 << std::hex << id << " was added to whitelist";
        }
    }
}

}  // namespace ark::verifier::debug
