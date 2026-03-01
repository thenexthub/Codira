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
#ifndef PANDA_RUNTIME_MEM_GC_LANG_GC_LANG_H
#define PANDA_RUNTIME_MEM_GC_LANG_GC_LANG_H

#include "runtime/mem/gc/gc.h"

namespace ark::mem {

// GCLang class is an interlayer between language-agnostic GC class and different implementations of GC.
// It contains language-specific methods that are used in several types of GC (such as StwGC, etc.)
//
//                              GC
//                              ^
//                              |
//                       GCLang<SpecificLanguage>
//                       ^           ^    ...   ^
//                       |           |    ...   |
// 	                    /            |    ...
//                     /             |    ...
// StwGC<SpecificLanguage> G1GC<SpecificLanguage> ...

template <class LanguageConfig>
class GCLang : public GC {
public:
    explicit GCLang(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);
    NO_COPY_SEMANTIC(GCLang);
    NO_MOVE_SEMANTIC(GCLang);
    void SetPandaVM(PandaVM *vm) override
    {
        rootManager_ = RootManager<LanguageConfig>(vm);
        GC::SetPandaVM(vm);
    }

    bool IsMutatorAllowed() override;

protected:
    ~GCLang() override;
    void UpdateRefsToMovedObjectsInPygoteSpace() override;
    void CommonUpdateAndSweepVmRefs() override;

    void VisitRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags) override
    {
        trace::ScopedTrace scopedTrace(__FUNCTION__);
        rootManager_.VisitNonHeapRoots(gcRootVisitor, flags);
    }

    void VisitClassRoots(const GCRootVisitor &gcRootVisitor) override
    {
        trace::ScopedTrace scopedTrace(__FUNCTION__);
        rootManager_.VisitClassRoots(gcRootVisitor);
    }

    void VisitCardTableRoots(CardTable *cardTable, const GCRootVisitor &gcRootVisitor,
                             const MemRangeChecker &rangeChecker, const ObjectChecker &rangeObjectChecker,
                             const ObjectChecker &fromObjectChecker, uint32_t processedFlag) override
    {
        rootManager_.VisitCardTableRoots(cardTable, GetObjectAllocator(), gcRootVisitor, rangeChecker,
                                         rangeObjectChecker, fromObjectChecker, processedFlag);
    }

    void PreRunPhasesImpl() override;

    size_t VerifyHeap() override;

private:
    void ClearLocalInternalAllocatorPools() override;

    RootManager<LanguageConfig> rootManager_ {nullptr};
    friend class RootManager<LanguageConfig>;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_LANG_GC_LANG_H
