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

#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/gc/dynamic/gc_dynamic_data.h"

namespace ark::mem {

template <class LanguageConfig>
GCLang<LanguageConfig>::GCLang(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GC(objectAllocator, settings)
{
    if constexpr (LanguageConfig::LANG_TYPE == LANG_TYPE_DYNAMIC) {  // NOLINT
        auto allocator = GetInternalAllocator();
        GCDynamicData *data = allocator->template New<GCDynamicData>(allocator);
        SetExtensionData(data);
    }
}

template <class LanguageConfig>
GCLang<LanguageConfig>::~GCLang()
{
    GCExtensionData *data = GetExtensionData();
    if (GetExtensionData() != nullptr) {
        InternalAllocatorPtr allocator = GetInternalAllocator();
        allocator->Delete(data);
    }
}

template <class LanguageConfig>
void GCLang<LanguageConfig>::ClearLocalInternalAllocatorPools()
{
    auto cleaner = [](ManagedThread *thread) {
        InternalAllocator<>::RemoveFreePoolsForLocalInternalAllocator(thread->GetLocalInternalAllocator());
        return true;
    };
    GetPandaVm()->GetThreadManager()->EnumerateThreads(cleaner);
}

template <class LanguageConfig>
size_t GCLang<LanguageConfig>::VerifyHeap()
{
    if (GetSettings()->EnableFastHeapVerifier()) {
        return FastHeapVerifier<LanguageConfig>(GetPandaVm()->GetHeapManager()).VerifyAll();
    }
    return HeapVerifier<LanguageConfig>(GetPandaVm()->GetHeapManager()).VerifyAll();
}

template <class LanguageConfig>
void GCLang<LanguageConfig>::UpdateRefsToMovedObjectsInPygoteSpace()
{
    GetObjectAllocator()->IterateNonRegularSizeObjects(
        [](ObjectHeader *obj) { ObjectHelpers<LanguageConfig::LANG_TYPE>::UpdateRefsToMovedObjects(obj); });
}

template <class LanguageConfig>
void GCLang<LanguageConfig>::CommonUpdateAndSweepVmRefs()
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);

    rootManager_.VisitNonHeapRoots([](GCRoot root) {
        if (root.GetObjectHeader()->IsForwarded()) {
            root.Update(GetForwardAddress(root.GetObjectHeader()));
        }
    });
    auto callback = [](ObjectHeader **objPtr) {
        ObjectHeader *object = *objPtr;
        if (object->IsForwarded()) {
            *objPtr = GetForwardAddress(object);
        }
        return ObjectStatus::ALIVE_OBJECT;
    };
    rootManager_.UpdateAndSweep(callback);
}

template <class LanguageConfig>
void GCLang<LanguageConfig>::PreRunPhasesImpl()
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LanguageConfig::MT_MODE != MT_MODE_SINGLE) {
        GetPandaVm()->FreeInternalResources();
    }
}

TEMPLATE_GC_IS_MUTATOR_ALLOWED()
TEMPLATE_CLASS_LANGUAGE_CONFIG(GCLang);

}  // namespace ark::mem
