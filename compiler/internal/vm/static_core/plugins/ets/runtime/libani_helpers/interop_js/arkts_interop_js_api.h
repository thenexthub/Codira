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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_INTEROP_JS_API_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_INTEROP_JS_API_H

#include <ani.h>
#include <node_api.h>

// NOLINTBEGIN(readability-identifier-naming)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens scope where N-API usage is allowed.
 *
 * This function creates a scope where N-API usage is allowed. It must be called from ANI code
 * to allow N-API usage. After the scope is opened, usage of ANI and other runtime APIs is not allowed.
 * Every opened scope must be destroyed via call to `arkts_napi_scope_close_n`.
 *
 * @param[in] env A pointer to the ANI environment structure.
 * @param[out] result A pointer to store `napi_env` corresponding to the opened scope.
 * @returns true if the scope was opened successful, false otherwise.
 */
__attribute__((visibility("default"))) bool arkts_napi_scope_open(ani_env *env, napi_env *result);

/**
 * @brief Destroys the current N-API scope and propagates references.
 *
 * This function destroys the scope which was previously opened via `arkts_napi_scope_open`.
 * After the scope is destroyed, usage of N-API is not allowed. Additionally it returns ANI compatible
 * `Any` references to the objects given in `napi_value` array to save results from the closed scope.
 *
 * @param[in] env A pointer to the N-API environment structure of the closed scope.
 * @param[in] nValues A size of `values` array. Might be 0, in this case `values` and `result` are ignored.
 * @param[in] values References to JS objects which must be returned in `result`.
 * Must be non null if `nValues` is not 0.
 * @param[out] result A pointer to store `Any` references to `values`. Must be non null if `nValues` is not 0.
 * @returns true if the scope was destroyed successful, false otherwise.
 */
__attribute__((visibility("default"))) bool arkts_napi_scope_close_n(napi_env env, size_t nValues, napi_value *values,
                                                                     ani_ref *result);

#ifdef __cplusplus
}
#endif
// NOLINTEND(readability-identifier-naming)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_ARKTS_INTEROP_JS_API_H
