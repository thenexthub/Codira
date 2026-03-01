/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::interop::js::ets_proxy {

void SharedReference::InitRef(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx)
{
    ASSERT(etsObject != nullptr);
    if (!etsObject->HasInteropIndex()) {
        etsObject->SetInteropIndex(refIdx);
    }
    SetETSObject(etsObject);
    jsRef_ = jsRef;
    ctx_ = ctx;
}

void SharedReference::InitETSObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::ETS>();
    InitRef(ctx, etsObject, jsRef, refIdx);
}

void SharedReference::InitJSObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::JS>();
    InitRef(ctx, etsObject, jsRef, refIdx);
}

void SharedReference::InitHybridObject(InteropCtx *ctx, EtsObject *etsObject, napi_ref jsRef, uint32_t refIdx)
{
    flags_.SetBit<SharedReferenceFlags::Bit::ETS>();
    flags_.SetBit<SharedReferenceFlags::Bit::JS>();
    InitRef(ctx, etsObject, jsRef, refIdx);
}

bool SharedReference::MarkIfNotMarked()
{
    return flags_.SetBit<SharedReferenceFlags::Bit::MARK>();
}

SharedReference::Iterator &SharedReference::Iterator::operator++()
{
    uint32_t index = ref_->flags_.GetNextIndex();
    if (index == 0U) {
        ref_ = nullptr;
    } else {
        ref_ = SharedReferenceStorage::GetCurrent()->GetItemByIndex(index);
    }
    return *this;
}

SharedReference::Iterator SharedReference::Iterator::operator++(int)  // NOLINT(cert-dcl21-cpp)
{
    auto result = *this;
    auto index = ref_->flags_.GetNextIndex();
    if (index == 0U) {
        ref_ = nullptr;
    } else {
        ref_ = SharedReferenceStorage::GetCurrent()->GetItemByIndex(index);
    }
    return result;
}

}  // namespace ark::ets::interop::js::ets_proxy
