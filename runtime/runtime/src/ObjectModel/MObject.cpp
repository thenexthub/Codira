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


#include "Base/Log.h"
#include "MObject.inline.h"
#include "Inspector/CodeAllocData.h"
namespace MapleRuntime {
MObject* MObject::NewObject(TypeInfo* ti, MSize size, AllocType allocType)
{
    auto addr = HeapManager::Allocate(size, allocType);
    if (LIKELY(addr != NULL_ADDRESS)) {
        (void)SetClassInfo(addr, ti);
    } else {
        return nullptr;
    }
#if defined(__OHOS__) && (__OHOS__ == 1)
    if (CodeAllocData::GetCodeAllocData()->IsRecording()) {
        CodeAllocData::GetCodeAllocData()->RecordAllocNodes(ti, size);
    }
#endif
    return Cast<MObject>(addr);
}

MObject* MObject::NewPinnedObject(TypeInfo* ti, MSize size)
{
    CHECK_DETAIL(ti->IsObjectType() == true, "must be object class.");
    auto addr = HeapManager::Allocate(size, AllocType::PINNED_OBJECT);
    if (LIKELY(addr != NULL_ADDRESS)) {
        (void)SetClassInfo(addr, ti);
    } else {
        return nullptr;
    }
#if defined(__OHOS__) && (__OHOS__ == 1)
    if (CodeAllocData::GetCodeAllocData()->IsRecording()) {
        CodeAllocData::GetCodeAllocData()->RecordAllocNodes(ti, size);
    }
#endif
    return Cast<MObject>(addr);
}

MObject* MObject::NewFinalizer(const TypeInfo* ti, MSize size)
{
    CHECK_DETAIL(ti->IsObjectType() == true, "must be object class.");
    auto addr = HeapManager::Allocate(size);
    if (LIKELY(addr != NULL_ADDRESS)) {
        (void)SetClassInfo(addr, const_cast<TypeInfo*>(ti));
        reinterpret_cast<BaseObject*>(addr)->OnFinalizerCreated();
    } else {
        return nullptr;
    }
#if defined(__OHOS__) && (__OHOS__ == 1)
    if (CodeAllocData::GetCodeAllocData()->IsRecording()) {
        CodeAllocData::GetCodeAllocData()->RecordAllocNodes(ti, size);
    }
#endif
    return Cast<MObject>(addr);
}
} // namespace MapleRuntime
