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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_H

#include <node_api.h>
#include "include/mem/panda_string.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/mem/mem.h"

namespace ark {
class Method;
}  // namespace ark

namespace ark::mem {
class Reference;
}  // namespace ark::mem

namespace ark::ets {
class EtsObject;
class EtsString;
class EtsCoroutine;
}  // namespace ark::ets

namespace ark::ets::interop::js {
class InteropCtx;

PANDA_PUBLIC_API napi_value CallETSInstance(EtsCoroutine *coro, InteropCtx *ctx, Method *method,
                                            Span<napi_value> jsargv, EtsObject *thisObj);
PANDA_PUBLIC_API napi_value CallETSStatic(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv);

PANDA_PUBLIC_API Expected<Method *, PandaString> ResolveEntryPoint(InteropCtx *ctx, std::string_view entryPoint);
uint8_t JSRuntimeInitJSCallClass();
uint8_t JSRuntimeInitJSNewClass();

template <char DELIM = '.', typename F>
static bool WalkQualifiedName(std::string_view name, F const &f)
{
    for (const char *p = name.data(); *p == DELIM;) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto e = ++p;
        while (*e != '\0' && *e != DELIM) {
            e++;
        }
        std::string substr(p, ToUintPtr(e) - ToUintPtr(p));
        if (UNLIKELY(!f(substr))) {
            return false;
        }
        p = e;
    }
    return true;
}

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_CALL_H
