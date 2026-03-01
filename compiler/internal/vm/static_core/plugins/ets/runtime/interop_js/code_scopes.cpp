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

#include "plugins/ets/runtime/interop_js/code_scopes-inl.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/native_api/arkts_interop_js_api_impl.h"

namespace ark::ets::interop::js {

ScopedInteropCallStackRecord::ScopedInteropCallStackRecord(EtsCoroutine *coro, char const *descr) : coro_(coro)
{
    [[maybe_unused]] auto status = OpenInteropCodeScope<false>(coro_, descr);
    ASSERT(status);
}

ScopedInteropCallStackRecord::~ScopedInteropCallStackRecord()
{
    [[maybe_unused]] auto status = CloseInteropCodeScope<false>(coro_);
    ASSERT(status);
}

InteropETSToJSCodeScope::InteropETSToJSCodeScope(EtsCoroutine *coro, char const *descr) : coro_(coro)
{
    [[maybe_unused]] auto status = OpenETSToJSScope(coro_, descr);
    ASSERT(status);
}

InteropETSToJSCodeScope::~InteropETSToJSCodeScope()
{
    [[maybe_unused]] auto status = CloseETSToJSScope(coro_);
    ASSERT(status);
}

InteropJSToETSCodeScope::InteropJSToETSCodeScope(EtsCoroutine *coro, char const *descr) : coro_(coro)
{
    [[maybe_unused]] auto status = OpenJSToETSScope(coro_, descr);
    ASSERT(status);
}

InteropJSToETSCodeScope::~InteropJSToETSCodeScope()
{
    [[maybe_unused]] auto status = CloseJSToETSScope(coro_);
    ASSERT(status);
}

}  // namespace ark::ets::interop::js
