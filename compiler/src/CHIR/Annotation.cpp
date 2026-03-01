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

#include "Codira/CHIR/Annotation.h"
#include <iostream>
#include <sstream>

#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
std::string AnnotationMap::ToString() const
{
    std::stringstream ss;
    ss << loc.ToString();
    for (auto& pair : annotations) {
        auto str = pair.second->ToString();
        if (str.empty()) {
            continue;
        }
        if (ss.str() != "") {
            ss << ", ";
        }
        ss << str;
    }
    return ss.str();
}

std::string SkipCheck::ToString()
{
    switch (kind) {
        case SkipKind::SKIP_DCE_WARNING:
            return "skip: dce warning";
        case SkipKind::SKIP_FORIN_EXIT:
            return "skip: for-in exit";
        case SkipKind::SKIP_VIC:
            return "skip: vic";
        default:
            return "";
    }
}

std::string WrappedRawMethod::ToString()
{
    // WrappedRawMethod may be removed body when removeUnusedImported，do not form it.
    auto wrapMethod = dynamic_cast<Func*>(rawMethod);
    if (wrapMethod != nullptr && !wrapMethod->GetBody()) {
        return "";
    }

    return "wrapped raw method: " + rawMethod->GetIdentifier();
}
} // namespace Codira::CHIR
