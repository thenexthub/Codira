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
#ifndef PANDA_RUNTIME_MEM_GC_GC_ROOT_H
#define PANDA_RUNTIME_MEM_GC_GC_ROOT_H

#include <algorithm>
#include <ostream>
#include <vector>

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_range.h"
#include "runtime/include/class.h"
#include "runtime/include/language_config.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc_root_type.h"

namespace ark {
class PandaVM;
class ManagedThread;
class StackWalker;
}  // namespace ark

namespace ark::mem {

class GCAdaptiveMarkingStack;

using GCMarkingStackType = GCAdaptiveMarkingStack;

enum class VisitGCRootFlags : uint32_t {
    ACCESS_ROOT_ALL = 1U,
    ACCESS_ROOT_ONLY_NEW = 1U << 1U,  // used at remark to iterate over new roots only for classes and stringtable
    ACCESS_ROOT_NONE = 1U << 2U,

    ACCESS_ROOT_AOT_STRINGS_ONLY_YOUNG = 1U << 3U,  // visit only young string roots in aot table

    START_RECORDING_NEW_ROOT = 1U << 10U,
    END_RECORDING_NEW_ROOT = 1U << 11U,
};

PANDA_PUBLIC_API uint32_t operator&(VisitGCRootFlags left, VisitGCRootFlags right);

PANDA_PUBLIC_API VisitGCRootFlags operator|(VisitGCRootFlags left, VisitGCRootFlags right);

/// @brief I am groot
class GCRoot final {
public:
    GCRoot(RootType type, ObjectHeader **obj)
        : type_(type), fromObject_(nullptr), object_(reinterpret_cast<ObjectPointerType *>(obj))
    {
    }

    GCRoot(RootType type, ObjectPointerType *obj) : type_(type), fromObject_(nullptr), object_(obj) {}

    GCRoot(RootType type, ObjectHeader *fromObject, ObjectPointerType *obj)
        : type_(type), fromObject_(fromObject), object_(obj)
    {
        ASSERT(type != RootType::ROOT_TENURED || fromObject != nullptr);
    }

    GCRoot(RootType type, ObjectHeader *fromObject, ObjectHeader **obj)
        : type_(type), fromObject_(fromObject), object_(reinterpret_cast<ObjectPointerType *>(obj))
    {
        ASSERT(type != RootType::ROOT_TENURED || fromObject != nullptr);
    }

    RootType GetType() const
    {
        return type_;
    }

    ObjectHeader *GetObjectHeader() const
    {
        return reinterpret_cast<ObjectHeader *>(*object_);
    }

    ObjectPointerType *GetObjectPointer() const
    {
        return object_;
    }

    ObjectHeader *GetFromObjectHeader() const
    {
        return fromObject_;
    }

    void Update(ObjectPointerType newAddr)
    {
        *object_ = newAddr;
    }

    void Update(ObjectHeader *newAddr)
    {
        *object_ = static_cast<ObjectPointerType>(ToUintPtr(newAddr));
    }

    friend std::ostream &operator<<(std::ostream &os, const GCRoot &root);

    ~GCRoot() = default;

    DEFAULT_COPY_SEMANTIC(GCRoot);
    DEFAULT_MOVE_SEMANTIC(GCRoot);

private:
    RootType type_;
    /**
     * From which object current root was found by reference. Usually from_object is nullptr, except when object was
     * found from card_table
     */
    ObjectHeader *fromObject_;
    ObjectPointerType *object_;
};

template <class LanguageConfig>
class RootManager final {
public:
    explicit RootManager(PandaVM *vm) : vm_(vm) {};
    /// Visit all non-heap roots(registers, thread locals, etc)
    void VisitNonHeapRoots(const GCRootVisitor &gcRootVisitor,
                           VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL);

    /// Visit local roots for frame
    void VisitLocalRoots(const GCRootVisitor &gcRootVisitor);

    /**
     * Visit card table roots
     * @param card_table - card table for scan
     * @param allocator - object allocator
     * @param root_visitor
     * @param range_checker - return true if we need to check mem range for current card
     * @param range_object_checker - return true if we should apply root_visitor to the object obtained during
     * traversing
     * @param from_object_checker - return true if we need to traverse objects approved by range_checker
     */
    void VisitCardTableRoots(CardTable *cardTable, ObjectAllocatorBase *allocator, GCRootVisitor rootVisitor,
                             MemRangeChecker rangeChecker, ObjectChecker rangeObjectChecker,
                             ObjectChecker fromObjectChecker, uint32_t processedFlag) const;

    /// Visit roots in class linker
    void VisitClassRoots(const GCRootVisitor &gcRootVisitor,
                         VisitGCRootFlags flags = VisitGCRootFlags::ACCESS_ROOT_ALL) const;

    void VisitAotStringRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags) const;

    void UpdateAndSweep(const ReferenceUpdater &callback);

private:
    /// Visit VM-specific roots
    void VisitVmRoots(const GCRootVisitor &gcRootVisitor) const;

    template <class VRegRef, class VRegInfo>
    void VisitRegisterRoot(VRegRef &vRegister, VRegInfo &regInfo, StackWalker &pframe,
                           const GCRootVisitor &gcRootVisitor);

    void VisitClassLinkerContextRoots(const GCRootVisitor &gcRootVisitor) const;

    /**
     * Visit roots for @param thread
     * @param thread
     */
    void VisitRootsForThread(ManagedThread *thread, const GCRootVisitor &gcRootVisitor) const;

    PandaVM *vm_ {nullptr};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_ROOT_H
