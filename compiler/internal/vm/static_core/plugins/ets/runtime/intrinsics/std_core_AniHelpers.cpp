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

#include "intrinsics.h"

namespace ark::ets::intrinsics {

void AsyncWorkNativeInvoke(int64_t nativeCbPtr, int64_t dataPtr, uint8_t needNativeScope)
{
    ani_env *env = EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
    auto *nativeCb = reinterpret_cast<void (*)(ani_env *, void *)>(nativeCbPtr);
    if (static_cast<bool>(needNativeScope)) {
        ScopedNativeCodeThread sn(ManagedThread::GetCurrent());
        nativeCb(env, reinterpret_cast<void *>(dataPtr));
        return;
    }
    nativeCb(env, reinterpret_cast<void *>(dataPtr));
}

void EventNativeInvoke(int64_t nativeCbPtr, int64_t dataPtr, uint8_t needNativeScope)
{
    auto *nativeCb = reinterpret_cast<void (*)(void *)>(nativeCbPtr);
    if (static_cast<bool>(needNativeScope)) {
        ScopedNativeCodeThread sn(ManagedThread::GetCurrent());
        nativeCb(reinterpret_cast<void *>(dataPtr));
        return;
    }
    nativeCb(reinterpret_cast<void *>(dataPtr));
}

}  // namespace ark::ets::intrinsics
