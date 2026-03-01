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
#include "global_object_storage.h"

#include <libarkbase/os/mutex.h>
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/include/class.h"
#include "reference.h"
#include "libarkbase/utils/logger.h"

namespace ark::mem {

GlobalObjectStorage::GlobalObjectStorage(mem::InternalAllocatorPtr allocator, size_t maxSize, bool enableSizeCheck)
    : allocator_(allocator)
{
    globalStorage_ = allocator->New<ArrayStorage>(allocator, maxSize, enableSizeCheck);
    weakStorage_ = allocator->New<ArrayStorage>(allocator, maxSize, enableSizeCheck);
    globalFixedStorage_ = allocator->New<ArrayStorage>(allocator, maxSize, enableSizeCheck, true);
}

GlobalObjectStorage::~GlobalObjectStorage()
{
    allocator_->Delete(globalStorage_);
    allocator_->Delete(weakStorage_);
    allocator_->Delete(globalFixedStorage_);
}

bool GlobalObjectStorage::IsValidGlobalRef(const Reference *ref) const
{
    ASSERT(ref != nullptr);
    Reference::ObjectType type = Reference::GetType(ref);
    AssertType(type);
    if (type == Reference::ObjectType::GLOBAL) {
        if (!globalStorage_->IsValidGlobalRef(ref)) {
            return false;
        }
    } else if (type == Reference::ObjectType::WEAK) {
        if (!weakStorage_->IsValidGlobalRef(ref)) {
            return false;
        }
    } else {
        if (!globalFixedStorage_->IsValidGlobalRef(ref)) {
            return false;
        }
    }
    return true;
}

Reference *GlobalObjectStorage::Add(const ObjectHeader *object, Reference::ObjectType type) const
{
    AssertType(type);
    if (object == nullptr) {
        return nullptr;
    }
    Reference *ref = nullptr;
    if (type == Reference::ObjectType::GLOBAL) {
        ref = globalStorage_->Add(object);
    } else if (type == Reference::ObjectType::WEAK) {
        ref = weakStorage_->Add(object);
    } else {
        ref = globalFixedStorage_->Add(object);
    }
    if (ref != nullptr) {
        ref = Reference::SetType(ref, type);
    }
    return ref;
}

void GlobalObjectStorage::Remove(const Reference *reference)
{
    if (reference == nullptr) {
        return;
    }
    auto type = reference->GetType();
    AssertType(type);
    reference = Reference::GetRefWithoutType(reference);
    if (type == Reference::ObjectType::GLOBAL) {
        globalStorage_->Remove(reference);
    } else if (type == Reference::ObjectType::WEAK) {
        weakStorage_->Remove(reference);
    } else {
        globalFixedStorage_->Remove(reference);
    }
}

PandaVector<ObjectHeader *> GlobalObjectStorage::GetAllObjects()
{
    auto objects = PandaVector<ObjectHeader *>(allocator_->Adapter());

    auto globalObjects = globalStorage_->GetAllObjects();
    objects.insert(objects.end(), globalObjects.begin(), globalObjects.end());

    auto weakObjects = weakStorage_->GetAllObjects();
    objects.insert(objects.end(), weakObjects.begin(), weakObjects.end());

    auto fixedObjects = globalFixedStorage_->GetAllObjects();
    objects.insert(objects.end(), fixedObjects.begin(), fixedObjects.end());

    return objects;
}

void GlobalObjectStorage::VisitObjects(const GCRootVisitor &gcRootVisitor, mem::RootType rootType)
{
    globalStorage_->VisitObjects(gcRootVisitor, rootType);
    globalFixedStorage_->VisitObjects(gcRootVisitor, rootType);
}

void GlobalObjectStorage::UpdateAndSweep(const ReferenceUpdater &updater)
{
    weakStorage_->UpdateAndSweep(updater);
}

void GlobalObjectStorage::ClearUnmarkedWeakRefs(const GC *gc, const mem::GC::ReferenceClearPredicateT &pred)
{
    ClearWeakRefs([gc, &pred](auto *obj) { return pred(obj) && !gc->IsMarked(obj); });
}

void GlobalObjectStorage::ClearWeakRefs(const mem::GC::ReferenceClearPredicateT &pred)
{
    weakStorage_->ClearWeakRefs(pred);
}

size_t GlobalObjectStorage::GetSize()
{
    return globalStorage_->GetSizeWithLock() + weakStorage_->GetSizeWithLock() + globalFixedStorage_->GetSizeWithLock();
}

void GlobalObjectStorage::Dump()
{
    globalStorage_->DumpWithLock();
}
}  // namespace ark::mem
