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

#include "Codira/CHIR/ImplicitImportedFuncMgr.h"

using namespace Codira;
using namespace CHIR;

ImplicitImportedFuncMgr& ImplicitImportedFuncMgr::Instance() noexcept
{
    static ImplicitImportedFuncMgr instance;
    return instance;
}

void ImplicitImportedFuncMgr::RegImplicitImportedFunc(const ImplicitImportedFunc& func, FuncKind funcKind) noexcept
{
    if (funcKind == FuncKind::GENERIC) {
        implicitImportedGenericFuncs.emplace_back(func);
    } else if (funcKind == FuncKind::NONE_GENERIC) {
        implicitImportedNonGenericFuncs.emplace_back(func);
    } else {
        CODEC_ASSERT(false && "Invalid funcKind.");
    }
}

std::vector<ImplicitImportedFunc> ImplicitImportedFuncMgr::GetImplicitImportedFuncs(FuncKind funcKind)
{
    static const auto COMP = [](const ImplicitImportedFunc& lhs, const ImplicitImportedFunc& rhs) {
        return lhs.parentName + lhs.identifier < rhs.parentName + rhs.identifier;
    };

    if (funcKind == FuncKind::GENERIC) {
        sort(implicitImportedGenericFuncs.begin(), implicitImportedGenericFuncs.end(), COMP);
        return implicitImportedGenericFuncs;
    } else if (funcKind == FuncKind::NONE_GENERIC) {
        sort(implicitImportedNonGenericFuncs.begin(), implicitImportedNonGenericFuncs.end(), COMP);
        return implicitImportedNonGenericFuncs;
    } else {
        CODEC_ASSERT(false && "Invalid funcKind.");
        return {};
    }
}
