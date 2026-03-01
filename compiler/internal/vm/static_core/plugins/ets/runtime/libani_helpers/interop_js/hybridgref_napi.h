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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_NAPI_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_NAPI_H

#include "hybridgref.h"

#include <node_api.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Saves reference which could be later accessed from both NAPI and ANI code
 * @param env of current NAPI call
 * @param value to be saved
 * @param result holding reference to the saved object
 * Notes:
 *  This API can be called only from NAPI code
 * Returned `hybridgref` can be used from both NAPI and ANI code
 */
__attribute__((visibility("default"))) bool hybridgref_create_from_napi(napi_env env, napi_value value,
                                                                        hybridgref *result);

/**
 * @brief Delete a reference previously created by `hybridgref_create_from_napi`
 * @param env of current NAPI call
 * @param ref to be deleted
 * Notes:
 *  This API can be called only from NAPI code
 */
__attribute__((visibility("default"))) bool hybridgref_delete_from_napi(napi_env env, hybridgref ref);

/**
 * @brief Returns napi_value for previously saved reference
 * @param env of current NAPI call
 * @param ref to saved reference
 * @param result holding valid 1.1 object
 * Notes:
 *  This API can be called only from NAPI code
 */
__attribute__((visibility("default"))) bool hybridgref_get_napi_value(napi_env env, hybridgref ref, napi_value *result);

#ifdef __cplusplus
}
#endif
// NOLINTEND(readability-identifier-naming)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_INTEROP_JS_HYBRIDGREF_NAPI_H
