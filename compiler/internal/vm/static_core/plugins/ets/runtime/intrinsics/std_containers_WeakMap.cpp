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

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::intrinsics {

/**
 * Validates that a key is suitable for use in WeakMap.
 * WeakMap keys must be reference types with stable object identity.
 * Value types (boxed primitives, strings, BigInt, null) are rejected because:
 * - They may be copied/re-boxed, breaking identity semantics
 * - They don't have stable addresses for weak reference tracking
 * @param key The object to validate as a WeakMap key (must not be nullptr)
 * @return true if key is a valid reference type, false otherwise
 */
extern "C" EtsBoolean EtsWeakMapValidateKey(EtsObject *key)
{
    ASSERT(key != nullptr);
    auto *klass = key->GetClass();
    // Reject value types that don't have stable object identity
    if (klass->IsNullValue() || klass->IsBoxed() || klass->IsStringClass() || klass->IsBigInt()) {
        return false;
    }
    // Only regular reference types are valid WeakMap keys
    return true;
}

}  // namespace ark::ets::intrinsics