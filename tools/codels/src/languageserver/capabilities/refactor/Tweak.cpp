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

#include "Tweak.h"
#include "../../logger/Logger.h"
#include "TweakRegistry.h"

namespace ark {
Tweak::Selection::Selection(ArkAST &arkAst, Range &range,
                            SelectionTree &&selectionTree, std::map<std::string, std::string> extraOptions)
    : arkAst(&arkAst), range(range), selectionTree(std::move(selectionTree))
{
    this->extraOptions = extraOptions;
}

std::vector<std::unique_ptr<Tweak>> Tweak::PrepareTweaks(const Tweak::Selection &selection,
    std::function<bool(const Tweak &)> filter)
{
    std::vector<std::unique_ptr<Tweak>> availableTweaks;
    for (const auto &id : TweakRegistry::AvailableIds()) {
        auto tweak = TweakRegistry::Create(id);
        if (tweak && filter(*tweak) && tweak->Prepare(selection)) {
            availableTweaks.push_back(std::move(tweak));
        }
    }
    return availableTweaks;
}

std::optional<std::unique_ptr<Tweak>> Tweak::PrepareTweak(std::string id, const Tweak::Selection &selection)
{
    for (const auto &tweakId : TweakRegistry::AvailableIds()) {
        if (tweakId != id) {
            continue;
        }
        auto tweak = TweakRegistry::Create(id);
        if (!tweak->Prepare(selection)) {
            return std::nullopt;
        }
        return std::move(tweak);
    }
    return std::nullopt;
}
} // namespace ark
