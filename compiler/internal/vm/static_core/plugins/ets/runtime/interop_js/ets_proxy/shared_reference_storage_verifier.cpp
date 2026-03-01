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

#include <cstring>
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage_verifier.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js::ets_proxy {

#define LOG_REF_VERIFIER LOG(ERROR, ETS_INTEROP_JS) << "REF_VERIFIER: "

ALWAYS_INLINE static bool IsJsObjectAlive([[maybe_unused]] const SharedReference *ref)
{
    bool result = false;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    napi_is_alive_object(ref->GetCtx()->GetJSEnv(), ref->GetJsRef(), &result);
#endif  // PANDA_JS_ETS_HYBRID_MODE
    return result;
}

ALWAYS_INLINE static bool JsObjectContainObject([[maybe_unused]] const SharedReference *ref)
{
    bool result = false;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    napi_is_contain_object(ref->GetCtx()->GetJSEnv(), ref->GetJsRef(), &result);
#endif  // PANDA_JS_ETS_HYBRID_MODE
    return result;
}

ALWAYS_INLINE static bool IsJsXrefObjectType([[maybe_unused]] const SharedReference *ref,
                                             [[maybe_unused]] napi_value jsObject)
{
    bool result = false;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    napi_is_xref_type(ref->GetCtx()->GetJSEnv(), jsObject, &result);
#endif  // PANDA_JS_ETS_HYBRID_MODE
    return result;
}

size_t SharedReferenceStorageVerifier::TraverseAllItems(const SharedReferenceStorage *const storage, XgcStatus status)
{
    size_t capacity = storage->Capacity();
    size_t failCount = 0;
    for (size_t index = 1U; index < capacity; ++index) {
        failCount += VerifyOneItem(storage, index, status);
    }
    LOG_IF(failCount > 0, ERROR, ETS_INTEROP_JS) << "REF_VERIFIER: Shared reference verify total fails: " << failCount;
    return failCount;
}

size_t SharedReferenceStorageVerifier::VerifyOneItem(const SharedReferenceStorage *const storage, size_t index,
                                                     XgcStatus status)
{
    size_t failCount = 0;
    const SharedReference *item = storage->GetItemByIndex(index);
    if (item->IsEmpty()) {
        LOG_REF_VERIFIER << "Shared references is empty object at " << index << ", capacity is" << storage->Capacity();
        return failCount;
    }
    failCount = CheckObjectAlive(item, index);
    failCount += CheckObjectReindex(storage, item, index);
    failCount += CheckObjectAddressValid(item, index);
    failCount += CheckJsObjectType(item, index);
    if (status == XgcStatus::XGC_FINISHED) {
        failCount += IsAllItemsMarked(item, index);
    }
    return static_cast<size_t>(failCount != 0U);
}

size_t SharedReferenceStorageVerifier::CheckObjectAlive(const SharedReference *item, size_t index)
{
    size_t failCount = CheckJsObjectAlive(item, index);
    failCount += CheckJEtsObjectAlive(item, index);
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckJEtsObjectAlive(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    ObjectHeader *obj = item->GetEtsObject()->GetCoreType();
    if (!item->GetCtx()->GetPandaEtsVM()->GetHeapManager()->IsLiveObject(obj)) {
        LOG_REF_VERIFIER << "Shared reference corruption found! ETS object address does not alive at " << std::hex
                         << item << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckJsObjectAlive(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    if (!IsJsObjectAlive(item)) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! JS object address does not alive at " << std::hex
                         << item << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckObjectReindex(const SharedReferenceStorage *const storage,
                                                          const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    if (!item->HasETSFlag() && item->HasJSFlag()) {
        failCount += CheckJsObjectReindex(storage, item, index);
    }
    failCount += CheckEtsObjectReindex(item, index);
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckJsObjectReindex(const SharedReferenceStorage *const storage,
                                                            const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    napi_value result = nullptr;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    napi_get_reference_value(const_cast<InteropCtx *>(item->GetCtx())->GetJSEnv(), item->GetJsRef(), &result);
#endif
    if (item != storage->GetReference(const_cast<InteropCtx *>(item->GetCtx())->GetJSEnv(), result)) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! JS object address cannot reindex at " << std::hex
                         << item << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckEtsObjectReindex(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    int32_t idx = item->GetEtsObject()->GetInteropIndex();
    if (const_cast<InteropCtx *>(item->GetCtx())->GetSharedRefStorage()->GetItemByIndex(idx) != item) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! Ets object address cannot reindex at " << std::hex
                         << item << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckObjectAddressValid(const SharedReference *item, size_t index)
{
    size_t failCount = CheckJsObjectAddress(item, index);
    failCount += CheckEtsObjectAddress(item, index);
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckJsObjectAddress(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    if (!JsObjectContainObject(item)) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! Js object address is invalid at " << std::hex << item
                         << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckEtsObjectAddress(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    ObjectHeader *obj = item->GetEtsObject()->GetCoreType();
    if (!const_cast<InteropCtx *>(item->GetCtx())->GetPandaEtsVM()->GetHeapManager()->ContainObject(obj)) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! Ets object address is invalid at " << std::hex << item
                         << "," << index;
        failCount++;
    }
    return failCount;
}

size_t SharedReferenceStorageVerifier::CheckJsObjectType(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    if (!CheckJsObjectTypeIsJsXref(item)) {
        LOG_REF_VERIFIER << "Shared Reference corruption found! Js Type is not JSType::JS_XREF_OBJECT at " << std::hex
                         << item << "," << index;
        failCount++;
    }
    return failCount;
}

bool SharedReferenceStorageVerifier::CheckJsObjectTypeIsJsXref(const SharedReference *item)
{
    napi_value jsObject = nullptr;
    napi_get_reference_value(const_cast<InteropCtx *>(item->GetCtx())->GetJSEnv(), item->GetJsRef(), &jsObject);
    return IsJsXrefObjectType(item, jsObject);
}

size_t SharedReferenceStorageVerifier::IsAllItemsMarked(const SharedReference *item, size_t index)
{
    size_t failCount = 0;
    if (!item->IsMarked()) {
        failCount++;
        LOG_REF_VERIFIER << "Shared Reference corruption found! Xgc finished but not marked at " << std::hex << item
                         << "," << index;
    }
    return failCount;
}
}  // namespace ark::ets::interop::js::ets_proxy