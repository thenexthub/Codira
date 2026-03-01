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

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "types/ets_array.h"

namespace ark::ets::intrinsics {

extern "C" EtsObject *StdCoreReflectInternalsCreateFixedArray(EtsClass *component, EtsInt length)
{
    ASSERT(component != nullptr);

    auto *coro = EtsCoroutine::GetCurrent();
    if (UNLIKELY(length < 0)) {
        ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_ERROR,
                               "Array length can't be negative");
        return nullptr;
    }

    auto *result = EtsObjectArray::Create(component, length);
    if (UNLIKELY(result == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    return result->AsObject();
}

}  // namespace ark::ets::intrinsics
