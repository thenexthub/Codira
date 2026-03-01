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

#include "bytecode_optimizer/runtime_adapter.h"
#include "runtime/include/method.h"

namespace ark {

// CC-OFFNXT(G.NAM.03) false positive
static bool IsEqual(panda_file::MethodDataAccessor mda, std::initializer_list<panda_file::Type::TypeId> shorties,
                    std::initializer_list<std::string_view> refTypes)
{
    bool equal = true;
    auto shortyIt = shorties.begin();
    auto refTypeIt = refTypes.begin();
    auto &pf = mda.GetPandaFile();

    auto visitType = [&equal, &shorties, &shortyIt, &refTypes, &refTypeIt, &pf](panda_file::Type type,
                                                                                panda_file::File::EntityId classId) {
        if (!equal) {
            return;
        }
        if (shortyIt == shorties.end() || *shortyIt++ != type.GetId()) {
            equal = false;
            return;
        }
        if (type.IsReference() &&
            (refTypeIt == refTypes.end() || *refTypeIt++ != utf::Mutf8AsCString(pf.GetStringData(classId).data))) {
            equal = false;
        }
    };
    mda.EnumerateTypesInProto(visitType, true);  // true for skipThis
    return equal && shortyIt == shorties.end() && refTypeIt == refTypes.end();
}

#include "generated/get_intrinsic_id.inc"

}  // namespace ark
