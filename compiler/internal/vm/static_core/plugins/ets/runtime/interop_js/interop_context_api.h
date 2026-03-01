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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_API_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_API_H

#include "libarkbase/macros.h"

namespace ark::ets {

class EtsCoroutine;

namespace interop::js {

/**
 * @brief The external function that creates the main interop context instance and does all other
 * required initialization (i.e. initalizes various modules, XGC and other things). Intended to be used by
 * the modules that should be isolated from the interop implementation details (e.g. ANI). This
 * requirement demands the use of void* instead of napi_env.
 *
 * @param mainCoro The main coroutine instance
 * @param napiEnv The napi_env for the main JSVM instance
 */
PANDA_PUBLIC_API bool CreateMainInteropContext(ark::ets::EtsCoroutine *mainCoro, void *napiEnv);

}  // namespace interop::js

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_INTEROP_CONTEXT_API_H