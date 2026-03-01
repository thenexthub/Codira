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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "runtime/coroutines/coroutine_worker.h"

namespace ark::ets::interop::js {

template <bool ETS_TO_JS>
inline bool OpenInteropCodeScope(EtsCoroutine *coro, char const *descr)
{
    if (UNLIKELY(coro == nullptr || coro != ManagedThread::GetCurrent())) {
        return false;
    }

    auto *ctx = InteropCtx::Current(coro);
    if constexpr (ETS_TO_JS) {
        ctx->UpdateInteropStackInfoIfNeeded();
    }
    ctx->GetOrCreateCallStack().AllocRecord(coro->GetCurrentFrame(), descr);
    return true;
}

template <bool ETS_TO_JS>
inline bool CloseInteropCodeScope(EtsCoroutine *coro)
{
    if (UNLIKELY(coro == nullptr || coro != ManagedThread::GetCurrent())) {
        return false;
    }

    auto *ctx = InteropCtx::Current(coro);
    if constexpr (ETS_TO_JS) {
        ctx->UpdateInteropStackInfoIfNeeded();
    }
    ctx->GetOrCreateCallStack().PopRecord();
    return true;
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CODE_SCOPES_INL_H
