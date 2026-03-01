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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H

#include <memory>
#include "libarkbase/macros.h"

struct NapiStackInfo;

namespace ark::ets {
class EtsCoroutine;
}  // namespace ark::ets

namespace ark::ets::interop::js {

class InteropCtx;

class StackInfoManagerBase {
public:
    StackInfoManagerBase([[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] EtsCoroutine *coro)
        : ctx_(ctx), mainCoro_(coro)
    {
        UNUSED_VAR(ctx_);
        UNUSED_VAR(mainCoro_);
    }
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void InitStackInfoIfNeeded() {};
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void UpdateStackInfoIfNeeded() {};
    ~StackInfoManagerBase() = default;

    NO_MOVE_SEMANTIC(StackInfoManagerBase);
    NO_COPY_SEMANTIC(StackInfoManagerBase);

protected:
    InteropCtx *ctx_;         // NOLINT(misc-non-private-member-variables-in-classes)
    EtsCoroutine *mainCoro_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class StackInfoManagerOhos : public StackInfoManagerBase {
public:
    StackInfoManagerOhos(InteropCtx *ctx, EtsCoroutine *coro);
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void InitStackInfoIfNeeded();
    // NOTE(konstanting, #23205): revert to ALWAYS_INLINE once the migration of ets_vm_plugin.cpp to ANI is completed
    PANDA_PUBLIC_API void UpdateStackInfoIfNeeded();
    ~StackInfoManagerOhos();

    NO_MOVE_SEMANTIC(StackInfoManagerOhos);
    NO_COPY_SEMANTIC(StackInfoManagerOhos);

private:
    std::unique_ptr<NapiStackInfo> mainStackInfo_ {};
};

#if defined(PANDA_TARGET_OHOS) || defined(PANDA_JS_ETS_HYBRID_MODE)
using StackInfoManager = StackInfoManagerOhos;
#else
using StackInfoManager = StackInfoManagerBase;
#endif

}  // namespace ark::ets::interop::js
#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_OHOS_STACK_INFO_H