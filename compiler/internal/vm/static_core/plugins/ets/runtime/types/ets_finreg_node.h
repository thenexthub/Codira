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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINREG_NODE_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINREG_NODE_H

#include "runtime/coroutines/coroutine_worker.h"
#include "runtime/coroutines/coroutine_worker_domain.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"

namespace ark::ets {

class EtsCoroutine;
class EtsFinalizationRegistry;
namespace test {
class EtsFinalizationRegistryLayoutTest;
}  // namespace test

/// @class A mirror class for std.core.FinRegNode
class EtsFinRegNode : public EtsWeakReference {
public:
    EtsFinRegNode() = delete;
    NO_COPY_SEMANTIC(EtsFinRegNode);
    NO_MOVE_SEMANTIC(EtsFinRegNode);
    ~EtsFinRegNode() = delete;

    bool HasToken() const
    {
        return ObjectAccessor::GetObject(this, GetTokenOffset()) != nullptr;
    }

    EtsFinRegNode *GetPrev() const
    {
        return reinterpret_cast<EtsFinRegNode *>(ObjectAccessor::GetObject(this, GetPrevOffset()));
    }

    void SetPrev(EtsFinRegNode *prev)
    {
        return ObjectAccessor::SetObject(this, GetPrevOffset(), ToCoreType(prev));
    }

    EtsFinRegNode *GetNext() const
    {
        return reinterpret_cast<EtsFinRegNode *>(ObjectAccessor::GetObject(this, GetNextOffset()));
    }

    void SetNext(EtsFinRegNode *next)
    {
        return ObjectAccessor::SetObject(this, GetNextOffset(), ToCoreType(next));
    }

    EtsInt GetBucketIdx() const
    {
        return bucketIdx_;
    }

    EtsFinalizationRegistry *GetFinalizationRegistry() const;
    CoroutineWorkerDomain GetWorkerDomain() const;
    CoroutineWorker::Id GetWorkerId() const;

private:
    static constexpr size_t GetTokenOffset()
    {
        return MEMBER_OFFSET(EtsFinRegNode, token_);
    }

    static constexpr size_t GetFinalizationRegistryOffset()
    {
        return MEMBER_OFFSET(EtsFinRegNode, finReg_);
    }

    static constexpr size_t GetPrevOffset()
    {
        return MEMBER_OFFSET(EtsFinRegNode, prev_);
    }

    static constexpr size_t GetNextOffset()
    {
        return MEMBER_OFFSET(EtsFinRegNode, next_);
    }

    ObjectPointer<EtsObject> token_;
    ObjectPointer<EtsObject> arg_;
    ObjectPointer<EtsObject> finReg_;
    ObjectPointer<EtsFinRegNode> prev_;
    ObjectPointer<EtsFinRegNode> next_;
    EtsInt bucketIdx_;

    friend class test::EtsFinalizationRegistryLayoutTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINREG_NODE_H
