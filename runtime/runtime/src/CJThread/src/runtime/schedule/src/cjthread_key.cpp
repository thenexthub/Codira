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


#include <cstdint>
#include "schedule_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CODEThreadKeyInternal g_codethreadKeys;

int CODEThreadKeyCreateInner(CODEThreadKey *key, DestructorFunc destructor)
{
    unsigned int index;
    if (key == nullptr) {
        return ERRNO_SCHD_CODETHREAD_KEY_INVALID;
    }

    index = atomic_fetch_add(&g_codethreadKeys.count, 1u);
    if (index >= CODETHREAD_KEYS_MAX) {
        atomic_fetch_sub(&g_codethreadKeys.count, 1u);
        return ERRNO_SCHD_CODETHREAD_KEY_FULL;
    }
    *key = index;
    atomic_store(&g_codethreadKeys.keyDestructor[index], (uintptr_t)destructor);

    return 0;
}

int CODEThreadSetspecificInner(CODEThreadKey key, void *value)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return ERRNO_SCHD_CODETHREAD_NULL;
    }
    if (key >= g_codethreadKeys.count) {
        return ERRNO_SCHD_CODETHREAD_KEY_INVALID;
    }
    codethread->localData[key] = value;
    return 0;
}

void *CODEThreadGetspecificInner(CODEThreadKey key)
{
    struct CODEThread *codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return nullptr;
    }
    if (key >= g_codethreadKeys.count) {
        return nullptr;
    }
    return codethread->localData[key];
}

void CODEThreadKeysClean(struct CODEThread *codethread)
{
    unsigned int i;
    DestructorFunc func;
    for (i = 0; i < g_codethreadKeys.count; i++) {
        func = reinterpret_cast<DestructorFunc>(atomic_load(&g_codethreadKeys.keyDestructor[i]));
        if (codethread->localData[i] != nullptr) {
            if (func != nullptr) {
                func(codethread->localData[i]);
            }
            codethread->localData[i] = nullptr;
        }
    }
}

#ifdef __cplusplus
}
#endif
