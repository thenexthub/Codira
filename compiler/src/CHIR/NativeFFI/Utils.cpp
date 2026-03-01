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
 * This file implements utility functions for NativeFFI logic in CHIR.
 */

#include "Codira/CHIR/NativeFFI/Utils.h"

using namespace Codira::CHIR;
using namespace Codira::CHIR::Native::FFI;

namespace {
bool IsObjCMirror(const ClassDef& classDef)
{
    return classDef.TestAttr(Attribute::OBJ_C_MIRROR);
}

bool IsJavaMirror(const ClassDef& classDef)
{
    return classDef.TestAttr(Attribute::JAVA_MIRROR);
}
} // namespace

bool Codira::CHIR::Native::FFI::IsMirror(const ClassDef& classDef)
{
    return IsObjCMirror(classDef) || IsJavaMirror(classDef);
}

std::vector<uint64_t> Codira::CHIR::Native::FFI::FindHasInitedField(const ClassDef& classDef)
{
    auto index = std::vector<uint64_t>{};
    const auto ivars = classDef.GetDirectInstanceVars();
    const auto superMembersNum = classDef.GetAllInstanceVarNum() - ivars.size();
    for (size_t i = 0; i < ivars.size(); ++i) {
        const auto ivar = ivars[i];

        if (ivar.TestAttr(Attribute::HAS_INITED_FIELD)) {
            index.emplace_back(superMembersNum + i);
            break;
        }
    }

    return index;
}
