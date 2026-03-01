/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZATION_REGISTRY_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZATION_REGISTRY_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_finreg_node.h"

namespace ark::ets {

namespace test {
class EtsFinalizationRegistryLayoutTest;
}  // namespace test

/// @class A mirror class for std.core.FinalizationRegistry
class EtsFinalizationRegistry : public EtsObject {
public:
    EtsFinalizationRegistry() = delete;
    NO_COPY_SEMANTIC(EtsFinalizationRegistry);
    NO_MOVE_SEMANTIC(EtsFinalizationRegistry);
    ~EtsFinalizationRegistry() = delete;

    CoroutineWorkerDomain GetWorkerDomain() const
    {
        return static_cast<CoroutineWorkerDomain>(workerDomain_);
    }

    CoroutineWorker::Id GetWorkerId() const
    {
        return static_cast<CoroutineWorker::Id>(workerId_);
    }

    EtsFinalizationRegistry *GetNextFinReg()
    {
        return reinterpret_cast<EtsFinalizationRegistry *>(
            ObjectAccessor::GetObject(GetCoreType(), GetNextFinRegOffset()));
    }

    void SetNextFinReg(EtsFinalizationRegistry *next)
    {
        ObjectAccessor::SetObject(GetCoreType(), GetNextFinRegOffset(), EtsObject::ToCoreType(next));
    }

    EtsFinRegNode *GetFinalizationQueueHead()
    {
        return reinterpret_cast<EtsFinRegNode *>(ObjectAccessor::GetObject(this, GetFinalizationQueueHeadOffset()));
    }

    void SetFinalizationQueueHead(EtsFinRegNode *head)
    {
        ObjectAccessor::SetObject(this, GetFinalizationQueueHeadOffset(), EtsObject::ToCoreType(head));
    }

    EtsFinRegNode *GetFinalizationQueueTail()
    {
        return reinterpret_cast<EtsFinRegNode *>(ObjectAccessor::GetObject(this, GetFinalizationQueueTailOffset()));
    }

    void SetFinalizationQueueTail(EtsFinRegNode *tail)
    {
        ObjectAccessor::SetObject(this, GetFinalizationQueueTailOffset(), EtsObject::ToCoreType(tail));
    }

    static void Enqueue(EtsFinRegNode *node, EtsFinalizationRegistry *finReg);

    static EtsFinalizationRegistry *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsFinalizationRegistry *>(obj);
    }

private:
    static constexpr size_t GetNextFinRegOffset()
    {
        return MEMBER_OFFSET(EtsFinalizationRegistry, nextFinReg_);
    }

    static constexpr size_t GetBucketsOffset()
    {
        return MEMBER_OFFSET(EtsFinalizationRegistry, buckets_);
    }

    static constexpr size_t GetNonUnregistrableListOffset()
    {
        return MEMBER_OFFSET(EtsFinalizationRegistry, nonUnregistrableList_);
    }

    EtsTypedObjectArray<EtsFinRegNode> *GetBuckets() const
    {
        return reinterpret_cast<EtsTypedObjectArray<EtsFinRegNode> *>(
            ObjectAccessor::GetObject(this, GetBucketsOffset()));
    }

    EtsFinRegNode *GetNonUnregistrableList() const
    {
        return reinterpret_cast<EtsFinRegNode *>(ObjectAccessor::GetObject(this, GetNonUnregistrableListOffset()));
    }

    void SetNonUnregistrableList(EtsFinRegNode *node)
    {
        ObjectAccessor::SetObject(this, GetNonUnregistrableListOffset(), EtsObject::ToCoreType(node));
    }

    static constexpr size_t GetFinalizationQueueHeadOffset()
    {
        return MEMBER_OFFSET(EtsFinalizationRegistry, finalizationQueueHead_);
    }

    static constexpr size_t GetFinalizationQueueTailOffset()
    {
        return MEMBER_OFFSET(EtsFinalizationRegistry, finalizationQueueTail_);
    }

    void DecrementListSize()
    {
        listSize_ -= 1;
    }

    void DecrementMapSize()
    {
        mapSize_ -= 1;
    }

    static void Remove(EtsFinRegNode *node, EtsFinalizationRegistry *finReg);

    ObjectPointer<EtsObject> callback_;
    ObjectPointer<EtsObject> mutex_;
    ObjectPointer<EtsTypedObjectArray<EtsFinRegNode>> buckets_;
    ObjectPointer<EtsFinRegNode> nonUnregistrableList_;
    ObjectPointer<EtsFinalizationRegistry> nextFinReg_;
    ObjectPointer<EtsFinRegNode> finalizationQueueHead_;
    ObjectPointer<EtsFinRegNode> finalizationQueueTail_;
    EtsInt workerId_;
    EtsInt workerDomain_;
    EtsInt mapSize_;
    EtsInt listSize_;

    friend class test::EtsFinalizationRegistryLayoutTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZATION_REGISTRY_H
