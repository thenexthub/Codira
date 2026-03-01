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

#ifndef RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H
#define RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H

#include <type_traits>
#include "runtime/mem/gc/generational-gc-base.h"

namespace ark::mem {

template <class LanguageConfig>
template <typename Marker, class... ReferenceCheckPredicate>
void GenerationalGC<LanguageConfig>::MarkStack(Marker *marker, GCMarkingStackType *stack,
                                               const GC::MarkPreprocess &markPreprocess,
                                               const ReferenceCheckPredicate &...refPred)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    // Check that we use correct type for ref_predicate (or not use it at all)
    static_assert(sizeof...(ReferenceCheckPredicate) == 0 ||
                  (sizeof...(ReferenceCheckPredicate) == 1 &&
                   std::is_constructible_v<ReferenceCheckPredicateT, ReferenceCheckPredicate...>));
    ASSERT(stack != nullptr);
    while (!stack->Empty()) {
        auto *object = this->PopObjectFromStack(stack);
        ASSERT(marker->IsMarked(object));
        ValidateObject(nullptr, object);
        auto *objectClass = object->template NotAtomicClassAddr<BaseClass>();
        // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
        TSAN_ANNOTATE_HAPPENS_AFTER(objectClass);
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);

        ASSERT(!object->IsForwarded());
        markPreprocess(object, objectClass);
        static_cast<Marker *>(marker)->MarkInstance(stack, object, objectClass, refPred...);
    }
}

template <class LanguageConfig>
template <typename Marker>
NO_THREAD_SAFETY_ANALYSIS void GenerationalGC<LanguageConfig>::MarkImpl(Marker *marker,
                                                                        GCMarkingStackType *objectsStack,
                                                                        CardTableVisitFlag visitCardTableRoots,
                                                                        const ReferenceCheckPredicateT &refPred,
                                                                        const MemRangeChecker &memRangeChecker,
                                                                        const GC::MarkPreprocess &markPreprocess)
{
    // concurrent visit class roots
    this->VisitClassRoots([this, marker, objectsStack](const GCRoot &gcRoot) {
        auto *object = gcRoot.GetObjectHeader();
        if (marker->MarkIfNotMarked(object)) {
            ASSERT(object != nullptr);
            objectsStack->PushToStack(RootType::ROOT_CLASS, object);
        } else {
            LOG_DEBUG_GC << "Skip root: " << object;
        }
    });
    MarkStack(marker, objectsStack, markPreprocess, refPred);
    {
        ScopedTiming t1("VisitInternalStringTable", *this->GetTiming());
        auto visitor = [marker, objectsStack](GCRoot root) {
            auto str = root.GetObjectHeader();
            if (marker->MarkIfNotMarked(str)) {
                ASSERT(str != nullptr);
                objectsStack->PushToStack(RootType::STRING_TABLE, str);
            }
        };
        this->GetPandaVm()->VisitStringTable(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL |
                                                          VisitGCRootFlags::START_RECORDING_NEW_ROOT);
    }
    MarkStack(marker, objectsStack, markPreprocess, refPred);

    // concurrent visit card table
    if (visitCardTableRoots == CardTableVisitFlag::VISIT_ENABLED) {
        this->VisitCardTableConcurrent(marker, objectsStack, refPred, memRangeChecker, markPreprocess);
    }
}

template <class LanguageConfig>
template <typename Marker>
void GenerationalGC<LanguageConfig>::VisitCardTableConcurrent(Marker *marker, GCMarkingStackType *objectsStack,
                                                              const ReferenceCheckPredicateT &refPred,
                                                              const MemRangeChecker &memRangeChecker,
                                                              const GC::MarkPreprocess &markPreprocess)
{
    GCRootVisitor gcMarkRoots = [this, marker, objectsStack, &refPred](const GCRoot &gcRoot) {
        ObjectHeader *fromObject = gcRoot.GetFromObjectHeader();
        if (UNLIKELY(fromObject != nullptr) &&
            this->IsReference(fromObject->ClassAddr<BaseClass>(), fromObject, refPred)) {
            LOG_DEBUG_GC << "Add reference: " << GetDebugInfoAboutObject(fromObject) << " to stack";
            marker->Mark(fromObject);
            this->ProcessReference(objectsStack, fromObject->ClassAddr<BaseClass>(), fromObject,
                                   GC::EmptyReferenceProcessPredicate);
        } else {
            auto *object = gcRoot.GetObjectHeader();
            if (marker->MarkIfNotMarked(object)) {
                objectsStack->PushToStack(gcRoot.GetType(), object);
            }
        }
    };

    auto allocator = this->GetObjectAllocator();
    MemRangeChecker rangeChecker = [&memRangeChecker](MemRange &memRange) -> bool { return memRangeChecker(memRange); };
    ObjectChecker tenuredObjectChecker = [&allocator](const ObjectHeader *objectHeader) -> bool {
        return !allocator->IsObjectInYoungSpace(objectHeader);
    };
    ObjectChecker fromObjectChecker = [marker](const ObjectHeader *objectHeader) -> bool {
        return marker->IsMarked(objectHeader);
    };
    this->VisitCardTableRoots(this->GetCardTable(), gcMarkRoots, rangeChecker, tenuredObjectChecker, fromObjectChecker,
                              CardTableProcessedFlag::VISIT_MARKED | CardTableProcessedFlag::VISIT_PROCESSED |
                                  CardTableProcessedFlag::SET_PROCESSED);
    MarkStack(marker, objectsStack, markPreprocess, refPred);
}

}  // namespace ark::mem
#endif  // RUNTIME_MEM_GC_GENERATIONAL_GC_BASE_INL_H
