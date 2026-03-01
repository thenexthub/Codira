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

#include "libarkbase/os/thread.h"
#include "hybridgref_napi.h"

namespace ark::ets::hygref {

using SharedRefIndex = uint32_t;

/// @brief Returns storage array
static bool GetStorage(napi_env env, napi_value *storage)
{
    static constexpr std::string_view HYBRIDGREF_STORAGE_NAME = "__hybridgref_internal_storage__";

    napi_value global;
    napi_status status;
    if (status = napi_get_global(env, &global); status != napi_ok) {
        return false;
    }

    napi_value tmpStorage;
    status = napi_get_named_property(env, global, HYBRIDGREF_STORAGE_NAME.data(), &tmpStorage);

    napi_valuetype type;
    bool shouldCreate =
        (status != napi_ok) || (napi_typeof(env, tmpStorage, &type) == napi_ok && type == napi_undefined);
    if (shouldCreate) {
        napi_value storageArray;
        if (napi_create_array(env, &storageArray) != napi_ok) {
            return false;
        }
        if (napi_set_named_property(env, global, HYBRIDGREF_STORAGE_NAME.data(), storageArray) != napi_ok) {
            return false;
        }

        napi_value tid;
        if (napi_create_uint32(env, ark::os::thread::GetCurrentThreadId(), &tid) != napi_ok) {
            return false;
        }
        if (napi_set_element(env, storageArray, 0, tid) != napi_ok) {
            return false;
        }
        *storage = storageArray;
        return true;
    }
    *storage = tmpStorage;
    return true;
}

bool HybridGrefCreateRefFromNapi(napi_env env, napi_value value, hybridgref *result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    napi_value storage;
    if (!GetStorage(env, &storage)) {
        return false;
    }
    uint32_t arrayLength;
    if (napi_get_array_length(env, storage, &arrayLength) != napi_ok) {
        return false;
    }
    if (UNLIKELY(arrayLength == 0)) {
        // Element at 0 position must always present and contain thread-id
        return false;
    }
    if (napi_set_element(env, storage, arrayLength, value) != napi_ok) {
        return false;
    }
    *result = reinterpret_cast<hybridgref>(arrayLength);
    return true;
}

static bool CheckCorrectThread(napi_env env, napi_value storage)
{
    napi_value tidHolder;
    if (napi_get_element(env, storage, 0, &tidHolder) != napi_ok) {
        return false;
    }
    uint32_t tid;
    if (napi_get_value_uint32(env, tidHolder, &tid) != napi_ok) {
        return false;
    }
    // Check that reference was created by the same JS VM instance
    return tid == ark::os::thread::GetCurrentThreadId();
}

bool HybridGrefDeleteRefFromNapi(napi_env env, hybridgref ref)
{
    if (env == nullptr) {
        return false;
    }
    napi_value storage;
    if (!GetStorage(env, &storage)) {
        return false;
    }

    uint32_t arrayLength;
    napi_get_array_length(env, storage, &arrayLength);
    if (UNLIKELY(arrayLength == 0)) {
        // Element at 0 position must always present and contain thread-id
        return false;
    }

    if (!CheckCorrectThread(env, storage)) {
        return false;
    }

    SharedRefIndex refIndex = reinterpret_cast<uintptr_t>(ref);
    if (refIndex == 0 || refIndex >= arrayLength) {
        return false;
    }

    napi_value undefinedValue;
    if (napi_get_undefined(env, &undefinedValue) != napi_ok) {
        return false;
    }

    return napi_set_element(env, storage, refIndex, undefinedValue) == napi_ok;
}

bool HybridGrefGetNapiValue(napi_env env, hybridgref ref, napi_value *result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    napi_value storage;
    if (!GetStorage(env, &storage)) {
        return false;
    }
    uint32_t arrayLength;
    napi_get_array_length(env, storage, &arrayLength);
    if (UNLIKELY(arrayLength == 0)) {
        // Element at 0 position must always present and contain thread-id
        return false;
    }

    if (!CheckCorrectThread(env, storage)) {
        return false;
    }

    SharedRefIndex refIndex = reinterpret_cast<uintptr_t>(ref);
    if (refIndex == 0 || refIndex >= arrayLength) {
        return false;
    }
    if (napi_get_element(env, storage, refIndex, result) != napi_ok) {
        return false;
    }
    return true;
}

}  // namespace ark::ets::hygref

// NOLINTBEGIN(readability-identifier-naming)
extern "C" bool hybridgref_create_from_napi(napi_env env, napi_value value, hybridgref *result)
{
    return ark::ets::hygref::HybridGrefCreateRefFromNapi(env, value, result);
}

extern "C" bool hybridgref_delete_from_napi(napi_env env, hybridgref ref)
{
    return ark::ets::hygref::HybridGrefDeleteRefFromNapi(env, ref);
}

extern "C" bool hybridgref_get_napi_value(napi_env env, hybridgref ref, napi_value *result)
{
    return ark::ets::hygref::HybridGrefGetNapiValue(env, ref, result);
}
// NOLINTEND(readability-identifier-naming)
