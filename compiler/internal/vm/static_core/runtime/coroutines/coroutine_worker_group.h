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

#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_GROUP_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_GROUP_H

#include "runtime/coroutines/affinity_mask.h"
#include "runtime/coroutines/coroutine_worker.h"
#include "runtime/coroutines/coroutine_worker_domain.h"

namespace ark {

class CoroutineManager;

/**
 * @brief A group of coroutine workers
 *
 * A coroutine worker group is the set of workers with a unique ID that can be targeted at the coroutine launch time.
 *
 * CoroutineManager's Launch* APIs accept the CoroutineWorkerGroup unique ID and then schedule the new coroutine to
 * on of the workers from the target worker group.
 */
class CoroutineWorkerGroup {
public:
    using Id = AffinityMask::Storage;

    CoroutineWorkerGroup() = delete;

    static Id InvalidId()
    {
        return Id().reset();
    }

    static Id AnyId()
    {
        return Id().set();
    }

    static Id Empty()
    {
        return Id().reset();
    }

    static Id ExtendGroup(const Id &group, CoroutineWorker::Id newWorkerId,
                          [[maybe_unused]] bool checkDuplicates = true)
    {
        ASSERT(newWorkerId != CoroutineWorker::INVALID_ID);
        ASSERT(!checkDuplicates || ((group & FromWorkerId(newWorkerId)) == Empty()));
        return group | FromWorkerId(newWorkerId);
    }

    static Id ShrinkGroup(const Id &group, CoroutineWorker::Id workerId)
    {
        ASSERT(workerId != CoroutineWorker::INVALID_ID);
        ASSERT((group & FromWorkerId(workerId)) != Empty());
        return group & ~FromWorkerId(workerId);
    }

    static CoroutineWorkerGroup::Id GenerateExactWorkerId(CoroutineWorker::Id hint)
    {
        ASSERT(hint != CoroutineWorker::INVALID_ID);
        return FromWorkerId(hint);
    }

    static bool HasOnlyWorker(const CoroutineWorkerGroup::Id &groupId, CoroutineWorker::Id workerId)
    {
        return groupId == FromWorkerId(workerId);
    }

    static bool HasWorker(const CoroutineWorkerGroup::Id &groupId, CoroutineWorker::Id workerId)
    {
        return (groupId & FromWorkerId(workerId)) != Empty();
    }

    // Converts 128 bitset into two uint64_t
    static std::tuple<uint64_t, uint64_t> ToTuple(const Id &groupId)
    {
        uint64_t lower = 0;
        uint64_t upper = 0;
        for (size_t i = 0; i < UL64; ++i) {
            if (groupId[i]) {
                lower |= static_cast<uint64_t>(1) << i;
            }
            if (groupId[i + UL64]) {
                upper |= static_cast<uint64_t>(1) << i;
            }
        }
        return {lower, upper};
    }

    // Converts two uint64_t into 128 bitset
    static Id FromTuple(const std::tuple<uint64_t, uint64_t> &t)
    {
        const auto &[lower, upper] = t;
        CoroutineWorkerGroup::Id groupId;
        for (size_t i = 0; i < UL64; ++i) {
            groupId[i] = (((lower >> i) & 1ULL) != 0U);
            groupId[i + UL64] = (((upper >> i) & 1ULL) != 0U);
        }
        return groupId;
    }

    static Id FromDomain(CoroutineManager *coroman, CoroutineWorkerDomain domain,
                         const PandaVector<CoroutineWorker::Id> &hint);

    static Id FromDomain(CoroutineManager *coroman, CoroutineWorkerDomain domain)
    {
        return FromDomain(coroman, domain, {});
    }

private:
    static Id FromWorkerId(CoroutineWorker::Id newId)
    {
        auto group = Empty();
        group |= 1U;
        return group << (size_t)newId;
    }

    static constexpr uint64_t UL64 = 64U;
};

static_assert(std::is_same_v<CoroutineWorkerGroup::Id, AffinityMask::CoroutineWorkerGroupId>);

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_H */
