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

// NOLINTBEGIN(readability-identifier-naming)
#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_ESVALUE_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_ESVALUE_H

#include <ani.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the native pointer previously wrapped in the ESValue.
 * This function returns the native pointer (originally set via napi_wrap or equivalent)
 * from an ESValue.
 * @param env The ANI environment in which this function is called.
 * @param esValue The previously wrapped ESValue.
 * @param result [out] Pointer to receive the unwrapped native pointer. Must not be null.
 * Notes:
 *  This API can be called only from ANI code.
 *  Experimental API, only for internal usage.
 */
__attribute__((visibility("default"))) bool arkts_esvalue_unwrap(ani_env *env, ani_object esValue, void **result);

#ifdef __cplusplus
}
#endif
// NOLINTEND(readability-identifier-naming)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_ESVALUE_H
