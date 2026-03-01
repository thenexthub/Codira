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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_EXTERNAL_ARRAY_BUFFER_H
#define PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_EXTERNAL_ARRAY_BUFFER_H

#include "ani.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AniFinalizer)(void *data, void *hint);

/**
 * @brief Instantiates ArrayBuffer with a given pointer set as the data storage.
 *
 * @param[in] env A pointer to the environment structure.
 * @param[in] externalData The pointer to be used by ArrayBuffer as data storage.
 * @param[in] length Size of memory allocated at `externalData`.
 * @param[out] resultBuffer A pointer to store the created ArrayBuffer.
 * @return Returns a status code of type `ani_status` indicating success or failure.
 */
__attribute__((visibility("default"))) ani_status CreateExternalArrayBuffer(ani_env *env, void *externalData,
                                                                            ani_size length,
                                                                            ani_arraybuffer *resultBuffer);

/**
 * @brief Instantiates ArrayBuffer with a given pointer set as the data storage and attaches finalizer to the instance.
 *
 * It is guaranteed that finalizer callback will be executed when either of the following happens:
 * 1. ArrayBuffer instance becomes non-reachable;
 * 2. VM is destructed.
 * There is no guarantee on the thread on which the callback will be executed,
 * including whether the thread will be managed or native.
 *
 * @param[in] env A pointer to the environment structure.
 * @param[in] externalData The pointer to be used by ArrayBuffer as data storage.
 * @param[in] length Size of memory allocated at `externalData`.
 * @param[in] finalizer Callback which will be executed after the ArrayBuffer becomes non-reachable.
 * @param[in] finalizerHint Additional argument will be passed to the `finalizer` callback.
 * @param[out] resultBuffer A pointer to store the created ArrayBuffer.
 * @return Returns a status code of type `ani_status` indicating success or failure.
 */
// CC-OFFNXT(G.FUN.01-CPP) public API
__attribute__((visibility("default"))) ani_status CreateFinalizableArrayBuffer(ani_env *env, void *externalData,
                                                                               ani_size length, AniFinalizer finalizer,
                                                                               void *finalizerHint,
                                                                               ani_arraybuffer *resultBuffer);

/**
 * @brief Detaches native data from the given external ArrayBuffer.
 *
 * This function unregisters previously attached finalizer, so its caller's responsibility to release memory.
 *
 * @param[in] env A pointer to the environment structure.
 * @param[in] buffer The reference to the ArrayBuffer which memory should be detached from.
 * @return Returns a status code of type `ani_status` indicating success or failure.
 */
__attribute__((visibility("default"))) ani_status DetachArrayBuffer(ani_env *env, ani_arraybuffer buffer);

/**
 * @brief Checks whether the ArrayBuffer was detached.
 *
 * @param[in] env A pointer to the environment structure.
 * @param[in] buffer The reference to the ArrayBuffer which will be checked.
 * @param[out] result A pointer to store the boolean result (true if the ArrayBuffer was detached, false otherwise).
 * @return Returns a status code of type `ani_status` indicating success or failure.
 */
__attribute__((visibility("default"))) ani_status IsDetachedArrayBuffer(ani_env *env, ani_arraybuffer buffer,
                                                                        ani_boolean *result);

#ifdef __cplusplus
}
#endif

#endif  // PANDA_PLUGINS_ETS_RUNTIME_LIBANI_HELPERS_EXTERNAL_ARRAY_BUFFER_H
