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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NATIVE_API_ARKTS_ESVALUE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NATIVE_API_ARKTS_ESVALUE_H

#include <ani.h>
#include <node_api.h>

#include "libarkbase/macros.h"

namespace ark::ets {
class EtsCoroutine;
}  // namespace ark::ets

namespace ark::ets::interop::js {

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool UnwrapESValue(ani_env *env, ani_object esValue, void **result);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool GetCurrentNapiEnv(ani_env *env, napi_env *result);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool OpenJSToETSScope(EtsCoroutine *coro, char const *descr = nullptr);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool CloseJSToETSScope(EtsCoroutine *coro);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool OpenETSToJSScope(EtsCoroutine *coro, char const *descr = nullptr);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool CloseETSToJSScope(EtsCoroutine *coro);

/// Experimental API, only for internal usage in runtime.
PANDA_PUBLIC_API bool CloseETSToJSScope(napi_env env, size_t nValues, napi_value *values, ani_ref *result);

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_NATIVE_API_ARKTS_ESVALUE_H
