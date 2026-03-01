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

#include "runtime/include/object_header.h"

#include "libarkbase/mem/mem.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/hclass.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/free_object.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/monitor_pool.h"
#include "runtime/handle_base-inl.h"

namespace ark {

namespace object_header_traits {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::atomic<uint32_t> g_hashSeed = std::atomic<uint32_t>(LINEAR_SEED + std::time(nullptr));

}  // namespace object_header_traits

/* static */
ObjectHeader *ObjectHeader::CreateObject(ManagedThread *thread, ark::BaseClass *klass, bool nonMovable)
{
    ASSERT(thread != nullptr);
    ASSERT(klass != nullptr);
#ifndef NDEBUG
    if (!klass->IsDynamicClass()) {
        auto cls = static_cast<ark::Class *>(klass);
        ASSERT(cls->IsInstantiable());
        ASSERT(!cls->IsArrayClass());
        ASSERT(!cls->IsStringClass());
    }
#endif

    size_t size = klass->GetObjectSize();
    ASSERT(size != 0);
    mem::HeapManager *heapManager = thread->GetVM()->GetHeapManager();
    ObjectHeader *obj {nullptr};
    if (UNLIKELY(heapManager->IsObjectFinalized(klass))) {
        nonMovable = true;
    }
    if (LIKELY(!nonMovable)) {
        obj = heapManager->AllocateObject(klass, size);
    } else {
        obj = heapManager->AllocateNonMovableObject(klass, size);
    }
    return obj;
}

ObjectHeader *ObjectHeader::CreateObject(ark::BaseClass *klass, bool nonMovable)
{
    return CreateObject(ManagedThread::GetCurrent(), klass, nonMovable);
}

/* static */
ObjectHeader *ObjectHeader::Create(ManagedThread *thread, BaseClass *klass)
{
    return CreateObject(thread, klass, false);
}

ObjectHeader *ObjectHeader::Create(BaseClass *klass)
{
    return CreateObject(klass, false);
}

/* static */
ObjectHeader *ObjectHeader::CreateNonMovable(BaseClass *klass)
{
    return CreateObject(klass, true);
}

uint32_t ObjectHeader::GetHashCodeFromMonitor(Monitor *monitorP)
{
    return monitorP->GetHashCode();
}

uint32_t ObjectHeader::GetHashCodeMTSingle()
{
    auto mark = GetMark();

    switch (mark.GetState()) {
        case MarkWord::STATE_UNLOCKED: {
            mark = mark.DecodeFromHash(GenerateHashCode());
            ASSERT(mark.GetState() == MarkWord::STATE_HASHED);
            SetMark(mark);
            return mark.GetHash();
        }
        case MarkWord::STATE_HASHED:
            return mark.GetHash();
        default:
            LOG(FATAL, RUNTIME) << "Error on GetHashCode(): invalid state";
            return 0;
    }
}

uint32_t ObjectHeader::GetHashCodeMTMulti()
{
    ObjectHeader *currentObj = this;
    while (true) {
        auto mark = currentObj->AtomicGetMark();
        auto *thread = MTManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, currentObj);

        switch (mark.GetState()) {
            case MarkWord::STATE_UNLOCKED: {
                auto hashMark = mark.DecodeFromHash(GenerateHashCode());
                ASSERT(hashMark.GetState() == MarkWord::STATE_HASHED);
                currentObj->AtomicSetMark(mark, hashMark);
                break;
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                os::thread::ThreadId ownerThreadId = mark.GetThreadId();
                if (ownerThreadId == thread->GetInternalId()) {
                    Monitor::Inflate(this, thread);
                } else {
                    Monitor::InflateThinLock(thread, handleObj);
                    currentObj = handleObj.GetPtr();
                }
                break;
            }
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitorId = mark.GetMonitorId();
                auto monitorP = thread->GetMonitorPool()->LookupMonitor(monitorId);
                if (monitorP != nullptr) {
                    return GetHashCodeFromMonitor(monitorP);
                }
                LOG(FATAL, RUNTIME) << "Error on GetHashCode(): no monitor on heavy locked state";
                break;
            }
            case MarkWord::STATE_HASHED: {
                return mark.GetHash();
            }
            default: {
                LOG(FATAL, RUNTIME) << "Error on GetHashCode(): invalid state";
            }
        }
    }
}

ObjectHeader *ObjectHeader::Clone(ObjectHeader *src)
{
    LOG_IF(src->ClassAddr<Class>()->GetManagedObject() == src, FATAL, RUNTIME) << "Can't clone a class";
    return ObjectHeader::ShallowCopy(src);
}

static ObjectHeader *AllocateObjectAndGetDst(ObjectHeader *src, Class *objectClass, size_t objSize,
                                             mem::HeapManager *heapManager)
{
    ObjectHeader *dst = nullptr;
    if (PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(src) == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT) {
        dst = heapManager->AllocateNonMovableObject(objectClass, objSize);
    } else {
        dst = heapManager->AllocateObject(objectClass, objSize);
    }
    return dst;
}

ObjectHeader *ObjectHeader::ShallowCopy(ObjectHeader *src)
{
    /*
     NOTE(d.trubenkov):
        use bariers for possible copied reference fields
    */
    auto objectClass = src->ClassAddr<Class>();
    std::size_t objSize = src->ObjectSize();

    // AllocateObject can trigger gc, use handle for src.
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    mem::HeapManager *heapManager = thread->GetVM()->GetHeapManager();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> srcHandle(thread, src);

    // Determine whether object is non-movable
    ObjectHeader *dst = AllocateObjectAndGetDst(src, objectClass, objSize, heapManager);
    if (dst == nullptr) {
        return nullptr;
    }
    ASSERT(PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(srcHandle.GetPtr()) ==
           PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(dst));

    Span<uint8_t> srcSp(reinterpret_cast<uint8_t *>(srcHandle.GetPtr()), objSize);
    Span<uint8_t> dstSp(reinterpret_cast<uint8_t *>(dst), objSize);
    constexpr const std::size_t WORD_SIZE = sizeof(uintptr_t);
    std::size_t bytesToCopy = objSize - ObjectHeader::ObjectHeaderSize();
    std::size_t wordsToCopy = bytesToCopy / WORD_SIZE;
    std::size_t wordsToCopyEnd = ObjectHeader::ObjectHeaderSize() + WORD_SIZE * wordsToCopy;
    std::size_t objectPointersToCopy = (bytesToCopy - WORD_SIZE * wordsToCopy) / OBJECT_POINTER_SIZE;
    std::size_t objectPointersToCopyEnd = wordsToCopyEnd + objectPointersToCopy * OBJECT_POINTER_SIZE;
    // copy words
    for (std::size_t i = ObjectHeader::ObjectHeaderSize(); i < wordsToCopyEnd; i += WORD_SIZE) {
        // Atomic with relaxed order reason: data race with src_handle with no synchronization or ordering constraints
        // imposed on other reads or writes
        reinterpret_cast<std::atomic<uintptr_t> *>(&dstSp[i])->store(
            reinterpret_cast<std::atomic<uintptr_t> *>(&srcSp[i])->load(std::memory_order_relaxed),
            std::memory_order_relaxed);
    }
    // copy remaining memory by object pointer
    for (std::size_t i = wordsToCopyEnd; i < objectPointersToCopyEnd; i += OBJECT_POINTER_SIZE) {
        reinterpret_cast<std::atomic<ark::ObjectPointerType> *>(&dstSp[i])->store(
            // Atomic with relaxed order reason: data race with src_handle with no synchronization or ordering
            // constraints imposed on other reads or writes
            reinterpret_cast<std::atomic<ark::ObjectPointerType> *>(&srcSp[i])->load(std::memory_order_relaxed),
            std::memory_order_relaxed);
    }
    // copy remaining memory by bytes
    for (std::size_t i = objectPointersToCopyEnd; i < objSize; i++) {
        // Atomic with relaxed order reason: data race with src_handle with no synchronization or ordering constraints
        // imposed on other reads or writes
        reinterpret_cast<std::atomic<uint8_t> *>(&dstSp[i])->store(
            reinterpret_cast<std::atomic<uint8_t> *>(&srcSp[i])->load(std::memory_order_relaxed),
            std::memory_order_relaxed);
    }

    // Call barriers here.
    auto *barrierSet = thread->GetBarrierSet();
    // We don't need pre barrier here because we don't change any links inside main object
    // Post barrier
    if (!mem::IsEmptyBarrier(barrierSet->GetPostType())) {
        if (!objectClass->IsArrayClass() || !objectClass->GetComponentType()->IsPrimitive()) {
            barrierSet->PostBarrier(dst, 0, objSize);
        }
    }
    return dst;
}

size_t ObjectHeader::ObjectSize() const
{
    auto *baseKlass = ClassAddr<BaseClass>();
    if (baseKlass->IsDynamicClass()) {
        return ObjectSizeDyn(baseKlass);
    }
    return ObjectSizeStatic(baseKlass);
}

size_t ObjectHeader::ObjectSizeDyn(BaseClass *baseKlass) const
{
    // if we do it concurrently, the real klass may be changed,
    // but we are ok with that
    auto *klass = static_cast<HClass *>(baseKlass);

    if (klass->IsArray()) {
        return static_cast<const coretypes::Array *>(this)->ObjectSize(TaggedValue::TaggedTypeSize());
    }
    if (klass->IsString()) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(klass->GetSourceLang());
        return ctx.GetStringSize(this);
    }
    if (klass->IsFreeObject()) {
        return static_cast<const mem::FreeObject *>(this)->GetSize();
    }
    return baseKlass->GetObjectSize();
}

size_t ObjectHeader::ObjectSizeStatic(BaseClass *baseKlass) const
{
    ASSERT(baseKlass == ClassAddr<BaseClass>());
    auto *klass = static_cast<Class *>(baseKlass);

    if (klass->IsArrayClass()) {
        return static_cast<const coretypes::Array *>(this)->ObjectSize(klass->GetComponentSize());
    }

    if (klass->IsStringClass()) {
        return static_cast<const coretypes::String *>(this)->ObjectSize();
    }

    if (klass->IsClassClass()) {
        auto cls = ark::Class::FromClassObject(const_cast<ObjectHeader *>(this));
        if (cls != nullptr) {
            return ark::Class::GetClassObjectSizeFromClass(cls, klass->GetSourceLang());
        }
    }
    return baseKlass->GetObjectSize();
}

}  // namespace ark
