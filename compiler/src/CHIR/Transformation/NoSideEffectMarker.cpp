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

#include "Codira/CHIR/Transformation/NoSideEffectMarker.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Codira/Utils/TaskQueue.h"
#include "Codira/CHIR/Analysis/Utils.h"

namespace Codira::CHIR {

static const std::unordered_set<std::string> STD_NO_SIDE_EFFECT_LIST = {
#include "Codira/CHIR/Transformation/StdNoSideEffectwhiteList.inc"
};

static const std::vector<std::string> NO_SIDE_EFFECT_PACKAGES = {"std"};

void NoSideEffectMarker::RunOnPackage(const Ptr<const Package>& package, bool isDebug)
{
    for (auto func : package->GetGlobalFuncs()) {
        RunOnFunc(func, isDebug);
    }
    for (auto imported : package->GetImportedVarAndFuncs()) {
        if (!imported->IsImportedVar()) {
            RunOnFunc(imported, isDebug);
        }
    }
}

void NoSideEffectMarker::RunOnFunc(const Ptr<Value>& value, bool isDebug)
{
    std::string packageName;
    std::string mangleName;
    if (auto func = DynamicCast<FuncBase>(value)) {
        packageName = func->GetPackageName();
        mangleName = func->GetRawMangledName();
    } else {
        return;
    }
    if (!CheckPackage(packageName)) {
        return;
    }
    if (STD_NO_SIDE_EFFECT_LIST.count(mangleName) == 0) {
        return;
    }
    value->EnableAttr(Attribute::NO_SIDE_EFFECT);

    if (isDebug) {
        std::string message = "[NoSideEffectMarker] The call to function " + value->GetSrcCodeIdentifier() +
            ToPosInfo(value->GetDebugLocation()) + " has been mark as no side effect.\n";
        std::cout << message;
    }
}

bool NoSideEffectMarker::CheckPackage(const std::string& packageName)
{
    for (const auto& whitePackage : NO_SIDE_EFFECT_PACKAGES) {
        if (whitePackage.size() > packageName.size()) {
            continue;
        }
        if (packageName.compare(0, whitePackage.length(), whitePackage) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace Codira::CHIR
