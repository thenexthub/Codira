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

#include "TweakRegistry.h"
#include "../../logger/Logger.h"
#include "Tweak.h"
#include "tweaks/ExtractFunction.h"
#include "tweaks/ExtractVariable.h"
#include "tweaks/IntroduceConstant.h"

namespace ark {
#define REGISTER_TWEAK(TweakClass) \
    namespace { \
    struct TweakClass##Registrar { \
        TweakClass##Registrar() noexcept { \
            TweakRegistry::RegisterTweak(#TweakClass, []{ \
                return std::make_unique<TweakClass>(); \
            }); \
        } \
    }; \
    [[maybe_unused]] TweakClass##Registrar TweakClass##_registrar; \
    }

REGISTER_TWEAK(ExtractFunction)
REGISTER_TWEAK(ExtractVariable)
REGISTER_TWEAK(IntroduceConstant)

std::unordered_map<std::string, TweakRegistry::Creator>& TweakRegistry::GetRegistry()
{
    static std::unordered_map<std::string, TweakRegistry::Creator> registry;
    return registry;
}

void TweakRegistry::RegisterTweak(const std::string &id, TweakRegistry::Creator creator)
{
    GetRegistry().emplace(id, std::move(creator));
}

std::unique_ptr<Tweak> TweakRegistry::Create(const std::string& id)
{
    auto &registry = GetRegistry();
    if (auto it = registry.find(id); it != registry.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> TweakRegistry::AvailableIds()
{
    std::vector<std::string> ids;
    for (const auto &[id, _] : GetRegistry()) {
        ids.push_back(id);
    }
    return ids;
}
} // namespace ark
